#pragma once

#include <ydb-cpp-sdk/library/yson/node/node.h>

namespace NYT {

namespace NYson {
struct IYsonConsumer;
} // namespace NYson

////////////////////////////////////////////////////////////////////////////////

void Serialize(const std::string& value, NYson::IYsonConsumer* consumer);
void Serialize(const std::string_view& value, NYson::IYsonConsumer* consumer);
void Serialize(const char* value, NYson::IYsonConsumer* consumer);
void Deserialize(std::string& value, const TNode& node);

void Serialize(signed char value, NYson::IYsonConsumer* consumer);
void Serialize(short value, NYson::IYsonConsumer* consumer);
void Serialize(int value, NYson::IYsonConsumer* consumer);
void Serialize(long value, NYson::IYsonConsumer* consumer);
void Serialize(long long value, NYson::IYsonConsumer* consumer);
void Deserialize(i64& value, const TNode& node);

void Serialize(unsigned char value, NYson::IYsonConsumer* consumer);
void Serialize(unsigned short value, NYson::IYsonConsumer* consumer);
void Serialize(unsigned int value, NYson::IYsonConsumer* consumer);
void Serialize(unsigned long value, NYson::IYsonConsumer* consumer);
void Serialize(unsigned long long value, NYson::IYsonConsumer* consumer);
void Deserialize(ui64& value, const TNode& node);

void Serialize(double value, NYson::IYsonConsumer* consumer);
void Deserialize(double& value, const TNode& node);

void Serialize(bool value, NYson::IYsonConsumer* consumer);
void Deserialize(bool& value, const TNode& node);

void Serialize(const TNode& node, NYson::IYsonConsumer* consumer);
void Deserialize(TNode& value, const TNode& node);

void Serialize(const std::unordered_map<std::string, std::string>& renameColumns, NYson::IYsonConsumer* consumer);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
