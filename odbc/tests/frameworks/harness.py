#!/usr/bin/env python3
import argparse
from collections import Counter
import fnmatch
import hashlib
import json
import os
import pwd
import re
import shutil
import subprocess
import sys
import tarfile
import time
import urllib.request
import uuid
from pathlib import Path
import yaml
ROOT = Path(__file__).resolve().parent
REGISTRY = ROOT / "registry.yaml"
RESULTS = Path("/results")
WORK = Path("/work")
IMAGE_RE = re.compile(r"^.+@sha256:[0-9a-f]{64}$")
SOURCE_RE = re.compile(r"^[0-9a-f]{64}$")
class HarnessError(ValueError):
    pass
def require(condition, message):
    if not condition:
        raise HarnessError(message)
def expand_pattern(pattern):
    match = re.search(r"\{([^{}]+)\}", pattern)
    if not match:
        return [pattern]
    return [expanded for value in match.group(1).split(",")
            for expanded in expand_pattern(pattern[:match.start()] + value + pattern[match.end():])]
def glob_ids(names, pattern):
    return {name for name in names if any(fnmatch.fnmatch(name, item) for item in expand_pattern(pattern))}
def read_yaml(path):
    try:
        return yaml.safe_load(path.read_text(encoding="utf-8"))
    except (OSError, yaml.YAMLError) as error:
        raise HarnessError(f"cannot read {path}: {error}") from error
def load_classifications(data, consumers):
    consumer_ids = {item.get("id") for item in consumers if isinstance(item, dict)}
    grouped = {consumer: {"unsupported": {}, "skipped": {}} for consumer in consumer_ids}
    classifications = data.get("classifications", [])
    require(isinstance(classifications, list), "classifications must be a list")
    for index, group in enumerate(classifications):
        require(isinstance(group, dict), f"classification {index}: must be a mapping")
        outcome, reason, tests = group.get("outcome"), group.get("reason"), group.get("tests")
        require(outcome in {"unsupported", "skipped"},
                f"classification {index}: invalid outcome")
        require(isinstance(reason, str) and reason.strip(),
                f"classification {index}: reason must be a non-empty string")
        match = group.get("match")
        require(isinstance(match, str) and match.strip(),
                f"classification {index}: a diagnostic match is required")
        re.compile(match)
        require(isinstance(tests, dict) and tests,
                f"classification {index}: tests must be a non-empty consumer mapping")
        for consumer, patterns in tests.items():
            require(consumer in consumer_ids, f"classification {index}: unknown consumer: {consumer}")
            require(isinstance(patterns, list) and patterns
                    and all(isinstance(pattern, str) and pattern for pattern in patterns),
                    f"classification {index}: {consumer} patterns must be non-empty strings")
            for pattern in patterns:
                require(pattern not in grouped[consumer][outcome],
                        f"duplicate classification: {consumer}: {pattern}")
                grouped[consumer][outcome][pattern] = {"reason": reason, "match": match}
    return grouped
def load_registry(path=REGISTRY):
    data = read_yaml(path); require(isinstance(data, dict) and data.get("schema_version") == 2,
                                    "unsupported registry schema")
    ydb = data.get("ydb", {}); consumers = data.get("consumers")
    require(IMAGE_RE.match(str(ydb.get("image", ""))), "YDB image must be digest-pinned")
    require(ydb.get("endpoint") and str(ydb.get("database", "")).startswith("/"),
            "YDB endpoint and absolute database are required")
    require(isinstance(consumers, list) and consumers, "registry has no consumers")
    classifications = load_classifications(data, consumers)
    seen = set()
    for item in consumers:
        consumer = item.get("id") if isinstance(item, dict) else None; adapter = path.parent / str(consumer)
        require(re.match(r"^[a-z][a-z0-9_-]*$", str(consumer or "")),
                f"invalid consumer id: {consumer}")
        require(consumer not in seen, f"duplicate consumer id: {consumer}"); seen.add(consumer)
        require(item.get("kind") in {"package", "smoke", "framework"}, f"{consumer}: invalid kind")
        require(item.get("tier") in {"smoke", "core", "expansion"}, f"{consumer}: invalid tier")
        for field in ("display_name", "language"): require(item.get(field), f"{consumer}: {field} is required")
        require(isinstance(item.get("runtime_version"), list)
                and all(isinstance(value, str) for value in item["runtime_version"]),
                f"{consumer}: runtime_version must be a command list")
        require(IMAGE_RE.match(str(item.get("runtime_image", ""))),
                f"{consumer}: runtime image must be digest-pinned")
        require((path.parent / item.get("dockerfile", f"{consumer}/Dockerfile")).is_file(),
                f"{consumer}: missing Dockerfile")
        runner, tests = adapter / "run-tests", item.get("tests", [])
        require(item["kind"] == "package" or tests or runner.is_file(),
                f"{consumer}: tests or run-tests are required")
        require(not runner.exists() or os.access(runner, os.X_OK), f"{consumer}: run-tests is not executable")
        for test in tests:
            require(isinstance(test, dict) and test.get("id") and test.get("name")
                    and isinstance(test.get("command"), list) and test["command"],
                    f"{consumer}: invalid declarative test")
            if test.get("output_regex"): re.compile(test["output_regex"])
        expected = item.get("expected", {})
        require(isinstance(expected, dict), f"{consumer}: expected results must be a mapping")
        require(not ({"unsupported", "skipped"} & set(expected)),
                f"{consumer}: classifications must be declared in the top-level classification list")
        expected.update(classifications[consumer])
        discovered = expected.get("discovered", False)
        required = expected.get("required", [])
        unsupported = expected.get("unsupported", {})
        skipped = expected.get("skipped", {})
        require(isinstance(discovered, bool), f"{consumer}: discovered must be boolean")
        require(isinstance(required, list) and (required or discovered),
                f"{consumer}: required tests are empty")
        require(all(isinstance(pattern, str) and pattern for pattern in required)
                and len(required) == len(set(required)),
                f"{consumer}: required tests contain invalid or duplicate patterns")
        require(isinstance(unsupported, dict), f"{consumer}: unsupported tests must be a mapping")
        require(isinstance(skipped, dict), f"{consumer}: skipped tests must be a mapping")
        require(all(isinstance(pattern, str) and pattern and isinstance(rule, dict)
                    and rule.get("reason") and rule.get("match")
                    for patterns in (unsupported, skipped) for pattern, rule in patterns.items()),
                f"{consumer}: expected-result patterns and reasons must be non-empty strings")
        require(not (set(unsupported) & set(skipped)),
                f"{consumer}: duplicate or unexplained expectations")
        broad = [pattern for patterns in (unsupported, skipped) for pattern in patterns
                 if pattern.endswith(".*")]
        require(not discovered or not broad,
                f"{consumer}: suite-wide expected-result patterns are forbidden: {', '.join(broad)}")
        require(not tests or {test["id"] for test in tests} == set(required),
                f"{consumer}: declarative tests differ from required tests")
        modes = item.get("modes", [])
        require(item["kind"] == "package" or (modes and set(modes) <= {"dsn", "connection_string"}),
                f"{consumer}: invalid connection modes")
        source = item.get("source")
        if item["kind"] == "framework":
            require(isinstance(source, dict) and source.get("kind") == "archive",
                    f"{consumer}: upstream source must be an archive")
            require(source.get("url") and source.get("revision")
                    and SOURCE_RE.match(str(source.get("sha256", ""))),
                    f"{consumer}: invalid pinned source")
            require(read_yaml(adapter / "upstream.lock") == source, f"{consumer}: upstream.lock differs from registry")
        else:
            require(source == "system", f"{consumer}: smoke/package source must be system")
    return data
def consumer_runs(data):
    for consumer in data["consumers"]:
        modes = consumer.get("modes") or ["all"]
        for mode in modes:
            run_id = consumer["id"] if mode == "all" else f"{consumer['id']}-{mode}"
            yield {"consumer": consumer["id"], "mode": mode, "run_id": run_id,
                   "display_name": consumer["display_name"],
                   "runtime_image": consumer["runtime_image"]}
def command(args, *, env=None, capture=False, user=None, check=True, input_text=None, timeout=None):
    options = {"env": env, "text": True, "check": check}
    if input_text is not None: options["input"] = input_text
    if capture: options.update(stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if user:
        account = pwd.getpwnam(user)
        def demote():
            os.setgroups([]); os.setgid(account.pw_gid); os.setuid(account.pw_uid)
        options["preexec_fn"] = demote
    return subprocess.run([str(value) for value in args], timeout=timeout, **options)
def append_result(output, test_id, name, status, start, stop, message="", log=None):
    document = json.loads(output.read_text()) if output.exists() else {"schema_version": 1, "tests": []}
    result = {"id": test_id, "name": name, "status": status, "start": start, "stop": stop}
    if message: result["message"] = message
    if log: result["attachments"] = [{"name": log.name, "path": log.name, "type": "text/plain"}]
    output.parent.mkdir(parents=True, exist_ok=True); document["tests"].append(result)
    output.write_text(json.dumps(document, indent=2, sort_keys=True) + "\n", encoding="utf-8")
def infrastructure_test_id(consumer_id, mode):
    return f"{consumer_id}.{mode}.infrastructure" if mode != "all" else f"{consumer_id}.infrastructure"
def logged(log, args, **options):
    result = command(args, capture=True, check=False, **options)
    log.append(f"$ {' '.join(map(str, args))}\n{result.stdout or ''}")
    require(result.returncode == 0, f"{args[0]} exited with status {result.returncode}")
    return result.stdout or ""
def record_case(native, test_id, name, action):
    start, lines, status, message = int(time.time() * 1000), [], "passed", ""
    try:
        action(lines)
    except Exception as error:
        status, message = "failed", str(error); lines.append(f"\n{type(error).__name__}: {error}\n")
    stop, log = int(time.time() * 1000), native / f"{test_id.replace('.', '-')}.log"
    log.write_text("".join(lines), encoding="utf-8")
    append_result(native / "results.json", test_id, name, status, start, stop, message, log)
    return status != "passed"
def safe_extract(archive, destination):
    destination.mkdir(parents=True); root = destination.resolve()
    def check(name):
        try:
            (destination / name).resolve().relative_to(root)
        except ValueError as error:
            raise HarnessError(f"unsafe upstream path: {name}") from error
    require(tarfile.is_tarfile(archive), "upstream source is not a tar archive")
    with tarfile.open(archive) as stream:
        for member in stream.getmembers():
            check(member.name); require(not member.islnk(), f"upstream hard link: {member.name}")
            if member.issym(): check(str(Path(member.name).parent / member.linkname))
        stream.extractall(destination)
    for path in sorted(destination.rglob("*"), reverse=True):
        if not path.is_symlink():
            path.chmod(0o555 if path.is_dir() or path.stat().st_mode & 0o111 else 0o444)
    destination.chmod(0o555)
def prepare_source(consumer):
    source = consumer["source"]
    if source == "system": return "system", "system", Path("/nonexistent-upstream")
    archive = WORK / "upstream.archive"; urllib.request.urlretrieve(source["url"], archive)
    actual = hashlib.sha256(archive.read_bytes()).hexdigest()
    require(actual == source["sha256"], f"upstream checksum mismatch: {actual}")
    upstream = WORK / "upstream"; safe_extract(archive, upstream)
    return source["revision"], source["sha256"], upstream
def inspect_package(package):
    field = lambda name: command(["dpkg-deb", "-f", package, name], capture=True).stdout.strip()
    require(field("Package") == "ydb-odbc", "artifact is not the ydb-odbc package")
    return field("Version"), hashlib.sha256(package.read_bytes()).hexdigest()
def install_package(package):
    command(["apt-get", "update"])
    command(["apt-get", "install", "-y", "--no-install-recommends", package])
    files = command(["dpkg-query", "-L", "ydb-odbc"], capture=True).stdout.splitlines()
    drivers = [Path(value).resolve() for value in files if value.endswith("/libydb-odbc.so")]
    require(len(drivers) == 1 and drivers[0].is_file(), f"expected one installed driver: {drivers}")
    driver = drivers[0]; owner = command(["dpkg-query", "-S", driver], capture=True).stdout.strip()
    require(owner.startswith("ydb-odbc:"), f"driver has wrong package owner: {owner}")
    registration = command(["odbcinst", "-q", "-d", "-n", "YDB"], capture=True).stdout
    require(f"Driver={driver}" in registration and f"Setup={driver}" in registration,
            "YDB driver registration is invalid")
    return str(driver)
def write_dsn(consumer_id, endpoint, database):
    dsn = f"YDB_{consumer_id}"
    Path("/etc/odbc.ini").write_text(f"[ODBC Data Sources]\n{dsn}=YDB ODBC Driver\n\n[{dsn}]\nDriver=YDB\n"
        f"Server={endpoint}\nDatabase={database}\nAuthMode=Anonymous\n")
    return dsn
def package_suite(package, native):
    multiarch = command(["dpkg-architecture", "-qDEB_HOST_MULTIARCH"], capture=True).stdout.splitlines()[-1]
    driver, template = Path(f"/usr/lib/{multiarch}/libydb-odbc.so"), Path("/usr/share/ydb-odbc/odbcinst.ini")
    old, unpacked, state = Path("/tmp/ydb-odbc-old.deb"), Path("/tmp/ydb-odbc-old"), {}
    def field(log, name): return logged(log, ["dpkg-deb", "-f", package, name]).strip()
    def registration(log):
        value = logged(log, ["odbcinst", "-q", "-d", "-n", "YDB"])
        for expected in (f"Driver={driver}", f"Setup={driver}", "UsageCount=1"): require(expected in value.splitlines(), f"missing registration: {expected}")
    def unchanged():
        require(all(hashlib.sha256(path.read_bytes()).hexdigest() == digest
                    for path, digest in state["configs"].items()), "ODBC configuration changed")
    def clean(log):
        logged(log, ["apt-get", "update"]); require(field(log, "Package") == "ydb-odbc", "wrong package name")
        require(field(log, "Architecture") == command(["dpkg", "--print-architecture"], capture=True).stdout.strip(),
                "wrong package architecture")
        depends = field(log, "Depends")
        for dependency in ("odbcinst", "libodbcinst2", "libc6"): require(re.search(fr"(^|, ){dependency}([ (]|,|$)", depends), f"missing dependency: {dependency}")
        unrelated = Path("/tmp/unrelated.ini")
        unrelated.write_text(f"[UnrelatedPackageTest]\nDriver=/usr/lib/{multiarch}/libodbc.so.2\nSetup=/usr/lib/{multiarch}/libodbcinst.so.2\n")
        logged(log, ["odbcinst", "-i", "-d", "-f", unrelated])
        configs = {Path("/etc/odbc.ini"): "[UnrelatedSystemDsn]\nDriver=UnrelatedPackageTest\n",
                   Path("/root/.odbc.ini"): "[UnrelatedUserDsn]\nDriver=UnrelatedPackageTest\n"}
        for path, content in configs.items(): path.write_text(content)
        state["configs"] = {path: hashlib.sha256(path.read_bytes()).hexdigest() for path in configs}
        shutil.rmtree(unpacked, ignore_errors=True); logged(log, ["dpkg-deb", "--raw-extract", package, unpacked])
        control, version = unpacked / "DEBIAN/control", field(log, "Version")
        control.write_text(re.sub(r"^Version: .*$", f"Version: {version}~integration1",
                                  control.read_text(), flags=re.MULTILINE))
        logged(log, ["dpkg-deb", "--build", unpacked, old]); logged(log, ["apt-get", "install", "-y", old])
        require(driver.is_file() and template.is_file(), "installed files are missing")
        require(f"Driver={driver}" in template.read_text().splitlines(), "invalid registration template")
        require(logged(log, ["dpkg-query", "-S", driver]).startswith("ydb-odbc:"), "wrong file owner"); registration(log)
    def upgrade(log):
        logged(log, ["apt-get", "install", "-y", package])
        require(logged(log, ["dpkg-query", "-W", "-f=${Version}", "ydb-odbc"]).strip()
                == field(log, "Version"), "upgrade version differs")
        registration(log); unchanged()
    def remove(log):
        logged(log, ["apt-get", "remove", "-y", "ydb-odbc"])
        require(not driver.exists() and not template.exists(), "package files remain")
        result = command(["odbcinst", "-q", "-d", "-n", "YDB"], capture=True, check=False)
        log.append(result.stdout or ""); require(result.returncode != 0, "YDB registration remains")
        logged(log, ["odbcinst", "-q", "-d", "-n", "UnrelatedPackageTest"]); unchanged()
    def replacement(log):
        logged(log, ["apt-get", "install", "-y", package]); registration(log)
        logged(log, ["odbcinst", "-u", "-d", "-n", "YDB"])
        replacement_file = Path("/tmp/replacement.ini"); replacement_file.write_text(f"[YDB]\nDriver=/usr/lib/{multiarch}/libodbc.so.2\nSetup=/usr/lib/{multiarch}/libodbcinst.so.2\n")
        logged(log, ["odbcinst", "-i", "-d", "-f", replacement_file])
        expected = logged(log, ["odbcinst", "-q", "-d", "-n", "YDB"]); logged(log, ["apt-get", "remove", "-y", "ydb-odbc"])
        require(logged(log, ["odbcinst", "-q", "-d", "-n", "YDB"]) == expected,
                "replacement registration changed"); unchanged()
    cases = [("package.clean_install", "Install and register ydb-odbc", clean),
             ("package.upgrade", "Upgrade without duplicate registration", upgrade),
             ("package.remove", "Remove without changing unrelated configuration", remove),
             ("package.replacement", "Preserve a replacement YDB registration", replacement)]
    return int(any([record_case(native, *case) for case in cases]))
def declarative_suite(consumer, env, native):
    values = {"dsn": env["YDB_ODBC_DSN"], "connection_string": env["YDB_ODBC_CONNECTION_STRING"],
              "endpoint": env["YDB_ENDPOINT"], "database": env["YDB_DATABASE"],
              "upstream": env["ODBC_UPSTREAM_DIR"]}
    failures = []
    for test in consumer["tests"]:
        args = [value.format(**values) for value in test["command"]]
        def action(log, test=test, args=args):
            output = logged(log, args, env=env, user="nobody", input_text=test.get("stdin"))
            require(not test.get("output_regex") or re.search(test["output_regex"], output, re.MULTILINE),
                    "output did not match expected pattern")
        failures.append(record_case(native, test["id"], test["name"], action))
    return int(any(failures))
def launch_consumer(consumer, mode, runtime, package, results):
    package, adapter = package.resolve(), ROOT / consumer
    item = next(item for item in load_registry()["consumers"] if item["id"] == consumer)
    dockerfile = ROOT / item.get("dockerfile", f"{consumer}/Dockerfile")
    require(runtime == item["runtime_image"], "runtime image differs from registry")
    require(package.is_file() and dockerfile.is_file(), "package or adapter is missing")
    results.mkdir(parents=True, exist_ok=True); results = results.resolve(); results.chmod(0o777)
    image_name = f"ydb-odbc-{consumer}:{os.getenv('GITHUB_RUN_ID', 'local')}-{os.getenv('GITHUB_RUN_ATTEMPT', '1')}"
    command(["docker", "build", "--build-arg", f"RUNTIME_IMAGE={runtime}",
             "-f", dockerfile, "-t", image_name, ROOT], timeout=1800)
    args = ["docker", "run", "--rm", "--network", "host", "--user", "0:0",
            "-v", f"{ROOT}:/harness:ro", "-v", f"{package.parent}:/packages:ro",
            "-v", f"{results}:/results", "-e", f"ODBC_PACKAGE_BASENAME={package.name}",
            "-e", f"ODBC_RUNTIME_IMAGE={runtime}", "-e", f"ODBC_TEST_MODE={mode}"]
    for name in ("DRIVER_COMMIT", "YDB_TEST_IMAGE", "YDB_ENDPOINT", "YDB_DATABASE"):
        require(os.getenv(name), f"{name} is required")
        args += ["-e", f"{name}={os.environ[name]}"]
    return command(args + [image_name, "python3", "/harness/harness.py", "run", "--consumer", consumer,
                           "--mode", mode],
                   check=False).returncode
def convert_allure(native, output, metadata):
    tests = json.loads(native.read_text())["tests"]; output.mkdir(exist_ok=True)
    for old in output.glob("*-result.json"): old.unlink()
    attachment_files = {}
    for test in tests:
        result_id = str(uuid.uuid5(uuid.NAMESPACE_URL, f"ydb-odbc:{metadata['consumer']}:{test['id']}"))
        attachments = []
        for index, attachment in enumerate(test.get("attachments", [])):
            source = native.parent / attachment["path"]; require(source.is_file(), f"missing attachment for {test['id']}: {source}")
            target = attachment_files.get(source)
            if target is None:
                attachment_id = uuid.uuid5(uuid.NAMESPACE_URL, f"ydb-odbc:{metadata['run_id']}:{attachment['path']}")
                target = f"{attachment_id}-attachment{source.suffix or '.txt'}"
                shutil.copyfile(source, output / target); attachment_files[source] = target
            attachments.append({"name": attachment.get("name", source.name),
                                "source": target, "type": attachment.get("type", "text/plain")})
        result = {
            "uuid": result_id, "historyId": f"{metadata['consumer']}::{test['id']}",
            "testCaseId": test["id"], "name": test.get("name", test["id"]),
            "fullName": f"{metadata['consumer']}.{test['id']}", "status": test["status"],
            "stage": "finished", "start": test["start"], "stop": test["stop"],
            "labels": [{"name": key, "value": str(metadata[value])} for key, value in
                       (("suite", "consumer"), ("language", "language"), ("tier", "tier"), ("driverCommit", "driver_commit"),
                        ("runtimeVersion", "runtime_version"), ("ydbImage", "ydb_image"),
                        ("connectionMode", "connection_mode"))] + ([{"name": "originalStatus", "value": test["status"]}] if test.get("expected") else []),
            "parameters": [{"name": key, "value": str(metadata[value])} for key, value in
                           (("endpoint", "endpoint"), ("database", "database"),
                            ("packageSha256", "package_sha256"),
                            ("upstreamRevision", "upstream_revision"),
                            ("upstreamSha256", "upstream_sha256"))],
            "attachments": attachments,
        }
        if test.get("expected"):
            result["labels"].append({"name": "expectedOutcome", "value": test["expected"]})
        if test.get("message") or test.get("trace"): result["statusDetails"] = {key: test[key] for key in ("message", "trace") if test.get(key)} | ({"known": True} if test.get("expected") else {})
        if test.get("expected"):
            result.setdefault("statusDetails", {})["message"] = (
                f"Expected {test['expected']}: {test['expected_reason']}\nReported {test['status'].upper()}: "
                + test.get("message", ""))
        (output / f"{result_id}-result.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    properties = {key: metadata[key] for key in
                  ("consumer", "connection_mode", "run_id", "runtime_version", "driver_commit", "package_sha256",
                   "ydb_image", "endpoint", "database", "upstream_revision", "upstream_sha256")}
    (output / "environment.properties").write_text("".join(f"{key}={value}\n" for key, value in sorted(properties.items())))
def matching_ids(actual, pattern):
    return {test_id for test_id, test in actual.items()
            if glob_ids({test_id, test.get("parent_id", test_id)}, pattern)}

def expected_result(test, expectations):
    matches = [(outcome, rule) for outcome in ("unsupported", "skipped")
               for pattern, rule in expectations.get(outcome, {}).items()
               if glob_ids({test["id"], test.get("parent_id", test["id"])}, pattern)]
    if len(matches) != 1:
        return None
    outcome, rule = matches[0]
    # Setup errors, crashes and missing results can never be expected.
    status = "skipped" if outcome == "skipped" else "failed"
    if test.get("status") != status or test.get("phase", "test") != "test":
        return None
    problems = test.get("problems") or [{"message": test.get("message", ""),
                                        "trace": test.get("trace", "")}]
    if not all(problem.get("kind", status) in ({"failure", "failed"} if status == "failed" else {"skipped"})
               and re.search(rule["match"], "\n".join(str(problem.get(key, ""))
                       for key in ("message", "trace", "location"))) for problem in problems):
        return None
    return outcome, rule["reason"]

def validate_classified_results(actual, expectations):
    errors = []
    patterns = {"required": expectations.get("required", []),
                "unsupported": expectations.get("unsupported", {}),
                "skipped": expectations.get("skipped", {})}
    matches = {outcome: {pattern: matching_ids(actual, pattern) for pattern in values}
               for outcome, values in patterns.items()}
    for outcome, classified in matches.items():
        for pattern, test_ids in classified.items():
            if not test_ids and (expectations.get("discovered") or outcome != "required"):
                errors.append(f"{outcome} expectation pattern matched no tests: {pattern}")
    for test_id, test in actual.items():
        exceptions = [(outcome, pattern) for outcome in ("unsupported", "skipped")
                      for pattern, ids in matches[outcome].items() if test_id in ids]
        if len(exceptions) > 1:
            errors.append(f"{test_id}: ambiguous expected result: {exceptions}")
        elif exceptions:
            outcome = exceptions[0][0]
            if not expected_result(test, expectations):
                errors.append(f"{test_id}: unexpected status or diagnostic for {outcome}: "
                              f"{test.get('status')}: {test.get('message', '')}")
        elif not any(test_id in ids for ids in matches["required"].values()):
            errors.append(f"{test_id}: missing result expectation")
        elif test.get("status") != "passed":
            errors.append(f"{test_id}: unexpected status {test.get('status')}: {test.get('message', '')}")
    return errors

def annotate_expected_results(consumer, native):
    document = json.loads(native.read_text())
    for test in document.get("tests", []):
        test.pop("expected", None); test.pop("expected_reason", None)
        match = expected_result(test, consumer["expected"])
        if match:
            test["expected"], test["expected_reason"] = match
    # The original message, trace and status are never replaced.
    native.write_text(json.dumps(document, indent=2, sort_keys=True) + "\n", encoding="utf-8")

def validate_results(consumer, native, allure, test_rc=0):
    errors, tests, document = [], [], {}
    try:
        document = json.loads(native.read_text())
        tests = document["tests"]
        require(isinstance(tests, list) and tests, "test suite is empty")
    except (OSError, KeyError, json.JSONDecodeError, HarnessError) as error:
        errors = [f"invalid results: {error}"]
    actual = {}
    for test in tests:
        test_id = test.get("id") if isinstance(test, dict) else None
        if not test_id or test_id in actual: errors.append(f"missing or duplicate test id: {test_id}")
        else: actual[test_id] = test
    expectations = consumer["expected"]; required = set(expectations["required"])
    if expectations.get("discovered") and not any(key.endswith(".infrastructure") for key in actual):
        inventory = document.get("inventory", [])
        if not inventory or len(inventory) != len(set(inventory)):
            errors.append("missing or duplicate discovered inventory")
        reported = {test.get("parent_id", test_id) for test_id, test in actual.items()} if document.get("inventory_kind") == "parents" else set(actual)
        if set(inventory) != reported:
            errors.append(f"inventory/result mismatch: missing={sorted(set(inventory) - reported)}, "
                          f"unexpected={sorted(reported - set(inventory))}")
        if document.get("inventory_kind") == "parents":
            rows = document.get("inventory_rows", {})
            if set(rows) != set(inventory): errors.append("missing data-row inventory")
            counts = Counter(test.get("parent_id", test_id) for test_id, test in actual.items())
            for parent, count in rows.items():
                if counts[parent] != count:
                    errors.append(f"{parent}: expected {count} data rows, reported {counts[parent]}")
    infrastructure = [test for test_id, test in actual.items() if test_id.endswith(".infrastructure")]
    if expectations.get("discovered") and infrastructure:
        for test in infrastructure:
            errors.append(f"{test['id']}: {test.get('message', 'infrastructure failure')}")
    else:
        errors += validate_classified_results(actual, expectations)
    if not expectations.get("discovered"):
        for test_id in sorted(required - set(actual)): errors.append(f"missing test: {test_id}")
        for test_id in sorted(set(actual) - required): errors.append(f"unexpected test: {test_id}")
    if test_rc < 0 or test_rc > 1: errors.append(f"test command exited abnormally: {test_rc}")
    if test_rc and tests and all(test.get("status") == "passed" for test in tests
                                 if isinstance(test, dict)):
        errors.append(f"test command exited {test_rc} although every reported test passed")
    if len(list(allure.glob("*-result.json"))) != len(tests): errors.append("Allure/native result count differs")
    validation = {"consumer": consumer["id"], "connection_mode": os.environ.get("ODBC_TEST_MODE", "all"),
                  "ok": not errors, "test_count": len(tests), "errors": errors,
                  "counts": {name: sum((expected_result(test, expectations) or
                                               ("unexpected_skip" if test.get("status") == "skipped" else test.get("status"),))[0] == name
                                       for test in actual.values())
                             for name in ("passed", "unsupported", "skipped", "unexpected_skip", "failed", "broken")}}

    (RESULTS / "validation.json").write_text(json.dumps(validation, indent=2, sort_keys=True) + "\n")
    for error in errors: print(f"result error: {error}", file=sys.stderr)
    return errors
def run_consumer(consumer_id, mode):
    os.environ["ODBC_TEST_MODE"] = mode
    data = load_registry(); consumer = next((x for x in data["consumers"] if x["id"] == consumer_id), None)
    require(consumer, f"unknown consumer: {consumer_id}")
    allowed_modes = consumer.get("modes") or ["all"]
    require(mode in allowed_modes, f"{consumer_id}: invalid connection mode: {mode}")
    adapter, package = ROOT / consumer_id, Path("/packages") / os.environ["ODBC_PACKAGE_BASENAME"]
    native, allure, home = RESULTS / "native", RESULTS / "allure", WORK / "home"
    for path in (native, allure):
        if path.exists(): shutil.rmtree(path)
    for path in (RESULTS / "metadata.json", RESULTS / "validation.json"):
        path.unlink(missing_ok=True)
    for path in (RESULTS, native, allure, WORK, home):
        path.mkdir(parents=True, exist_ok=True); path.chmod(0o777)
    version, package_hash = inspect_package(package); endpoint, database = os.environ["YDB_ENDPOINT"], os.environ["YDB_DATABASE"]
    metadata = {
        "consumer": consumer_id, "display_name": consumer["display_name"],
        "connection_mode": mode, "run_id": consumer_id if mode == "all" else f"{consumer_id}-{mode}",
        "kind": consumer["kind"], "language": consumer["language"], "tier": consumer["tier"],
        "runtime_image": os.environ["ODBC_RUNTIME_IMAGE"], "runtime_version": "unknown",
        "driver_commit": os.environ["DRIVER_COMMIT"], "package_sha256": package_hash,
        "package_version": version, "ydb_image": os.environ["YDB_TEST_IMAGE"],
        "endpoint": endpoint, "database": database,
        "runtime_packages_sha256": hashlib.sha256(Path("/runtime-packages.txt").read_bytes()).hexdigest() if Path("/runtime-packages.txt").exists() else "unknown",
        "source_kind": "system" if consumer["source"] == "system" else "archive",
        "upstream_revision": "unknown", "upstream_sha256": "unknown",
        "test_sources_sha256": hashlib.sha256((adapter / "test-sources.lock").read_bytes()).hexdigest() if (adapter / "test-sources.lock").exists() else "none",
        "fixture_sha256": hashlib.sha256(b"".join(path.read_bytes() for path in sorted(adapter.glob("*.patch")))).hexdigest(),
    }
    test_rc, error = 0, None
    try:
        if consumer["kind"] != "package":
            metadata["driver_path"] = install_package(package)
        revision, source_hash, upstream = prepare_source(consumer)
        metadata.update(upstream_revision=revision, upstream_sha256=source_hash)
        dsn = write_dsn(consumer_id, endpoint, database) if consumer["kind"] != "package" else ""
        env = os.environ.copy(); env.pop("LD_LIBRARY_PATH", None)
        env.update({
            "HOME": str(home), "ODBCINI": "/etc/odbc.ini", "YDB_ODBC_DSN": dsn,
            "YDB_ODBC_CONNECTION_STRING":
                f"Driver={{YDB}};Server={endpoint};Database={database};AuthMode=Anonymous",
            "ODBC_NATIVE_RESULTS_DIR": str(native), "ODBC_UPSTREAM_DIR": str(upstream),
            "ODBC_ADAPTER_DIR": str(adapter), "ODBC_HARNESS_DIR": str(ROOT),
            "YDB_ENDPOINT": endpoint, "YDB_DATABASE": database,
            "ODBC_TEST_MODE": mode,
        })
        run_user = None if consumer["kind"] == "package" else "nobody"
        runtime_output = command(consumer["runtime_version"], env=env, capture=True, user=run_user).stdout.strip()
        metadata["runtime_version"] = runtime_output.splitlines()[0]
        (RESULTS / "metadata.json").write_text(json.dumps(metadata, indent=2, sort_keys=True) + "\n")
        if consumer.get("runner") == "package-contract":
            test_rc = package_suite(package, native)
        elif consumer.get("tests"):
            test_rc = declarative_suite(consumer, env, native)
        else:
            test_rc = command([adapter / "run-tests"], env=env, user=run_user, check=False, timeout=1800).returncode
    except Exception as exception:  # preserve infrastructure failures as test output
        error = str(exception)
    (RESULTS / "metadata.json").write_text(json.dumps(metadata, indent=2, sort_keys=True) + "\n")
    results_file = native / "results.json"
    if error or not results_file.exists():
        now = int(time.time() * 1000)
        append_result(results_file, infrastructure_test_id(consumer_id, mode), f"{consumer_id} test infrastructure",
                      "broken", now, now, error or f"test command exited {test_rc} without results")
    try:
        annotate_expected_results(consumer, results_file)
        convert_allure(results_file, allure, metadata)
        validation_errors = validate_results(consumer, results_file, allure, test_rc)
    except Exception as exception:
        print(f"result finalization failed: {exception}", file=sys.stderr); return 1
    return int(bool(error or validation_errors))
def aggregate(source, output):
    expected = {item["run_id"] for item in consumer_runs(load_registry())}
    native_out, allure_out = output / "native", output / "allure-results"
    native_out.mkdir(parents=True, exist_ok=True); allure_out.mkdir(exist_ok=True)
    found, rows, errors, provenance = set(), [], [], None
    for metadata_path in source.rglob("metadata.json"):
        root = metadata_path.parent
        try:
            metadata = json.loads(metadata_path.read_text())
            validation = json.loads((root / "validation.json").read_text())
            run_id = metadata["run_id"]
            require(run_id in expected, f"unexpected result artifact: {run_id}")
            require(run_id not in found, f"duplicate artifact: {run_id}"); found.add(run_id)
            results = json.loads((root / "native/results.json").read_text())["tests"]
            require(len(results) == validation["test_count"], f"{run_id}: native/validation count differs")
            require(len(list((root / "allure").glob("*-result.json"))) == len(results),
                    f"{run_id}: Allure/native count differs")
            identity = tuple(metadata[key] for key in ("driver_commit", "package_sha256", "ydb_image"))
            require(all(identity), f"{run_id}: incomplete provenance")
            if provenance is None: provenance = identity
            require(identity == provenance, f"{run_id}: mismatched artifact provenance")
            require(not os.getenv("DRIVER_COMMIT") or identity[0] == os.environ["DRIVER_COMMIT"],
                    f"{run_id}: wrong driver commit")
            require(not os.getenv("ODBC_PACKAGE_SHA256") or identity[1] == os.environ["ODBC_PACKAGE_SHA256"],
                    f"{run_id}: wrong package checksum")
            shutil.copytree(root / "native", native_out / run_id, dirs_exist_ok=True)
            for path in (metadata_path, root / "validation.json"): shutil.copy2(path, native_out / run_id / path.name)
            for artifact in (root / "allure").iterdir():
                if artifact.name == "environment.properties": continue
                target = allure_out / artifact.name
                require(not target.exists(), f"duplicate Allure artifact: {artifact.name}")
                shutil.copy2(artifact, target)
            for path in (root / "allure").glob("*-result.json"):
                for attachment in json.loads(path.read_text()).get("attachments", []):
                    require((allure_out / attachment["source"]).is_file(), f"missing Allure attachment: {attachment['source']}")
            counts = validation.get("counts", {})
            rows.append((run_id, validation["test_count"], counts, "passed" if validation.get("ok") else "FAILED"))
            if not validation.get("ok"):
                errors.append(f"{run_id}: unexpected test outcomes; see native/{run_id}/validation.json")
        except (OSError, KeyError, json.JSONDecodeError, HarnessError) as error:
            errors.append(f"invalid artifact at {root}: {error}")
    if expected - found: errors.append(f"missing result artifacts: {', '.join(sorted(expected - found))}")
    lines = ["## ODBC integration results", "", "| Consumer | Tests | Passed | Unsupported | Expected skips | Unexpected | Status |",
             "| --- | ---: | ---: | ---: | ---: | ---: | --- |"]
    lines += [f"| {name} | {count} | {counts.get('passed', 0)} | {counts.get('unsupported', 0)} | "
              f"{counts.get('skipped', 0)} | {sum(counts.get(key, 0) for key in ('failed','broken','unexpected_skip'))} | {status} |" for name, count, counts, status in sorted(rows)]
    if errors: lines += ["", "### Validation failures", ""] + [f"- {error}" for error in errors]
    summary = "\n".join(lines) + "\n"; (output / "summary.md").write_text(summary); print(summary)
    return int(bool(errors))
def main():
    parser = argparse.ArgumentParser(description="YDB ODBC integration harness")
    commands = parser.add_subparsers(dest="command", required=True)
    registry = commands.add_parser("registry"); registry.add_argument("--registry", type=Path, default=REGISTRY)
    registry.add_argument("--matrix", action="store_true"); registry.add_argument("--config", action="store_true")
    run = commands.add_parser("run"); run.add_argument("--consumer", required=True)
    run.add_argument("--mode", default="all")
    launch = commands.add_parser("launch"); launch.add_argument("--consumer", required=True)
    launch.add_argument("--mode", default="all")
    launch.add_argument("--runtime-image", required=True); launch.add_argument("--package", type=Path, required=True)
    launch.add_argument("--results", type=Path, required=True)
    aggregation = commands.add_parser("aggregate"); aggregation.add_argument("--input", type=Path, required=True)
    aggregation.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        if args.command == "registry":
            data = load_registry(args.registry.resolve())
            if args.matrix:
                print(json.dumps({"include": [item for item in consumer_runs(data)
                    if item["mode"] != "all"]}, separators=(",", ":")))
            elif args.config:
                package = next(item for item in data["consumers"] if item["kind"] == "package")
                print(json.dumps({"ydb": data["ydb"], "package_runtime_image": package["runtime_image"]},
                                 separators=(",", ":")))
            else: print(f"Validated {len(data['consumers'])} ODBC consumers")
            return 0
        if args.command == "run": return run_consumer(args.consumer, args.mode)
        if args.command == "launch": return launch_consumer(args.consumer, args.mode, args.runtime_image, args.package, args.results)
        return aggregate(args.input, args.output)
    except HarnessError as error:
        print(f"harness error: {error}", file=sys.stderr)
        return 1
if __name__ == "__main__":
    raise SystemExit(main())
