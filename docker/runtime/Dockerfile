FROM debian:buster
RUN apt-get update
RUN apt-get dist-upgrade -y
RUN apt-get install -y libncurses6
RUN apt-get install -y libpocofoundation60 \
                       libpoconet60
COPY CursATE /usr/local/bin/
ENTRYPOINT ["CursATE"]
