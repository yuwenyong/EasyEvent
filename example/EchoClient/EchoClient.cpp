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

        tryConnect();
    }

    void tryConnect() {
        auto client = TcpClient::create(_ioLoop);
        client->connect([this] (ConnectionPtr conn, const std::error_code& ec) {
            onConnect(std::move(conn), ec);
        }, "127.0.0.1", 12345);
    }

    void tryRead() {
        _conn->readUntil("\n", [this](std::string data) {
            onRecvLine(std::move(data));
        });
    }

    void onConnect(ConnectionPtr conn, const std::error_code& ec) {
        if (ec) {
            onConnectFailed(ec);        } else {
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
    ConnectionPtr _conn;
};

int main (int argc, char **argv) {
    Logger* logger = Log::instance().getLogger("EchoClient");

    IOLoop ioLoop(false);
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