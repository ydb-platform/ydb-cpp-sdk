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
    def validate(self, tests, expected, test_rc=0):
        context = tempfile.TemporaryDirectory(); self.addCleanup(context.cleanup)
        root = Path(context.name); native, allure = root / "native", root / "allure"
        native.mkdir(); allure.mkdir(); (native / "results.json").write_text(json.dumps({"tests": tests}))
        for index in range(len(tests)): (allure / f"{index}-result.json").write_text("{}")
        with mock.patch.object(harness, "RESULTS", root):
            return harness.validate_results({"id": "sample", "expected": expected},
                                            native / "results.json", allure, test_rc)
    def test_registry_guards(self):
        registry = harness.load_registry()
        runs = list(harness.consumer_runs(registry))
        expected_run_ids = {
            consumer["id"] if mode == "all" else f"{consumer['id']}-{mode}"
            for consumer in registry["consumers"] for mode in (consumer.get("modes") or ["all"])
        }
        self.assertEqual({run["run_id"] for run in runs}, expected_run_ids)
        self.assertEqual(len(runs), len(expected_run_ids))
        self.assertEqual(harness.infrastructure_test_id("consumer", "dsn"),
                         "consumer.dsn.infrastructure")
        self.assertEqual(harness.infrastructure_test_id("consumer", "all"),
                         "consumer.infrastructure")
        def framework(data):
            return next(item for item in data["consumers"] if item["kind"] == "framework")
        def discovered(data):
            return next(item for item in data["consumers"]
                        if item.get("expected", {}).get("discovered"))
        def duplicate(data, _):
            data["consumers"].append(dict(data["consumers"][0]))
        def unpinned(data, _):
            data["consumers"][0]["runtime_image"] = "ubuntu:24.04"
        def changed_source(data, _):
            framework(data)["source"]["sha256"] = "0" * 64
        def add_classification(data, outcome, reason, consumer, pattern):
            data.setdefault("classifications", []).append({
                "outcome": outcome, "reason": reason, "tests": {consumer: [pattern]},
            })
        def broad_failure(data, _):
            consumer = discovered(data)
            add_classification(data, "unsupported", "too broad", consumer["id"],
                               f"{consumer['id']}.*.future_suite.*")
        def duplicate_classification(data, _):
            consumer = discovered(data)
            pattern = f"{consumer['id']}.*.future_suite.case"
            add_classification(data, "unsupported", "known failure", consumer["id"], pattern)
            add_classification(data, "skipped", "also skipped", consumer["id"], pattern)
        def empty_reason(data, _):
            consumer = discovered(data)
            add_classification(data, "unsupported", "", consumer["id"],
                               f"{consumer['id']}.*.future_suite.case")
        cases = [(duplicate, "duplicate consumer"),
                 (unpinned, "digest-pinned"),
                 (changed_source, "upstream.lock"),
                 (broad_failure, "suite-wide expected-result patterns"),
                 (duplicate_classification, "duplicate or unexplained expectations"),
                 (empty_reason, "reason must be a non-empty string")]
        for change, message in cases:
            with self.subTest(message=message), self.assertRaisesRegex(harness.HarnessError, message):
                harness.load_registry(self.registry(change))
    def test_result_guards(self):
        required = {"required": ["sample.case"]}
        self.assertFalse(self.validate([{"id": "sample.case", "status": "passed"}], required))
        for tests, message in (([], "empty"), ([{"id": "other", "status": "passed"}], "missing"),
                               ([{"id": "sample.case", "status": "skipped"}], "status")):
            self.assertTrue(any(message in error for error in self.validate(tests, required)))
        fixed_failure = {"required": ["sample.case"],
                         "unsupported": {"sample.case": "known failure"}}
        self.assertFalse(self.validate(
            [{"id": "sample.case", "status": "failed", "message": "raw assertion"}],
            fixed_failure, test_rc=1))
        discovered = {
            "discovered": True,
            "required": ["sample.*.*"],
            "unsupported": {"sample.legacy.knownFailure": "known failure"},
            "skipped": {"sample.upstream.disabled": "upstream gate"},
        }
        tests = [
            {"id": "sample.core.passes", "status": "passed"},
            {"id": "sample.future.newSuiteCase", "status": "passed"},
            {"id": "sample.legacy.knownFailure", "status": "failed", "message": "failed assertion"},
            {"id": "sample.upstream.disabled", "status": "skipped", "message": "disabled upstream"},
        ]
        self.assertFalse(self.validate(tests, discovered, test_rc=1))
        passing_failure = [dict(test) for test in tests]
        passing_failure[2] = {"id": "sample.legacy.knownFailure", "status": "passed"}
        self.assertTrue(any("for unsupported" in error
                            for error in self.validate(passing_failure, discovered)))
        stale = dict(discovered)
        stale["unsupported"] = dict(discovered["unsupported"])
        stale["unsupported"]["sample.removed.case"] = "stale"
        self.assertTrue(any("pattern matched no tests" in error
                            for error in self.validate(tests, stale, test_rc=1)))
        ambiguous = {
            "discovered": True,
            "required": ["sample.*"],
            "unsupported": {"sample.*.knownFailure": "known failure"},
            "skipped": {"sample.legacy.*": "upstream gate"},
        }
        self.assertTrue(any("ambiguous expected result" in error for error in self.validate(
            [{"id": "sample.legacy.knownFailure", "status": "failed", "message": "failure"}],
            ambiguous, test_rc=1)))
        self.assertTrue(any("although every reported test passed" in error for error in self.validate(
            [{"id": "sample.core.passes", "status": "passed"}],
            {"discovered": True, "required": ["sample.*"]}, test_rc=2)))
        self.assertEqual(harness.glob_ids(
            {"sample.one.alpha", "sample.two.beta", "sample.three.gamma"},
            "sample.{one,two}.{alpha,beta}"),
            {"sample.one.alpha", "sample.two.beta"})
        infrastructure = [{"id": "sample.dsn.infrastructure", "status": "broken",
                           "message": "fixture patch exited 2"}]
        errors = self.validate(infrastructure, discovered)
        self.assertEqual(errors, ["sample.dsn.infrastructure: fixture patch exited 2"])
    def test_manual_expected_reason_reaches_allure(self):
        context = tempfile.TemporaryDirectory(); self.addCleanup(context.cleanup)
        root = Path(context.name); native, allure = root / "native" / "results.json", root / "allure"
        native.parent.mkdir()
        native.write_text(json.dumps({"tests": [{
            "id": "sample.suite.knownFailure", "name": "known failure", "status": "failed",
            "message": "raw assertion", "trace": "raw stack", "start": 1, "stop": 2,
        }]}))
        config = {"classifications": [
            {"outcome": "unsupported", "reason": "manually classified reason",
             "tests": {"sample": ["sample.suite.knownFailure"]}},
            {"outcome": "unsupported", "reason": "second reason",
             "tests": {"sample": ["sample.suite.knownFailure"]}},
        ]}
        expected = {"discovered": True, "required": ["sample.*.*"]}
        expected.update(harness.load_classifications(config, [{"id": "sample"}])["sample"])
        consumer = {"id": "sample", "expected": expected}
        harness.annotate_expected_results(consumer, native)
        metadata = {
            "consumer": "sample", "language": "C++", "tier": "core",
            "driver_commit": "commit", "runtime_version": "runtime", "ydb_image": "ydb",
            "connection_mode": "dsn", "run_id": "sample-dsn", "endpoint": "localhost:2136",
            "database": "/local", "package_sha256": "package",
            "upstream_revision": "revision", "upstream_sha256": "source",
        }
        harness.convert_allure(native, allure, metadata)
        result = json.loads(next(allure.glob("*-result.json")).read_text())
        self.assertEqual(result["statusDetails"]["message"],
                         "Expected unsupported: manually classified reason; second reason")
        self.assertTrue(result["statusDetails"]["known"])
        self.assertIn("Reported FAILED: raw assertion", result["statusDetails"]["trace"])
        self.assertIn("raw stack", result["statusDetails"]["trace"])
        self.assertIn({"name": "originalStatus", "value": "failed"}, result["labels"])
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
        def assertion_shape(line):
            shape = re.sub(r"\s+", "", string_literal.sub('""', line[1:]))
            if shape.startswith("QVERIFY(q.prepare("):
                return "QVERIFY(q.prepare(...))"
            return shape
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
