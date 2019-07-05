# TODOs

## library part

* saving filter tree
* loading filter tree
* sorting?
* maybe adding a child to Explode maybe should mean adding to all subnodes? how to do this? factory?
* search inside a given node set via regex (string is already implemented)
* add support for arrays in all the places in the code
  - think about nice syntax for Path support of arrays
* filter that saves all logs after spotting a given pattern (grep-like); add option to save N logs before as well.
* disk/RAM auto-switching backend for `Tree::Logs`, so that it's possible to process more logs than there is RAM in computer.
* make `Tree::Logs` lock-free so that processing can be done in a way more parallel mode than it is being done now...
  - maybe logs could be in a lock-free balanced tree?
* save given logs to a file
  - as JSONs
  - as human-readable lines
* make sort-field configurable (if field does not exist use SN)
* `Filter::Grep` should support empty path, as a mean of defining "any path"
* make `Net::TcpServer` use `Path` as a parameter, instead of a hardcoded value.


## UI

* key shortcuts for tabs switching fast (small 'm'arks and big 'Marks'?)
* colored highlighting of searched elements
* runtime edition of silent tags
* runtime edition of priority tags
* export logs from a given filter to a file
* make deleting root node possible, if there is just one child - then it will pivot root element with its child (useful for dropping non-interesting logs)
* embed version tag (if present) and commit hash into a binary, so that it can be displayed on request
* generic mechanism for searching inside whole data source (usefull for different small-volume windows, i.e. non-log-lists)
* how about a scripting language, allowing interactive searching and tree building? eg. 'search for pattern X, once found do this, start that, etc...'?
* add option to plot:
  - histogram (with and without buckets)
  - data over time
  - log and/or normal scale
* tags for log-entries and log-lists (i.e. to switch fast between log locations and log screens).
* home/end for forms (eg. filter selection).
* key shortcut to goto a given log entry, but on a different log-list (i.e. should pop-up filter tree screen and when navigating to a given one, select a given log there).
* 'r' to refresh screen view (in case of filter tree preview)
* `follow mode` - add a hey to auto-refresh screen upon new logs arrival (i.e. keep view always at the end).
* `^o` shortcut for confirming windows? just to minimize number of key strokes required to get filter up and running...
* add logs-per-second into a status bar.
* add split between runnig threads and queue length (just for readability).


## misc

## issues
* ncurses vs. `$TERM` settings:
  - why does it require special TERM via screen?
  - why does it require special TERM via PuTTY?
  - how does mc solve this?
