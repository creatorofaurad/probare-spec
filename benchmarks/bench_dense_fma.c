#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <immintrin.h>
#include <x86intrin.h>

#define N 4096
#define ITERATIONS 100000

float a_arr[N] __attribute__((aligned(64)));
float b_arr[N] __attribute__((aligned(64)));

uint64_t times_scalar[ITERATIONS];
uint64_t times_autovec[ITERATIONS];
uint64_t times_fma_ilp[ITERATIONS];

static inline uint64_t rdtsc_fence(void) {
    unsigned int aux;
    uint64_t t = __rdtscp(&aux);
    _mm_lfence();
    return t;
}

// 1. Scalar Baseline: 1 multiply-add per iteration, serial dependency chain
__attribute__((noinline))
float run_scalar(const float* a, const float* b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

// 2. Clang Auto-Vectorized (-O3)
__attribute__((noinline))
float run_autovec(const float* restrict a, const float* restrict b, int n) {
    float sum = 0.0f;
    #pragma clang loop vectorize(enable)
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

// 3. Probare Hardware FMA Model: 8-way unrolled independent AVX2 accumulators
// Eliminates data-dependency stalls and saturates dual FMA hardware execution ports
__attribute__((noinline))
float run_fma_ilp(const float* restrict a, const float* restrict b, int n) {
    __m256 acc0 = _mm256_setzero_ps();
    __m256 acc1 = _mm256_setzero_ps();
    __m256 acc2 = _mm256_setzero_ps();
    __m256 acc3 = _mm256_setzero_ps();
    __m256 acc4 = _mm256_setzero_ps();
    __m256 acc5 = _mm256_setzero_ps();
    __m256 acc6 = _mm256_setzero_ps();
    __m256 acc7 = _mm256_setzero_ps();

    for (int i = 0; i < n; i += 64) {
        acc0 = _mm256_fmadd_ps(_mm256_load_ps(&a[i + 0]),  _mm256_load_ps(&b[i + 0]),  acc0);
        acc1 = _mm256_fmadd_ps(_mm256_load_ps(&a[i + 8]),  _mm256_load_ps(&b[i + 8]),  acc1);
        acc2 = _mm256_fmadd_ps(_mm256_load_ps(&a[i + 16]), _mm256_load_ps(&b[i + 16]), acc2);
        acc3 = _mm256_fmadd_ps(_mm256_load_ps(&a[i + 24]), _mm256_load_ps(&b[i + 24]), acc3);
        acc4 = _mm256_fmadd_ps(_mm256_load_ps(&a[i + 32]), _mm256_load_ps(&b[i + 32]), acc4);
        acc5 = _mm256_fmadd_ps(_mm256_load_ps(&a[i + 40]), _mm256_load_ps(&b[i + 40]), acc5);
        acc6 = _mm256_fmadd_ps(_mm256_load_ps(&a[i + 48]), _mm256_load_ps(&b[i + 48]), acc6);
        acc7 = _mm256_fmadd_ps(_mm256_load_ps(&a[i + 56]), _mm256_load_ps(&b[i + 56]), acc7);
    }

    acc0 = _mm256_add_ps(acc0, acc1);
    acc2 = _mm256_add_ps(acc2, acc3);
    acc4 = _mm256_add_ps(acc4, acc5);
    acc6 = _mm256_add_ps(acc6, acc7);

    acc0 = _mm256_add_ps(acc0, acc2);
    acc4 = _mm256_add_ps(acc4, acc6);
    acc0 = _mm256_add_ps(acc0, acc4);

    float buf[8] __attribute__((aligned(32)));
    _mm256_store_ps(buf, acc0);
    return buf[0] + buf[1] + buf[2] + buf[3] + buf[4] + buf[5] + buf[6] + buf[7];
}

int cmp_u64(const void* a, const void* b) {
    uint64_t va = *(const uint64_t*)a;
    uint64_t vb = *(const uint64_t*)b;
    return (va > vb) - (va < vb);
}

void print_percentiles(const char* name, uint64_t* arr, int n) {
    qsort(arr, n, sizeof(uint64_t), cmp_u64);
    uint64_t min = arr[0];
    uint64_t p50 = arr[(int)(n * 0.50)];
    uint64_t p90 = arr[(int)(n * 0.90)];
    uint64_t p99 = arr[(int)(n * 0.99)];
    uint64_t p9999 = arr[(int)(n * 0.9999)];
    uint64_t max = arr[n - 1];
    printf("%-26s | %-6lu | %-6lu | %-6lu | %-6lu | %-6lu | %-6lu\n",
           name, min, p50, p90, p99, p9999, max);
}

int main(void) {
    for (int i = 0; i < N; i++) {
        a_arr[i] = (float)(i % 13) * 0.25f;
        b_arr[i] = (float)(i % 7) * 0.5f;
    }

    // Warm-up to populate hardware L1 cache
    volatile float s = 0;
    for (int i = 0; i < 2000; i++) {
        s += run_scalar(a_arr, b_arr, N);
        s += run_autovec(a_arr, b_arr, N);
        s += run_fma_ilp(a_arr, b_arr, N);
    }

    for (int i = 0; i < ITERATIONS; i++) {
        uint64_t t0 = rdtsc_fence();
        s += run_scalar(a_arr, b_arr, N);
        uint64_t t1 = rdtsc_fence();
        times_scalar[i] = t1 - t0;
    }

    for (int i = 0; i < ITERATIONS; i++) {
        uint64_t t0 = rdtsc_fence();
        s += run_autovec(a_arr, b_arr, N);
        uint64_t t1 = rdtsc_fence();
        times_autovec[i] = t1 - t0;
    }

    for (int i = 0; i < ITERATIONS; i++) {
        uint64_t t0 = rdtsc_fence();
        s += run_fma_ilp(a_arr, b_arr, N);
        uint64_t t1 = rdtsc_fence();
        times_fma_ilp[i] = t1 - t0;
    }

    printf("==========================================================================================\n");
    printf("KERNEL                     | MIN    | P50    | P90    | P99    | P99.99 | MAX\n");
    printf("---------------------------+--------+--------+--------+--------+--------+-----------------\n");
    print_percentiles("Scalar Baseline (Serial)", times_scalar, ITERATIONS);
    print_percentiles("Clang Auto-Vec (-O3)", times_autovec, ITERATIONS);
    print_percentiles("Probare 8-Acc FMA Pipeline", times_fma_ilp, ITERATIONS);
    printf("==========================================================================================\n");

    return 0;
}
