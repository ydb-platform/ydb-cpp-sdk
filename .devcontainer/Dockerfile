FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

# CPM supplies project libraries at configure time. The image contains only
# platform prerequisites and developer tools.
RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates ccache clang cmake gdb git libidn11-dev libssl-dev lld ninja-build \
      pkg-config python3 ragel yasm \
    && rm -rf /var/lib/apt/lists/*

ENV CPM_SOURCE_CACHE=/workspaces/.cache/cpm
