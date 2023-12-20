#include "stdinc.h"
#include "dheap.h"

// parent of item, leftmost and rightmost children
#define p(x) (((x)-1)/d)
#define left(x) (d*(x)+1)
#define right(x) (d*((x)+1))

dheap::dheap(int N1, int d1) {
// Initialize a heap to store items in {1,...,N}.
	N = N1; d = d1; n = 0;
	h = new item[N];
	pos = new int[N];
	kvec = new keytyp[N];
	for (int i = 0; i < N; i++) pos[i] = -1;
}

dheap::~dheap() { delete [] h; delete [] pos; delete [] kvec; }

void dheap::insert(item i, keytyp k) {
// Add i to heap.
	//printf("dbg insert item %d key %lu\n", i, k);
	//fflush(stdout);
	if ((i < 0) || (i >= N)) {
		fprintf(stderr, "dheap::insert(i=%d, k=%lu) Attempting to insert item outside of range [0..(N-1)] for N=%d in dheap\n",
                        i, k, N);
		abort();
		exit(1);
	}
	kvec[i] = k; siftup(i,n); n++;
}

void dheap::remove(item i) {
// Remove item i from heap. Name remove is used since delete is C++ keyword.
	if ((i < 0) || (i >= N)) {
		fprintf(stderr, "dheap::remove(i=%d) Attempting to remove item outside of range [0..(N-1)] for N=%d in dheap\n",
                        i, N);
		abort();
		exit(1);
	}
	--n;
	int j = h[n];
	     if (i != j && kvec[j] <= kvec[i]) siftup(j,pos[i]);
	else if (i != j && kvec[j] >  kvec[i]) siftdown(j,pos[i]);
	pos[i] = -1;
}

int dheap::deletemin() {
// Remove and return item with smallest key.
	//printf("dbg deletemin\n");
	//fflush(stdout);
	if (n == 0) return -1;
	item i = h[0];
	remove(h[0]);
        //printf("    -> item %d key %lu\n", i, kvec[i]);
        //fflush(stdout);
	return i;
}

void dheap::siftup(item i, int x) {
// Shift i up from position x to restore heap order.
	int px = p(x);
	while (x > 0 && kvec[h[px]] > kvec[i]) {
		h[x] = h[px]; pos[h[x]] = x;
		x = px; px = p(x);
	}
	h[x] = i; pos[i] = x;
}

void dheap::siftdown(item i, int x) {
// Shift i down from position x to restore heap order.
	int cx = minchild(x);
	while (cx != -1 && kvec[h[cx]] < kvec[i]) {
		h[x] = h[cx]; pos[h[x]] = x;
		x = cx; cx = minchild(x);
	}
	h[x] = i; pos[i] = x;
}

// Return the position of the child of the item at position x
// having minimum key.
int dheap::minchild(int x) {
	int y, minc;
	if ((minc = left(x)) >= n) return -1;
	for (y = minc + 1; y <= right(x) && y < n; y++) {
		if (kvec[h[y]] < kvec[h[minc]]) minc = y;
	}
	return minc;
}

void dheap::changekey(item i, keytyp k) {
// Change the key of i and restore heap order.
	if ((i < 0) || (i >= N)) {
		fprintf(stderr, "dheap::changekey(i=%d, k=%lu) Attempting to change key of item outside of range [0..(N-1)] for N=%d in dheap\n",
                        i, k, N);
		abort();
		exit(1);
	}
	keytyp ki = kvec[i]; kvec[i] = k;
	     if (k < ki) siftup(i,pos[i]);
	else if (k > ki) siftdown(i,pos[i]);
}

// Print the contents of the heap.
void dheap::print() {
	int x;
	printf("   h:");
	for (x = 1; x <= n; x++) printf(" %2d",h[x]);
	printf("\nkvec:");
	for (x = 1; x <= n; x++) printf(" %8lx",kvec[h[x]]);
	printf("\n pos:");
	for (x = 1; x <= n; x++) printf(" %2d",pos[h[x]]);
	putchar('\n');
}
