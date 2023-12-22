// Header file for binary trie for route lookup.
#define NODESIZE 4

class trie {
	struct nodeItem {
           int numtuples;		
           int *tuplelist;       
	   int lc, rc;		// left and right children
	};
	int	N;			// max number of nodes in trie
	int	n;			// current number of nodes in trie
	int	bound;		// limit on depth of trie
	int 	root;			// root of trie
	int 	freelist;		// first nodeItem on free list
	nodeItem *nodeSet;		// base of array of NodeItems
        int     numprefix;
        int     numtuples;
        int     numaccess;

public:		trie(int=100,int=32, int=64);
		~trie();
	int	*lookup(unsigned, unsigned int*);
	void	insert(int,int, int);	
	int	trieSize() { return n*NODESIZE+numprefix*(numtuples/8+1); };
        int     trieAccess() { return numaccess; };
};
