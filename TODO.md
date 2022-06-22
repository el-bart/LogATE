# TODOs

## library part

* saving filter tree
* loading filter tree
* search inside a given node set via regex (string is already implemented)
* filter that saves all logs after spotting a given pattern (grep-like); add option to save N logs before as well.
  - useful for dynamic content selection (eg. "find first experiment and then split into x/y/z filters")
* disk/RAM auto-switching backend for `Tree::Logs`, so that it's possible to process more logs than there is RAM in computer.
* make `Tree::Logs` lock-free so that processing can be done in a way more parallel mode than it is being done now...
  - maybe logs could be in a lock-free balanced tree?
  - versioned tree would be needed to provide a steady view of a dynamic structure!
* `Filter::Grep` should support empty path, as a mean of defining "any path"
* make `FilterFactory` use JSON as input for options. this will be both more readable and easier to integrate with filter configuration from files.
* add filter option to spit out its configuration as JSON (useful for writing down configs).


## UI

* colored highlighting of searched elements
* make deleting root node possible, if there is just one child - then it will pivot root element with its child (useful for dropping non-interesting logs)
* embed version tag (if present) and commit hash into a binary, so that it can be displayed on request
* generic mechanism for searching inside whole data source (useful for different small-volume windows, i.e. non-log-lists)
* add option to plot:
  - histogram (with and without buckets)
  - data over time
  - log and/or normal scale
* 'r' to refresh screen view (in case of filter tree preview)
* add logs-per-second into a status bar.
* add screen with a preview of a background jobs and their progress.
* replace all `switch`es for keys with `map`s of `key_name` to action.
* add option to prune logs until a given one (to ease analysis of stuff that has been pulled in, though is not really needed).
* ^home/^end and home/end -> reverse meaning of these and add `g`/`G` for easy home/end of the list.
* when moving to a bookmark, do not center view, when element is already in a visible set.


## misc

* port tests from doctest to catch2
* add CI (CircleCI?)


## issues
* ncurses vs. `$TERM` settings:
  - why does it require special TERM via screen?
  - why does it require special TERM via PuTTY?
  - how does mc solve this?
* status of LogList does not refresh correctly, when tasks queue size decrease by rows of magnitude.
