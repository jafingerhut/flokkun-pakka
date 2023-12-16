class trie {
	struct nodeItem {
		bool isleaf;            //is a leaf node if true
  		int nrules;             //number of rules in this node
            int *ruleid;
  		int sbit;               //selected bit
  		int child0;             //left child pointer
  		int child1;             //right child pointer
	};
	int	N;			// max number of nodes in trie
	int	n;			// current number of nodes in trie
	int     pass;                   // max trie depth
	int     n2;                     // removed rule from root
	int     n3;                     // number of rules stored
	float   n4;                     // number of memory access;
	int     k0, k1;
	int     jumptablesize; 
	int     bucketSize;                 
	int     numrules;
	struct  pc_rule *rule;
	int 	  freelist;		// first nodeItem on free list
	nodeItem *nodeSet;		// base of array of NodeItems
	
	int     selectbit(nodeItem *v);
	void    buildjumptable();
	void    createtrie();
	void    remove_redundancy(nodeItem *);

public:		//trie(int=10000, int=100, int=16, int=1000, struct pc_rule*, int=1);
                trie(int, int, int, struct pc_rule*, int, int);
		~trie();

	int     trieLookup(unsigned *);
	int	  trieSize();
	int     trieDepth();
	int     trieRedun();
	int     trieRule();
	float   trieMemAccess();
};

inline int trie::trieSize() {return n;}
inline int trie::trieDepth() {return pass;}
inline int trie::trieRedun() {return n2;}
inline int trie::trieRule() {return n3;}
inline float trie::trieMemAccess() {return n4;}

