ARG RUNTIME_IMAGE
FROM ${RUNTIME_IMAGE}
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential ca-certificates patch python3 python3-yaml unixodbc unixodbc-dev odbcinst \
    erlang-base erlang-dev erlang-odbc erlang-common-test erlang-tools \
    php-cli php-odbc \
    ghc libghc-hdbc-dev libghc-concurrent-extra-dev libghc-utf8-string-dev libghc-hunit-dev libghc-quickcheck2-dev \
    libghc-old-time-dev libghc-old-locale-dev libghc-convertible-dev \
    ruby ruby-dev \
    lua5.4 liblua5.4-dev \
    r-base-core r-cran-rcpp r-cran-bit64 r-cran-blob r-cran-hms r-cran-r6 r-cran-vctrs r-cran-rlang r-cran-withr r-cran-cli r-cran-testthat r-cran-dbitest r-cran-pkgload \
    && dpkg-query -W > /runtime-packages.txt \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y --no-install-recommends php-mbstring php-xml \
    && dpkg-query -W > /runtime-packages.txt && rm -rf /var/lib/apt/lists/*
