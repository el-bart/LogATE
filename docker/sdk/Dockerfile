FROM baszerr/but-sdk:v1.1.0

RUN apt-get update
RUN apt-get dist-upgrade -y

RUN apt-get install -y git wget
ENV BUT_VER=1.1.0
RUN cd /usr/src/ && \
    wget https://github.com/el-bart/but/archive/v${BUT_VER}.tar.gz && \
    tar xzvf v${BUT_VER}.tar.gz && \
    cd but-${BUT_VER}/build && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && \
    ninja && \
    ninja install
