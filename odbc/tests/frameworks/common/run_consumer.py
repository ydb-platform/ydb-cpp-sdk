#!/usr/bin/env python3

import hashlib
import json
import os
import pwd
import stat
import subprocess
import sys
import tarfile
import time
import urllib.request
import zipfile
from pathlib import Path

sys.path.insert(0, "/harness/frameworks")
from validate_registry import validate_registry  # noqa: E402


HARNESS_ROOT = Path("/harness")
FRAMEWORKS_ROOT = HARNESS_ROOT / "frameworks"
REPORTING_ROOT = HARNESS_ROOT / "reporting"
RESULTS_ROOT = Path("/results")
WORK_ROOT = Path("/work")


def run(command, *, env=None, check=True, capture=False, user=None):
    kwargs = {
        "env": env,
        "check": check,
        "text": True,
    }
    if capture:
        kwargs["stdout"] = subprocess.PIPE
        kwargs["stderr"] = subprocess.STDOUT
    if user is not None:
        account = pwd.getpwnam(user)

        def demote():
            os.setgroups([])
            os.setgid(account.pw_gid)
            os.setuid(account.pw_uid)

        kwargs["preexec_fn"] = demote
    return subprocess.run([str(item) for item in command], **kwargs)


def safe_extract(archive: Path, destination: Path):
    destination.mkdir(parents=True, exist_ok=True)
    destination_real = destination.resolve()

    def ensure_safe(name: str):
        target = (destination / name).resolve()
        try:
            target.relative_to(destination_real)
        except ValueError as error:
            raise RuntimeError(f"upstream archive contains unsafe path: {name}") from error

    if tarfile.is_tarfile(archive):
        with tarfile.open(archive) as stream:
            for member in stream.getmembers():
                ensure_safe(member.name)
                if member.issym() or member.islnk():
                    raise RuntimeError(f"upstream archive contains a link: {member.name}")
            stream.extractall(destination)
    elif zipfile.is_zipfile(archive):
        with zipfile.ZipFile(archive) as stream:
            for member in stream.infolist():
                ensure_safe(member.filename)
                if stat.S_ISLNK(member.external_attr >> 16):
                    raise RuntimeError(f"upstream archive contains a link: {member.filename}")
            stream.extractall(destination)
    else:
        raise RuntimeError("upstream source is neither a tar nor zip archive")


def prepare_upstream(consumer):
    source = consumer["source"]
    if source["kind"] == "system":
        return "system", "system", Path("/nonexistent-upstream")

    archive = WORK_ROOT / "upstream.archive"
    urllib.request.urlretrieve(source["url"], archive)
    actual = hashlib.sha256(archive.read_bytes()).hexdigest()
    if actual != source["sha256"]:
        raise RuntimeError(f"upstream checksum mismatch: expected {source['sha256']}, got {actual}")
    upstream = WORK_ROOT / "upstream"
    safe_extract(archive, upstream)
    for path in sorted(upstream.rglob("*"), reverse=True):
        path.chmod(0o555 if path.is_dir() else 0o444)
    upstream.chmod(0o555)
    return source["revision"], source["sha256"], upstream


def verify_package(package: Path):
    package_name = run(["dpkg-deb", "-f", package, "Package"], capture=True).stdout.strip()
    if package_name != "ydb-odbc":
        raise RuntimeError(f"expected ydb-odbc package, got {package_name}")
    package_version = run(["dpkg-deb", "-f", package, "Version"], capture=True).stdout.strip()
    package_sha256 = hashlib.sha256(package.read_bytes()).hexdigest()

    run(["apt-get", "update"])
    run(["apt-get", "install", "-y", "--no-install-recommends", package])

    installed_files = run(["dpkg-query", "-L", "ydb-odbc"], capture=True).stdout.splitlines()
    driver_paths = [Path(line) for line in installed_files if line.endswith("/libydb-odbc.so")]
    if len(driver_paths) != 1 or not driver_paths[0].is_file():
        raise RuntimeError(f"expected one installed ODBC library, got {driver_paths}")
    driver_path = driver_paths[0].resolve()
    if any(part in {"build", "workspace"} for part in driver_path.parts):
        raise RuntimeError(f"driver resolves into a build tree: {driver_path}")
    owner = run(["dpkg-query", "-S", driver_path], capture=True).stdout.strip()
    if not owner.startswith("ydb-odbc:"):
        raise RuntimeError(f"installed driver is not owned by ydb-odbc: {owner}")
    registration = run(["odbcinst", "-q", "-d", "-n", "YDB"], capture=True).stdout.splitlines()
    if f"Driver={driver_path}" not in registration or f"Setup={driver_path}" not in registration:
        raise RuntimeError(f"YDB registration does not reference {driver_path}: {registration}")
    return package_version, package_sha256, str(driver_path)


def append_infrastructure_result(path: Path, consumer_id: str, message: str):
    now = int(time.time() * 1000)
    document = {
        "schema_version": 1,
        "tests": [{
            "id": f"{consumer_id}.infrastructure",
            "name": f"{consumer_id} test infrastructure",
            "status": "broken",
            "start": now,
            "stop": now,
            "message": message,
        }],
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(document, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main():
    consumer_id = os.environ["ODBC_CONSUMER_ID"]
    registry = validate_registry(FRAMEWORKS_ROOT / "registry.yaml")
    consumer = next(item for item in registry["consumers"] if item["id"] == consumer_id)
    adapter_root = FRAMEWORKS_ROOT / consumer_id
    package = Path("/packages") / os.environ["ODBC_PACKAGE_BASENAME"]

    RESULTS_ROOT.mkdir(parents=True, exist_ok=True)
    (RESULTS_ROOT / "native").mkdir(exist_ok=True)
    (RESULTS_ROOT / "allure").mkdir(exist_ok=True)
    WORK_ROOT.mkdir(exist_ok=True)
    home = WORK_ROOT / "home"
    home.mkdir(exist_ok=True)
    for path in (RESULTS_ROOT, RESULTS_ROOT / "native", RESULTS_ROOT / "allure", WORK_ROOT, home):
        path.chmod(0o777)

    package_version, package_sha256, driver_path = verify_package(package)
    upstream_revision, upstream_sha256, upstream_dir = prepare_upstream(consumer)

    dsn = f"YDB_{consumer_id}"
    endpoint = os.environ["YDB_ENDPOINT"]
    database = os.environ["YDB_DATABASE"]
    Path("/etc/odbc.ini").write_text(
        "[ODBC Data Sources]\n"
        f"{dsn}=YDB ODBC Driver\n\n"
        f"[{dsn}]\n"
        "Driver=YDB\n"
        f"Server={endpoint}\n"
        f"Database={database}\n"
        "AuthMode=Anonymous\n",
        encoding="utf-8",
    )

    test_env = os.environ.copy()
    test_env.pop("LD_LIBRARY_PATH", None)
    test_env.update({
        "HOME": str(home),
        "ODBCINI": "/etc/odbc.ini",
        "YDB_ODBC_DSN": dsn,
        "YDB_ODBC_CONNECTION_STRING": (
            f"Driver={{YDB}};Server={endpoint};Database={database};AuthMode=Anonymous"
        ),
        "YDB_ENDPOINT": endpoint,
        "YDB_DATABASE": database,
        "ODBC_RESULTS_DIR": str(RESULTS_ROOT),
        "ODBC_NATIVE_RESULTS_DIR": str(RESULTS_ROOT / "native"),
        "ODBC_ALLURE_RESULTS_DIR": str(RESULTS_ROOT / "allure"),
        "ODBC_UPSTREAM_DIR": str(upstream_dir),
        "ODBC_ADAPTER_DIR": str(adapter_root),
        "ODBC_HARNESS_DIR": str(HARNESS_ROOT),
    })

    runtime_version_command = adapter_root / consumer["commands"]["runtime_version"]
    runtime_version = run([runtime_version_command], env=test_env, capture=True, user="nobody").stdout.strip()
    metadata = {
        "consumer": consumer_id,
        "display_name": consumer["display_name"],
        "kind": consumer["kind"],
        "language": consumer["language"],
        "tier": consumer["tier"],
        "runtime_image": os.environ["ODBC_RUNTIME_IMAGE"],
        "runtime_version": runtime_version,
        "driver_commit": os.environ["DRIVER_COMMIT"],
        "driver_path": driver_path,
        "package_sha256": package_sha256,
        "package_version": package_version,
        "ydb_image": os.environ["YDB_TEST_IMAGE"],
        "endpoint": endpoint,
        "database": database,
        "connection_modes": consumer["modes"],
        "source_kind": consumer["source"]["kind"],
        "upstream_revision": upstream_revision,
        "upstream_sha256": upstream_sha256,
    }
    (RESULTS_ROOT / "metadata.json").write_text(
        json.dumps(metadata, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    test_command = adapter_root / consumer["commands"]["test"]
    test_process = run([test_command], env=test_env, check=False, user="nobody")
    native_results = RESULTS_ROOT / "native" / "results.json"
    if not native_results.is_file():
        append_infrastructure_result(native_results, consumer_id,
                                     f"test command exited {test_process.returncode} without results")

    example_process = None
    if "example" in consumer["commands"]:
        example_command = adapter_root / consumer["commands"]["example"]
        example_start = int(time.time() * 1000)
        example_process = run(
            [example_command], env=test_env, check=False, capture=True, user="nobody")
        example_stop = int(time.time() * 1000)
        example_log = RESULTS_ROOT / "native" / "upstream-example.log"
        example_log.write_text(example_process.stdout or "", encoding="utf-8")
        run([
            FRAMEWORKS_ROOT / "common" / "record_result.py",
            "--output", native_results,
            "--id", f"{consumer_id}.example",
            "--name", f"{consumer['display_name']} upstream example",
            "--status", "passed" if example_process.returncode == 0 else "failed",
            "--start", example_start,
            "--stop", example_stop,
            "--message", ("" if example_process.returncode == 0
                          else f"upstream example exited {example_process.returncode}"),
            "--log", example_log,
        ])

    convert_command = adapter_root / consumer["commands"]["convert"]
    convert_process = run([convert_command], env=test_env, check=False, user="nobody")
    validation_process = run([
        REPORTING_ROOT / "validate_results.py",
        "--results", RESULTS_ROOT,
        "--expectations", adapter_root / consumer["expectations"],
    ], env=test_env, check=False)

    failed = test_process.returncode or convert_process.returncode or validation_process.returncode
    if example_process is not None:
        failed = failed or example_process.returncode
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
