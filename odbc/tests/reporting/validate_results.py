#!/usr/bin/env python3

import argparse
import json
import sys
from pathlib import Path

import yaml


REQUIRED_METADATA = {
    "consumer", "language", "tier", "runtime_image", "runtime_version",
    "driver_commit", "package_sha256", "package_version", "ydb_image",
    "endpoint", "database", "source_kind",
}


def main():
    parser = argparse.ArgumentParser(description="Validate one ODBC consumer result set")
    parser.add_argument("--results", type=Path, required=True)
    parser.add_argument("--expectations", type=Path, required=True)
    args = parser.parse_args()

    errors = []
    metadata_path = args.results / "metadata.json"
    native_path = args.results / "native" / "results.json"
    validation_path = args.results / "validation.json"

    try:
        metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
        missing_metadata = sorted(REQUIRED_METADATA - metadata.keys())
        if missing_metadata:
            errors.append(f"missing metadata: {', '.join(missing_metadata)}")
    except (OSError, json.JSONDecodeError) as error:
        metadata = {}
        errors.append(f"invalid metadata.json: {error}")

    try:
        native = json.loads(native_path.read_text(encoding="utf-8"))
        tests = native.get("tests")
        if not isinstance(tests, list) or not tests:
            errors.append("test suite is empty")
            tests = []
    except (OSError, json.JSONDecodeError) as error:
        tests = []
        errors.append(f"invalid native/results.json: {error}")

    try:
        expectations = yaml.safe_load(args.expectations.read_text(encoding="utf-8"))["tests"]
    except (OSError, TypeError, KeyError, yaml.YAMLError) as error:
        expectations = {}
        errors.append(f"invalid expectations: {error}")

    actual = {}
    for test in tests:
        test_id = test.get("id") if isinstance(test, dict) else None
        if not test_id:
            errors.append("result with missing test id")
            continue
        if test_id in actual:
            errors.append(f"duplicate test result: {test_id}")
        actual[test_id] = test

    missing = sorted(set(expectations) - set(actual))
    unexpected = sorted(set(actual) - set(expectations))
    if missing:
        errors.append(f"missing tests: {', '.join(missing)}")
    if unexpected:
        errors.append(f"unexpected tests: {', '.join(unexpected)}")

    for test_id in sorted(set(expectations) & set(actual)):
        disposition = expectations[test_id].get("disposition")
        status = actual[test_id].get("status")
        if status not in {"passed", "failed", "skipped", "broken"}:
            errors.append(f"{test_id}: invalid status {status}")
        elif disposition == "required" and status != "passed":
            errors.append(f"{test_id}: required test finished as {status}")
        elif disposition == "unsupported" and status != "skipped":
            errors.append(f"{test_id}: unsupported test must be reported as skipped, got {status}")

    allure_results = list((args.results / "allure").glob("*-result.json"))
    if len(allure_results) != len(tests):
        errors.append(f"Allure result count {len(allure_results)} does not match native count {len(tests)}")

    validation = {
        "consumer": metadata.get("consumer", "unknown"),
        "ok": not errors,
        "test_count": len(tests),
        "errors": errors,
    }
    validation_path.write_text(json.dumps(validation, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if errors:
        for error in errors:
            print(f"result error: {error}", file=sys.stderr)
        return 1
    print(f"Validated {len(tests)} tests for {validation['consumer']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
