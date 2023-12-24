#include <stdio.h>
#include "rules.h"

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
    llist rule;
    
    parseargs(argc, argv);
    loadrule(fpr, rule);
    numrules = rule.length();
    printf("the number of rules read = %d\n", numrules);
    writerule(fpw, rule);
    fclose(fpr);
    fclose(fpw);
}
