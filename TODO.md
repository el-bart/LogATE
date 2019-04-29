# TODOs

## library part

* saving filter tree
* loading filter tree
* sorting?
* filter accepting all nodes, but dropping a given set of fields when displaying
* multi-threaded logs processing
* binary split node (matching and non-matching nodes as children)
* Explode root node should have all logs in it!
* adding a child to Explode maybe should mean adding to all subnodes? how to do this? factory?
* search inside a given node set
* maybe it would be better to keep all logs internally as a compact-formed text line and just decode on a fly?


## UI

* key shortcuts for tabs switching fast
* colored highlighting of searched elements
* auto-hiding columns used for value-based "explode" filtering
* runtime edition of silent tags
* runtime edition of priority tags
* export logs from a given filter to a file
* show number of logs per subtree
* display stats: number of lines, location in file (floating point % + line number)
* move all (child) filters to a given log line (or close to it, when given log is not in all of them)
* display number of errors from networking stack


## misc

* runtime image - autobuilding version, taht can be used just by downloading from docker hub.
