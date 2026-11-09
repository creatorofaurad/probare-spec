#!/bin/bash
# Requires root
set -e

echo "Tuning kernel for minimal jitter..."
echo 4910 | sudo -S sh -c '
    echo 0 > /proc/sys/kernel/sched_rt_runtime_us 2>/dev/null || true
    echo 1 > /proc/sys/kernel/nmi_watchdog 2>/dev/null || true
'

# Note: isolcpus and nohz_full require GRUB reboot, log this as a prerequisite.
echo "[PREREQUISITE] To eliminate remaining scheduler noise, ensure GRUB cmdline has: isolcpus=2,3 nohz_full=2,3 rcu_nocbs=2,3"

if [ -f "./probare_predicated" ]; then
    perf record -e interrupts,page-faults,cs -o perf_data.data ./probare_predicated || true
    perf report -i perf_data.data || true
fi
