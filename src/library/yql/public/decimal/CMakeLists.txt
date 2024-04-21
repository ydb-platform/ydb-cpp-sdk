#add_subdirectory(ut)
add_library(yql-public-decimal)

target_link_libraries(yql-public-decimal PUBLIC
  yutil
)

target_sources(yql-public-decimal PRIVATE
  yql_decimal.cpp
  yql_decimal_serialize.cpp
)


