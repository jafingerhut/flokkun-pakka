#include "stdinc.h"
#include "woo.h"
#include "trie.h"

FILE *fpr;      // ruleset file
FILE *fpt;      // trace file
int bucketSize = 16; 
int nbit0=0;
int nbit1=0;
int nbit2=0;
int nbit3=0;
int nbit4=0;

struct prefix *expand_prefix(unsigned ai, unsigned aj){

  unsigned mask = 0xFFFFFFFF;
  int  k;
  struct prefix *pf = (prefix *)malloc(sizeof(prefix));
  int i = 0;

  if(ai == aj) {
    //printf("a: Prefix = %4x, Mask = %4x\n", ai, mask);
    pf->value[i] = ai;
    pf->length[i] = 16;
    pf->nvalid = 1;
    return pf;
  } 
            
  while(1) {
    for(k=0; (ai & (int)pow(2,k)) == 0 && k <= 15 ; k ++);
    if(ai + pow(2,k) - 1 == aj) {		
      //printf("b: Prefix = %4x, Mask = %4x, k=%d\n", ai>>k<<k, mask<<k, k);
      pf->value[i]=ai>>k<<k;
      pf->length[i]=16-k;
      pf->nvalid=i+1;
      return pf;
    }else if (ai + pow(2,k) - 1 < aj) {
      //printf("c: Prefix = %4x, Mask = %4x, k=%d\n", ai>>k<<k, mask<<k, k);
      pf->value[i]=ai>>k<<k;
      pf->length[i]=16-k;
      ai = ai + (int)pow(2,k);
    }else{
      break;
    } // end if
        
    i++;

  } // end while
	
  if(ai == aj){
    //printf("d: Prefix = %4x, Mask = %4x\n", ai, mask);
    pf->value[i]=ai;
    pf->length[i]=16;
    pf->nvalid=i+1;
    return pf;
  } // end if

  while(ai < aj) {

    for(k=15; ((ai ^ aj) & (int)pow(2,k)) == 0 ; k--);
    if(ai + pow(2,k+1) - 1 == aj) {
      //printf("e: Prefix = %4x, Mask = %4x\n", ai>>(k+1)<<(k+1), mask<<(k+1));
      pf->value[i]=ai>>(k+1)<<(k+1);
      pf->length[i]=15-k;
      pf->nvalid=i+1;
      return pf;
    } else {
      //printf("f: Prefix = %4x, Mask = %4x\n", ai>>k<<k, mask<<k);
      pf->value[i]=ai>>k<<k;
      pf->length[i]=16-k;
      ai = ai + (int)pow(2,k);
    } // end if	
 
    i++;
		
  } // end while
	
  //printf("g: Prefix = %4x, Mask = %4x\n", ai, mask);
  pf->value[i]=ai;
  pf->length[i]=16;
  pf->nvalid=i+1;
        
  return pf;

}

int loadrule(FILE *fp, pc_rule *rule){
  
  int tmp;
  unsigned sip1, sip2, sip3, sip4, siplen;
  unsigned dip1, dip2, dip3, dip4, diplen;
  unsigned proto, protomask;
  int i = 0;
  
  while(1){
    
    if(fscanf(fp,"@%d.%d.%d.%d/%d\t%d.%d.%d.%d/%d\t%d : %d\t%d : %d\t%x/%x\t%x/%x\n", 
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
    rule[i].id = i;
    i++;
  }
  return i;
}

void parseargs(int argc, char *argv[]) {
  int	c;
  bool	ok = 1;
  while ((c = getopt(argc, argv, "b:r:t:hu:v:")) != -1) {
    switch (c) {
    	case 'b':
    	  bucketSize = atoi(optarg);
    	  break;
	case 'r':
	  fpr = fopen(optarg, "r");
        break;
      case 't':
        fpt = fopen(optarg, "r");
        break;
	case 'h':
	  printf("woo [-b bucketSize][-r ruleset][-t trace][-u sipbits][-v dipbits]\n");
	  exit(1);
	  break;
	case 'u':
	  nbit0 = atoi(optarg);
	  break;
	case 'v':
	  nbit1 = atoi(optarg);
	  break;
	default:
	  ok = 0;
    }
  }
  if(bucketSize <= 0 || bucketSize > MAXBUCKETS){
    printf("binth should be greater than 0 and less than %d\n", MAXBUCKETS);
    ok = 0;
  }		
  if(fpr == NULL){
    printf("can't open ruleset file\n");
    ok = 0;
  }
  if(nbit0 <0 || nbit0 > 16){
    printf("sip bits error\n");
    ok = 0;
  }
  if(nbit1 <0 || nbit1 > 16){
    printf("dip bits error\n");
    ok = 0;
  }  
  if (!ok || optind < argc) {
    fprintf (stderr, "woo [-b bucketSize][-r ruleset][-t trace][-u sipbits][-v dipbits]\n\n");
    fprintf (stderr, "Type \"woo -h\" for help\n");
    exit(1);
  }

}

int main(int argc, char* argv[]){

  int numrules=0;  // actual number of rules in rule set
  int numexprules= 0;
  struct pc_rule *rule; 
  struct pc_rule *exprule;
  unsigned header[MAXDIMENSIONS];
  int matchid, fid;
  int i,j,k;
  char *s = (char *)calloc(200, sizeof(char));
  struct prefix *pf1 =(prefix *)malloc(sizeof(prefix));
  struct prefix *pf2 =(prefix *)malloc(sizeof(prefix));
  
  parseargs(argc, argv);
   
  while(fgets(s, 200, fpr) != NULL)numrules++;
  rewind(fpr);
          
  rule = (pc_rule *)calloc(numrules, sizeof(pc_rule));
  numrules = loadrule(fpr, rule);
  
  printf("the number of rules = %d\n", numrules);
  
  //for(i=0;i<numrules;i++){
  //  printf("%d: %x:%x %x:%x %u:%u %u:%u %u:%u\n", i,
  //    rule[i].field[0].low, rule[i].field[0].high, 
  //    rule[i].field[1].low, rule[i].field[1].high,
  //    rule[i].field[2].low, rule[i].field[2].high,
  //    rule[i].field[3].low, rule[i].field[3].high, 
  //    rule[i].field[4].low, rule[i].field[4].high);
  //}
  
  for(i=0;i<numrules;i++){
    pf1 = expand_prefix(rule[i].field[2].low, rule[i].field[2].high);
    pf2 = expand_prefix(rule[i].field[3].low, rule[i].field[3].high);
    numexprules += pf1->nvalid * pf2->nvalid;
  }
  
  printf("the expended number of rules = %d\n", numexprules);
  
  exprule = (pc_rule *)calloc(numexprules, sizeof(pc_rule));
  
  int index = 0;
  
  for(i=0; i<numrules; i++){
    pf1 = expand_prefix(rule[i].field[2].low, rule[i].field[2].high);
    pf2 = expand_prefix(rule[i].field[3].low, rule[i].field[3].high);
    for(j=0; j<pf1->nvalid; j++){
      for(k=0; k<pf2->nvalid; k++){
        exprule[index].field[0].low = rule[i].field[0].low;
        exprule[index].field[0].high = rule[i].field[0].high;
        exprule[index].field[1].low = rule[i].field[1].low;
        exprule[index].field[1].high = rule[i].field[1].high;
        exprule[index].field[4].low = rule[i].field[4].low;
        exprule[index].field[4].high = rule[i].field[4].high;
        exprule[index].field[2].low = pf1->value[j];
        exprule[index].field[2].high = pf1->value[j] + (int)pow(2, 16-pf1->length[j]) - 1;
        exprule[index].field[3].low = pf2->value[k];
        exprule[index].field[3].high = pf2->value[k] + (int)pow(2, 16-pf2->length[k]) - 1;
        exprule[index].id = i;
        index++;
      }
    }
  }
  
  //for(i=0;i<numexprules;i++){
  //  printf("%d (%d): %x:%x %x:%x %u:%u %u:%u %u:%u\n", i, exprule[i].id,
  //    exprule[i].field[0].low, exprule[i].field[0].high, 
  //    exprule[i].field[1].low, exprule[i].field[1].high,
  //    exprule[i].field[2].low, exprule[i].field[2].high,
  //    exprule[i].field[3].low, exprule[i].field[3].high,
  //    exprule[i].field[4].low, exprule[i].field[4].high);
  //}
  
  trie T(1000*numexprules, numexprules, bucketSize, exprule, nbit0, nbit1);
  
  printf("*************************\n");
  printf("number of nodes = %d\n", T.trieSize());
  printf("max trie depth = %d\n", T.trieDepth()); 
  printf("remove redun = %d\n", T.trieRedun());
  printf("Strored rules = %d\n", T.trieRule());
  printf("Bytes/filter = %f\n", (T.trieSize()*NODESIZE + numrules*RULESIZE + T.trieRule()*RULEPTSIZE)*1.0/numrules);
  printf("*************************\n");
  
  if(fpt != NULL){
    i=0; j=0;
    while(fscanf(fpt,"%u %u %d %d %d %d\n", 
          &header[0], &header[1], &header[2], &header[3], &header[4], &fid) != Null){
      i++;
      
      if((matchid = T.trieLookup(header)) == -1){
        printf("? packet %d match NO rule, should be %d\n", i, fid+1);
        j++;
      }else if(matchid == fid){
        //printf("packet %d match rule %d\n", i, matchid);
      }else if(matchid > fid){
        printf("? packet %d match lower priority rule %d, should be %d\n", i, matchid+1, fid+1);
        j++;
      }else{
        printf("! packet %d match higher priority rule %d, should be %d\n", i, matchid+1, fid+1);
        j++;
      }
    }
    printf("%d packets are classified, %d of them are misclassified\n", i, j);
    printf("# of bytes accessed/pkt = %f\n", T.trieMemAccess()/i);
  }else{
    printf("No packet trace input\n");
  }
  
}  
