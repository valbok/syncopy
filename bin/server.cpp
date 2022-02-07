/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#include "msg.h"
#include "rpc/server.h"
#include <fstream>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << argv[0] << " DESTINATION_DIR [HOST [PORT]]" << std::endl;
        return 0;
    }

    const std::string dst_dir = argv[1];
    const std::string host = argc > 2 ? argv[2] : "127.0.0.1";
    const uint16_t port = argc > 3 ? std::stoi(argv[3]) : 4567;

    std::cout << "dst dir : " << dst_dir << std::endl;
    std::cout << "host    : " << host << std::endl;
    std::cout << "port    : " << port << std::endl;
    try {
        syncopy::File::chdir(dst_dir);
        rpc::server srv(host, port);

        srv.bind("dirs", [] { return syncopy::rpc::dirs("."); });
        srv.bind("mkdir", [] (const std::string &d) {
            auto dir = syncopy::rpc::escape(d);
            if (dir.empty())
                return;
            std::cout << "mkdir: " << dir << std::endl;
            syncopy::File::mkdir(dir);
        });
        srv.bind("rmdir", [] (const std::string &d) {
            auto dir = syncopy::rpc::escape(d);
            if (dir.empty())
                return;
            std::cout << "rmdir: " << dir << std::endl;
            syncopy::File::rmdir(dir);
        });
        srv.bind("files", [] { return syncopy::rpc::files("."); });
        srv.bind("signature", [] (const std::string &p) {
            auto path = syncopy::rpc::escape(p);
            if (path.empty())
                return syncopy::rpc::Msg<syncopy::Signature>{};
            std::cout << "signature: " << path << std::endl;
            syncopy::File f(path);
            return syncopy::rpc::Msg<syncopy::Signature>(f.signature());
        });
        srv.bind("patch", [] (const std::string &p, const syncopy::rpc::Msg<syncopy::Delta> &msg) {
            bool result = true;
            auto path = syncopy::rpc::escape(p);
            if (path.empty())
                return false;
            std::cout << "patch: " << path << std::endl;
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