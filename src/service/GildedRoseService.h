#ifndef GILDED_ROSE_SERVICE_H
#define GILDED_ROSE_SERVICE_H

/**
 * @class GildedRoseService
 * @brief Manages the daily inventory settlement process.
 * 
 * Implements the core business logic using the Strategy Pattern
 * to update the Quality and SellIn values of all items at the end of each day.
 * 
 * Performance optimizations (Experiment 3):
 * - dailySettlement(): uses pooled strategy objects (no heap allocation per item)
 * - dailySettlementParallel(): multi-threaded batch processing with thread pool
 */
class GildedRoseService {
public:
    /**
     * @brief Executes the daily settlement logic (serial, optimized).
     * 
     * Uses pooled strategy objects via getStrategy() to avoid
     * new/delete overhead in the hot loop.
     */
    void dailySettlement();

    /**
     * @brief Executes the daily settlement logic in parallel.
     * 
     * Partitions items into chunks and processes each chunk in a separate
     * thread using a thread pool. Strategy objects are stateless and shared
     * safely across threads.
     * 
     * @param numThreads Number of worker threads (default: 4).
     */
    void dailySettlementParallel(int numThreads = 4);
};

#endif // GILDED_ROSE_SERVICE_H
