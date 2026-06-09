#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "../controller/Dispatcher.h"
#include <atomic>
#include <cstddef>

/**
 * @class HttpServer
 * @brief A real BSD-socket HTTP server that fronts the existing Dispatcher.
 *
 * Experiment 4 (Scalability): the original backend only *simulated* requests
 * in-process (main.cpp built HttpRequest objects directly), so it could never
 * be load-tested by an external tool. This server exposes the same routing
 * logic over a real TCP socket so tools like k6/ApacheBench can drive it.
 *
 * Two concurrency models are provided to make the before/after comparison
 * concrete:
 *   - Mode::SINGLE_THREAD  : accept() -> handle -> close, strictly serial.
 *                            This is the scalability bottleneck baseline: one
 *                            slow request blocks every other client.
 *   - Mode::THREAD_POOL    : a fixed pool of worker threads pulls accepted
 *                            sockets from a queue, so N requests run in
 *                            parallel (vertical scale-up across CPU cores).
 *
 * The dispatch logic is identical in both modes; only the request-servicing
 * concurrency differs. That isolates the scalability optimization as the
 * single independent variable in the load tests.
 */
class HttpServer {
public:
    enum class Mode { SINGLE_THREAD, THREAD_POOL };

    /**
     * @param dispatcher  Routing/business component (not owned).
     * @param port        TCP port to listen on.
     * @param mode        Concurrency model.
     * @param numThreads  Worker count when mode == THREAD_POOL.
     */
    HttpServer(Dispatcher* dispatcher, int port, Mode mode,
               size_t numThreads = 8);

    /// Binds, listens, and serves until stop() is called. Blocks the caller.
    void run();

    /// Requests a graceful shutdown of the accept loop.
    void stop();

private:
    void serveSingleThread(int listenFd);
    void serveThreadPool(int listenFd);
    /// Reads one HTTP request from a connected socket, dispatches it, writes
    /// the response, and closes the socket.
    void handleConnection(int clientFd);

    Dispatcher* dispatcher_;
    int port_;
    Mode mode_;
    size_t numThreads_;
    std::atomic<bool> running_{false};
    int listenFd_{-1};
};

#endif // HTTP_SERVER_H
