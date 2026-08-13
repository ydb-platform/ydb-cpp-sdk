#!/usr/bin/env python3

import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import yaml


TESTS_ROOT = Path(__file__).resolve().parent
FRAMEWORKS_ROOT = TESTS_ROOT / "frameworks"
REPORTING_ROOT = TESTS_ROOT / "reporting"


class RegistryTests(unittest.TestCase):
    def run_validator(self, registry: Path):
        return subprocess.run(
            [sys.executable, FRAMEWORKS_ROOT / "validate_registry.py", "--registry", registry],
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    def test_current_registry_is_valid(self):
        result = self.run_validator(FRAMEWORKS_ROOT / "registry.yaml")
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_duplicate_consumer_is_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            copied = Path(temp) / "frameworks"
            shutil.copytree(FRAMEWORKS_ROOT, copied)
            registry_path = copied / "registry.yaml"
            registry = yaml.safe_load(registry_path.read_text(encoding="utf-8"))
            registry["consumers"].append(dict(registry["consumers"][0]))
            registry_path.write_text(yaml.safe_dump(registry, sort_keys=False), encoding="utf-8")
            result = self.run_validator(registry_path)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("duplicate consumer id", result.stderr)

    def test_mutable_runtime_image_is_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            copied = Path(temp) / "frameworks"
            shutil.copytree(FRAMEWORKS_ROOT, copied)
            registry_path = copied / "registry.yaml"
            registry = yaml.safe_load(registry_path.read_text(encoding="utf-8"))
            registry["consumers"][0]["runtime_image"] = "ubuntu:24.04"
            registry_path.write_text(yaml.safe_dump(registry, sort_keys=False), encoding="utf-8")
            result = self.run_validator(registry_path)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("pinned by sha256 digest", result.stderr)

    def test_binding_requires_a_reported_upstream_example(self):
        with tempfile.TemporaryDirectory() as temp:
            copied = Path(temp) / "frameworks"
            shutil.copytree(FRAMEWORKS_ROOT, copied)
            registry_path = copied / "registry.yaml"
            registry = yaml.safe_load(registry_path.read_text(encoding="utf-8"))
            consumer = registry["consumers"][0]
            consumer["kind"] = "binding"
            consumer["commands"]["example"] = "run-tests"
            consumer["source"] = {
                "kind": "archive",
                "url": "https://example.invalid/upstream.tar.gz",
                "revision": "v1.0.0",
                "sha256": "0" * 64,
            }
            (copied / consumer["id"] / "upstream.lock").write_text(
                yaml.safe_dump(consumer["source"], sort_keys=False), encoding="utf-8")
            registry_path.write_text(yaml.safe_dump(registry, sort_keys=False), encoding="utf-8")
            result = self.run_validator(registry_path)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("expectations must include isql.example", result.stderr)


class ResultValidationTests(unittest.TestCase):
    def create_results(self, root: Path, tests):
        native = root / "native"
        allure = root / "allure"
        native.mkdir(parents=True)
        allure.mkdir()
        metadata = {
            "consumer": "sample",
            "language": "test",
            "tier": "smoke",
            "runtime_image": "example@sha256:" + "0" * 64,
            "runtime_version": "1",
            "driver_commit": "abc",
            "package_sha256": "1" * 64,
            "package_version": "1.0.0",
            "ydb_image": "ydb@sha256:" + "2" * 64,
            "endpoint": "localhost:2136",
            "database": "/local",
            "source_kind": "system",
        }
        (root / "metadata.json").write_text(json.dumps(metadata), encoding="utf-8")
        (native / "results.json").write_text(
            json.dumps({"schema_version": 1, "tests": tests}), encoding="utf-8")
        for index, _ in enumerate(tests):
            (allure / f"{index}-result.json").write_text("{}", encoding="utf-8")

    def run_validator(self, results: Path, expectations: Path):
        return subprocess.run(
            [
                sys.executable,
                REPORTING_ROOT / "validate_results.py",
                "--results", results,
                "--expectations", expectations,
            ],
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    def write_expectations(self, path: Path, disposition="required"):
        path.write_text(
            yaml.safe_dump({
                "schema_version": 1,
                "tests": {"sample.case": {"disposition": disposition, "reason": "not supported"}},
            }),
            encoding="utf-8",
        )

    def test_required_passing_result_is_valid(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            results = root / "results"
            expectations = root / "expectations.yaml"
            self.create_results(results, [{"id": "sample.case", "status": "passed"}])
            self.write_expectations(expectations)
            result = self.run_validator(results, expectations)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_required_skip_is_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            results = root / "results"
            expectations = root / "expectations.yaml"
            self.create_results(results, [{"id": "sample.case", "status": "skipped"}])
            self.write_expectations(expectations)
            result = self.run_validator(results, expectations)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("required test finished as skipped", result.stderr)

    def test_empty_suite_is_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            results = root / "results"
            expectations = root / "expectations.yaml"
            self.create_results(results, [])
            self.write_expectations(expectations)
            result = self.run_validator(results, expectations)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("test suite is empty", result.stderr)


class AggregationTests(unittest.TestCase):
    def test_missing_artifact_fails_with_summary(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            source = root / "source"
            output = root / "output"
            source.mkdir()
            result = subprocess.run(
                [
                    sys.executable,
                    REPORTING_ROOT / "aggregate_results.py",
                    "--input", source,
                    "--output", output,
                    "--expected", "package-contract,isql,qt",
                ],
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            self.assertNotEqual(result.returncode, 0)
            summary = (output / "summary.md").read_text(encoding="utf-8")
            self.assertIn("missing result artifacts: isql, package-contract, qt", summary)


if __name__ == "__main__":
    unittest.main()
