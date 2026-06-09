#ifndef CACHE_SERVICE_H
#define CACHE_SERVICE_H

#include "../utils/LRUCache.h"
#include <optional>
#include <string>

/**
 * @class CacheService
 * @brief Application-level cache façade for read-heavy endpoints.
 *
 * Scalability rationale (Experiment 4):
 *   Under load, recomputing an aggregate report on every request makes the
 *   CPU the bottleneck and caps throughput. A cache turns repeated identical
 *   reads into O(1) lookups, shifting the system from compute-bound to
 *   memory-bound and letting the thread pool absorb far more concurrency.
 *
 * Two backends are supported behind one interface:
 *   - In-process LRU (always available, single-instance).
 *   - Redis (compiled in with -DUSE_REDIS, linked against hiredis): a shared
 *     cache that multiple backend instances behind a load balancer can hit,
 *     which is what makes horizontal scale-out coherent.
 *
 * Backend selection is by env var GR_REDIS=host:port. If unset (or built
 * without USE_REDIS), the in-process LRU is used. This mirrors the
 * human-vs-AI division of labour discussed in the report: the architectural
 * decision (when/what to cache, invalidation policy) is human-made; the
 * mechanical client wiring is the kind of code AI generates well.
 */
class CacheService {
public:
    CacheService();
    ~CacheService();

    /// Returns the cached value for @p key, or nullopt on miss.
    std::optional<std::string> get(const std::string& key);

    /// Stores @p value under @p key (with optional TTL in seconds for Redis).
    void set(const std::string& key, const std::string& value, int ttlSeconds = 0);

    /// Removes @p key from the cache (write-through invalidation).
    void invalidate(const std::string& key);

    /// True if the Redis backend is active; false means in-process LRU.
    bool usingRedis() const { return usingRedis_; }

    /// Human-readable backend name for logging/reporting.
    std::string backendName() const;

private:
    bool usingRedis_ = false;
    LRUCache<std::string, std::string> local_{256};

#ifdef USE_REDIS
    void* redis_ = nullptr; // redisContext* (opaque to avoid leaking the header)
    bool connectRedis(const std::string& host, int port);
#endif
};

#endif // CACHE_SERVICE_H
