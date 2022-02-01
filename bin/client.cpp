/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#include "msg.h"
#include "rpc/client.h"
#include <iostream>
#include <chrono>

const std::string syncopy_ext = ".syncopy";

int main(int argc, char *argv[])
{
    const std::string host = argc > 1 ? argv[1] : "127.0.0.1";
    const uint16_t port = argc > 2 ? std::stoi(argv[2]) : 4567;

    try {
        rpc::client client(host, port);

        while (true) {
            auto remote_dirs = client.call("dirs").as<std::set<std::string>>();
            auto local_dirs = syncopy::rpc::dirs(".");

            for (auto &d : remote_dirs) {
                auto it = local_dirs.find(d);
                if (it == local_dirs.end())
                    client.call("rmdir", d);
            }

            for (auto &d : local_dirs)
                client.call("mkdir", d);

            auto remote_files = client.call("files").as<std::map<std::string, syncopy::rpc::Stat>>();
            auto local_files = syncopy::rpc::files(".");

            for (auto &f : remote_files) {
                auto it = local_files.find(f.first);
                if (it == local_files.end())
                    client.call("rmdir", f.first);
            }

            for (auto &f : local_files) {
                syncopy::File locf(f.first);
                if (locf.ext() == syncopy_ext)
                    locf.remove();

                auto it = remote_files.find(f.first);
                if (it != remote_files.end() && it->second == f.second)
                    continue;

                std::cout << "> signature: " << f.first << " ..." << std::flush;
                auto rpcsig = client.call("signature", f.first).as<syncopy::rpc::Msg<syncopy::Signature>>();
                auto sig = rpcsig.unpack();
                std::cout << " < ok, chunks: " << sig.chunks.size();
                syncopy::File cur(f.first);
                std::cout << ". Creating delta, size: " << cur.size() << " ..." << std::flush;
                auto delta = cur.delta(sig);
                std::cout << " ok, chunks: " << delta.chunks.size() << std::endl;
                std::cout << "> patching ..." << std::flush;
                if (client.call("patch", f.first, syncopy::rpc::Msg<syncopy::Delta>(delta)).as<bool>())
                    std::cout << " < ok" << std::endl;
                else
                    std::cerr << " < fail, could not patch" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}