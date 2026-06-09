#include "CacheService.h"
#include "../utils/Logger.h"
#include <cstdlib>
#include <string>

#ifdef USE_REDIS
#include <hiredis/hiredis.h>
#endif

CacheService::CacheService() {
#ifdef USE_REDIS
    // GR_REDIS=host:port enables the shared Redis backend; absence falls back
    // to the in-process LRU so a single instance still works with no broker.
    if (const char* env = std::getenv("GR_REDIS")) {
        std::string spec(env);
        std::string host = "127.0.0.1";
        int port = 6379;
        size_t colon = spec.find(':');
        if (colon != std::string::npos) {
            host = spec.substr(0, colon);
            port = std::atoi(spec.substr(colon + 1).c_str());
        } else if (!spec.empty()) {
            host = spec;
        }
        if (connectRedis(host, port)) {
            usingRedis_ = true;
            Logger::logInfo("CacheService using Redis at " + host + ":" +
                                std::to_string(port),
                            "CacheService");
        } else {
            Logger::logWarn("Redis connect failed; using in-process LRU",
                            "CacheService");
        }
    }
#endif
}

CacheService::~CacheService() {
#ifdef USE_REDIS
    if (redis_) redisFree(static_cast<redisContext*>(redis_));
#endif
}

std::string CacheService::backendName() const {
    return usingRedis_ ? "Redis (shared)" : "In-process LRU";
}

#ifdef USE_REDIS
bool CacheService::connectRedis(const std::string& host, int port) {
    redisContext* ctx = redisConnect(host.c_str(), port);
    if (!ctx || ctx->err) {
        if (ctx) redisFree(ctx);
        return false;
    }
    redis_ = ctx;
    return true;
}
#endif

std::optional<std::string> CacheService::get(const std::string& key) {
#ifdef USE_REDIS
    if (usingRedis_ && redis_) {
        auto* ctx = static_cast<redisContext*>(redis_);
        redisReply* reply =
            static_cast<redisReply*>(redisCommand(ctx, "GET %s", key.c_str()));
        if (!reply) return std::nullopt;
        std::optional<std::string> result;
        if (reply->type == REDIS_REPLY_STRING) {
            result = std::string(reply->str, reply->len);
        }
        freeReplyObject(reply);
        return result;
    }
#endif
    return local_.get(key);
}

void CacheService::set(const std::string& key, const std::string& value,
                       int ttlSeconds) {
#ifdef USE_REDIS
    if (usingRedis_ && redis_) {
        auto* ctx = static_cast<redisContext*>(redis_);
        redisReply* reply;
        if (ttlSeconds > 0) {
            reply = static_cast<redisReply*>(redisCommand(
                ctx, "SETEX %s %d %s", key.c_str(), ttlSeconds, value.c_str()));
        } else {
            reply = static_cast<redisReply*>(
                redisCommand(ctx, "SET %s %s", key.c_str(), value.c_str()));
        }
        if (reply) freeReplyObject(reply);
        return;
    }
#endif
    (void)ttlSeconds; // LRU has no TTL; eviction is capacity-based
    local_.put(key, value);
}

void CacheService::invalidate(const std::string& key) {
#ifdef USE_REDIS
    if (usingRedis_ && redis_) {
        auto* ctx = static_cast<redisContext*>(redis_);
        redisReply* reply =
            static_cast<redisReply*>(redisCommand(ctx, "DEL %s", key.c_str()));
        if (reply) freeReplyObject(reply);
        return;
    }
#endif
    local_.invalidate(key);
}
