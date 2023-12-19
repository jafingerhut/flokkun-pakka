#include "stdinc.h"
#include "tss.h"
#include "trie.h"

FILE *fpr;       // ruleset file
FILE *fpt;       // test trace file

struct prefix *expand_prefix(unsigned ai, unsigned aj){

  //unsigned mask = 0xFFFFFFFF;
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

int numBits(unsigned low, unsigned high){
  int i = 0;
  while((low & (1<<i)) == 0 && (high & (1<<i))!= 0){
    i++;
    if(i == 32) break;
  }
  return i;
}


int loadrule(FILE *fp, pc_rule *rule){

  int tmp;
  unsigned sip1, sip2, sip3, sip4, siplen;
  unsigned dip1, dip2, dip3, dip4, diplen;
  unsigned proto, protomask;
  int i = 0;

  while(1){

    if(fscanf(fp,"@%d.%d.%d.%d/%d %d.%d.%d.%d/%d %d : %d %d : %d %x/%x\n",
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
    rule[i].id = i+1;
    i++;
  }

  return i;
}

void parseargs(int argc, char *argv[]) {
  int  c;
  bool ok = 1;
  while ((c = getopt(argc, argv, "r:t:h")) != -1) {
    switch (c) {
        case 'r':
          fpr = fopen(optarg, "r");
        break;
        case 't':
          fpt = fopen(optarg, "r");
          break;
        case 'h':
          printf("tss [-r ruleset][-t trace][-h]\n");
          exit(1);
          break;
        default:
          ok = 0;
    }
  }

  if(fpr == NULL){
    printf("can't open ruleset file\n");
    ok = 0;
  }
  if (!ok || optind < argc) {
    fprintf (stderr, "tss [-r ruleset][-t trace][-h]\n");
    exit(1);
  }
}

int main(int argc, char* argv[]){

  int numrules=0;  // actual number of rules in rule set
  int numexprules= 0;
  struct pc_rule *rule;
  struct pc_rule_tuple *exprule;
  int i,j,k;
  char *s = (char *)calloc(200, sizeof(char));
  struct prefix *pf1 =(prefix *)malloc(sizeof(prefix));
  struct prefix *pf2 =(prefix *)malloc(sizeof(prefix));
  int numtuples;
  struct tuple *tuplelist = (tuple *)calloc(MAXRULES, sizeof(tuple));
  struct tuple currenttuple;

  unsigned header[MAXDIMENSIONS];

  int flag;
  int done = 0;

  parseargs(argc, argv);

  while(fgets(s, 200, fpr) != NULL)numrules++;
  rewind(fpr);

  free(s);

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
    free(pf1);
    free(pf2);
  }

  printf("the expended number of rules = %d\n", numexprules);


  exprule = (pc_rule_tuple *)calloc(numexprules, sizeof(pc_rule_tuple));

  int index = 0;

  for(i=0; i<numrules; i++){
    pf1 = expand_prefix(rule[i].field[2].low, rule[i].field[2].high);
    pf2 = expand_prefix(rule[i].field[3].low, rule[i].field[3].high);
    for(j=0; j<pf1->nvalid; j++){
      for(k=0; k<pf2->nvalid; k++){
        exprule[index].ruleid = i+1;
        exprule[index].field[0].low = rule[i].field[0].low;
        exprule[index].field[0].high = rule[i].field[0].high;
        exprule[index].field[1].low = rule[i].field[1].low;
        exprule[index].field[1].high = rule[i].field[1].high;
        exprule[index].field[2].low = pf1->value[j];
        exprule[index].field[2].high = pf1->value[j] + (int)pow(2, 16-pf1->length[j]) - 1;
        exprule[index].field[3].low = pf2->value[k];
        exprule[index].field[3].high = pf2->value[k] + (int)pow(2, 16-pf2->length[k]) - 1;
        exprule[index].field[4].low = rule[i].field[4].low;
        exprule[index].field[4].high = rule[i].field[4].high;
        index++;
      }
    }
    free(pf1);
    free(pf2);
  }

  free(rule);


  numtuples = 0;
  for(i=0;i<numexprules;i++){
    currenttuple.tuplesize = 1;
    currenttuple.siplen = 32-numBits(exprule[i].field[0].low, exprule[i].field[0].high);
    currenttuple.diplen = 32-numBits(exprule[i].field[1].low, exprule[i].field[1].high);
    currenttuple.splen = 16-numBits(exprule[i].field[2].low, exprule[i].field[2].high);
    currenttuple.dplen = 16-numBits(exprule[i].field[3].low, exprule[i].field[3].high);
    currenttuple.protolen = 8-numBits(exprule[i].field[4].low, exprule[i].field[4].high);
    flag = 1;
    for(j=0; j<numtuples; j++){
      if(currenttuple.siplen == tuplelist[j].siplen &&
         currenttuple.diplen == tuplelist[j].diplen &&
         currenttuple.protolen == tuplelist[j].protolen &&
         currenttuple.splen == tuplelist[j].splen &&
         currenttuple.dplen == tuplelist[j].dplen){
        tuplelist[j].tuplesize++;
        exprule[i].tupleid = j;
        flag = 0;
        break;
      }
    }
    if(flag == 1){
      tuplelist[numtuples] = currenttuple;
      exprule[i].tupleid = numtuples;
      numtuples ++;
    }

  }

  for(i=0; i<numtuples; i++){
    tuplelist[i].rulelist = (int *)calloc(tuplelist[i].tuplesize, sizeof(int));
  }

  int tuplesize;
  for(i=0; i<numtuples; i++){
    tuplesize = 0;
    for(j=0; j<numexprules; j++){
      currenttuple.siplen = 32-numBits(exprule[j].field[0].low, exprule[j].field[0].high);
      currenttuple.diplen = 32-numBits(exprule[j].field[1].low, exprule[j].field[1].high);
      currenttuple.splen = 16-numBits(exprule[j].field[2].low, exprule[j].field[2].high);
      currenttuple.dplen = 16-numBits(exprule[j].field[3].low, exprule[j].field[3].high);
      currenttuple.protolen = 8-numBits(exprule[j].field[4].low, exprule[j].field[4].high);
      if(currenttuple.siplen == tuplelist[i].siplen &&
         currenttuple.diplen == tuplelist[i].diplen &&
         currenttuple.protolen == tuplelist[i].protolen &&
         currenttuple.splen == tuplelist[i].splen &&
         currenttuple.dplen == tuplelist[i].dplen){
        tuplelist[i].rulelist[tuplesize] = j;
        tuplesize ++;
      }
    }
  }

  printf("%d tuples\n", numtuples);
  //for(i=0; i<numtuples; i++){
  //  printf("(%2d, %2d, %2d, %2d, %2d) %6d\trules: ", tuplelist[i].siplen, tuplelist[i].diplen,
  //          tuplelist[i].splen, tuplelist[i].dplen, tuplelist[i].protolen, tuplelist[i].tuplesize);
  //  for(j=0; j<tuplelist[i].tuplesize; j++){
  //    printf("%d ", tuplelist[i].rulelist[j]);
  //  }
  //  printf("\n");
  //}

  //for(i=0;i<numexprules;i++){
  //  printf("%4d(%4d): %8x:%8x %8x:%8x %4x:%4x %4x:%4x %2x:%2x %d\n", i, exprule[i].ruleid,
  //    exprule[i].field[0].low, exprule[i].field[0].high,
  //    exprule[i].field[1].low, exprule[i].field[1].high,
  //    exprule[i].field[2].low, exprule[i].field[2].high,
  //    exprule[i].field[3].low, exprule[i].field[3].high,
  //    exprule[i].field[4].low, exprule[i].field[4].high, exprule[i].tupleid);
  // }

  trie TS(10*numexprules,32, numtuples);
  trie TD(10*numexprules,32, numtuples);
  int siplen;
  int sipprefix;
  int diplen;
  int dipprefix;

  for(i=0;i<numexprules;i++){
    siplen = 32-numBits(exprule[i].field[0].low, exprule[i].field[0].high);
    sipprefix = (exprule[i].field[0].low >> (32-siplen)) & ((unsigned)pow(2, siplen)-1);
    diplen = 32-numBits(exprule[i].field[1].low, exprule[i].field[1].high);
    dipprefix = (exprule[i].field[1].low >> (32-diplen)) & ((unsigned)pow(2, diplen)-1);
    for(j=0;j<numexprules;j++){
      if(exprule[j].field[0].low <= exprule[i].field[0].low &&
         exprule[j].field[0].high >= exprule[i].field[0].high){
        TS.insert(sipprefix, siplen, exprule[j].tupleid);
        //printf("%d is ancestor of %d\n", j, i);
        //printf("%d sip insert %d (%x, %d, %d)\n", i,j,sipprefix, siplen, exprule[j].tupleid);
      }
      if(exprule[j].field[1].low <= exprule[i].field[1].low &&
         exprule[j].field[1].high >= exprule[i].field[1].high){
        TD.insert(dipprefix, diplen, exprule[j].tupleid);
        //printf("%d is ancestor of %d\n", j, i);
        //printf("%d dip insert %d (%x, %d, %d)\n", i,j,dipprefix, diplen, exprule[j].tupleid);
      }
    }
  }

  float storageperfilter = (float)(TS.trieSize() + TD.trieSize() + FILTERSIZE*numexprules)/numrules;
  printf("storage %f bytes/filter\n", storageperfilter);

  int fid;
  int match;
  int sipnumtuples;
  int dipnumtuples;
  int *siptuplelist;
  int *diptuplelist;
  int tuplecount;
  int maxcount = 0;
  int totalcount = 0;
  int misclassified_count = 0;
  index = 0;
  //perform packet classification
  if(fpt != NULL){
    done = 1;
    int matchid;

    while(fscanf(fpt,"%u %u %d %d %d %d\n", &header[0], &header[1], &header[2], &header[3], &header[4], &fid) == 6){
      index ++;
      siptuplelist = TS.lookup(header[0], &sipnumtuples);
      diptuplelist = TD.lookup(header[1], &dipnumtuples);
      //printf("packet %d: %x %x %x %x %x ---> \n", index, header[0], header[1], header[2], header[3], header[4]);
      //printf("sip tuple list\n");
      //for(i=0; i<sipnumtuples; i++){
      //  printf("%d ", siptuplelist[i]);
      // }
      //printf("\ndip tuple list\n");
      //for(i=0; i<dipnumtuples; i++){
      //  printf("%d ", diptuplelist[i]);
      // }
      //printf("\n");

      i=j=tuplecount=match=0;
      matchid = numexprules;
      while(i < sipnumtuples && j < dipnumtuples){
        if(siptuplelist[i] == diptuplelist[j]){
          //printf("search tuple %d\n", siptuplelist[i]);
          for(k=0; k<tuplelist[siptuplelist[i]].tuplesize; k++){
            if(exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[0].low  <= header[0] &&
               exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[0].high >= header[0] &&
               exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[1].low  <= header[1] &&
               exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[1].high >= header[1] &&
               exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[2].low  <= header[2] &&
               exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[2].high >= header[2] &&
               exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[3].low  <= header[3] &&
               exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[3].high >= header[3] &&
               exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[4].low  <= header[4] &&
               exprule[tuplelist[siptuplelist[i]].rulelist[k]].field[4].high >= header[4]){
              match = 1;
              //printf("packet %d matches rule %d(%d)\n", index, tuplelist[siptuplelist[i]].rulelist[k], exprule[tuplelist[siptuplelist[i]].rulelist[k]].ruleid);
              if(exprule[tuplelist[siptuplelist[i]].rulelist[k]].ruleid < matchid) matchid = exprule[tuplelist[siptuplelist[i]].rulelist[k]].ruleid;
              break;
            }
          }
          tuplecount ++;
          i++; j++;
        }else if(siptuplelist[i] < diptuplelist[j]){
          i++;
        }else{
          j++;
        }
      }
      totalcount += tuplecount;
      if(tuplecount > maxcount) maxcount = tuplecount;
      if (tuplecount == 0) {
          printf("packet %d has no tuple to search\n", index);
      } else if (match == 0) {
          printf("packet %d matches no rule, should be %d\n", index, fid);
          ++misclassified_count;
      } else if (matchid != fid) {
          printf("packet %d matches rule %d, should be %d\n", index, exprule[matchid].ruleid, fid);
          ++misclassified_count;
      }
    }
    printf("check %d tuples at most\n", maxcount);
    printf("Avg accessed tuples %f\n", (float)totalcount/index);
  }
  if (done == 1) {
      printf("%d packets are classified, %d of them are misclassified\n",
             index, misclassified_count);
  }
  printf("avg case throughput %f bytes/lookup \n", (float)(TS.trieAccess() + TD.trieAccess() + totalcount*FILTERSIZE)/index);
  printf("worst case throughput %f bytes/lookup \n", (float)(TS.trieAccess() + TD.trieAccess())/index + maxcount*FILTERSIZE);
  free(exprule);
  if (misclassified_count != 0) {
      exit(1);
  }
}
