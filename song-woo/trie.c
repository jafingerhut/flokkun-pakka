#include "stdinc.h"
#include "woo.h"
#include "trie.h"
#include "dheap.h"
#include "list.h"

void trie::init_freelist() {
    int i;

    freelist = jumptablesize + 1;	// create list of unallocated nodes
    for (i = freelist; i < N; i++) nodeSet[i].child0 = i+1;
    nodeSet[N].child0 = Null;
}

int trie::alloc_node() {
    int ret;

    if (freelist == Null) {
        char buf[512];
        snprintf(buf, sizeof(buf), "trie: freelist, originally containing %d entries, is exhausted", N);
        fatal(buf);
    }
    ret = freelist;
    freelist = nodeSet[freelist].child0;
    return ret;
}

trie::trie(int N1, int numrules1, int bucketSize1, struct pc_rule* rule1, int kk0, int kk1) {
// Initialize trie that can have up to N1 nodes.
  int i;
  N = N1;
  numrules = numrules1;
  bucketSize = bucketSize1;
  rule = rule1;
  nodeSet = new nodeItem[N+1];
  n2 = 0;
  n3 = 0;
  n4 = 0;
  k0=kk0; k1=kk1; 
  jumptablesize = (int)pow(2,k0+k1);
  n = jumptablesize;
  
  printf("size of tree node %lu\n", sizeof(nodeItem));
  
  buildjumptable();
  init_freelist();
  createtrie();
}

trie::~trie() { delete [] nodeSet; }

void trie::remove_redundancy(nodeItem *v){
  	
  int cover;
  int tmp, tmp2;
  
  if(v->nrules == 1) return;
  
  tmp = v->nrules -1;
  tmp2 = v->nrules -2;
  while(tmp >= 1){
    for(int i = tmp2; i >= 0; i--){
      for(int k= 0; k < MAXDIMENSIONS; k++){
        cover = 1;
        if(rule[v->ruleid[i]].field[k].low > rule[v->ruleid[tmp]].field[k].low ||
           rule[v->ruleid[i]].field[k].high< rule[v->ruleid[tmp]].field[k].high){
          cover = 0;
          break;
        }
      }
      if(cover == 1){
      	
        for(int j = tmp; j < v->nrules-1; j++){
          v->ruleid[j] = v->ruleid[j+1]; 	
        }
      	
        v->nrules --;
        //printf("rule %d is covered by %d, becomes %d rules @ %d\n", v->ruleid[tmp], v->ruleid[i], v->nrules, pass); 
        n2++;
        break;
      }
    }
    tmp --;
    tmp2 --;
  } 	                       
   		
}

int trie::selectbit(nodeItem *v){
  
  int i,j,k;
  int count0[104];
  int count1[104];
  int counts[104];
  int diff[104];
  int dmin, dmax, smin, smax;
  double pref[104];
  dheap H(104,2);

  for(i=0; i<104; i++){
    count0[i] = 0;
    count1[i] = 0;
    counts[i] = 0;
  }
  
  for(j=0; j<v->nrules; j++){
    for(i=31; i>=0; i--){
      if((rule[v->ruleid[j]].field[0].low >> i & 1) == (rule[v->ruleid[j]].field[0].high >> i & 1)){
        if((rule[v->ruleid[j]].field[0].low >> i & 1) == 1){
          count1[31-i]++;
        }else{
          count0[31-i]++;
        }
      }else{
        break;
      }
    }
    for(k=i; k>=0; k--){
      counts[31-k]++;
    }
    
    for(i=31; i>=0; i--){
      if((rule[v->ruleid[j]].field[1].low >> i & 1) == (rule[v->ruleid[j]].field[1].high >> i & 1)){
        if((rule[v->ruleid[j]].field[1].low >> i & 1) == 1){
          count1[63-i]++;
        }else{
          count0[63-i]++;
        }
      }else{
        break;
      }
    }
    for(k=i; k>=0; k--){
      counts[63-k]++;
    }
    
    for(i=15; i>=0; i--){
      if((rule[v->ruleid[j]].field[2].low >> i & 1) == (rule[v->ruleid[j]].field[2].high >> i & 1)){
        if((rule[v->ruleid[j]].field[2].low >> i & 1) == 1){
          count1[79-i]++;
        }else{
          count0[79-i]++;
        }
      }else{
        break;
      }
    }
    for(k=i; k>=0; k--){
      counts[79-k]++;
    }
    
    for(i=15; i>=0; i--){
      if((rule[v->ruleid[j]].field[3].low >> i & 1) == (rule[v->ruleid[j]].field[3].high >> i & 1)){
        if((rule[v->ruleid[j]].field[3].low >> i & 1) == 1){
          count1[95-i]++;
        }else{
          count0[95-i]++;
        }
      }else{
        break;
      }
    }
    for(k=i; k>=0; k--){
      counts[95-k]++;
    }
    
    for(i=7; i>=0; i--){
      if((rule[v->ruleid[j]].field[4].low >> i & 1) == (rule[v->ruleid[j]].field[4].high >> i & 1)){
        if((rule[v->ruleid[j]].field[4].low >> i & 1) == 1){
          count1[103-i]++;
        }else{
          count0[103-i]++;
        }
      }else{
        break;
      }
    }
    for(k=i; k>=0; k--){
      counts[103-k]++;
    }
  }
        
  for(i=0; i <104; i++){
    diff[i] = abs(count0[i]-count1[i]);
  }
  
  dmin = BIGINT; smin = BIGINT;
  dmax = 0; smax = 0;
  for(i=0; i <104; i++){
    if(diff[i]<dmin) dmin = diff[i];
    if(counts[i]<smin) smin = counts[i];
    if(diff[i]>dmax) dmax = diff[i];
    if(counts[i]>smax) smax = counts[i];
  }	
  
  for(i=0; i <104; i++){
    pref[i]=(double)(diff[i]-dmin)/(dmax-dmin) + (double)(counts[i]-smin)/(smax-smin);
  }
    
  for(i=0; i < 104; i++){
    H.insert(i, pref[i]);
  }
  
  //printf("select bit %d with pref %f\n", H.findmin(), H.key(H.findmin()));
  
  j = H.deletemin();
  
  return j;

}

void trie::buildjumptable(){
  
  int i,j, index;
  int i0, i1;
  
  for(i0=0; i0<pow(2,k0); i0++){
    for(i1=0; i1<pow(2,k1); i1++){
      i=i0*(int)pow(2, k1) + i1 + 1;
      nodeSet[i].nrules=0;
      for(j=0; j<numrules; j++){
            	
      if((((rule[j].field[0].low >> (32-k0)) & (int)(pow(2,k0)-1)) <= i0) && (((rule[j].field[0].high >> (32-k0)) & (int)(pow(2,k0)-1)) >= i0) &&
         (((rule[j].field[1].low >> (32-k1)) & (int)(pow(2,k1)-1)) <= i1) && (((rule[j].field[1].high >> (32-k1)) & (int)(pow(2,k1)-1)) >= i1)){
             nodeSet[i].nrules ++;
        }
      }
      nodeSet[i].ruleid = (int *)calloc(nodeSet[i].nrules, sizeof(int));
      index = 0;
      for(j=0; j<numrules; j++){
        if((((rule[j].field[0].low >> (32-k0)) & (int)(pow(2,k0)-1)) <= i0) && (((rule[j].field[0].high >> (32-k0)) & (int)(pow(2,k0)-1)) >= i0) &&
           (((rule[j].field[1].low >> (32-k1)) & (int)(pow(2,k1)-1)) <= i1) && (((rule[j].field[1].high >> (32-k1)) & (int)(pow(2,k1)-1)) >= i1)){
             nodeSet[i].ruleid[index] = j;
             index ++;
        }
      }
    }
  }
  	
}

void trie::createtrie(){
	
  list Q(MAXNODES);
  int last;
  int u,v,w;
  int cbit;
  int lindex, rindex;
  int localpass;
  float worstcase = 0.0;
  
  pass = 0;
  
  for(int j=1; j<= jumptablesize; j++){

    
    remove_redundancy(&nodeSet[j]);

    if(nodeSet[j].nrules <= bucketSize){
      nodeSet[j].isleaf = 1;
      continue;
    }else{

      nodeSet[j].isleaf = 0;
      Q &= j; last = j; localpass = 0;
  
      while(Q(1) != Null){
  	
        v = Q(1); Q <<= 1;

        //printf("@layer %d, dequeue node %d with %d rules \n", pass, v, nodeSet[v].nrules);
        cbit = selectbit(&nodeSet[v]);
        nodeSet[v].sbit = cbit;
        //printf("choose bit %d\n", cbit);
        u = alloc_node();
        nodeSet[v].child0 = u;
        nodeSet[u].ruleid = (int *)calloc(nodeSet[v].nrules, sizeof(int));
        lindex = 0;

        w = alloc_node();
        nodeSet[v].child1 = w;
        nodeSet[w].ruleid = (int *)calloc(nodeSet[v].nrules, sizeof(int));
        rindex = 0;
    
        n +=2;
    
        for(int i = 0; i < nodeSet[v].nrules; i++){ 
          if(cbit < 32){
              if((rule[nodeSet[v].ruleid[i]].field[0].low >> (31-cbit) & 1) == (rule[nodeSet[v].ruleid[i]].field[0].high >> (31-cbit) & 1)){
              if((rule[nodeSet[v].ruleid[i]].field[0].low >> (31-cbit) & 1) == 0){
                nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
                lindex ++;
              }else{
                nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
                rindex ++;
              }
            }else{
              nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
              lindex ++;
              nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
              rindex ++;
            }
          }else if(cbit < 64){
              if((rule[nodeSet[v].ruleid[i]].field[1].low >> (63-cbit) & 1) == (rule[nodeSet[v].ruleid[i]].field[1].high >> (63-cbit) & 1)){
              if((rule[nodeSet[v].ruleid[i]].field[1].low >> (63-cbit) & 1) == 0){
                nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
                lindex ++;
              }else{
                nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
                rindex ++;
              }
            }else{
              nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
              lindex ++;
              nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
              rindex ++;
            }
          }else if(cbit < 80){
              if((rule[nodeSet[v].ruleid[i]].field[2].low >> (79-cbit) & 1) == (rule[nodeSet[v].ruleid[i]].field[2].high >> (79-cbit) & 1)){
              if((rule[nodeSet[v].ruleid[i]].field[2].low >> (79-cbit) & 1) == 0){
                nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
                lindex ++;
              }else{
                nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
                rindex ++;
              }
            }else{
              nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
              lindex ++;
              nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
              rindex ++;
            }
          }else if(cbit < 96){
              if((rule[nodeSet[v].ruleid[i]].field[3].low >> (95-cbit) & 1) == (rule[nodeSet[v].ruleid[i]].field[3].high >> (95-cbit) & 1)){
              if((rule[nodeSet[v].ruleid[i]].field[3].low >> (95-cbit) & 1) == 0){
                nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
                lindex ++;
              }else{
                nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
                rindex ++;
              }
            }else{
              nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
              lindex ++;
              nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
             rindex ++;
            }
          }else{
            if((rule[nodeSet[v].ruleid[i]].field[4].low >> (103-cbit) & 1) == (rule[nodeSet[v].ruleid[i]].field[4].high >> (103-cbit) & 1)){
              if((rule[nodeSet[v].ruleid[i]].field[4].low >> (103-cbit) & 1) == 0){
                nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
                lindex ++;
              }else{
                nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
                rindex ++;
              }
            }else{
              nodeSet[u].ruleid[lindex] = nodeSet[v].ruleid[i];
              lindex ++;
              nodeSet[w].ruleid[rindex] = nodeSet[v].ruleid[i];
              rindex ++;
            }
          }
        }
    
        nodeSet[u].nrules = lindex;
        nodeSet[w].nrules = rindex;
    
        if(lindex > bucketSize){
          nodeSet[u].isleaf = 0;
          Q &= u;
          //printf("enqueue lnode %d with %d rules\n", u, lindex); 
        }else{
          nodeSet[u].isleaf = 1;
          n3 += lindex;
          if(worstcase < localpass*NODESIZE+lindex*(RULESIZE+RULEPTSIZE)) worstcase = localpass*NODESIZE+lindex*(RULESIZE+RULEPTSIZE);
        }
        if(rindex > bucketSize){
          nodeSet[w].isleaf = 0;
          Q &= w;
          //printf("enqueue rnode %d with %d rules\n", w, rindex); 
        }else{
          nodeSet[w].isleaf = 1;
          n3 += rindex;
          if(worstcase < localpass*NODESIZE+rindex*(RULESIZE+RULEPTSIZE)) worstcase = localpass*NODESIZE+rindex*(RULESIZE+RULEPTSIZE);
        }
          
        if(v == last){
          localpass ++;
          last = Q.tail();
        }
    
      }
    }
    
    if(pass < localpass) pass = localpass;
    
  }	

  printf("worst case %fbytes/lookup\n", worstcase);
  
}

int trie::trieLookup(unsigned* header){
  
  int cnode;
  int match = 0;
  int cover;
  int cbit;
  int i, k;
  int i0,i1;
  
  
  i0 = header[0] >> (32-k0) & (int)(pow(2,k0)-1);
  i1 = header[1] >> (32-k1) & (int)(pow(2,k1)-1);
  cnode =i0*(int)pow(2, k1) + i1 +1;
  n4+=NODESIZE;
  
  //printf("%d, %d, %d, %d, %d => %d\n", i0, i1, i2, i3, i4, cnode);
  
  while(nodeSet[cnode].isleaf != 1){
    //printf("cnode is %d\n", cnode);
    n4+=NODESIZE;
    cbit = nodeSet[cnode].sbit;
    if(cbit < 32){
      if((header[0] >> (31-cbit) & 1) == 0){
        cnode = nodeSet[cnode].child0;	 
      }else{
        cnode = nodeSet[cnode].child1;
      }  
    }else if(cbit < 64){
      if((header[1] >> (63-cbit) & 1) == 0){
        cnode = nodeSet[cnode].child0;	 
      }else{
        cnode = nodeSet[cnode].child1;
      }  
    }else if(cbit < 80){
      if((header[2] >> (79-cbit) & 1) == 0){
        cnode = nodeSet[cnode].child0;	 
      }else{
        cnode = nodeSet[cnode].child1;
      }  
    }else if(cbit < 96){
      if((header[3] >> (95-cbit) & 1) == 0){
        cnode = nodeSet[cnode].child0;	 
      }else{
        cnode = nodeSet[cnode].child1;
      }  
    }else{
      if((header[4] >> (103-cbit) & 1) == 0){
        cnode = nodeSet[cnode].child0;	 
      }else{
        cnode = nodeSet[cnode].child1;
      }  
    }
  }

  for(i = 0; i < nodeSet[cnode].nrules; i++){
    n4+= RULESIZE+RULEPTSIZE;
    cover = 1;
    for(k = 0; k < MAXDIMENSIONS; k++){
      if(rule[nodeSet[cnode].ruleid[i]].field[k].low > header[k] ||
         rule[nodeSet[cnode].ruleid[i]].field[k].high< header[k]){
         cover = 0;
         break;
      }
    }
    if(cover == 1){
      match = 1;
      break;
    }
  }

  
  if(match == 1){
    return rule[nodeSet[cnode].ruleid[i]].id+1;
  }else{
    return -1;
  }
		
}
