FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    libqt6sql6-odbc \
    odbcinst \
    qt6-base-dev \
    unixodbc \
    && rm -rf /var/lib/apt/lists/*

COPY test_project /test_project
COPY component_test /component_test
COPY odbc_qt_test /odbc_qt_test
RUN cmake -S /odbc_qt_test -B /odbc_qt_test/build \
    && cmake --build /odbc_qt_test/build -j"$(nproc)"
WORKDIR /test_project
