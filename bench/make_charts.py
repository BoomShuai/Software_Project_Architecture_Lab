#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate Experiment 4 scalability charts from bench/results/summary.csv.

Produces four PNGs under bench/results/:
  throughput.png  - req/s per scenario (log scale; the cache effect spans 3 orders)
  latency.png     - avg & p95 latency per scenario
  concurrency.png - A-vs-B isolated (concurrency-only, no cache) for a fair read
  speedup.png     - relative speedup vs. the single-thread baseline

Styling follows a scientific-publication palette (color-blind-safe Okabe-Ito
derived tones), muted gridlines, and Chinese labels to match the report.
"""
import csv
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib import font_manager

HERE = os.path.dirname(os.path.abspath(__file__))
RESULTS = os.path.join(HERE, "results")
CSV = os.path.join(RESULTS, "summary.csv")

# ---- Chinese font (first available on this machine) -------------------------
_installed = {f.name for f in font_manager.fontManager.ttflist}
for _cand in ["Songti SC", "STHeiti", "Hiragino Sans GB", "Arial Unicode MS"]:
    if _cand in _installed:
        plt.rcParams["font.sans-serif"] = [_cand]
        break
plt.rcParams["axes.unicode_minus"] = False

# ---- Scientific publication palette (Okabe-Ito, color-blind safe) -----------
# Blue, orange, bluish-green, vermillion -- distinguishable in grayscale too.
PALETTE = ["#0072B2", "#E69F00", "#009E73", "#D55E00"]
GRID = "#B0B0B0"
plt.rcParams.update({
    "figure.facecolor": "white",
    "axes.facecolor": "white",
    "axes.edgecolor": "#333333",
    "axes.linewidth": 0.9,
    "axes.titlesize": 13,
    "axes.labelsize": 11.5,
    "font.size": 11,
    "savefig.dpi": 150,
})

LABELS = {
    "A_baseline_single": "A 基线\n单线程·无缓存",
    "B_threadpool_nocache": "B 垂直扩展\n线程池·无缓存",
    "C_threadpool_cache": "C 加缓存\n线程池·缓存",
    "D_loadbalanced": "D 水平扩展\n3实例·负载均衡",
}
ORDER = ["A_baseline_single", "B_threadpool_nocache",
         "C_threadpool_cache", "D_loadbalanced"]


def load():
    rows = {}
    with open(CSV) as f:
        for r in csv.DictReader(f):
            rows[r["scenario"]] = r
    return [rows[k] for k in ORDER if k in rows]


def fnum(row, key):
    v = row.get(key, "")
    return float(v) if v not in ("", None) else 0.0


def _style_axes(ax, ygrid=True):
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    if ygrid:
        ax.grid(axis="y", alpha=0.35, color=GRID, linewidth=0.7,
                which="both", zorder=0)
        ax.set_axisbelow(True)


def chart_throughput(data):
    labels = [LABELS[r["scenario"]] for r in data]
    vals = [fnum(r, "reqs_per_sec") for r in data]
    fig, ax = plt.subplots(figsize=(8.6, 5.2))
    bars = ax.bar(labels, vals, color=PALETTE[:len(data)],
                  edgecolor="white", linewidth=0.8, zorder=3, width=0.62)
    ax.set_yscale("log")
    ax.set_ylabel("吞吐量  requests / second（对数坐标）")
    ax.set_title("图 1  各优化阶段吞吐量对比\n50 并发用户 · 200 万条库存 · 日结报表接口",
                 fontweight="bold")
    for b, v in zip(bars, vals):
        ax.text(b.get_x() + b.get_width() / 2, v * 1.12,
                f"{v:,.0f}", ha="center", va="bottom",
                fontsize=10.5, fontweight="bold", color="#222222")
    ax.set_ylim(top=max(vals) * 3)
    _style_axes(ax)
    fig.tight_layout()
    out = os.path.join(RESULTS, "throughput.png")
    fig.savefig(out)
    plt.close(fig)
    print("wrote", out)


def chart_latency(data):
    labels = [LABELS[r["scenario"]] for r in data]
    avg = [fnum(r, "latency_avg_ms") for r in data]
    p95 = [fnum(r, "latency_p95_ms") for r in data]
    x = range(len(labels))
    w = 0.36
    fig, ax = plt.subplots(figsize=(8.6, 5.2))
    ax.bar([i - w / 2 for i in x], avg, w, label="平均延迟",
           color=PALETTE[0], edgecolor="white", linewidth=0.8, zorder=3)
    ax.bar([i + w / 2 for i in x], p95, w, label="p95 延迟",
           color=PALETTE[1], edgecolor="white", linewidth=0.8, zorder=3)
    ax.set_yscale("log")
    ax.set_ylabel("延迟  毫秒 ms（对数坐标）")
    ax.set_xticks(list(x))
    ax.set_xticklabels(labels)
    ax.set_title("图 2  各优化阶段请求延迟对比", fontweight="bold")
    ax.legend(frameon=False, fontsize=10.5)
    _style_axes(ax)
    fig.tight_layout()
    out = os.path.join(RESULTS, "latency.png")
    fig.savefig(out)
    plt.close(fig)
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
    bars = ax.bar(labels, vals, color=[PALETTE[0], PALETTE[1]],
                  edgecolor="white", linewidth=0.8, zorder=3, width=0.55)
    ax.set_ylabel("吞吐量  requests / second")
    ax.set_title("图 3  并发效果隔离对比\n均无缓存，仅比较单线程与线程池",
                 fontweight="bold")
    for b, v in zip(bars, vals):
        ax.text(b.get_x() + b.get_width() / 2, v,
                f"{v:,.0f}", ha="center", va="bottom",
                fontweight="bold", color="#222222")
    speedup = vals[1] / vals[0] if vals[0] else 0
    ax.annotate(f"提升 {speedup:.1f} 倍",
                xy=(1, vals[1]), xytext=(0.5, max(vals) * 0.95),
                ha="center", fontsize=12, color=PALETTE[2], fontweight="bold")
    ax.set_ylim(top=max(vals) * 1.2)
    _style_axes(ax)
    fig.tight_layout()
    out = os.path.join(RESULTS, "concurrency.png")
    fig.savefig(out)
    plt.close(fig)
    print("wrote", out)


def chart_speedup(data):
    """Relative speedup vs. the single-thread baseline (log scale)."""
    base = fnum(data[0], "reqs_per_sec")
    labels = [LABELS[r["scenario"]] for r in data]
    vals = [fnum(r, "reqs_per_sec") / base for r in data]
    fig, ax = plt.subplots(figsize=(8.6, 5.2))
    bars = ax.bar(labels, vals, color=PALETTE[:len(data)],
                  edgecolor="white", linewidth=0.8, zorder=3, width=0.62)
    ax.set_yscale("log")
    ax.set_ylabel("相对基线的加速比（×，对数坐标）")
    ax.set_title("图 4  相对单线程基线的加速比", fontweight="bold")
    for b, v in zip(bars, vals):
        ax.text(b.get_x() + b.get_width() / 2, v * 1.12,
                f"{v:,.1f}×", ha="center", va="bottom",
                fontsize=10.5, fontweight="bold", color="#222222")
    ax.set_ylim(top=max(vals) * 3)
    ax.axhline(1, color=GRID, linewidth=1.0, linestyle="--", zorder=1)
    _style_axes(ax)
    fig.tight_layout()
    out = os.path.join(RESULTS, "speedup.png")
    fig.savefig(out)
    plt.close(fig)
    print("wrote", out)


if __name__ == "__main__":
    data = load()
    chart_throughput(data)
    chart_latency(data)
    chart_concurrency(data)
    chart_speedup(data)
