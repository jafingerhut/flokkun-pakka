Possible data structures for representing rules and rule sets:

For a rule, a map with these keys:

+ :field - value is a vector of "match criteria" (see below).
+ :priority - a number.  In many cases a rule set might be represented
  as a vector, where the relative priority is implied by the relative
  order of rules in the vector, and thus :priority would not be
  specified for the rules.
+ other new fields as desired in particular use cases, such as:
  + :comment - a string with notes about where the rule comes from


A match criteria can be one of many different possible values, but
here are what I expect some common cases to be:

+ {:kind :range :low 10 :high 20}
+ {:kind :ternary :value <number> :mask <number>}
+ {:kind :prefix :value <number> :prefix-length <number>}
+ {:kind :union :match-criteria <set of match criteria>}
  + can be used to specify group-based classification problems, and
    also represent things that are not typically expected to be
    supported in such problems, such as a union of match criteria
    where at least one is ternary, and at least one is range.
