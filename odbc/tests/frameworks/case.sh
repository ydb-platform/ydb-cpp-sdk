#!/usr/bin/env bash

failed=0
run_case() {
    local id="$1" name="$2"
    shift 2
    local log="${ODBC_NATIVE_RESULTS_DIR}/${id//./-}.log" start stop rc status message
    start="$(date +%s%3N)"
    (set -e; "$@") >"$log" 2>&1
    rc=$?
    stop="$(date +%s%3N)"
    status=passed
    message=""
    if [[ "$rc" -ne 0 ]]; then
        status=failed
        message="${name} exited with status ${rc}"
        failed=1
    fi
    python3 "${ODBC_HARNESS_DIR}/harness.py" record \
        --output "${ODBC_NATIVE_RESULTS_DIR}/results.json" \
        --id "$id" --name "$name" --status "$status" \
        --start "$start" --stop "$stop" --message "$message" --log "$log"
}
