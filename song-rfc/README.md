This directory contains a modified version of the source code in the
`orig/song-rfc` directory.

The first changes made were eliminating errors and warnings when
compiling with `g++ -Wall` when using these G++ versions:

+ 9.4.0
+ 13.2.0

Some occurrences of out-of-bounds array accesses and other memory
issues detect by Valgrind's memcheck tool have been fixed.

Several occurrences of very similar code have been factored into
functions, reducing the length of the code, and hopefully it is at
least as clear to understand it.

Improvements that might be interesting to make in the future:

+ Replace the linear search for an equal matching rule list with
  something faster, e.g. a hash table or balanced binary search tree.
  This would not add any features, but would significantly speed up
  the code when the number of distinct equivalence id's is in the tens
  of thousands.
+ Add the notion of "Adjacency Groups" as described in Sec. 7.1 of
  Gupta and McKeown's 1999 paper on RFC.  This could help
  significantly reduce the memory rqeuired for the RFC tables, but
  only when there are identical actions across rules.
+ Generalize the kinds of classification problem instances handled to
  include instances of the group-based classifiction problem.
