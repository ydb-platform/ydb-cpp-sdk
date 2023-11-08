// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.26.0
// 	protoc        v3.19.0
// source: ydb/library/yql/providers/generic/connector/app/config/server.proto

package config

import (
	common "github.com/ydb-platform/ydb/ydb/library/yql/providers/generic/connector/api/common"
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

// ELogLevel enumerates standard levels of logging
type ELogLevel int32

const (
	ELogLevel_TRACE ELogLevel = 0
	ELogLevel_DEBUG ELogLevel = 1
	ELogLevel_INFO  ELogLevel = 2
	ELogLevel_WARN  ELogLevel = 3
	ELogLevel_ERROR ELogLevel = 4
	ELogLevel_FATAL ELogLevel = 5
)

// Enum value maps for ELogLevel.
var (
	ELogLevel_name = map[int32]string{
		0: "TRACE",
		1: "DEBUG",
		2: "INFO",
		3: "WARN",
		4: "ERROR",
		5: "FATAL",
	}
	ELogLevel_value = map[string]int32{
		"TRACE": 0,
		"DEBUG": 1,
		"INFO":  2,
		"WARN":  3,
		"ERROR": 4,
		"FATAL": 5,
	}
)

func (x ELogLevel) Enum() *ELogLevel {
	p := new(ELogLevel)
	*p = x
	return p
}

func (x ELogLevel) String() string {
	return protoimpl.X.EnumStringOf(x.Descriptor(), protoreflect.EnumNumber(x))
}

func (ELogLevel) Descriptor() protoreflect.EnumDescriptor {
	return file_ydb_library_yql_providers_generic_connector_app_config_server_proto_enumTypes[0].Descriptor()
}

func (ELogLevel) Type() protoreflect.EnumType {
	return &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_enumTypes[0]
}

func (x ELogLevel) Number() protoreflect.EnumNumber {
	return protoreflect.EnumNumber(x)
}

// Deprecated: Use ELogLevel.Descriptor instead.
func (ELogLevel) EnumDescriptor() ([]byte, []int) {
	return file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescGZIP(), []int{0}
}

// Connector server configuration
type TServerConfig struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	// TODO: remove it after YQ-2057
	//
	// Deprecated: Do not use.
	Endpoint *common.TEndpoint `protobuf:"bytes,1,opt,name=endpoint,proto3" json:"endpoint,omitempty"`
	// Deprecated: Do not use.
	Tls             *TServerTLSConfig       `protobuf:"bytes,2,opt,name=tls,proto3" json:"tls,omitempty"`
	ConnectorServer *TConnectorServerConfig `protobuf:"bytes,5,opt,name=connector_server,json=connectorServer,proto3" json:"connector_server,omitempty"`
	// This is a rough restriction for YQ memory consumption until
	// https://st.yandex-team.ru/YQ-2057 is implemented.
	// Leave it empty if you want to avoid any memory limits.
	ReadLimit *TServerReadLimit `protobuf:"bytes,3,opt,name=read_limit,json=readLimit,proto3" json:"read_limit,omitempty"`
	// Logger config
	Logger *TLoggerConfig `protobuf:"bytes,4,opt,name=logger,proto3" json:"logger,omitempty"`
	// Go runtime profiler.
	// Disabled if this part of config is empty.
	PprofServer *TPprofServerConfig `protobuf:"bytes,6,opt,name=pprof_server,json=pprofServer,proto3" json:"pprof_server,omitempty"`
}

func (x *TServerConfig) Reset() {
	*x = TServerConfig{}
	if protoimpl.UnsafeEnabled {
		mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *TServerConfig) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*TServerConfig) ProtoMessage() {}

func (x *TServerConfig) ProtoReflect() protoreflect.Message {
	mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use TServerConfig.ProtoReflect.Descriptor instead.
func (*TServerConfig) Descriptor() ([]byte, []int) {
	return file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescGZIP(), []int{0}
}

// Deprecated: Do not use.
func (x *TServerConfig) GetEndpoint() *common.TEndpoint {
	if x != nil {
		return x.Endpoint
	}
	return nil
}

// Deprecated: Do not use.
func (x *TServerConfig) GetTls() *TServerTLSConfig {
	if x != nil {
		return x.Tls
	}
	return nil
}

func (x *TServerConfig) GetConnectorServer() *TConnectorServerConfig {
	if x != nil {
		return x.ConnectorServer
	}
	return nil
}

func (x *TServerConfig) GetReadLimit() *TServerReadLimit {
	if x != nil {
		return x.ReadLimit
	}
	return nil
}

func (x *TServerConfig) GetLogger() *TLoggerConfig {
	if x != nil {
		return x.Logger
	}
	return nil
}

func (x *TServerConfig) GetPprofServer() *TPprofServerConfig {
	if x != nil {
		return x.PprofServer
	}
	return nil
}

// TConnectorServerConfig - configuration of the main GRPC server
type TConnectorServerConfig struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	// Network address server will be listening on
	Endpoint *common.TEndpoint `protobuf:"bytes,1,opt,name=endpoint,proto3" json:"endpoint,omitempty"`
	// TLS settings.
	// Leave it empty for insecure connections.
	Tls *TServerTLSConfig `protobuf:"bytes,2,opt,name=tls,proto3" json:"tls,omitempty"`
}

func (x *TConnectorServerConfig) Reset() {
	*x = TConnectorServerConfig{}
	if protoimpl.UnsafeEnabled {
		mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[1]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *TConnectorServerConfig) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*TConnectorServerConfig) ProtoMessage() {}

func (x *TConnectorServerConfig) ProtoReflect() protoreflect.Message {
	mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[1]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use TConnectorServerConfig.ProtoReflect.Descriptor instead.
func (*TConnectorServerConfig) Descriptor() ([]byte, []int) {
	return file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescGZIP(), []int{1}
}

func (x *TConnectorServerConfig) GetEndpoint() *common.TEndpoint {
	if x != nil {
		return x.Endpoint
	}
	return nil
}

func (x *TConnectorServerConfig) GetTls() *TServerTLSConfig {
	if x != nil {
		return x.Tls
	}
	return nil
}

type TServerTLSConfig struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	// TLS private key path
	Key string `protobuf:"bytes,2,opt,name=key,proto3" json:"key,omitempty"`
	// TLS public cert path
	Cert string `protobuf:"bytes,3,opt,name=cert,proto3" json:"cert,omitempty"`
}

func (x *TServerTLSConfig) Reset() {
	*x = TServerTLSConfig{}
	if protoimpl.UnsafeEnabled {
		mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[2]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *TServerTLSConfig) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*TServerTLSConfig) ProtoMessage() {}

func (x *TServerTLSConfig) ProtoReflect() protoreflect.Message {
	mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[2]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use TServerTLSConfig.ProtoReflect.Descriptor instead.
func (*TServerTLSConfig) Descriptor() ([]byte, []int) {
	return file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescGZIP(), []int{2}
}

func (x *TServerTLSConfig) GetKey() string {
	if x != nil {
		return x.Key
	}
	return ""
}

func (x *TServerTLSConfig) GetCert() string {
	if x != nil {
		return x.Cert
	}
	return ""
}

// ServerReadLimit limitates the amount of data extracted from the data source on every read request.
type TServerReadLimit struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	// The number of rows extracted from the data source
	Rows uint64 `protobuf:"varint,1,opt,name=rows,proto3" json:"rows,omitempty"`
}

func (x *TServerReadLimit) Reset() {
	*x = TServerReadLimit{}
	if protoimpl.UnsafeEnabled {
		mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[3]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *TServerReadLimit) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*TServerReadLimit) ProtoMessage() {}

func (x *TServerReadLimit) ProtoReflect() protoreflect.Message {
	mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[3]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use TServerReadLimit.ProtoReflect.Descriptor instead.
func (*TServerReadLimit) Descriptor() ([]byte, []int) {
	return file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescGZIP(), []int{3}
}

func (x *TServerReadLimit) GetRows() uint64 {
	if x != nil {
		return x.Rows
	}
	return 0
}

// TLogger represents logger configuration
type TLoggerConfig struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	// Level of logging
	LogLevel ELogLevel `protobuf:"varint,1,opt,name=log_level,json=logLevel,proto3,enum=NYql.Connector.App.Config.ELogLevel" json:"log_level,omitempty"`
	// Is logging of queries enabled
	EnableSqlQueryLogging bool `protobuf:"varint,2,opt,name=enable_sql_query_logging,json=enableSqlQueryLogging,proto3" json:"enable_sql_query_logging,omitempty"`
}

func (x *TLoggerConfig) Reset() {
	*x = TLoggerConfig{}
	if protoimpl.UnsafeEnabled {
		mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[4]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *TLoggerConfig) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*TLoggerConfig) ProtoMessage() {}

func (x *TLoggerConfig) ProtoReflect() protoreflect.Message {
	mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[4]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use TLoggerConfig.ProtoReflect.Descriptor instead.
func (*TLoggerConfig) Descriptor() ([]byte, []int) {
	return file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescGZIP(), []int{4}
}

func (x *TLoggerConfig) GetLogLevel() ELogLevel {
	if x != nil {
		return x.LogLevel
	}
	return ELogLevel_TRACE
}

func (x *TLoggerConfig) GetEnableSqlQueryLogging() bool {
	if x != nil {
		return x.EnableSqlQueryLogging
	}
	return false
}

// TPprofServerConfig configures HTTP server delivering Go runtime profiler data
type TPprofServerConfig struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	// Network address server will be listening on
	Endpoint *common.TEndpoint `protobuf:"bytes,1,opt,name=endpoint,proto3" json:"endpoint,omitempty"`
	// TLS settings.
	// Leave it empty for insecure connections.
	Tls *TServerTLSConfig `protobuf:"bytes,2,opt,name=tls,proto3" json:"tls,omitempty"`
}

func (x *TPprofServerConfig) Reset() {
	*x = TPprofServerConfig{}
	if protoimpl.UnsafeEnabled {
		mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[5]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *TPprofServerConfig) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*TPprofServerConfig) ProtoMessage() {}

func (x *TPprofServerConfig) ProtoReflect() protoreflect.Message {
	mi := &file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[5]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use TPprofServerConfig.ProtoReflect.Descriptor instead.
func (*TPprofServerConfig) Descriptor() ([]byte, []int) {
	return file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescGZIP(), []int{5}
}

func (x *TPprofServerConfig) GetEndpoint() *common.TEndpoint {
	if x != nil {
		return x.Endpoint
	}
	return nil
}

func (x *TPprofServerConfig) GetTls() *TServerTLSConfig {
	if x != nil {
		return x.Tls
	}
	return nil
}

var File_ydb_library_yql_providers_generic_connector_app_config_server_proto protoreflect.FileDescriptor

var file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDesc = []byte{
	0x0a, 0x43, 0x79, 0x64, 0x62, 0x2f, 0x6c, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79, 0x2f, 0x79, 0x71,
	0x6c, 0x2f, 0x70, 0x72, 0x6f, 0x76, 0x69, 0x64, 0x65, 0x72, 0x73, 0x2f, 0x67, 0x65, 0x6e, 0x65,
	0x72, 0x69, 0x63, 0x2f, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x2f, 0x61, 0x70,
	0x70, 0x2f, 0x63, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x2f, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x2e,
	0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x19, 0x4e, 0x59, 0x71, 0x6c, 0x2e, 0x43, 0x6f, 0x6e, 0x6e,
	0x65, 0x63, 0x74, 0x6f, 0x72, 0x2e, 0x41, 0x70, 0x70, 0x2e, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67,
	0x1a, 0x45, 0x79, 0x64, 0x62, 0x2f, 0x6c, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79, 0x2f, 0x79, 0x71,
	0x6c, 0x2f, 0x70, 0x72, 0x6f, 0x76, 0x69, 0x64, 0x65, 0x72, 0x73, 0x2f, 0x67, 0x65, 0x6e, 0x65,
	0x72, 0x69, 0x63, 0x2f, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x2f, 0x61, 0x70,
	0x69, 0x2f, 0x63, 0x6f, 0x6d, 0x6d, 0x6f, 0x6e, 0x2f, 0x65, 0x6e, 0x64, 0x70, 0x6f, 0x69, 0x6e,
	0x74, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x22, 0xd1, 0x03, 0x0a, 0x0d, 0x54, 0x53, 0x65, 0x72,
	0x76, 0x65, 0x72, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x12, 0x3f, 0x0a, 0x08, 0x65, 0x6e, 0x64,
	0x70, 0x6f, 0x69, 0x6e, 0x74, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x1f, 0x2e, 0x4e, 0x59,
	0x71, 0x6c, 0x2e, 0x4e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x2e, 0x4e, 0x41,
	0x70, 0x69, 0x2e, 0x54, 0x45, 0x6e, 0x64, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x42, 0x02, 0x18, 0x01,
	0x52, 0x08, 0x65, 0x6e, 0x64, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x12, 0x41, 0x0a, 0x03, 0x74, 0x6c,
	0x73, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x2b, 0x2e, 0x4e, 0x59, 0x71, 0x6c, 0x2e, 0x43,
	0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x2e, 0x41, 0x70, 0x70, 0x2e, 0x43, 0x6f, 0x6e,
	0x66, 0x69, 0x67, 0x2e, 0x54, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x54, 0x4c, 0x53, 0x43, 0x6f,
	0x6e, 0x66, 0x69, 0x67, 0x42, 0x02, 0x18, 0x01, 0x52, 0x03, 0x74, 0x6c, 0x73, 0x12, 0x5c, 0x0a,
	0x10, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x65, 0x72, 0x76, 0x65,
	0x72, 0x18, 0x05, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x31, 0x2e, 0x4e, 0x59, 0x71, 0x6c, 0x2e, 0x43,
	0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x2e, 0x41, 0x70, 0x70, 0x2e, 0x43, 0x6f, 0x6e,
	0x66, 0x69, 0x67, 0x2e, 0x54, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x53, 0x65,
	0x72, 0x76, 0x65, 0x72, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x52, 0x0f, 0x63, 0x6f, 0x6e, 0x6e,
	0x65, 0x63, 0x74, 0x6f, 0x72, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x12, 0x4a, 0x0a, 0x0a, 0x72,
	0x65, 0x61, 0x64, 0x5f, 0x6c, 0x69, 0x6d, 0x69, 0x74, 0x18, 0x03, 0x20, 0x01, 0x28, 0x0b, 0x32,
	0x2b, 0x2e, 0x4e, 0x59, 0x71, 0x6c, 0x2e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72,
	0x2e, 0x41, 0x70, 0x70, 0x2e, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x2e, 0x54, 0x53, 0x65, 0x72,
	0x76, 0x65, 0x72, 0x52, 0x65, 0x61, 0x64, 0x4c, 0x69, 0x6d, 0x69, 0x74, 0x52, 0x09, 0x72, 0x65,
	0x61, 0x64, 0x4c, 0x69, 0x6d, 0x69, 0x74, 0x12, 0x40, 0x0a, 0x06, 0x6c, 0x6f, 0x67, 0x67, 0x65,
	0x72, 0x18, 0x04, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x28, 0x2e, 0x4e, 0x59, 0x71, 0x6c, 0x2e, 0x43,
	0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x2e, 0x41, 0x70, 0x70, 0x2e, 0x43, 0x6f, 0x6e,
	0x66, 0x69, 0x67, 0x2e, 0x54, 0x4c, 0x6f, 0x67, 0x67, 0x65, 0x72, 0x43, 0x6f, 0x6e, 0x66, 0x69,
	0x67, 0x52, 0x06, 0x6c, 0x6f, 0x67, 0x67, 0x65, 0x72, 0x12, 0x50, 0x0a, 0x0c, 0x70, 0x70, 0x72,
	0x6f, 0x66, 0x5f, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x18, 0x06, 0x20, 0x01, 0x28, 0x0b, 0x32,
	0x2d, 0x2e, 0x4e, 0x59, 0x71, 0x6c, 0x2e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72,
	0x2e, 0x41, 0x70, 0x70, 0x2e, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x2e, 0x54, 0x50, 0x70, 0x72,
	0x6f, 0x66, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x52, 0x0b,
	0x70, 0x70, 0x72, 0x6f, 0x66, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x22, 0x94, 0x01, 0x0a, 0x16,
	0x54, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
	0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x12, 0x3b, 0x0a, 0x08, 0x65, 0x6e, 0x64, 0x70, 0x6f, 0x69,
	0x6e, 0x74, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x1f, 0x2e, 0x4e, 0x59, 0x71, 0x6c, 0x2e,
	0x4e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x2e, 0x4e, 0x41, 0x70, 0x69, 0x2e,
	0x54, 0x45, 0x6e, 0x64, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x52, 0x08, 0x65, 0x6e, 0x64, 0x70, 0x6f,
	0x69, 0x6e, 0x74, 0x12, 0x3d, 0x0a, 0x03, 0x74, 0x6c, 0x73, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0b,
	0x32, 0x2b, 0x2e, 0x4e, 0x59, 0x71, 0x6c, 0x2e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f,
	0x72, 0x2e, 0x41, 0x70, 0x70, 0x2e, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x2e, 0x54, 0x53, 0x65,
	0x72, 0x76, 0x65, 0x72, 0x54, 0x4c, 0x53, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x52, 0x03, 0x74,
	0x6c, 0x73, 0x22, 0x3e, 0x0a, 0x10, 0x54, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x54, 0x4c, 0x53,
	0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x12, 0x10, 0x0a, 0x03, 0x6b, 0x65, 0x79, 0x18, 0x02, 0x20,
	0x01, 0x28, 0x09, 0x52, 0x03, 0x6b, 0x65, 0x79, 0x12, 0x12, 0x0a, 0x04, 0x63, 0x65, 0x72, 0x74,
	0x18, 0x03, 0x20, 0x01, 0x28, 0x09, 0x52, 0x04, 0x63, 0x65, 0x72, 0x74, 0x4a, 0x04, 0x08, 0x01,
	0x10, 0x02, 0x22, 0x26, 0x0a, 0x10, 0x54, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x52, 0x65, 0x61,
	0x64, 0x4c, 0x69, 0x6d, 0x69, 0x74, 0x12, 0x12, 0x0a, 0x04, 0x72, 0x6f, 0x77, 0x73, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x04, 0x52, 0x04, 0x72, 0x6f, 0x77, 0x73, 0x22, 0x8b, 0x01, 0x0a, 0x0d, 0x54,
	0x4c, 0x6f, 0x67, 0x67, 0x65, 0x72, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x12, 0x41, 0x0a, 0x09,
	0x6c, 0x6f, 0x67, 0x5f, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0e, 0x32,
	0x24, 0x2e, 0x4e, 0x59, 0x71, 0x6c, 0x2e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72,
	0x2e, 0x41, 0x70, 0x70, 0x2e, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x2e, 0x45, 0x4c, 0x6f, 0x67,
	0x4c, 0x65, 0x76, 0x65, 0x6c, 0x52, 0x08, 0x6c, 0x6f, 0x67, 0x4c, 0x65, 0x76, 0x65, 0x6c, 0x12,
	0x37, 0x0a, 0x18, 0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x5f, 0x73, 0x71, 0x6c, 0x5f, 0x71, 0x75,
	0x65, 0x72, 0x79, 0x5f, 0x6c, 0x6f, 0x67, 0x67, 0x69, 0x6e, 0x67, 0x18, 0x02, 0x20, 0x01, 0x28,
	0x08, 0x52, 0x15, 0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x53, 0x71, 0x6c, 0x51, 0x75, 0x65, 0x72,
	0x79, 0x4c, 0x6f, 0x67, 0x67, 0x69, 0x6e, 0x67, 0x22, 0x90, 0x01, 0x0a, 0x12, 0x54, 0x50, 0x70,
	0x72, 0x6f, 0x66, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x12,
	0x3b, 0x0a, 0x08, 0x65, 0x6e, 0x64, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x18, 0x01, 0x20, 0x01, 0x28,
	0x0b, 0x32, 0x1f, 0x2e, 0x4e, 0x59, 0x71, 0x6c, 0x2e, 0x4e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63,
	0x74, 0x6f, 0x72, 0x2e, 0x4e, 0x41, 0x70, 0x69, 0x2e, 0x54, 0x45, 0x6e, 0x64, 0x70, 0x6f, 0x69,
	0x6e, 0x74, 0x52, 0x08, 0x65, 0x6e, 0x64, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x12, 0x3d, 0x0a, 0x03,
	0x74, 0x6c, 0x73, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0b, 0x32, 0x2b, 0x2e, 0x4e, 0x59, 0x71, 0x6c,
	0x2e, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x2e, 0x41, 0x70, 0x70, 0x2e, 0x43,
	0x6f, 0x6e, 0x66, 0x69, 0x67, 0x2e, 0x54, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x54, 0x4c, 0x53,
	0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x52, 0x03, 0x74, 0x6c, 0x73, 0x2a, 0x4b, 0x0a, 0x09, 0x45,
	0x4c, 0x6f, 0x67, 0x4c, 0x65, 0x76, 0x65, 0x6c, 0x12, 0x09, 0x0a, 0x05, 0x54, 0x52, 0x41, 0x43,
	0x45, 0x10, 0x00, 0x12, 0x09, 0x0a, 0x05, 0x44, 0x45, 0x42, 0x55, 0x47, 0x10, 0x01, 0x12, 0x08,
	0x0a, 0x04, 0x49, 0x4e, 0x46, 0x4f, 0x10, 0x02, 0x12, 0x08, 0x0a, 0x04, 0x57, 0x41, 0x52, 0x4e,
	0x10, 0x03, 0x12, 0x09, 0x0a, 0x05, 0x45, 0x52, 0x52, 0x4f, 0x52, 0x10, 0x04, 0x12, 0x09, 0x0a,
	0x05, 0x46, 0x41, 0x54, 0x41, 0x4c, 0x10, 0x05, 0x42, 0x49, 0x5a, 0x47, 0x61, 0x2e, 0x79, 0x61,
	0x6e, 0x64, 0x65, 0x78, 0x2d, 0x74, 0x65, 0x61, 0x6d, 0x2e, 0x72, 0x75, 0x2f, 0x79, 0x64, 0x62,
	0x2f, 0x6c, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79, 0x2f, 0x79, 0x71, 0x6c, 0x2f, 0x70, 0x72, 0x6f,
	0x76, 0x69, 0x64, 0x65, 0x72, 0x73, 0x2f, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x2f, 0x63,
	0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x2f, 0x61, 0x70, 0x70, 0x2f, 0x63, 0x6f, 0x6e,
	0x66, 0x69, 0x67, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescOnce sync.Once
	file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescData = file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDesc
)

func file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescGZIP() []byte {
	file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescOnce.Do(func() {
		file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescData = protoimpl.X.CompressGZIP(file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescData)
	})
	return file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDescData
}

var file_ydb_library_yql_providers_generic_connector_app_config_server_proto_enumTypes = make([]protoimpl.EnumInfo, 1)
var file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes = make([]protoimpl.MessageInfo, 6)
var file_ydb_library_yql_providers_generic_connector_app_config_server_proto_goTypes = []interface{}{
	(ELogLevel)(0),                 // 0: NYql.Connector.App.Config.ELogLevel
	(*TServerConfig)(nil),          // 1: NYql.Connector.App.Config.TServerConfig
	(*TConnectorServerConfig)(nil), // 2: NYql.Connector.App.Config.TConnectorServerConfig
	(*TServerTLSConfig)(nil),       // 3: NYql.Connector.App.Config.TServerTLSConfig
	(*TServerReadLimit)(nil),       // 4: NYql.Connector.App.Config.TServerReadLimit
	(*TLoggerConfig)(nil),          // 5: NYql.Connector.App.Config.TLoggerConfig
	(*TPprofServerConfig)(nil),     // 6: NYql.Connector.App.Config.TPprofServerConfig
	(*common.TEndpoint)(nil),       // 7: NYql.NConnector.NApi.TEndpoint
}
var file_ydb_library_yql_providers_generic_connector_app_config_server_proto_depIdxs = []int32{
	7,  // 0: NYql.Connector.App.Config.TServerConfig.endpoint:type_name -> NYql.NConnector.NApi.TEndpoint
	3,  // 1: NYql.Connector.App.Config.TServerConfig.tls:type_name -> NYql.Connector.App.Config.TServerTLSConfig
	2,  // 2: NYql.Connector.App.Config.TServerConfig.connector_server:type_name -> NYql.Connector.App.Config.TConnectorServerConfig
	4,  // 3: NYql.Connector.App.Config.TServerConfig.read_limit:type_name -> NYql.Connector.App.Config.TServerReadLimit
	5,  // 4: NYql.Connector.App.Config.TServerConfig.logger:type_name -> NYql.Connector.App.Config.TLoggerConfig
	6,  // 5: NYql.Connector.App.Config.TServerConfig.pprof_server:type_name -> NYql.Connector.App.Config.TPprofServerConfig
	7,  // 6: NYql.Connector.App.Config.TConnectorServerConfig.endpoint:type_name -> NYql.NConnector.NApi.TEndpoint
	3,  // 7: NYql.Connector.App.Config.TConnectorServerConfig.tls:type_name -> NYql.Connector.App.Config.TServerTLSConfig
	0,  // 8: NYql.Connector.App.Config.TLoggerConfig.log_level:type_name -> NYql.Connector.App.Config.ELogLevel
	7,  // 9: NYql.Connector.App.Config.TPprofServerConfig.endpoint:type_name -> NYql.NConnector.NApi.TEndpoint
	3,  // 10: NYql.Connector.App.Config.TPprofServerConfig.tls:type_name -> NYql.Connector.App.Config.TServerTLSConfig
	11, // [11:11] is the sub-list for method output_type
	11, // [11:11] is the sub-list for method input_type
	11, // [11:11] is the sub-list for extension type_name
	11, // [11:11] is the sub-list for extension extendee
	0,  // [0:11] is the sub-list for field type_name
}

func init() { file_ydb_library_yql_providers_generic_connector_app_config_server_proto_init() }
func file_ydb_library_yql_providers_generic_connector_app_config_server_proto_init() {
	if File_ydb_library_yql_providers_generic_connector_app_config_server_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*TServerConfig); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[1].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*TConnectorServerConfig); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[2].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*TServerTLSConfig); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[3].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*TServerReadLimit); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[4].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*TLoggerConfig); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes[5].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*TPprofServerConfig); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDesc,
			NumEnums:      1,
			NumMessages:   6,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_ydb_library_yql_providers_generic_connector_app_config_server_proto_goTypes,
		DependencyIndexes: file_ydb_library_yql_providers_generic_connector_app_config_server_proto_depIdxs,
		EnumInfos:         file_ydb_library_yql_providers_generic_connector_app_config_server_proto_enumTypes,
		MessageInfos:      file_ydb_library_yql_providers_generic_connector_app_config_server_proto_msgTypes,
	}.Build()
	File_ydb_library_yql_providers_generic_connector_app_config_server_proto = out.File
	file_ydb_library_yql_providers_generic_connector_app_config_server_proto_rawDesc = nil
	file_ydb_library_yql_providers_generic_connector_app_config_server_proto_goTypes = nil
	file_ydb_library_yql_providers_generic_connector_app_config_server_proto_depIdxs = nil
}
