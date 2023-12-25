#include "stdinc.hpp"
#include "og.hpp"
#include "llist.hpp"
#include "rules.hpp"

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
    printf("// the number of rules = %d\n", numrules);
    
    bool show_edge_labels = false;
    int num_esub = 0;
    int num_lsub = 0;
    int num_eq = 0;
    int num_conf = 0;
    printf("digraph overlap_graph {\n");
    printf("    rankdir=\"LR\";\n");
    printf("    node [shape=\"box\"];\n");
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
                printf("    R%d -> R%d [", i+1, j+1);
                if (show_edge_labels) {
                    printf("label=\"esub\" ");
                }
                printf("color=\"green\"];\n");
                break;
            case RULE_COMPARE_LATER_STRICT_SUBSET:
                ++num_lsub;
                printf("    // R%d", i+1);
                print_rule(stdout, r1);
                printf("\n");
                printf("    // R%d", j+1);
                print_rule(stdout, r2);
                printf("\n");
                printf("    R%d -> R%d [", i+1, j+1);
                if (show_edge_labels) {
                    printf("label=\"lsub\" ");
                }
                printf("color=\"red\" style=\"bold\"];\n");
                break;
            case RULE_COMPARE_EQUAL:
                ++num_eq;
                printf("    // R%d", i+1);
                print_rule(stdout, r1);
                printf("\n");
                printf("    // R%d", j+1);
                print_rule(stdout, r2);
                printf("\n");
                printf("    R%d -> R%d [", i+1, j+1);
                if (show_edge_labels) {
                    printf("label=\"eq\" ");
                }
                printf("color=\"red\" style=\"bold\"];\n");
                break;
            case RULE_COMPARE_CONFLICT:
                ++num_conf;
                printf("    R%d -> R%d [", i+1, j+1);
                if (show_edge_labels) {
                    printf("label=\"conf\" ");
                }
                printf("color=\"blue\"];\n");
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
        }
    }
    printf("}\n");
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
