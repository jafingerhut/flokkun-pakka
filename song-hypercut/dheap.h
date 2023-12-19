// Header file for d-heap data structure. Maintains a subset
// of items in {1,...,m}, where item has a key.

typedef unsigned long keytyp;
typedef int item;

class dheap {
	int	N;			// max number of items in heap
	int	n;			// number of items in heap
	int	d;			// base of heap
	item	*h;			// {h[1],...,h[n]} is set of items
	int	*pos;			// pos[i] gives position of i in h
	keytyp	*kvec;			// kvec[i] is key of item i

	item	minchild(item);		// returm smallest child of item
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
inline int dheap::findmin() { return n == 0 ? Null : h[1]; }

// Return key of i.
inline keytyp dheap::key(item i) {
    if ((i < 1) || (i > N)) {
        fprintf(stderr, "dheap::key(i=%d) Attempting to get key of item outside of range [1..N] for N=%d in dheap\n",
                i, N);
        abort();
        exit(1);
    }
    return kvec[i];
}

// Return true if i in heap, else false.
inline bit dheap::member(item i) {
    if ((i < 1) || (i > N)) {
        fprintf(stderr, "dheap::member(i=%d) Attempting to check membership of item outside of range [1..N] for N=%d in dheap\n",
                i, N);
        abort();
        exit(1);
    }
    return pos[i] != Null;
}

// Return true if heap is empty, else false.
inline bit dheap::empty() { return n == 0; };
