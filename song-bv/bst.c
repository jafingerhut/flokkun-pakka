#include <bitset>
#include "stdinc.h"
#include "bv.h"
#include "dheap.h"
#include "list.h"
#include "bst.h"

#define skey(x) (nodeSet[x].key)
#define left(x) (nodeSet[x].lc)
#define right(x) (nodeSet[x].rc)
#define list(x) (nodeSet[x].rulelist)
#define numrules(x) (nodeSet[x].numrules)
#define low(x) (nodeSet[x].low)
#define high(x) (nodeSet[x].high)
#define isleaf(x) (nodeSet[x].isleaf)
#define bitvector(x) (nodeSet[x].bitvector)

void bst::init_freelist() {
    int i;

    freelist = 2;	// create list of unallocated nodes
    for (i = 2; i < N; i++) left(i) = i+1;
    left(N) = Null;
}

int bst::alloc_node(int dbg_val) {
    int ret;

    if (freelist == Null) {
        fatal("BST::out of storage space");
    }
    ret = freelist;
    freelist = left(freelist);
    n++;
    //if (ret == 17) {
    //    printf("dbg %x alloc_node %d\n", dbg_val, ret);
    //}
    return ret;
}

int *bst::find_candidate_rules(int numrules, struct pc_rule *rule, int dim, unsigned long mink, unsigned long maxk, int *out_num_rules, std::bitset<MAXRULES> *inout_bitvec) {
    int current_num_rules;
    int i;
    int *rule_list;

    current_num_rules = 0;
    for (i = 0; i < numrules; i++) {
        if (rule[i].field[dim].low <= mink && rule[i].field[dim].high >= maxk) {
            current_num_rules++;
        }
    }
    rule_list = (int *) calloc(current_num_rules, sizeof(int));
    current_num_rules = 0;
    inout_bitvec->reset();
    for (i = 0; i < numrules; i++) {
        if (rule[i].field[dim].low <= mink && rule[i].field[dim].high >= maxk) {
            rule_list[current_num_rules] = i;
            inout_bitvec->set(i);
            current_num_rules++;
        }
    }
    *out_num_rules = current_num_rules;
    return rule_list;
}

bst::bst(int numrules, struct pc_rule *rule, int dim) {
    // Initialize BST
    int i;
    N = 4*numrules+3; n = 0; n1=0;
    dheap H(2*numrules + 3, 2);
    nodeSet = new nodeItem[N+1];
    root = 1; left(root) = right(root) = Null;
    init_freelist();

    for (i = 0; i < numrules; i++) {
        H.insert(i, rule[i].field[dim].low);
        H.insert(numrules+i, rule[i].field[dim].high);
    }
    H.insert(2*numrules, 0);
    if (dim == 0 || dim == 1) {
        //H.insert(2*numrules+1, 4294967295);
        H.insert(2*numrules+1, 0xFFFFFFFF);
    } else if (dim == 2) {
        H.insert(2*numrules+1, 255);
    } else {
        H.insert(2*numrules+1, 65535);
    }

    unsigned long current_end_point = 0;
    unsigned long *endpoint = (unsigned long *)calloc(2*numrules+2, sizeof(unsigned long));
    int nendpoint = 0;

    while (H.findmin() != -1) {
        while (current_end_point == H.key(H.findmin())) {
            H.deletemin();
            if (H.findmin() == -1) {
                break;
            }
        }
        endpoint[nendpoint] = current_end_point;
        nendpoint++;
        if (H.findmin() == -1) {
            break;
        }
        current_end_point = H.key(H.findmin());
    }
    nei = nendpoint;
    list Q(4*numrules+2);
    Q &= root;
    low(root) = 0;
    high(root) = nendpoint-1;
    isleaf(root) = 0;
    int v, temp;

    while (Q(1) != Null) {
        v = Q(1); Q <<= 1;
        temp = (int)(high(v) - low(v))/2 + low(v);
        //printf("%d (%d, %d)\n", v, low(v), high(v));
        if (high(v) == low(v)+1) {
            if (low(v) == 0) {
                skey(v) = endpoint[0];
            } else {
                skey(v) = endpoint[nendpoint-1];
            }
        } else {
            skey(v) = endpoint[temp];
        }
        list(v) = find_candidate_rules(numrules, rule, dim, skey(v), skey(v), &numrules(v), &bitvector(v));

        if (low(v) == 0 && high(v) == 1) {
            if (high(v) != nendpoint -1) {
                right(v) = alloc_node((dim << 8) + 1);
                isleaf(right(v)) = 1;
                list(right(v)) = find_candidate_rules(numrules, rule, dim, endpoint[0], endpoint[1], &numrules(right(v)), &bitvector(right(v)));
            } else {
                right(v) = alloc_node((dim << 8) + 2);
                isleaf(right(v)) = 0;
                skey(right(v)) = endpoint[1];
                list(right(v)) = find_candidate_rules(numrules, rule, dim, endpoint[1], endpoint[1], &numrules(right(v)), &bitvector(right(v)));

                left(right(v)) = alloc_node((dim << 8) + 3);
                isleaf(left(right(v))) = 1;
                list(left(right(v))) = find_candidate_rules(numrules, rule, dim, endpoint[0], endpoint[1], &numrules(left(right(v))), &bitvector(left(right(v))));
            }
        } else if (low(v) == nendpoint-2 && high(v) == nendpoint-1) {
            left(v) = alloc_node((dim << 8) + 4);
            isleaf(left(v)) = 1;
            list(left(v)) = find_candidate_rules(numrules, rule, dim, endpoint[nendpoint-2], endpoint[nendpoint-1], &numrules(left(v)), &bitvector(left(v)));
        } else {
            if (low(v)+1 == temp && low(v) == 0) {
                left(v) = alloc_node((dim << 8) + 5);
                low(left(v)) = low(v);
                high(left(v)) = temp;
                isleaf(left(v)) = 0;
                Q &= left(v);
                if (temp+1 < high(v)) {
                    right(v) = alloc_node((dim << 8) + 6);
                    low(right(v)) = temp;
                    high(right(v)) = high(v);
                    isleaf(right(v)) = 0;
                    Q &= right(v);
                } else {
                    right(v) = alloc_node((dim << 8) + 7);
                    isleaf(right(v)) = 1;
                    list(right(v)) = find_candidate_rules(numrules, rule, dim, endpoint[temp], endpoint[high(v)], &numrules(right(v)), &bitvector(right(v)));
                }
            } else if (high(v)-1 == temp && high(v) == nendpoint-1) {
                right(v) = alloc_node((dim << 8) + 8);
                low(right(v)) = temp;
                high(right(v)) = high(v);
                isleaf(right(v)) = 0;
                Q &= right(v);

                left(v) = alloc_node((dim << 8) + 9);
                isleaf(left(v)) = 1;
                list(left(v)) = find_candidate_rules(numrules, rule, dim, endpoint[low(v)], endpoint[temp], &numrules(left(v)), &bitvector(left(v)));
            } else {
                if (temp-1 == low(v)) {
                    left(v) = alloc_node((dim << 8) + 10);
                    isleaf(left(v)) = 1;
                    list(left(v)) = find_candidate_rules(numrules, rule, dim, endpoint[low(v)], endpoint[temp], &numrules(left(v)), &bitvector(left(v)));
                } else {
                    left(v) = alloc_node((dim << 8) + 11);
                    low(left(v)) = low(v);
                    high(left(v)) = temp;
                    isleaf(left(v)) = 0;
                    Q &= left(v);
                }
                if (temp+1 == high(v)) {
                    right(v) = alloc_node((dim << 8) + 12);
                    isleaf(right(v)) = 1;
                    list(right(v)) = find_candidate_rules(numrules, rule, dim, endpoint[temp], endpoint[high(v)], &numrules(right(v)), &bitvector(right(v)));
                } else {
                    right(v) = alloc_node((dim << 8) + 13);
                    low(right(v)) = temp;
                    high(right(v)) = high(v);
                    isleaf(right(v)) = 0;
                    Q &= right(v);
                }
            }
        }
    }
}

bst::~bst() { delete [] nodeSet; }

int* bst::lookup(unsigned long mykey, int *nrules) {
    register int p;
    p = root;		// p is current node in the BST
    //printf("bst::lookup(mykey %lu) with root %d\n", mykey, root);

    while (1) {
        //printf("mykey %u, current key %u @ %d (%d, %d)\n", mykey, skey(p), p, low(p), high(p));
        //printf("p %d", p);
        //fflush(stdout);
        //printf(" isleaf %d", isleaf(p));
        //fflush(stdout);
        //printf(" mykey %lu", mykey);
        //fflush(stdout);
        //printf(" skey(p) %lu", skey(p));
        //printf("    ");
        //fflush(stdout);
        //printf("p %d isleaf %d mykey %lu skey(p) %lu\n",
        //       p, isleaf(p), mykey, skey(p));
        //fflush(stdout);
        if (isleaf(p) == 1) {
            //printf("reach leaf node with %d rules\n", numrules(p));
            //fflush(stdout);
            *nrules = numrules(p);
            return list(p);
        } else if (mykey == skey(p)) {
            //printf("found the key with %d rules\n", numrules(p));
            //fflush(stdout);
            *nrules = numrules(p);
            return list(p);
        } else if (mykey > skey(p)) {
            //printf("go to right child\n");
            //fflush(stdout);
            p = right(p); n1++;
        } else {
            //printf("go to left child\n");
            //fflush(stdout);
            p = left(p); n1++;
        }
    }
}

std::bitset<MAXRULES> bst::bvlookup(unsigned long mykey) {
    register int p;
    p = root;		// p is current node in the BST

    while (1) {
        //printf("mykey %u, current key %u @ %d (%d, %d)\n", mykey, skey(p), p, low(p), high(p));
        if (isleaf(p) == 1) {
            //printf("reach leaf node\n");
            return bitvector(p);
        } else if (mykey == skey(p)) {
            //printf("meet the key\n");
            return bitvector(p);
        } else if (mykey > skey(p)) {
            p = right(p); n1++;
        } else {
            p = left(p); n1++;
        }
    }
}
