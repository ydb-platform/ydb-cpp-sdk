
# This file was generated by the build system used internally in the Yandex monorepo.
# Only simple modifications are allowed (adding source-files to targets, adding simple properties
# like target_include_directories). These modifications will be ported to original
# ya.make files by maintainers. Any complex modifications which can't be ported back to the
# original buildsystem will not be accepted.


if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  set(YASM_FLAGS -f elf64 -D UNIX -D _x86_64_ -D_YASM_ -g dwarf2)
  set(RAGEL_FLAGS -L -I ${YDB_SDK_SOURCE_DIR}/)
endif()

<<<<<<< HEAD
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
=======
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" AND NOT HAVE_CUDA)
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
  set(RAGEL_FLAGS -L -I ${YDB_SDK_SOURCE_DIR}/)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  set(YASM_FLAGS -f macho64 -D DARWIN -D UNIX -D _x86_64_ -D_YASM_)
  set(RAGEL_FLAGS -L -I ${YDB_SDK_SOURCE_DIR}/)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
  set(RAGEL_FLAGS -L -I ${YDB_SDK_SOURCE_DIR}/)
endif()

if(WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  set(YASM_FLAGS -f win64 -D WIN64 -D _x86_64_ -D_YASM_)
  set(RAGEL_FLAGS -L -I ${YDB_SDK_SOURCE_DIR}/)
endif()

