# song-bv

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors nor warnings

All tests pass!


# song-hicut

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors.  About a dozen warnings about
  comparison of integer eexpressions of different signedness:
  'unsigned int' and 'int'.

All tests pass except: fw1_5K fw1_10K

Failures are for known reasons that parameter settings are too small,
and their names are printed on a failure.


# song-hypercut

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors.  About a dozen warnings about
  comparison of integer eexpressions of different signedness:
  'unsigned int' and 'int'.

All tests pass except: fw1_1K fw1_5K fw1_10K ipc1_5K ipc1_10K

Failures are for known reasons that parameter settings are too small,
and their names are printed on a failure.


# song-rfc

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors nor warnings

All tests pass except: fw1_5K fw1_10K ipc1_5K ipc1_10K

Failures are for known reasons that parameter settings are too small,
and their names are printed on a failure.


# song-tss

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors.  Only 2 warnings about
  comparison of integer eexpressions of different signedness:
  'unsigned int' and 'int'.

All tests pass!


# song-woo

Compiles with:
+ `g++ -O3 -Wall` v9.4.0 with no errors.  About 10 warnings about
  comparison of integer eexpressions of different signedness:
  'unsigned int' and 'int'.

All tests pass except: fw1_5K fw1_10K

Failures are for known reasons that parameter settings are too small,
and their names are printed on a failure.
