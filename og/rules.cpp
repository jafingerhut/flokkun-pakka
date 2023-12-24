#include "rules.hpp"
#include <stdio.h>

unsigned int mask(int k)
{
    if (k < 0 || k > 32) {
        char buf[512];
        snprintf(buf, sizeof(buf), "mask parameter k=%d must be in range [0,32]\n", k);
        fatal(buf);
    }
    if (k == 32) {
        return 0xffffffff;
    } else {
        return (((unsigned int) 1) << k) - 1;
    }
}

void loadrule(FILE *fp, llist& rule_list)
{
    unsigned sip1, sip2, sip3, sip4, siplen, sip;
    unsigned dip1, dip2, dip3, dip4, diplen, dip;
    unsigned proto, protomask;
    int i = 0;
    char buf[512];
    struct pc_rule *newrule;
    
    while (1) {
        newrule = (struct pc_rule *) malloc(sizeof(struct pc_rule));
        if (fscanf(fp,"@%u.%u.%u.%u/%u %u.%u.%u.%u/%u %u : %u %u : %u %x/%x\n",
                   &sip1, &sip2, &sip3, &sip4, &siplen,
                   &dip1, &dip2, &dip3, &dip4, &diplen,
                   &(newrule->field[3].low), &(newrule->field[3].high),
                   &(newrule->field[4].low), &(newrule->field[4].high),
                   &proto, &protomask) != 16) {
            break;
        }
        if (sip1 > 255 || sip2 > 255 || sip3 > 255 || sip4 > 255) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has source IPv4 address %u.%u.%u.%u with some value greater than 255, which is not supported.\n",
                     i+1, sip1, sip2, sip3, sip4);
            fatal(buf);
        }
        if (dip1 > 255 || dip2 > 255 || dip3 > 255 || dip4 > 255) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has destination IPv4 address %u.%u.%u.%u with some value greater than 255, which is not supported.\n",
                     i+1, dip1, dip2, dip3, dip4);
            fatal(buf);
        }
        if ((newrule->field[3].low > newrule->field[3].high) ||
            (newrule->field[3].high > 65535)) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has L4 source port range [%u,%u] with lo larger than hi, or hi greater than 65535, which is not supported.\n",
                     i+1, newrule->field[3].low, newrule->field[3].high);
            fatal(buf);
        }
        if ((newrule->field[4].low > newrule->field[4].high) ||
            (newrule->field[4].high > 65535)) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has L4 source port range [%u,%u] with lo larger than hi, or hi greater than 65535, which is not supported.\n",
                     i+1, newrule->field[4].low, newrule->field[4].high);
            fatal(buf);
        }
        if (proto > 255) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has protocol %u larger than 255, which is not supported.\n",
                     i+1, proto);
            fatal(buf);
        }
        if (siplen < 0 || siplen > 32) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has source IPv4 prefix length %u outside of range [0,32], which is not supported.\n",
                     i+1, siplen);
            fatal(buf);
        }
        sip = ((sip1 << 24) | (sip2 << 16) | (sip3 << 8) | sip4);
        unsigned int smask = mask(32-siplen);
        if ((sip & smask) != 0) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has source IPv4 address %u.%u.%u.%u and prefix len %u with non-0 bits after the prefix length, which is not supported.\n",
                     i+1, sip1, sip2, sip3, sip4, siplen);
            fatal(buf);
        }
        newrule->field[0].low = sip;
        newrule->field[0].high = sip + smask;
        if (diplen < 0 || diplen > 32) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has destination IPv4 prefix length %u outside of range [0,32], which is not supported.\n",
                     i+1, diplen);
            fatal(buf);
        }
        dip = ((dip1 << 24) | (dip2 << 16) | (dip3 << 8) | dip4);
        unsigned int dmask = mask(32-diplen);
        if ((dip & dmask) != 0) {
            snprintf(buf, sizeof(buf),
                     "Rule %d has destination IPv4 address %u.%u.%u.%u and prefix len %u with non-0 bits after the prefix length, which is not supported.\n",
                     i+1, dip1, dip2, dip3, dip4, diplen);
            fatal(buf);
        }
        newrule->field[1].low = dip;
        newrule->field[1].high = dip + dmask;
        if (protomask == 0xff) {
            newrule->field[2].low = proto;
            newrule->field[2].high = proto;
        } else if (protomask == 0) {
            newrule->field[2].low = 0;
            newrule->field[2].high = 0xff;
        } else {
            fatal("Protocol mask error\n");
        }
        rule_list &= newrule;
        ++i;
    }
}

void min_max_to_prefix32(unsigned int low, unsigned int high,
                         unsigned int *out_prefix,
                         int *out_prefix_len)
{
    unsigned int maybe_mask = low ^ high;

    // If (maybe_mask+1)&maybe_mask is 0, then it should be of the
    // form (1 << w) - 1 for some integer w in [0,32].
    if (((maybe_mask + 1) & maybe_mask) != 0) {
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "min_max_to_prefix: Range [%u,%u] is not equivalent to any prefix match criteria\n",
                 low, high);
        fatal(buf);
    }
    
    // Do a binary search on k in range [0,32] to see which of the
    // mask(j) return values that maybe_mask is equal to.
    int a = 0;
    int b = 32;
    while (a < b) {
        int mid = (a + b) / 2;
        //printf("maybe_mask 0x%08x a %2d b %2d mid %2d\n",
        //       maybe_mask, a, b, mid);
        if (maybe_mask == mask(mid)) {
            a = b = mid;
            break;
        } else if (maybe_mask < mask(mid)) {
            b = mid;
        } else {
            // Ensure that a increases by at least 1
            a = max(mid, a+1);
        }
    }
    if (maybe_mask != mask(a)) {
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "min_max_to_prefix: Internal error for range [%u,%u] maybe_mask=%u - binary search ending with a=%d mask(a)=%u could not find correct arg to mask()\n",
                 low, high, maybe_mask, a, mask(a));
        fatal(buf);
    }
    *out_prefix_len = 32 - a;
    *out_prefix = low;
}

void writerule(FILE *fp, llist& rule_list)
{
    int i = 0;
    struct pc_rule *r;
    struct llist_node *rn;
    unsigned int field0_prefix, field1_prefix;
    int field0_prefixlen, field1_prefixlen, field2_prefixlen;

    for (i = 0, rn = rule_list.first_node(); rn != NULL; rn = rule_list.next_node(rn), i++) {
        r = (struct pc_rule *) rule_list.node_item(rn);
        if ((r->field[0].low > r->field[0].high) ||
            (r->field[1].low > r->field[1].high) ||
            (r->field[2].low > r->field[2].high) ||
            (r->field[3].low > r->field[3].high) ||
            (r->field[4].low > r->field[4].high) ||
            (r->field[2].high > 255) ||
            (r->field[3].high > 65535) ||
            (r->field[4].high > 65535))
        {
            fprintf(stderr, "writerule: For rule %d there is a field's low value larger than its high value, or L4 port values greater than 65535, or proto value greater than 255.\n",
                    i+1);
            print_rule(stderr, r);
            exit(1);
        }
        min_max_to_prefix32(r->field[0].low, r->field[0].high,
                            &field0_prefix,
                            &field0_prefixlen);
        min_max_to_prefix32(r->field[1].low, r->field[1].high,
                            &field1_prefix,
                            &field1_prefixlen);
        if ((r->field[2].low == 0) && (r->field[2].high == 0xff)) {
            field2_prefixlen = 0;
        } else if (r->field[2].low == r->field[2].high) {
            field2_prefixlen = 8;
        } else {
            fprintf(stderr, "writerule: For rule %d proto field range [%u,%u] is neither single value, nor [0,255], which are only ranges supported.\n",
                    i+1, r->field[2].low, r->field[2].high);
            exit(1);
        }
        fprintf(fp, "@%u.%u.%u.%u/%u %u.%u.%u.%u/%u %u : %u %u : %u 0x%02X/0x%02X\n",
                (field0_prefix >> 24) & 0xff,
                (field0_prefix >> 16) & 0xff,
                (field0_prefix >>  8) & 0xff,
                (field0_prefix >>  0) & 0xff,
                field0_prefixlen,
                (field1_prefix >> 24) & 0xff,
                (field1_prefix >> 16) & 0xff,
                (field1_prefix >>  8) & 0xff,
                (field1_prefix >>  0) & 0xff,
                field1_prefixlen,
                r->field[3].low,
                r->field[3].high,
                r->field[4].low,
                r->field[4].high,
                r->field[2].low,
                (field2_prefixlen == 8) ? 0xff : 0);
        ++i;
    }
}

bool rules_disjoint(struct pc_rule *r1, struct pc_rule *r2)
{
    int i;
    for (i = 0; i < MAXDIMENSIONS; i++) {
        if (r1->field[i].high < r2->field[i].low) {
            return true;
        }
        if (r2->field[i].high < r1->field[i].low) {
            return true;
        }
    }
    return false;
}

bool rule_subset(struct pc_rule *r1, struct pc_rule *r2)
{
    int i;
    for (i = 0; i < MAXDIMENSIONS; i++) {
        if ((r1->field[i].low >= r2->field[i].low) &&
            (r1->field[i].high <= r2->field[i].high))
        {
            // Then for this field i, r1 is a subset of r2.  Keep
            // checking the rest of the fields.
        } else {
            return false;
        }
    }
    return true;
}

int compare_rules(struct pc_rule *r1, struct pc_rule *r2)
{
    bool earlier_subset, later_subset;
    if (rules_disjoint(r1, r2)) {
        return RULE_COMPARE_DISJOINT;
    }
    earlier_subset = rule_subset(r1, r2);
    later_subset = rule_subset(r2, r1);
    if (earlier_subset && later_subset) {
        return RULE_COMPARE_EQUAL;
    } else if (earlier_subset) {
        return RULE_COMPARE_EARLIER_STRICT_SUBSET;
    } else if (later_subset) {
        return RULE_COMPARE_LATER_STRICT_SUBSET;
    }
    return RULE_COMPARE_CONFLICT;
}

void print_rule(FILE *fp, struct pc_rule *r)
{
    int i;
    for (i = 0; i < MAXDIMENSIONS; i++) {
        if (i != 0) {
            fprintf(fp, " ");
        }
        fprintf(fp, "[%u, %u]", r->field[i].low, r->field[i].high);
    }
}
