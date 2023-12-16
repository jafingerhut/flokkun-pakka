/***************************************
   chunk_id  chunk_size  header-field
       0         16       s.ip[15:0]
       1         16       s.ip[31:16]
       2         16       d.ip[15:0]
       3         16       d.ip[31:16]
       4         8        proto
       5         16       s.port
       6         16       d.port   
****************************************/

#include "stdinc.h"
#include "rfc.h"
#include "dheap.h"

int  phase = 4;  // number of pahses
FILE *fpr;       // ruleset file
FILE *fpt;       // test trace file

int p0_table[7][65536];                  //phase 0 chunk tables
int p1_table[4][MAXTABLE];               //phase 1 chunk tables
int p2_table[2][MAXTABLE];               //phase 2 chunk tables
int p3_table[MAXTABLE];                  //phase 3 chunk tables
struct eq p0_eq[7][2*MAXRULES];          //phase 0 chunk equivalence class
struct eq p1_eq[4][2*MAXRULES];          //phase 1 chunk equivalence class
struct eq p2_eq[2][2*MAXRULES];          //phase 2 chunk equivalence class
struct eq p3_eq[2*MAXRULES];             //phase 3 chunk equivalence class
int p0_neq[7];                           //phase 0 number of chunk equivalence classes
int p1_neq[4];                           //phase 1 number of chunk equivalence classes
int p2_neq[2];                           //phase 2 number of chunk equivalence classes
int p3_neq;                              //phase 3 number of chunk equivalence classes

int preprocessing_2chunk(eq *a, int na, eq *b, int nb, eq *x, int *tb){
  int i, j, k, r;
  int current_num_rules;
  int *current_rule_list = NULL;
  int current_eq_id;
  int match;

  current_eq_id = -1;
  for(i=0; i<na; i++){
    for(j=0; j<nb; j++){
      // get the intersection rule set
      if(a[i].numrules == 0 || b[j].numrules == 0) {
      	current_num_rules = 0;
      }else{
        k=0; r=0;
        current_num_rules = 0;
        while(k < a[i].numrules && r < b[j].numrules){
          if(a[i].rulelist[k] == b[j].rulelist[r]){
            current_num_rules ++;
            k++; r++;
          }else if(a[i].rulelist[k] > b[j].rulelist[r]){
            r++;
          }else{
            k++;
          }
        }
        current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
        k=0; r=0;
        current_num_rules = 0;
        while(k < a[i].numrules && r < b[j].numrules){
          if(a[i].rulelist[k] == b[j].rulelist[r]){
            current_rule_list[current_num_rules] = a[i].rulelist[k];
            current_num_rules ++;
            k++; r++;
          }else if(a[i].rulelist[k] > b[j].rulelist[r]){
            r++;
          }else{
            k++;
          }
        }
      }     
      //set the equivalence classes
      match = 0;
      for(k=0; k<=current_eq_id; k++){
        if(current_num_rules == x[k].numrules){
          match = 1;
          for(r=0; r<current_num_rules; r++){
            if(x[k].rulelist[r] != current_rule_list[r]){
              match = 0;
              break;
            }
          }
          if(match == 1){
            tb[i*nb +j] = k;
            break;
          }
        }
      }
      if(match == 0){
        current_eq_id ++;
        x[current_eq_id].numrules = current_num_rules;
        x[current_eq_id].rulelist = current_rule_list;
        tb[i*nb +j] = current_eq_id;
      }
    }
  }
  return current_eq_id+1;
}

int preprocessing_3chunk(eq *a, int na, eq *b, int nb, eq *c, int nc, eq *x, int *tb){
  int i, j, s, k, r, t;
  int current_num_rules;
  int *current_rule_list = NULL;
  int current_eq_id;
  int match;

  current_eq_id = -1;
  for(i=0; i<na; i++){
    for(j=0; j<nb; j++){
      for(s=0; s<nc; s++){
   
        //get the intersection list
        if(a[i].numrules == 0 || b[j].numrules == 0 || c[s].numrules == 0) {
          current_num_rules = 0;
        }else{
          k=0; r=0; t=0;
          current_num_rules = 0;
          while(k < a[i].numrules && r < b[j].numrules && t < c[s].numrules){
            if(a[i].rulelist[k] == b[j].rulelist[r] && a[i].rulelist[k] == c[s].rulelist[t]){
              current_num_rules ++;
              k++; r++; t++;
            }else if(a[i].rulelist[k] <= b[j].rulelist[r] && a[i].rulelist[k] <= c[s].rulelist[t]){
              k++;
            }else if(b[j].rulelist[r] <= a[i].rulelist[k] && b[j].rulelist[r] <= c[s].rulelist[t]){
              r++;
            }else{
              t++;
            }
          }
          current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
          k=0; r=0; t=0;
          current_num_rules = 0;
          while(k < a[i].numrules && r < b[j].numrules && t < c[s].numrules){
            if(a[i].rulelist[k] == b[j].rulelist[r] && a[i].rulelist[k] == c[s].rulelist[t]){
            current_rule_list[current_num_rules] = a[i].rulelist[k];
            current_num_rules ++;
              k++; r++; t++;
            }else if(a[i].rulelist[k] <= b[j].rulelist[r] && a[i].rulelist[k] <= c[s].rulelist[t]){
              k++;
            }else if(b[j].rulelist[r] <= a[i].rulelist[k] && b[j].rulelist[r] <= c[s].rulelist[t]){
              r++;
            }else{
              t++;
            }
          }
        }
        
        //set the equivalent classes
        match = 0;
        for(k=0; k<=current_eq_id; k++){
          if(current_num_rules == x[k].numrules){
            match = 1;
            for(r=0; r<current_num_rules; r++){
              if(x[k].rulelist[r] != current_rule_list[r]){
                match = 0;
                break;
              }
            }
            if(match == 1){
              tb[i*nb*nc +j*nc +s] = k;
              break;
            }
          }
        }
        if(match == 0){
          current_eq_id ++;
          x[current_eq_id].numrules = current_num_rules;
          x[current_eq_id].rulelist = current_rule_list;
          tb[i*nb*nc +j*nc +s] = current_eq_id;
        }
      }
    }
  }
  return current_eq_id+1;
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
  while ((c = getopt(argc, argv, "p:r:t:h")) != -1) {
    switch (c) {
	case 'p':
	  phase = atoi(optarg);
	  break;
	case 'r':
	  fpr = fopen(optarg, "r");
        break;
	case 't':
	  fpt = fopen(optarg, "r");
	  break;
	case 'h':
	  printf("rfc [-p phase][-r ruleset][-t trace][-h]\n");
	  exit(1);
	  break;
	default:
	  ok = 0;
    }
  }
  
  if(phase < 3 || phase > 4){
    printf("number of phases should be either 3 or 4\n");
    ok = 0;
  }	
  if(fpr == NULL){
    printf("can't open ruleset file\n");
    ok = 0;
  }
  if (!ok || optind < argc) {
    fprintf (stderr, "rfc [-p phase][-r ruleset][-t trace][-h]\n");
    exit(1);
  }
}

int preprocessing_phase0(int chunk_id, pc_rule *rule, int numrules) {

  int i,j,k;
  dheap H(2*MAXRULES+1, 2);
  int match;
  int current_eq_id;
  int current_end_point;
  int current_num_rules;
  int *current_rule_list;
  int npoints;

  //sort the end points
  if(chunk_id == 0){
    for(i=0; i<numrules; i++){
      //printf("%d --> %d:%d\n", i, rule[i].field[0].low & 0xFFFF, rule[i].field[0].high & 0xFFFF);
      H.insert(i, rule[i].field[0].low & 0xFFFF);
      H.insert(numrules+i, rule[i].field[0].high & 0xFFFF);
    }
    H.insert(2*numrules, 0);
    H.insert(2*numrules+1, 65535);
  }else if(chunk_id == 1){
    for(i=0; i<numrules; i++){
      //printf("%d --> %d:%d\n", i, (rule[i].field[0].low >> 16) & 0xFFFF, (rule[i].field[0].high >> 16) & 0xFFFF);
      H.insert(i, (rule[i].field[0].low >> 16) & 0xFFFF);
      H.insert(numrules+i, (rule[i].field[0].high >> 16) & 0xFFFF);
    }
    H.insert(2*numrules, 0);
    H.insert(2*numrules+1, 65535);
  }else if(chunk_id == 2){
    for(i=0; i<numrules; i++){
      H.insert(i, rule[i].field[1].low & 0xFFFF);
      H.insert(numrules+i, rule[i].field[1].high & 0xFFFF);
    }
    H.insert(2*numrules, 0);
    H.insert(2*numrules+1, 65535);
  }else if(chunk_id == 3){
    for(i=0; i<numrules; i++){
      H.insert(i, (rule[i].field[1].low >> 16) & 0xFFFF);
      H.insert(numrules+i, (rule[i].field[1].high >> 16) & 0xFFFF);
    }
    H.insert(2*numrules, 0);
    H.insert(2*numrules+1, 65535);
  }else if(chunk_id == 4){
    for(i=0; i<numrules; i++){
      H.insert(i, rule[i].field[2].low);
      H.insert(numrules+i, rule[i].field[2].high);
    }
    H.insert(2*numrules, 0);
    H.insert(2*numrules+1, 65535);
  }else if(chunk_id == 5){
    for(i=0; i<numrules; i++){
      H.insert(i, rule[i].field[3].low);
      H.insert(numrules+i, rule[i].field[3].high);
    }
    H.insert(2*numrules, 0);
    H.insert(2*numrules+1, 65535);
  }else{
    for(i=0; i<numrules; i++){
      H.insert(i, rule[i].field[4].low);
      H.insert(numrules+i, rule[i].field[4].high);
    }
    H.insert(2*numrules, 0);
    H.insert(2*numrules+1, 65535);
  }
  
  //assign equivalence classes
  current_eq_id = -1;
  current_end_point = 0;
  npoints = 1;

  while(H.findmin() != Null){

    while(current_end_point == H.key(H.findmin())){
      H.deletemin();
    }
    
    //printf("current end point %d\n", current_end_point);
    current_num_rules = 0;
    k = 0;
    if(chunk_id == 0){
      for(i=0; i<numrules; i++){
        if((rule[i].field[0].low & 0xFFFF) <= current_end_point &&
           (rule[i].field[0].high & 0xFFFF) >= current_end_point){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if((rule[i].field[0].low & 0xFFFF) <= current_end_point &&
           (rule[i].field[0].high & 0xFFFF) >= current_end_point){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 1){
      for(i=0; i<numrules; i++){
        if(((rule[i].field[0].low >> 16) & 0xFFFF) <= current_end_point &&
           ((rule[i].field[0].high >> 16) & 0xFFFF) >= current_end_point){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(((rule[i].field[0].low >> 16) & 0xFFFF) <= current_end_point &&
           ((rule[i].field[0].high >> 16) & 0xFFFF) >= current_end_point){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 2){
      for(i=0; i<numrules; i++){
        if((rule[i].field[1].low & 0xFFFF) <= current_end_point &&
           (rule[i].field[1].high & 0xFFFF) >= current_end_point){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if((rule[i].field[1].low & 0xFFFF) <= current_end_point &&
           (rule[i].field[1].high & 0xFFFF) >= current_end_point){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 3){
      for(i=0; i<numrules; i++){
        if(((rule[i].field[1].low >> 16) & 0xFFFF) <= current_end_point &&
           ((rule[i].field[1].high >> 16) & 0xFFFF) >= current_end_point){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(((rule[i].field[1].low >> 16) & 0xFFFF) <= current_end_point &&
           ((rule[i].field[1].high >> 16) & 0xFFFF) >= current_end_point){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 4){
      for(i=0; i<numrules; i++){
        if(rule[i].field[2].low <= current_end_point &&
           rule[i].field[2].high >= current_end_point){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(rule[i].field[2].low <= current_end_point &&
           rule[i].field[2].high >= current_end_point){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 5){
      for(i=0; i<numrules; i++){
        if(rule[i].field[3].low <= current_end_point &&
           rule[i].field[3].high >= current_end_point){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(rule[i].field[3].low <= current_end_point &&
           rule[i].field[3].high >= current_end_point){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else{
      for(i=0; i<numrules; i++){
        if(rule[i].field[4].low <= current_end_point &&
           rule[i].field[4].high >= current_end_point){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(rule[i].field[4].low <= current_end_point &&
           rule[i].field[4].high >= current_end_point){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }
    
    //printf("current num rules %d\n", current_num_rules);

    match = 0;
    for(i=0; i<=current_eq_id; i++){
      if(current_num_rules == p0_eq[chunk_id][i].numrules){
        match = 1;
        for(j=0; j<current_num_rules; j++){
          if(p0_eq[chunk_id][i].rulelist[j] != current_rule_list[j]){
            match = 0;
            break;
          }
        }
        if(match == 1){
          p0_table[chunk_id][current_end_point] = i;
          break;
        }
      }
    }
    if(match == 0){
      current_eq_id ++;
      p0_eq[chunk_id][current_eq_id].numrules = current_num_rules;
      p0_eq[chunk_id][current_eq_id].rulelist = current_rule_list;
      p0_table[chunk_id][current_end_point] = current_eq_id;
    }

    current_num_rules = 0;
    k = 0;
    if(chunk_id == 0){
      for(i=0; i<numrules; i++){
        if((rule[i].field[0].low & 0xFFFF) <= current_end_point &&
           (rule[i].field[0].high & 0xFFFF) >= H.key(H.findmin())){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if((rule[i].field[0].low & 0xFFFF) <= current_end_point &&
           (rule[i].field[0].high & 0xFFFF) >= H.key(H.findmin())){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 1){
      for(i=0; i<numrules; i++){
        if(((rule[i].field[0].low >> 16) & 0xFFFF) <= current_end_point &&
           ((rule[i].field[0].high >> 16) & 0xFFFF) >= H.key(H.findmin())){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(((rule[i].field[0].low >> 16) & 0xFFFF) <= current_end_point &&
           ((rule[i].field[0].high >> 16) & 0xFFFF) >= H.key(H.findmin())){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 2){
      for(i=0; i<numrules; i++){
        if((rule[i].field[1].low & 0xFFFF) <= current_end_point &&
           (rule[i].field[1].high & 0xFFFF) >= H.key(H.findmin())){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if((rule[i].field[1].low & 0xFFFF) <= current_end_point &&
           (rule[i].field[1].high & 0xFFFF) >= H.key(H.findmin())){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 3){
      for(i=0; i<numrules; i++){
        if(((rule[i].field[1].low >> 16) & 0xFFFF) <= current_end_point &&
           ((rule[i].field[1].high >> 16) & 0xFFFF) >= H.key(H.findmin())){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(((rule[i].field[1].low >> 16) & 0xFFFF) <= current_end_point &&
           ((rule[i].field[1].high >> 16) & 0xFFFF) >= H.key(H.findmin())){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 4){
      for(i=0; i<numrules; i++){
        if(rule[i].field[2].low <= current_end_point &&
           rule[i].field[2].high >= H.key(H.findmin())){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(rule[i].field[2].low <= current_end_point &&
           rule[i].field[2].high >= H.key(H.findmin())){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else if(chunk_id == 5){
      for(i=0; i<numrules; i++){
        if(rule[i].field[3].low <= current_end_point &&
           rule[i].field[3].high >= H.key(H.findmin())){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(rule[i].field[3].low <= current_end_point &&
           rule[i].field[3].high >= H.key(H.findmin())){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }else{
      for(i=0; i<numrules; i++){
        if(rule[i].field[4].low <= current_end_point &&
           rule[i].field[4].high >= H.key(H.findmin())){
          current_num_rules ++;
        }
      }
      current_rule_list = (int *)calloc(current_num_rules, sizeof(int));
      for(i=0; i<numrules; i++){
        if(rule[i].field[4].low <= current_end_point &&
           rule[i].field[4].high >= H.key(H.findmin())){
          current_rule_list[k] = i;
          k++;             
        }
      }
    }
    
    //printf("current num rules %d\n", current_num_rules);

    match = 0;
    for(i=0; i<=current_eq_id; i++){
      if(current_num_rules == p0_eq[chunk_id][i].numrules){
        match = 1;
        for(j=0; j<current_num_rules; j++){
          if(p0_eq[chunk_id][i].rulelist[j] != current_rule_list[j]){
            match = 0;
            break;
          }
        }
        if(match == 1){
          for(j=current_end_point+1; j<H.key(H.findmin()); j++){
            p0_table[chunk_id][j] = i;
          }
          break;
        }
      }
    }
    if(match == 0){
      current_eq_id ++;
      p0_eq[chunk_id][current_eq_id].numrules = current_num_rules;
      p0_eq[chunk_id][current_eq_id].rulelist = current_rule_list;
      for(i=current_end_point+1; i<H.key(H.findmin()); i++){
        p0_table[chunk_id][i] = current_eq_id;
      }
    }

    current_end_point = H.key(H.findmin());
    npoints ++;

  }
  
  //printf("%d end points in total\n", npoints);
  return current_eq_id+1;
}

int main(int argc, char* argv[]){

  int numrules=0;  // actual number of rules in rule set
  struct pc_rule *rule; 
  int i,j,k;
  unsigned a, b, c, d, e, f, g;
  int header[MAXDIMENSIONS];
  char *s = (char *)calloc(200, sizeof(char));
  int done;
  int fid;
  int size = 0;
  int access = 0;
  int tmp;

  parseargs(argc, argv);
   
  while(fgets(s, 200, fpr) != NULL)numrules++;
  rewind(fpr);
  
  free(s);
  
  rule = (pc_rule *)calloc(numrules, sizeof(pc_rule));
  numrules = loadrule(fpr, rule);
  
  printf("the number of rules = %d\n", numrules);
  
  //initilization
  for(i=0; i<7; i++){
    p0_neq[i] = 0;
    for(j=0; j<=65535; j++) p0_table[i][j] = 0;
    for(j=0; j<2*MAXRULES; j++) {
      p0_eq[i][j].numrules = 0;
      p0_eq[i][j].rulelist = NULL;
    }
  }
  for(i=0; i<4; i++){
    p1_neq[i] = 0;
    for(j=0; j<MAXTABLE; j++) p1_table[i][j] = 0;
    for(j=0; j<2*MAXRULES; j++) {
      p1_eq[i][j].numrules = 0;
      p1_eq[i][j].rulelist = NULL;
    }
  }
  for(i=0; i<2; i++){
    p2_neq[i] = 0;
    for(j=0; j<MAXTABLE; j++) p2_table[i][j] = 0;
    for(j=0; j<2*MAXRULES; j++) {
      p2_eq[i][j].numrules = 0;
      p2_eq[i][j].rulelist = NULL;
    }
  }
  p3_neq = 0;
  for(j=0; j<MAXTABLE; j++) p3_table[j] = 0;
  for(j=0; j<2*MAXRULES; j++) {
    p3_eq[j].numrules = 0;
    p3_eq[j].rulelist = NULL;
  }

  //phase 0 preprocessing
  for(i=0; i<7; i++){
    p0_neq[i] = preprocessing_phase0(i, rule, numrules);
    printf("Chunk %d has %d equivalence classes\n", i, p0_neq[i]);
    if(i == 4) {
      tmp = (int)((log(p0_neq[i])/log(2))/8)+1;
      access += tmp;
      size += tmp * 256;
    }else if(p0_neq[i] > 2) {
      tmp = (int)((log(p0_neq[i])/log(2))/8)+1;
      access += tmp;
      size += tmp * 65536;
    }
    printf("size = %d, access = %d\n", size, access);
    //for(j=0; j<p0_neq[i]; j++){
    //  printf("(%d) %d: ", j, p0_eq[i][j].numrules);
    //  for(k=0; k<p0_eq[i][j].numrules; k++){
    //    printf("%d ", p0_eq[i][j].rulelist[k]);
    //  }
    //  printf("\n");
    //}
    //printf("\n");
  }

  switch(phase){
    case 4:
      //**********************************************************************************************************************
      //phase 1 network
      printf("\nstart phase 1:\n");
      p1_neq[0] = preprocessing_2chunk(p0_eq[0], p0_neq[0], p0_eq[1], p0_neq[1], p1_eq[0], p1_table[0]);
      p1_neq[1] = preprocessing_2chunk(p0_eq[2], p0_neq[2], p0_eq[3], p0_neq[3], p1_eq[1], p1_table[1]);
      p1_neq[2] = preprocessing_3chunk(p0_eq[4], p0_neq[4], p0_eq[5], p0_neq[5], p0_eq[6], p0_neq[6], p1_eq[2], p1_table[2]);

      printf("phase 1 table (%d, %d), (%d, %d), (%d, %d)\n", 
              p1_neq[0], p0_neq[0]*p0_neq[1],
              p1_neq[1], p0_neq[2]*p0_neq[3],
              p1_neq[2], p0_neq[4]*p0_neq[5]*p0_neq[6]);

      tmp = (int)((log(p1_neq[0])/log(2))/8)+1;
      access += tmp;
      size += tmp * p0_neq[0]*p0_neq[1];

      tmp = (int)((log(p1_neq[1])/log(2))/8)+1;
      access += tmp;
      size += tmp * p0_neq[2]*p0_neq[3];

      tmp = (int)((log(p1_neq[2])/log(2))/8)+1;
      access += tmp;
      size += tmp * p0_neq[4]*p0_neq[5]*p0_neq[6];

      printf("size = %d, access = %d\n", size, access);
      //phase 2 network
      printf("\nstart phase 2:\n");
      p2_neq[0] = preprocessing_2chunk(p1_eq[0], p1_neq[0], p1_eq[1], p1_neq[1], p2_eq[0], p2_table[0]);

      printf("phase 2 table (%d, %d)\n", p2_neq[0], p1_neq[0]*p1_neq[1]);
  
      tmp = (int)((log(p2_neq[0])/log(2))/8)+1;
      access += tmp;
      size += tmp * p1_neq[0]*p1_neq[1];

      printf("size = %d, access = %d\n", size, access);
      //phase 3 network
      printf("\nstart phase 3:\n");
      p3_neq = preprocessing_2chunk(p1_eq[2], p1_neq[2], p2_eq[0], p2_neq[0], p3_eq, p3_table);
      
      printf("phase 3 table (%d, %d)\n", p3_neq, p1_neq[2]*p2_neq[0]);
      
      access += 2;
      size += 2 * p1_neq[2]*p2_neq[0];
      printf("size = %d, access = %d\n", size, access);
      //**********************************************************************************************************************
      break;
    case 3:
      //**********************************************************************************************************************
      //configuration 1--> 2,2,3;3
      //phase 1 network
      printf("\nstart phase 1:\n");
      p1_neq[0] = preprocessing_2chunk(p0_eq[0], p0_neq[0], p0_eq[1], p0_neq[1], p1_eq[0], p1_table[0]);
      p1_neq[1] = preprocessing_2chunk(p0_eq[2], p0_neq[2], p0_eq[3], p0_neq[3], p1_eq[1], p1_table[1]);
      p1_neq[2] = preprocessing_3chunk(p0_eq[4], p0_neq[4], p0_eq[5], p0_neq[5], p0_eq[6], p0_neq[6], p1_eq[2], p1_table[2]);

      printf("phase 1 table (%d, %d), (%d, %d), (%d, %d)\n", 
              p1_neq[0], p0_neq[0]*p0_neq[1],
              p1_neq[1], p0_neq[2]*p0_neq[3],
              p1_neq[2], p0_neq[4]*p0_neq[5]*p0_neq[6]);

      tmp = (int)((log(p1_neq[0])/log(2))/8)+1;
      access += tmp;
      size += tmp * p0_neq[0]*p0_neq[1];

      tmp = (int)((log(p1_neq[1])/log(2))/8)+1;
      access += tmp;
      size += tmp * p0_neq[2]*p0_neq[3];

      tmp = (int)((log(p1_neq[2])/log(2))/8)+1;
      access += tmp;
      size += tmp * p0_neq[4]*p0_neq[5]*p0_neq[6];

      printf("size = %d, access = %d\n", size, access);
      //phase 2 network
      printf("\nstart phase 2:\n");
      p2_neq[0] = preprocessing_3chunk(p1_eq[0], p1_neq[0], p1_eq[1], p1_neq[1], p1_eq[2], p1_neq[2], p2_eq[0], p2_table[0]);

      printf("phase 2 table (%d, %d)\n", p2_neq[0], p1_neq[0]*p1_neq[1]*p1_neq[2]);

      access += 2;
      size += 2 * p1_neq[0]*p1_neq[1]*p1_neq[2];
      printf("size = %d, access = %d\n", size, access);
      //**********************************************************************************************************************
      break;
  }
  printf("\n%10.1f bytes/filter, %d bytes per packet lookup\n", (float)size/numrules+FILTERSIZE, access); 
  //perform packet classification
  if(fpt != NULL){
    done = 1;
    int index = 0;
    while(fscanf(fpt,"%u %u %d %d %d %d\n", &header[0], &header[1], &header[3], &header[4], &header[2], &fid) != Null){
      index ++;
      //phase 0
      a = p0_table[0][header[0] & 0xFFFF];
      b = p0_table[1][(header[0] >> 16) & 0xFFFF];
      c = p0_table[2][header[1] & 0xFFFF]; 
      d = p0_table[3][(header[1] >> 16) & 0xFFFF];
      e = p0_table[4][header[2]]; 
      f = p0_table[5][header[3]];
      g = p0_table[6][header[4]];
      
      //phase 1
      a = p1_table[0][a*p0_neq[1]+b];
      c = p1_table[1][c*p0_neq[3]+d];
      e = p1_table[2][e*p0_neq[5]*p0_neq[6]+f*p0_neq[6]+g];
      
      if(phase == 4){
        //phase 2
        a = p2_table[0][a*p1_neq[1]+c];
      
        //phase 3
        a = p3_table[e*p2_neq[0]+a];
      
        if(p3_eq[a].numrules == 0)printf("No rule matches packet %d\n", index);
        else if(p3_eq[a].rulelist[0] != fid-1){
          printf("Match rule %d, should be %d\n", p3_eq[a].rulelist[0], fid-1);
          done = 0;
        }
      }else{
        //phase 2
        a = p2_table[0][a*p1_neq[1]*p1_neq[2]+c*p1_neq[2]+e];

        if(p2_eq[0][a].numrules == 0)printf("No rule matches packet %d\n", index);
        else if(p2_eq[0][a].rulelist[0] != fid-1){
          printf("Match rule %d, should be %d\n", p2_eq[0][a].rulelist[0], fid-1);
          done = 0;
        }
      } 
    }
  }
  if(done)printf("\npacket classification done without any error!\n");  

  free(rule);
}  
  


