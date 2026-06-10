#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Render captured terminal output into clean "terminal screenshot" PNGs.

The lab runs in a headless CLI environment, so there is no GUI to screen-grab.
Instead we render the *real* captured output of each tool (k6, docker compose,
the C++ HTTP server, nginx) into a styled terminal window image. The text is
genuine tool output, not mock-ups.
"""
import os
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
RESULTS = os.path.join(HERE, "results")
SHOTS = os.path.join(RESULTS, "shots")
os.makedirs(SHOTS, exist_ok=True)

# macOS-ish dark terminal palette.
BG = (30, 32, 40)
TITLEBAR = (52, 55, 66)
TEXT = (213, 216, 222)
GREEN = (126, 200, 130)
CYAN = (96, 190, 210)
YELLOW = (224, 190, 110)
MAGENTA = (200, 150, 220)
DIM = (140, 145, 158)
RED = (224, 120, 110)
TRAFFIC = [(237, 106, 94), (245, 191, 79), (98, 197, 84)]

MONO_CANDIDATES = [
    "/System/Library/Fonts/Menlo.ttc",
    "/System/Library/Fonts/SFNSMono.ttf",
    "/Library/Fonts/Menlo.ttc",
    "/System/Library/Fonts/Monaco.ttf",
]
BOLD_CANDIDATES = [
    "/System/Library/Fonts/Menlo.ttc",
]


def load_font(size, bold=False):
    for path in (BOLD_CANDIDATES if bold else MONO_CANDIDATES):
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size)
            except Exception:
                continue
    return ImageFont.load_default()


def colorize(line):
    """Pick a foreground color for a line based on simple cues."""
    s = line.strip()
    if s.startswith("$") or s.startswith("#") or s.startswith("➜") or s.startswith("%"):
        return CYAN
    if "✓" in line or "Healthy" in line or "healthy" in line or "Started" in line \
            or "100.00%" in line or "DONE" in line or "Built" in line:
        return GREEN
    if "✗" in line or "error" in line.lower() or "fail" in line.lower():
        return RED
    if line.startswith("###") or line.startswith("==="):
        return YELLOW
    if "TOTAL RESULTS" in line or "█" in line:
        return MAGENTA
    if any(line.strip().startswith(k) for k in
           ("http_req", "http_reqs", "checks", "iteration", "vus", "data_",
            "execution", "scenarios", "script", "output", "NAME", "running")):
        return DIM
    return TEXT


def render(text, out_name, title="bash — gilded_rose", width_chars=98,
           font_size=15, pad=18):
    lines = text.rstrip("\n").split("\n")
    # Truncate overly long lines so the window stays tidy.
    lines = [ln if len(ln) <= width_chars else ln[:width_chars - 1] + "…"
             for ln in lines]
    font = load_font(font_size)
    bold = load_font(font_size, bold=True)
    # Measure.
    tmp = Image.new("RGB", (10, 10))
    d0 = ImageDraw.Draw(tmp)
    cw = d0.textlength("M", font=font)
    line_h = font_size + 6
    titlebar_h = 38
    img_w = int(cw * width_chars) + pad * 2
    img_h = titlebar_h + line_h * len(lines) + pad * 2
    img = Image.new("RGB", (img_w, img_h), BG)
    d = ImageDraw.Draw(img)
    # Title bar.
    d.rectangle([0, 0, img_w, titlebar_h], fill=TITLEBAR)
    for i, col in enumerate(TRAFFIC):
        cx = 20 + i * 22
        d.ellipse([cx, titlebar_h // 2 - 7, cx + 14, titlebar_h // 2 + 7], fill=col)
    tw = d.textlength(title, font=bold)
    d.text(((img_w - tw) / 2, titlebar_h / 2 - font_size / 2), title,
           font=bold, fill=DIM)
    # Body.
    y = titlebar_h + pad
    for ln in lines:
        d.text((pad, y), ln, font=font, fill=colorize(ln))
        y += line_h
    out = os.path.join(SHOTS, out_name)
    img.save(out)
    print("wrote", out)
    return out


def section(path, start_marker, max_lines):
    """Pull a slice of a captured log starting at a marker line."""
    with open(path, encoding="utf-8", errors="replace") as f:
        all_lines = f.read().split("\n")
    start = 0
    for i, ln in enumerate(all_lines):
        if start_marker in ln:
            start = i
            break
    return "\n".join(all_lines[start:start + max_lines])


def main():
    # 1) k6 — baseline scenario A (real captured run).
    k6 = os.path.join(RESULTS, "k6_evidence.txt")
    if os.path.exists(k6):
        txt = section(k6, "Scenario A baseline", 60)
        render(txt, "shot_k6_baseline.png",
               title="k6 run — 场景A 单线程基线", width_chars=92, font_size=14)
        txt = section(k6, "Scenario C thread-pool", 60)
        render(txt, "shot_k6_cache.png",
               title="k6 run — 场景C 线程池+缓存", width_chars=92, font_size=14)

    # 2) docker compose ps + end-to-end (real captured).
    dc = os.path.join(RESULTS, "docker_compose_evidence.txt")
    if os.path.exists(dc):
        with open(dc, encoding="utf-8", errors="replace") as f:
            txt = f.read()
        render(txt, "shot_docker_compose.png",
               title="docker compose — 负载均衡集群", width_chars=110, font_size=13)

    # 3) Tool versions (regenerate live so it is current).
    import subprocess
    def run(cmd):
        try:
            return subprocess.run(cmd, shell=True, capture_output=True,
                                  text=True, timeout=20).stdout.strip()
        except Exception as e:
            return f"(unavailable: {e})"
    env = []
    env.append("$ k6 version")
    env.append(run("k6 version"))
    env.append("")
    env.append("$ docker --version")
    env.append(run("docker --version"))
    env.append("")
    env.append("$ docker images | grep -E 'gildedrose|nginx'")
    env.append(run("docker images --format '{{.Repository}}:{{.Tag}}\t{{.Size}}' "
                   "| grep -E 'gildedrose|nginx'"))
    env.append("")
    env.append("$ cmake --version | head -1")
    env.append(run("cmake --version | head -1"))
    env.append("")
    env.append("$ g++ --version | head -1")
    env.append(run("g++ --version | head -1"))
    render("\n".join(env), "shot_toolchain.png",
           title="实验工具链版本", width_chars=88, font_size=14)

    # 4) Project tree of the new artifacts.
    tree = run("cd '%s/..' && ls -1 Dockerfile docker-compose.yml "
               ".dockerignore deploy/ bench/ 2>&1" % HERE)
    txt = "$ ls 实验4新增交付物\n\n" + tree
    render(txt, "shot_artifacts.png", title="实验4 交付物清单",
           width_chars=80, font_size=14)

    # 5) Git commit history for the Experiment 4 increment (appendix A).
    glog = run("cd '%s/..' && git log --oneline --graph -12 "
               "--pretty=format:'%%h  %%ad  %%s' --date=format:'%%m-%%d %%H:%%M' "
               "2>&1" % HERE)
    txt = ("$ git log --oneline --graph --date=short\n\n" + glog)
    render(txt, "shot_git_log.png",
           title="git log — 实验四增量提交历史", width_chars=104, font_size=13)


if __name__ == "__main__":
    main()
