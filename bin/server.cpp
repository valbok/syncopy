/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#include "msg.h"
#include "rpc/server.h"
#include <fstream>

int main(int argc, char *argv[])
{
    const std::string host = argc > 1 ? argv[1] : "127.0.0.1";
    const uint16_t port = argc > 2 ? std::stoi(argv[2]) : 4567;

    std::cout << "host: " << host << std::endl;
    std::cout << "port: " << port << std::endl;
    try {
        rpc::server srv(host, port);

        srv.bind("dirs", [] { return syncopy::rpc::dirs("."); });
        srv.bind("mkdir", [] (const std::string &dir) { syncopy::File::mkdir(dir); });
        srv.bind("rmdir", [] (const std::string &dir) { syncopy::File::rmdir(dir); });
        srv.bind("files", [] { return syncopy::rpc::files("."); });
        srv.bind("signature", [] (const std::string &path) {
            syncopy::File f(path);
            return syncopy::rpc::Msg<syncopy::Signature>(f.signature());
        });

        srv.bind("patch", [] (const std::string &path, const syncopy::rpc::Msg<syncopy::Delta> &msg) {
            bool result = true;
            syncopy::File dst(path);
            if (!dst.exists())
                dst.write({});
            auto delta = msg.unpack();
            if (!dst.patch(delta)) {
                std::cerr << "Could not patch: " << dst.path() << std::endl;
                result = false;
            }
            return result;
        });


        srv.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}