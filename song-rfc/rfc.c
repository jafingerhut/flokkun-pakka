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

int  phase = 4;  // number of phases
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

unsigned int
search_for_eq_id_add_new_if_not_found(int chunk_id,
                                      unsigned int *num_eq_ids,
                                      unsigned int current_num_rules,
                                      int *current_rule_list,
                                      int *out_match)
{
    unsigned int i, j;
    
    *out_match = 0;
    for (i = 0; i < *num_eq_ids; i++) {
        if (current_num_rules == p0_eq[chunk_id][i].numrules) {
            *out_match = 1;
            for (j = 0; j < current_num_rules; j++) {
                if (p0_eq[chunk_id][i].rulelist[j] != current_rule_list[j]) {
                    *out_match = 0;
                    break;
                }
            }
            if (*out_match == 1) {
                break;
            }
        }
    }
    if (*out_match == 0) {
        p0_eq[chunk_id][*num_eq_ids].numrules = current_num_rules;
        p0_eq[chunk_id][*num_eq_ids].rulelist = current_rule_list;
        i = *num_eq_ids;
        (*num_eq_ids)++;
    }
    return i;
}

// TODO: make search_for_eq_id_add_new_if_not_found and
// search_for_eq_id_add_new_if_not_found2 into a single function,
// after I figure out a correct way to do so.
unsigned int
search_for_eq_id_add_new_if_not_found2(eq *x,
                                       unsigned int *num_eq_ids,
                                       unsigned int current_num_rules,
                                       int *current_rule_list,
                                       int *out_match)
{
    unsigned int i, j;
    
    *out_match = 0;
    for (i = 0; i < *num_eq_ids; i++) {
        if (current_num_rules == x[i].numrules) {
            *out_match = 1;
            for (j = 0; j < current_num_rules; j++) {
                if (x[i].rulelist[j] != current_rule_list[j]) {
                    *out_match = 0;
                    break;
                }
            }
            if (*out_match == 1) {
                break;
            }
        }
    }
    if (*out_match == 0) {
        x[*num_eq_ids].numrules = current_num_rules;
        x[*num_eq_ids].rulelist = current_rule_list;
        i = *num_eq_ids;
        (*num_eq_ids)++;
    }
    return i;
}

int preprocessing_2chunk(eq *a, int na, eq *b, int nb, eq *x, int *tb, unsigned int numrules) {
    int i, j;
    unsigned int k, r;
    unsigned int current_num_rules;
    int *current_rule_list = NULL;
    unsigned int num_eq_ids;
    int match;
    unsigned int matching_eq_id;
    int *rule_list;
    
    rule_list = (int *) calloc(numrules, sizeof(int));
    num_eq_ids = 0;
    for (i = 0; i < na; i++) {
        for (j = 0; j < nb; j++) {
            // get the intersection rule set
            if (a[i].numrules == 0 || b[j].numrules == 0) {
                current_num_rules = 0;
            } else {
                k=0; r=0;
                current_num_rules = 0;
                while (k < a[i].numrules && r < b[j].numrules) {
                    if (a[i].rulelist[k] == b[j].rulelist[r]) {
                        rule_list[current_num_rules] = a[i].rulelist[k];
                        current_num_rules++;
                        k++; r++;
                    } else if (a[i].rulelist[k] > b[j].rulelist[r]) {
                        r++;
                    } else {
                        k++;
                    }
                }
                current_rule_list = (int *) calloc(current_num_rules, sizeof(int));
                for (k = 0; k < current_num_rules; k++) {
                    current_rule_list[k] = rule_list[k];
                }
            }
            // set the equivalence classes
            matching_eq_id = search_for_eq_id_add_new_if_not_found2(
                                     x, &num_eq_ids, current_num_rules,
                                     current_rule_list, &match);
            tb[i*nb +j] = matching_eq_id;
        }
    }
    free(rule_list);
    return num_eq_ids;
}

int preprocessing_3chunk(eq *a, unsigned int na, eq *b, int nb, eq *c, int nc, eq *x, int *tb, unsigned int numrules) {
    int j, s;
    unsigned int i, k, r, t;
    unsigned int current_num_rules;
    int *current_rule_list = NULL;
    unsigned int num_eq_ids;
    int match;
    unsigned int matching_eq_id;
    int *rule_list;
    
    rule_list = (int *) calloc(numrules, sizeof(int));
    num_eq_ids = 0;
    for (i = 0; i < na; i++) {
        for (j = 0; j < nb; j++) {
            for (s = 0; s < nc; s++) {
                // get the intersection list
                if (a[i].numrules == 0 || b[j].numrules == 0 || c[s].numrules == 0) {
                    current_num_rules = 0;
                } else {
                    k = 0; r = 0; t = 0;
                    current_num_rules = 0;
                    while (k < a[i].numrules && r < b[j].numrules && t < c[s].numrules) {
                        if (a[i].rulelist[k] == b[j].rulelist[r] && a[i].rulelist[k] == c[s].rulelist[t]) {
                            rule_list[current_num_rules] = a[i].rulelist[k];
                            current_num_rules++;
                            k++; r++; t++;
                        } else if (a[i].rulelist[k] <= b[j].rulelist[r] && a[i].rulelist[k] <= c[s].rulelist[t]) {
                            k++;
                        } else if (b[j].rulelist[r] <= a[i].rulelist[k] && b[j].rulelist[r] <= c[s].rulelist[t]) {
                            r++;
                        } else {
                            t++;
                        }
                    }
                    current_rule_list = (int *) calloc(current_num_rules, sizeof(int));
                    for (k = 0; k < current_num_rules; k++) {
                        current_rule_list[k] = rule_list[k];
                    }
                }
                // set the equivalent classes
                matching_eq_id = search_for_eq_id_add_new_if_not_found2(
                                         x, &num_eq_ids, current_num_rules,
                                         current_rule_list, &match);
                tb[i*nb*nc +j*nc + s] = matching_eq_id;
            }
        }
    }
    return num_eq_ids;
}


int loadrule(FILE *fp, pc_rule *rule) {

  int tmp;
  unsigned sip1, sip2, sip3, sip4, siplen;
  unsigned dip1, dip2, dip3, dip4, diplen;
  unsigned proto, protomask;
  int i = 0;

  while (1) {

    if (fscanf(fp,"@%d.%d.%d.%d/%d %d.%d.%d.%d/%d %d : %d %d : %d %x/%x\n",
        &sip1, &sip2, &sip3, &sip4, &siplen, &dip1, &dip2, &dip3, &dip4, &diplen,
        &rule[i].field[3].low, &rule[i].field[3].high, &rule[i].field[4].low, &rule[i].field[4].high,
        &proto, &protomask) != 16) break;
    if (siplen == 0) {
      rule[i].field[0].low = 0;
      rule[i].field[0].high = 0xFFFFFFFF;
    } else if (siplen > 0 && siplen <= 8) {
      tmp = sip1<<24;
      rule[i].field[0].low = tmp;
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;
    } else if (siplen > 8 && siplen <= 16) {
      tmp = sip1<<24; tmp += sip2<<16;
      rule[i].field[0].low = tmp; 	
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;	
    } else if (siplen > 16 && siplen <= 24) {
      tmp = sip1<<24; tmp += sip2<<16; tmp +=sip3<<8;
      rule[i].field[0].low = tmp; 	
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;			
    } else if (siplen > 24 && siplen <= 32) {
      tmp = sip1<<24; tmp += sip2<<16; tmp += sip3<<8; tmp += sip4;
      rule[i].field[0].low = tmp;
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;	
    } else {
      printf("Src IP length exceeds 32\n");
      return 0;
    }
    if (diplen == 0) {
      rule[i].field[1].low = 0;
      rule[i].field[1].high = 0xFFFFFFFF;
    } else if (diplen > 0 && diplen <= 8) {
      tmp = dip1<<24;
      rule[i].field[1].low = tmp;
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;
    } else if (diplen > 8 && diplen <= 16) {
      tmp = dip1<<24; tmp +=dip2<<16;
      rule[i].field[1].low = tmp; 	
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;	
    } else if (diplen > 16 && diplen <= 24) {
      tmp = dip1<<24; tmp +=dip2<<16; tmp+=dip3<<8;
      rule[i].field[1].low = tmp; 	
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;			
    } else if (diplen > 24 && diplen <= 32) {
      tmp = dip1<<24; tmp +=dip2<<16; tmp+=dip3<<8; tmp +=dip4;
      rule[i].field[1].low = tmp; 	
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;	
    } else {
      printf("Dest IP length exceeds 32\n");
      return 0;
    }
    if (protomask == 0xFF) {
      rule[i].field[2].low = proto;
      rule[i].field[2].high = proto;
    } else if (protomask == 0) {
      rule[i].field[2].low = 0;
      rule[i].field[2].high = 0xFF;
    } else {
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
	  printf("rfc [-p phase] [-r ruleset] [-t trace] [-h]\n");
	  exit(1);
	  break;
	default:
	  ok = 0;
    }
  }

  if (phase < 3 || phase > 4) {
    printf("number of phases should be either 3 or 4\n");
    ok = 0;
  }	
  if (fpr == NULL) {
    printf("can't open ruleset file\n");
    ok = 0;
  }
  if (!ok || optind < argc) {
    fprintf (stderr, "rfc [-p phase] [-r ruleset] [-t trace] [-h]\n");
    exit(1);
  }
}

void get_chunk_lo_hi(int chunk_id, const pc_rule *rule,
                     unsigned int *out_lo, unsigned int *out_hi)
{
    switch (chunk_id) {
    case 0:
        *out_lo = rule->field[0].low & 0xFFFF;
        *out_hi = rule->field[0].high & 0xFFFF;
        break;
    case 1:
        *out_lo = (rule->field[0].low >> 16) & 0xFFFF;
        *out_hi = (rule->field[0].high >> 16) & 0xFFFF;
        break;
    case 2:
        *out_lo = rule->field[1].low & 0xFFFF;
        *out_hi = rule->field[1].high & 0xFFFF;
        break;
    case 3:
        *out_lo = (rule->field[1].low >> 16) & 0xFFFF;
        *out_hi = (rule->field[1].high >> 16) & 0xFFFF;
        break;
    case 4:
        *out_lo = rule->field[2].low;
        *out_hi = rule->field[2].high;
        break;
    case 5:
        *out_lo = rule->field[3].low;
        *out_hi = rule->field[3].high;
        break;
    case 6:
        *out_lo = rule->field[4].low;
        *out_hi = rule->field[4].high;
        break;
    default:
        break;
    }
}

int preprocessing_phase0(int chunk_id, pc_rule *rule, unsigned int numrules) {
  unsigned int i, j;
  dheap H(2*MAXRULES+2, 2);
  int match;
  unsigned int num_eq_ids;
  unsigned int current_end_point;
  unsigned int current_num_rules;
  int *current_rule_list;
  unsigned int matching_eq_id;
  unsigned char in_heap[65536];
  int *rule_list;

  for (i = 0; i < 65536; i++) {
      in_heap[i] = 0;
  }
  //sort the end points
  unsigned int lo;
  unsigned int hi;
  H.insert(2*numrules+1, 0);
  H.insert(2*numrules+2, 65535);
  in_heap[0] = 1;
  in_heap[65535] = 1;
  for (i = 0; i < numrules; i++) {
      get_chunk_lo_hi(chunk_id, &(rule[i]), &lo, &hi);
      if (in_heap[lo] == 0) {
          H.insert(i+1, lo);
          in_heap[lo] = 1;
      }
      if (in_heap[hi] == 0) {
          H.insert(numrules+i+1, hi);
          in_heap[hi] = 1;
      }
  }

  //assign equivalence classes
  num_eq_ids = 0;
  current_end_point = 0;
  rule_list = (int *) calloc(numrules, sizeof(int));

  while (H.findmin() != Null) {

      while (true) {
          if (current_end_point != H.key(H.findmin())) {
              break;
          }
          H.deletemin();
          if (H.findmin() == Null) {
              break;
          }
      }

    // printf("current end point %d\n", current_end_point);
    current_num_rules = 0;
    for (i = 0; i < numrules; i++) {
        get_chunk_lo_hi(chunk_id, &(rule[i]), &lo, &hi);
        if ((lo <= current_end_point) && (hi >= current_end_point)) {
            rule_list[current_num_rules] = i;
            current_num_rules++;
        }
    }
    current_rule_list = (int *) calloc(current_num_rules, sizeof(int));
    for (i = 0; i < current_num_rules; i++) {
        current_rule_list[i] = rule_list[i];
    }

    //printf("current num rules %d\n", current_num_rules);
    matching_eq_id = search_for_eq_id_add_new_if_not_found(chunk_id,
                                                           &num_eq_ids,
                                                           current_num_rules,
                                                           current_rule_list,
                                                           &match);
    p0_table[chunk_id][current_end_point] = matching_eq_id;
    if (match == 1) {
        // If match is 0, the allocated current_rule_list will be
        // pointed-to and used for the rest of this call to
        // preprocessing_phase0's execution.  But if match is 1, it is
        // not needed any longer.
        free(current_rule_list);
    }

    if (H.findmin() == Null) {
        //printf("dbg H.findmin() is Null when current_end_point is %d\n",
        //       current_end_point);
        break;
    }
    //printf("dbg current range [%d, %lu]\n", current_end_point, H.key(H.findmin()));
    current_num_rules = 0;
    for (i = 0; i < numrules; i++) {
        get_chunk_lo_hi(chunk_id, &(rule[i]), &lo, &hi);
        if ((lo <= current_end_point) && (hi >= H.key(H.findmin()))) {
            rule_list[current_num_rules] = i;
            current_num_rules++;
        }
    }
    current_rule_list = (int *) calloc(current_num_rules, sizeof(int));
    for (i = 0; i < current_num_rules; i++) {
        current_rule_list[i] = rule_list[i];
    }

    //printf("current num rules %d\n", current_num_rules);
    matching_eq_id = search_for_eq_id_add_new_if_not_found(chunk_id,
                                                           &num_eq_ids,
                                                           current_num_rules,
                                                           current_rule_list,
                                                           &match);
    for (j = current_end_point+1; j < H.key(H.findmin()); j++) {
        p0_table[chunk_id][j] = matching_eq_id;
    }
    if (match == 1) {
        free(current_rule_list);
    }

    current_end_point = H.key(H.findmin());
  }
  free(rule_list);
  return num_eq_ids;
}

int main(int argc, char* argv[]) {

  int numrules=0;  // actual number of rules in rule set
  struct pc_rule *rule;
  int i,j;
  unsigned a, b, c, d, e, f, g;
  int header[MAXDIMENSIONS];
  char *s = (char *)calloc(200, sizeof(char));
  int done;
  int fid;
  int size = 0;
  int access = 0;
  int tmp;

  parseargs(argc, argv);

  while (fgets(s, 200, fpr) != NULL) numrules++;
  rewind(fpr);

  free(s);

  rule = (pc_rule *) calloc(numrules, sizeof(pc_rule));
  numrules = loadrule(fpr, rule);

  printf("the number of rules = %d\n", numrules);

  // initialization
  for (i=0; i<7; i++) {
    p0_neq[i] = 0;
    for (j=0; j<=65535; j++) p0_table[i][j] = 0;
    for (j=0; j<2*MAXRULES; j++) {
      p0_eq[i][j].numrules = 0;
      p0_eq[i][j].rulelist = NULL;
    }
  }
  for (i=0; i<4; i++) {
    p1_neq[i] = 0;
    for (j=0; j<MAXTABLE; j++) p1_table[i][j] = 0;
    for (j=0; j<2*MAXRULES; j++) {
      p1_eq[i][j].numrules = 0;
      p1_eq[i][j].rulelist = NULL;
    }
  }
  for (i=0; i<2; i++) {
    p2_neq[i] = 0;
    for (j=0; j<MAXTABLE; j++) p2_table[i][j] = 0;
    for (j=0; j<2*MAXRULES; j++) {
      p2_eq[i][j].numrules = 0;
      p2_eq[i][j].rulelist = NULL;
    }
  }
  p3_neq = 0;
  for (j=0; j<MAXTABLE; j++) p3_table[j] = 0;
  for (j=0; j<2*MAXRULES; j++) {
    p3_eq[j].numrules = 0;
    p3_eq[j].rulelist = NULL;
  }

  //phase 0 preprocessing
  for (i=0; i<7; i++) {
    p0_neq[i] = preprocessing_phase0(i, rule, numrules);
    printf("Chunk %d has %d equivalence classes\n", i, p0_neq[i]);
    if (i == 4) {
      tmp = (int)((log(p0_neq[i])/log(2))/8)+1;
      access += tmp;
      size += tmp * 256;
    } else if (p0_neq[i] > 2) {
      tmp = (int)((log(p0_neq[i])/log(2))/8)+1;
      access += tmp;
      size += tmp * 65536;
    }
    printf("size = %d, access = %d\n", size, access);
    //for (j=0; j<p0_neq[i]; j++) {
    //  printf("(%d) %d: ", j, p0_eq[i][j].numrules);
    //  for (k=0; k<p0_eq[i][j].numrules; k++) {
    //    printf("%d ", p0_eq[i][j].rulelist[k]);
    //  }
    //  printf("\n");
    //}
    //printf("\n");
  }

  switch (phase) {
    case 4:
      //**********************************************************************************************************************
      //phase 1 network
      printf("\nstart phase 1:\n");
      p1_neq[0] = preprocessing_2chunk(p0_eq[0], p0_neq[0], p0_eq[1], p0_neq[1], p1_eq[0], p1_table[0], numrules);
      p1_neq[1] = preprocessing_2chunk(p0_eq[2], p0_neq[2], p0_eq[3], p0_neq[3], p1_eq[1], p1_table[1], numrules);
      p1_neq[2] = preprocessing_3chunk(p0_eq[4], p0_neq[4], p0_eq[5], p0_neq[5], p0_eq[6], p0_neq[6], p1_eq[2], p1_table[2], numrules);

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
      p2_neq[0] = preprocessing_2chunk(p1_eq[0], p1_neq[0], p1_eq[1], p1_neq[1], p2_eq[0], p2_table[0], numrules);

      printf("phase 2 table (%d, %d)\n", p2_neq[0], p1_neq[0]*p1_neq[1]);

      tmp = (int)((log(p2_neq[0])/log(2))/8)+1;
      access += tmp;
      size += tmp * p1_neq[0]*p1_neq[1];

      printf("size = %d, access = %d\n", size, access);
      //phase 3 network
      printf("\nstart phase 3:\n");
      p3_neq = preprocessing_2chunk(p1_eq[2], p1_neq[2], p2_eq[0], p2_neq[0], p3_eq, p3_table, numrules);

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
      p1_neq[0] = preprocessing_2chunk(p0_eq[0], p0_neq[0], p0_eq[1], p0_neq[1], p1_eq[0], p1_table[0], numrules);
      p1_neq[1] = preprocessing_2chunk(p0_eq[2], p0_neq[2], p0_eq[3], p0_neq[3], p1_eq[1], p1_table[1], numrules);
      p1_neq[2] = preprocessing_3chunk(p0_eq[4], p0_neq[4], p0_eq[5], p0_neq[5], p0_eq[6], p0_neq[6], p1_eq[2], p1_table[2], numrules);

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
      p2_neq[0] = preprocessing_3chunk(p1_eq[0], p1_neq[0], p1_eq[1], p1_neq[1], p1_eq[2], p1_neq[2], p2_eq[0], p2_table[0], numrules);

      printf("phase 2 table (%d, %d)\n", p2_neq[0], p1_neq[0]*p1_neq[1]*p1_neq[2]);

      access += 2;
      size += 2 * p1_neq[0]*p1_neq[1]*p1_neq[2];
      printf("size = %d, access = %d\n", size, access);
      //**********************************************************************************************************************
      break;
  }
  printf("\n%10.1f bytes/filter, %d bytes per packet lookup\n", (float)size/numrules+FILTERSIZE, access);
  //perform packet classification
  if (fpt != NULL) {
    done = 1;
    int index = 0;
    while (fscanf(fpt,"%u %u %d %d %d %d\n", &header[0], &header[1], &header[3], &header[4], &header[2], &fid) == 6) {
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

      if (phase == 4) {
        //phase 2
        a = p2_table[0][a*p1_neq[1]+c];

        //phase 3
        a = p3_table[e*p2_neq[0]+a];

        if (p3_eq[a].numrules == 0)printf("No rule matches packet %d\n", index);
        else if (p3_eq[a].rulelist[0] != fid-1) {
          printf("Match rule %d, should be %d\n", p3_eq[a].rulelist[0], fid-1);
          done = 0;
        }
      } else {
        //phase 2
        a = p2_table[0][a*p1_neq[1]*p1_neq[2]+c*p1_neq[2]+e];

        if (p2_eq[0][a].numrules == 0)printf("No rule matches packet %d\n", index);
        else if (p2_eq[0][a].rulelist[0] != fid-1) {
          printf("Match rule %d, should be %d\n", p2_eq[0][a].rulelist[0], fid-1);
          done = 0;
        }
      }
    }
    if (done) {
        printf("\npacket classification done on %d packets without any error!\n", index);
    } else {
        printf("\nError encountered during packet trace test.\n");
        exit(1);
    }
  }

  free(rule);
}
