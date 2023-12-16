class trie {
	struct nodeItem {
		bool isleaf;            //is a leaf node if true
  		int nrules;             //number of rules in this node  
            int *ruleid;             //rule ids in this node
  		struct range field[MAXDIMENSIONS];      
  		unsigned int dim : 3;  	//0:sip; 1:dip; 2:proto; 3:sp; 4:dp
  		unsigned int ncuts; //number of cuts
            int *child;             //child pointers
	};
	int	N;			// max number of nodes in trie
	int	n;			// current number of nodes in trie
	int   pass;             // max trie depth
	int   n2;               // removed rules during preprocessing
	int   n3;               // number of rules stored
	float n4;               // number of memory access;
	int   binth;                 
	float spfac;
	int   numrules;
	int   opt;              // dimension choose option
	struct pc_rule *rule;
	int 	root;			// root of trie
	int 	freelist;		// first nodeItem on free list
	nodeItem *nodeSet;	// base of array of NodeItems
	
	void    choose_np_dim(nodeItem *v, int *d, int *np);
	void    remove_redundancy(nodeItem *v); 
	void    createtrie();

public:		//trie(int=10000, int=100, int=16, int=1000, struct pc_rule*, int=1);
            trie(int, int, int, float, struct pc_rule*, int);
		~trie();

	int     trieLookup(int*);
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

