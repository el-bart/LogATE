# release notes

# dev
* `Filter::Explode` "exploding" logs based on a given field value.
* `Filter::Grep` to search for selecting a given key/value configurations.
* auto-hiding columns used selection, so that logs are not cluttered with repeated, identical values.
* searching elements in a given node, byt key/value text.
* priority columns for displaying (for now values are hardcoded).
* display number of errors from networking stack.
* display stats: number of lines, location in file (floating point % + line number).
* `Filter::AcceptAll` accepting all nodes, optionally trimming a given set of fields when displaying.
* `Filter::From` getting only logs from a given point.
* `Filter::To` getting only logs up to a given point in time.
* visual browser for a filter tree.
* error handling in UI.
* input from network socket.
* JSON input format.
* live log view.
* human-readable printing of a log.
* loads of memory and speed optimizations... ;)
* move all filters to a given log line (or close to it, when given log is not in all of them).
* move all child filters to a given log line (or close to it, when given log is not in all of them).
* multi-threaded logs processing.
* ncurses basesed viewer.
* show number of logs per subtree.
* window with verbose display of a selected log.
