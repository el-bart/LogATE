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


## nvaigation

once CursATE opens, press `h` to get a help screen, with all supported key mappings.
the most important ones are:
* `t` -- enables tree view of all filters
* `enter` -- opens log entry, to be able to select field to filter by (also by pressing `enter`)
* `J` -- set all views to this (or closes possible) log line
* `/` -- search string in logs
* `q` -- quit CursATE, or close nested window
