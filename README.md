# Introduction

This repository contains programs and information about the _packet
classification problem_.

I do not know the Icelandic language, but according to Google
Translate, "flokkun pakka" is Icelandic for "packet classification".
I used Google Translate to find the words in many languages for
"packet classification", and liked Icelandic's version the best.


# The Filter Set Analyzer program

A set of programs and data files originally published in 2005 called
_ClassBench_ can be found
[here](https://github.com/jafingerhut/classbench).

The ClassBench research papers mention a program called the _Filter
Set Analyzer_, but this program was never published.  I have attempted
to implement my own version of this program in the programming
language [Clojure](https://clojure.org).

I may publish a precompiled binary that requires fewer setup steps to
run, but for now you must install the following to run it:

+ A Java Virtual Machine.  Very likely any version 8 or later will
  suffice.  I have tested most with OpenJDK 17.  Binary installation
  packages of the JDK for many operating systems can be found at the
  [Adoptium web site](https://adoptium.net/temurin).
+ [Clojure](https://clojure.org).  See install instructions
  [here](https://clojure.org/guides/install_clojure).

After installing those, you can run it as shown in the following
example, which reads the filter set from the file
`orig/song-filterset/acl1_10K`, and writes a parameter file to
`acl1_10K_seed`:

```bash
./scripts/test_write_ipv4_classbench_parameter_file.sh orig/song-filterset/acl1_10K acl1_10K_seed
```

