#!/usr/bin/env python3
"""Generate Experiment 4 scalability charts from bench/results/summary.csv.

Produces three PNGs under bench/results/:
  throughput.png  - req/s per scenario (log scale; the cache effect spans 3 orders)
  latency.png     - avg & p95 latency per scenario
  concurrency.png - A-vs-B isolated (concurrency-only, no cache) for a fair read

These charts are embedded in the experiment report.
"""
import csv
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

HERE = os.path.dirname(os.path.abspath(__file__))
RESULTS = os.path.join(HERE, "results")
CSV = os.path.join(RESULTS, "summary.csv")

LABELS = {
    "A_baseline_single": "A. Baseline\n(single-thread)",
    "B_threadpool_nocache": "B. Thread pool\n(no cache)",
    "C_threadpool_cache": "C. Thread pool\n+ cache",
    "D_loadbalanced": "D. 3 instances\n+ Nginx LB",
}
ORDER = ["A_baseline_single", "B_threadpool_nocache",
         "C_threadpool_cache", "D_loadbalanced"]
COLORS = ["#b0413e", "#e0903a", "#3a8d5b", "#3a6ea5"]


def load():
    rows = {}
    with open(CSV) as f:
        for r in csv.DictReader(f):
            rows[r["scenario"]] = r
    return [rows[k] for k in ORDER if k in rows]


def fnum(row, key):
    v = row.get(key, "")
    return float(v) if v not in ("", None) else 0.0


def chart_throughput(data):
    labels = [LABELS[r["scenario"]] for r in data]
    vals = [fnum(r, "reqs_per_sec") for r in data]
    fig, ax = plt.subplots(figsize=(9, 5.5))
    bars = ax.bar(labels, vals, color=COLORS[:len(data)])
    ax.set_yscale("log")
    ax.set_ylabel("Throughput (requests / second, log scale)")
    ax.set_title("Experiment 4: Throughput by Optimization Stage\n"
                 "Read-heavy report endpoint, 50 concurrent users, 2M-item inventory")
    for b, v in zip(bars, vals):
        ax.text(b.get_x() + b.get_width() / 2, v * 1.1,
                f"{v:,.0f}", ha="center", va="bottom", fontsize=10, fontweight="bold")
    ax.grid(axis="y", alpha=0.3, which="both")
    fig.tight_layout()
    out = os.path.join(RESULTS, "throughput.png")
    fig.savefig(out, dpi=130)
    print("wrote", out)


def chart_latency(data):
    labels = [LABELS[r["scenario"]] for r in data]
    avg = [fnum(r, "latency_avg_ms") for r in data]
    p95 = [fnum(r, "latency_p95_ms") for r in data]
    x = range(len(labels))
    w = 0.38
    fig, ax = plt.subplots(figsize=(9, 5.5))
    ax.bar([i - w / 2 for i in x], avg, w, label="avg", color="#3a6ea5")
    ax.bar([i + w / 2 for i in x], p95, w, label="p95", color="#e0903a")
    ax.set_yscale("log")
    ax.set_ylabel("Latency (ms, log scale)")
    ax.set_xticks(list(x))
    ax.set_xticklabels(labels)
    ax.set_title("Experiment 4: Request Latency by Optimization Stage")
    ax.legend()
    ax.grid(axis="y", alpha=0.3, which="both")
    fig.tight_layout()
    out = os.path.join(RESULTS, "latency.png")
    fig.savefig(out, dpi=130)
    print("wrote", out)


def chart_concurrency(data):
    """A vs B only: both uncached, so this isolates the concurrency effect
    that the cache otherwise dwarfs on the full chart."""
    sub = [r for r in data if r["scenario"] in
           ("A_baseline_single", "B_threadpool_nocache")]
    if len(sub) < 2:
        return
    labels = [LABELS[r["scenario"]] for r in sub]
    vals = [fnum(r, "reqs_per_sec") for r in sub]
    fig, ax = plt.subplots(figsize=(6, 5))
    bars = ax.bar(labels, vals, color=["#b0413e", "#e0903a"])
    ax.set_ylabel("Throughput (requests / second)")
    ax.set_title("Concurrency in isolation (no cache)\n"
                 "Thread pool vs single-thread on CPU-bound work")
    for b, v in zip(bars, vals):
        ax.text(b.get_x() + b.get_width() / 2, v,
                f"{v:,.0f}", ha="center", va="bottom", fontweight="bold")
    speedup = vals[1] / vals[0] if vals[0] else 0
    ax.text(0.5, 0.92, f"{speedup:.1f}x speedup", transform=ax.transAxes,
            ha="center", fontsize=12, color="#3a8d5b", fontweight="bold")
    ax.grid(axis="y", alpha=0.3)
    fig.tight_layout()
    out = os.path.join(RESULTS, "concurrency.png")
    fig.savefig(out, dpi=130)
    print("wrote", out)


if __name__ == "__main__":
    data = load()
    chart_throughput(data)
    chart_latency(data)
    chart_concurrency(data)
