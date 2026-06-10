#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Render an architecture / component diagram for the Experiment 4 report.

Two figures:
  arch_component.png - the request path: k6 -> Nginx -> 3 backends -> cache -> DB
  arch_evolution.png - before/after: single-thread+no-cache vs pool+cache+LB

Drawn with matplotlib primitives so it embeds cleanly in Word without a separate
diagramming tool. Scientific muted palette, Chinese labels.
"""
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch
from matplotlib import font_manager

HERE = os.path.dirname(os.path.abspath(__file__))
RESULTS = os.path.join(HERE, "results")

_installed = {f.name for f in font_manager.fontManager.ttflist}
for _cand in ["Songti SC", "STHeiti", "Hiragino Sans GB", "Arial Unicode MS"]:
    if _cand in _installed:
        plt.rcParams["font.sans-serif"] = [_cand]
        break
plt.rcParams["axes.unicode_minus"] = False

C_CLIENT = "#0072B2"
C_LB = "#E69F00"
C_BACKEND = "#009E73"
C_CACHE = "#56B4E9"
C_DB = "#D55E00"
C_TEXT = "#1A1A1A"
EDGE = "#444444"


def _box(ax, x, y, w, h, text, fc, fontsize=11, tc="white"):
    box = FancyBboxPatch((x, y), w, h,
                         boxstyle="round,pad=0.02,rounding_size=0.06",
                         linewidth=1.2, edgecolor=EDGE, facecolor=fc, zorder=3)
    ax.add_patch(box)
    ax.text(x + w / 2, y + h / 2, text, ha="center", va="center",
            fontsize=fontsize, color=tc, fontweight="bold", zorder=4)
    return (x + w / 2, y + h / 2, w, h)


def _arrow(ax, p1, p2, text=None, color=EDGE, style="-|>"):
    a = FancyArrowPatch(p1, p2, arrowstyle=style, mutation_scale=16,
                        linewidth=1.4, color=color, zorder=2,
                        shrinkA=2, shrinkB=2)
    ax.add_patch(a)
    if text:
        mx, my = (p1[0] + p2[0]) / 2, (p1[1] + p2[1]) / 2
        ax.text(mx, my + 0.12, text, ha="center", va="bottom",
                fontsize=9, color="#333333", zorder=4)


def component_diagram():
    fig, ax = plt.subplots(figsize=(9.2, 6.4))
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 10)
    ax.axis("off")

    # client
    cx = _box(ax, 0.4, 8.4, 2.4, 1.0, "客户端 / k6\n50 并发用户", C_CLIENT, 11)
    # load balancer
    lb = _box(ax, 3.7, 8.4, 2.6, 1.0, "Nginx 负载均衡\n:8088  least_conn", C_LB, 11,
              tc="#1A1A1A")
    # three backends
    b1 = _box(ax, 0.4, 5.6, 2.7, 1.1, "后端实例1 :9001\nHttpServer", C_BACKEND, 10)
    b2 = _box(ax, 3.65, 5.6, 2.7, 1.1, "后端实例2 :9002\nThreadPool", C_BACKEND, 10)
    b3 = _box(ax, 6.9, 5.6, 2.7, 1.1, "后端实例3 :9003\nDispatcher", C_BACKEND, 10)
    # report service
    rs = _box(ax, 2.6, 3.5, 4.8, 1.0, "ReportService  日结报表汇总", "#7570B3", 11)
    # cache + db
    ca = _box(ax, 0.6, 1.3, 3.6, 1.0, "ReportCache 进程内缓存\n命中 O(1)", C_CACHE, 10,
              tc="#1A1A1A")
    db = _box(ax, 5.6, 1.3, 3.6, 1.0, "ItemRepository\nSQLite  未命中 O(n)", C_DB, 10)

    _arrow(ax, (cx[0], 8.4), (lb[0], 8.4 + 0), color=C_CLIENT)
    _arrow(ax, (1.6, 8.4), (1.6, 6.7), text="HTTP keep-alive")
    _arrow(ax, (lb[0], 8.4), (5.0, 6.7))
    _arrow(ax, (lb[0] + 0.9, 8.4), (8.25, 6.7))
    for b in (b1, b2, b3):
        _arrow(ax, (b[0], 5.6), (rs[0], 4.5))
    _arrow(ax, (rs[0] - 1.4, 3.5), (ca[0], 2.3), text="先查缓存")
    _arrow(ax, (rs[0] + 1.4, 3.5), (db[0], 2.3), text="未命中回源")

    ax.set_title("图  Gilded Rose 后端请求链路与组件结构",
                 fontsize=13, fontweight="bold", pad=12)
    fig.tight_layout()
    out = os.path.join(RESULTS, "arch_component.png")
    fig.savefig(out, dpi=150)
    plt.close(fig)
    print("wrote", out)


def evolution_diagram():
    fig, axes = plt.subplots(1, 2, figsize=(10.2, 4.6))
    titles = ["改造前  单进程串行", "改造后  并发 + 缓存 + 负载均衡"]
    for ax in axes:
        ax.set_xlim(0, 10)
        ax.set_ylim(0, 10)
        ax.axis("off")

    ax = axes[0]
    _box(ax, 2.5, 7.6, 5, 1.1, "单线程主循环\n每请求新建连接", "#999999", 11)
    _box(ax, 2.5, 4.8, 5, 1.1, "ReportService\n每次 O(n) 全量遍历", "#B0413E", 11)
    _box(ax, 2.5, 2.0, 5, 1.1, "SQLite 库存", C_DB, 11)
    _arrow(ax, (5, 7.6), (5, 5.9))
    _arrow(ax, (5, 4.8), (5, 3.1))
    ax.text(5, 0.7, "瓶颈：串行阻塞 · 无缓存 · 端口易耗尽",
            ha="center", fontsize=10, color="#B0413E", fontweight="bold")
    ax.set_title(titles[0], fontsize=12, fontweight="bold")

    ax = axes[1]
    _box(ax, 0.5, 7.6, 9, 1.0, "Nginx 负载均衡  least_conn + 故障转移", C_LB, 10.5,
         tc="#1A1A1A")
    _box(ax, 0.4, 5.0, 2.9, 1.1, "实例1\n线程池", C_BACKEND, 10)
    _box(ax, 3.55, 5.0, 2.9, 1.1, "实例2\n线程池", C_BACKEND, 10)
    _box(ax, 6.7, 5.0, 2.9, 1.1, "实例3\n线程池", C_BACKEND, 10)
    _box(ax, 1.2, 2.0, 3.4, 1.1, "命中缓存\nO(1)", C_CACHE, 10, tc="#1A1A1A")
    _box(ax, 5.4, 2.0, 3.4, 1.1, "SQLite", C_DB, 10)
    for x in (1.85, 5.0, 8.15):
        _arrow(ax, (5, 7.6), (x, 6.1))
    _arrow(ax, (3.0, 5.0), (2.9, 3.1))
    _arrow(ax, (7.0, 5.0), (7.1, 3.1))
    ax.text(5, 0.7, "并发 + keep-alive + 缓存 + 横向扩展",
            ha="center", fontsize=10, color=C_BACKEND, fontweight="bold")
    ax.set_title(titles[1], fontsize=12, fontweight="bold")

    fig.suptitle("图  架构演化对比：改造前 → 改造后", fontsize=13, fontweight="bold")
    fig.tight_layout(rect=[0, 0, 1, 0.95])
    out = os.path.join(RESULTS, "arch_evolution.png")
    fig.savefig(out, dpi=150)
    plt.close(fig)
    print("wrote", out)


if __name__ == "__main__":
    component_diagram()
    evolution_diagram()
