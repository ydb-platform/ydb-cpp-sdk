#!/usr/bin/env bash
set -euo pipefail

: "${OUTPUT_DIR:?OUTPUT_DIR is required}"
: "${GITHUB_REPOSITORY:?GITHUB_REPOSITORY is required}"
: "${GITHUB_RUN_ID:?GITHUB_RUN_ID is required}"

MARKER='<!-- ydb-cpp-sdk-coverage -->'
SUMMARY_JSON="${OUTPUT_DIR}/summary.json"

if [[ -z "${PR_NUMBER:-}" ]]; then
  echo "PR_NUMBER is not set; skipping coverage PR comment"
  exit 0
fi

if [[ ! -f "${SUMMARY_JSON}" ]]; then
  echo "::warning::${SUMMARY_JSON} not found; skipping coverage PR comment"
  exit 0
fi

if ! command -v gh >/dev/null 2>&1; then
  echo "::error::gh CLI is required to post PR comments"
  exit 1
fi

BODY="$(MARKER="${MARKER}" SUMMARY_JSON="${SUMMARY_JSON}" GITHUB_REPOSITORY="${GITHUB_REPOSITORY}" GITHUB_RUN_ID="${GITHUB_RUN_ID}" python3 <<'PY'
import json
import os
from pathlib import Path

marker = os.environ["MARKER"]
summary = json.loads(Path(os.environ["SUMMARY_JSON"]).read_text())
repo = os.environ["GITHUB_REPOSITORY"]
run_id = os.environ["GITHUB_RUN_ID"]

line = summary["line"]
branch = summary["branch"]
func = summary["function"]

def bar(percent: float) -> str:
    filled = max(0, min(10, int(round(percent / 10))))
    return "█" * filled + "░" * (10 - filled)

files = [
    f for f in summary.get("files", [])
    if f.get("line_total", 0) >= 20
]
files.sort(key=lambda f: f.get("line_percent") or 0)
lowest = files[:8]

run_url = f"https://github.com/{repo}/actions/runs/{run_id}"
lines = [
    marker,
    "## Code coverage",
    "",
    f"Workflow run: [Coverage job #{run_id}]({run_url}) · download the **coverage-report** artifact for the HTML report.",
    "",
    "| Metric | Coverage | Covered / total |",
    "| --- | --- | --- |",
    f"| Line | **{line['percent']:.1f}%** {bar(line['percent'])} | {line['covered']} / {line['num']} |",
    f"| Branch | **{branch['percent']:.1f}%** {bar(branch['percent'])} | {branch['covered']} / {branch['num']} |",
    f"| Function | **{func['percent']:.1f}%** {bar(func['percent'])} | {func['covered']} / {func['num']} |",
    "",
    "Scope: `src/`, `include/ydb-cpp-sdk/`, `plugins/` (contrib, tests, and `_deps` excluded).",
]

if lowest:
    lines.extend([
        "",
        "<details>",
        "<summary>Lowest line coverage (≥ 20 lines)</summary>",
        "",
        "| File | Line % | Covered / total |",
        "| --- | ---: | --- |",
    ])
    for f in lowest:
        pct = f.get("line_percent") or 0
        lines.append(
            f"| `{f['filename']}` | {pct:.1f}% | {f['line_covered']} / {f['line_total']} |"
        )
    lines.extend(["", "</details>"])

print("\n".join(lines))
PY
)"

COMMENT_ID="$(gh api "repos/${GITHUB_REPOSITORY}/issues/${PR_NUMBER}/comments" \
  --paginate \
  --jq ".[] | select(.body | contains(\"${MARKER}\")) | .id" \
  | head -n1)"

if [[ -n "${COMMENT_ID}" ]]; then
  gh api -X PATCH "repos/${GITHUB_REPOSITORY}/issues/comments/${COMMENT_ID}" \
    -f body="${BODY}" >/dev/null
  echo "Updated coverage comment ${COMMENT_ID} on PR #${PR_NUMBER}"
else
  gh api "repos/${GITHUB_REPOSITORY}/issues/${PR_NUMBER}/comments" \
    -f body="${BODY}" >/dev/null
  echo "Posted coverage comment on PR #${PR_NUMBER}"
fi
