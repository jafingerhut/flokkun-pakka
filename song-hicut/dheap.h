// Header file for d-heap data structure. Maintains a subset
// of items in {0,...,N-1}, where each item has a key.

typedef unsigned long keytyp;
typedef int item;

class dheap {
	int	N;			// max number of items in heap
	int	n;			// number of items in heap
	int	d;			// base of heap
	item	*h;			// {h[0],...,h[n-1]} is set of items
	int	*pos;			// pos[i] gives position of i in h
	keytyp	*kvec;			// kvec[i] is key of item i

	item	minchild(item);		// return smallest child of item
	void	siftup(item,int);	// move item up to restore heap order
	void	siftdown(item,int);	// move item down to restore heap order
public:		dheap(int=100,int=2);
		~dheap();
	item	findmin();		// return the item with smallest key
	keytyp	key(item);		// return the key of item
	bit	member(item);		// return true if item in heap
	bit	empty();		// return true if heap is empty
	void	insert(item,keytyp);	// insert item with specified key
	void	remove(item);		// remove item from heap
	item 	deletemin();		// delete and return smallest item
	void	changekey(item,keytyp);	// change the key of an item
	void	print();		// print the heap
};

// Return item with smallest key.
inline int dheap::findmin() { return n == 0 ? -1 : h[0]; }

// Return key of i.
inline keytyp dheap::key(item i) {
    if ((i < 0) || (i >= N)) {
        fprintf(stderr, "dheap::key(i=%d) Attempting to get key of item outside of range [0..(N-1)] for N=%d in dheap\n",
                i, N);
        fflush(stderr);
        abort();
        exit(1);
    }
    return kvec[i];
}

// Return true if i in heap, else false.
inline bit dheap::member(item i) {
    if ((i < 0) || (i >= N)) {
        fprintf(stderr, "dheap::member(i=%d) Attempting to detemrine membership of item outside of range [0..(N-1)] for N=%d in dheap\n",
                i, N);
        abort();
        exit(1);
    }
    return pos[i] != -1;
}

// Return true if heap is empty, else false.
inline bit dheap::empty() { return n == 0; };
