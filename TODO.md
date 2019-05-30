# TODOs

## library part

* saving filter tree
* loading filter tree
* sorting?
* maybe adding a child to Explode maybe should mean adding to all subnodes? how to do this? factory?
* search inside a given node set via regex (string is already implemented)
* add support for arrays in all the places in the code
* think about nice syntax for Path support of arrays
* speed up input logs parsing (main bottleneck now):
  - consider adding simple non-UTF8 JSON cutter, that will extract 1 JSON from input socket and pass it on for parsing.
  - use thread pool for parsing JSONs and appending them to the filter tree.
  - replace input streams with raw sockets for performance.
* filter that saves all logs after spotting a given pattern (grep-like); add option to save N logs before as well.


## UI

* key shortcuts for tabs switching fast (small 'm'arks and big 'Marks'?)
* colored highlighting of searched elements
* runtime edition of silent tags
* runtime edition of priority tags
* export logs from a given filter to a file
* make deleting root node possible, if there is just one child - then it will pivot root element with its child (useful for dropping non-interesting logs)
* embed version tag (if present) and commit hash into a binary, so that it can be displayed on request
* escape key handling (w/o extra 0.5s delay, plz)
* generic mechanism for searching inside whole data source (usefull for different small-volume windows, i.e. non-log-lists)
* save given logs to a file


## misc
