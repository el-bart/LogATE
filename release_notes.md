# release notes

## dev
* `cursate` startup script improvements.
* RPM package building code.


## v1.0.0
parallel processing and many usability features.

* searching in logs is now done in parallel, with all threads possible.
* added `follow mode` to log list window (with `F` key) - it auto-focuses on last log in the window.
* adding child nodes to parents adds all elements using multiple threads now (see status bar for overall progress).
* fixed bug with preserving button states between form iterations.
* multiple input streams are now handled in parallel.
* UI now reports tasks being processed and queue size as separate numbers (for readability).
* worker threads now have priority queue for: UI, filters and batch processing (in that order).
* fixed problem with some key shortcuts on forms.
* caching index positions is now disabled for entries that already have a cached entry close enough (tm).
* added (book)marks - mark created with `m<key>` combination makes reference that can then be recalled with `\`<key>` (on current view) and `~<key>` on original window mark was put.
* improved creation of new sub-filters, by passing copying data to a separate thread.
* decreased memory usage when searching.
* searches are now started immediately, regardless of a log window size.


## v0.2.1
minor bugfix
* added missing `boost::program_options` library to a runtime image.


## v0.2.0
sorting by key, more key-shortcuts, command line options, saving JSON/text of a given view and vastly improved input parsing.

* fixed off-by-one bug when doing reverse search (previous element was never checked).
* fixed annoying bug when a canceled search resulted with a "not found" error dialog.
* all logs are now sorted by `key path` (user-defined JSON path to a date-time field, that is used as a sort criteria).
* usual PAS' hardcodes are now exposed as command line options.
* added more shortcuts to forms (see `h` / help for details).
* helper script for starting CursATE in a PAS-aware mode.
* sending logs as JSON/text to a given `host:port`.
* using raw sockets reading for speed.
* input parsing is now parallel, which makes it all > 4x faster on 8 core CPU.
* escape key support has been added, alongside with `q` shortcut.
* input form now allow usage of ^DEL and ^BACKSPACE to erase until EOL and begin of line respectively.
* status bar now contains info about background threads and how many of them are still processing sth.
* `Filter::BinarySplit` implemented - it is essentially the `Filter::Grep`, that stores both matching and non-matching logs, split into two groups.
* `Filter::Explode` no longer trims explode field in the root node view - only in children.


## v0.1.0
first release of PoC.

* `Filter::Explode` "exploding" logs based on a given field value.
* `Filter::Grep` to search for selecting a given key/value configurations.
* auto-hiding columns used selection, so that logs are not cluttered with repeated, identical values.
* searching elements in a given node, by key/value text.
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
* ncurses based viewer.
* show number of logs per subtree.
* window with verbose display of a selected log.
