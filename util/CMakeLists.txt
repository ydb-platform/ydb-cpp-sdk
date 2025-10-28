add_subdirectory(charset)
add_subdirectory(draft)

if (YDB_SDK_TESTS)
  add_ydb_test(NAME util-datetime-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/datetime
    SOURCES
      datetime/base_ut.cpp
      datetime/cputimer_ut.cpp
      datetime/parser_deprecated_ut.cpp
      datetime/parser_ut.cpp
      datetime/process_uptime_ut.cpp
      datetime/uptime_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME util-digest-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/digest
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
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/folder
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

  add_ydb_test(NAME util-generic-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/generic
    SOURCES
      generic/adaptor_ut.cpp
      generic/algorithm_ut.cpp
      generic/array_ref_ut.cpp
      generic/array_size_ut.cpp
      generic/bitmap_ut.cpp
      generic/bitops_ut.cpp
      generic/buffer_ut.cpp
      generic/cast_ut.cpp
      generic/deque_ut.cpp
      generic/enum_range_ut.cpp
      generic/explicit_type_ut.cpp
      generic/flags_ut.cpp
      generic/function_ref_ut.cpp
      generic/function_ut.cpp
      generic/guid_ut.cpp
      generic/hash_primes_ut.cpp
      generic/hash_ut.cpp
      generic/intrlist_ut.cpp
      generic/is_in_ut.cpp
      generic/iterator_range_ut.cpp
      generic/iterator_ut.cpp
      generic/lazy_value_ut.cpp
      generic/list_ut.cpp
      generic/mapfindptr_ut.cpp
      generic/map_ut.cpp
      generic/maybe_ut.cpp
      generic/mem_copy_ut.cpp
      generic/objects_counter_ut.cpp
      generic/overloaded_ut.cpp
      generic/ptr_ut.cpp
      generic/queue_ut.cpp
      generic/scope_ut.cpp
      generic/serialized_enum_ut.cpp
      generic/set_ut.cpp
      generic/singleton_ut.cpp
      generic/size_literals_ut.cpp
      generic/stack_ut.cpp
      generic/store_policy_ut.cpp
      generic/strbuf_ut.cpp
      generic/string_transparent_hash_ut.cpp
      generic/string_ut.cpp
      generic/typelist_ut.cpp
      generic/typetraits_ut.cpp
      generic/utility_ut.cpp
      generic/va_args_ut.cpp
      generic/vector_ut.cpp
      generic/xrange_ut.cpp
      generic/ylimits_ut.cpp
      generic/ymath_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME util-generic-yexception_ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/generic
    SOURCES
      generic/yexception_ut.c
      generic/yexception_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )
  if (CMAKE_SYSTEM_NAME MATCHES "^(Linux|Darwin)$")
    target_compile_definitions(util-generic-yexception_ut
      PRIVATE
        LIBCXX_BUILDING_LIBCXXRT
        LIBCXX_BUILDING_LIBGCC
    )
  endif()

  add_ydb_test(NAME util-memory-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/memory
    SOURCES
      memory/addstorage_ut.cpp
      memory/blob_ut.cpp
      memory/pool_ut.cpp
      memory/smallobj_ut.cpp
      memory/tempbuf_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME util-network-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/network
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

  add_ydb_test(NAME util-random-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/random
    SOURCES
      random/common_ops_ut.cpp
      random/easy_ut.cpp
      random/entropy_ut.cpp
      random/fast_ut.cpp
      random/mersenne_ut.cpp
      random/normal_ut.cpp
      random/random_ut.cpp
      random/shuffle_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
    TEST_ARG
      --filter-file filter.txt
  )
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/random/filter.txt
    "-TestCommonRNG::TestStlCompatibility"
  )

  add_ydb_test(NAME util-stream-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/stream
    SOURCES
      stream/aligned_ut.cpp
      stream/buffered_ut.cpp
      stream/buffer_ut.cpp
      stream/direct_io_ut.cpp
      stream/file_ut.cpp
      stream/format_std_ut.cpp
      stream/format_ut.cpp
      stream/hex_ut.cpp
      stream/input_ut.cpp
      stream/ios_ut.cpp
      stream/labeled_ut.cpp
      stream/length_ut.cpp
      stream/mem_ut.cpp
      stream/multi_ut.cpp
      stream/printf_ut.cpp
      stream/str_ut.cpp
      stream/tokenizer_ut.cpp
      stream/walk_ut.cpp
      stream/zerocopy_output_ut.cpp
      stream/zlib_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME util-string-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/string
    SOURCES
      string/ascii_ut.cpp
      string/builder_ut.cpp
      string/cast_ut.cpp
      string/escape_ut.cpp
      string/hex_ut.cpp
      string/join_ut.cpp
      string/printf_ut.cpp
      string/split_ut.cpp
      string/strip_ut.cpp
      string/strspn_ut.cpp
      string/subst_ut.cpp
      string/type_ut.cpp
      string/util_ut.cpp
      string/vector_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME util-system-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/system
    SOURCES
      system/align_ut.cpp
      system/atexit_ut.cpp
      system/backtrace_ut.cpp
      system/byteorder_ut.cpp
      system/compat_ut.cpp
      system/compiler_ut.cpp
      system/condvar_ut.cpp
      system/context_ut.cpp
      system/cpu_id_ut.cpp
      system/daemon_ut.cpp
      system/datetime_ut.cpp
      system/direct_io_ut.cpp
      system/env_ut.cpp
      system/error_ut.cpp
      system/event_ut.cpp
      system/execpath_ut.cpp
      system/filemap_ut.cpp
      system/file_ut.cpp
      system/flock_ut.cpp
      system/fs_ut.cpp
      system/getpid_ut.cpp
      system/guard_ut.cpp
      system/hi_lo_ut.cpp
      system/hostname_ut.cpp
      system/info_ut.cpp
      system/interrupt_signals_ut.cpp
      system/mem_info_ut.cpp
      system/mincore_ut.cpp
      system/mktemp_ut.cpp
      system/mutex_ut.cpp
      system/nice_ut.cpp
      system/pipe_ut.cpp
      system/platform_ut.cpp

      # NOTE: `progname_ut` checks if the executable file's name, aka the target's
      # name, is either "util-system-ut" or slightly different variations
      system/progname_ut.cpp

      system/rusage_ut.cpp
      system/rwlock_ut.cpp
      system/sanitizers_ut.cpp
      system/shellcommand_ut.cpp
      system/shmat_ut.cpp
      system/spinlock_ut.cpp
      system/src_location_ut.cpp
      system/src_root_ut.cpp
      system/tempfile_ut.cpp
      system/tls_ut.cpp

      # NOTE: this test shouldn't be run with STL, so it's explicitly disabled
      # system/type_name_ut.cpp

      # NOTE: `thread_ut` compares the executable file's name and the current
      # thread name, whose length can't be more than 16 bytes in Linux,
      # so the test name shouldn't be longer than "util-system-ut"
      system/thread_ut.cpp

      system/types_ut.cpp
      system/user_ut.cpp
      system/yassert_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
    TEST_ARG
      --filter-file filter.txt
  )
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/system/filter.txt
    "-TBackTraceTest::TestPrintBackTrace\n"
    "-TBackTraceTest::TestSetFormatBackTraceFn"
  )

  add_ydb_test(NAME util-system-unaligned_mem_ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/system
    SOURCES
      system/unaligned_mem_ut.cpp
    LINK_LIBRARIES
      yutil
      benchmark-clobber
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME util-system-fstat_ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/system
    OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/system/testing_out_stuff
    SOURCES
      system/fstat_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_ydb_test(NAME util-system-fs_win_ut
      WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/system
      SOURCES
        system/fs_win_ut.cpp
      LINK_LIBRARIES
        yutil
        cpp-testing-unittest_main
      LABELS
        unit
    )
  endif()

  add_ydb_test(NAME util-thread-ut
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/thread
    SOURCES
      thread/factory_ut.cpp
      thread/lfqueue_ut.cpp
      thread/lfstack_ut.cpp
      thread/pool_ut.cpp
      thread/singleton_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
      threading-future
    LABELS
      unit
  )
endif(YDB_SDK_TESTS)

_ydb_sdk_add_library(yutil)

target_compile_options(yutil PUBLIC
  -DTSTRING_IS_STD_STRING
  -DNO_CUSTOM_CHAR_PTR_STD_COMPARATOR
)

target_compile_options(yutil PRIVATE
  -Wnarrowing
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
  ${YDB_SDK_SOURCE_DIR}/util/datetime/process_uptime.cpp
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

# NOTE: these definitions are needed to `util/system/src_root.h` works properly
file(TO_NATIVE_PATH "${ARCADIA_ROOT}" ARCADIA_ROOT)
file(TO_NATIVE_PATH "${ARCADIA_BUILD_ROOT}" ARCADIA_BUILD_ROOT)
target_compile_definitions(yutil PUBLIC
  ARCADIA_ROOT=${ARCADIA_ROOT}
  ARCADIA_BUILD_ROOT=${ARCADIA_BUILD_ROOT}
)

_ydb_sdk_install_targets(TARGETS yutil)
