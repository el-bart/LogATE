FROM debian:11
RUN apt-get update
RUN apt-get dist-upgrade -y

RUN apt-get install -y libncurses6
RUN apt-get install -y libpocofoundation60 \
                       libpoconet60
RUN apt-get install -y libboost-program-options1.67.0

COPY CursATE /usr/local/bin/
USER 9000:9000
ENTRYPOINT ["CursATE"]
