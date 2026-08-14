import json, re, shutil, sys, tempfile, unittest
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
        registry = harness.load_registry()
        self.assertEqual(len(registry["consumers"]), 3)
        self.assertEqual(harness.infrastructure_test_id("qt", "dsn"), "qt.dsn.infrastructure")
        self.assertEqual(harness.infrastructure_test_id("qt", "connection_string"),
                         "qt.connection_string.infrastructure")
        self.assertEqual(harness.infrastructure_test_id("package-contract", "all"),
                         "package-contract.infrastructure")
        self.assertEqual({run["run_id"] for run in harness.consumer_runs(registry)},
                         {"package-contract", "isql-dsn", "qt-dsn", "qt-connection_string"})
        cases = [(lambda data, _: data["consumers"].append(dict(data["consumers"][0])), "duplicate"),
                 (lambda data, _: data["consumers"][0].update(runtime_image="ubuntu:24.04"), "digest-pinned"),
                 (lambda data, _: data["consumers"][2]["source"].update(sha256="0" * 64), "upstream.lock"),
                 (lambda data, _: data["consumers"][2]["expected"]["unsupported"].update(
                     {"qt.*.qsqlquery.*": "too broad"}), "suite-wide Qt exclusions")]
        for change, message in cases:
            with self.subTest(message=message), self.assertRaisesRegex(harness.HarnessError, message):
                harness.load_registry(self.registry(change))
    def test_result_guards(self):
        required = {"required": ["sample.case"]}
        self.assertFalse(self.validate([{"id": "sample.case", "status": "passed"}], required))
        for tests, message in (([], "empty"), ([{"id": "other", "status": "passed"}], "missing"),
                               ([{"id": "sample.case", "status": "skipped"}], "status")):
            self.assertTrue(any(message in error for error in self.validate(tests, required)))
        discovered = {"discovered": True, "required": ["sample.*"],
                      "unsupported": {"sample.unsupported": "known"}}
        tests = [{"id": "sample.ok", "status": "passed"},
                 {"id": "sample.unsupported", "status": "broken", "message": "known"}]
        self.assertFalse(self.validate(tests, discovered))
        infrastructure = [{"id": "sample.dsn.infrastructure", "status": "broken",
                           "message": "fixture patch exited 2"}]
        errors = self.validate(infrastructure, discovered)
        self.assertEqual(errors, ["sample.dsn.infrastructure: fixture patch exited 2"])
    def test_allure_ids_include_connection_mode(self):
        context = tempfile.TemporaryDirectory(); self.addCleanup(context.cleanup)
        root = Path(context.name); identities = []
        for mode in ("dsn", "connection_string"):
            native, allure = root / mode / "native" / "results.json", root / mode / "allure"
            native.parent.mkdir(parents=True)
            native.write_text(json.dumps({"tests": [{
                "id": harness.infrastructure_test_id("qt", mode), "name": "infrastructure",
                "status": "broken", "start": 1, "stop": 2, "message": "failure"}]}))
            metadata = {"consumer": "qt", "language": "C++", "tier": "core",
                        "driver_commit": "commit", "runtime_version": "6.4.2",
                        "ydb_image": "ydb", "connection_mode": mode,
                        "run_id": f"qt-{mode}", "endpoint": "localhost:2136",
                        "database": "/local", "package_sha256": "package",
                        "upstream_revision": "revision", "upstream_sha256": "source"}
            harness.convert_allure(native, allure, metadata)
            result = json.loads(next(allure.glob("*-result.json")).read_text())
            identities.append((result["uuid"], result["historyId"], result["testCaseId"]))
        self.assertEqual(len({identity[0] for identity in identities}), 2)
        self.assertEqual(len({identity[1] for identity in identities}), 2)
        self.assertEqual(len({identity[2] for identity in identities}), 2)
    def test_qt_fixture_patch_is_reviewable(self):
        patch = (FRAMEWORKS / "qt" / "ydb.patch").read_text()
        lock = yaml.safe_load((FRAMEWORKS / "qt" / "upstream.lock").read_text())
        runner = (FRAMEWORKS / "qt" / "run-tests").read_text()
        self.assertIn(f"Applies to qtbase revision {lock['revision']}.", patch)
        self.assertIn('["patch", "--batch", "--forward", "-p1", "-i", patch]', runner)
        self.assertNotIn("adapt_fixtures", runner)
        self.assertNotIn("re.subn", runner)
        self.assertNotIn("retry", runner)
        self.assertNotIn("YDB_QT_TEST_MODE", runner + patch)
        self.assertIn("Path(root).chmod(0o755)", runner)
        self.assertIn("fixture patch exited", runner)
        self.assertIn('q.exec("drop table if exists " + tableName)', patch)
        self.assertIn('"qsqldatabase": {"transaction"}', runner)
        changed = [line for line in patch.splitlines()
                   if line[:1] in {"+", "-"} and not line.startswith(("+++", "---"))]
        assertion = re.compile(r"\b(?:QCOMPARE|QVERIFY|QFAIL|QSKIP|QEXPECT_FAIL)\b")
        string_literal = re.compile(r'"(?:\\.|[^"\\])*"')
        assertion_shape = lambda line: re.sub(r"\s+", "", string_literal.sub('""', line[1:]))
        removed_assertions = [assertion_shape(line) for line in changed
                              if line.startswith("-") and assertion.search(line)]
        added_assertions = [assertion_shape(line) for line in changed
                            if line.startswith("+") and assertion.search(line)]
        self.assertCountEqual(added_assertions, removed_assertions)
        removed_keys = [line for line in changed
                        if line.startswith("-") and "primary key" in line.lower()]
        added_keys = [line for line in changed
                      if line.startswith("+") and "primary key" in line.lower()]
        self.assertGreaterEqual(len(added_keys), len(removed_keys))
if __name__ == "__main__": unittest.main()
