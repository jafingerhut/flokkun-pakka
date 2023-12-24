#include <stdio.h>
#include "rules.h"

int main(int argc, char *argv[]) {
    int i;

    printf(" i          m ?     prefix len\n");
    printf("-- ---------- - ---------- ---\n");
    for (i = 0; i <= 32; i++) {
        unsigned int m = mask(i);
        bool ismask = (((m + 1) & m) == 0);
        
        unsigned int prefix;
        int prefix_len;
        unsigned int low = 0;
        unsigned int high = m;
        min_max_to_prefix32(low, high, &prefix, &prefix_len);
        printf("%2d 0x%08x %s 0x%08x %3d\n",
               i, m, (ismask ? "T" : "F"),
               prefix, prefix_len);
        unsigned int prefix_mask;
        if (prefix_len == 0) {
            prefix_mask = 0;
        } else {
            prefix_mask = (0xffffffff >> (32 - prefix_len)) << (32 - prefix_len);
        }
        printf("    ( low & prefix_mask) = 0x%08x & 0x%08x = 0x%08x\n",
               low, prefix_mask, (low & prefix_mask));
        printf("    (high & prefix_mask) = 0x%08x & 0x%08x = 0x%08x\n",
               high, prefix_mask, (high & prefix_mask));
        if ((low & prefix_mask) != (high & prefix_mask)) {
            fprintf(stderr, "Results should be same but are not.  Test failed\n");
            exit(1);
        }
        if (prefix_len != (32-i)) {
            fprintf(stderr, "prefix_len = %d != %d = (32-i).  Test failed\n",
                    prefix_len, 32-i);
            exit(1);
        }
    }
    printf("\nAll tests passed.\n");
    exit(0);
}
