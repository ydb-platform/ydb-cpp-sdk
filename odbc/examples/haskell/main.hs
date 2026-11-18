import Control.Exception (bracket)
import Control.Monad (forM_)
import Database.HDBC
import Database.HDBC.ODBC (connectODBC)
import System.Environment (getArgs, getEnv)
import System.Exit (die)

main :: IO ()
main = do
    args <- getArgs
    connectionString <- case args of
        [] -> getEnv "ODBC_CONNECTION_STRING"
        [value] -> pure value
        _ -> die "Usage: runghc main.hs [connection-string]"
    bracket (connectODBC connectionString) disconnect $ \connection -> do
        rows <- quickQuery' connection
            "SELECT CAST(? AS Int64) AS answer, CAST(? AS Utf8) AS message"
            [toSql (42 :: Integer), toSql ("Hello YDB" :: String)]
        forM_ rows $ \row -> case row of
            [number, message] ->
                putStrLn $ show (fromSql number :: Integer) ++ "\t" ++ (fromSql message :: String)
            _ -> fail "Expected two columns"
        commit connection
