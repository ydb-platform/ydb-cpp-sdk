#!/bin/bash

tmp_dir=$(mktemp -d)

echo "Copying sources..."

cp -r $1/ydb/public/sdk/cpp/* $tmp_dir
echo "tmp_dir: $tmp_dir"

rm -r $tmp_dir/client
rm -r $tmp_dir/src/client/cms
rm -r $tmp_dir/src/client/config
rm -r $tmp_dir/src/client/debug
rm -r $tmp_dir/src/client/draft

rm -r $tmp_dir/include/ydb-cpp-sdk/client/cms
rm -r $tmp_dir/include/ydb-cpp-sdk/client/config
rm -r $tmp_dir/include/ydb-cpp-sdk/client/debug
rm -r $tmp_dir/include/ydb-cpp-sdk/client/draft

rm -r $tmp_dir/tests/unit/client/draft

mkdir -p $tmp_dir/src/api/client/yc_private
mkdir -p $tmp_dir/src/api/client/yc_public

cp -r $1/ydb/public/api/client/yc_private/iam $tmp_dir/src/api/client/yc_private
cp -r $1/ydb/public/api/client/yc_private/operation $tmp_dir/src/api/client/yc_private
cp -r $1/ydb/public/api/client/yc_public/common $tmp_dir/src/api/client/yc_public
cp -r $1/ydb/public/api/client/yc_public/iam $tmp_dir/src/api/client/yc_public
cp -r $1/ydb/public/api/grpc $tmp_dir/src/api
cp -r $1/ydb/public/api/protos $tmp_dir/src/api

rm -r $tmp_dir/src/api/protos/out
rm $tmp_dir/include/ydb-cpp-sdk/type_switcher.h $tmp_dir/src/version.h

cp -r $2/util $tmp_dir
cp -r $2/library $tmp_dir

cp -r $2/.devcontainer $tmp_dir
cp -r $2/.git $tmp_dir
cp -r $2/.github $tmp_dir
cp -r $2/contrib $tmp_dir
cp -r $2/cmake $tmp_dir
cp -r $2/scripts $tmp_dir
cp -r $2/third_party $tmp_dir
cp -r $2/tools $tmp_dir

cp $2/.gitignore $tmp_dir
cp $2/.gitmodules $tmp_dir
cp $2/CMakePresets.json $tmp_dir
cp $2/CMakeLists.txt $tmp_dir
cp $2/LICENSE $tmp_dir
cp $2/README.md $tmp_dir

cp $2/include/ydb-cpp-sdk/type_switcher.h $tmp_dir/include/ydb-cpp-sdk/type_switcher.h
cp $2/src/version.h $tmp_dir/src/version.h

cd $2

find src/ include/ tests/ examples/ -type f -name "CMakeLists.txt" | while read f;
do
    mkdir -p "$(dirname "$tmp_dir/$f")" && cp -p $f $tmp_dir/$f
done

cd -

echo "Copying completed"
echo "Patching sources..."

rm -rf $tmp_dir/adapters $tmp_dir/client

SED_INCLUDE='(^\s*#include\s*)(<|\")'

find $tmp_dir -type f -regex ".*\(h\|cpp\|cpp.in\|c\|ipp\|jnj\|rl6\|h.txt\|proto\)$" | while read f;
do
    sed -i -E \
        's/'$SED_INCLUDE'ydb\/public\/api\//\1\2src\/api\//g;
         s/(^\s*import\s*)(\")ydb\/public\/api\//\1\2src\/api\//g;
         s/'$SED_INCLUDE'ydb\/public\/sdk\/cpp\/include\//\1\2/g;
         s/'$SED_INCLUDE'ydb\/public\/sdk\/cpp\//\1\2/g;
         s/'$SED_INCLUDE'library\/cpp\/retry\/retry_policy\.h/\1\2ydb-cpp-sdk\/library\/retry\/retry_policy\.h/g;
         s/'$SED_INCLUDE'library\/cpp\/string_utils\/base64\/base64\.h/\1\2src\/library\/string_utils\/base64\/base64\.h/g;
         s/(inline\s*)Dev/\1V3/g;
         s/(inline\s*namespace\s*)Dev/\1V3/g;' $f
done

echo "Patching completed"
echo "RSync..."

rsync -c -I -W -r --delete --filter '- **/ya.make' --filter '- sdk_common.inc' $tmp_dir/ $2
rm -rf $tmp_dir

echo "RSync completed"
