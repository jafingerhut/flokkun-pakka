#include <bitset>
#include "stdinc.h"
#include "bv.h"
#include "dheap.h"
#include "list.h"
#include "bst.h"

#define skey(x) (nodeSet[x].key)
#define left(x) (nodeSet[x].lc)
#define right(x) (nodeSet[x].rc)
#define list(x) (nodeSet[x].rulelist)
#define numrules(x) (nodeSet[x].numrules)
#define low(x) (nodeSet[x].low)
#define high(x) (nodeSet[x].high)
#define isleaf(x) (nodeSet[x].isleaf)
#define bitvector(x) (nodeSet[x].bitvector)

bst::bst(int numrules, struct pc_rule *rule, int dim) {
  //Initialize BST 
  int i;  
  N = 4*numrules+3; n = 0; n1=0;
  dheap H(2*numrules +3, 2);
  nodeSet = new nodeItem[N+1];
  root = 1; left(root) = right(root) = Null;
  freelist = 2;	// create list of unallocated nodes
  for (i = 2; i < N; i++) left(i) = i+1;
  left(N) = Null;

  for(i=0; i<numrules; i++){
    H.insert(i, rule[i].field[dim].low);
    H.insert(numrules+i, rule[i].field[dim].high);
  }
  H.insert(2*numrules, 0);
  if(dim == 0 || dim == 1){
    //H.insert(2*numrules+1, 4294967295);
    H.insert(2*numrules+1, 0xFFFFFFFF);
  }else if(dim == 2){
    H.insert(2*numrules+1, 255);
  }else{
    H.insert(2*numrules+1, 65535);
  }

  unsigned long current_end_point = 0;
  unsigned long *endpoint = (unsigned long *)calloc(2*numrules+2, sizeof(unsigned long));
  int nendpoint = 0;

  while(H.findmin() != Null){
    while(current_end_point == H.key(H.findmin())){
      H.deletemin();
    }
    endpoint[nendpoint] = current_end_point;
    nendpoint ++;
    current_end_point = H.key(H.findmin());
  }
  nei = nendpoint;
  int current_num_rules;
  list Q(4*numrules+2);
  Q &= root; 
  low(root) = 0;
  high(root) = nendpoint-1;
  isleaf(root) = 0;
  int v, temp;
 
  while(Q(1) != Null){
    v = Q(1); Q <<= 1;
    temp = (int)(high(v) - low(v))/2 + low(v);
    //printf("%d (%d, %d)\n", v, low(v), high(v));
    if(high(v) == low(v)+1){
      if(low(v) == 0){
        skey(v) = endpoint[0];
      }else{
        skey(v) = endpoint[nendpoint-1];
      }
    }else{
      skey(v) = endpoint[temp];
    }
    //store rules that cover the key value
    current_num_rules = 0;
    for(i=0; i<numrules; i++){
      if(rule[i].field[dim].low <= skey(v) && rule[i].field[dim].high >= skey(v)){
        current_num_rules ++;
      }
    }
    list(v) = (int *)calloc(current_num_rules, sizeof(int));
    current_num_rules = 0;
    bitvector(v).reset();
    for(i=0; i<numrules; i++){
      if(rule[i].field[dim].low <= skey(v) && rule[i].field[dim].high >= skey(v)){
        list(v)[current_num_rules] = i;
        bitvector(v).set(i);
        current_num_rules ++;
      }
    }
    numrules(v) = current_num_rules;
      
    if(low(v) == 0 && high(v) == 1){
      if(high(v) != nendpoint -1){
        right(v) = freelist; 
        if(right(v) == Null) fatal("BST::out of storage space");
        freelist = left(freelist); n++;
        isleaf(right(v)) = 1;
        //store rules in (0,1)
        current_num_rules = 0;
        for(i=0; i<numrules; i++){
          if(rule[i].field[dim].low <= endpoint[0] && rule[i].field[dim].high >= endpoint[1]){
            current_num_rules ++;
          }
        }
        list(right(v)) = (int *)calloc(current_num_rules, sizeof(int));
        current_num_rules = 0;
        bitvector(right(v)).reset();
        for(i=0; i<numrules; i++){
          if(rule[i].field[dim].low <= endpoint[0] && rule[i].field[dim].high >= endpoint[1]){
            list(right(v))[current_num_rules] = i;
            bitvector(right(v)).set(i);
            current_num_rules ++;
          }
        }
        numrules(right(v)) = current_num_rules;
      }else{
        right(v) = freelist; 
        if(right(v) == Null) fatal("BST::out of storage space");
        freelist = left(freelist); n++;
        isleaf(right(v)) = 0;
        skey(right(v)) = endpoint[1];
        current_num_rules = 0;
        for(i=0; i<numrules; i++){
          if(rule[i].field[dim].low <= endpoint[1] && rule[i].field[dim].high >= endpoint[1]){
            current_num_rules ++;
          }
        }
        list(right(v)) = (int *)calloc(current_num_rules, sizeof(int));
        current_num_rules = 0;
        bitvector(right(v)).reset();
        for(i=0; i<numrules; i++){
          if(rule[i].field[dim].low <= endpoint[1] && rule[i].field[dim].high >= endpoint[1]){
            list(right(v))[current_num_rules] = i;
            bitvector(right(v)).set(i);
            current_num_rules ++;
          }
        }
        numrules(right(v)) = current_num_rules;
        
        left(right(v)) = freelist;
        if(left(right(v)) == Null) fatal("BST::out of storage space");
        freelist = left(freelist); n++;
        isleaf(left(right(v))) = 1;
        current_num_rules = 0;
        for(i=0; i<numrules; i++){
          if(rule[i].field[dim].low <= endpoint[0] && rule[i].field[dim].high >= endpoint[1]){
            current_num_rules ++;
          }
        }
        list(left(right(v))) = (int *)calloc(current_num_rules, sizeof(int));
        current_num_rules = 0;
        bitvector(left(right(v))).reset();
        for(i=0; i<numrules; i++){
          if(rule[i].field[dim].low <= endpoint[0] && rule[i].field[dim].high >= endpoint[1]){
            list(left(right(v)))[current_num_rules] = i;
            bitvector(left(right(v))).set(i);
            current_num_rules ++;
          }
        }
        numrules(left(right(v))) = current_num_rules;

      }
    }else if(low(v) == nendpoint-2 && high(v) == nendpoint-1){
      left(v) = freelist;
      if(left(v) == Null) fatal("BST::out of storage space");
      freelist = left(freelist); n++;
      isleaf(left(v)) = 1;
      //store rules in (nendpoint-2, nendpoint-1)
      current_num_rules = 0;
      for(i=0; i<numrules; i++){
        if(rule[i].field[dim].low <= endpoint[nendpoint-2] && rule[i].field[dim].high >= endpoint[nendpoint-1]){
          current_num_rules ++;
        }
      }
      list(left(v)) = (int *)calloc(current_num_rules, sizeof(int));
      current_num_rules = 0;
      bitvector(left(v)).reset();
      for(i=0; i<numrules; i++){
        if(rule[i].field[dim].low <= endpoint[nendpoint-2] && rule[i].field[dim].high >= endpoint[nendpoint-1]){
          list(left(v))[current_num_rules] = i;
          bitvector(left(v)).set(i);
          current_num_rules ++;
        }
      }
      numrules(left(v)) = current_num_rules;

    }else{

      if(low(v)+1 == temp && low(v) == 0){

        left(v) = freelist;
        if(left(v) == Null) fatal("BST::out of storage space");
        freelist = left(freelist); n++;
        low(left(v)) = low(v);
        high(left(v)) = temp;
        isleaf(left(v)) = 0;
        Q &= left(v);
        
        if(temp+1 < high(v)){
          right(v) = freelist;
          if(right(v) == Null) fatal("BST::out of storage space");
          freelist = left(freelist); n++;
          low(right(v)) = temp;
          high(right(v)) = high(v);
          isleaf(left(v)) = 0;
          Q &= right(v);
        }else{
          right(v) = freelist;
          if(right(v) == Null) fatal("BST::out of storage space");
          freelist = left(freelist); n++;
          isleaf(right(v)) = 1;
          //store rules
          current_num_rules = 0;
          for(i=0; i<numrules; i++){
            if(rule[i].field[dim].low <= endpoint[temp] && rule[i].field[dim].high >= endpoint[high(v)]){
              current_num_rules ++;
            }
          }
          list(right(v)) = (int *)calloc(current_num_rules, sizeof(int));
          current_num_rules = 0;
          bitvector(right(v)).reset();
          for(i=0; i<numrules; i++){
            if(rule[i].field[dim].low <= endpoint[temp] && rule[i].field[dim].high >= endpoint[high(v)]){
              list(right(v))[current_num_rules] = i;
              bitvector(right(v)).set(i);
              current_num_rules ++;
            }
          }
          numrules(right(v)) = current_num_rules;
        }

      }else if(high(v)-1 == temp && high(v) == nendpoint-1){

        right(v) = freelist;
        if(right(v) == Null) fatal("BST::out of storage space");
        freelist = left(freelist); n++;
        low(right(v)) = temp;
        high(right(v)) = high(v);
        isleaf(left(v)) = 0;
        Q &= right(v);
        
        left(v) = freelist;
        if(left(v) == Null) fatal("BST::out of storage space");
        freelist = left(freelist); n++;
        isleaf(left(v)) = 1;
        //store rules
        current_num_rules = 0;
        for(i=0; i<numrules; i++){
          if(rule[i].field[dim].low <= endpoint[low(v)] && rule[i].field[dim].high >= endpoint[temp]){
            current_num_rules ++;
          }
        }
        list(left(v)) = (int *)calloc(current_num_rules, sizeof(int));
        current_num_rules = 0;
        bitvector(left(v)).reset();
        for(i=0; i<numrules; i++){
          if(rule[i].field[dim].low <= endpoint[low(v)] && rule[i].field[dim].high >= endpoint[temp]){
            list(left(v))[current_num_rules] = i;
            bitvector(left(v)).set(i);
            current_num_rules ++;
          }
        }
        numrules(left(v)) = current_num_rules;

      }else{
       
        if(temp-1 == low(v)){

          left(v) = freelist;
          if(left(v) == Null) fatal("BST::out of storage space");
          freelist = left(freelist); n++;
          isleaf(left(v)) = 1;
          //store rules
          current_num_rules = 0;
          for(i=0; i<numrules; i++){
            if(rule[i].field[dim].low <= endpoint[low(v)] && rule[i].field[dim].high >= endpoint[temp]){
              current_num_rules ++;
            }
          }
          list(left(v)) = (int *)calloc(current_num_rules, sizeof(int));
          current_num_rules = 0;
          bitvector(left(v)).reset();
          for(i=0; i<numrules; i++){
            if(rule[i].field[dim].low <= endpoint[low(v)] && rule[i].field[dim].high >= endpoint[temp]){
              list(left(v))[current_num_rules] = i;
              bitvector(left(v)).set(i);
              current_num_rules ++;
            }
          }
          numrules(left(v)) = current_num_rules;
        
        }else{
          
          left(v) = freelist;
          if(left(v) == Null) fatal("BST::out of storage space");
          freelist = left(freelist); n++;
          low(left(v)) = low(v);
          high(left(v)) = temp;
          isleaf(left(v)) = 0;
          Q &= left(v);

        }

        if(temp+1 == high(v)){
  
          right(v) = freelist;
          if(right(v) == Null) fatal("BST::out of storage space");
          freelist = left(freelist); n++;
          isleaf(right(v)) = 1;
          //store rules
          current_num_rules = 0;
          for(i=0; i<numrules; i++){
            if(rule[i].field[dim].low <= endpoint[temp] && rule[i].field[dim].high >= endpoint[high(v)]){
              current_num_rules ++;
            }
          }
          list(right(v)) = (int *)calloc(current_num_rules, sizeof(int));
          current_num_rules = 0;
          bitvector(right(v)).reset();
          for(i=0; i<numrules; i++){
            if(rule[i].field[dim].low <= endpoint[temp] && rule[i].field[dim].high >= endpoint[high(v)]){
              list(right(v))[current_num_rules] = i;
              bitvector(right(v)).reset();
              current_num_rules ++;
            }
          }
          numrules(right(v)) = current_num_rules;

        }else{
        
          right(v) = freelist;
          if(right(v) == Null) fatal("BST::out of storage space");
          freelist = left(freelist); n++;
          low(right(v)) = temp;
          high(right(v)) = high(v);
          isleaf(left(v)) = 0;
          Q &= right(v);
        
        }
 
      }
        
    }

  } 
    
}

bst::~bst() { delete [] nodeSet; }

int* bst::lookup(unsigned long mykey, int *nrules) {
  register int p;
  p = root;		// p is current node in the BST
	
  while(1){
    //printf("mykey %u, current key %u @ %d (%d, %d)\n", mykey, skey(p), p, low(p), high(p)); 
    if(isleaf(p) == 1){
      //printf("reach leaf node\n");
      *nrules = numrules(p);
      return list(p);
    }else if(mykey == skey(p)){
      //printf("meet the key\n");
      *nrules = numrules(p);
      return list(p);
    }else if(mykey > skey(p)){
      p = right(p); n1++;
    }else{
      p = left(p); n1++;
    }
  }
}

std::bitset<MAXRULES> bst::bvlookup(unsigned long mykey) {
  register int p;
  p = root;		// p is current node in the BST
	
  while(1){
    //printf("mykey %u, current key %u @ %d (%d, %d)\n", mykey, skey(p), p, low(p), high(p)); 
    if(isleaf(p) == 1){
      //printf("reach leaf node\n");
      return bitvector(p);
    }else if(mykey == skey(p)){
      //printf("meet the key\n");
      return bitvector(p);
    }else if(mykey > skey(p)){
      p = right(p); n1++;
    }else{
      p = left(p); n1++;
    }
  }
}


