#include "stdinc.h"
#include "llist.h"

#define MAXDIMENSIONS 5

struct range {
    unsigned low;
    unsigned high;
};

struct pc_rule {
    struct range field[MAXDIMENSIONS];
};

const int RULE_COMPARE_DISJOINT = 0;
const int RULE_COMPARE_EARLIER_STRICT_SUBSET = 1;
const int RULE_COMPARE_LATER_STRICT_SUBSET = 2;
const int RULE_COMPARE_EQUAL = 3;
const int RULE_COMPARE_CONFLICT = 4;

unsigned int mask(int k);
void loadrule(FILE *fp, llist& rule_list);
void min_max_to_prefix32(unsigned int low, unsigned int high,
                         unsigned int *out_prefix,
                         int *out_prefix_len);
bool rules_disjoint(struct pc_rule *r1, struct pc_rule *r2);
bool rule_subset(struct pc_rule *r1, struct pc_rule *r2);
int compare_rules(struct pc_rule *r1, struct pc_rule *r2);
void print_rule(FILE *fp, struct pc_rule *r);
void writerule(FILE *fp, llist& rule_list);
