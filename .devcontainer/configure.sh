#!/bin/sh
WORKSPACE_FOLDER=$1

cd $1
mkdir -p build
git submodule update --init --recursive
ccache -o cache_dir=/root/.ccache
cmake --preset release-test-with-ccache-basedir -DCMAKE_EXPORT_COMPILE_COMMANDS=1
