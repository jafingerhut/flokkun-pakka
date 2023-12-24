/*
 * Author: J. Andrew Fingerhut (andy.fingerhut@gmail.com)
 *
 * Read a set of rules in ClassBench IPv4 5-tuple syntax.
 *
 * Write out a set of rules in the same format, with all conflicting
 * rules added, implemented as described by algorithm AddNewFilter in
 * Figure 5 of section III.A. of the following paper:
 *
 * Hari Adiseshu, Subhash Suri, Guru Parulkar, "Detecting and
 * resolving packet filter conflicts", 2000, INFOCOM 2000, DOI
 * 10.1109/INFCOM.2000.832496,
 * https://www.researchgate.net/publication/3842432_Detecting_and_resolving_packet_filter_conflicts
 * 
 */

#include "stdinc.h"
#include "og.h"
#include "llist.h"
#include "rules.h"

FILE *fpr;       // ruleset file

void parseargs(int argc, char *argv[]) {
    int	c;
    bool ok = 1;
    fpr = NULL;
    while ((c = getopt(argc, argv, "r:h")) != -1) {
        switch (c) {
	case 'r':
            fpr = fopen(optarg, "r");
            if (fpr == NULL) {
                printf("can't open ruleset file: %s\n", optarg);
                ok = 0;
            }
            break;
	case 'h':
            printf("%s [-r ruleset] [-h]\n", argv[0]);
            exit(1);
            break;
	default:
            ok = 0;
        }
    }
    if (fpr == NULL) {
        fprintf(stderr, "Ruleset file must be specified.\n");
    }
    if (!ok || optind < argc) {
        fprintf(stderr, "rfc [-r ruleset] [-h]\n");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    int numrules = 0;  // actual number of rules in rule set
    llist rule;
    int i, j;
    int compare_result;
    
    parseargs(argc, argv);
    
    loadrule(fpr, rule);
    numrules = rule.length();
    printf("the number of rules = %d\n", numrules);
    
    int num_esub = 0;
    int num_lsub = 0;
    int num_eq = 0;
    int num_conf = 0;
    struct llist_node *r1n, *r2n;
    for (i = 0, r1n = rule.first_node(); r1n != NULL; r1n = rule.next_node(r1n), i++) {
        struct pc_rule *r1 = (struct pc_rule *) rule.node_item(r1n);
        for (j = i+1, r2n = rule.next_node(r1n); r2n != NULL; r2n = rule.next_node(r2n), j++) {
            struct pc_rule *r2 = (struct pc_rule *) rule.node_item(r2n);
            compare_result = compare_rules(r1, r2);
            switch (compare_result) {
            case RULE_COMPARE_DISJOINT:
                // print nothing
                break;
            case RULE_COMPARE_EARLIER_STRICT_SUBSET:
                ++num_esub;
                break;
            case RULE_COMPARE_LATER_STRICT_SUBSET:
                ++num_lsub;
                break;
            case RULE_COMPARE_EQUAL:
                ++num_eq;
                break;
            case RULE_COMPARE_CONFLICT:
                ++num_conf;
                break;
            default:
                {
                    char buf[512];
                    snprintf(buf, sizeof(buf),
                             "compare_result has unexpcted value %d.  Internal error.\n",
                             compare_result);
                    fatal(buf);
                }
            }
        }
    }
    printf("// Number of rules: %d\n", numrules);
    printf("// Number of rule pairs that have each relationship:\n");
    printf("//     %10d (avg %.1lf / rule) earlier is strict subset\n",
           num_esub,
           (double) num_esub / numrules);
    printf("//     %10d (avg %.1lf / rule) later is strict subset\n",
           num_lsub,
           (double) num_lsub / numrules);
    printf("//     %10d (avg %.1lf / rule) equal\n",
           num_eq,
           (double) num_eq / numrules);
    printf("//     %10d (avg %.1lf / rule) conflict\n",
           num_conf,
           (double) num_conf / numrules);
}
