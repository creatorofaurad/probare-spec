import sys
import numpy as np
from scipy import stats

def analyze(filename):
    data = np.loadtxt(filename)
    n = len(data)
    p9999_idx = int(n * 0.9999)
    p9999 = np.sort(data)[p9999_idx]
    # Bootstrap 95% CI for P99.99
    boot_means = []
    for _ in range(1000):
        sample = np.random.choice(data, size=n, replace=True)
        boot_means.append(np.sort(sample)[p9999_idx])
    ci = np.percentile(boot_means, [2.5, 97.5])
    print(f"P99.99: {p9999:.2f} cycles | 95% CI: [{ci[0]:.2f}, {ci[1]:.2f}]")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        analyze(sys.argv[1])
    else:
        print("Usage: python3 analyze_tail_stats.py <timing_file.txt>")
