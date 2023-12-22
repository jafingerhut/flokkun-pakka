#include "stdinc.h"
#include "hicut.h"
#include "trie.h"

int binth = 16;      // leaf threashold
float spfac = 2;     // space explosion factor
int opt = 3;         // dimension selection
FILE *fpr;           // ruleset file
FILE *fpt;           // test trace file

int loadrule(FILE *fp, pc_rule *rule){

  int tmp;
  unsigned sip1, sip2, sip3, sip4, siplen;
  unsigned dip1, dip2, dip3, dip4, diplen;
  unsigned proto, protomask;
  int i = 0;

  while(1){

    if(fscanf(fp,"@%d.%d.%d.%d/%d\t%d.%d.%d.%d/%d\t%d : %d\t%d : %d\t%x/%x\n",
        &sip1, &sip2, &sip3, &sip4, &siplen, &dip1, &dip2, &dip3, &dip4, &diplen,
        &rule[i].field[2].low, &rule[i].field[2].high, &rule[i].field[3].low, &rule[i].field[3].high,
        &proto, &protomask) != 16) break;
    if(siplen == 0){
      rule[i].field[0].low = 0;
      rule[i].field[0].high = 0xFFFFFFFF;
    }else if(siplen > 0 && siplen <= 8){
      tmp = sip1<<24;
      rule[i].field[0].low = tmp;
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;
    }else if(siplen > 8 && siplen <= 16){
      tmp = sip1<<24; tmp += sip2<<16;
      rule[i].field[0].low = tmp;
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;
    }else if(siplen > 16 && siplen <= 24){
      tmp = sip1<<24; tmp += sip2<<16; tmp +=sip3<<8;
      rule[i].field[0].low = tmp;
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;
    }else if(siplen > 24 && siplen <= 32){
      tmp = sip1<<24; tmp += sip2<<16; tmp += sip3<<8; tmp += sip4;
      rule[i].field[0].low = tmp;
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;
    }else{
      printf("Src IP length exceeds 32\n");
      return 0;
    }
    if(diplen == 0){
      rule[i].field[1].low = 0;
      rule[i].field[1].high = 0xFFFFFFFF;
    }else if(diplen > 0 && diplen <= 8){
      tmp = dip1<<24;
      rule[i].field[1].low = tmp;
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;
    }else if(diplen > 8 && diplen <= 16){
      tmp = dip1<<24; tmp +=dip2<<16;
      rule[i].field[1].low = tmp;
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;
    }else if(diplen > 16 && diplen <= 24){
      tmp = dip1<<24; tmp +=dip2<<16; tmp+=dip3<<8;
      rule[i].field[1].low = tmp;
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;
    }else if(diplen > 24 && diplen <= 32){
      tmp = dip1<<24; tmp +=dip2<<16; tmp+=dip3<<8; tmp +=dip4;
      rule[i].field[1].low = tmp;
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;
    }else{
      printf("Dest IP length exceeds 32\n");
      return 0;
    }
    if(protomask == 0xFF){
      rule[i].field[4].low = proto;
      rule[i].field[4].high = proto;
    }else if(protomask == 0){
      rule[i].field[4].low = 0;
      rule[i].field[4].high = 0xFF;
    }else{
      printf("Protocol mask error\n");
      return 0;
    }

    i++;
  }
  return i;
}

void parseargs(int argc, char *argv[]) {
  int  c;
  bool ok = 1;
  while ((c = getopt(argc, argv, "b:s:o:r:t:h")) != -1) {
    switch (c) {
        case 'b':
          binth = atoi(optarg);
          break;
        case 's':
          spfac = atof(optarg);
          break;
        case 'o':
          opt = atoi(optarg);
          break;
        case 'r':
          fpr = fopen(optarg, "r");
          break;
        case 't':
          fpt = fopen(optarg, "r");
          break;
        case 'h':
          printf("hicut [-b binth][-s spfac][-o opt][-r ruleset][-t trace]\n");
          printf("mail me: hs1@arl.wustl.edu\n");
          exit(1);
          break;
        default:
          ok = 0;
    }
  }

  if(binth <= 0 || binth > MAXBUCKETS){
    printf("binth should be greater than 0 and less than %d\n", MAXBUCKETS);
    ok = 0;
  }
  if(spfac < 1.0){
    printf("space factor should be >= 1\n");
    ok = 0;
  }
  if(opt < 0 || opt > 3){
    printf("option shoudl be between 0 and 3\n");
    ok = 0;
  }
  if(fpr == NULL){
    printf("can't open ruleset file\n");
    ok = 0;
  }
  if (!ok || optind < argc) {
    fprintf (stderr, "hicut [-b binth][-s spfac][-o opt][-r ruleset][-t trace]\n");
    fprintf (stderr, "Type \"hicut -h\" for help\n");
    exit(1);
  }

  printf("Bucket Size =  %d\n", binth);
  printf("Space Factor = %f\n", spfac);
  printf("Option = %d\n", opt);

}

int main(int argc, char* argv[]){

  int numrules=0;  // actual number of rules in rule set
  struct pc_rule *rule;
  int i,j;
  unsigned int header[MAXDIMENSIONS];
  int matchid, fid;
  char *s = (char *)calloc(200, sizeof(char));

  parseargs(argc, argv);

  while(fgets(s, 200, fpr) != NULL)numrules++;
  rewind(fpr);

  rule = (pc_rule *)calloc(numrules, sizeof(pc_rule));
  numrules = loadrule(fpr, rule);

  printf("the number of rules = %d\n", numrules);
  fflush(stdout);

  //for(i=0;i<numrules;i++){
  //  printf("%d: %x:%x %x:%x %u:%u %u:%u %u:%u\n", i,
  //    rule[i].field[0].low, rule[i].field[0].high,
  //    rule[i].field[1].low, rule[i].field[1].high,
  //    rule[i].field[2].low, rule[i].field[2].high,
  //    rule[i].field[3].low, rule[i].field[3].high,
  //    rule[i].field[4].low, rule[i].field[4].high);
  //}

  trie T(MAXNODES, numrules, binth, spfac, rule, opt);

  printf("*************************\n");
  fflush(stdout);
  printf("number of nodes = %d\n", T.trieSize());
  printf("max trie depth = %d\n", T.trieDepth());
  printf("remove redun = %d\n", T.trieRedun());
  printf("Strored rules = %d\n", T.trieRule());
  printf("Bytes/filter = %f\n", (T.trieSize()*NODESIZE + numrules*RULESIZE + T.trieRule()*RULEPTSIZE)*4/numrules);
  printf("*************************\n");
  fflush(stdout);

  if(fpt != NULL){
    i=0; j=0;
    while(fscanf(fpt,"%u %u %d %d %d %d\n",
          &header[0], &header[1], &header[2], &header[3], &header[4], &fid) == 6){
      i++;

      if((matchid = T.trieLookup(header)) == -1){
        printf("? packet %d match NO rule, should be %d\n", i, fid+1);
        j++;
      }else if(matchid == fid-1){
        //printf("packet %d match rule %d\n", i, matchid);
      }else if(matchid > fid-1){
        printf("? packet %d match lower priority rule %d, should be %d\n", i, matchid+1, fid);
        j++;
      }else{
        printf("! packet %d match higher priority rule %d, should be %d\n", i, matchid+1, fid);
      }
    }
    printf("%d packets are classified, %d of them are misclassified\n", i, j);
    printf("# of bytes accessed/pkt = %f\n", T.trieMemAccess()*4.0/i);
    if (j != 0) {
      exit(1);
    }
  }else{
    printf("No packet trace input\n");
  }

}
