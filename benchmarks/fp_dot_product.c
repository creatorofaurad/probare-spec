#include <stdint.h>
#include <stdlib.h>

// Pure FP dot product without fused constants to test standard FP semantics
float fp_dot_product(const float* a, const float* b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}
