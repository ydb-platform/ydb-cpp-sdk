#!/bin/sh
WORKSPACE_FOLDER=$1

cd $1
mkdir -p build
git submodule update --init --recursive
cmake --preset release-test-with-ccache-basedir -DCMAKE_EXPORT_COMPILE_COMMANDS=1
