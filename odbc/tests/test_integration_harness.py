import json, shutil, sys, tempfile, unittest
from pathlib import Path
from unittest import mock
import yaml
FRAMEWORKS = Path(__file__).resolve().parent / "frameworks"
sys.path.insert(0, str(FRAMEWORKS))
import harness  # noqa: E402
class HarnessTests(unittest.TestCase):
    def registry(self, change=lambda data, root: None):
        context = tempfile.TemporaryDirectory(); self.addCleanup(context.cleanup)
        root = Path(context.name) / "frameworks"; shutil.copytree(FRAMEWORKS, root)
        path = root / "registry.yaml"; data = yaml.safe_load(path.read_text())
        change(data, root); path.write_text(yaml.safe_dump(data, sort_keys=False)); return path
    def validate(self, tests, expected):
        context = tempfile.TemporaryDirectory(); self.addCleanup(context.cleanup)
        root = Path(context.name); native, allure = root / "native", root / "allure"
        native.mkdir(); allure.mkdir(); (native / "results.json").write_text(json.dumps({"tests": tests}))
        for index in range(len(tests)): (allure / f"{index}-result.json").write_text("{}")
        with mock.patch.object(harness, "RESULTS", root):
            return harness.validate_results({"id": "sample", "expected": expected}, native / "results.json", allure)
    def test_registry_guards(self):
        self.assertEqual(len(harness.load_registry()["consumers"]), 3)
        cases = [(lambda data, _: data["consumers"].append(dict(data["consumers"][0])), "duplicate"),
                 (lambda data, _: data["consumers"][0].update(runtime_image="ubuntu:24.04"), "digest-pinned"),
                 (lambda data, _: data["consumers"][2]["source"].update(sha256="0" * 64), "upstream.lock")]
        for change, message in cases:
            with self.subTest(message=message), self.assertRaisesRegex(harness.HarnessError, message):
                harness.load_registry(self.registry(change))
    def test_result_guards(self):
        required = {"required": ["sample.case"]}
        self.assertFalse(self.validate([{"id": "sample.case", "status": "passed"}], required))
        for tests, message in (([], "empty"), ([{"id": "other", "status": "passed"}], "missing"),
                               ([{"id": "sample.case", "status": "skipped"}], "status")):
            self.assertTrue(any(message in error for error in self.validate(tests, required)))
        discovered = {"discovered": True, "required": ["sample.ok"],
                      "unsupported": {"sample.unsupported": "known"}}
        tests = [{"id": "sample.ok", "status": "passed"},
                 {"id": "sample.unsupported", "status": "failed", "message": "known"}]
        self.assertFalse(self.validate(tests, discovered))
if __name__ == "__main__": unittest.main()
