# TODOs

## library part

* saving filter tree
* loading filter tree
* search inside a given node set via regex (string is already implemented)
* add support for arrays in all the places in the code
  - think about nice syntax for Path support of arrays (`jq` as an inspiration?)
* filter that saves all logs after spotting a given pattern (grep-like); add option to save N logs before as well.
  - useful for dynamic content sellection (eg. "find first experiment and then split into x/y/z filters")
* disk/RAM auto-switching backend for `Tree::Logs`, so that it's possible to process more logs than there is RAM in computer.
* make `Tree::Logs` lock-free so that processing can be done in a way more parallel mode than it is being done now...
  - maybe logs could be in a lock-free balanced tree?
  - versioned tree would be needed to provide a steady view of a dynamic structure!
* `Filter::Grep` should support empty path, as a mean of defining "any path"
* add parallel processing of filters data (now it is possible, since we no longer need to write at the end only, to preserve global order!)


## UI

* colored highlighting of searched elements
* make deleting root node possible, if there is just one child - then it will pivot root element with its child (useful for dropping non-interesting logs)
* embed version tag (if present) and commit hash into a binary, so that it can be displayed on request
* generic mechanism for searching inside whole data source (usefull for different small-volume windows, i.e. non-log-lists)
* add option to plot:
  - histogram (with and without buckets)
  - data over time
  - log and/or normal scale
* 'r' to refresh screen view (in case of filter tree preview)
* `follow mode` - add a hey to auto-refresh screen upon new logs arrival (i.e. keep view always at the end).
* add logs-per-second into a status bar.
* add split between runnig threads and queue length (just for readability).
* add screen with a preview of a background jobs and their progress.
* background addition of nodes should add multiple smaller jobs, so that there is no freeze when bg lobs locks logs when copying.


## misc

## issues
* ncurses vs. `$TERM` settings:
  - why does it require special TERM via screen?
  - why does it require special TERM via PuTTY?
  - how does mc solve this?
