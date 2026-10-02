ARG RUNTIME_IMAGE=ubuntu:24.04@sha256:019e8eb29a85e74d64925745884f2ec79aa27e3feab36353d24656f4d6b89467
FROM ${RUNTIME_IMAGE}
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        libqt6sql6-odbc \
        ninja-build \
        patch \
        python3 \
        python3-yaml \
        qt6-base-dev \
        qt6-base-private-dev \
    && rm -rf /var/lib/apt/lists/*
