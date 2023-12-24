#include "stdinc.h"
#include "llist.h"

llist::llist() {
    first = last = NULL;
    len = 0;
}

// No free or destructor is called on the objects pointed at by the
// (void *) pointers.  That is up to the caller to do if they wish.
llist::~llist() {
    clear();
}

// Remove all elements from list.
void llist::clear() {
    // Why doesn't the next line compile?
    //this <<= len;

    struct llist_node *cur, *next;
    if (first == NULL) {
        return;
    }
    cur = first;
    do {
        next = cur->next;
        free(cur);
        cur = next;
    } while (cur != NULL);
    first = last = NULL;
    len = 0;
}

// Return the i-th element, where the first is 0.
// Takes time linear in i.
// Returns NULL if there is no such element in the list.
void *llist::operator()(int i) {
    if (i >= len) {
        return NULL;
    }
    struct llist_node *cur = first;
    while (i != 0) {
        --i;
        cur = cur->next;
    }
    return cur->item;
}

// Add item to the end of the list.
void llist::operator&=(void *item) {
    struct llist_node *n = (struct llist_node *) malloc(sizeof(struct llist_node));
    n->item = item;
    n->next = NULL;
    ++len;
    if (last == NULL) {
        first = last = n;
    } else {
        last->next = n;
        last = n;
    }
}

// Remove the first i items.
void llist::operator<<=(int i) {
    struct llist_node *cur;
    while (first != NULL && i--) {
        cur = first;
        first = first->next;
        free(cur);
        --len;
    }
    if (first == NULL) {
        last = NULL;
    }
}

/*
void llist::operator=(llist& L) {
	if (N < L.N) {
		N = L.N;
		delete [] next; next = new int[L.N+1];
		first = last = Null;
		for (int i = 1; i <= N; i++) next[i] = -1;
		next[Null] = Null;
	} else clear();
	for (int i = L(1); i != Null; i = L.suc(i))
		(*this) &= i;
}
*/

// Add item to the front of the list.
void llist::push(void *item) {
    struct llist_node *n = (struct llist_node *) malloc(sizeof(struct llist_node));
    n->item = item;
    n->next = first;
    ++len;
    first = n;
    if (last == NULL) {
        last = n;
    }
}

/*
// Return true if i in list, else false.
bit llist::mbr(int i) {
	return next[i] != -1;
}
*/

/*
// Return the successor of i.
int llist::suc(int i) {
	if (next[i] == -1) fatal("llist::suc: item not on list");
	return next[i];
}
*/

/*
// Print the contents of the list.
void llist::print() {
    for (int i = first; i != Null; i = next[i]) {
        printf("%d ",i);
    }
}
*/
