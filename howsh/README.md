howsh
=====

This is a demonstration of implementing a simple shell with C and Unix.

It handles pipes and (optional) redirection of the following form:
```
command1 < input | command2 | ... | commandn > output
```
The space between the redirection character and the filename is optional.
Commands are space-separated words; there is currently no support for quotes or escape characters.

Builtin functions include `cd` and `exit`; they must be the only command in a pipeline,
which is not much of a restriction because they don't have any I/O.

Some material adapted from:
* https://github.com/brenns10/lsh
* https://github.com/tokenrove/build-your-own-shell