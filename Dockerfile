# =============================================================================
# Multi-stage build for the GildedRose backend (Experiment 4).
# Stage 1 compiles with the full toolchain; stage 2 ships only the binary and
# the SQLite runtime lib, keeping the image small.
# =============================================================================
FROM debian:bookworm-slim AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        g++ cmake make libsqlite3-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY CMakeLists.txt ./
COPY src ./src

# Tests need GoogleTest fetched from the network; the image build only needs
# the application binary, so tests stay OFF here.
RUN cmake -S . -B build -DBUILD_TESTS=OFF \
    && cmake --build build --target GildedRoseBackend -j"$(nproc)"

# ----------------------------------------------------------------------------
FROM debian:bookworm-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        libsqlite3-0 curl \
    && rm -rf /var/lib/apt/lists/*

# Run as a non-root user: the server binds a port but needs no privileges.
RUN useradd --system --no-create-home --uid 10001 appuser

WORKDIR /app
COPY --from=build /src/build/GildedRoseBackend /app/GildedRoseBackend

USER appuser
EXPOSE 8080

# Tunable at run time via compose; defaults give a pooled, cached instance.
ENV GR_CACHE=1 \
    GR_SEED_ITEMS=2000000

# Container-level health check hits the existing item endpoint.
HEALTHCHECK --interval=10s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -fsS -H "Authorization: Bearer admin_secret_token" \
        "http://localhost:8080/api/items?id=1" || exit 1

ENTRYPOINT ["/app/GildedRoseBackend"]
CMD ["--serve", "--port", "8080", "--mode", "pool", "--threads", "16"]
