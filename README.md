# LogATE

Log Analysis Tool Enhanced for processing and visualizing structured logs.

## idea

the concept is very universal - pretty much anything can generate such logs,
though it is best to use a logger already generates structure logs.

structured logs in C++ can be generated with [https://github.com/el-bart/but](BUT::Log) library.


## quick start

just run `./build_image` script.
it will prepare SDK (docker image), build CursATE from sources and create output docker image called `cursate`.
at the end it will display handy commands to both run CursATE and pipe-in some logs.
