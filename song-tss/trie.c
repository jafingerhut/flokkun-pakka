#include "stdinc.h"
#include "trie.h"

#define numtuples(x) (nodeSet[x].numtuples)
#define tuplelist(x) (nodeSet[x].tuplelist)
#define left(x) (nodeSet[x].lc)
#define right(x) (nodeSet[x].rc)

trie::trie(int N1, int bound1, int numtuple1) {
// Initialize trie that can have up to N1 nodes.
	int i;
	N = N1; n = 0;
	bound = bound1;
        numtuples = numtuple1;
        numaccess = 0;
        numprefix = 0;
	if (bound > 32) fatal("trie::trie: bound limit of 32");
	nodeSet = new nodeItem[N+1];
	root = 1; left(root) = right(root) = Null; numtuples(root) = 0;
	freelist = 2;	// create list of unallocated nodes
	for (i = 2; i < N; i++) left(i) = i+1;
	left(N) = Null;
}

trie::~trie() { delete [] nodeSet; }

int *trie::lookup(unsigned adr, unsigned int *num) {
	int i, p, q;

	q = Null;		// deepest next hop value found so far
	p = root;		// p is current node in the trie
	i = bound-1;		// bit i of adr is current address bit
      //printf("root(%d:%d)", root,numtuples(root));
      if(numtuples(root) > 0) q = root;
	while (i >= 0 && p != Null) {
          numaccess += NODESIZE;
          if ((adr & (1 << i)) == 0) {
              p = left(p);
              //printf("-->0(%d:%d)", p, numtuples(p));
 	  }else{
              p = right(p);
              //printf("-->1(%d:%d)", p, numtuples(p));
          }
	  i--;
          if (p != Null && numtuples(p) > 0) q = p;               
	}
      //printf("*** @ node %d ***\n", q);
      numaccess += numtuples/8+1;
      *num = numtuples(q);
      return tuplelist(q);
}

void trie::insert(int prefix, int len, int tupleid) {
	int i, p;
      int j, flag, flag2;

	if (len > bound) fatal("trie::insert: prefix length exceeds bound.");

	// Find node at which prefix must be inserted, or place
	// where it branches outside current data structure.
	p = root;		// p is current node in the trie
	i = len-1;		// prefix[i] is current address bit
	while (i >= 0) {
		if ((prefix & (1 << i)) == 0) {
			if (left(p) == Null) break;
			p = left(p);
		} else {
			if (right(p) == Null) break;
			p = right(p);
		}
		i--;
	}

	// Extend the trie using remaining bits of prefix.
	while (i >= 0) {
		if ((prefix & (1 << i)) == 0) {
			left(p) = freelist; p = left(p);
		} else {
			right(p) = freelist; p = right(p);
		}
		if (p == Null) fatal("trie::insert - out of storage space");
		freelist = left(freelist); n++;
		left(p) = right(p) = Null; numtuples(p) = 0;
		i--;
	}
      flag = 1;
      for(int j=0; j<numtuples(p); j++){
        if(tuplelist(p)[j] == tupleid){
          flag = 0;
          break;
        }
      }
      if(flag == 1){
        if(numtuples(p) == 0){
          numprefix ++;
          tuplelist(p) = (int *)calloc(1, sizeof(int));
          tuplelist(p)[numtuples(p)] = tupleid;
	    numtuples(p) ++;
        }else{
          tuplelist(p) = (int *)realloc(tuplelist(p), (numtuples(p)+1)*sizeof(int));
          flag2 = 1;
          for(i=0; i<numtuples(p); i++){
            if(tupleid < tuplelist(p)[i]){
              for(j=numtuples(p); j>i; j--){
                tuplelist(p)[j] = tuplelist(p)[j-1];
              }
              tuplelist(p)[i] = tupleid;
              flag2 = 0;
              break;
            }
          }
          if(flag2) tuplelist(p)[numtuples(p)] = tupleid;
          numtuples(p) ++;
        }         
      }
      //printf("flag = %d, tupleid = %d, p = %d\n", flag, tupleid, p);
      //for(int j=0; j<numtuples(p); j++) printf("** %d ", tuplelist(p)[j]);
      //printf("\n");
	return;
}
