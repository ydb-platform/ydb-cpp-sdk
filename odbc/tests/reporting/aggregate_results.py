#!/usr/bin/env python3

import argparse
import json
import shutil
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description="Aggregate ODBC consumer artifacts")
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--expected", required=True, help="comma-separated consumer ids")
    parser.add_argument("--summary", type=Path)
    args = parser.parse_args()

    expected = {item for item in args.expected.split(",") if item}
    args.output.mkdir(parents=True, exist_ok=True)
    allure_output = args.output / "allure-results"
    native_output = args.output / "native"
    allure_output.mkdir(exist_ok=True)
    native_output.mkdir(exist_ok=True)

    found = {}
    errors = []
    rows = []
    for metadata_path in args.input.rglob("metadata.json"):
        result_root = metadata_path.parent
        try:
            metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
            validation = json.loads((result_root / "validation.json").read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as error:
            errors.append(f"invalid artifact at {result_root}: {error}")
            continue
        consumer = metadata.get("consumer")
        if not consumer:
            errors.append(f"artifact at {result_root} has no consumer id")
            continue
        if consumer in found:
            errors.append(f"duplicate result artifact for {consumer}")
            continue
        found[consumer] = result_root
        if not validation.get("ok"):
            errors.append(f"{consumer}: result validation failed")

        source_native = result_root / "native"
        source_allure = result_root / "allure"
        if not source_native.is_dir():
            errors.append(f"{consumer}: native result directory is missing")
            continue
        if not source_allure.is_dir():
            errors.append(f"{consumer}: Allure result directory is missing")
            continue
        consumer_native = native_output / consumer
        shutil.copytree(source_native, consumer_native, dirs_exist_ok=True)
        shutil.copy2(metadata_path, consumer_native / "metadata.json")
        shutil.copy2(result_root / "validation.json", consumer_native / "validation.json")
        for source in source_allure.iterdir():
            if source.name == "environment.properties":
                target = allure_output / f"{consumer}-environment.properties"
            else:
                target = allure_output / f"{consumer}-{source.name}"
            shutil.copy2(source, target)
        rows.append((consumer, validation.get("test_count", 0), "passed" if validation.get("ok") else "failed"))

    missing = sorted(expected - set(found))
    unexpected = sorted(set(found) - expected)
    if missing:
        errors.append(f"missing result artifacts: {', '.join(missing)}")
    if unexpected:
        errors.append(f"unexpected result artifacts: {', '.join(unexpected)}")

    summary_lines = [
        "## ODBC integration results",
        "",
        "| Consumer | Tests | Status |",
        "| --- | ---: | --- |",
    ]
    for consumer, count, status in sorted(rows):
        summary_lines.append(f"| {consumer} | {count} | {status} |")
    if errors:
        summary_lines.extend(["", "### Aggregation errors", ""] + [f"- {error}" for error in errors])
    summary = "\n".join(summary_lines) + "\n"
    (args.output / "summary.md").write_text(summary, encoding="utf-8")
    if args.summary:
        with args.summary.open("a", encoding="utf-8") as stream:
            stream.write(summary)
    print(summary)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
