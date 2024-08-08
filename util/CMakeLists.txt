add_subdirectory(charset)
add_subdirectory(draft)

if (YDB_SDK_TESTS)
  add_ydb_multiple_tests(PREFIX util BASE_DIR datetime
    FILES
      base_ut.cpp
      cputimer_ut.cpp
      parser_deprecated_ut.cpp
      parser_ut.cpp
      process_uptime_ut.cpp
      uptime_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_multiple_tests(PREFIX util BASE_DIR digest
    FILES
      city_ut.cpp
      fnv_ut.cpp
      multi_ut.cpp
      murmur_ut.cpp
      sequence_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_multiple_tests(PREFIX util BASE_DIR folder
    FILES
      dirut_ut.cpp
      filelist_ut.cpp
      fts_ut.cpp
      iterator_ut.cpp
      path_ut.cpp
      pathsplit_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
      threading-future
    LABELS
      unit
  )

  add_ydb_multiple_tests(PREFIX util BASE_DIR generic
    FILES
      adaptor_ut.cpp
      algorithm_ut.cpp
      array_ref_ut.cpp
      array_size_ut.cpp
      bitmap_ut.cpp
      bitops_ut.cpp
      buffer_ut.cpp
      cast_ut.cpp
      deque_ut.cpp
      enum_range_ut.cpp
      explicit_type_ut.cpp
      flags_ut.cpp
      function_ref_ut.cpp
      function_ut.cpp
      guid_ut.cpp
      hash_primes_ut.cpp
      hash_ut.cpp
      intrlist_ut.cpp
      is_in_ut.cpp
      iterator_range_ut.cpp
      iterator_ut.cpp
      lazy_value_ut.cpp
      list_ut.cpp
      mapfindptr_ut.cpp
      map_ut.cpp
      maybe_ut.cpp
      mem_copy_ut.cpp
      objects_counter_ut.cpp
      overloaded_ut.cpp
      ptr_ut.cpp
      queue_ut.cpp
      scope_ut.cpp
      serialized_enum_ut.cpp
      set_ut.cpp
      singleton_ut.cpp
      size_literals_ut.cpp
      stack_ut.cpp
      store_policy_ut.cpp
      strbuf_ut.cpp
      # See fix/util-unit-tests branch
      # string_transparent_hash_ut.cpp
      string_ut.cpp
      typelist_ut.cpp
      typetraits_ut.cpp
      utility_ut.cpp
      va_args_ut.cpp
      vector_ut.cpp
      xrange_ut.cpp
      ylimits_ut.cpp
      ymath_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_test(NAME util-generic-yexception_ut
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

  add_ydb_multiple_tests(PREFIX util BASE_DIR memory
    FILES
      addstorage_ut.cpp
      blob_ut.cpp
      pool_ut.cpp
      smallobj_ut.cpp
      tempbuf_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_multiple_tests(PREFIX util BASE_DIR network
    FILES
      address_ut.cpp
      endpoint_ut.cpp
      ip_ut.cpp
      poller_ut.cpp
      sock_ut.cpp
      socket_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
      threading-future
    LABELS
      unit
  )

  add_ydb_multiple_tests(PREFIX util BASE_DIR random
    FILES
      # See fix/util-unit-tests branch
      # common_ops_ut.cpp
      easy_ut.cpp
      entropy_ut.cpp
      fast_ut.cpp
      mersenne_ut.cpp
      normal_ut.cpp
      random_ut.cpp
      shuffle_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_multiple_tests(PREFIX util BASE_DIR stream
    FILES
      aligned_ut.cpp
      buffered_ut.cpp
      buffer_ut.cpp
      direct_io_ut.cpp
      file_ut.cpp
      format_std_ut.cpp
      format_ut.cpp
      hex_ut.cpp
      input_ut.cpp
      ios_ut.cpp
      labeled_ut.cpp
      length_ut.cpp
      mem_ut.cpp
      multi_ut.cpp
      printf_ut.cpp
      str_ut.cpp
      tokenizer_ut.cpp
      walk_ut.cpp
      zerocopy_output_ut.cpp
      zlib_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_multiple_tests(PREFIX util BASE_DIR string
    FILES
      ascii_ut.cpp
      builder_ut.cpp
      cast_ut.cpp
      escape_ut.cpp
      hex_ut.cpp
      join_ut.cpp
      printf_ut.cpp
      split_ut.cpp
      strip_ut.cpp
      strspn_ut.cpp
      subst_ut.cpp
      type_ut.cpp
      util_ut.cpp
      vector_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  add_ydb_multiple_tests(PREFIX util BASE_DIR system
    FILES
      align_ut.cpp
      atexit_ut.cpp
      backtrace_ut.cpp
      byteorder_ut.cpp
      compat_ut.cpp
      compiler_ut.cpp
      condvar_ut.cpp
      context_ut.cpp
      cpu_id_ut.cpp
      daemon_ut.cpp
      datetime_ut.cpp
      # See fix/util-unit-tests branch
      # direct_io_ut.cpp
      env_ut.cpp
      error_ut.cpp
      event_ut.cpp
      execpath_ut.cpp
      filemap_ut.cpp
      file_ut.cpp
      flock_ut.cpp
      # See fix/util-unit-tests branch
      # fstat_ut.cpp
      fs_ut.cpp
      getpid_ut.cpp
      guard_ut.cpp
      hi_lo_ut.cpp
      hostname_ut.cpp
      info_ut.cpp
      interrupt_signals_ut.cpp
      mem_info_ut.cpp
      mincore_ut.cpp
      mktemp_ut.cpp
      mutex_ut.cpp
      nice_ut.cpp
      pipe_ut.cpp
      platform_ut.cpp
      rusage_ut.cpp
      rwlock_ut.cpp
      sanitizers_ut.cpp
      shellcommand_ut.cpp
      shmat_ut.cpp
      spinlock_ut.cpp
      src_location_ut.cpp
      src_root_ut.cpp
      tempfile_ut.cpp
      tls_ut.cpp
      # See fix/util-unit-tests branch
      # type_name_ut.cpp
      types_ut.cpp
      # TODO: add library/cpp/testing/benchmark
      #       depends only on NBench::Clobber, that's a memory optimization barrier
      # unaligned_mem_ut.cpp
      user_ut.cpp
      yassert_ut.cpp
    ADD_YDB_TEST_ARGS
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  # This test checks if the executable file's name (the same name as the target)
  # is either "util-system-ut" or slightly different variations.
  # It also compares the executable file's name and the current thread name,
  # whose length can't be more than 16 bytes in Linux, so "util-system-ut" is
  # a suitable name.
  add_ydb_test(NAME util-system-ut
    SOURCES
      system/progname_ut.cpp
      system/thread_ut.cpp
    LINK_LIBRARIES
      yutil
      cpp-testing-unittest_main
    LABELS
      unit
  )

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_ydb_test(NAME util-system-fs_win_ut
      SOURCES
        system/fs_win_ut.cpp
      LINK_LIBRARIES
        yutil
        cpp-testing-unittest_main
      LABELS
        unit
    )
  endif()

  add_ydb_multiple_tests(PREFIX util BASE_DIR thread
    FILES
      factory_ut.cpp
      lfqueue_ut.cpp
      lfstack_ut.cpp
      pool_ut.cpp
      singleton_ut.cpp
    ADD_YDB_TEST_ARGS
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

# These definitions are needed to `util/system/src_root.h` works properly
target_compile_definitions(yutil PUBLIC
  ARCADIA_ROOT=${YDB_SDK_SOURCE_DIR}
  ARCADIA_BUILD_ROOT=${YDB_SDK_BINARY_DIR}
)

_ydb_sdk_install_targets(TARGETS yutil)
