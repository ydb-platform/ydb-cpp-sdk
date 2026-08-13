#!/usr/bin/env bash
set -uo pipefail

package="/packages/${ODBC_PACKAGE_BASENAME:?}"
results_dir="${ODBC_RESULTS_DIR:?}"
native_dir="${results_dir}/native"
results_file="${native_dir}/results.json"
multiarch="$(dpkg-architecture -qDEB_HOST_MULTIARCH)"
driver_path="/usr/lib/${multiarch}/libydb-odbc.so"
driver_template="/usr/share/ydb-odbc/odbcinst.ini"
package_version="$(dpkg-deb -f "$package" Version)"
package_sha256="$(sha256sum "$package" | awk '{print $1}')"
old_package="/tmp/ydb-odbc-old.deb"
failed=0

mkdir -p "$native_dir" "${results_dir}/allure"
chmod 0777 "$results_dir" "$native_dir" "${results_dir}/allure"
apt-get update

record_case() {
    local test_id="$1"
    local name="$2"
    local function_name="$3"
    local log_file="${native_dir}/${test_id//./-}.log"
    local start stop rc status message

    start="$(date +%s%3N)"
    (set -euo pipefail; "$function_name") >"$log_file" 2>&1
    rc=$?
    stop="$(date +%s%3N)"
    status=passed
    message=""
    if [[ "$rc" -ne 0 ]]; then
        status=failed
        message="${name} failed with status ${rc}"
        failed=1
    fi
    python3 "${ODBC_HARNESS_DIR}/frameworks/common/record_result.py" \
        --output "$results_file" \
        --id "$test_id" \
        --name "$name" \
        --status "$status" \
        --start "$start" \
        --stop "$stop" \
        --message "$message" \
        --log "$log_file"
}

verify_ydb_registration() {
    local registration
    registration="$(odbcinst -q -d -n YDB)"
    grep -Fx "Driver=${driver_path}" <<<"$registration"
    grep -Fx "Setup=${driver_path}" <<<"$registration"
    grep -Fx "UsageCount=1" <<<"$registration"
}

test_metadata() {
    local package_name package_arch package_depends host_arch
    package_name="$(dpkg-deb -f "$package" Package)"
    package_arch="$(dpkg-deb -f "$package" Architecture)"
    package_depends="$(dpkg-deb -f "$package" Depends)"
    host_arch="$(dpkg --print-architecture)"
    test "$package_name" = ydb-odbc
    test "$package_arch" = "$host_arch"
    test -n "$package_version"
    for dependency in odbcinst libodbcinst2 libc6; do
        grep -Eq "(^|, )${dependency}([ (]|,|$)" <<<"$package_depends"
    done
}

setup_unrelated() {
    cat >/tmp/unrelated-odbcinst.ini <<EOF_UNRELATED
[UnrelatedPackageTest]
Description=Unrelated package test driver
Driver=/usr/lib/${multiarch}/libodbc.so.2
Setup=/usr/lib/${multiarch}/libodbcinst.so.2
EOF_UNRELATED
    odbcinst -i -d -f /tmp/unrelated-odbcinst.ini

    cat >/etc/odbc.ini <<EOF_ODBCINI
[ODBC Data Sources]
UnrelatedSystemDsn=Unrelated package test driver

[UnrelatedSystemDsn]
Driver=UnrelatedPackageTest
EOF_ODBCINI
    cat >/root/.odbc.ini <<EOF_USER_ODBCINI
[UnrelatedUserDsn]
Driver=UnrelatedPackageTest
EOF_USER_ODBCINI
    sha256sum /etc/odbc.ini /root/.odbc.ini >/tmp/odbc-ini.sha256
    odbcinst -q -d -n UnrelatedPackageTest
}

test_clean_install() {
    rm -rf /tmp/ydb-odbc-old
    dpkg-deb --raw-extract "$package" /tmp/ydb-odbc-old
    sed -i "s/^Version: .*/Version: ${package_version}~integration1/" \
        /tmp/ydb-odbc-old/DEBIAN/control
    dpkg-deb --build /tmp/ydb-odbc-old "$old_package"
    apt-get install -y "$old_package"
    test -f "$driver_path"
    test -f "$driver_template"
    grep -Fx "Driver=${driver_path}" "$driver_template"
    dpkg-query -S "$driver_path" | grep -Eq "^ydb-odbc(:[^:]+)?: ${driver_path}$"
    verify_ydb_registration
}

test_upgrade() {
    apt-get install -y "$package"
    test "$(dpkg-query -W -f='${Version}' ydb-odbc)" = "$package_version"
    verify_ydb_registration
    sha256sum --check /tmp/odbc-ini.sha256
}

test_remove_preserves_config() {
    apt-get remove -y ydb-odbc
    test ! -e "$driver_path"
    test ! -e "$driver_template"
    ! odbcinst -q -d -n YDB >/dev/null 2>&1
    odbcinst -q -d -n UnrelatedPackageTest
    sha256sum --check /tmp/odbc-ini.sha256
}

test_remove_preserves_replacement() {
    apt-get install -y "$package"
    verify_ydb_registration
    odbcinst -u -d -n YDB
    cat >/tmp/replacement-ydb-odbcinst.ini <<EOF_REPLACEMENT
[YDB]
Description=Replacement YDB ODBC driver
Driver=/usr/lib/${multiarch}/libodbc.so.2
Setup=/usr/lib/${multiarch}/libodbcinst.so.2
EOF_REPLACEMENT
    odbcinst -i -d -f /tmp/replacement-ydb-odbcinst.ini
    local replacement_registration
    replacement_registration="$(odbcinst -q -d -n YDB)"
    apt-get remove -y ydb-odbc
    test "$(odbcinst -q -d -n YDB)" = "$replacement_registration"
    sha256sum --check /tmp/odbc-ini.sha256
    odbcinst -u -d -n YDB
}

record_case package.metadata "Validate ydb-odbc metadata" test_metadata
record_case package.unrelated_setup "Create unrelated ODBC configuration" setup_unrelated
record_case package.clean_install "Install ydb-odbc in a clean container" test_clean_install
record_case package.upgrade "Upgrade ydb-odbc without duplicate registration" test_upgrade
record_case package.remove_preserves_config \
    "Remove ydb-odbc without changing unrelated configuration" test_remove_preserves_config
record_case package.remove_preserves_replacement \
    "Preserve a replacement YDB registration during removal" test_remove_preserves_replacement

python3 - "$results_dir/metadata.json" <<'PY'
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path

package = Path("/packages") / os.environ["ODBC_PACKAGE_BASENAME"]
metadata = {
    "consumer": "package-contract",
    "display_name": "Debian package contract",
    "kind": "package",
    "language": "shell",
    "tier": "smoke",
    "runtime_image": os.environ["ODBC_RUNTIME_IMAGE"],
    "runtime_version": subprocess.check_output(
        ["sh", "-c", ". /etc/os-release && printf '%s %s' \"$NAME\" \"$VERSION_ID\""],
        text=True,
    ),
    "driver_commit": os.environ["DRIVER_COMMIT"],
    "package_sha256": hashlib.sha256(package.read_bytes()).hexdigest(),
    "package_version": subprocess.check_output(["dpkg-deb", "-f", package, "Version"], text=True).strip(),
    "ydb_image": os.environ["YDB_TEST_IMAGE"],
    "endpoint": os.environ["YDB_ENDPOINT"],
    "database": os.environ["YDB_DATABASE"],
    "source_kind": "system",
    "upstream_revision": "system",
    "upstream_sha256": "system",
}
Path(sys.argv[1]).write_text(json.dumps(metadata, indent=2, sort_keys=True) + "\n", encoding="utf-8")
PY

exit "$failed"
