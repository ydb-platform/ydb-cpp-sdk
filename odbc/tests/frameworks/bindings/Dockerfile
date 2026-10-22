ARG RUNTIME_IMAGE
FROM ${RUNTIME_IMAGE}
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential ca-certificates patch python3 python3-yaml unixodbc unixodbc-dev odbcinst \
    php-cli php-odbc \
    ruby ruby-dev \
    lua5.4 liblua5.4-dev \
    r-base-core r-cran-rcpp r-cran-bit64 r-cran-blob r-cran-hms r-cran-r6 r-cran-vctrs r-cran-rlang r-cran-withr r-cran-cli r-cran-testthat r-cran-dbitest r-cran-pkgload \
    && dpkg-query -W > /runtime-packages.txt \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y --no-install-recommends php-mbstring php-xml \
    && dpkg-query -W > /runtime-packages.txt && rm -rf /var/lib/apt/lists/*
