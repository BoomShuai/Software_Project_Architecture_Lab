#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "../utils/ThreadPool.h"
#include "../utils/Logger.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <string>

HttpServer::HttpServer(Dispatcher* dispatcher, int port, Mode mode,
                       size_t numThreads)
    : dispatcher_(dispatcher), port_(port), mode_(mode),
      numThreads_(numThreads) {}

void HttpServer::stop() {
    running_ = false;
    if (listenFd_ >= 0) {
        ::shutdown(listenFd_, SHUT_RDWR);
        ::close(listenFd_);
        listenFd_ = -1;
    }
}

void HttpServer::run() {
    int listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        Logger::logError("socket() failed", "HttpServer");
        return;
    }

    // SO_REUSEADDR lets us restart the server immediately without waiting for
    // the kernel TIME_WAIT window, which matters when re-running benchmarks.
    int opt = 1;
    ::setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port_));

    if (::bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        Logger::logError("bind() failed on port " + std::to_string(port_),
                         "HttpServer");
        ::close(listenFd);
        return;
    }

    // A deep backlog keeps connections queued in the kernel instead of being
    // refused while workers are busy -- important under load-test bursts.
    if (::listen(listenFd, 1024) < 0) {
        Logger::logError("listen() failed", "HttpServer");
        ::close(listenFd);
        return;
    }

    listenFd_ = listenFd;
    running_ = true;

    const char* modeName =
        (mode_ == Mode::SINGLE_THREAD) ? "SINGLE_THREAD" : "THREAD_POOL";
    Logger::logInfo("HttpServer listening on port " + std::to_string(port_) +
                        " [mode=" + modeName + "]",
                    "HttpServer");

    if (mode_ == Mode::SINGLE_THREAD) {
        serveSingleThread(listenFd);
    } else {
        serveThreadPool(listenFd);
    }

    if (listenFd_ >= 0) {
        ::close(listenFd_);
        listenFd_ = -1;
    }
}

// Baseline bottleneck: one connection is fully serviced before accept() is
// called again. A single slow handler stalls the entire request queue, so
// throughput is capped at (1 / mean-latency) regardless of available cores.
void HttpServer::serveSingleThread(int listenFd) {
    while (running_) {
        int clientFd = ::accept(listenFd, nullptr, nullptr);
        if (clientFd < 0) {
            if (!running_) break;
            continue;
        }
        handleConnection(clientFd);
    }
}

// Scale-up optimization: the acceptor thread does nothing but accept and hand
// the socket to a worker pool. Up to numThreads_ requests are serviced in
// parallel, so throughput scales with CPU cores until another resource
// (lock, DB) becomes the bottleneck.
void HttpServer::serveThreadPool(int listenFd) {
    ThreadPool pool(numThreads_);
    while (running_) {
        int clientFd = ::accept(listenFd, nullptr, nullptr);
        if (clientFd < 0) {
            if (!running_) break;
            continue;
        }
        // Capture by value: the worker owns and closes this descriptor.
        pool.submit([this, clientFd]() { handleConnection(clientFd); });
    }
}

void HttpServer::handleConnection(int clientFd) {
    // Disable Nagle's algorithm: our responses are tiny JSON blobs, and
    // batching them adds tens of milliseconds of artificial latency that would
    // distort the benchmark.
    int one = 1;
    ::setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    // Idle timeout on the socket so a kept-alive connection that goes quiet
    // releases its worker instead of blocking a pool slot forever.
    timeval tv{};
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    ::setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::string buffer;
    char chunk[8192];

    // HTTP/1.1 keep-alive loop: serve every request that arrives on this one
    // connection, then move on. Reusing the connection avoids a TCP handshake
    // (and an ephemeral port + TIME_WAIT socket) per request, which is what
    // lets a load generator sustain high request rates without exhausting the
    // OS port range -- the failure that corrupted the first benchmark run.
    while (running_) {
        // Read until we have a full header block for the next request.
        size_t headerEnd;
        while ((headerEnd = buffer.find("\r\n\r\n")) == std::string::npos) {
            ssize_t n = ::recv(clientFd, chunk, sizeof(chunk), 0);
            if (n <= 0) { ::close(clientFd); return; } // closed / timed out
            buffer.append(chunk, static_cast<size_t>(n));
            if (buffer.size() > 65536) { ::close(clientFd); return; }
        }

        // This API carries no request bodies (params are in the query string),
        // so the header terminator delimits one complete request.
        std::string requestText = buffer.substr(0, headerEnd + 4);
        buffer.erase(0, headerEnd + 4); // keep any pipelined bytes

        HttpRequest request("", "");
        request.parseRawRequest(requestText);

        // Honor the client's keep-alive preference (HTTP/1.1 defaults to keep
        // -alive unless the client says otherwise). The single-thread baseline
        // intentionally runs connection-per-request: a kept-alive connection
        // would monopolize the lone worker and starve every other client, so
        // the naive server closes after each response. The thread-pool server
        // keeps connections alive, which is one of the things being optimized.
        std::string conn = request.getHeader("Connection");
        bool keepAlive = (mode_ == Mode::THREAD_POOL) &&
                         (conn != "close" && conn != "Close");

        HttpResponse response = dispatcher_->dispatch(request);
        response.setHeader("Connection", keepAlive ? "keep-alive" : "close");

        std::string out = response.generateRawResponse();
        size_t sent = 0;
        while (sent < out.size()) {
            ssize_t n = ::send(clientFd, out.data() + sent, out.size() - sent, 0);
            if (n <= 0) { ::close(clientFd); return; }
            sent += static_cast<size_t>(n);
        }

        if (!keepAlive) break;
    }

    ::close(clientFd);
}
