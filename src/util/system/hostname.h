#pragma once

#include <src/util/generic/fwd.h>

const char* GetHostName();
const std::string& HostName();

const char* GetFQDNHostName();
const std::string& FQDNHostName();
bool IsFQDN(const std::string& name);
