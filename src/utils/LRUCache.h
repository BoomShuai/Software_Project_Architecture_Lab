#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <list>
#include <optional>
#include <mutex>

/**
 * @class LRUCache
 * @brief Thread-safe Least Recently Used (LRU) cache implementation.
 *
 * Uses a combination of std::list and std::unordered_map to achieve
 * O(1) time complexity for both get() and put() operations.
 *
 * Design rationale:
 * - std::list provides O(1) insertion/removal at both ends (for LRU eviction)
 * - std::unordered_map provides O(1) lookup by key
 * - Combined, they form an efficient LRU cache structure
 *
 * @tparam Key The key type (must be hashable).
 * @tparam Value The value type.
 */
template<typename Key, typename Value>
class LRUCache {
public:
    /**
     * @brief Constructs an LRU cache with the given capacity.
     * @param capacity Maximum number of entries to store.
     */
    explicit LRUCache(size_t capacity = 64) : capacity_(capacity) {}

    /**
     * @brief Retrieves a value from the cache.
     *
     * If found, the entry is moved to the front (most recently used).
     *
     * @param key The key to look up.
     * @return The cached value, or std::nullopt if not found.
     */
    std::optional<Value> get(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it == map_.end()) {
            ++missCount_;
            return std::nullopt;
        }
        // Move accessed entry to front (most recently used)
        list_.splice(list_.begin(), list_, it->second);
        ++hitCount_;
        return it->second->second;
    }

    /**
     * @brief Inserts or updates a key-value pair in the cache.
     *
     * If the cache is full, the least recently used entry is evicted.
     *
     * @param key The key.
     * @param value The value to cache.
     */
    void put(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            // Update existing entry and move to front
            it->second->second = value;
            list_.splice(list_.begin(), list_, it->second);
            return;
        }

        // Evict LRU entry if at capacity
        if (list_.size() >= capacity_) {
            auto lastIt = std::prev(list_.end());
            map_.erase(lastIt->first);
            list_.pop_back();
        }

        // Insert new entry at front
        list_.emplace_front(key, value);
        map_[key] = list_.begin();
    }

    /**
     * @brief Invalidates (removes) a specific entry from the cache.
     * @param key The key to remove.
     */
    void invalidate(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            list_.erase(it->second);
            map_.erase(it);
        }
    }

    /**
     * @brief Clears all entries from the cache.
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        list_.clear();
        map_.clear();
        hitCount_ = 0;
        missCount_ = 0;
    }

    /**
     * @brief Returns the current number of cached entries.
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return list_.size();
    }

    /**
     * @brief Returns cache hit count (for profiling/benchmarking).
     */
    size_t getHitCount() const { return hitCount_; }

    /**
     * @brief Returns cache miss count (for profiling/benchmarking).
     */
    size_t getMissCount() const { return missCount_; }

    /**
     * @brief Returns cache hit rate as a percentage.
     */
    double getHitRate() const {
        size_t total = hitCount_ + missCount_;
        return total > 0 ? static_cast<double>(hitCount_) / total * 100.0 : 0.0;
    }

private:
    size_t capacity_;
    // Doubly-linked list: front = most recently used, back = least recently used
    std::list<std::pair<Key, Value>> list_;
    // Map: key -> iterator into the list for O(1) lookup
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> map_;
    mutable std::mutex mutex_;
    size_t hitCount_ = 0;
    size_t missCount_ = 0;
};

#endif // LRU_CACHE_H
