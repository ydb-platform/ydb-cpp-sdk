#!/usr/bin/env python3

import argparse
import json
import shutil
import uuid
from pathlib import Path


STATUS_MAP = {
    "passed": "passed",
    "failed": "failed",
    "skipped": "skipped",
    "broken": "broken",
}


def main():
    parser = argparse.ArgumentParser(description="Convert normalized ODBC results to Allure JSON")
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--metadata", type=Path, required=True)
    args = parser.parse_args()

    normalized = json.loads(args.input.read_text(encoding="utf-8"))
    metadata = json.loads(args.metadata.read_text(encoding="utf-8"))
    tests = normalized.get("tests")
    if not isinstance(tests, list) or not tests:
        raise SystemExit("normalized result contains no tests")

    args.output.mkdir(parents=True, exist_ok=True)
    for test in tests:
        test_id = test["id"]
        result_uuid = str(uuid.uuid5(uuid.NAMESPACE_URL, f"ydb-odbc:{metadata['consumer']}:{test_id}"))
        attachments = []
        for index, attachment in enumerate(test.get("attachments", [])):
            source = args.input.parent / attachment["path"]
            if not source.is_file():
                raise SystemExit(f"missing attachment for {test_id}: {source}")
            suffix = source.suffix or ".txt"
            target_name = f"{result_uuid}-{index}-attachment{suffix}"
            shutil.copyfile(source, args.output / target_name)
            attachments.append({
                "name": attachment.get("name", source.name),
                "source": target_name,
                "type": attachment.get("type", "text/plain"),
            })

        details = {}
        if test.get("message"):
            details["message"] = test["message"]
        if test.get("trace"):
            details["trace"] = test["trace"]
        result = {
            "uuid": result_uuid,
            "historyId": f"{metadata['consumer']}::{test_id}",
            "testCaseId": test_id,
            "name": test.get("name", test_id),
            "fullName": f"{metadata['consumer']}.{test_id}",
            "status": STATUS_MAP[test["status"]],
            "stage": "finished",
            "start": int(test["start"]),
            "stop": int(test["stop"]),
            "labels": [
                {"name": "suite", "value": metadata["consumer"]},
                {"name": "language", "value": metadata["language"]},
                {"name": "tier", "value": metadata["tier"]},
                {"name": "driverCommit", "value": metadata["driver_commit"]},
                {"name": "runtimeVersion", "value": metadata["runtime_version"]},
                {"name": "ydbImage", "value": metadata["ydb_image"]},
            ],
            "parameters": [
                {"name": "endpoint", "value": metadata["endpoint"]},
                {"name": "database", "value": metadata["database"]},
                {"name": "packageSha256", "value": metadata["package_sha256"]},
                {"name": "upstreamRevision", "value": metadata.get("upstream_revision", "system")},
                {"name": "upstreamSha256", "value": metadata.get("upstream_sha256", "system")},
            ],
            "attachments": attachments,
        }
        if details:
            result["statusDetails"] = details
        (args.output / f"{result_uuid}-result.json").write_text(
            json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    properties = {
        "consumer": metadata["consumer"],
        "runtime.version": metadata["runtime_version"],
        "driver.commit": metadata["driver_commit"],
        "package.sha256": metadata["package_sha256"],
        "ydb.image": metadata["ydb_image"],
        "ydb.endpoint": metadata["endpoint"],
        "ydb.database": metadata["database"],
        "upstream.revision": metadata.get("upstream_revision", "system"),
        "upstream.sha256": metadata.get("upstream_sha256", "system"),
    }
    (args.output / "environment.properties").write_text(
        "".join(f"{key}={value}\n" for key, value in sorted(properties.items())), encoding="utf-8")


if __name__ == "__main__":
    main()
