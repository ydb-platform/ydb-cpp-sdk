# Generated by devtools/yamaker from nixpkgs 22.11.

PROTO_LIBRARY()

LICENSE(Apache-2.0)

LICENSE_TEXTS(.yandex_meta/licenses.list.txt)

VERSION(1.61.0)

ORIGINAL_SOURCE(https://github.com/googleapis/python-api-common-protos/archive/v1.61.0.tar.gz)

PY_NAMESPACE(.)

PROTO_NAMESPACE(
    GLOBAL
    contrib/libs/googleapis-common-protos
)

GRPC()

SRCS(
    google/api/annotations.proto
    google/api/auth.proto
    google/api/backend.proto
    google/api/billing.proto
    google/api/client.proto
    google/api/config_change.proto
    google/api/consumer.proto
    google/api/context.proto
    google/api/control.proto
    google/api/distribution.proto
    google/api/documentation.proto
    google/api/endpoint.proto
    google/api/error_reason.proto
    google/api/field_behavior.proto
    google/api/field_info.proto
    google/api/http.proto
    google/api/httpbody.proto
    google/api/label.proto
    google/api/launch_stage.proto
    google/api/log.proto
    google/api/logging.proto
    google/api/metric.proto
    google/api/monitored_resource.proto
    google/api/monitoring.proto
    google/api/policy.proto
    google/api/quota.proto
    google/api/resource.proto
    google/api/routing.proto
    google/api/service.proto
    google/api/source_info.proto
    google/api/system_parameter.proto
    google/api/usage.proto
    google/api/visibility.proto
    google/cloud/extended_operations.proto
    google/cloud/location/locations.proto
    google/gapic/metadata/gapic_metadata.proto
    google/logging/type/http_request.proto
    google/logging/type/log_severity.proto
    google/longrunning/operations.proto
    google/rpc/code.proto
    google/rpc/context/attribute_context.proto
    google/rpc/context/audit_context.proto
    google/rpc/error_details.proto
    google/rpc/http.proto
    google/rpc/status.proto
    google/type/calendar_period.proto
    google/type/color.proto
    google/type/date.proto
    google/type/datetime.proto
    google/type/dayofweek.proto
    google/type/decimal.proto
    google/type/expr.proto
    google/type/fraction.proto
    google/type/interval.proto
    google/type/latlng.proto
    google/type/localized_text.proto
    google/type/money.proto
    google/type/month.proto
    google/type/phone_number.proto
    google/type/postal_address.proto
    google/type/quaternion.proto
    google/type/timeofday.proto
)

EXCLUDE_TAGS(GO_PROTO)

END()
