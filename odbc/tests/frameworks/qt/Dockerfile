ARG RUNTIME_IMAGE=ubuntu:24.04@sha256:019e8eb29a85e74d64925745884f2ec79aa27e3feab36353d24656f4d6b89467
FROM ${RUNTIME_IMAGE}
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        libqt6sql6-odbc \
        python3 \
        python3-yaml \
        qt6-base-dev \
    && rm -rf /var/lib/apt/lists/*
COPY example /opt/ydb-odbc-qt-test-src
RUN cmake -S /opt/ydb-odbc-qt-test-src -B /opt/ydb-odbc-qt-test-build \
    && cmake --build /opt/ydb-odbc-qt-test-build --parallel \
    && install -m 0755 /opt/ydb-odbc-qt-test-build/ydb_odbc_qt_test /usr/local/bin/ydb_odbc_qt_test \
    && rm -rf /opt/ydb-odbc-qt-test-build /opt/ydb-odbc-qt-test-src
