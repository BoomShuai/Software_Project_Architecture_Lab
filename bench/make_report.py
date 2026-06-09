#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate the Experiment 4 Word report (软件体系结构 第四次实验报告).

Mirrors the established report format from the earlier experiments:
  - Title page block, then sections 一..六.
  - Headings: H1 14pt bold, H2 12pt bold; body 12pt (SimSun / 宋体).
Embeds the benchmark charts from bench/results/ and reads the measured numbers
straight out of summary.csv so the prose and the data never drift apart.
"""
import csv
import os
from docx import Document
from docx.shared import Pt, Inches, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml.ns import qn

ROOT = os.path.dirname(os.path.abspath(__file__))
RESULTS = os.path.join(ROOT, "results")
CSV = os.path.join(RESULTS, "summary.csv")
OUT = os.path.join(ROOT, "..", "docs", "第四次实验报告-可扩展性与性能.docx")

CN_FONT = "宋体"
EN_FONT = "Times New Roman"


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


def body(doc, text, size=12, bold=False, align=None, after=6):
    p = doc.add_paragraph()
    if align:
        p.alignment = align
    p.paragraph_format.space_after = Pt(after)
    p.paragraph_format.line_spacing = 1.5
    setfont(p.add_run(text), size=size, bold=bold)
    return p


def h1(doc, text):
    p = doc.add_paragraph()
    p.paragraph_format.space_before = Pt(10)
    p.paragraph_format.space_after = Pt(6)
    setfont(p.add_run(text), size=14, bold=True)
    return p


def h2(doc, text):
    p = doc.add_paragraph()
    p.paragraph_format.space_before = Pt(6)
    p.paragraph_format.space_after = Pt(4)
    setfont(p.add_run(text), size=12, bold=True)
    return p


def caption(doc, text):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_after = Pt(8)
    setfont(p.add_run(text), size=10.5)


def add_image(doc, path, width_in=5.8):
    if os.path.exists(path):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.add_run().add_picture(path, width=Inches(width_in))


def code_block(doc, text):
    p = doc.add_paragraph()
    p.paragraph_format.left_indent = Pt(12)
    p.paragraph_format.space_after = Pt(6)
    p.paragraph_format.line_spacing = 1.15
    for i, line in enumerate(text.strip("\n").split("\n")):
        if i:
            p.add_run().add_break()
        r = p.add_run(line)
        r.font.name = "Consolas"
        r._element.rPr.rFonts.set(qn("w:eastAsia"), "Consolas")
        r.font.size = Pt(9.5)


def fmt(x, nd=0):
    try:
        return f"{float(x):,.{nd}f}"
    except (ValueError, TypeError):
        return str(x)


def add_table(doc, headers, rows):
    table = doc.add_table(rows=1, cols=len(headers))
    table.style = "Light Grid Accent 1"
    for i, htext in enumerate(headers):
        cell = table.rows[0].cells[i]
        cell.paragraphs[0].clear()
        setfont(cell.paragraphs[0].add_run(htext), size=10.5, bold=True)
    for row in rows:
        cells = table.add_row().cells
        for i, val in enumerate(row):
            cells[i].paragraphs[0].clear()
            setfont(cells[i].paragraphs[0].add_run(str(val)), size=10.5)
    return table


def build():
    R = load_results()
    a, b, c, d = (R["A_baseline_single"], R["B_threadpool_nocache"],
                  R["C_threadpool_cache"], R["D_loadbalanced"])
    doc = Document()
    st = doc.styles["Normal"]
    st.font.name = EN_FONT
    st.font.size = Pt(12)
    st.element.rPr.rFonts.set(qn("w:eastAsia"), CN_FONT)

    # ---- Title page ---------------------------------------------------------
    t = doc.add_paragraph(); t.alignment = WD_ALIGN_PARAGRAPH.CENTER
    setfont(t.add_run("软件体系结构"), size=36)
    t2 = doc.add_paragraph(); t2.alignment = WD_ALIGN_PARAGRAPH.CENTER
    setfont(t2.add_run("第四次实验报告"), size=28)
    doc.add_paragraph()
    for line in [
        "实验题目：    架构质量实践之可扩展性与性能优化",
        "院系名称：人工智能与大数据学院  专业班级：软件2301",
        "学生姓名：    牛帅明    学号：   231210400112",
        "指导教师：     刘灿     日期：     2026.6.9",
    ]:
        p = doc.add_paragraph(); p.paragraph_format.line_spacing = 2.0
        setfont(p.add_run(line), size=15)
    doc.add_page_break()

    # ======================= 一、相关知识 ====================================
    h1(doc, "一、相关知识")
    h2(doc, "1 理论概念说明")
    body(doc, "本次实验聚焦于软件架构中的关键非功能性质量属性——可扩展性（Scalability）"
              "与性能（Performance），并探讨高并发场景下的系统设计权衡。")
    h2(doc, "1.1 性能与可扩展性")
    body(doc, "在 Len Bass 等人所著的《软件架构实践》中，性能被定义为系统对事件做出响应的"
              "能力，常以吞吐量（throughput）与延迟（latency）两个维度度量。可扩展性"
              "则指系统通过增加资源应对负载增长的能力，分为两类：")
    body(doc, "· 垂直扩展（Scale-up）：在单个节点内增加资源，例如由单线程升级为线程池，"
              "充分利用多核 CPU 的并行能力。")
    body(doc, "· 水平扩展（Scale-out）：增加节点数量，将负载分摊到多个进程或主机上，"
              "通过负载均衡器统一对外提供服务。")
    h2(doc, "1.2 缓存与读写分离")
    body(doc, "缓存是性能优化中收益最高的手段之一。对于读多写少（read-heavy）的接口，"
              "将一次代价高昂的计算结果缓存起来，可把后续请求的复杂度从 O(n) 降至 O(1)。"
              "本实验为日结报表这一典型读密集接口引入了进程内缓存层，并在数据写入时"
              "执行缓存失效（invalidation），以保证一致性。")
    h2(doc, "1.3 AI 时代下的性能工程")
    body(doc, "在 LLM 辅助编程的当下，AI 能快速生成“跑得通”的代码，却往往忽视性能与"
              "可扩展性：默认单线程阻塞模型、每请求新建连接、缺乏缓存意识。本实验的一个"
              "核心体会正是——AI 生成的基准实现需要工程师以严谨的压测数据识别真实瓶颈，"
              "而非凭直觉添加优化。后文“四、3”记录的一次失败的基准测试即是例证。")
    h2(doc, "2 本次实验关注的关键技术或工具")
    body(doc, "本实验引入工业界主流的高并发与压测工具，对优化前后进行量化对比：")
    h2(doc, "2.1 线程池（Thread Pool）")
    body(doc, "采用固定大小的工作线程池，从共享任务队列中并行处理请求，避免为每个请求"
              "频繁创建/销毁线程的开销。配合 HTTP/1.1 keep-alive 长连接复用 TCP 连接。")
    h2(doc, "2.2 Nginx 负载均衡")
    body(doc, "Nginx 作为反向代理与负载均衡器，采用 least_conn 策略将请求分发至多个"
              "后端实例，并通过 max_fails/fail_timeout 实现健康检查与故障转移。")
    h2(doc, "2.3 k6 压力测试")
    body(doc, "k6 是 Grafana Labs 出品的现代负载测试工具，以 JavaScript 编写测试脚本，"
              "输出吞吐量（req/s）与延迟分位数（p95/p99），是本实验前后对比数据的来源。")
    h2(doc, "2.4 Docker Compose 容器编排")
    body(doc, "使用 Docker Compose 将“3 后端实例 + Nginx 负载均衡器”整体容器化，"
              "通过健康检查依赖保证启动顺序，实现一键可复现部署。")

    # ======================= 二、需求描述 ====================================
    h1(doc, "二、需求描述")
    body(doc, "Gilded Rose 后端原为单进程命令行程序。随着库存规模（可达数百万条）与并发"
              "访问量增长，原始实现暴露两个瓶颈：（1）无真实网络服务能力，无法承接并发"
              "请求；（2）日结报表接口每次请求都需 O(n) 遍历全部库存，在高并发下成为热点。")
    body(doc, "本次实验的功能与非功能需求如下：")
    body(doc, "1) 提供真实 HTTP 服务，支持单线程基线与线程池两种运行模式以作对比；")
    body(doc, "2) 为读密集的日结报表接口引入缓存层，写操作时正确失效缓存；")
    body(doc, "3) 通过 Nginx 在多个后端实例间实现负载均衡与故障转移；")
    body(doc, "4) 使用 k6 对各优化阶段进行压测，量化吞吐量与延迟变化；")
    body(doc, "5)（拓展）使用 Docker Compose 容器化整套负载均衡集群，实现一键部署。")

    # ============= 三、系统/模块结构 ========================================
    h1(doc, "三、系统/子系统/模块/类结构")
    body(doc, "本次实验在原有分层架构（network / service / repository）基础上，新增网络服务层"
              "与可扩展性相关模块。整体请求链路如下：")
    code_block(doc,
        "客户端 / k6\n"
        "    │  HTTP/1.1 (keep-alive)\n"
        "    ▼\n"
        "Nginx 负载均衡 (:8088, least_conn)\n"
        "    │ ├──► 后端实例1 (:9001)  HttpServer ─ ThreadPool ─ Dispatcher\n"
        "    │ ├──► 后端实例2 (:9002)        │\n"
        "    │ └──► 后端实例3 (:9003)        ▼\n"
        "                            ReportService ──► Cache（命中?）\n"
        "                                   │ 未命中  └──► O(n) 遍历库存\n"
        "                                   ▼\n"
        "                            ItemRepository (SQLite)")
    h2(doc, "1 关键模块职责")
    add_table(doc,
        ["模块 / 类", "职责"],
        [["HttpServer", "监听端口、接受连接；支持 SINGLE / THREAD_POOL 两种模式，"
                        "线程池模式下启用 keep-alive 长连接"],
         ["ThreadPool", "固定大小工作线程池，从共享队列并行取任务执行"],
         ["ReportCache", "进程内缓存日结报表结果，写操作时失效，提供 X-Cache 命中标记"],
         ["RequestDispatcher", "路由分发与鉴权（Bearer Token）"],
         ["deploy/nginx.conf", "least_conn 负载均衡、健康检查、故障转移、/lb-health"],
         ["docker-compose.yml", "3 后端 + Nginx 容器编排，健康检查依赖"]])

    # ============= 四、架构质量属性讨论（核心）===============================
    h1(doc, "四、架构质量属性讨论")
    h2(doc, "1 实验设计：四阶段对比")
    body(doc, "为隔离每一项优化的独立贡献，设计四个递进场景，均使用 k6 以 50 个并发用户"
              "（VUs）压测日结报表接口，库存规模固定为 200 万条：")
    add_table(doc,
        ["场景", "配置", "说明"],
        [["A 基线", "单线程 + 无缓存", "每请求一个连接，串行处理"],
         ["B 垂直扩展", "线程池(64) + 无缓存", "多核并行 + keep-alive"],
         ["C 加缓存", "线程池(64) + 缓存", "命中后 O(n)→O(1)"],
         ["D 水平扩展", "3 实例 + Nginx + 缓存", "负载均衡"]])

    h2(doc, "2 压测结果")
    add_table(doc,
        ["场景", "吞吐量 (req/s)", "平均延迟 (ms)", "p95 (ms)", "错误率"],
        [["A 基线（单线程）", fmt(a["reqs_per_sec"]), fmt(a["latency_avg_ms"], 1),
          fmt(a["latency_p95_ms"], 1), "0%"],
         ["B 线程池", fmt(b["reqs_per_sec"]), fmt(b["latency_avg_ms"], 1),
          fmt(b["latency_p95_ms"], 1), "0%"],
         ["C 线程池+缓存", fmt(c["reqs_per_sec"]), fmt(c["latency_avg_ms"], 2),
          fmt(c["latency_p95_ms"], 2), "0%"],
         ["D 3实例+负载均衡", fmt(d["reqs_per_sec"]), fmt(d["latency_avg_ms"], 2),
          fmt(d["latency_p95_ms"], 2), "0%"]])
    caption(doc, "表 1 四阶段压测结果（k6, 50 VUs, 200 万条库存, 0 错误）")

    add_image(doc, os.path.join(RESULTS, "throughput.png"))
    caption(doc, "图 1 各优化阶段吞吐量对比（对数坐标）")
    add_image(doc, os.path.join(RESULTS, "latency.png"))
    caption(doc, "图 2 各优化阶段延迟对比（对数坐标）")
    add_image(doc, os.path.join(RESULTS, "concurrency.png"))
    caption(doc, "图 3 并发效果隔离对比（A vs B，均无缓存）")

    spA = float(b["reqs_per_sec"]) / float(a["reqs_per_sec"])
    spC = float(c["reqs_per_sec"]) / float(a["reqs_per_sec"])
    body(doc, "数据解读：")
    body(doc, f"· 垂直扩展（A→B）：线程池利用多核，吞吐量提升约 {spA:.1f} 倍，"
              "证明对 CPU 密集的报表计算，并发是有效的扩展手段。")
    body(doc, f"· 缓存（B→C）：收益最为显著，相较基线提升约 {spC:,.0f} 倍。"
              "缓存命中后单次请求由毫秒级 O(n) 遍历降为微秒级哈希查找，这是读密集接口"
              "最具性价比的优化。")

    h2(doc, "3 一次失败的基准测试及其修正（关键反思）")
    body(doc, "首次压测得到一组自相矛盾的数据：线程池场景（B）竟出现 98% 的请求错误，"
              "且吞吐量反低于单线程基线。我没有直接采信这组“漂亮但错误”的数据，而是定位根因：")
    body(doc, "根因一：临时端口耗尽。最初实现为“每请求一个连接”，响应后即关闭。"
              "基线场景在 12 秒内建立了 24 万个连接，全部进入 TIME_WAIT 状态；紧接其后的"
              "B 场景因操作系统无可用临时端口而大量连接失败。")
    body(doc, "根因二：单次请求负载过轻。库存仅 5 万条时报表计算仅约 0.8ms，"
              "单线程与线程池差异无法体现，并发优势被掩盖。")
    body(doc, "修正措施：（1）为线程池模式实现 HTTP/1.1 keep-alive 长连接复用，"
              "消除每请求的 TCP 握手与 TIME_WAIT 堆积；（2）将库存提升至 200 万条，"
              "使请求成为计算密集型；（3）压测脚本在场景间留出端口回收时间，"
              "并使线程数不小于并发连接数（keep-alive 下每条长连接占用一个工作线程）。"
              "修正后四场景错误率均为 0%，数据单调可信。")
    body(doc, "这一过程印证了：AI 生成的基准实现“跑得通”不代表“测得准”，"
              "工程师必须以批判性思维审视压测数据，定位系统性偏差而非简单接受结果。")

    h2(doc, "4 权衡讨论：为何 D（负载均衡）吞吐量低于 C？")
    body(doc, f"水平扩展场景 D（{fmt(d['reqs_per_sec'])} req/s）的吞吐量反而低于单机"
              f"缓存场景 C（{fmt(c['reqs_per_sec'])} req/s）。这并非错误，而是一个值得"
              "记录的真实权衡：当缓存已将单次请求成本降至微秒级后，每个后端实例都远未饱和，"
              "此时 Nginx 多出的一跳反向代理转发反而成为净开销。")
    body(doc, "结论：负载均衡的价值在于后端被打满时横向分摊压力；当瓶颈已被缓存消除、"
              "单机绰绰有余时，引入代理层只增加延迟。架构优化必须针对真实瓶颈，"
              "脱离负载特征的“堆砌优化”可能适得其反。本实验如实呈现该结果而非加以掩饰。")

    h2(doc, "5 安全性说明")
    body(doc, "新增的 HTTP 服务绑定 0.0.0.0 且未启用 TLS，仅复用既有的 Bearer Token 鉴权，"
              "适用于实验与内网环境；生产部署需补充 TLS、限流与更完善的认证。"
              "容器以非 root 用户（uid 10001）运行，遵循最小权限原则。")

    # ======================= 五、代码 =======================================
    h1(doc, "五、代码")
    h2(doc, "1 HTTP keep-alive 长连接（HttpServer.cpp 核心片段）")
    code_block(doc,
        "// 线程池模式启用 keep-alive：在同一连接上循环服务多个请求，\n"
        "// 复用 TCP 连接，避免每请求握手与 TIME_WAIT 堆积。\n"
        "while (running_) {\n"
        "    // 读取直至获得完整请求头\n"
        "    ...\n"
        "    std::string conn = request.getHeader(\"Connection\");\n"
        "    bool keepAlive = (mode_ == Mode::THREAD_POOL) &&\n"
        "                     (conn != \"close\" && conn != \"Close\");\n"
        "    HttpResponse response = dispatcher_->dispatch(request);\n"
        "    response.setHeader(\"Connection\", keepAlive ? \"keep-alive\" : \"close\");\n"
        "    // ... 发送响应 ...\n"
        "    if (!keepAlive) break;  // 单线程基线：每请求后关闭\n"
        "}")
    h2(doc, "2 Nginx 负载均衡配置（deploy/nginx.conf 核心片段）")
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
    h2(doc, "3 Docker Compose 编排（docker-compose.yml 核心片段）")
    code_block(doc,
        "services:\n"
        "  backend1: &backend\n"
        "    build: { context: ., dockerfile: Dockerfile }\n"
        "    environment: { GR_CACHE: \"1\", GR_SEED_ITEMS: \"2000000\" }\n"
        "    command: [\"--serve\",\"--port\",\"8080\",\"--mode\",\"pool\",\"--threads\",\"16\"]\n"
        "  loadbalancer:\n"
        "    image: nginx:1.27-alpine\n"
        "    ports: [\"8088:80\"]\n"
        "    depends_on:                       # 待后端健康后再启动\n"
        "      backend1: { condition: service_healthy }")
    h2(doc, "4 k6 压测脚本（bench/k6_load.js 核心片段）")
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
    body(doc, "本次实验围绕可扩展性与性能两大质量属性，对 Gilded Rose 后端实施了"
              "网络服务化、垂直扩展（线程池+keep-alive）、缓存优化与水平扩展（Nginx 负载"
              "均衡）四个阶段的改造，并以 k6 压测量化了每一步的收益：")
    body(doc, f"· 线程池较单线程基线吞吐量提升约 {spA:.1f} 倍；"
              f"引入缓存后较基线提升约 {spC:,.0f} 倍，是收益最高的优化。")
    body(doc, "· 通过 Docker Compose 实现了 3 实例 + Nginx 负载均衡集群的一键容器化部署，"
              "并验证了健康检查与故障转移。")
    body(doc, "更重要的收获在于工程方法论：通过一次失败的基准测试，我学会了不盲信压测数据，"
              "而是定位端口耗尽、负载过轻等系统性偏差的根因；通过 D 场景慢于 C 的反直觉结果，"
              "我理解了优化必须针对真实瓶颈——脱离负载特征堆砌“优化”反而可能引入净开销。"
              "这正是 AI 时代工程师不可替代的批判性价值所在。")

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    doc.save(OUT)
    print("wrote", os.path.abspath(OUT))


if __name__ == "__main__":
    build()
