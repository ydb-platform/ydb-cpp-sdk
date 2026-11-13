# Debian package contract

The default suite checks clean installation, reinstallation, removal and replacement
registration. Reinstallation uses the unchanged candidate package and is reported as
`package.reinstall`.

Supply an actual older package to add `package.upgrade`:

```sh
python3 odbc/tests/frameworks/harness.py launch --consumer package-contract \
  --runtime-image "$RUNTIME_IMAGE" --package "$PACKAGE" --results "$RESULTS" \
  --previous-package "$PREVIOUS_PACKAGE" \
  --previous-package-sha256 "$PREVIOUS_PACKAGE_SHA256" \
  --previous-package-source "$PREVIOUS_PACKAGE_SOURCE"
```

The corresponding environment variables are `ODBC_PREVIOUS_PACKAGE`,
`ODBC_PREVIOUS_PACKAGE_SHA256` and `ODBC_PREVIOUS_PACKAGE_SOURCE`. The source can be
an artifact URL or source revision. The launcher mounts the older artifact read-only.
Both packages must be `ydb-odbc` for the host architecture, and Debian must compare
the previous version as strictly older. An optional checksum is checked before any
package installation. Packages are never relabelled or rebuilt by the suite.

The upgrade case removes the clean-install candidate, installs the supplied previous
package, and upgrades it to the candidate. Reinstallation and removal follow. Each
step checks the single YDB registration, its usage count and multiarch driver path,
and preserves YDB and unrelated user/system DSNs and the unrelated driver registration.
Native `package-upgrade.json`, the upgrade log and result metadata retain both package
versions and checksums and the supplied previous source.

An existing prebuilt baseline is `ydb-odbc_3.22.0_amd64.deb` from source revision
`ad4290aeb7a1bbdfb92c100d067db6f58b98e40c`, with SHA256
`a1220ebeb2d4958292f19a491c39dfa3242811cd8eb69f92755b61b6e3775ea9`.
It is supplied explicitly; it is not a published ODBC release or an automatically
downloaded fixture. Release publication remains a separate action.
