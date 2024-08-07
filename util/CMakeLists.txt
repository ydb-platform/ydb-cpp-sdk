add_subdirectory(charset)
add_subdirectory(draft)

if (YDB_SDK_TESTS)
  add_ydb_test(NAME util-digest-ut
    SOURCES
      digest/city_ut.cpp
      digest/fnv_ut.cpp
      digest/multi_ut.cpp
      digest/murmur_ut.cpp
      digest/sequence_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )
  add_ydb_test(NAME util-folder-ut
    SOURCES
      folder/dirut_ut.cpp
      folder/filelist_ut.cpp
      folder/fts_ut.cpp
      folder/iterator_ut.cpp
      folder/path_ut.cpp
      folder/pathsplit_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
      threading-future
    LABELS
      unit
  )
  add_ydb_test(NAME util-network-ut
    SOURCES
      network/address_ut.cpp
      network/endpoint_ut.cpp
      network/ip_ut.cpp
      network/poller_ut.cpp
      network/sock_ut.cpp
      network/socket_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
      threading-future
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(yutil)

target_compile_options(yutil PUBLIC
  -DTSTRING_IS_STD_STRING
)

target_compile_options(yutil PRIVATE
  -Wnarrowing
  -DNO_CUSTOM_CHAR_PTR_STD_COMPARATOR
)

target_link_libraries(yutil PUBLIC
  util-charset
  contrib-libs-libc_compat
  ZLIB::ZLIB
  double-conversion::double-conversion
)

target_joined_source(yutil
  all_datetime.cpp
  ${YDB_SDK_SOURCE_DIR}/util/datetime/base.cpp
  ${YDB_SDK_SOURCE_DIR}/util/datetime/constants.cpp
  ${YDB_SDK_SOURCE_DIR}/util/datetime/cputimer.cpp
  ${YDB_SDK_SOURCE_DIR}/util/datetime/systime.cpp
  ${YDB_SDK_SOURCE_DIR}/util/datetime/uptime.cpp
)

target_joined_source(yutil
  all_digest.cpp
  ${YDB_SDK_SOURCE_DIR}/util/digest/fnv.cpp
  ${YDB_SDK_SOURCE_DIR}/util/digest/multi.cpp
  ${YDB_SDK_SOURCE_DIR}/util/digest/murmur.cpp
  ${YDB_SDK_SOURCE_DIR}/util/digest/numeric.cpp
  ${YDB_SDK_SOURCE_DIR}/util/digest/sequence.cpp
)

target_joined_source(yutil
  all_util.cpp
  ${YDB_SDK_SOURCE_DIR}/util/ysafeptr.cpp
  ${YDB_SDK_SOURCE_DIR}/util/ysaveload.cpp
  ${YDB_SDK_SOURCE_DIR}/util/str_stl.cpp
)

target_joined_source(yutil
  all_folder.cpp
  ${YDB_SDK_SOURCE_DIR}/util/folder/dirut.cpp
  ${YDB_SDK_SOURCE_DIR}/util/folder/filelist.cpp
  ${YDB_SDK_SOURCE_DIR}/util/folder/fts.cpp
  ${YDB_SDK_SOURCE_DIR}/util/folder/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/util/folder/iterator.cpp
  ${YDB_SDK_SOURCE_DIR}/util/folder/path.cpp
  ${YDB_SDK_SOURCE_DIR}/util/folder/pathsplit.cpp
  ${YDB_SDK_SOURCE_DIR}/util/folder/tempdir.cpp
)

target_joined_source(yutil
  all_generic.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/adaptor.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/algorithm.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/array_ref.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/array_size.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/bitmap.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/bitops.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/bt_exception.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/buffer.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/cast.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/deque.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/explicit_type.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/fastqueue.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/flags.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/function.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/function_ref.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/guid.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/hash.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/hash_multi_map.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/hash_table.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/hash_primes.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/hash_set.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/hide_ptr.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/intrlist.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/is_in.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/iterator.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/iterator_range.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/lazy_value.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/list.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/map.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/mapfindptr.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/maybe.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/mem_copy.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/noncopyable.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/object_counter.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/overloaded.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/ptr.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/queue.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/refcount.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/scope.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/serialized_enum.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/set.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/singleton.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/size_literals.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/stack.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/store_policy.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/strbuf.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/strfcpy.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/string.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/typelist.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/typetraits.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/utility.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/va_args.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/variant.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/vector.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/xrange.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/yexception.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/ylimits.cpp
  ${YDB_SDK_SOURCE_DIR}/util/generic/ymath.cpp
)

target_joined_source(yutil
  all_memory.cpp
  ${YDB_SDK_SOURCE_DIR}/util/memory/addstorage.cpp
  ${YDB_SDK_SOURCE_DIR}/util/memory/alloc.cpp
  ${YDB_SDK_SOURCE_DIR}/util/memory/blob.cpp
  ${YDB_SDK_SOURCE_DIR}/util/memory/mmapalloc.cpp
  ${YDB_SDK_SOURCE_DIR}/util/memory/pool.cpp
  ${YDB_SDK_SOURCE_DIR}/util/memory/segmented_string_pool.cpp
  ${YDB_SDK_SOURCE_DIR}/util/memory/segpool_alloc.cpp
  ${YDB_SDK_SOURCE_DIR}/util/memory/smallobj.cpp
  ${YDB_SDK_SOURCE_DIR}/util/memory/tempbuf.cpp
)

target_joined_source(yutil
  all_network.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/address.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/endpoint.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/hostip.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/init.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/interface.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/iovec.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/ip.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/nonblock.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/pair.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/poller.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/pollerimpl.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/sock.cpp
  ${YDB_SDK_SOURCE_DIR}/util/network/socket.cpp
)

target_joined_source(yutil
  all_random.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/common_ops.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/easy.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/entropy.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/fast.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/lcg_engine.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/mersenne32.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/mersenne64.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/mersenne.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/normal.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/shuffle.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/init_atfork.cpp
)

target_joined_source(yutil
  all_stream.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/aligned.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/buffer.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/buffered.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/debug.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/direct_io.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/file.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/format.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/hex.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/holder.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/input.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/labeled.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/length.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/mem.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/multi.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/null.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/output.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/pipe.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/printf.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/str.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/tee.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/tempbuf.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/tokenizer.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/trace.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/walk.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/zerocopy.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/zerocopy_output.cpp
  ${YDB_SDK_SOURCE_DIR}/util/stream/zlib.cpp
)

target_joined_source(yutil
  all_string.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/ascii.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/builder.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/cstriter.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/escape.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/hex.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/join.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/printf.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/reverse.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/split.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/strip.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/strspn.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/subst.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/type.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/util.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/vector.cpp
)

target_joined_source(yutil
  all_system_1.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/atexit.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/backtrace.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/compat.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/condvar.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/daemon.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/datetime.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/defaults.c
  ${YDB_SDK_SOURCE_DIR}/util/system/direct_io.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/dynlib.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/env.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/error.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/event.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/fasttime.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/file.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/file_lock.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/filemap.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/flock.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/fs.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/fstat.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/getpid.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/hi_lo.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/hostname.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/hp_timer.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/info.cpp
)

target_joined_source(yutil
  all_system_2.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/context.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/execpath.cpp
)

target_joined_source(yutil
  all_system_3.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/align.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/byteorder.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/cpu_id.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/fhandle.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/guard.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/interrupt_signals.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/madvise.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/maxlen.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/mincore.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/mktemp.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/mlock.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/mutex.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/nice.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/pipe.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/platform.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/progname.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/protect.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/rusage.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/rwlock.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/sanitizers.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/shellcommand.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/shmat.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/sigset.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/spinlock.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/spin_wait.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/src_location.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/sys_alloc.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/sysstat.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/tempfile.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/tls.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/type_name.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/unaligned_mem.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/user.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/utime.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/yassert.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/yield.cpp
)

target_joined_source(yutil
  all_system_4.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/mem_info.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/sem.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/thread.cpp
  ${YDB_SDK_SOURCE_DIR}/util/system/types.cpp
)

target_joined_source(yutil
  all_thread.cpp
  ${YDB_SDK_SOURCE_DIR}/util/thread/factory.cpp
  ${YDB_SDK_SOURCE_DIR}/util/thread/fwd.cpp
  ${YDB_SDK_SOURCE_DIR}/util/thread/lfqueue.cpp
  ${YDB_SDK_SOURCE_DIR}/util/thread/lfstack.cpp
  ${YDB_SDK_SOURCE_DIR}/util/thread/pool.cpp
  ${YDB_SDK_SOURCE_DIR}/util/thread/singleton.cpp
)

target_sources(yutil PRIVATE
  ${YDB_SDK_SOURCE_DIR}/util/system/compiler.cpp
  ${YDB_SDK_SOURCE_DIR}/util/digest/city.cpp
  ${YDB_SDK_SOURCE_DIR}/util/random/random.cpp
  ${YDB_SDK_SOURCE_DIR}/util/string/cast.cpp
)

if (NOT WIN32)
  target_ragel_lexers(yutil
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/util/datetime/parser.rl6
    -CG2
  )
endif ()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  target_yasm_source(yutil
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/util/system/context_x86.asm
    -I
    ${YDB_SDK_BINARY_DIR}
    -I
    ${YDB_SDK_SOURCE_DIR}
  )
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
  target_sources(yutil PRIVATE
    ${YDB_SDK_SOURCE_DIR}/util/system/context_aarch64.S
  )
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_sources(yutil PRIVATE
    ${YDB_SDK_SOURCE_DIR}/util/system/valgrind.cpp
    ${YDB_SDK_SOURCE_DIR}/util/system/mktemp_system.cpp
  )
elseif (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_sources(yutil PRIVATE
    ${YDB_SDK_SOURCE_DIR}/util/datetime/strptime.cpp
    ${YDB_SDK_SOURCE_DIR}/util/folder/lstat_win.c
    ${YDB_SDK_SOURCE_DIR}/util/folder/dirent_win.c
    ${YDB_SDK_SOURCE_DIR}/util/system/err.cpp
    ${YDB_SDK_SOURCE_DIR}/util/system/fs_win.cpp
    ${YDB_SDK_SOURCE_DIR}/util/system/winint.cpp
    ${YDB_SDK_SOURCE_DIR}/util/system/mktemp_system.cpp
  )
  target_ragel_lexers(yutil
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/util/datetime/parser.rl6
    -CT0
  )
endif()

target_compile_definitions(yutil PUBLIC
  ARCADIA_ROOT_CMAKE_HELPER=${YDB_SDK_SOURCE_DIR}
  ARCADIA_BUILD_ROOT_CMAKE_HELPER=${YDB_SDK_BINARY_DIR}
)

_ydb_sdk_install_targets(TARGETS yutil)
