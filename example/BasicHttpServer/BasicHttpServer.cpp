//
// Created by yuwenyong.vincent on 2021/9/25.
//
#include "EasyEvent/EasyEvent.h"

using namespace EasyEvent;

int main (int argc, char **argv) {
    UnusedParameter(argc);
    UnusedParameter(argv);

    Logger* logger = Log::instance().getLogger("EchoServer");

    IOLoop ioLoop(logger, true);

    HttpServerOptions options;
    auto server = HttpServer::create(&ioLoop, [](HttpServerRequestPtr request) {
        ResponseStartLine startLine("HTTP/1.1", 200, "OK");
        HttpHeaders headers;
        headers["Content-Type"] = "text/html";
        request->getConnection()->writeHeaders(startLine, headers);
        request->getConnection()->write("<html><body>Hello world!");
        request->getConnection()->write("Hello world!");
        request->getConnection()->write("Hello world!");
        request->getConnection()->write("Hello world!");
        request->getConnection()->write("Hello world!</body></html>\r\n");
        request->getConnection()->finish();
    }, options);

    server->listen(12345);
    LOG_INFO(logger) << "Start listening on port: " << 12345;
    ioLoop.start();
    return 0;
}