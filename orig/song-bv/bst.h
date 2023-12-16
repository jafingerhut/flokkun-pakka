// Header file for binary search tree
#include <bitset>

class bst {
	struct nodeItem {
        int   isleaf;     // indicate leaf nodes
        unsigned long key;		// binary search key
        int   low, high;  // auxillary ranges 
        int   numrules;   // number of stored rules
        int   *rulelist;  // stored rulelist when matching the key
        std::bitset<MAXRULES> bitvector;
        int   lc, rc;	// left and right children
	};
	int	N;	// max number of nodes in BST
	int	n;	// current number of nodes in BST
        int     n1;     // number of memory access for lookups (before intersection}
        int     nei;    // number of elementary intervals
	int 	root;		// root of BST
	int 	freelist;	// first nodeItem on free list
	nodeItem *nodeSet;	// base of array of NodeItems

public:	bst(int, struct pc_rule*, int);
		~bst();
	int*  lookup(unsigned long, int*);	// return the matched rule list
      std::bitset<MAXRULES> bvlookup(unsigned long); //using bitvector 
	int	bstSize();
      int   memaccess();
      int  numei();
};

inline int bst::bstSize() {return n;}
inline int bst::memaccess() {return n1;}
inline int bst::numei() {return nei;}


