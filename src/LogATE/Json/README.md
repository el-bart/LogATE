# yet another JSON...?

why on earth would one write her own JSON parser?!
well - i was surprised to learn that it does make sense, myself. ;)

the thing is that input JSON parsing takes a significant amount of time, and since this happens on the input,
it cannot be simply palatalized.
the proposed solution is to, instead of parsing JSON, have a simple input pre-processor, that separates one JSON at a time.
then such a "selection" is passed to a threads pool for further processing (i.e. "real parsing").
this way a lot of time is saved on the input end of the system.

this "pre-parser" shall be:
* resumable (i.e. process input byte by byte).
* fast at a cost of precision (i.e. it is allowed to not validate exact syntax of strings, as long as it is fast).
* never rejects a valid JSON.
* never damages a valid JSON.
* remove all unnecessary whitespaces.
* JSON can start with object or array - single element "JSONs", though valid, can be ignored here.
