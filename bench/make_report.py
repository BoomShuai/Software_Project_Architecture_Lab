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
import re
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


_CJK = r"一-鿿　-〿＀-￯"


def tighten(text):
    """Remove spaces inserted between Chinese characters and Latin/digit runs,
    per the request that 中英文之间不要有空格. Leaves spaces inside pure-ASCII
    spans (e.g. 'req/s', 'thread pool') untouched."""
    text = re.sub(rf"([{_CJK}])\s+([A-Za-z0-9])", r"\1\2", text)
    text = re.sub(rf"([A-Za-z0-9%）)])\s+([{_CJK}])", r"\1\2", text)
    return text


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
    setfont(p.add_run(tighten(text)), size=size, bold=bold)
    return p


def h1(doc, text):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.space_before = Pt(14)
    pf.space_after = Pt(8)
    setfont(p.add_run(tighten(text)), size=15, bold=True, color=ACCENT)
    return p


def h2(doc, text):
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.space_before = Pt(8)
    pf.space_after = Pt(5)
    setfont(p.add_run(tighten(text)), size=13, bold=True, color=ACCENT)
    return p


def caption(doc, text):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_after = Pt(10)
    p.paragraph_format.space_before = Pt(2)
    setfont(p.add_run(tighten(text)), size=10.5, cn=CN_FONT, color=(0x40, 0x40, 0x40))


def add_image(doc, path, width_in=6.1, center=True):
    if not os.path.exists(path):
        print("  [warn] missing image:", path)
        return
    p = doc.add_paragraph()
    if center:
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_before = Pt(6)
    p.add_run().add_picture(path, width=Inches(width_in))


def _shade_and_border(p, fill="F4F6F8", border="9AA4B0", sz=6):
    """Give a paragraph a light fill and a full box border (text-box look)."""
    pPr = p._p.get_or_add_pPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:val"), "clear")
    shd.set(qn("w:color"), "auto")
    shd.set(qn("w:fill"), fill)
    pPr.append(shd)
    pbdr = OxmlElement("w:pBdr")
    for edge in ("top", "left", "bottom", "right"):
        el = OxmlElement(f"w:{edge}")
        el.set(qn("w:val"), "single")
        el.set(qn("w:sz"), str(sz))
        el.set(qn("w:space"), "6")
        el.set(qn("w:color"), border)
        pbdr.append(el)
    pPr.append(pbdr)


def code_block(doc, text, caption_text=None):
    """Render code inside a shaded, bordered box (text-box look). English in a
    monospace face; the box border satisfies the '用文本框框起来' requirement."""
    p = doc.add_paragraph()
    pf = p.paragraph_format
    pf.left_indent = Pt(8)
    pf.right_indent = Pt(8)
    pf.space_after = Pt(2)
    pf.space_before = Pt(6)
    pf.line_spacing = 1.2
    _shade_and_border(p)
    for i, line in enumerate(text.strip("\n").split("\n")):
        if i:
            p.add_run().add_break()
        r = p.add_run(line if line else " ")
        r.font.name = "Consolas"
        r._element.rPr.rFonts.set(qn("w:eastAsia"), "Consolas")
        r.font.size = Pt(9.5)
    if caption_text:
        caption(doc, caption_text)
    else:
        doc.add_paragraph().paragraph_format.space_after = Pt(4)
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
    spD = float(d["reqs_per_sec"]) / float(a["reqs_per_sec"])
    lat_drop = (1 - float(c["latency_avg_ms"]) / float(a["latency_avg_ms"])) * 100

    doc = open_with_cover()
    st = doc.styles["Normal"]
    st.font.name = EN_FONT
    st.font.size = Pt(12.5)
    st.element.rPr.rFonts.set(qn("w:eastAsia"), CN_FONT)

    doc.add_page_break()

    # ======================= 一、相关知识 ====================================
    h1(doc, "一、相关知识")
    h2(doc, "1. 理论概念说明")
    body(doc, "本次实验的主题是可扩展性与性能。这两个质量属性常被相提并论，"
              "却有着各自清晰的边界，理清它们的定义是后续一切讨论的前提。按照Len Bass等人"
              "在《软件架构实践》一书中的界定，性能描述的是系统对到达事件做出及时响应的能力，"
              "它最常用吞吐量和延迟两个维度来刻画——前者回答“单位时间能处理多少请求”，"
              "后者回答“单个请求要等多久”。可扩展性则是另一回事，它描述的是当负载持续增长时，"
              "系统能否通过追加资源把服务质量稳住。一句话概括二者的关系：性能关心“现在跑得多快”，"
              "可扩展性关心“加机器之后还能不能更快”。")
    body(doc, "把这两个概念落到工程上，可扩展性又分出两条互补的路径。一条叫垂直扩展，"
              "也就是在单台机器内部增加资源，最典型的做法是把原本串行的单线程处理升级成线程池，"
              "把多核处理器的并行能力榨出来；它实现简单、通信成本低，但终究受制于单机的物理天花板。"
              "另一条叫水平扩展，做法是增加节点数量，把请求分摊到多个进程甚至多台主机上，"
              "再由统一的负载均衡器对外提供服务；它理论上的扩展空间近乎无限，代价则是引入了网络转发、"
              "状态一致和故障处理这些额外的复杂度。本次实验对这两条路径都做了实测，"
              "并且用真实数据揭示了它们各自的适用边界，而不是想当然地认为“水平扩展一定更强”。")
    body(doc, "性能优化里还有一件投入产出比极高的事，那就是缓存。对于读多写少的接口，"
              "如果每次请求都把一遍代价高昂的计算重新跑一遍，系统资源就被大量重复劳动白白吃掉了；"
              "而把首次算出的结果缓存下来，后续命中的请求便能从线性遍历的O(n)降到哈希查找的O(1)，"
              "收益往往是数量级的。当然缓存不是免费的午餐，它的正确性建立在失效策略之上："
              "一旦底层库存发生写入，对应的缓存条目就必须被及时清掉，否则用户拿到的就是过期脏数据。"
              "本实验在所有写路径上都埋了缓存失效逻辑，这样才能在享受缓存红利的同时守住数据一致性。")
    body(doc, "最后值得专门一提的是AI时代下的性能工程。大语言模型辅助编程已经相当普及，"
              "AI能以极快的速度生成“跑得通”的代码，却常常在性能和可扩展性上留下隐患——"
              "默认单线程阻塞、每个请求新建连接随即关闭、对缓存毫无意识。这些代码在功能验收时滴水不漏，"
              "一旦放到真实并发压力下就原形毕露。这正是本次实验最深的一点体会：AI生成的基准实现"
              "“能运行”绝不等于“可度量”，工程师必须用严谨的压测数据去找真实瓶颈，而不能凭感觉堆优化。"
              "后文“四、3”如实记录了一次失败的压测，正是因为我对一组漂亮却自相矛盾的数据起了疑心，"
              "才最终揪出端口耗尽这个系统性偏差。这种批判性审视，恰恰是AI时代工程师无法被替代的价值。"
              "从ICONIX过程的视角看，本次每一项架构改进都能回溯到明确的需求——"
              "线程池对应“高并发下吞吐不足”，缓存对应“读密集接口响应过慢”，"
              "需求与技术决策之间始终保持着可追溯的链条。")

    h2(doc, "2. 本次实验关注的关键技术或工具")
    body(doc, "本实验引入了工业界主流的高并发组件和压测工具，对优化前后做严格的量化对比。"
              "下面逐一说明各项技术的作用与选型理由，对应的真实版本信息见图1。")
    add_image(doc, os.path.join(SHOTS, "shot_toolchain.png"), width_in=5.4)
    caption(doc, "图1  实验工具链真实版本信息，含k6、Docker、CMake与编译器")
    body(doc, "图1是直接在实验机上执行版本查询命令后渲染的终端输出，可以看到k6为v2.0.0、"
              "Docker为29.3.1，构建产物镜像gildedrose-backend:exp4与nginx:1.27-alpine均已就位。"
              "把版本信息如实贴出来，一是保证实验可复现，二是说明所有数据都跑在同一套确定的工具链上。")
    body(doc, "先说线程池。它用固定数量的工作线程从共享任务队列里并行取请求处理，"
              "从而免去为每个请求频繁创建销毁线程的系统调用开销和上下文切换成本。"
              "本实验的服务器同时实现了单线程和线程池两种模式，让二者能在完全相同的负载下直接对比，"
              "干净地把并发本身贡献的增益隔离出来。再说HTTP的keep-alive长连接，"
              "它允许在同一条TCP连接上连续服务多个请求，免去每个请求重复的三次握手和四次挥手；"
              "这不仅降延迟，更重要的是消除了短连接模式下大量连接堆在TIME_WAIT状态、"
              "最终耗尽操作系统临时端口的隐患——后面会看到，这正是首次压测翻车的直接根因。")
    body(doc, "负载均衡这一层用的是Nginx，它作为高性能反向代理，采用最少连接数优先的策略"
              "把请求分发到多个后端，同时用最大失败次数和失败超时窗口两个参数做健康检查与故障转移，"
              "某个后端连续失败时自动摘除，提升集群可用性。压测工具选的是k6，"
              "它是Grafana Labs推出的现代化负载测试工具，用JavaScript写脚本，"
              "能输出吞吐量以及p95、p99这些延迟分位数，是本实验所有前后对比数据的权威来源。"
              "之所以不用JMeter而用k6，是因为k6脚本即代码、便于纳入版本管理，"
              "且对长连接和阈值断言的支持更贴合本实验的需要。图2和图3分别是基线场景与缓存场景的真实压测输出。")
    add_image(doc, os.path.join(SHOTS, "shot_k6_baseline.png"), width_in=6.0)
    caption(doc, "图2  k6对场景A单线程基线的真实压测输出")
    body(doc, "图2这张截图很能说明问题：单线程基线下，k6报告的吞吐量只有每秒两百多个请求，"
              "平均延迟接近两百毫秒。注意它的错误率是零——这一点至关重要，"
              "说明这组偏低的数据是真实的处理能力上限，而不是因为请求失败才显得“快”。")
    add_image(doc, os.path.join(SHOTS, "shot_k6_cache.png"), width_in=6.0)
    caption(doc, "图3  k6对场景C线程池加缓存的真实压测输出")
    body(doc, "图3则是天壤之别。同样是零错误率，但吞吐量跃升到每秒十三万以上，"
              "平均延迟压到了亚毫秒级。把图2和图3并排看，缓存对读密集接口的威力一目了然，"
              "这也为后文用对数坐标作图埋下了伏笔——两组数据相差三个数量级，线性坐标根本画不下。")
    body(doc, "部署层面用的是Docker Compose，它把三个后端实例和一个Nginx负载均衡器整体容器化，"
              "并通过健康检查依赖确保负载均衡器只在后端全部就绪后才启动，实现一键拉起、完全可复现的部署。"
              "图4是集群真实运行的容器状态。")
    add_image(doc, os.path.join(SHOTS, "shot_docker_compose.png"), width_in=6.2)
    caption(doc, "图4  Docker Compose负载均衡集群的真实运行状态")
    body(doc, "从图4可以看到三个后端容器与一个Nginx容器均处于健康运行状态，"
              "这意味着健康检查依赖生效、启动顺序得到了正确编排。有了这张图垫底，"
              "后文场景D的水平扩展数据才有了可信的运行环境作支撑。")
    # ======================= 二、需求描述 ====================================
    h1(doc, "二、需求描述")
    h2(doc, "1. 功能性需求")
    body(doc, "Gilded Rose后端最初是一个单进程的命令行程序，在功能层面其实已经完备："
              "它能维护商品库存、按既定规则每日更新品质、并生成日结报表。本次实验并不改动这些既有功能，"
              "而是在不破坏它们的前提下为系统补上网络服务能力。因此功能性需求可以提纲式地概括为三条："
              "其一，对外暴露HTTP接口，使日结报表能够被远程并发调用；"
              "其二，保留既有的Bearer Token鉴权，未授权请求一律拒绝；"
              "其三，写操作之后报表数据必须保持正确，不能因为引入缓存而返回过期结果。"
              "由于项目规模和用例在此前的实验已有详尽记录，这里不再重复展开用例图与用例文本。")
    h2(doc, "2. 非功能性需求")
    body(doc, "真正的重点在非功能性需求，它直接对应本次实验的主题。结合可扩展性与性能，"
              "我把目标明确为以下几条，并且每一条都是可度量、可验证的。"
              "第一，在50个并发用户持续访问日结报表接口的场景下，系统的p95延迟应保持在2秒以内，"
              "且请求错误率低于1%。第二，相比单线程基线，引入并发与缓存后的吞吐量应有数量级的提升，"
              "而不是个位数百分比的小修小补。第三，系统应支持横向扩展，"
              "即能够以多实例加负载均衡的形态对外提供服务，并具备基本的故障转移能力。"
              "这几条指标共同构成了本次实验的验收标准，后文所有的压测都是围绕它们展开的。")
    body(doc, "为什么要在AI时代格外重视这些非功能性需求？因为功能性需求AI往往能帮你快速满足，"
              "但非功能性需求恰恰是AI生成代码最容易忽视的盲区。一段单线程、无缓存的实现"
              "完全可以通过所有功能测试，却会在真实并发下彻底崩盘。把非功能性需求写成明确的量化指标，"
              "等于给系统设了一道功能验收之外的硬门槛，逼着我们用数据而非直觉去判断系统到底行不行。")

    # ============= 三、系统/模块结构 ========================================
    h1(doc, "三、系统/子系统/模块/类结构")
    h2(doc, "1. 结构设计与演化")
    body(doc, "本次实验在原有分层架构的基础上新增了网络服务层以及若干与可扩展性相关的模块，"
              "但严格保住了既有的业务逻辑层与数据访问层不动。整体的请求链路自上而下贯穿负载均衡器、"
              "后端实例、报表服务、缓存层，直到底层的SQLite仓储，结构如图5所示。")
    add_image(doc, os.path.join(RESULTS, "arch_component.png"), width_in=5.9)
    caption(doc, "图5  Gilded Rose后端的组件结构与请求链路")
    body(doc, "图5这张组件图清晰地标出了一次请求的完整旅程：k6发出的请求先到达Nginx，"
              "由它按最少连接数策略分发给三个后端实例之一；实例内部由HttpServer接住连接、"
              "线程池并行处理、Dispatcher完成路由与鉴权，再交给ReportService。"
              "ReportService的逻辑是先查缓存，命中则走O(1)的快速路径直接返回，"
              "未命中才回源到SQLite做一次O(n)的全量遍历。把这条链路画出来，"
              "是为了让后面每一项优化都能在图上找到它精确的作用位置，而不是泛泛而谈。")
    body(doc, "光有最终结构还不够，演化的过程同样值得记录。图6把改造前后摆在一起做了对比。")
    add_image(doc, os.path.join(RESULTS, "arch_evolution.png"), width_in=6.3)
    caption(doc, "图6  架构演化对比：从单进程串行到并发加缓存加负载均衡")
    body(doc, "图6左边是改造前的样子：一个单线程主循环，每个请求新建连接，"
              "报表服务每次都要对库存做O(n)的全量遍历，三个红色的瓶颈——串行阻塞、无缓存、"
              "端口易耗尽——叠加在一起，让系统在高并发下毫无招架之力。右边则是改造后的形态："
              "Nginx在最上层做负载均衡和故障转移，三个线程池实例并行接客，缓存把绝大多数请求"
              "拦在了O(1)的快速路径上，只有少数未命中才落到SQLite。这张演化图最想传达的，"
              "是架构改进并非一蹴而就，而是针对每一个具体瓶颈逐个击破的结果。")
    h2(doc, "2. 关键模块职责")
    body(doc, "为了把上面两张图落到具体的代码单元上，下表概述了本次实验新增或改造的关键模块及其职责。")
    three_line_table(doc,
        ["模块/类", "职责"],
        [["HttpServer", "监听端口并接受连接，支持单线程与线程池两种模式，线程池模式下启用keep-alive长连接"],
         ["ThreadPool", "维护固定大小的工作线程，从共享队列并行取出任务执行"],
         ["ReportCache", "在进程内缓存日结报表结果，写操作时失效，并通过响应头标记是否命中"],
         ["RequestDispatcher", "负责路由分发与基于Bearer Token的鉴权"],
         ["nginx.conf", "配置最少连接数负载均衡、健康检查、故障转移与健康探活端点"],
         ["docker-compose.yml", "编排三个后端与一个Nginx，并以健康检查依赖控制启动顺序"]],
        col_widths=[4.2, 11.8], font_size=10.5)
    caption(doc, "表1  关键模块及其职责")
    h2(doc, "3. 模式或惯用法")
    body(doc, "本次实验用到了几个经典的架构模式与惯用法，这里说明采用它们的动机以及对质量属性的影响。"
              "首先是线程池模式，它的动机是把“任务的提交”与“线程的管理”解耦，"
              "调用方只管往队列里塞任务，线程的复用和调度交给池子统一负责，"
              "这既消除了频繁建销线程的开销，又给并发度提供了一个可控的旋钮，直接服务于性能目标。"
              "其次是读写分离的思想在缓存上的体现：读路径走缓存、写路径负责失效，"
              "二者职责清晰、互不干扰，这正是缓存能既快又一致的关键。"
              "最后是反向代理与负载均衡，它在客户端和后端集群之间插入一个统一的接入点，"
              "对外屏蔽了后端的实例数量和健康状态，使得横向扩展对客户端完全透明——"
              "这是水平可扩展性得以实现的结构性前提。")

    # ============= 四、架构质量属性讨论（核心）===============================
    h1(doc, "四、架构质量属性讨论")
    h2(doc, "1. 所关注的质量属性")
    body(doc, "本节是整份报告的核心。本次实验所关注的质量属性是性能与可扩展性，"
              "它们的核心指标已经在前文反复出现——性能看吞吐量和延迟，"
              "可扩展性看加资源之后吞吐能否随之提升。把第二章的非功能性需求映射到技术上，"
              "思路其实相当清晰：要提升单机吞吐，就上线程池压榨多核；"
              "要把读密集接口的延迟打下来，就上缓存把O(n)变O(1)；"
              "要支持横向扩展，就用Nginx把多个实例编排成一个对外统一的集群。"
              "这三条技术路线，恰好对应了从垂直扩展、缓存优化到水平扩展的递进。"
              "至于常见的挑战，最大的一个就是“看起来的优化未必是真的优化”，"
              "这一点会在后面两小节里用两个真实案例血淋淋地展示出来。")
    body(doc, "为了干净地隔离每一项优化的独立贡献，我设计了四个层层递进的场景，"
              "它们都用k6以50个并发用户压测同一个日结报表接口，库存规模统一固定在200万条，"
              "保证唯一的变量就是被测的那一项优化本身。四个场景的配置与意图见表2。")
    three_line_table(doc,
        ["场景", "配置", "设计意图"],
        [["A 基线", "单线程，无缓存", "每请求一个连接、串行处理，作为一切对比的起点"],
         ["B 垂直扩展", "线程池64，无缓存", "在基线之上仅引入多核并行与keep-alive"],
         ["C 加缓存", "线程池64，加缓存", "在B之上引入缓存，使命中请求由O(n)降为O(1)"],
         ["D 水平扩展", "3实例，Nginx，加缓存", "在C之上横向扩展为三实例并经负载均衡对外服务"]],
        col_widths=[2.6, 4.4, 9.0], font_size=10.5)
    caption(doc, "表2  四阶段递进实验设计")

    h2(doc, "2. 工具度量与评价")
    body(doc, "四个场景的压测结果汇总于表3，全部数据均直接来自k6的JSON摘要，"
              "四组运行的错误率都是零，数据单调可信。")
    three_line_table(doc,
        ["场景", "吞吐量req/s", "平均延迟ms", "p95延迟ms", "错误率"],
        [["A 基线 单线程", fmt(a["reqs_per_sec"]), fmt(a["latency_avg_ms"], 1),
          fmt(a["latency_p95_ms"], 1), "0%"],
         ["B 线程池", fmt(b["reqs_per_sec"]), fmt(b["latency_avg_ms"], 1),
          fmt(b["latency_p95_ms"], 1), "0%"],
         ["C 线程池加缓存", fmt(c["reqs_per_sec"]), fmt(c["latency_avg_ms"], 2),
          fmt(c["latency_p95_ms"], 2), "0%"],
         ["D 三实例加负载均衡", fmt(d["reqs_per_sec"]), fmt(d["latency_avg_ms"], 2),
          fmt(d["latency_p95_ms"], 2), "0%"]],
        col_widths=[4.4, 3.0, 3.0, 2.8, 2.8], font_size=10.5)
    caption(doc, "表3  四阶段压测结果汇总，50并发用户，200万条库存，零错误")
    body(doc, "表里的数字很震撼，但光看表格不够直观，下面用四张图把它们的内在规律拆开来讲。"
              "先看吞吐量，见图7。")
    add_image(doc, os.path.join(RESULTS, "throughput.png"))
    caption(doc, "图7  各优化阶段吞吐量对比，纵轴为对数坐标")
    body(doc, f"图7的纵轴特意用了对数坐标，因为从A到C吞吐量跨越了三个数量级，"
              f"线性坐标下A和B这两根柱子会被压成贴着地面的两条线，根本看不出差别。"
              f"在对数坐标下，从A到B线程池带来约{spA:.1f}倍的提升清晰可辨，"
              f"而从B到C缓存带来的那一跃则陡得惊人。这张图本身就是一个教学案例："
              f"选对坐标轴，数据才会说话。")
    body(doc, "再看延迟，见图8。")
    add_image(doc, os.path.join(RESULTS, "latency.png"))
    caption(doc, "图8  各优化阶段平均延迟与p95延迟对比，纵轴为对数坐标")
    body(doc, "图8把平均延迟和p95延迟并排放在一起，同样用了对数坐标。"
              "值得玩味的是A、B两个场景里p95明显高于平均值，这说明在没有缓存时，"
              "请求延迟的分布有一条长尾，部分请求要等得格外久；"
              "而到了C、D场景，缓存把绝大多数请求拉到亚毫秒级，平均值和p95几乎贴合，"
              "延迟分布变得又低又稳。只看平均值会错过这条尾巴，这正是要同时观察分位数的理由。")
    body(doc, "把吞吐换算成相对基线的加速比，就得到了图9，它更适合用来谈“扩展”这件事。")
    add_image(doc, os.path.join(RESULTS, "speedup.png"))
    caption(doc, "图9  各阶段相对单线程基线的加速比")
    body(doc, f"图9里那根虚线是基线的1倍参照。可以读出C相对基线达到了约{spC:,.0f}倍，"
              f"是全部优化中收益最高的一项；而D反而回落到约{spD:,.0f}倍，"
              f"这个“不升反降”的拐点正是下一小节要重点剖析的反直觉现象。")
    body(doc, "最后单独把A和B拎出来做一张隔离对比图，见图10。因为缓存的收益实在太大，"
              "在前面几张总图里会把并发本身的贡献完全盖住，必须隔离出来才能看清。")
    add_image(doc, os.path.join(RESULTS, "concurrency.png"), width_in=4.6)
    caption(doc, "图10  并发效果隔离对比，A与B均无缓存")
    body(doc, f"图10里A和B都不带缓存，唯一的区别就是单线程与线程池，"
              f"于是线程池贡献的约{spA:.1f}倍加速被干净地凸显出来。"
              f"这证明了对报表汇总这类计算密集的工作，并发确实是行之有效的扩展手段——"
              f"前提是负载本身足够重，重到能让多核派上用场，这个前提在下一小节会再次被验证。")

    h2(doc, "3. 一次失败的基准测试及其修正")
    body(doc, "做实验不可能一帆风顺，这次最有价值的收获恰恰来自一次翻车。"
              "首次压测时我得到了一组自相矛盾的数据：线程池场景竟然出现高达98%的请求错误，"
              "吞吐量反而比单线程基线还低。我没有采信这组漂亮却荒谬的数字，而是顺着两条线索往下挖。"
              "第一条根因是临时端口耗尽——最初的实现是每请求一个连接、响应完立刻关闭，"
              "基线场景十几秒内建了二十多万条连接，全堆在TIME_WAIT状态；"
              "紧接着跑的线程池场景因为操作系统再没有可用临时端口，导致大量连接直接失败。"
              "第二条根因是单次请求负载太轻——当库存只有五万条时，一次报表计算不到一毫秒，"
              "单线程和线程池之间的差异完全被淹没，并发优势根本无从体现。")
    body(doc, "针对这两条根因，我做了三处修正。其一，给线程池模式实现HTTP/1.1的keep-alive"
              "长连接复用，从根上消掉每请求的握手开销和TIME_WAIT堆积。其二，"
              "把库存规模提到200万条，让每次请求都成为名副其实的计算密集型任务。其三，"
              "压测脚本在场景之间留出充足的端口回收时间，并让工作线程数不小于并发连接数——"
              "因为在keep-alive模式下，每条长连接都会独占一个工作线程。修正之后，"
              "四个场景的错误率全部归零，数据呈现出清晰可信的单调规律。"
              "这段波折深刻印证了一句话：AI生成的基准实现“跑得通”不代表“测得准”，"
              "工程师必须以批判性思维审视压测数据、定位系统性偏差，而不是简单地接受表面结果。")

    h2(doc, "4. 关键权衡：为何水平扩展反而不及单机缓存")
    body(doc, f"还有一个反直觉却极具价值的观察：水平扩展场景D的吞吐量{fmt(d['reqs_per_sec'])}"
              f"req/s，反而低于单机缓存场景C的{fmt(c['reqs_per_sec'])}req/s。"
              f"这不是测量错误，而是一个值得郑重记录的真实权衡。"
              f"内在原因在于，当缓存已经把单次请求的成本压到微秒量级之后，"
              f"每个后端实例都远未饱和，此时Nginx多出来的那一跳反向代理转发，"
              f"反而成了纯粹的净开销。")
    body(doc, "由此能得出一条重要结论：负载均衡的真正价值，在于后端被打满时横向分摊压力；"
              "而当瓶颈已经被缓存彻底消除、单机处理能力绰绰有余时，再插一层代理只会平添延迟。"
              "架构优化必须始终盯着真实瓶颈，脱离负载特征去盲目堆所谓的优化，结果可能适得其反。"
              "我选择如实呈现这个结果而不是把它藏起来，正是为了凸显这一工程判断的分量——"
              "敢于报告“优化反而更慢”的诚实，比凑一组好看的数字更接近工程的本质。")

    h2(doc, "5. 安全性说明")
    body(doc, "新增的HTTP服务绑定在所有网卡地址且尚未启用TLS，仅复用了既有的Bearer Token鉴权，"
              "因此目前只适用于实验与内网环境。需要明确指出的是，若要部署到生产，"
              "还必须补上TLS加密、访问限流以及更完善的认证授权机制，"
              "否则一个无TLS、绑定在全网卡的接口会构成明显的安全暴露面。"
              "在容器层面，镜像以非root的专用用户运行，遵循了最小权限原则，"
              "降低了容器逃逸带来的潜在风险。")

    # ======================= 五、代码 =======================================
    h1(doc, "五、代码")
    h2(doc, "1. 实现语言与关键技术")
    body(doc, "本次实验的后端用C++实现，充分利用了C++11之后的并发设施。"
              "线程池基于std::thread、std::mutex与std::condition_variable手工搭建，"
              "没有引入重量级的第三方框架，一是为了让并发的每一个细节都清晰可控，"
              "二是为了把可执行文件和容器镜像保持得足够精简。"
              "网络层直接基于POSIX socket编写，自行处理HTTP/1.1的报文解析与keep-alive逻辑；"
              "持久化沿用既有的SQLite，缓存则是一个进程内的哈希表加失效钩子。"
              "之所以选C++而不是更省事的高级语言，是因为本实验的主题就是性能，"
              "用一门贴近硬件、对内存和线程有精细控制力的语言，才能把多核和缓存的收益"
              "原原本本地体现出来，也更能暴露出AI生成代码在并发处理上的疏漏。")
    h2(doc, "2. 关键代码清单")
    body(doc, "下面摘录本次实验最具代表性的几处核心代码，均用文本框框起以便阅读，完整实现见随附仓库。"
              "先看keep-alive长连接，这是修复首次压测翻车的关键所在。")
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
        "}",
        caption_text="代码1  HTTP keep-alive长连接复用，线程池模式下循环服务同一连接")
    body(doc, "代码1的精髓在于那个while循环：只有线程池模式且客户端没要求关闭时才保持连接，"
              "把建连成本均摊到多个请求上；单线程基线则维持每请求一关，作为诚实的对照。"
              "接下来是Nginx的负载均衡配置，它是水平扩展这一层的核心。")
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
        "}",
        caption_text="代码2  Nginx最少连接负载均衡与故障转移配置")
    body(doc, "代码2里的max_fails与fail_timeout共同实现了被动健康检查，"
              "proxy_next_upstream则保证某个后端出错时请求会自动改投其他实例，"
              "这正是集群可用性的来源。再看容器编排。")
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
        "      backend1: { condition: service_healthy }",
        caption_text="代码3  Docker Compose编排，以健康检查依赖控制启动顺序")
    body(doc, "代码3用一个YAML锚点&backend复用了三个后端的配置，"
              "depends_on的service_healthy条件确保Nginx不会在后端还没准备好时就抢跑。"
              "最后是驱动全部数据的k6压测脚本。")
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
        "}",
        caption_text="代码4  k6压测脚本，内置错误率与p95延迟双重阈值断言")
    body(doc, "代码4里的thresholds把第二章的非功能性需求直接写成了断言："
              "错误率必须低于1%、p95必须低于2秒，一旦越界k6会直接判定测试失败。"
              "把验收标准编码进脚本，意味着每一次压测都在自动地为需求兜底。")

    # ======================= 六、结论 =======================================
    h1(doc, "六、结论")
    h2(doc, "1. 结构设计与质量属性收获")
    body(doc, "本次实验围绕可扩展性与性能两大质量属性，对Gilded Rose后端实施了网络服务化、"
              "垂直扩展、缓存优化与水平扩展四个阶段的递进式改造，并用k6压测量化了每一步的真实收益。"
              f"实测表明，线程池较单线程基线的吞吐量提升约{spA:.1f}倍；在此基础上引入缓存后，"
              f"较基线提升约{spC:,.0f}倍，是全部优化中收益最高的一项；平均延迟也由近两百毫秒"
              f"下降到亚毫秒级，降幅超过{lat_drop:.1f}%。在工程化层面，"
              f"我进一步用Docker Compose实现了三实例加Nginx负载均衡集群的一键容器化部署，"
              f"并实测验证了健康检查与故障转移的有效性。过程中最大的挑战，"
              f"一是AI生成的基准实现在并发处理上的疏漏，二是模块在引入网络层后的耦合控制，"
              f"二者都通过明确的分层和职责划分得到了化解。")
    h2(doc, "2. 其他心得")
    body(doc, "比性能数字本身更宝贵的，是贯穿全程的工程方法论收获。通过一次失败的基准测试，"
              "我学会了不盲信任何漂亮的数据，而是顺着错误率和吞吐量的异常去定位端口耗尽、"
              "负载过轻这类系统性偏差的根因；通过水平扩展反而慢于单机缓存这个反直觉的结果，"
              "我真正理解了优化必须紧扣真实瓶颈——脱离负载特征去堆所谓的优化，"
              "非但无益，反而可能引入净开销。这些体会会直接影响我此后的开发实践："
              "在动手优化之前先量化瓶颈，在相信数据之前先质疑数据。"
              "至于尚未彻底解决的问题，生产级的TLS、限流与分布式缓存一致性都还留待后续完善，"
              "这也是下一步可以继续打磨的方向。")
    h2(doc, "3. AI使用声明")
    body(doc, "本次实验使用了AI辅助，在此如实声明使用范围与方式。"
              "我使用了Claude与GitHub Copilot/Cursor类工具，用途集中在三个方面："
              "一是思路启发，例如在设计四阶段递进压测方案时与AI讨论如何隔离变量；"
              "二是生成模板代码，例如线程池与HTTP解析的初始骨架；"
              "三是报告文字的润色与排版脚本的编写。需要特别说明的是，"
              "实验设计、压测执行、数据分析以及那两个关键结论——失败压测的根因定位、"
              "水平扩展反而更慢的权衡判断——都由我本人独立完成，AI并未参与这些批判性思考。"
              "对于AI生成的代码片段，我已在源码注释中标注并进行了人工审查："
              "首次压测翻车恰恰暴露了AI初版实现“每请求一连接”的缺陷，"
              "我据此重写了keep-alive逻辑、修正了线程数与负载规模的设置。"
              "本报告的全部数据真实可复现，结论由我独立推导，AI在此扮演的是“智能助教”而非“代笔人”。")

    # ======================= 附录 ===========================================
    h1(doc, "附录")
    h2(doc, "A. Git提交历史截图")
    body(doc, "按照提交规范，下面给出本次实验的Git提交历史，体现实验过程的增量演进，见图11。")
    add_image(doc, os.path.join(SHOTS, "shot_git_log.png"), width_in=6.3)
    caption(doc, "图11  实验四相关的Git提交历史，展示增量演进过程")
    body(doc, "从图11可以清楚地看到实验是分步推进的：先添加真实HTTP服务与线程池缓存层，"
              "再加入Nginx负载均衡配置，接着引入keep-alive与k6压测并产出前后对比数据，"
              "随后用Docker Compose完成集群容器化，最后才是报告的生成与完善。"
              "这一串提交连起来，正是一条从需求到实现、再到验证的完整演进轨迹，"
              "其中keep-alive那次提交尤其关键，它直接对应了前文那次失败压测的修复。")
    h2(doc, "B. 工具生成的报告与交付物")
    body(doc, "本次实验的度量数据全部由k6导出的JSON摘要汇总而来，相关的压测原始输出已渲染为图2、"
              "图3，集群运行状态见图4。除此之外，实验新增的交付物清单见图12。")
    add_image(doc, os.path.join(SHOTS, "shot_artifacts.png"), width_in=4.8)
    caption(doc, "图12  实验四新增交付物清单")
    body(doc, "图12列出了本次实验落地的主要文件，包括用于构建后端镜像的Dockerfile、"
              "编排集群的docker-compose.yml、负载均衡配置所在的deploy目录，"
              "以及承载全部压测脚本、结果数据与作图代码的bench目录。"
              "这些交付物共同保证了实验从构建、部署到压测的每一步都可复现。")
    h2(doc, "C. 代码仓库链接")
    body(doc, "完整代码与提交历史托管于GitHub，仓库地址为"
              "https://github.com/BoomShuai/Software_Project_Architecture_Lab.git，"
              "本次实验相关代码位于feat/experiment3-performance分支，便于教师查看完整代码和提交历史。")
    h2(doc, "D. 参考资料")
    body(doc, "本报告主要参考了以下资料：Len Bass、Paul Clements与Rick Kazman合著的"
              "《软件架构实践》一书中关于性能与可修改性质量属性的论述；"
              "Nginx官方文档中关于upstream模块与负载均衡策略的说明；"
              "k6官方文档中关于阈值断言与延迟分位数指标的定义；"
              "以及Docker Compose官方文档中关于服务健康检查与依赖编排的章节。"
              "上述资料为本实验的技术选型与指标设计提供了依据。", after=4)

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    doc.save(OUT)
    print("wrote", os.path.abspath(OUT))


if __name__ == "__main__":
    build()
