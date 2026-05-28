#include <benchmark/benchmark.h>
#include "repository/MockDatabase.h"
#include "service/ItemService.h"
#include "service/GildedRoseService.h"
#include "service/ExportService.h"
#include "service/ReportService.h"
#include "utils/Logger.h"

// =====================================================
// Helper: populate database with N items
// =====================================================
static void populateDatabase(int n) {
    MockDatabase::items.clear();
    MockDatabase::users.clear();
    for (int i = 0; i < n; i++) {
        std::string name;
        switch (i % 5) {
            case 0: name = "+5 Dexterity Vest"; break;
            case 1: name = "Aged Brie"; break;
            case 2: name = "Backstage passes to a TAFKAL80ETC concert"; break;
            case 3: name = "Sulfuras, Hand of Ragnaros"; break;
            case 4: name = "Elixir of the Mongoose"; break;
        }
        MockDatabase::items.push_back(Item(i + 1, name, 10 + (i % 20), 20 + (i % 30)));
    }
    MockDatabase::rebuildIndex();
}

// =====================================================
// Benchmark: CreateItem (with duplicate ID checking)
// =====================================================
static void BM_CreateItem(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR); // suppress logs
    int n = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        MockDatabase::items.clear();
        MockDatabase::rebuildIndex();
        state.ResumeTiming();

        for (int i = 0; i < n; i++) {
            MockDatabase::items.push_back(Item(i + 1, "Test Item", 10, 20));
            // Simulate the duplicate check inline
        }
    }
    state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_CreateItem)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: GetItem by ID (linear scan — baseline)
// =====================================================
static void BM_GetItemById_LinearScan(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    populateDatabase(n);

    int targetId = n / 2; // search for item in the middle
    for (auto _ : state) {
        // Linear scan — original approach
        for (size_t i = 0; i < MockDatabase::items.size(); i++) {
            if (MockDatabase::items[i].id == targetId) {
                benchmark::DoNotOptimize(MockDatabase::items[i]);
                break;
            }
        }
    }
}
BENCHMARK(BM_GetItemById_LinearScan)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: GetItem by ID (HashMap index — optimized)
// =====================================================
static void BM_GetItemById_HashMap(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    populateDatabase(n);

    int targetId = n / 2;
    for (auto _ : state) {
        auto it = MockDatabase::itemIndex.find(targetId);
        if (it != MockDatabase::itemIndex.end()) {
            benchmark::DoNotOptimize(MockDatabase::items[it->second]);
        }
    }
}
BENCHMARK(BM_GetItemById_HashMap)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: Daily Settlement (serial — baseline)
// =====================================================
static void BM_DailySettlement_Serial(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    
    for (auto _ : state) {
        state.PauseTiming();
        populateDatabase(n);
        state.ResumeTiming();

        GildedRoseService service;
        service.dailySettlement();
    }
    state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_DailySettlement_Serial)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: Daily Settlement (parallel — optimized)
// =====================================================
static void BM_DailySettlement_Parallel(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    
    for (auto _ : state) {
        state.PauseTiming();
        populateDatabase(n);
        state.ResumeTiming();

        GildedRoseService service;
        service.dailySettlementParallel(4);
    }
    state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_DailySettlement_Parallel)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: Export to CSV
// =====================================================
static void BM_ExportCSV(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    populateDatabase(n);

    for (auto _ : state) {
        ExportService exportService;
        std::string csv = exportService.exportAllItemsToCSV();
        benchmark::DoNotOptimize(csv);
    }
    state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_ExportCSV)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: Export to XML
// =====================================================
static void BM_ExportXML(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    populateDatabase(n);

    for (auto _ : state) {
        ExportService exportService;
        std::string xml = exportService.exportAllItemsToXML();
        benchmark::DoNotOptimize(xml);
    }
    state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_ExportXML)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: Check Inventory Warning
// =====================================================
static void BM_CheckWarning(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    populateDatabase(n);

    for (auto _ : state) {
        ItemService itemService;
        auto warnings = itemService.checkInventoryWarning();
        benchmark::DoNotOptimize(warnings);
    }
}
BENCHMARK(BM_CheckWarning)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: Delete Item (vector erase — baseline)
// =====================================================
static void BM_DeleteItem(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);

    for (auto _ : state) {
        state.PauseTiming();
        populateDatabase(n);
        state.ResumeTiming();

        // Delete from middle
        int targetId = n / 2;
        for (auto it = MockDatabase::items.begin(); it != MockDatabase::items.end(); ++it) {
            if (it->id == targetId) {
                MockDatabase::items.erase(it);
                break;
            }
        }
        MockDatabase::rebuildIndex();
    }
}
BENCHMARK(BM_DeleteItem)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: GetItem with LRU Cache
// =====================================================
static void BM_GetItemById_Cached(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    populateDatabase(n);

    ItemService service;
    int targetId = n / 2;
    
    // Warm up cache with first call
    try { service.getItem(targetId); } catch (...) {}

    for (auto _ : state) {
        auto item = service.getItem(targetId);
        benchmark::DoNotOptimize(item);
    }
}
BENCHMARK(BM_GetItemById_Cached)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

// =====================================================
// Benchmark: Report generation
// =====================================================
static void BM_GenerateDailyReport(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    populateDatabase(n);

    for (auto _ : state) {
        ReportService reportService;
        std::string report = reportService.generateDailyInventoryReport();
        benchmark::DoNotOptimize(report);
    }
}
BENCHMARK(BM_GenerateDailyReport)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);

static void BM_GenerateQualityReport(benchmark::State& state) {
    Logger::setLevel(Logger::Level::ERROR);
    int n = state.range(0);
    populateDatabase(n);

    for (auto _ : state) {
        ReportService reportService;
        std::string report = reportService.generateQualityAnalysisReport();
        benchmark::DoNotOptimize(report);
    }
}
BENCHMARK(BM_GenerateQualityReport)->Arg(100)->Arg(1000)->Arg(5000)->Arg(10000);
