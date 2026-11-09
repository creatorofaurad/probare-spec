// Sparse conditional accumulation kernel
// Stress tests vectorizer predication, translation validation, and bounds safety.

void sparse_accumulate(float* restrict dst, const float* restrict src, const int* restrict mask, int n) {
    for (int i = 0; i < n; i++) {
        if (mask[i] > 0) {
            dst[i] += src[i] * 2.5f;
        }
    }
}
