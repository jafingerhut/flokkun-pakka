#include "stdinc.h"
#include "bv.h"
#include "bst.h"

FILE *fpr;       // ruleset file
FILE *fpt;       // test trace file

int loadrule(FILE *fp, pc_rule *rule){
  
  int tmp;
  unsigned sip1, sip2, sip3, sip4, siplen;
  unsigned dip1, dip2, dip3, dip4, diplen;
  unsigned proto, protomask;
  int i = 0;
  
  while(1){
    
    if(fscanf(fp,"@%d.%d.%d.%d/%d %d.%d.%d.%d/%d %d : %d %d : %d %x/%x\n", 
        &sip1, &sip2, &sip3, &sip4, &siplen, &dip1, &dip2, &dip3, &dip4, &diplen, 
        &rule[i].field[3].low, &rule[i].field[3].high, &rule[i].field[4].low, &rule[i].field[4].high,
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
      rule[i].field[2].low = proto;
      rule[i].field[2].high = proto;
    }else if(protomask == 0){
      rule[i].field[2].low = 0;
      rule[i].field[2].high = 0xFF;
    }else{
      printf("Protocol mask error\n");
      return 0;
    }
    i++;
  }

  return i;
}

void parseargs(int argc, char *argv[]) {
  int	c;
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
	  printf("bv [-r ruleset][-t trace][-h]\n");
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
    fprintf (stderr, "bv [-r ruleset][-t trace][-h]\n");
    exit(1);
  }
}

int main(int argc, char* argv[]){

  int numrules=0;  // actual number of rules in rule set
  struct pc_rule *rule; 
  int i,j,k;
  int a,b,c,d,e;
  int ai, bi, ci, di, ei;
  int header[MAXDIMENSIONS];
  int fid;
  char *s = (char *)calloc(200, sizeof(char));
  int done = 0;

  parseargs(argc, argv);
   
  while(fgets(s, 200, fpr) != NULL)numrules++;
  rewind(fpr);
  
  free(s);
  
  rule = (pc_rule *)calloc(numrules, sizeof(pc_rule));
  numrules = loadrule(fpr, rule);
  
  printf("the number of rules = %d\n", numrules);
  
  bst T0(numrules, rule, 0);
  bst T1(numrules, rule, 1);
  bst T2(numrules, rule, 2);
  bst T3(numrules, rule, 3);
  bst T4(numrules, rule, 4);
 
  printf("numer %d:%d:%d:%d:%d, numnode %d:%d:%d:%d:%d\n", T0.numei(), T1.numei(), T2.numei(), T3.numei(), T4.numei(),
         T0.bstSize(), T1.bstSize(), T2.bstSize(), T3.bstSize(), T4.bstSize());
  int size = (T0.bstSize()+T1.bstSize())*9 + (T3.bstSize()+T4.bstSize())*7+ T2.bstSize()*6;
  size += (T0.numei()+T1.numei()+T2.numei()+T3.numei()+T4.numei())*numrules/8;
  printf("memory usage %d bytes/filter\n", size/numrules); 
  int access = (int)(log(T0.numei())/log(2))*9+(int)(log(T1.numei())/log(2))*9+(int)(log(T2.numei())/log(2))*6+(int)(log(T3.numei())/log(2))*7+(int)(log(T4.numei())/log(2))*7;
  printf("Worst case %d bytes/packet lookup\n", access + 5*numrules/8);
  int *list0 = NULL;
  int *list1;
  int *list2;
  int *list3;
  int *list4;
  int *list;
  
  int match;
  int access2 = 0;
  //perform packet classification
  if(fpt != NULL){
    done = 1;
    int index = 0;
    while(fscanf(fpt,"%u %u %d %d %d %d\n", &header[0], &header[1], &header[3], &header[4], &header[2], &fid) != Null){
      index ++;

      list0 = T0.lookup(header[0], &a);
      //printf("%d ", a);fflush(stdin);
      list1 = T1.lookup(header[1], &b);
      //printf("%d ", b);fflush(stdin);
      list2 = T2.lookup(header[2], &c);
      //printf("%d ", c);fflush(stdin);
      list3 = T3.lookup(header[3], &d);
      //printf("%d ", d);fflush(stdin);
      list4 = T4.lookup(header[4], &e);
      //printf("%d\n", e);fflush(stdin);
      
      //printf("* ");for(i=0; i<a; i++)printf("%d ", list0[i]);
      //printf("* ");for(i=0; i<b; i++)printf("%d ", list1[i]);
      //printf("* ");for(i=0; i<c; i++)printf("%d ", list2[i]);
      //printf("* ");for(i=0; i<d; i++)printf("%d ", list3[i]);
      //printf("* ");for(i=0; i<e; i++)printf("%d ", list4[i]);

      ai=bi=ci=di=ei=0;
      match = 0;
      while(ai < a && bi < b && ci < c && di < d && ei < e){
        if(list0[ai] == list1[bi] && list0[ai] == list2[ci] && list0[ai] == list3[di] && list0[ai] == list4[ei]){
          if(list0[ai] == fid-1) match = 1;
          else{
            done = 0;
            printf("match %d, should be %d\n", list0[ai], fid-1);
          }
          //ai++; bi++; ci++; di++; ei++;
          access2 += (int)(log(fid)/log(2))+1; 
          break;
        }else if(list0[ai] <= list1[bi] && list0[ai] <= list2[ci] && list0[ai] <= list3[di] && list0[ai] <= list4[ei]){
          ai++;
        }else if(list1[bi] <= list0[ai] && list1[bi] <= list2[ci] && list1[bi] <= list3[di] && list1[bi] <= list4[ei]){
          bi++;
        }else if(list2[ci] <= list0[ai] && list2[ci] <= list1[bi] && list2[ci] <= list3[di] && list2[ci] <= list4[ei]){
          ci++;
        }else if(list3[di] <= list0[ai] && list3[di] <= list1[bi] && list3[di] <= list2[ci] && list3[di] <= list4[ei]){
          di++;
        }else{
          ei++;
        }
      }    
      if(match == 0)printf("No rule match packet %d\n", index);
    }  
    printf("average case %d bytes/packet lookup\n", access+5*access2/index); 
  }       
  
  if(done == 1)printf("packet classification done without any error!\n");  
  free(rule);
} 
     

  


