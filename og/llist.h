#ifndef __LLIST_H__
#define __LLIST_H__

// Header file for data structure representing a linked list of things
// pointed at by (void *) pointers.

struct llist_node {
    struct llist_node *next;
    void              *item;
};

class llist {
    struct llist_node *first; // first node of list
    struct llist_node *last;  // last node of list
    int               len;    // number of elements in list
    
public:
    llist();
    ~llist();
    int length();
    void *operator()(int); // access item
    void operator&=(void *);  // append item
    void operator<<=(int); // remove initial items
    //void operator=(list&); // list assignment
    void push(void *);        // push item onto front of list
    //bit  mbr(int);         // return true if member of list
    //int  suc(int);         // return successor
    void *tail();           // return last item on list
    void clear();          // remove everything
    //void print();          // print the items on list
    struct llist_node *first_node();
    struct llist_node *next_node(struct llist_node *);
    void *node_item(struct llist_node *);
};

inline int llist::length() {
    return len;
}

inline void *llist::tail() {
    if (last == NULL) {
        return NULL;
    }
    return last->item;
}

inline struct llist_node *llist::first_node() {
    return first;
}

inline struct llist_node *llist::next_node(struct llist_node *n) {
    return n->next;
}

inline void *llist::node_item(struct llist_node *n) {
    return n->item;
}

#endif // __LLIST_H__
