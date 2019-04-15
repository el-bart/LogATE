#FROM baszerr/but-sdk:v1.2.0
FROM baszerr/but-sdk:latest
RUN apt-get update
RUN apt-get dist-upgrade -y
RUN apt-get install -y git wget
#ENV BUT_VER=1.2.0
#RUN cd /usr/src/ && \
#    wget "https://github.com/el-bart/but/archive/v${BUT_VER}.tar.gz" && \
#    tar xzvf "v${BUT_VER}.tar.gz" && \
#    cd "but-${BUT_VER}/build" && \
#    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && \
#    ninja && \
#    ninja install
RUN apt-get install unzip
RUN cd /usr/src/ && \
    wget -O "but-master.zip" "https://codeload.github.com/el-bart/but/zip/master" && \
    unzip "but-master.zip" && \
    cd "but-master/build" && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && \
    ninja && \
    ninja install
ENV DOCTEST_VER=2.2.0
RUN cd /usr/src/ && \
    wget "https://github.com/onqtam/doctest/archive/${DOCTEST_VER}.tar.gz" && \
    tar xzvf "${DOCTEST_VER}.tar.gz" && \
    cp -rv "doctest-${DOCTEST_VER}/doctest" /usr/local/include
RUN apt-get install -y libpoco-dev
RUN apt-get install -y libncurses-dev
RUN apt-get install -y gdb
RUN apt-get install -y libcdk5-doc libcdk5-dev
