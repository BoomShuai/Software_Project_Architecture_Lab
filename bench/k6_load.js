// =============================================================================
// k6 load test for the GildedRose backend - Experiment 4 (scalability).
//
// Drives the read-heavy report endpoint, which does O(n) work over the seeded
// inventory. This is the hot path where the thread-pool, cache, and load
// balancer optimizations show up.
//
// Configurable via env vars (passed with `k6 run -e KEY=val`):
//   BASE_URL  target base URL          (default http://localhost:8080)
//   VUS       concurrent virtual users (default 50)
//   DURATION  test duration            (default 20s)
//   ENDPOINT  path to hit              (default /api/reports/daily)
//
// Output: k6 prints http_req_duration percentiles and http_reqs rate, which
// are the throughput (req/s) and latency numbers used in the report.
// =============================================================================
import http from 'k6/http';
import { check } from 'k6';
const BASE_URL = __ENV.BASE_URL || 'http://localhost:8080';
const ENDPOINT = __ENV.ENDPOINT || '/api/reports/daily';
const TOKEN = 'Bearer admin_secret_token';

export const options = {
    vus: parseInt(__ENV.VUS || '50'),
    duration: __ENV.DURATION || '20s',
    // Thresholds turn the run into a pass/fail signal we can cite in the report.
    thresholds: {
        http_req_failed: ['rate<0.01'],     // <1% errors
        http_req_duration: ['p(95)<2000'],  // 95% under 2s
    },
};

export default function () {
    const res = http.get(`${BASE_URL}${ENDPOINT}`, {
        headers: { Authorization: TOKEN },
    });
    check(res, {
        'status is 200': (r) => r.status === 200,
    });
}

// Emit a compact JSON summary alongside the human-readable stdout so the
// runner script can collect req/s and latency percentiles for the charts.
export function handleSummary(data) {
    const m = data.metrics;
    const out = {
        scenario: __ENV.SCENARIO || 'unnamed',
        vus: parseInt(__ENV.VUS || '50'),
        duration: __ENV.DURATION || '20s',
        reqs_total: m.http_reqs ? m.http_reqs.values.count : 0,
        reqs_per_sec: m.http_reqs ? m.http_reqs.values.rate : 0,
        latency_avg_ms: m.http_req_duration ? m.http_req_duration.values.avg : 0,
        latency_p95_ms: m.http_req_duration ? m.http_req_duration.values['p(95)'] : 0,
        latency_p99_ms: m.http_req_duration ? m.http_req_duration.values['p(99)'] : 0,
        latency_max_ms: m.http_req_duration ? m.http_req_duration.values.max : 0,
        error_rate: m.http_req_failed ? m.http_req_failed.values.rate : 0,
    };
    const path = __ENV.SUMMARY_OUT || '/tmp/k6_summary.json';
    const result = {};
    result[path] = JSON.stringify(out, null, 2);
    // Also keep the default end-of-test stdout report.
    result['stdout'] = textSummary(data);
    return result;
}

// Minimal text summary (avoids importing k6 remote modules in offline runs).
function textSummary(data) {
    const m = data.metrics;
    const rps = m.http_reqs ? m.http_reqs.values.rate.toFixed(1) : '0';
    const p95 = m.http_req_duration ? m.http_req_duration.values['p(95)'].toFixed(1) : '0';
    const avg = m.http_req_duration ? m.http_req_duration.values.avg.toFixed(1) : '0';
    const errs = m.http_req_failed ? (m.http_req_failed.values.rate * 100).toFixed(2) : '0';
    return `\n  scenario=${__ENV.SCENARIO || 'unnamed'}  ` +
           `req/s=${rps}  avg=${avg}ms  p95=${p95}ms  errors=${errs}%\n`;
}
