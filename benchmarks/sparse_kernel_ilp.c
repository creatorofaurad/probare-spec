#include <stdint.h>

__attribute__((noinline))
int32_t sparse_accumulate_ilp(const int32_t* __restrict__ a,
                              const int32_t* __restrict__ b,
                              const uint32_t* __restrict__ mask,
                              int n) {
    int32_t acc0 = 0, acc1 = 0, acc2 = 0, acc3 = 0;
    int i = 0;
    for (; i + 64 <= n; i += 64) {
        for (int chunk = 0; chunk < 4; chunk++) {
            int base = i + chunk * 16;
            for (int j = 0; j < 16; j++) {
                if (mask[base + j] != 0) {
                    int32_t prod = a[base + j] * b[base + j];
                    if      (chunk == 0) acc0 += prod;
                    else if (chunk == 1) acc1 += prod;
                    else if (chunk == 2) acc2 += prod;
                    else                 acc3 += prod;
                }
            }
        }
    }
    for (; i < n; i++) if (mask[i] != 0) acc0 += a[i] * b[i];
    return acc0 + acc1 + acc2 + acc3;
}
