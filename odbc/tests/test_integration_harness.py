import json
import shutil
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock
import yaml
FRAMEWORKS = Path(__file__).resolve().parent / "frameworks"
sys.path.insert(0, str(FRAMEWORKS))
import harness  # noqa: E402
class RegistryTests(unittest.TestCase):
    def mutate(self, callback):
        context = tempfile.TemporaryDirectory()
        self.addCleanup(context.cleanup)
        root = Path(context.name) / "frameworks"
        shutil.copytree(FRAMEWORKS, root)
        path = root / "registry.yaml"
        data = yaml.safe_load(path.read_text())
        callback(data, root)
        path.write_text(yaml.safe_dump(data, sort_keys=False))
        return path
    def test_current_registry(self):
        self.assertEqual(len(harness.load_registry()["consumers"]), 3)
    def test_rejects_duplicate_and_mutable_image(self):
        cases = [
            (lambda data, _: data["consumers"].append(dict(data["consumers"][0])), "duplicate"),
            (lambda data, _: data["consumers"][0].update(runtime_image="ubuntu:24.04"),
             "digest-pinned"),
        ]
        for mutation, message in cases:
            with self.subTest(message=message), self.assertRaisesRegex(harness.HarnessError, message):
                harness.load_registry(self.mutate(mutation))
    def test_binding_requires_pinned_source_and_example(self):
        def mutation(data, root):
            item = data["consumers"][1]
            item["kind"] = "binding"
            item["source"] = {"kind": "archive", "url": "https://example.invalid/source.tgz",
                              "revision": "v1", "sha256": "0" * 64}
            (root / item["id"] / "upstream.lock").write_text(yaml.safe_dump(item["source"]))
        with self.assertRaisesRegex(harness.HarnessError, "example"):
            harness.load_registry(self.mutate(mutation))
class ResultTests(unittest.TestCase):
    def validate(self, tests, required=("sample.case",)):
        context = tempfile.TemporaryDirectory()
        self.addCleanup(context.cleanup)
        root = Path(context.name)
        native, allure = root / "native", root / "allure"
        native.mkdir()
        allure.mkdir()
        (native / "results.json").write_text(json.dumps({"tests": tests}))
        for index in range(len(tests)):
            (allure / f"{index}-result.json").write_text("{}")
        consumer = {"id": "sample", "expected": {"required": list(required)}}
        with mock.patch.object(harness, "RESULTS", root):
            return harness.validate_results(consumer, native / "results.json", allure)
    def test_rejects_empty_missing_and_skipped_results(self):
        cases = [
            ([], "empty"),
            ([{"id": "other", "status": "passed"}], "missing test"),
            ([{"id": "sample.case", "status": "skipped"}], "unexpected status"),
        ]
        for tests, message in cases:
            with self.subTest(message=message):
                self.assertTrue(any(message in error for error in self.validate(tests)))
    def test_accepts_required_pass(self):
        self.assertFalse(self.validate([{"id": "sample.case", "status": "passed"}]))
    def test_aggregate_rejects_missing_artifacts(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.assertEqual(harness.aggregate(root / "input", root / "output"), 1)
            self.assertIn("missing result artifacts", (root / "output" / "summary.md").read_text())
if __name__ == "__main__":
    unittest.main()
