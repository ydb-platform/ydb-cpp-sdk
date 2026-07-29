#!/bin/bash
set -euo pipefail

tmp_dir=$(mktemp -d)

sync_upstream_tree() {
    local upstream_repo=$1
    local oss_repo=$2
    local destination_root=$3
    local tree=$4
    local mode=${5:-full}
    local previous_commit
    local current_commit
    local merge_dir
    local status
    local path
    local new_path
    local local_file
    local local_target
    local base_file
    local upstream_file
    local merged_file
    local conflicts=0

    previous_commit=$(cat "$oss_repo/.github/last_commit.txt")
    current_commit=$(git -C "$upstream_repo" rev-parse HEAD)
    merge_dir=$(mktemp -d "$tmp_dir/upstream-merge.XXXXXX")

    if ! git -C "$upstream_repo" merge-base --is-ancestor "$previous_commit" "$current_commit"; then
        echo "Cannot sync $tree: $previous_commit is not an ancestor of $current_commit" >&2
        return 1
    fi

    while IFS=$'\t' read -r status path new_path; do
        [ -n "$path" ] || continue

        # Standalone builds use CMake and deliberately do not import Arcadia build files.
        if [ "$(basename "$path")" = "ya.make" ] ||
            { [ -n "${new_path:-}" ] && [ "$(basename "$new_path")" = "ya.make" ]; }; then
            continue
        fi

        local_file="$destination_root/$path"
        base_file="$merge_dir/base"
        upstream_file="$merge_dir/upstream"
        merged_file="$merge_dir/merged"

        case "$status" in
            R*)
                local_target="$destination_root/$new_path"
                if [ ! -f "$local_file" ]; then
                    if [ "$mode" = "managed" ]; then
                        continue
                    fi
                    echo "Cannot rename upstream file missing locally: $path" >&2
                    conflicts=$((conflicts + 1))
                    continue
                fi
                if [ -e "$local_target" ] && [ "$local_target" != "$local_file" ]; then
                    echo "Cannot rename upstream file onto existing local file: $new_path" >&2
                    conflicts=$((conflicts + 1))
                    continue
                fi

                git -C "$upstream_repo" show "$previous_commit:$path" > "$base_file"
                git -C "$upstream_repo" show "$current_commit:$new_path" > "$upstream_file"
                mkdir -p "$(dirname "$local_target")"

                if cmp -s "$local_file" "$base_file"; then
                    cp "$upstream_file" "$local_target"
                elif git merge-file -p "$local_file" "$base_file" "$upstream_file" > "$merged_file"; then
                    cp "$merged_file" "$local_target"
                else
                    echo "Cannot merge renamed upstream and standalone changes: $path -> $new_path" >&2
                    conflicts=$((conflicts + 1))
                    continue
                fi

                if [ "$local_target" != "$local_file" ]; then
                    rm "$local_file"
                fi
                ;;
            A)
                if [ "$mode" = "managed" ] && [ ! -d "$(dirname "$local_file")" ]; then
                    continue
                fi
                if [ ! -e "$local_file" ]; then
                    mkdir -p "$(dirname "$local_file")"
                    git -C "$upstream_repo" show "$current_commit:$path" > "$local_file"
                else
                    git -C "$upstream_repo" show "$current_commit:$path" > "$upstream_file"
                    if ! cmp -s "$local_file" "$upstream_file"; then
                        echo "Cannot import added upstream file modified locally: $path" >&2
                        conflicts=$((conflicts + 1))
                    fi
                fi
                ;;
            D)
                if [ -e "$local_file" ]; then
                    git -C "$upstream_repo" show "$previous_commit:$path" > "$base_file"
                    if cmp -s "$local_file" "$base_file"; then
                        rm "$local_file"
                    else
                        echo "Cannot delete upstream file modified locally: $path" >&2
                        conflicts=$((conflicts + 1))
                    fi
                fi
                ;;
            M)
                if [ ! -f "$local_file" ]; then
                    if [ "$mode" = "managed" ]; then
                        continue
                    fi
                    echo "Cannot update upstream file missing locally: $path" >&2
                    conflicts=$((conflicts + 1))
                    continue
                fi

                git -C "$upstream_repo" show "$previous_commit:$path" > "$base_file"
                git -C "$upstream_repo" show "$current_commit:$path" > "$upstream_file"

                if cmp -s "$local_file" "$upstream_file"; then
                    continue
                fi

                if cmp -s "$local_file" "$base_file"; then
                    cp "$upstream_file" "$local_file"
                    continue
                fi

                if git merge-file -p "$local_file" "$base_file" "$upstream_file" > "$merged_file"; then
                    cp "$merged_file" "$local_file"
                else
                    echo "Cannot merge upstream and standalone changes: $path" >&2
                    conflicts=$((conflicts + 1))
                fi
                ;;
            *)
                echo "Unsupported upstream change '$status' for $path" >&2
                conflicts=$((conflicts + 1))
                ;;
        esac
    done < <(git -C "$upstream_repo" diff --name-status --find-renames "$previous_commit..$current_commit" -- "$tree")

    rm -rf "$merge_dir"

    if [ "$conflicts" -ne 0 ]; then
        echo "Failed to import $tree: $conflicts conflicting change(s)" >&2
        return 1
    fi
}

echo "Copying sources..."

cp -r "$1"/ydb/public/sdk/cpp/* "$tmp_dir"
echo "tmp_dir: $tmp_dir"

rm -r $tmp_dir/src/client/arrow
rm -r $tmp_dir/src/client/cms
rm -r $tmp_dir/src/client/config
rm -r $tmp_dir/src/client/debug
rm -r $tmp_dir/src/client/draft

rm -r $tmp_dir/include/ydb-cpp-sdk/client/arrow
rm -r $tmp_dir/include/ydb-cpp-sdk/client/cms
rm -r $tmp_dir/include/ydb-cpp-sdk/client/config
rm -r $tmp_dir/include/ydb-cpp-sdk/client/debug
rm -r $tmp_dir/include/ydb-cpp-sdk/client/draft

rm -r $tmp_dir/tests/unit/client/draft

mkdir -p $tmp_dir/src/api/client/yc_private
mkdir -p $tmp_dir/src/api/client/yc_private/accessservice
mkdir -p $tmp_dir/src/api/client/yc_public

cp -r $1/ydb/public/api/client/yc_private/accessservice/sensitive.proto $tmp_dir/src/api/client/yc_private/accessservice/sensitive.proto
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

sync_upstream_tree "$1" "$2" "$tmp_dir" util
sync_upstream_tree "$1" "$2" "$tmp_dir" library/cpp managed
sync_upstream_tree "$1" "$2" "$tmp_dir" contrib/libs/libc_compat managed
sync_upstream_tree "$1" "$2" "$tmp_dir" contrib/libs/lzmasdk managed
sync_upstream_tree "$1" "$2" "$tmp_dir" tools/enum_parser managed
sync_upstream_tree "$1" "$2" "$tmp_dir" tools/rescompiler managed

cp $2/.gitignore $tmp_dir
cp $2/.gitmodules $tmp_dir
cp $2/CMakePresets.json $tmp_dir
cp $2/CMakeLists.txt $tmp_dir
cp $2/LICENSE $tmp_dir
cp $2/README.md $tmp_dir
for oss_test_dir in slo_workloads deb_package; do
  if [ -d "$2/tests/$oss_test_dir" ]; then
    rm -rf "$tmp_dir/tests/$oss_test_dir"
    mkdir -p "$tmp_dir/tests"
    cp -a "$2/tests/$oss_test_dir" "$tmp_dir/tests/"
  fi
done

cp $2/include/ydb-cpp-sdk/type_switcher.h $tmp_dir/include/ydb-cpp-sdk/type_switcher.h
cp $2/include/ydb-cpp-sdk/stlfwd.h $tmp_dir/include/ydb-cpp-sdk/stlfwd.h
cp $2/src/version.h $tmp_dir/src/version.h

cd $2

find src/ include/ tests/ examples/ plugins/ -type f -name "CMakeLists.txt" | while read f;
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
