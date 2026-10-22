module Main where
import qualified Tests
import Test.HUnit
import Test.HUnit.Text (PutText(..), runTestText)
import System.Environment (getArgs, getEnv)
import System.FilePath ((</>))
import Control.Monad (forM_)

leaves prefix (TestLabel label test) = leaves (prefix ++ label ++ "/") test
leaves prefix (TestList tests) = concat [leaves (prefix ++ show i ++ "/") test | (i,test) <- zip [0..] tests]
leaves prefix test = [(prefix,test)]

main = do
  args <- getArgs
  directory <- getEnv "ODBC_NATIVE_RESULTS_DIR"
  let tests = leaves "" Tests.tests
  if args == ["--list"]
    then forM_ tests $ \(name,_) -> putStrLn ("@YDB_INVENTORY\t" ++ name)
    else forM_ (zip [0..] tests) $ \(i,(name,test)) -> do
      (counts,output) <- runTestText (PutText (\message _ messages -> return (messages ++ message ++ "\n")) "") test
      let filename = "hunit-" ++ show i ++ ".log"
      writeFile (directory </> filename) output
      putStrLn ("@YDB_CASE\t" ++ name ++ "\t" ++ (if errors counts + failures counts == 0 then "passed" else "failed") ++ "\t" ++ filename)
