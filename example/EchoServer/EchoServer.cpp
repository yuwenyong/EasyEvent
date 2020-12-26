//
// Created by yuwenyong on 2020/12/25.
//

#include "EasyEvent/EasyEvent.h"

using namespace EasyEvent;

class EchoSession: public std::enable_shared_from_this<EchoSession> {
public:
    explicit EchoSession(const ConnectionPtr& conn, Logger* logger)
        : _conn(conn, true), _logger(logger) {

    }

    void start() {
        tryRead();
        _conn->setCloseCallback([this, self=shared_from_this()](std::error_code ec) {
            onClosed(ec);
        });
        _conn.release();
    }

    void tryRead() {
        _conn->readUntil("\n", [this, self=shared_from_this()](std::string data) {
            onReadLine(std::move(data));
        });
    }

    void onReadLine(std::string data) {
        LOG_INFO(_logger) << "Data received: " << data;
        _conn->write(std::move(data));
        tryRead();
    }

    void onClosed(std::error_code ec) {
        LOG_INFO(_logger) << "Closed: " << ec;
        _conn.takeover();
    }
protected:
    ConnectionHolder _conn;
    Logger* _logger;
};

int main (int argc, char **argv) {
    Logger* logger = Log::instance().createLogger("EchoServer", LOG_LEVEL_DEBUG, LOGGER_FLAGS_ASYNC);
    logger->addSink(ConsoleSink::create(true, LOG_LEVEL_DEBUG));

    IOLoop ioLoop(logger, true);
    LOG_INFO(logger) << "Start listening on port: " << 12345;
    auto server = TcpServer::create(&ioLoop);
    server->bind(12345);
    server->start([logger](ConnectionPtr connection, const Address& addr) {
        LOG_INFO(logger) << "Get connection from: " << addr;
        auto session = std::make_shared<EchoSession>(connection, logger);
        session->start();
    });
    ioLoop.start();
    return 0;
}