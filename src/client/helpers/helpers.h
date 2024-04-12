#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/helpers/helpers.h
#include <ydb-cpp-sdk/client/driver/driver.h>
========
#include <src/client/ydb_driver/driver.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/helpers/helpers.h

namespace NYdb {

//! Checks the following environment variables and creates TDriverConfig with the first appeared:
//! YDB_SERVICE_ACCOUNT_KEY_FILE_CREDENTIALS=<path-to-file> — service account key file,
//! YDB_ANONYMOUS_CREDENTIALS="1" — uses anonymous access (used for test installation),
//! YDB_METADATA_CREDENTIALS="1" — uses metadata service,
//! YDB_ACCESS_TOKEN_CREDENTIALS=<access-token> — access token (for example, IAM-token).
//! If grpcs protocol is given in endpoint (or protocol is empty), enables SSL and uses
//! certificate from resourses and user cert from env variable "YDB_SERVICE_ACCOUNT_KEY_FILE_CREDENTIALS"
TDriverConfig CreateFromEnvironment(const std::string& connectionString = "");
//! Creates TDriverConfig from cloud service account key file
TDriverConfig CreateFromSaKeyFile(const std::string& saKeyFile, const std::string& connectionString = "");

}
