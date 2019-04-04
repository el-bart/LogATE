the idea is to provide a set of pretty-printers that will:
1) provide minimal, readable output (user defines what "readable" means, by selecting: key=value, or value outputs)
2) drop fields that are marked as "trimmed" by filter tree, so that only relevant pieces of information are displayed
3) sort fields by "display order" provided by user (eg. timestamp and priority should probably be first, etc.)

this will be a bit of play to implement both, thus is left for later time, if project proves to be useful in practice.
