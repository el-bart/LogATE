# LogATE

Log Analysis Tool Enhanced for processing and visualizing structured logs.

## idea

the concept is very universal - pretty much anything can generate such logs,
though it is best to use a logger already generates structure logs.

structured logs in C++ can be generated with BUT::Log (https://github.com/el-bart/but) library.

## CI

[![travis CI build status](https://travis-ci.org/el-bart/logate.svg?branch=master)](https://travis-ci.org/el-bart/logate)

download automated build of [BUT's SDK Docker image](https://hub.docker.com/r/baszerr/logate-sdk/), with a command: `docker pull baszerr/logate-sdk`.
