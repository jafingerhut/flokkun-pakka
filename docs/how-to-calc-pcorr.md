Let r be a rule with prefix match criteria for the source and
destination IP address fields, where p(s) is the prefix length for the
source address, and p(d) is the prefix length of the destination
address.

Definition: a(r) is the largest integer such that

+ a(r) <= min(p(s), p(d)), and
+ the most significant a(r) bits of the source address are equal to
  the most significant a(r) bits of the destination address.

d(r) is true if a(r) < min(p(s), p(d)), which is the case if and only
if there is an exact bit position in both prefixes that are different
from each other.

If d(r) is false, a(r) = min(p(s), p(d)), which is the case if and
only if all exact match bits in both prefixes are the same, and the
next bit position is a wildcard bit position in either one of the
prefixes, or both of them (or there is not next bit position because
p(s)=p(d)=the full bit width of the field).


So, for each value j of r in [0,32], there will be:

(a) a set of rules with a(r)=j and d(r)=true
(b) a set of rules with a(r)=j and d(r)=false

Let c(j,true) be the count of the rules in the set (a), and c(j,false)
be the count of the rules in the set (b).

c(32,true) must be 0, because it is not possible for 32 < min(p(s),
p(d)) for 32-bit addresses.

I am guessing that each of the 32 values in the parameter file will be
a sum of some of a subset of these c(j,b) values.  If so, which
subsets?

What is the formula for index 1?


Formula for index 32?

Among all rules where SA and DA match each other in the first 31 bits,
_and_ are exact match in the 32nd bit, calculate (# that match in 32nd
bit) / (# that match in 32nd bit + # that mismatch in 32nd bit).

c(32,false) / [ c(32,false) + c(31,true) ]


Formula for index 31?

Among all rules where SA and DA match each other in the first 30 bits,
_and_ are exact match in the 32nd bit, calculate (# that match in 31st
bit) / (# that match in 31st bit + # that mismatch in 31st bit).

[ c(32,false) + c(31,true) + c(31,false) ] /
[ c(32,false) + c(31,true) + c(31,false) + c(30,true) ]
