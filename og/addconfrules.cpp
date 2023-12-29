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

#include "stdinc.hpp"
#include <string.h>
#include "llist.hpp"
#include "rules.hpp"

FILE *fpr;       // ruleset file to read
FILE *fpw;       // ruleset file to write

void print_usage(char *progname) {
    printf("%s [-r ruleset_to_read] [-w ruleset_to_write] [-h]\n", progname);
}

void parseargs(int argc, char *argv[]) {
    int	c;
    bool ok = 1;
    fpr = NULL;
    fpw = NULL;
    while ((c = getopt(argc, argv, "r:w:h")) != -1) {
        switch (c) {
	case 'r':
            fpr = fopen(optarg, "r");
            if (fpr == NULL) {
                printf("can't open input ruleset file: %s\n", optarg);
                ok = 0;
            }
            break;
        case 'w':
            fpw = fopen(optarg, "w");
            if (fpw == NULL) {
                printf("can't open output ruleset file: %s\n", optarg);
                ok = 0;
            }
            break;
	case 'h':
            print_usage(argv[0]);
            exit(1);
            break;
	default:
            ok = 0;
        }
    }
    if (fpr == NULL) {
        fprintf(stderr, "Input ruleset file must be specified.\n");
        ok = 0;
    }
    if (fpw == NULL) {
        fprintf(stderr, "Output ruleset file must be specified.\n");
        ok = 0;
    }
    if (!ok || optind < argc) {
        print_usage(argv[0]);
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    int numrules = 0;  // actual number of rules in rule set
    llist rules_in;
    llist rules_opt;
    llist rules_out;
    int i;
    int compare_result;
    struct llist_node *node_in, *node_out;
    
    parseargs(argc, argv);
    
    loadrule(fpr, rules_in);
    numrules = rules_in.length();
    printf("the number of rules = %d\n", numrules);
    
    for (i = 0, node_in = rules_in.first_node(); node_in != NULL; node_in = rules_in.next_node(node_in), i++) {
        struct pc_rule *rule_in = (struct pc_rule *) rules_in.node_item(node_in);
        char buf[512];
        snprintf(buf, sizeof(buf), "Original rule %d", i+1);
        int sz = strlen(buf) + 1;
        rule_in->comment = (char *) malloc(sz);
        strncpy(rule_in->comment, buf, sz);
    }

    int num_unmatchable_removed = remove_unmatchable(rules_in, rules_opt);
    printf("removed %d unmatchable rules\n", num_unmatchable_removed);
    
    int num_esub = 0;
    int num_lsub = 0;
    int num_eq = 0;
    int num_conf = 0;
    num_unmatchable_removed = 0;
    int num_intersection_rules_added = 0;
    for (i = 0, node_in = rules_opt.first_node(); node_in != NULL; node_in = rules_opt.next_node(node_in), i++) {
        struct pc_rule *rule_in = (struct pc_rule *) rules_opt.node_item(node_in);
        llist maybe_next_rules_out;
        bool delete_unmatchable_rule = false;
        int num_added_for_this_rule_in = 0;
        for (node_out = rules_out.first_node(); node_out != NULL; node_out = rules_out.next_node(node_out)) {
            struct pc_rule *rule_out = (struct pc_rule *) rules_out.node_item(node_out);
            compare_result = compare_rules(rule_out, rule_in);
            switch (compare_result) {
            case RULE_COMPARE_DISJOINT:
                break;
            case RULE_COMPARE_EARLIER_STRICT_SUBSET:
                ++num_esub;
                break;
            case RULE_COMPARE_LATER_STRICT_SUBSET:
                ++num_lsub;
                delete_unmatchable_rule = true;
                break;
            case RULE_COMPARE_EQUAL:
                ++num_eq;
                delete_unmatchable_rule = true;
                break;
            case RULE_COMPARE_CONFLICT:
                {
                    char buf[1024];
                    ++num_conf;
                    ++num_added_for_this_rule_in;
                    struct pc_rule *new_rule = alloc_rule();
                    snprintf(buf, sizeof(buf),
                             "New rule %d combined from (%s) and (%s)",
                             num_conf, rule_out->comment, rule_in->comment);
                    int sz = strlen(buf) + 1;
                    new_rule->comment = (char *) malloc(sz);
                    strncpy(new_rule->comment, buf, sz);
                    rule_intersection(new_rule, rule_out, rule_in);
                    // TODO: Verify that return value is not empty rule,
                    // which would be some kind of bug in this code.
                    maybe_next_rules_out &= new_rule;
                }
                break;
            default:
                {
                    char buf[512];
                    snprintf(buf, sizeof(buf),
                             "compare_result has unexpected value %d.  Internal error.\n",
                             compare_result);
                    fatal(buf);
                }
            }
            if (delete_unmatchable_rule) {
                break;
            }
            maybe_next_rules_out &= rule_out;
        }
        if (delete_unmatchable_rule) {
            ++num_unmatchable_removed;
        } else {
            // Erase current contents of rules_out and replace it with
            // the contents of maybe_next_rules_out.
            num_intersection_rules_added += num_added_for_this_rule_in;
            rules_out.clear();
            rules_out.push_list(maybe_next_rules_out);
            rules_out &= rule_in;
        }
    }
    writerule(fpw, rules_out);
    printf("%10d input rules\n", numrules);
    printf("%10d (avg %.1lf / rule) unmatchable rules removed\n",
           num_unmatchable_removed,
           (double) num_unmatchable_removed / numrules);
    printf("%10d (avg %.1lf / rule) new intersection rules added\n",
           num_intersection_rules_added,
           (double) num_intersection_rules_added / numrules);
    printf("%10d (avg %.1lf / rule) output rules\n",
           rules_out.length(),
           (double) rules_out.length() / numrules);
    
    printf("%10d (avg %.1lf / rule) earlier is strict subset\n",
           num_esub,
           (double) num_esub / numrules);
    printf("%10d (avg %.1lf / rule) conflict\n",
           num_conf,
           (double) num_conf / numrules);
}
