#include "stdinc.h"
#include "og.h"
#include "llist.h"

FILE *fpr;       // ruleset file

const int RULE_COMPARE_DISJOINT = 0;
const int RULE_COMPARE_EARLIER_STRICT_SUBSET = 1;
const int RULE_COMPARE_LATER_STRICT_SUBSET = 2;
const int RULE_COMPARE_EQUAL = 3;
const int RULE_COMPARE_CONFLICT = 4;

unsigned int mask(int k)
{
    if (k < 0 || k > 32) {
        char buf[512];
        snprintf(buf, sizeof(buf), "mask parameter k=%d must be in range [0,32]\n", k);
        fatal(buf);
    }
    if (k == 32) {
        return 0xffffffff;
    } else {
        return (((unsigned int) 1) << k) - 1;
    }
}

void loadrule(FILE *fp, llist& rule_list)
{
    unsigned sip1, sip2, sip3, sip4, siplen, sip;
    unsigned dip1, dip2, dip3, dip4, diplen, dip;
    unsigned proto, protomask;
    int i = 0;
    char buf[512];
    struct pc_rule *newrule;
    
    while (1) {
        newrule = (struct pc_rule *) malloc(sizeof(struct pc_rule));
        if (fscanf(fp,"@%u.%u.%u.%u/%u %u.%u.%u.%u/%u %u : %u %u : %u %x/%x\n",
                   &sip1, &sip2, &sip3, &sip4, &siplen,
                   &dip1, &dip2, &dip3, &dip4, &diplen,
                   &(newrule->field[3].low), &(newrule->field[3].high),
                   &(newrule->field[4].low), &(newrule->field[4].high),
                   &proto, &protomask) != 16) {
            break;
        }
        if (sip1 > 255 || sip2 > 255 || sip3 > 255 || sip4 > 255) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has source IPv4 address %u.%u.%u.%u with some value greater than 255, which is not supported.\n",
                     i+1, sip1, sip2, sip3, sip4);
            fatal(buf);
        }
        if (dip1 > 255 || dip2 > 255 || dip3 > 255 || dip4 > 255) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has destination IPv4 address %u.%u.%u.%u with some value greater than 255, which is not supported.\n",
                     i+1, dip1, dip2, dip3, dip4);
            fatal(buf);
        }
        if ((newrule->field[3].low > newrule->field[3].high) ||
            (newrule->field[3].high > 65535)) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has L4 source port range [%u,%u] with lo larger than hi, or hi greater than 65535, which is not supported.\n",
                     i+1, newrule->field[3].low, newrule->field[3].high);
            fatal(buf);
        }
        if ((newrule->field[4].low > newrule->field[4].high) ||
            (newrule->field[4].high > 65535)) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has L4 source port range [%u,%u] with lo larger than hi, or hi greater than 65535, which is not supported.\n",
                     i+1, newrule->field[4].low, newrule->field[4].high);
            fatal(buf);
        }
        if (proto > 255) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has protocol %u larger than 255, which is not supported.\n",
                     i+1, proto);
            fatal(buf);
        }
        if (siplen < 0 || siplen > 32) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has source IPv4 prefix length %u outside of range [0,32], which is not supported.\n",
                     i+1, siplen);
            fatal(buf);
        }
        sip = ((sip1 << 24) | (sip2 << 16) | (sip3 << 8) | sip4);
        unsigned int smask = mask(32-siplen);
        if ((sip & smask) != 0) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has source IPv4 address %u.%u.%u.%u and prefix len %u with non-0 bits after the prefix length, which is not supported.\n",
                     i+1, sip1, sip2, sip3, sip4, siplen);
            fatal(buf);
        }
        newrule->field[0].low = sip;
        newrule->field[0].high = sip + smask;
        if (diplen < 0 || diplen > 32) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has destination IPv4 prefix length %u outside of range [0,32], which is not supported.\n",
                     i+1, diplen);
            fatal(buf);
        }
        dip = ((dip1 << 24) | (dip2 << 16) | (dip3 << 8) | dip4);
        unsigned int dmask = mask(32-diplen);
        if ((dip & dmask) != 0) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has destination IPv4 address %u.%u.%u.%u and prefix len %u with non-0 bits after the prefix length, which is not supported.\n",
                     i+1, dip1, dip2, dip3, dip4, diplen);
            fatal(buf);
        }
        newrule->field[1].low = dip;
        newrule->field[1].high = dip + dmask;
        if (protomask == 0xff) {
            newrule->field[2].low = proto;
            newrule->field[2].high = proto;
        } else if (protomask == 0) {
            newrule->field[2].low = 0;
            newrule->field[2].high = 0xff;
        } else {
            fatal("Protocol mask error\n");
        }
        rule_list &= newrule;
        ++i;
    }
}

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

bool rules_disjoint(struct pc_rule *r1, struct pc_rule *r2)
{
    int i;
    for (i = 0; i < MAXDIMENSIONS; i++) {
        if (r1->field[i].high < r2->field[i].low) {
            return true;
        }
        if (r2->field[i].high < r1->field[i].low) {
            return true;
        }
    }
    return false;
}

bool rule_subset(struct pc_rule *r1, struct pc_rule *r2)
{
    int i;
    for (i = 0; i < MAXDIMENSIONS; i++) {
        if ((r1->field[i].low >= r2->field[i].low) &&
            (r1->field[i].high <= r2->field[i].high))
        {
            // Then for this field i, r1 is a subset of r2.  Keep
            // checking the rest of the fields.
        } else {
            return false;
        }
    }
    return true;
}

int compare_rules(struct pc_rule *r1, struct pc_rule *r2)
{
    bool earlier_subset, later_subset;
    if (rules_disjoint(r1, r2)) {
        return RULE_COMPARE_DISJOINT;
    }
    earlier_subset = rule_subset(r1, r2);
    later_subset = rule_subset(r2, r1);
    if (earlier_subset && later_subset) {
        return RULE_COMPARE_EQUAL;
    } else if (earlier_subset) {
        return RULE_COMPARE_EARLIER_STRICT_SUBSET;
    } else if (later_subset) {
        return RULE_COMPARE_LATER_STRICT_SUBSET;
    }
    return RULE_COMPARE_CONFLICT;
}

void print_rule(struct pc_rule *r)
{
    int i;
    for (i = 0; i < MAXDIMENSIONS; i++) {
        if (i != 0) {
            printf(" ");
        }
        printf("[%u, %u]", r->field[i].low, r->field[i].high);
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
                print_rule(r1);
                printf("\n");
                printf("    // R%d", j+1);
                print_rule(r2);
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
                print_rule(r1);
                printf("\n");
                printf("    // R%d", j+1);
                print_rule(r2);
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
                             "compare_result has unexpcted value %d.  Internal error.\n",
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
