# Adapt build-tree copies only; never rewrite the imported source checkout.
set(YDB_SDK_MINGW_COMPAT_DIR "${YDB_SDK_BINARY_DIR}/mingw-compat")
set(_mingw_adapted_files
  util/system/compat.h util/system/compat.cpp util/system/yassert.h
  util/system/cpu_id.cpp util/system/thread.cpp
  util/datetime/systime.h util/datetime/systime.cpp util/folder/dirent_win.c)

# Keep quoted includes within the three directories that need adaptation.
file(GLOB_RECURSE _mingw_util_files CONFIGURE_DEPENDS
  RELATIVE "${YDB_SDK_SOURCE_DIR}"
  "${YDB_SDK_SOURCE_DIR}/util/system/*"
  "${YDB_SDK_SOURCE_DIR}/util/datetime/*"
  "${YDB_SDK_SOURCE_DIR}/util/folder/*")
foreach(_path IN LISTS _mingw_util_files)
  if(NOT _path IN_LIST _mingw_adapted_files)
    configure_file("${YDB_SDK_SOURCE_DIR}/${_path}"
      "${YDB_SDK_MINGW_COMPAT_DIR}/${_path}" COPYONLY)
  endif()
endforeach()

macro(_ydb_sdk_mingw_replace Before After)
  string(FIND "${_text}" "${Before}" _match)
  if(_match EQUAL -1)
    message(FATAL_ERROR "MinGW adaptation no longer matches ${_path}; review the imported source")
  endif()
  string(REPLACE "${Before}" "${After}" _text "${_text}")
endmacro()

foreach(_path IN LISTS _mingw_adapted_files)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${YDB_SDK_SOURCE_DIR}/${_path}")
  file(READ "${YDB_SDK_SOURCE_DIR}/${_path}" _text)
  string(REPLACE "\r\n" "\n" _text "${_text}")
  if(_path STREQUAL "util/system/compat.h")
    _ydb_sdk_mingw_replace("#if defined(_unix_)" "#if defined(_unix_) || defined(__MINGW32__)")
    _ydb_sdk_mingw_replace("    using pid_t = int;" "")
    _ydb_sdk_mingw_replace("#if defined(_MSC_VER)" "#if defined(_win_)")
    _ydb_sdk_mingw_replace("#ifdef _win_\n\n    #ifdef sleep" "#if defined(_win_) && !defined(__MINGW32__)\n\n    #ifdef sleep")
    _ydb_sdk_mingw_replace("#ifdef _win_\nint ftruncate" "#if defined(_win_) && !defined(__MINGW32__)\nint ftruncate")
  elseif(_path STREQUAL "util/system/compat.cpp")
    _ydb_sdk_mingw_replace("#ifdef _win_\n\nvoid sleep" "#if defined(_win_) && !defined(__MINGW32__)\n\nvoid sleep")
  elseif(_path STREQUAL "util/system/yassert.h")
    _ydb_sdk_mingw_replace("#if !defined(_MSC_VER)" "#if !defined(_MSC_VER) && !defined(__MINGW32__)")
  elseif(_path STREQUAL "util/system/thread.cpp")
    _ydb_sdk_mingw_replace("#elif defined(_MSC_VER)" "#elif defined(_win_)")
  elseif(_path STREQUAL "util/system/cpu_id.cpp")
    _ydb_sdk_mingw_replace("#if defined(_x86_) && !defined(_win_)" "#if defined(_x86_)")
    _ydb_sdk_mingw_replace("_xgetbv(" "YdbSdkReadXcr(")
    _ydb_sdk_mingw_replace("#if defined(_MSC_VER)" "#if defined(_win_)")
  elseif(_path STREQUAL "util/datetime/systime.h")
    _ydb_sdk_mingw_replace("int gettimeofday(struct timeval* tp, void*);" "#include <sys/time.h>")
  elseif(_path STREQUAL "util/datetime/systime.cpp")
    _ydb_sdk_mingw_replace("int gettimeofday(timeval* tp, void*) {" "#ifndef __MINGW32__\nint gettimeofday(timeval* tp, void*) {")
    _ydb_sdk_mingw_replace("tm* localtime_r(" "#endif\n\ntm* localtime_r(")
  elseif(_path STREQUAL "util/folder/dirent_win.c")
    _ydb_sdk_mingw_replace("    #include <stdio.h>" "    #include <stdio.h>\n    #include <errno.h>")
    _ydb_sdk_mingw_replace("    __acrt_errno_map_os_error(GetLastError());" [=[    // MinGW does not export the private UCRT error mapper.
    switch (GetLastError()) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_INVALID_DRIVE:
        case ERROR_BAD_NETPATH:
        case ERROR_BAD_NET_NAME: errno = ENOENT; break;
        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
        case ERROR_LOCK_VIOLATION: errno = EACCES; break;
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY: errno = ENOMEM; break;
        case ERROR_TOO_MANY_OPEN_FILES: errno = EMFILE; break;
        case ERROR_DIRECTORY: errno = ENOTDIR; break;
        case ERROR_FILENAME_EXCED_RANGE: errno = ENAMETOOLONG; break;
        default: errno = EINVAL; break;
    }]=])
  endif()
  set(_output "${YDB_SDK_MINGW_COMPAT_DIR}/${_path}")
  set(_previous "")
  if(EXISTS "${_output}")
    file(READ "${_output}" _previous)
  endif()
  if(NOT _previous STREQUAL _text)
    get_filename_component(_directory "${_output}" DIRECTORY)
    file(MAKE_DIRECTORY "${_directory}")
    file(WRITE "${_output}" "${_text}")
  endif()
endforeach()
