#!/usr/bin/env bash
# =============================================================================
# cluster.sh - start/stop a load-balanced GildedRose backend cluster.
# Experiment 4: horizontal scale-out behind Nginx.
#
#   ./deploy/cluster.sh start [N_THREADS_PER_INSTANCE]
#   ./deploy/cluster.sh stop
#
# Starts 3 backend instances on 9001-9003 (THREAD_POOL mode, cache + large
# inventory seeded) and an Nginx load balancer on :8088.
# =============================================================================
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/build_exp4/GildedRoseBackend"
NGINX_CONF="$ROOT/deploy/nginx.conf"
NGINX_PREFIX="$ROOT/deploy"
PORTS=(9001 9002 9003)
THREADS="${2:-4}"

start() {
    if [[ ! -x "$BIN" ]]; then
        echo "Backend binary not found at $BIN. Build build_exp4 first." >&2
        exit 1
    fi
    for p in "${PORTS[@]}"; do
        GR_SEED_ITEMS="${GR_SEED_ITEMS:-50000}" GR_CACHE="${GR_CACHE:-1}" \
            "$BIN" --serve --port "$p" --mode pool --threads "$THREADS" \
            >"/tmp/gr_backend_$p.log" 2>&1 &
        echo "started backend on :$p (threads=$THREADS)"
    done
    sleep 1
    nginx -c "$NGINX_CONF" -p "$NGINX_PREFIX"
    echo "Nginx load balancer up on http://localhost:8088"
}

stop() {
    nginx -s stop -c "$NGINX_CONF" -p "$NGINX_PREFIX" 2>/dev/null || true
    pkill -f "GildedRoseBackend --serve" 2>/dev/null || true
    echo "cluster stopped"
}

case "${1:-}" in
    start) start ;;
    stop)  stop ;;
    *) echo "usage: $0 {start|stop} [threads]" >&2; exit 1 ;;
esac
