"""Run the same boundary scenarios through the installed public bindings."""
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parent
CASES = ("connection", "binding", "metadata", "row_count", "transactions",
         "independent_connections", "cleanup", "multiple_results")


def case_names(consumer):
    # DBI/odbc and LuaSQL expose no public result-set advancement API.
    cases = [case for case in CASES if case != "multiple_results" or consumer not in {"r", "lua"}]
    if consumer in {"qt", "ruby"}:
        cases.append("concurrency")
    return cases


def variants(consumer):
    return ("odbc", "odbc_utf8") if consumer == "ruby" else ("",)


def inventory(consumer, mode):
    return [f"{consumer}.{mode}.contract." + (variant + "." if variant else "") + case
            for variant in variants(consumer) for case in case_names(consumer)]


def parse_results(consumer, mode, variant, output, returncode, started, stopped, log):
    records, errors = {}, []
    names = case_names(consumer)
    for line in output.splitlines():
        if not line.startswith("@YDB_CONTRACT\t"):
            continue
        try:
            _, name, status, diagnostic = line.split("\t", 3)
            if name not in names or name in records or status not in {"passed", "failed"}:
                raise ValueError("unexpected, duplicate or invalid contract record")
            records[name] = status, bytes.fromhex(diagnostic).decode("utf-8", errors="replace")
        except ValueError as error:
            errors.append(str(error))
    if returncode not in (0, 1):
        errors.append(f"contract process exited {returncode}")
    if returncode == 1 and len(records) == len(names) and all(r[0] == "passed" for r in records.values()):
        errors.append("contract process failed despite passing records")
    prefix = f"{consumer}.{mode}.contract." + (variant + "." if variant else "")
    return [dict(id=prefix + name, name="Binding contract: " + name,
                 status="broken" if errors or name not in records else records[name][0],
                 message="; ".join(errors) if errors else records.get(name, (None, "Missing contract result"))[1],
                 phase="test", start=started, stop=stopped,
                 attachments=[dict(name=log.name, path=log.name, type="text/plain")])
            for name in names]


def run(consumer, mode):
    native = Path(os.environ["ODBC_NATIVE_RESULTS_DIR"])
    env = os.environ.copy()
    connection = env["YDB_ODBC_DSN" if mode == "dsn" else "YDB_ODBC_CONNECTION_STRING"]
    commands = []
    if consumer == "ruby":
        commands = [(variant, ["ruby", "-I", "/work/source/" + directory,
                              ROOT / "ruby.rb", variant, mode, connection])
                    for variant, directory in (("odbc", "ext"), ("odbc_utf8", "ext/utf8"))]
    elif consumer == "r":
        env["R_LIBS_USER"] = "/work/R-library"
        commands = [("", ["Rscript", ROOT / "r.R", mode, connection])]
    elif consumer == "lua":
        env["LUA_CPATH"] = "/work/modules/?.so;;"
        commands = [("", ["lua5.4", ROOT / "lua.lua", mode, connection])]
    elif consumer in {"php-odbc", "php-pdo"}:
        commands = [("", ["php", ROOT / "php.php", consumer])]
    elif consumer == "qt":
        flags = subprocess.check_output(["pkg-config", "--cflags", "--libs", "Qt6Core", "Qt6Sql"], text=True)
        subprocess.run(["g++", "-std=c++20", "-pthread", str(ROOT / "qt.cpp"), "-o", "/work/qt-contract",
                        *shlex.split(flags)], check=True)
        commands = [("", ["/work/qt-contract", connection])]
    else:
        raise ValueError("Unknown binding: " + consumer)
    results = []
    for variant, command in commands:
        log = native / ("contract-" + (variant or consumer) + ".log")
        started = int(time.time() * 1000)
        try:
            completed = subprocess.run([str(arg) for arg in command], env=env, text=True,
                                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=180)
            output, code = completed.stdout, completed.returncode
        except subprocess.TimeoutExpired as error:
            output = (error.stdout or b"").decode("utf-8", errors="replace")
            code = 124
        log.write_text(output)
        results.extend(parse_results(consumer, mode, variant, output, code, started,
                                     int(time.time() * 1000), log))
    document = {"schema_version": 1, "inventory": inventory(consumer, mode), "tests": results}
    (native / "contracts.json").write_text(json.dumps(document, indent=2) + "\n")
    return int(any(result["status"] != "passed" for result in results))


def merge(native):
    path = native / "results.json"
    upstream = json.loads(path.read_text())
    contract = json.loads((native / "contracts.json").read_text())
    upstream["inventory"].extend(contract["inventory"])
    upstream["tests"].extend(contract["tests"])
    if upstream.get("inventory_kind") == "parents":
        upstream["inventory_rows"].update(dict.fromkeys(contract["inventory"], 1))
    path.write_text(json.dumps(upstream, indent=2) + "\n")


if __name__ == "__main__":
    raise SystemExit(run(*sys.argv[1:]))
