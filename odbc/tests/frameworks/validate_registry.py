#!/usr/bin/env python3

import argparse
import json
import os
import re
import sys
from pathlib import Path

import yaml


ID_RE = re.compile(r"^[a-z][a-z0-9_-]*$")
DIGEST_RE = re.compile(r"^.+@sha256:[0-9a-f]{64}$")
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
COMMAND_KEYS = {"test", "convert", "runtime_version"}


class RegistryError(ValueError):
    pass


def load_yaml(path: Path):
    try:
        with path.open(encoding="utf-8") as stream:
            return yaml.safe_load(stream)
    except (OSError, yaml.YAMLError) as error:
        raise RegistryError(f"cannot read {path}: {error}") from error


def require(condition: bool, message: str):
    if not condition:
        raise RegistryError(message)


def resolve_child(root: Path, relative: str, description: str) -> Path:
    require(isinstance(relative, str) and relative, f"{description} must be a non-empty path")
    candidate = (root / relative).resolve()
    try:
        candidate.relative_to(root.resolve())
    except ValueError as error:
        raise RegistryError(f"{description} escapes its adapter directory: {relative}") from error
    return candidate


def validate_expectations(path: Path, consumer_id: str):
    data = load_yaml(path)
    require(isinstance(data, dict), f"{consumer_id}: expectations must be a mapping")
    require(data.get("schema_version") == 1, f"{consumer_id}: unsupported expectations schema")
    tests = data.get("tests")
    require(isinstance(tests, dict) and tests, f"{consumer_id}: expectations must list tests")
    for test_id, expectation in tests.items():
        require(isinstance(test_id, str) and test_id, f"{consumer_id}: invalid expected test id")
        require(isinstance(expectation, dict), f"{consumer_id}: expectation for {test_id} must be a mapping")
        disposition = expectation.get("disposition")
        require(disposition in {"required", "unsupported"},
                f"{consumer_id}: invalid disposition for {test_id}: {disposition}")
        if disposition == "unsupported":
            require(bool(expectation.get("reason")),
                    f"{consumer_id}: unsupported test {test_id} requires a reason")
    return tests


def validate_registry(path: Path):
    data = load_yaml(path)
    require(isinstance(data, dict), "registry root must be a mapping")
    require(data.get("schema_version") == 1, "unsupported registry schema_version")

    workflow = data.get("workflow")
    require(isinstance(workflow, dict), "registry.workflow must be a mapping")
    require(isinstance(workflow.get("package_runtime_image"), str)
            and DIGEST_RE.match(workflow["package_runtime_image"]),
            "registry.workflow.package_runtime_image must be pinned by sha256 digest")

    ydb = data.get("ydb")
    require(isinstance(ydb, dict), "registry.ydb must be a mapping")
    require(isinstance(ydb.get("image"), str) and DIGEST_RE.match(ydb["image"]),
            "registry.ydb.image must be pinned by sha256 digest")
    require(isinstance(ydb.get("endpoint"), str) and ydb["endpoint"], "registry.ydb.endpoint is required")
    require(isinstance(ydb.get("database"), str) and ydb["database"].startswith("/"),
            "registry.ydb.database must be an absolute YDB path")

    consumers = data.get("consumers")
    require(isinstance(consumers, list) and consumers, "registry must contain at least one consumer")
    registry_root = path.parent.resolve()
    seen = set()

    for consumer in consumers:
        require(isinstance(consumer, dict), "each consumer must be a mapping")
        consumer_id = consumer.get("id")
        require(isinstance(consumer_id, str) and ID_RE.match(consumer_id),
                f"invalid consumer id: {consumer_id}")
        require(consumer_id not in seen, f"duplicate consumer id: {consumer_id}")
        seen.add(consumer_id)

        require(consumer.get("kind") in {"smoke", "binding"},
                f"{consumer_id}: kind must be smoke or binding")
        require(consumer.get("tier") in {"smoke", "core", "expansion"},
                f"{consumer_id}: invalid tier")
        require(bool(consumer.get("display_name")), f"{consumer_id}: display_name is required")
        require(bool(consumer.get("language")), f"{consumer_id}: language is required")
        require(isinstance(consumer.get("runtime_image"), str)
                and DIGEST_RE.match(consumer["runtime_image"]),
                f"{consumer_id}: runtime_image must be pinned by sha256 digest")
        require(consumer.get("result_format") == "ydb-odbc-normalized-v1",
                f"{consumer_id}: unsupported result_format")

        adapter_root = (registry_root / consumer_id).resolve()
        require(adapter_root.is_dir(), f"{consumer_id}: adapter directory is missing")
        require((adapter_root / "Dockerfile").is_file(), f"{consumer_id}: Dockerfile is missing")

        commands = consumer.get("commands")
        require(isinstance(commands, dict), f"{consumer_id}: commands must be a mapping")
        missing_commands = COMMAND_KEYS - commands.keys()
        require(not missing_commands,
                f"{consumer_id}: missing commands: {', '.join(sorted(missing_commands))}")
        if consumer["kind"] == "binding":
            require("example" in commands, f"{consumer_id}: binding requires an example command")
        for name, relative in commands.items():
            require(name in COMMAND_KEYS | {"example"}, f"{consumer_id}: unknown command {name}")
            command_path = resolve_child(adapter_root, relative, f"{consumer_id}.{name}")
            require(command_path.is_file(), f"{consumer_id}: command does not exist: {relative}")
            require(os.access(command_path, os.X_OK), f"{consumer_id}: command is not executable: {relative}")

        expectations = resolve_child(adapter_root, consumer.get("expectations"),
                                     f"{consumer_id}.expectations")
        require(expectations.is_file(), f"{consumer_id}: expectations file is missing")
        expected_tests = validate_expectations(expectations, consumer_id)
        if consumer["kind"] == "binding":
            example_id = f"{consumer_id}.example"
            require(example_id in expected_tests,
                    f"{consumer_id}: expectations must include {example_id}")
            require(expected_tests[example_id]["disposition"] == "required",
                    f"{consumer_id}: {example_id} must be required")

        modes = consumer.get("modes")
        require(isinstance(modes, list) and modes, f"{consumer_id}: modes must not be empty")
        require(set(modes) <= {"dsn", "connection_string"}, f"{consumer_id}: unsupported connection mode")

        source = consumer.get("source")
        require(isinstance(source, dict), f"{consumer_id}: source must be a mapping")
        if consumer["kind"] == "smoke":
            require(source.get("kind") == "system", f"{consumer_id}: smoke source must be system")
        else:
            require(source.get("kind") == "archive", f"{consumer_id}: binding source must be archive")
            for field in ("url", "revision", "sha256"):
                require(bool(source.get(field)), f"{consumer_id}: source.{field} is required")
            require(SHA256_RE.match(source["sha256"]) is not None,
                    f"{consumer_id}: source.sha256 must be lowercase SHA-256")
            lock_path = adapter_root / "upstream.lock"
            require(lock_path.is_file(), f"{consumer_id}: upstream.lock is missing")
            lock = load_yaml(lock_path)
            require(lock == source, f"{consumer_id}: upstream.lock must exactly match source")

    return data


def matrix(data):
    return {
        "include": [
            {
                "consumer": item["id"],
                "display_name": item["display_name"],
                "runtime_image": item["runtime_image"],
            }
            for item in data["consumers"]
        ]
    }


def main():
    default_registry = Path(__file__).with_name("registry.yaml")
    parser = argparse.ArgumentParser(description="Validate the ODBC consumer registry")
    parser.add_argument("--registry", type=Path, default=default_registry)
    parser.add_argument("--matrix", action="store_true", help="print the GitHub Actions matrix as JSON")
    parser.add_argument("--consumer", help="print one validated consumer as JSON")
    parser.add_argument("--workflow", action="store_true", help="print workflow configuration as JSON")
    args = parser.parse_args()

    try:
        data = validate_registry(args.registry.resolve())
        if args.consumer:
            selected = next((item for item in data["consumers"] if item["id"] == args.consumer), None)
            require(selected is not None, f"unknown consumer: {args.consumer}")
            print(json.dumps(selected, separators=(",", ":")))
        elif args.workflow:
            output = {
                "package_runtime_image": data["workflow"]["package_runtime_image"],
                "ydb": data["ydb"],
                "consumer_ids": [item["id"] for item in data["consumers"]],
            }
            print(json.dumps(output, separators=(",", ":")))
        elif args.matrix:
            print(json.dumps(matrix(data), separators=(",", ":")))
        else:
            print(f"Validated {len(data['consumers'])} ODBC consumers")
    except RegistryError as error:
        print(f"registry error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
