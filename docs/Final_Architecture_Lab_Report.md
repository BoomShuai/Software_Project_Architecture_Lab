# 软件项目架构实验：基于 Gilded Rose C++ 后端系统的架构重构与质量度量

## 一、 相关知识

### 1. 理论概念说明
本次实验的核心议题聚焦于软件架构中的两大核心非功能性质量属性：**可读性（Readability）**与**可修改性（Modifiability）**，并兼顾**安全性（Security）**。
*   **可修改性**：在《软件架构实践》等经典文献中，可修改性被定义为系统在遭受变更（如增加新特性、修改缺陷或适应新环境）时，能够以较低的成本、较快的速度完成修改，且不引入新缺陷的能力。其核心在于“高内聚、低耦合”以及“对扩展开放，对修改封闭”（开闭原则 OCP）。
*   **可读性**：指代码被人类工程师理解的难易程度。可读性直接决定了系统长期的可维护性。
*   **AI 时代下的挑战**：在 LLM（大语言模型）辅助编程爆发的当下，AI 极大地加速了“跑通流程”的样板代码生成。然而，AI 倾向于生成扁平化、缺乏深度抽象的面条代码（Spaghetti Code），且往往忽略安全边界（如缓冲区溢出、硬编码阈值）。这就造成了代码功能正确，但**可扩展性严重不足**。人类工程师面临的首要难题，变成了“如何阅读并改造 AI 生成的劣质逻辑”。
*   **ICONIX 过程思想**：ICONIX 是一种敏捷的面向对象建模方法，强调“用例驱动”。从用例模型推导健壮性图，再映射为类图。在本实验中，所有的架构改进（如拆分不同的商品结算策略类）都严格追溯至初始的“商品结算业务需求”，确保系统演化没有偏离业务本质。

### 2. 本次实验关注的关键技术或工具
为杜绝“主观评判”，本次实验引入了工业界主流的自动化静态分析与 CI/CD 工具：
*   **SonarCloud / SonarQube**：用于量化代码异味（Code Smell）、圈复杂度（Cyclomatic Complexity）、重复率及可维护性指数。选择它的理由是它与 GitHub Actions 无缝集成，能够直观地以图表展示复杂度曲线，并提供权威的质量门禁（Quality Gate）。
*   **CodeQL**：GitHub 原生的语义查询安全扫描引擎。选择理由是它能深入抽象语法树（AST）分析数据流，对 C++ 中的底层内存漏洞（如 `strcpy` 溢出）和外部注入（SQL 注入）有着极高的捕获率。

---

## 二、 需求描述

### 1. 功能性需求
本实验基于经典的 Gilded Rose（镀金玫瑰）商店后端系统。核心用例包括：
*   **日常库存结算**：系统每日自动遍历所有商品，根据具体商品类型（常规物品、陈年干酪 Aged Brie、演唱会门票 Backstage passes、传奇物品 Sulfuras）的不同规则，更新其销售期限（SellIn）和质量（Quality）。
*   **API 接口服务**：提供一套 HTTP 接口供前端调用，实现商品增删改查及强制库存检查。

### 2. 非功能性需求（重点）
鉴于原始 AI 生成代码已满足上述基本功能，本次实验的核心诉求集中于架构质量升级：
1.  **安全性需求**：系统必须免疫常见的底层网络攻击。期望指标：CodeQL 扫描报告中 CWE 漏洞（特别是 CWE-120 溢出和 CWE-89 注入）降为 0。
2.  **可修改性需求**：消除原系统中的“上帝类”。期望指标：核心结算方法的 McCabe 圈复杂度必须降至 15 以下，且后续新增任何商品规则时，不需要修改核心分发逻辑（即完全遵循 OCP）。
3.  **可维护性/可读性需求**：代码可维护性评分在 SonarCloud 上应达到 A 级。消除所有的“魔法数字”与宽泛的 `std::exception`，代之以枚举常量和具名异常。

在 AI 时代，系统的高速迭代使得架构腐化速度极快，因此确立严格的自动化质量门禁（门槛要求）对维持系统生命周期至关重要。

---

## 三、 系统 / 子系统 / 模块结构

### 1. 结构设计与演化
实验对象的原始框架由 AI 生成，其初始结构中存在严重的抽象层次缺失。

*   **最初版本（重构前）存在的问题**：
    *   `ItemController` 模块强耦合鉴权逻辑（直接调用 `TokenInterceptor`），导致控制器充当了安全员。
    *   `GildedRoseService` 是一个典型的上帝类（God Class），负责所有业务逻辑的分发。其 `dailySettlement()` 方法通过一层又一层的 `if-else` 来判别商品类型，圈复杂度极高。
*   **改进后的版本（重构后）差异与原因**：
    *   **提取异常与常量层**：提取 `Exceptions.h` 和 `Constants.h`，提升了可读性和日志的追踪能力。
    *   **AOP 切面解耦**：将 `TokenInterceptor` 的鉴权逻辑上移至 `Dispatcher` 分发器中统一拦截。控制器（Controller）不再处理安全事务，专职业务。
    *   **策略层下沉**：建立 `ItemStrategyFactory`，将复杂的业务规则分散至具体的 `ItemUpdateStrategy` 子类中。

### 2. 模式或惯用法
*   **策略模式 (Strategy Pattern) + 简单工厂模式**：引入这两个模式的根本动机是为了解决 `GildedRoseService` 中无限膨胀的条件分支。对架构质量的影响极其深远：使得结算逻辑在“可扩展性”维度满分，每当业务需要增加新商品（如魔法物品），开发人员只需继承 `ItemUpdateStrategy` 接口，而无需改动调用方的任何一行代码。
*   **面向切面编程 (AOP 惯用法)**：通过在统一的 `Dispatcher` 外层包裹拦截器，消除了跨模块的冗余代码，极大提升了控制器的可测试性与代码的信噪比。

---

## 四、 架构质量属性讨论

### 1. 所关注的质量属性
本次实验着重解决两大质量属性：
*   **可修改性（Modifiability）**：核心挑战在于如何不破坏现有四种商品规则逻辑的前提下，拆解重达 80 多行的面条代码。实现思路是：定义公共行为接口 -> 将现有分支抽取为子类 -> 利用工厂动态绑定。
*   **安全性（Security）**：核心挑战在于 C++ 底层对指针与内存访问的宽容。实现思路是：严禁在网络反序列化中使用 `char[]` 和 `strcpy`，在数据库交互中强制使用预编译（Prepared Statements）。

### 2. 结合代码阐述改进思路
在重构前，AI 生成的安全防御极不规范。例如处理数据库交互时，仅仅进行了简单的字符串拼接，导致 SQL 注入漏洞：
*   **原代码（高危）**：`std::string sql = "SELECT * FROM items WHERE name = '" + name + "';";`
*   **改进思路**：在 C++ 中调用 SQLite3 API 时，必须改用占位符。我重写了封装方法，使用了 `sqlite3_prepare_v2` 配合 `sqlite3_bind_text`。这使得系统底层将外部输入严格视作数据而非执行指令，确保了极高的安全性。

在处理核心逻辑 `dailySettlement()` 时，原代码是一个长达数屏的 `for` 循环与深层 `if` 嵌套。重构后，这部分代码变为：
```cpp
void GildedRoseService::dailySettlement() {
    for (auto& item : MockDatabase::items) {
        std::unique_ptr<ItemUpdateStrategy> strategy(ItemStrategyFactory::createStrategy(item.name));
        strategy->update(item);
    }
}
```
这种设计在代码层面完美映射了可修改性属性：利用多态在运行时进行分支选择。

### 3. 工具度量与评价
通过集成 GitHub Actions CI/CD，得到了明确的前后对比数据：
*   **圈复杂度降低**：根据 SonarCloud 面板，`GildedRoseService.cpp` 的 Cognitive Complexity（认知复杂度）发生了断崖式下跌，代码行数从原先近百行缩减为 12 行。
*   **漏洞归零**：CodeQL 的安全报警（Security Review）完全解除。
*   **改进原因**：多态分派取代了静态的 if-else 分支，消除了环回路径；预编译绑定消除了注入风险。

> **[🖼️ 请在此处插入附图：SonarCloud Cognitive Complexity 走势图或 Code 视图截图，证明复杂度骤降]**
> **[🖼️ 请在此处插入附图：GitHub Actions CodeQL Security Analysis 通过的绿勾截图]**

---

## 五、 代码

### 1. 实现语言与关键技术
*   **实现语言**：选用 **C++17**。理由是 C++17 提供了极佳的系统级控制力与标准库特性。如使用 `<memory>` 库中的 `std::unique_ptr` 进行策略对象的 RAII 内存管理，避免内存泄漏；使用 `std::string_view` 或 `std::string::substr` 替代原生指针操作。
*   **数据库组件**：使用本地嵌入式关系型数据库 **SQLite3**。因其轻量且提供完善的 C 语言 API（预编译宏），非常适合在架构实验中验证参数化查询防注入技术。

### 2. 关键代码清单

**片段一：安全性优化（防 SQL 注入）**
```cpp
// 在 MockDatabase.cpp 中，使用 SQLite3 预编译代替拼接，提高系统安全性
std::string sql = "SELECT * FROM USERS WHERE USERNAME = ?;";
sqlite3_stmt* stmt;
// 1. 准备预编译语句
if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    // 2. 动态绑定输入，防止恶意转义符截断指令
    sqlite3_bind_text(stmt, 1, userInput.c_str(), -1, SQLITE_TRANSIENT);
    // 3. 执行查询
    // ...
}
```

**片段二：可修改性优化（策略模式多态拆解）**
```cpp
// ItemStrategyFactory.cpp - 运用工厂屏蔽具体实例化，增强扩展性
ItemUpdateStrategy* ItemStrategyFactory::createStrategy(const std::string& itemName) {
    if (itemName == Constants::AGED_BRIE) {
        return new AgedBrieStrategy();
    } else if (itemName == Constants::BACKSTAGE_PASS) {
        return new BackstagePassStrategy();
    } else if (itemName == Constants::SULFURAS) {
        return new SulfurasStrategy();
    } else {
        return new NormalItemStrategy();
    }
}
```

**片段三：AOP 思想解耦（拦截器分离）**
```cpp
// Dispatcher.cpp - 将鉴权前置，利用异常中断流，保证控制器单一职责
try {
    // 除了登录接口，一律执行 Token 切面验证
    if (path != "/api/auth/login") {
        TokenInterceptor interceptor;
        interceptor.preHandle(token); // 若鉴权失败，将抛出 UnauthorizedException
    }
    // ... 正常业务路由分发 ...
} catch (const UnauthorizedException& e) {
    response.setStatusCode(401);
    response.setBody("{\"error\":\"Unauthorized\"}");
}
```

---

## 六、 结论

### 1. 结构设计与质量属性收获
在本次实验中，我将一个充满代码异味的 AI 生成雏形项目，重构为了具备工业级架构规范的系统。
最大的收获在于深刻验证了面向对象原则对**可修改性**的提升：通过策略模式，核心代码圈复杂度呈断崖式下跌，可维护性指数被 SonarCloud 评为 A 级。同时，系统**安全性**得到了根本保证，消灭了 CWE 榜单上排名靠前的缓冲区溢出与 SQL 注入风险。过程中遇到的最大挑战是如何在 C++ 中优雅地实现类似 Java 的动态代理（最终采用了静态分发器拦截的方式达到 AOP 效果）。

### 2. 其他心得
AI 编程助手的确极大地缩短了开发周期，它能瞬间完成大量样板代码的堆砌。然而，AI 对系统架构的前瞻性非常薄弱，容易堆积技术债。这给了我极大的启示：**当编码变得廉价时，架构设计的价值就越发凸显**。人类工程师必须把握技术选型、设计模式的运用边界，并建立 CI/CD 质量门禁。
同时，我也发现了重构中的一个遗留问题：由于前期缺乏自动化单元测试（Test-Driven Development），重构后的代码在 SonarCloud 的 Coverage（覆盖率）红线门禁中亮了红灯。这是后续实践中必须第一时间补齐的短板。

### 3. AI 使用声明（必填）
*   **是否使用 AI 辅助**：是。
*   **使用的工具**：Antigravity Agent（基于大模型辅助研发）。
*   **具体用途**：利用 Agent 辅助生成了初始的 C++ 业务逻辑代码与基础脚手架；在排查复杂 C++ CMake 编译错误与分析 CodeQL 报警日志时，使用 AI 进行了思路启发。
*   **人工审查机制**：AI 生成的初始代码存在巨大的安全漏洞与冗余判断。本人对其进行了彻底的人工 Code Review，亲自制定了重构架构图，手动实施了策略模式改写与安全漏洞修复，并主导编写了本份详尽的实验分析报告。

---

## 附录（可选）
A. **Git 提交历史截图**（[🖼️ 建议在此处插入多次重构 Commit 的截图]）
B. **工具生成的完整报告**（[🖼️ 建议在此处插入 SonarCloud 的仪表板总览图或 Quality Gate 报告]）
C. **代码仓库链接**：`https://github.com/BoomShuai/Software_Project_Architecture_Lab`
D. **参考资料列表**：
1. 《软件架构实践（第 4 版）》，Len Bass 等。
2. SonarCloud 官方文档与 C++ 代码质量规范。
3. CodeQL 针对 CWE-120 与 CWE-89 的漏洞修复指引。
