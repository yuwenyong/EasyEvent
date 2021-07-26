//
// Created by yuwenyong.vincent on 2021/6/27.
//

//
// Created by yuwenyong on 2020/12/25.
//

#include "EasyEvent/EasyEvent.h"

using namespace EasyEvent;

class Session {
public:
    static Session& getInstance() {
        static Session instance;
        return instance;
    }

    void start(IOLoop* ioLoop, Logger* logger) {
        _ioLoop = ioLoop;
        _logger = logger;
        _context = SslContext::createDefaultClientContext();
        _context->setVerifyMode(SslVerifyMode::CertNone);
        tryConnect();
    }

    void tryConnect() {
        TcpClient::connect(_ioLoop, "127.0.0.1", 22345, [this] (ConnectionPtr conn, const std::error_code& ec) {
            onConnect(std::move(conn), ec);
        }, _context);
    }

    void tryRead() {
        _conn->readUntil("\n", [this](std::string data) {
            onRecvLine(std::move(data));
        });
    }

    void onConnect(ConnectionPtr conn, const std::error_code& ec) {
        if (ec) {
            onConnectFailed(ec);
        } else {
            onConnectSuccess(std::move(conn));
        }
    }

    void onConnectSuccess(ConnectionPtr conn) {
        LOG_INFO(_logger) << "On connect success";
        _conn = std::move(conn);
        _conn->setCloseCallback([this](std::error_code ec) {
            onClosed(ec);
        });
        tryRead();
    }

    void onConnectFailed(const std::error_code& ec) {
        LOG_ERROR(_logger) << "On connect failed: " << ec << " [" << ec.message() << "]";
        _ioLoop->callLater(Time::seconds(3), [this]() {
            tryConnect();
        });
    }

    void onClosed(std::error_code ec) {
        LOG_ERROR(_logger) << "Connection closed " << ec << " [" << ec.message() << "]";
        _conn = nullptr;
        _ioLoop->callLater(Time::seconds(3), [this]() {
            tryConnect();
        });
    }

    void onRecvLine(std::string line) {
        LOG_INFO(_logger) << "Data received: " << line;
        tryRead();
    }

    void stop() {
        _ioLoop->stop();
    }

    void sendLine(std::string data) {
        LOG_INFO(_logger) << "Send line: " << data;
        if (_conn) {
            _ioLoop->addCallback([this, data=std::move(data)]() mutable {
                if (_conn) {
                    _conn->write(std::move(data));
                }
            });
        }
    }
protected:
    Session() = default;

    IOLoop* _ioLoop{nullptr};
    Logger* _logger{nullptr};
    SslContextPtr _context;
    ConnectionPtr _conn;
};

int main (int argc, char **argv) {
    UnusedParameter(argc);
    UnusedParameter(argv);
    Logger* logger = Log::instance().getLogger("EchoClientSecure");

    IOLoop ioLoop(logger, false);
    Session::getInstance().start(&ioLoop, logger);

    std::thread consoleThread([]() {
        while (true) {
            std::string data;
            std::getline(std::cin, data);
            if (data == "quit") {
                Session::getInstance().stop();
                break;
            }
            data.push_back('\n');
            Session::getInstance().sendLine(std::move(data));
        }
    });

    ioLoop.start();
    consoleThread.join();
    return 0;
}