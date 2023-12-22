TODO:

+ Replace all uses of pow(2,k) with pow2(k) that I added to stdinc.h
  in song-woo.
+ Find all calls to log() and see if they can be replaced with
  something that does fast exact integer arithmetic, not floating
  point.  OK if it is used to report statistics, but not in the packet
  classification part of code.
+ See if there is any use of float or double anywhere in packet
  classification code, and replace with integer arithmetic if so.


# song-bv

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors nor warnings
+ `g++ -O3 -Wall` v13.2.0 with no errors.  2 warnings about ISO C++17
  does not allow 'register' storage class specifier.

All tests pass!

Handles `bc1*` test cases without any scaling problems, due to how
algorithm works.


# song-hicut

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors nor warnings
+ Similarly with g++ v13.2.0

All tests pass except: fw1_5K fw1_10K

Failures are for known reasons that parameter settings are too small,
and their names are printed on a failure.

Handles `bc1*` test cases up to `bc1_k40`, but exhausts freelist for
`bc1_k100`.  The worst case bytes/lookup and bytes/filter values grow
very fast with k, which I suspect is a property of the hicut algorithm
and those rule sets.


# song-hypercut

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors nor warnings
+ Similarly with g++ v13.2.0

All tests pass except: fw1_1K fw1_5K fw1_10K ipc1_5K ipc1_10K

Failures are for known reasons that parameter settings are too small,
and their names are printed on a failure.

Similar to song-hicut in its handling of bc1* rule sets.


# song-rfc

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors nor warnings
+ Similarly with g++ v13.2.0

All tests pass except: fw1_5K fw1_10K ipc1_5K ipc1_10K

Failures are for known reasons that parameter settings are too small,
and their names are printed on a failure.


## Table size of RFC for bc1 rule sets

Command used to generate the file of rules for a particular value of
K:
```
./gen-bc1.py --k ${K} > bc1_k${K}
```

Command used to find the size of the table required in phase 3 of the
RFC implementation in directory `song-rfc`:

```
./rfc -r bc1_k${K} | grep -A 1 'start phase 3'
```

| k value for rule set | Table size required in phase 3 | Formula that matches this value |
| ------------------------- | ------------------------------------------------------- | ------------------------------- |
|   5 |        1332 |   (5+1)^4 +   (5+1)^2 |
|  25 |     457,652 |  (25+1)^4 +  (25+1)^2 |
|  40 |   2,827,442 |  (40+1)^4 +  (40+1)^2 |
| 100 | 104,070,602 | (100+1)^4 + (100+1)^2 |


# song-tss

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors nor warnings
+ Similarly with g++ v13.2.0

All tests pass!

Handles `bc1*` test cases without any scaling problems, due to how
algorithm works.


# song-woo

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors nor warnings
+ Similarly with g++ v13.2.0

All tests pass except: fw1_5K fw1_10K

Failures are for known reasons that parameter settings are too small,
and their names are printed on a failure.

Fails with `genrules/bc1_k5`, and all other `bc1*` files.  This seems
like it is either a bug in the song-woo code, or a pretty severe
limitation in the algorithm.

TODO: Consider investigating whether this is a bug in the
implementation that can be fixed, without a lot of effort.
