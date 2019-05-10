# TODOs

## library part

* saving filter tree
* loading filter tree
* sorting?
* filter accepting all nodes, but dropping a given set of fields when displaying
* binary split node (matching and non-matching nodes as children)
* adding a child to Explode maybe should mean adding to all subnodes? how to do this? factory?
* search inside a given node set via regex (string is already implemented)
* Filter::Grep should filter-out grep-field optionally only


## UI

* key shortcuts for tabs switching fast
* colored highlighting of searched elements
* auto-hiding columns used for value-based "explode" filtering
* runtime edition of silent tags
* runtime edition of priority tags
* export logs from a given filter to a file
* display stats: number of lines, location in file (floating point % + line number)
* display number of errors from networking stack
* handle failed node addition gracefully
* trimmed fields should not be visible
* make deleting root node possible, if there is just one child - then it will pivot root element with its child (useful for dropping non-interesting logs)


## misc

* runtime image - autobuilding version, taht can be used just by downloading from docker hub.
* run profiler to see where the bottlenecks are
