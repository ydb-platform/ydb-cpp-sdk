ARG RUNTIME_IMAGE=ubuntu:24.04@sha256:019e8eb29a85e74d64925745884f2ec79aa27e3feab36353d24656f4d6b89467
FROM ${RUNTIME_IMAGE}

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        dpkg-dev \
        odbcinst \
        python3 \
        unixodbc \
    && rm -rf /var/lib/apt/lists/*
