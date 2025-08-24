FROM debian:13
RUN apt-get update && apt-get dist-upgrade -y

RUN apt-get update && apt-get install -y \
    cmake \
    ninja-build
RUN apt-get update && apt-get install -y \
    g++ \
    clang \
    llvm
RUN apt-get update && apt-get install -y \
    libboost-all-dev \
    libncurses-dev \
    libpoco-dev \
    nlohmann-json3-dev
RUN apt-get update && apt-get install -y \
    gdb \
    git \
    kcachegrind \
    valgrind \
    wget

ENV DOCTEST_VER=2.4.11
RUN cd /usr/src/ && \
    wget "https://github.com/doctest/doctest/archive/refs/tags/v${DOCTEST_VER}.tar.gz" && \
    tar xzvf "v${DOCTEST_VER}.tar.gz" && \
    cp -rv "doctest-${DOCTEST_VER}/doctest" /usr/local/include && \
    rm "v${DOCTEST_VER}.tar.gz"

ENV BUT_VER=2.2.0
RUN cd /usr/src/ && \
    wget "https://github.com/el-bart/but/archive/v${BUT_VER}.tar.gz" && \
    tar xzvf "v${BUT_VER}.tar.gz" && \
    cd "but-${BUT_VER}/" && \
    mkdir -p "build" && \
    cd "build" && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../src/cmake/toolchains/gcc.cmake .. && \
    ninja && \
    ninja install
