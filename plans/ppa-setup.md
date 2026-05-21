# PPA Setup Guide for YDB C++ SDK

This document describes how to create and configure a Launchpad PPA for
distributing YDB C++ SDK Debian packages.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [GPG Key Generation](#gpg-key-generation)
3. [Launchpad Account Setup](#launchpad-account-setup)
4. [PPA Creation](#ppa-creation)
5. [Local Configuration](#local-configuration)
6. [GitHub Secrets Configuration](#github-secrets-configuration)
7. [Testing the Upload Flow](#testing-the-upload-flow)
8. [Package Upload Order](#package-upload-order)
9. [Version Bumping](#version-bumping)
10. [Troubleshooting](#troubleshooting)

---

## Prerequisites

Install the required tools on your development machine (Ubuntu 22.04+):

```bash
sudo apt-get update
sudo apt-get install -y \
    devscripts \
    dput \
    debhelper \
    dpkg-dev \
    gnupg \
    git
```

## GPG Key Generation

Launchpad requires a GPG key to sign source packages.

### Generate a new key

```bash
gpg --full-generate-key
```

When prompted:
- **Key type**: RSA and RSA (default)
- **Key size**: 4096 bits
- **Expiration**: 0 (does not expire) — or set a reasonable expiration
- **Real name**: `YDB Team` (or your name)
- **Email**: `ydb-team@ydb.tech` (must match your Launchpad account email)
- **Passphrase**: choose a strong passphrase

### Find your key ID

```bash
gpg --list-secret-keys --keyid-format long
```

Output example:
```
sec   rsa4096/ABCDEF1234567890 2024-01-01 [SC]
      FINGERPRINT1234567890ABCDEF1234567890ABCDEF
uid                 [ultimate] YDB Team <ydb-team@ydb.tech>
ssb   rsa4096/1234567890ABCDEF 2024-01-01 [E]
```

Your **key ID** is `ABCDEF1234567890` (the part after `rsa4096/`).
Your **fingerprint** is the full 40-character hex string.

### Upload key to Ubuntu keyserver

```bash
gpg --keyserver keyserver.ubuntu.com --send-keys ABCDEF1234567890
```

Wait a few minutes for propagation.

### Export the private key (for CI)

```bash
gpg --armor --export-secret-keys ABCDEF1234567890 > gpg-private-key.asc
```

**Keep this file secure!** It will be stored as a GitHub secret.

## Launchpad Account Setup

1. Go to https://launchpad.net and create an account (or sign in).
2. Navigate to https://launchpad.net/~/+editpgpkeys
3. Paste your GPG key **fingerprint** (40-character hex string).
4. Launchpad will send an encrypted email to verify ownership.
5. Decrypt the email and follow the confirmation link.

## PPA Creation

1. Go to https://launchpad.net/~/+activate-ppa
2. Fill in:
   - **URL**: `ydb-cpp-sdk`
   - **Display name**: `YDB C++ SDK`
   - **Description**:
     ```
     YDB C++ SDK development packages and dependencies.
     
     Packages:
     - yandex-googleapis-api-common-protos: compiled Google API common proto specs
     - yandex-opentelemetry-cpp-dev: OpenTelemetry C++ SDK
     - libydb-cpp-dev: YDB C++ SDK core library
     - libydb-cpp-iam-dev: YDB C++ SDK IAM plugin
     - libydb-cpp-otel-metrics-dev: YDB C++ SDK OpenTelemetry metrics plugin
     - libydb-cpp-otel-tracing-dev: YDB C++ SDK OpenTelemetry tracing plugin
     ```
3. Click **Activate**.

The PPA URL will be: `ppa:YOUR_LAUNCHPAD_USERNAME/ydb-cpp-sdk`

### Enable additional architectures (optional)

By default, PPAs build for `amd64` and `i386`. To add `arm64`:

1. Go to your PPA page on Launchpad
2. Click **Change details**
3. Under **Processors**, check `ARM ARMv8 (arm64)`

## Local Configuration

### Configure dput

Create or edit `~/.dput.cf`:

```ini
[ydb-cpp-sdk]
fqdn = ppa.launchpad.net
method = ftp
incoming = ~YOUR_LAUNCHPAD_USERNAME/ubuntu/ydb-cpp-sdk/
login = anonymous
allow_unsigned_uploads = 0
```

Alternatively, use the shorthand PPA syntax with `dput`:
```bash
dput ppa:YOUR_LAUNCHPAD_USERNAME/ydb-cpp-sdk <changes_file>
```

## GitHub Secrets Configuration

For automated CI publishing, add these secrets to your GitHub repository
(Settings → Secrets and variables → Actions):

| Secret Name | Description | How to obtain |
|---|---|---|
| `GPG_PRIVATE_KEY` | ASCII-armored GPG private key | `gpg --armor --export-secret-keys KEY_ID` |
| `GPG_PASSPHRASE` | Passphrase for the GPG key | The passphrase you chose during key generation |
| `LAUNCHPAD_PPA` | PPA identifier | `ppa:YOUR_USERNAME/ydb-cpp-sdk` |

### Setting secrets via GitHub CLI

```bash
gh secret set GPG_PRIVATE_KEY < gpg-private-key.asc
gh secret set GPG_PASSPHRASE --body "your-passphrase"
gh secret set LAUNCHPAD_PPA --body "ppa:ydb-team/ydb-cpp-sdk"
```

## Testing the Upload Flow

### Dry run (no upload)

```bash
# Build all source packages without uploading
./scripts/upload_to_ppa.sh --all --dry-run --series noble

# Build only googleapis
./scripts/upload_to_ppa.sh --googleapis --dry-run

# Build only SDK
./scripts/upload_to_ppa.sh --sdk --dry-run --series noble
```

### Actual upload

```bash
# Upload all packages
./scripts/upload_to_ppa.sh --all --gpg-key ABCDEF1234567890 --series noble

# Upload only googleapis first (required before SDK)
./scripts/upload_to_ppa.sh --googleapis --gpg-key ABCDEF1234567890

# Then upload opentelemetry-cpp
./scripts/upload_to_ppa.sh --otel --gpg-key ABCDEF1234567890

# Then upload SDK (after googleapis and otel are published)
./scripts/upload_to_ppa.sh --sdk --gpg-key ABCDEF1234567890
```

### Via GitHub Actions

1. Go to **Actions** → **Publish to PPA**
2. Click **Run workflow**
3. Select parameters:
   - Series: `noble`
   - Packages: `all`
   - Dry run: `true` (for testing)
4. Review the output
5. Re-run with dry_run=`false` for actual upload

## Package Upload Order

**Important**: Packages must be uploaded and **published** in this order
because of build dependencies:

1. `yandex-googleapis-api-common-protos` — no PPA dependencies
2. `yandex-opentelemetry-cpp` — no PPA dependencies
3. `ydb-cpp-sdk` — depends on both (1) and (2) at build time

Wait for each package to be **published** on Launchpad before uploading
the next one. Launchpad build times vary but typically take 15–60 minutes.

You can check build status at:
`https://launchpad.net/~YOUR_USERNAME/+archive/ubuntu/ydb-cpp-sdk/+packages`

## Version Bumping

### Rules for PPA versions

- Each upload to the same series must have a **unique version**.
- Use the `~SERIES1` suffix convention: `3.18.0-1~noble1`
- For rebuilds, increment the suffix: `3.18.0-1~noble2`, `3.18.0-1~noble3`
- For multiple series, use different suffixes: `3.18.0-1~noble1`, `3.18.0-1~jammy1`

### Updating the SDK version

1. Edit `src/version.h` and update `YDB_SDK_VERSION`
2. The `generate-debian-directory.sh` script automatically extracts the version
3. Run `./scripts/upload_to_ppa.sh --sdk --series noble`

### Updating the googleapis version

1. Update the `CPACK_PACKAGE_VERSION` in `scripts/googleapis_deb/CMakeLists.txt`
2. Run `./scripts/upload_to_ppa.sh --googleapis --series noble`

## Troubleshooting

### "Signature verification failed"

- Ensure your GPG key is uploaded to `keyserver.ubuntu.com`
- Ensure the key is registered on your Launchpad account
- Wait a few minutes after uploading the key for propagation

### "Version already exists"

- Increment the PPA suffix (e.g., `~noble1` → `~noble2`)
- Or delete the existing package from the PPA web interface

### "Build dependency not satisfiable"

- Ensure prerequisite packages are published in the PPA first
- Check that package names in `debian/control` match exactly

### "Source format 3.0 (quilt) requires .orig.tar.gz"

- Run without `--skip-orig` flag
- Or ensure the `.orig.tar.gz` was previously uploaded for this version

### Build fails on Launchpad

- Check the build log on the Launchpad PPA page
- Common issues: missing Build-Depends, network access during build
  (Launchpad builds are sandboxed — no internet access)
- For opentelemetry-cpp: the FetchContent approach won't work on Launchpad;
  the source must be included in the orig tarball

### "Rejected: Could not find person"

- The PPA identifier is wrong. Use `ppa:USERNAME/PPA_NAME` format.
- Verify with: `https://launchpad.net/~USERNAME/+archive/ubuntu/PPA_NAME`
