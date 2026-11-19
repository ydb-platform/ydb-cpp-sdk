import Control.Concurrent (threadDelay)
import Control.Exception (bracket, bracket_, throwIO)
import Control.Monad (forM_, void, when)
import qualified Data.ByteString as BS
import Data.List (intercalate)
import Data.Time.Calendar (fromGregorian)
import Database.HDBC
import Database.HDBC.ODBC (connectODBC, setAutoCommit)
import System.Environment (getArgs, getEnv)
import System.Exit (die)
import System.IO (hSetEncoding, stdout, utf8)
import System.Posix.Process (getProcessID)
import Text.Read (readMaybe)

retryTransaction :: IConnection conn => conn -> (Int -> IO a) -> IO (Int, a)
retryTransaction connection action = attempt 1
  where
    attempt number = catchSql (do
        result <- action number
        commit connection
        pure (number, result)) $ \err -> do
            rollback connection
            -- HDBC-odbc stores SQLSTATEs as a shown list. Ambiguous outcomes propagate.
            let states = maybe [seState err] id (readMaybe (seState err))
            if states == ["40001"] && number < 3
                then do
                    putStrLn $ "retry SQLSTATE: " ++ intercalate "," states
                    threadDelay (10000 * number)
                    attempt (number + 1)
                else throwIO err

main :: IO ()
main = do
    hSetEncoding stdout utf8
    args <- getArgs
    connectionString <- case args of
        [] -> getEnv "ODBC_CONNECTION_STRING"
        [value] -> pure value
        _ -> die "Usage: runghc main.hs [connection-string]"
    let close connection = rollback connection >> disconnect connection
    bracket (connectODBC connectionString) close $ \connection -> do
        processId <- getProcessID
        let table = "odbc_example_hdbc_" ++ show processId
            create = "CREATE TABLE " ++ table ++ " (id Int64, message Utf8, note Utf8, " ++
                "score Double, created Date, minimum Int64, maximum Int64, payload String, " ++
                "long_text Utf8, PRIMARY KEY (id))"
            cleanup = do
                rollback connection
                void $ setAutoCommit connection True
                runRaw connection ("DROP TABLE " ++ table)
            display SqlNull = "NULL"
            display value = fromSql value :: String
            printRows label rows = forM_ rows $ \row ->
                putStrLn $ label ++ intercalate "\t" (map display row)
        -- CREATE fails if the example's table already exists. Only our table is removed.
        bracket_ (void (setAutoCommit connection True) >> runRaw connection create) cleanup $ do
            tables <- getTables connection
            putStrLn $ "table found: " ++ show (table `elem` tables)
            schema <- describeTable connection table
            putStrLn $ "schema: " ++ intercalate ", "
                [name ++ "=" ++ show (colType column) | (name, column) <- schema]

            inserted <- run connection ("INSERT INTO " ++ table ++
                " (id, message, note, score, created, minimum, maximum, payload, long_text) " ++
                "VALUES (CAST(? AS Int64), CAST(? AS Utf8), CAST(? AS Utf8), CAST(? AS Double), " ++
                "CAST(? AS Date), CAST(? AS Int64), CAST(? AS Int64), CAST(? AS String), CAST(? AS Utf8))")
                [toSql (1 :: Integer), toSql ("Привет 世界 😀" :: String), SqlNull,
                 toSql (3.5 :: Double), toSql (fromGregorian 2024 2 29),
                 toSql (-9223372036854775808 :: Integer), toSql (9223372036854775807 :: Integer),
                 toSql (BS.pack [0, 1, 127, 128, 255]), toSql (replicate 12000 'x')]
            putStrLn $ "inserted: " ++ show inserted
            bracket (prepare connection ("SELECT * FROM " ++ table ++ " WHERE id = 1")) finish $ \statement -> do
                void $ execute statement []
                names <- getColumnNames statement
                putStrLn $ "columns: " ++ intercalate ", " names
                rows <- fetchAllRows' statement
                forM_ rows $ \row -> case row of
                    [key, message, note, score, created, minimumValue, maximumValue, payload, longText] -> do
                        printRows "row: " [[key, message, note, score, created, minimumValue, maximumValue]]
                        putStrLn $ "binary: " ++ show (BS.unpack (fromSql payload))
                        putStrLn $ "long text length: " ++ show (length (fromSql longText :: String))
                        putStrLn $ "long text intact: " ++ show ((fromSql longText :: String) == replicate 12000 'x')
                    _ -> fail "Expected nine columns"

            void $ setAutoCommit connection False
            bracket (prepare connection ("INSERT INTO " ++ table ++
                " (id, message) VALUES (CAST(? AS Int64), CAST(? AS Utf8))")) finish $ \statement ->
                executeMany statement [[toSql key, toSql message] |
                    (key, message) <- [(2 :: Integer, "batch two" :: String), (3, "batch three")]]
            bracket (prepare connection ("SELECT id, message FROM " ++ table ++
                " WHERE id = CAST(? AS Int64)")) finish $ \statement ->
                forM_ [2, 3 :: Integer] $ \key -> do
                    void $ execute statement [toSql key]
                    fetchAllRows' statement >>= printRows "reused: "

            -- HDBC starts with autocommit disabled; commit and rollback end each transaction.
            commit connection
            -- Autocommit gives each update its own affected-row count.
            void $ setAutoCommit connection True
            changed <- run connection ("UPDATE " ++ table ++ " SET message = CAST(? AS Utf8) WHERE id = 3")
                [toSql ("updated" :: String)]
            putStrLn $ "updated: " ++ show changed
            unchanged <- run connection ("UPDATE " ++ table ++ " SET message = CAST(? AS Utf8) WHERE id = 99")
                [toSql ("missing" :: String)]
            putStrLn $ "updated missing: " ++ show unchanged
            deleted <- run connection ("DELETE FROM " ++ table ++ " WHERE id = 3") []
            putStrLn $ "deleted: " ++ show deleted
            void $ setAutoCommit connection False
            void $ run connection ("UPDATE " ++ table ++ " SET message = CAST(? AS Utf8) WHERE id = 2")
                [toSql ("temporary" :: String)]
            rollback connection
            quickQuery' connection ("SELECT id, message FROM " ++ table ++ " WHERE id = 2") []
                >>= printRows "after rollback: "
            void $ run connection ("UPDATE " ++ table ++ " SET message = CAST(? AS Utf8) WHERE id = 2")
                [toSql ("committed" :: String)]
            commit connection
            bracket (clone connection) close $ \reader -> do
                quickQuery' reader ("SELECT id, message FROM " ++ table ++ " WHERE id = 2") []
                    >>= printRows "after commit: "
                commit reader
                (attempts, ()) <- retryTransaction connection $ \number -> do
                    values <- quickQuery' connection ("SELECT score FROM " ++ table ++ " WHERE id = 1") []
                    score <- case values of
                        [[value]] -> pure (fromSql value :: Double)
                        _ -> fail "Expected one score"
                    -- An independent writer forces a real conflict on the first attempt.
                    when (number == 1) $ do
                        void $ run reader ("UPDATE " ++ table ++ " SET score = 9.0 WHERE id = 1") []
                        commit reader
                    void $ run connection ("UPDATE " ++ table ++ " SET score = CAST(? AS Double) WHERE id = 1")
                        [toSql (score + 1)]
                putStrLn $ "transaction attempts: " ++ show attempts
                quickQuery' connection ("SELECT id, score FROM " ++ table ++ " WHERE id = 1") []
                    >>= printRows "after retry: "
                commit connection
