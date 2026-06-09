#!/usr/bin/env bash
# =============================================================================
# run_benchmarks.sh - Experiment 4 scalability benchmark suite.
#
# Runs k6 against the report endpoint under four configurations to isolate the
# effect of each optimization, collecting req/s and latency into JSON + a CSV
# summary that the report's charts are built from.
#
#   Scenario A  baseline       single-thread server, no cache
#   Scenario B  scale-up       thread-pool server, no cache
#   Scenario C  + cache        thread-pool server, cache ON
#   Scenario D  + load balance  3 instances behind Nginx, cache ON
#
# Usage:  ./bench/run_benchmarks.sh
# Output: bench/results/*.json, bench/results/summary.csv
# =============================================================================
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/build_exp4/GildedRoseBackend"
K6="$ROOT/bench/k6_load.js"
NGINX_CONF="$ROOT/deploy/nginx.conf"
NGINX_PREFIX="$ROOT/deploy"
RESULTS="$ROOT/bench/results"

VUS="${VUS:-50}"
DURATION="${DURATION:-20s}"
SEED="${GR_SEED_ITEMS:-2000000}"
# Pooled scenarios size the worker count >= concurrent connections. With HTTP
# keep-alive, each persistent connection holds one worker for its lifetime, so
# a pool smaller than the client's connection count would starve connections
# and make thread starvation -- not cores/cache/LB -- the bottleneck. Sizing
# threads >= VUS removes that artificial limit so the comparison is clean.
POOL_THREADS="${POOL_THREADS:-64}"

mkdir -p "$RESULTS"

cleanup() {
    nginx -s stop -c "$NGINX_CONF" -p "$NGINX_PREFIX" 2>/dev/null || true
    pkill -f "GildedRoseBackend --serve" 2>/dev/null || true
}
trap cleanup EXIT

wait_up() { # wait_up <port>
    for _ in $(seq 1 30); do
        if curl -s -o /dev/null "http://localhost:$1/api/items?id=1" \
            -H "Authorization: Bearer admin_secret_token"; then return 0; fi
        sleep 0.2
    done
    return 1
}

run_scenario() { # run_scenario <name> <base_url>
    local name="$1" url="$2"
    echo ">>> Scenario $name against $url (VUS=$VUS, DURATION=$DURATION)"
    SCENARIO="$name" VUS="$VUS" DURATION="$DURATION" \
        SUMMARY_OUT="$RESULTS/$name.json" BASE_URL="$url" \
        k6 run -e SCENARIO="$name" -e VUS="$VUS" -e DURATION="$DURATION" \
            -e SUMMARY_OUT="$RESULTS/$name.json" -e BASE_URL="$url" \
            "$K6" 2>&1 | tail -3
}

# Drain interval between scenarios. The single-thread baseline runs connection
# -per-request and leaves thousands of sockets in TIME_WAIT; a short pause lets
# the OS reclaim ephemeral ports so the next scenario starts from a clean slate
# (port exhaustion across back-to-back scenarios corrupted the first run).
DRAIN="${DRAIN:-8}"

# ---- Scenario A: single-thread, no cache -----------------------------------
cleanup
GR_SEED_ITEMS="$SEED" "$BIN" --serve --port 8080 --mode single >/tmp/grA.log 2>&1 &
wait_up 8080 && run_scenario "A_baseline_single" "http://localhost:8080"
cleanup; sleep "$DRAIN"

# ---- Scenario B: thread-pool, no cache -------------------------------------
GR_SEED_ITEMS="$SEED" "$BIN" --serve --port 8080 --mode pool --threads "$POOL_THREADS" >/tmp/grB.log 2>&1 &
wait_up 8080 && run_scenario "B_threadpool_nocache" "http://localhost:8080"
cleanup; sleep "$DRAIN"

# ---- Scenario C: thread-pool + cache ---------------------------------------
GR_SEED_ITEMS="$SEED" GR_CACHE=1 "$BIN" --serve --port 8080 --mode pool --threads "$POOL_THREADS" >/tmp/grC.log 2>&1 &
wait_up 8080 && run_scenario "C_threadpool_cache" "http://localhost:8080"
cleanup; sleep "$DRAIN"

# ---- Scenario D: 3 instances + Nginx load balancer + cache -----------------
# Each instance gets a third of the pool so total worker count matches the
# single-instance pooled scenarios -- D adds processes, not just threads, so
# the comparison isolates the load-balancer effect rather than extra threads.
LB_THREADS=$(( POOL_THREADS / 3 + 1 ))
for p in 9001 9002 9003; do
    GR_SEED_ITEMS="$SEED" GR_CACHE=1 "$BIN" --serve --port "$p" --mode pool --threads "$LB_THREADS" \
        >"/tmp/grD_$p.log" 2>&1 &
done
wait_up 9001 && wait_up 9002 && wait_up 9003
nginx -c "$NGINX_CONF" -p "$NGINX_PREFIX" && sleep 1
run_scenario "D_loadbalanced" "http://localhost:8088"
cleanup

# ---- Build CSV summary ------------------------------------------------------
CSV="$RESULTS/summary.csv"
echo "scenario,vus,reqs_total,reqs_per_sec,latency_avg_ms,latency_p95_ms,latency_p99_ms,error_rate" > "$CSV"
for f in "$RESULTS"/*.json; do
    [ -e "$f" ] || continue
    python3 - "$f" >> "$CSV" <<'PY'
import json, sys
d = json.load(open(sys.argv[1]))
print(",".join(str(d.get(k, "")) for k in
    ["scenario","vus","reqs_total","reqs_per_sec",
     "latency_avg_ms","latency_p95_ms","latency_p99_ms","error_rate"]))
PY
done

echo ""
echo "=== SUMMARY ($CSV) ==="
column -t -s, "$CSV"
