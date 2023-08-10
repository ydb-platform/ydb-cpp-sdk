# Generated by devtools/yamaker from nixpkgs 22.11.

LIBRARY()

LICENSE(
    "(GPL-2.0-only WITH Linux-syscall-note OR MIT)" AND
    "(LGPL-2.1-only OR MIT)" AND
    MIT
)

LICENSE_TEXTS(.yandex_meta/licenses.list.txt)

VERSION(2.4)

ORIGINAL_SOURCE(https://github.com/axboe/liburing/archive/liburing-2.4.tar.gz)

ADDINCL(
    GLOBAL contrib/libs/liburing/src/include
)

NO_COMPILER_WARNINGS()

NO_RUNTIME()

CFLAGS(
    -DLIBURING_INTERNAL
)

SRCS(
    src/nolibc.c
    src/queue.c
    src/register.c
    src/setup.c
    src/syscall.c
    src/version.c
)

END()

RECURSE(
    test/232c93d07b74.t
    test/35fa71a030ca.t
    test/500f9fbadef8.t
    test/7ad0e4b2f83c.t
    test/8a9973408177.t
    test/917257daa0fe.t
    test/a0908ae19763.t
    test/a4c0b3decb33.t
    test/accept-link.t
    test/accept-reuse.t
    test/accept-test.t
    test/accept.t
    test/across-fork.t
    test/b19062a56726.t
    test/b5837bd5311d.t
    test/buf-ring.t
    test/ce593a6c480a.t
    test/close-opath.t
    test/connect-rep.t
    test/connect.t
    test/cq-full.t
    test/cq-overflow.t
    test/cq-peek-batch.t
    test/cq-ready.t
    test/cq-size.t
    test/d4ae271dfaae.t
    test/d77a67ed5f27.t
    test/defer-taskrun.t
    test/defer.t
    test/double-poll-crash.t
    test/drop-submit.t
    test/eeed8b54e0df.t
    test/empty-eownerdead.t
    test/eploop.t
    test/eventfd-disable.t
    test/eventfd-reg.t
    test/eventfd-ring.t
    test/eventfd.t
    test/evloop.t
    test/exec-target.t
    test/exit-no-cleanup.t
    test/fadvise.t
    test/fallocate.t
    test/fc2a85cb02ef.t
    test/fd-pass.t
    test/file-register.t
    test/file-update.t
    test/file-verify.t
    test/files-exit-hang-poll.t
    test/files-exit-hang-timeout.t
    test/fixed-buf-iter.t
    test/fixed-link.t
    test/fixed-reuse.t
    test/fpos.t
    test/fsync.t
    test/hardlink.t
    test/io-cancel.t
    test/io_uring_enter.t
    test/io_uring_passthrough.t
    test/io_uring_register.t
    test/io_uring_setup.t
    test/iopoll-leak.t
    test/iopoll-overflow.t
    test/iopoll.t
    test/lfs-openat-write.t
    test/lfs-openat.t
    test/link-timeout.t
    test/link.t
    test/link_drain.t
    test/madvise.t
    test/mkdir.t
    test/msg-ring-flags.t
    test/msg-ring-overflow.t
    test/msg-ring.t
    test/multicqes_drain.t
    test/nolibc.t
    test/nop-all-sizes.t
    test/nop.t
    test/open-close.t
    test/open-direct-link.t
    test/open-direct-pick.t
    test/openat2.t
    test/personality.t
    test/pipe-bug.t
    test/pipe-eof.t
    test/pipe-reuse.t
    test/poll-cancel-all.t
    test/poll-cancel-ton.t
    test/poll-cancel.t
    test/poll-link.t
    test/poll-many.t
    test/poll-mshot-overflow.t
    test/poll-mshot-update.t
    test/poll-race-mshot.t
    test/poll-race.t
    test/poll-ring.t
    test/poll-v-poll.t
    test/poll.t
    test/pollfree.t
    test/probe.t
    test/read-before-exit.t
    test/read-write.t
    test/recv-msgall-stream.t
    test/recv-msgall.t
    test/recv-multishot.t
    test/reg-hint.t
    test/reg-reg-ring.t
    test/regbuf-merge.t
    test/register-restrictions.t
    test/rename.t
    test/ring-leak.t
    test/ring-leak2.t
    test/ringbuf-read.t
    test/rsrc_tags.t
    test/rw_merge_test.t
    test/self.t
    test/send-zerocopy.t
    test/send_recv.t
    test/send_recvmsg.t
    test/sendmsg_fs_cve.t
    test/shared-wq.t
    test/short-read.t
    test/shutdown.t
    test/sigfd-deadlock.t
    test/single-issuer.t
    test/skip-cqe.t
    test/socket-rw-eagain.t
    test/socket-rw-offset.t
    test/socket-rw.t
    test/socket.t
    test/splice.t
    test/sq-full-cpp.t
    test/sq-full.t
    test/sq-poll-dup.t
    test/sq-poll-kthread.t
    test/sq-poll-share.t
    test/sq-space_left.t
    test/sqpoll-cancel-hang.t
    test/sqpoll-disable-exit.t
    test/sqpoll-exit-hang.t
    test/sqpoll-sleep.t
    test/stdout.t
    test/submit-and-wait.t
    test/submit-link-fail.t
    test/submit-reuse.t
    test/symlink.t
    test/sync-cancel.t
    test/teardowns.t
    test/thread-exit.t
    test/timeout-new.t
    test/timeout.t
    test/tty-write-dpoll.t
    test/unlink.t
    test/version.t
    test/wakeup-hang.t
    test/xattr.t
)