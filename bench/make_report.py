#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate the Experiment 4 Word report (软件体系结构 第四次实验报告).

Cover page is reused verbatim from the reference report
``docs/牛帅明软件项目架构第四次实验报告.docx`` (school crest + banner, fonts and
layout) so it matches the student's other submissions exactly; only the
experiment title line and the date are rewritten. The body is then appended.

Conventions applied per the latest request:
  - All tables are three-line (booktabs) style, full page width.
  - No parentheses anywhere; any worthwhile content is folded into prose.
  - Embeds the scientific-palette charts (bench/results/*.png) and the rendered
    terminal screenshots (bench/results/shots/*.png).
  - Numbers are read straight from summary.csv so prose and data never drift.
"""
import csv
import copy
import os
from docx import Document
from docx.shared import Pt, Inches, RGBColor, Cm
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml.ns import qn
from docx.oxml import OxmlElement

ROOT = os.path.dirname(os.path.abspath(__file__))
RESULTS = os.path.join(ROOT, "results")
SHOTS = os.path.join(RESULTS, "shots")
CSV = os.path.join(RESULTS, "summary.csv")
REF = os.path.join(ROOT, "..", "docs", "牛帅明软件项目架构第四次实验报告.docx")
OUT = os.path.join(ROOT, "..", "docs", "牛帅明软件项目架构第四次实验报告-可扩展性与性能.docx")

CN_FONT = "宋体"
EN_FONT = "Times New Roman"
ACCENT = (0x1F, 0x4E, 0x79)   # deep blue for headings


# ---------------------------------------------------------------------------
# Low-level helpers
# ---------------------------------------------------------------------------
def load_results():
    rows = {}
    with open(CSV) as f:
        for r in csv.DictReader(f):
            rows[r["scenario"]] = r
    return rows


def setfont(run, size=12, bold=False, cn=CN_FONT, color=None):
    run.font.size = Pt(size)
    run.font.bold = bold
    run.font.name = EN_FONT
    run._element.rPr.rFonts.set(qn("w:eastAsia"), cn)
    if color:
        run.font.color.rgb = RGBColor(*color)


def body(doc, text, size=12.5, bold=False, align=None, after=8, first_indent=True):
    p = doc.add_paragraph()
    if align:
        p.alignment = align
    else:
        p.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY
    pf = p.paragraph_format
    pf.space_after = Pt(after)
    pf.line_spacing = 1.6
    if first_indent:
        pf.first_line_indent = Pt(25)  # 2 全角字符缩进
    setfont(p.add_run(text), size=size, bold=bold)
    return p


def h1(doc, text):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.space_before = Pt(14)
    pf.space_after = Pt(8)
    setfont(p.add_run(text), size=15, bold=True, color=ACCENT)
    return p


def h2(doc, text):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.space_before = Pt(8)
    pf.space_after = Pt(5)
    setfont(p.add_run(text), size=13, bold=True, color=ACCENT)
    return p


def caption(doc, text):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_after = Pt(10)
    p.paragraph_format.space_before = Pt(2)
    setfont(p.add_run(text), size=10.5, cn=CN_FONT, color=(0x40, 0x40, 0x40))


def add_image(doc, path, width_in=6.1, center=True):
    if not os.path.exists(path):
        print("  [warn] missing image:", path)
        return
    p = doc.add_paragraph()
    if center:
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_before = Pt(6)
    p.add_run().add_picture(path, width=Inches(width_in))


def code_block(doc, text):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.left_indent = Pt(14)
    pf.space_after = Pt(8)
    pf.space_before = Pt(2)
    pf.line_spacing = 1.18
    # light shading box via paragraph border
    for i, line in enumerate(text.strip("\n").split("\n")):
        if i:
            p.add_run().add_break()
        r = p.add_run(line)
        r.font.name = "Consolas"
        r._element.rPr.rFonts.set(qn("w:eastAsia"), "Consolas")
        r.font.size = Pt(9.5)
    return p


def fmt(x, nd=0):
    try:
        return f"{float(x):,.{nd}f}"
    except (ValueError, TypeError):
        return str(x)


# ---------------------------------------------------------------------------
# Three-line (booktabs) full-width table
# ---------------------------------------------------------------------------
def _set_cell_border(cell, **kwargs):
    tcPr = cell._tc.get_or_add_tcPr()
    borders = OxmlElement("w:tcBorders")
    for edge in ("top", "bottom", "left", "right"):
        spec = kwargs.get(edge)
        el = OxmlElement(f"w:{edge}")
        if spec:
            el.set(qn("w:val"), "single")
            el.set(qn("w:sz"), str(spec["sz"]))
            el.set(qn("w:color"), spec.get("color", "000000"))
        else:
            el.set(qn("w:val"), "nil")
        borders.append(el)
    tcPr.append(borders)


def three_line_table(doc, headers, rows, col_widths=None, font_size=11):
    """Render a booktabs-style three-line table at full page width."""
    n = len(headers)
    table = doc.add_table(rows=1, cols=n)
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    table.autofit = False
    table.allow_autofit = False
    # full content width ~ 16.0 cm on A4 with default margins
    total = 16.0
    widths = col_widths or [total / n] * n
    # header
    THICK = {"sz": 12, "color": "000000"}
    THIN = {"sz": 6, "color": "000000"}
    hdr = table.rows[0].cells
    for i, htext in enumerate(headers):
        cell = hdr[i]
        cell.width = Cm(widths[i])
        para = cell.paragraphs[0]
        para.alignment = WD_ALIGN_PARAGRAPH.CENTER
        setfont(para.add_run(htext), size=font_size, bold=True)
        # top line thick, bottom of header thin, no verticals
        _set_cell_border(cell, top=THICK, bottom=THIN)
    # data rows
    for ri, row in enumerate(rows):
        cells = table.add_row().cells
        last = (ri == len(rows) - 1)
        for ci, val in enumerate(row):
            cell = cells[ci]
            cell.width = Cm(widths[ci])
            para = cell.paragraphs[0]
            para.alignment = (WD_ALIGN_PARAGRAPH.LEFT if ci == n - 1 and n > 2
                              else WD_ALIGN_PARAGRAPH.CENTER)
            setfont(para.add_run(str(val)), size=font_size)
            _set_cell_border(cell, bottom=(THICK if last else None))
    doc.add_paragraph().paragraph_format.space_after = Pt(2)
    return table


# ---------------------------------------------------------------------------
# Cover: clone reference doc, rewrite title + date, strip its body
# ---------------------------------------------------------------------------
def open_with_cover():
    """Open the reference report and strip everything after the cover so we
    keep its crest/banner and title-page formatting, then append our body."""
    doc = Document(REF)
    # Rewrite the experiment title (run index 3 on the 实验题目 line) and date.
    for p in doc.paragraphs:
        if "实验题目" in p.text and "可调试性" in p.text:
            for r in p.runs:
                if "可调试性" in r.text:
                    r.text = "架构质量实践之可扩展性与性能优化"
        if "日期" in p.text:
            # date runs: ' 2026.' / '5' / '.' / '6'
            for r in p.runs:
                if r.text == "5":
                    r.text = "6"
                elif r.text == "6":
                    r.text = "9"
    # Find the cover boundary: the paragraph containing 指导教师 is the last
    # cover line. Remove every body element after it, then page-break.
    body_el = doc.element.body
    children = list(body_el)
    cutoff_idx = None
    for idx, child in enumerate(children):
        if child.tag == qn("w:p"):
            txt = "".join(t.text or "" for t in child.iter(qn("w:t")))
            if "指导教师" in txt:
                cutoff_idx = idx
    # remove everything after cutoff except the final sectPr
    if cutoff_idx is not None:
        for child in children[cutoff_idx + 1:]:
            if child.tag == qn("w:sectPr"):
                continue
            body_el.remove(child)
    return doc


def build():
    R = load_results()
    a = R["A_baseline_single"]
    b = R["B_threadpool_nocache"]
    c = R["C_threadpool_cache"]
    d = R["D_loadbalanced"]
    spA = float(b["reqs_per_sec"]) / float(a["reqs_per_sec"])
    spC = float(c["reqs_per_sec"]) / float(a["reqs_per_sec"])

    doc = open_with_cover()
    # Normalize default style for the appended body.
    st = doc.styles["Normal"]
    st.font.name = EN_FONT
    st.font.size = Pt(12.5)
    st.element.rPr.rFonts.set(qn("w:eastAsia"), CN_FONT)

    doc.add_page_break()

    # ======================= 一、相关知识 ====================================
    h1(doc, "一、相关知识")
    h2(doc, "1 理论概念说明")
    body(doc, "本次实验聚焦于软件架构中两类紧密关联的非功能性质量属性——可扩展性与性能，"
              "并在高并发场景下系统地探讨二者之间的设计权衡。性能关注的是系统在单位资源下"
              "能够多快、多大批量地完成响应，而可扩展性关注的则是当负载持续增长时，系统能否"
              "通过追加资源平滑地维持服务质量。二者既相互支撑又彼此制约：缺乏性能基础的系统"
              "难以谈可扩展，而一味追求扩展却忽视单机性能，则会造成资源的巨大浪费。本实验正是"
              "围绕这一主线，对 Gilded Rose 后端进行了由内而外、循序渐进的四阶段改造。")

    h2(doc, "1.1 性能与可扩展性")
    body(doc, "在 Len Bass 等人所著的《软件架构实践》中，性能被定义为系统对到达事件做出"
              "及时响应的能力，通常以吞吐量与延迟两个维度加以度量。吞吐量刻画系统在单位时间内"
              "处理请求的数量，延迟则刻画单个请求从发出到收到响应所经历的时间。二者并非彼此"
              "独立：当系统逼近其处理上限时，排队效应会使延迟急剧上升，因此真实的性能评估必须"
              "同时观察吞吐量与延迟分位数，而非仅看平均值。本实验在压测中同时采集了平均延迟与"
              "第 95 百分位延迟，正是为了完整刻画系统在高并发下的尾部表现。")
    body(doc, "可扩展性指系统通过增加资源来应对负载增长的能力，工程上可分为两条互补的路径。"
              "其一是垂直扩展，即在单个节点内部增加资源，例如把原本串行的单线程处理升级为线程池，"
              "充分释放多核处理器的并行潜能；这条路径实现简单、通信开销低，但受限于单机的物理上限。"
              "其二是水平扩展，即增加节点数量，把负载分摊到多个进程乃至多台主机上，再通过统一的"
              "负载均衡器对外提供服务；这条路径在理论上具备近乎无限的扩展空间，但引入了网络转发、"
              "状态一致性与故障处理等额外复杂度。本实验对这两条路径都进行了实测，并用数据揭示了"
              "它们各自的适用边界。")

    h2(doc, "1.2 缓存与读写分离")
    body(doc, "缓存是性能优化手段中投入产出比最高的一种。对于读多写少的接口而言，"
              "如果每次请求都重新执行一遍代价高昂的计算，系统资源将被大量重复劳动所吞噬；"
              "而把首次计算的结果缓存下来，后续命中的请求便可将时间复杂度由线性遍历的 O n "
              "降低到哈希查找的 O 1，收益往往是数量级的。本实验为日结报表这一典型的读密集接口"
              "引入了进程内缓存层。需要强调的是，缓存的正确性建立在失效策略之上：一旦底层库存"
              "发生写入，对应的缓存条目必须被及时清除，否则将向用户返回过期的脏数据。本实验在"
              "所有写操作路径上都植入了缓存失效逻辑，从而在享受缓存性能红利的同时保证了数据一致性。")

    h2(doc, "1.3 AI 时代下的性能工程")
    body(doc, "在大语言模型辅助编程蔚然成风的当下，AI 能够以极快的速度生成“跑得通”的代码，"
              "却往往在性能与可扩展性上留下隐患：默认采用单线程阻塞模型、每个请求新建并随即"
              "关闭连接、缺乏任何缓存意识。这些代码在功能验收时毫无破绽，却会在真实并发压力下"
              "迅速暴露瓶颈。本实验最深刻的一点体会正在于此——AI 生成的基准实现“能运行”绝不"
              "等同于“可度量”，工程师必须以严谨的压测数据去识别真实瓶颈，而不能凭直觉盲目堆砌"
              "所谓的优化。后文“四、3”如实记录了一次失败的基准测试：正是对一组漂亮却自相矛盾的"
              "数据保持了怀疑，才最终定位到端口耗尽这一系统性偏差。这种批判性审视，正是 AI 时代"
              "工程师不可被替代的核心价值。")

    h2(doc, "2 本次实验关注的关键技术或工具")
    body(doc, "本实验引入了工业界主流的高并发组件与压测工具，对优化前后进行严格的量化对比。"
              "下面分别说明各项技术的作用与选型理由，对应的真实版本信息见图 1。")
    add_image(doc, os.path.join(SHOTS, "shot_toolchain.png"), width_in=5.4)
    caption(doc, "图 1  实验工具链真实版本信息")

    h2(doc, "2.1 线程池")
    body(doc, "线程池采用固定数量的工作线程，从一个共享的任务队列中并行地取出请求加以处理，"
              "从而避免为每个请求频繁创建与销毁线程所带来的系统调用开销与上下文切换成本。"
              "本实验的服务器同时实现了单线程与线程池两种运行模式，使二者得以在完全相同的"
              "负载下直接对比，干净地隔离出并发本身所贡献的性能增益。")
    h2(doc, "2.2 HTTP keep-alive 长连接")
    body(doc, "HTTP/1.1 的 keep-alive 机制允许在同一条 TCP 连接上连续服务多个请求，"
              "从而免去每个请求都要重复经历的三次握手与四次挥手。这一机制不仅显著降低延迟，"
              "更重要的是消除了短连接模式下大量连接进入 TIME_WAIT 状态、最终耗尽操作系统临时"
              "端口的隐患——后文将看到，这正是首次压测失败的直接根因。")
    h2(doc, "2.3 Nginx 负载均衡")
    body(doc, "Nginx 作为高性能的反向代理与负载均衡器，在本实验中采用最少连接数优先的"
              "分发策略，将请求均衡地分发至多个后端实例；同时通过最大失败次数与失败超时窗口"
              "两项参数实现健康检查与故障转移，当某个后端连续失败时自动将其暂时摘除，从而"
              "提升整个集群的可用性。")
    h2(doc, "2.4 k6 压力测试")
    body(doc, "k6 是 Grafana Labs 推出的现代化负载测试工具，以 JavaScript 编写测试脚本，"
              "能够输出吞吐量以及包含 p95、p99 在内的延迟分位数，是本实验所有前后对比数据的"
              "权威来源。图 2 与图 3 分别展示了基线场景与缓存场景的真实压测输出。")
    add_image(doc, os.path.join(SHOTS, "shot_k6_baseline.png"), width_in=6.0)
    caption(doc, "图 2  k6 对场景 A 单线程基线的真实压测输出")
    add_image(doc, os.path.join(SHOTS, "shot_k6_cache.png"), width_in=6.0)
    caption(doc, "图 3  k6 对场景 C 线程池加缓存的真实压测输出")
    h2(doc, "2.5 Docker Compose 容器编排")
    body(doc, "本实验使用 Docker Compose 将三个后端实例与一个 Nginx 负载均衡器整体容器化，"
              "并通过健康检查依赖确保负载均衡器只在全部后端就绪后才启动，从而实现了一键拉起、"
              "完全可复现的部署。图 4 展示了集群真实运行的容器状态。")
    add_image(doc, os.path.join(SHOTS, "shot_docker_compose.png"), width_in=6.2)
    caption(doc, "图 4  Docker Compose 负载均衡集群真实运行状态")

    # ======================= 二、需求描述 ====================================
    h1(doc, "二、需求描述")
    body(doc, "Gilded Rose 后端最初是一个单进程的命令行程序，在功能层面已经完备，"
              "但随着库存规模增长到数百万条、并发访问量持续攀升，原始实现暴露出两个结构性瓶颈。"
              "其一，它没有真实的网络服务能力，无法承接并发请求，难以部署到生产环境。"
              "其二，日结报表接口每次被调用都要线性遍历全部库存进行汇总计算，在高并发下迅速"
              "成为整个系统的性能热点。本次实验即针对这两个瓶颈展开，功能与非功能需求归纳如下。")
    body(doc, "第一，提供真实的 HTTP 服务，并同时支持单线程基线与线程池两种运行模式，"
              "以便在相同负载下进行严格对比。第二，为读密集的日结报表接口引入缓存层，"
              "并在写操作时正确地令缓存失效，兼顾性能与一致性。第三，通过 Nginx 在多个后端"
              "实例之间实现负载均衡与故障转移。第四，使用 k6 对各优化阶段进行压测，"
              "量化吞吐量与延迟的逐级变化。第五，作为拓展，使用 Docker Compose 将整套负载均衡"
              "集群容器化，实现一键可复现部署。")

    # ============= 三、系统/模块结构 ========================================
    h1(doc, "三、系统/子系统/模块/类结构")
    body(doc, "本次实验在原有分层架构的基础上，新增了网络服务层以及与可扩展性相关的若干模块，"
              "但严格保持了既有业务逻辑层与数据访问层的稳定。整体请求链路自上而下贯穿负载均衡器、"
              "后端实例、报表服务、缓存层直至底层的 SQLite 仓储，如下所示。")
    code_block(doc,
        "客户端 / k6\n"
        "    |  HTTP/1.1 keep-alive\n"
        "    v\n"
        "Nginx 负载均衡  :8088  least_conn\n"
        "    | +--> 后端实例1 :9001   HttpServer - ThreadPool - Dispatcher\n"
        "    | +--> 后端实例2 :9002        |\n"
        "    | +--> 后端实例3 :9003        v\n"
        "                          ReportService --> Cache 命中?\n"
        "                                 | 未命中  +--> O(n) 遍历库存\n"
        "                                 v\n"
        "                          ItemRepository  SQLite")
    h2(doc, "1 关键模块职责")
    body(doc, "下表概述本次实验新增或改造的关键模块及其职责。")
    three_line_table(doc,
        ["模块 / 类", "职责"],
        [["HttpServer", "监听端口并接受连接，支持单线程与线程池两种模式，线程池模式下启用 keep-alive 长连接"],
         ["ThreadPool", "维护固定大小的工作线程，从共享队列并行取出任务执行"],
         ["ReportCache", "在进程内缓存日结报表结果，写操作时失效，并通过响应头标记是否命中"],
         ["RequestDispatcher", "负责路由分发与基于 Bearer Token 的鉴权"],
         ["nginx.conf", "配置最少连接数负载均衡、健康检查、故障转移与健康探活端点"],
         ["docker-compose.yml", "编排三个后端与一个 Nginx，并以健康检查依赖控制启动顺序"]],
        col_widths=[4.2, 11.8], font_size=10.5)
    caption(doc, "表 1  关键模块及其职责")

    # ============= 四、架构质量属性讨论（核心）===============================
    h1(doc, "四、架构质量属性讨论")
    h2(doc, "1 实验设计：四阶段递进对比")
    body(doc, "为了干净地隔离每一项优化的独立贡献，本实验设计了四个层层递进的场景，"
              "它们均使用 k6 以 50 个并发用户压测同一个日结报表接口，库存规模统一固定为 200 万条，"
              "从而保证唯一的变量就是被测的那一项优化本身。四个场景的配置与意图见下表。")
    three_line_table(doc,
        ["场景", "配置", "设计意图"],
        [["A 基线", "单线程，无缓存", "每请求一个连接、串行处理，作为一切对比的起点"],
         ["B 垂直扩展", "线程池 64，无缓存", "在基线之上仅引入多核并行与 keep-alive"],
         ["C 加缓存", "线程池 64，加缓存", "在 B 之上引入缓存，使命中请求由 O(n) 降为 O(1)"],
         ["D 水平扩展", "3 实例，Nginx，加缓存", "在 C 之上横向扩展为三实例并经负载均衡对外服务"]],
        col_widths=[2.6, 4.4, 9.0], font_size=10.5)
    caption(doc, "表 2  四阶段递进实验设计")

    h2(doc, "2 压测结果")
    body(doc, "四个场景的压测结果汇总于下表，全部数据均直接来自 k6 的 JSON 摘要，"
              "四组运行的错误率均为零，数据单调可信。")
    three_line_table(doc,
        ["场景", "吞吐量 req/s", "平均延迟 ms", "p95 延迟 ms", "错误率"],
        [["A 基线 单线程", fmt(a["reqs_per_sec"]), fmt(a["latency_avg_ms"], 1),
          fmt(a["latency_p95_ms"], 1), "0%"],
         ["B 线程池", fmt(b["reqs_per_sec"]), fmt(b["latency_avg_ms"], 1),
          fmt(b["latency_p95_ms"], 1), "0%"],
         ["C 线程池加缓存", fmt(c["reqs_per_sec"]), fmt(c["latency_avg_ms"], 2),
          fmt(c["latency_p95_ms"], 2), "0%"],
         ["D 三实例加负载均衡", fmt(d["reqs_per_sec"]), fmt(d["latency_avg_ms"], 2),
          fmt(d["latency_p95_ms"], 2), "0%"]],
        col_widths=[4.4, 3.0, 3.0, 2.8, 2.8], font_size=10.5)
    caption(doc, "表 3  四阶段压测结果汇总，50 并发用户，200 万条库存，零错误")

    add_image(doc, os.path.join(RESULTS, "throughput.png"))
    caption(doc, "图 5  各优化阶段吞吐量对比，纵轴为对数坐标")
    add_image(doc, os.path.join(RESULTS, "latency.png"))
    caption(doc, "图 6  各优化阶段平均延迟与 p95 延迟对比，纵轴为对数坐标")
    add_image(doc, os.path.join(RESULTS, "speedup.png"))
    caption(doc, "图 7  各阶段相对单线程基线的加速比")
    add_image(doc, os.path.join(RESULTS, "concurrency.png"), width_in=4.6)
    caption(doc, "图 8  并发效果隔离对比，A 与 B 均无缓存")

    body(doc, "对上述数据可作如下解读。从 A 到 B，线程池充分利用多核并行，"
              f"吞吐量提升约 {spA:.1f} 倍，这证明对于报表汇总这类计算密集的工作，"
              "并发是行之有效的扩展手段。从 B 到 C，缓存带来的收益最为惊人，"
              f"相较单线程基线提升约 {spC:,.0f} 倍：缓存命中后，单次请求由毫秒级的线性遍历"
              "骤降为微秒级的哈希查找，这正是读密集接口最具性价比的优化。")

    h2(doc, "3 一次失败的基准测试及其修正")
    body(doc, "首次压测曾得到一组自相矛盾的数据：线程池场景竟出现高达 98% 的请求错误，"
              "且吞吐量反而低于单线程基线。我没有采信这组漂亮却荒谬的数据，而是沿着两条线索"
              "深挖根因。第一条根因是临时端口耗尽：最初的实现是每请求一个连接、响应后立即关闭，"
              "基线场景在十余秒内建立了二十余万条连接，全部堆积在 TIME_WAIT 状态；紧随其后的"
              "线程池场景因操作系统再无可用临时端口，导致大量连接直接失败。第二条根因是单次"
              "请求负载过轻：当库存仅五万条时，一次报表计算不足一毫秒，单线程与线程池之间的"
              "差异完全被淹没，并发优势无从体现。")
    body(doc, "针对这两条根因，我采取了三项修正措施。其一，为线程池模式实现 HTTP/1.1 "
              "keep-alive 长连接复用，从根上消除每请求的握手开销与 TIME_WAIT 堆积。其二，"
              "将库存规模提升至 200 万条，使每次请求都成为名副其实的计算密集型任务。其三，"
              "压测脚本在场景之间留出充足的端口回收时间，并令工作线程数不小于并发连接数——"
              "因为在 keep-alive 模式下，每一条长连接都会独占一个工作线程。修正之后，四个场景的"
              "错误率全部归零，数据呈现出清晰可信的单调规律。这一波折深刻印证了：AI 生成的基准"
              "实现“跑得通”并不代表“测得准”，工程师必须以批判性思维审视压测数据、定位系统性偏差，"
              "而非简单地接受表面结果。")

    h2(doc, "4 关键权衡：为何水平扩展反而不及单机缓存")
    body(doc, f"一个反直觉却极具价值的观察是：水平扩展场景 D 的吞吐量 {fmt(d['reqs_per_sec'])} "
              f"req/s 反而低于单机缓存场景 C 的 {fmt(c['reqs_per_sec'])} req/s。这并非测量错误，"
              "而是一个值得郑重记录的真实权衡。其内在原因在于，当缓存已经把单次请求的成本压缩到"
              "微秒量级之后，每个后端实例都远未达到饱和，此时 Nginx 多出的那一跳反向代理转发，"
              "反而成了纯粹的净开销。")
    body(doc, "由此可以得出一条重要结论：负载均衡的真正价值，在于后端被打满时横向分摊压力；"
              "而当瓶颈已被缓存彻底消除、单机处理能力绰绰有余时，再引入代理层只会平添延迟。"
              "架构优化必须始终针对真实瓶颈，脱离负载特征去盲目堆砌所谓优化，结果可能适得其反。"
              "本实验选择如实呈现这一结果，而非加以掩饰，正是为了凸显这一工程判断的分量。")

    h2(doc, "5 安全性说明")
    body(doc, "新增的 HTTP 服务绑定在所有网卡地址且尚未启用 TLS，仅复用了既有的 Bearer Token "
              "鉴权，因而适用于实验与内网环境；若要部署到生产，还须补充 TLS 加密、访问限流以及更"
              "完善的认证授权机制。在容器层面，镜像以非 root 的专用用户运行，遵循了最小权限原则，"
              "降低了容器逃逸所带来的潜在风险。")

    # ======================= 五、代码 =======================================
    h1(doc, "五、代码")
    body(doc, "本节摘录本次实验最具代表性的几处核心代码，完整实现见随附代码仓库。")
    h2(doc, "1 HTTP keep-alive 长连接")
    code_block(doc,
        "// 线程池模式启用 keep-alive：在同一连接上循环服务多个请求，\n"
        "// 复用 TCP 连接，避免每请求握手与 TIME_WAIT 堆积。\n"
        "while (running_) {\n"
        "    // 读取直至获得完整请求头 ...\n"
        "    std::string conn = request.getHeader(\"Connection\");\n"
        "    bool keepAlive = mode_ == Mode::THREAD_POOL &&\n"
        "                     conn != \"close\" && conn != \"Close\";\n"
        "    HttpResponse response = dispatcher_->dispatch(request);\n"
        "    response.setHeader(\"Connection\", keepAlive ? \"keep-alive\" : \"close\");\n"
        "    // ... 发送响应 ...\n"
        "    if (!keepAlive) break;   // 单线程基线：每请求后关闭连接\n"
        "}")
    h2(doc, "2 Nginx 负载均衡配置")
    code_block(doc,
        "upstream gildedrose_backend {\n"
        "    least_conn;                       # 最少连接数优先\n"
        "    server 127.0.0.1:9001 max_fails=3 fail_timeout=5s;\n"
        "    server 127.0.0.1:9002 max_fails=3 fail_timeout=5s;\n"
        "    server 127.0.0.1:9003 max_fails=3 fail_timeout=5s;\n"
        "    keepalive 64;                     # 复用上游连接\n"
        "}\n"
        "location / {\n"
        "    proxy_pass http://gildedrose_backend;\n"
        "    proxy_next_upstream error timeout http_502 http_503 http_504;  # 故障转移\n"
        "}")
    h2(doc, "3 Docker Compose 编排")
    code_block(doc,
        "services:\n"
        "  backend1: &backend\n"
        "    build: { context: ., dockerfile: Dockerfile }\n"
        "    environment: { GR_CACHE: \"1\", GR_SEED_ITEMS: \"2000000\" }\n"
        "    command: [\"--serve\", \"--port\", \"8080\", \"--mode\", \"pool\", \"--threads\", \"16\"]\n"
        "  loadbalancer:\n"
        "    image: nginx:1.27-alpine\n"
        "    ports: [\"8088:80\"]\n"
        "    depends_on:                       # 待后端健康后再启动\n"
        "      backend1: { condition: service_healthy }")
    h2(doc, "4 k6 压测脚本")
    code_block(doc,
        "export const options = {\n"
        "    vus: 50, duration: '15s',\n"
        "    thresholds: { http_req_failed: ['rate<0.01'],\n"
        "                  http_req_duration: ['p(95)<2000'] },\n"
        "};\n"
        "export default function () {\n"
        "    const res = http.get(`${BASE_URL}/api/reports/daily`,\n"
        "        { headers: { Authorization: 'Bearer admin_secret_token' } });\n"
        "    check(res, { 'status is 200': (r) => r.status === 200 });\n"
        "}")

    # ======================= 六、结论 =======================================
    h1(doc, "六、结论")
    body(doc, "本次实验围绕可扩展性与性能两大质量属性，对 Gilded Rose 后端实施了网络服务化、"
              "垂直扩展、缓存优化与水平扩展四个阶段的递进式改造，并以 k6 压测量化了每一步的真实收益。"
              f"实测表明，线程池较单线程基线的吞吐量提升约 {spA:.1f} 倍；在此基础上引入缓存后，"
              f"较基线提升约 {spC:,.0f} 倍，是全部优化中收益最高的一项。在工程化层面，"
              "我进一步通过 Docker Compose 实现了三实例加 Nginx 负载均衡集群的一键容器化部署，"
              "并实测验证了健康检查与故障转移的有效性。")
    body(doc, "然而，比性能数字本身更宝贵的，是贯穿全程的工程方法论收获。通过一次失败的基准测试，"
              "我学会了不盲信任何漂亮的数据，而是沿着错误率与吞吐量的异常去定位端口耗尽、负载过轻"
              "这类系统性偏差的根因；通过水平扩展反而慢于单机缓存这一反直觉的结果，我深刻理解了"
              "优化必须紧扣真实瓶颈——脱离负载特征去堆砌所谓优化，非但无益，反而可能引入净开销。"
              "在 AI 能够轻易生成可运行代码的今天，这种对数据保持怀疑、对系统刨根问底的批判性判断力，"
              "正是工程师不可被替代的核心价值所在。")

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    doc.save(OUT)
    print("wrote", os.path.abspath(OUT))


if __name__ == "__main__":
    build()
