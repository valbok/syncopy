/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#include "msg.h"
#include "rpc/client.h"
#include <iostream>
#include <chrono>
#include <thread>

const std::string syncopy_ext = ".syncopy";

class Syncopy
{
public:
    Syncopy(std::string const &addr, uint16_t port) : client(addr, port) {}

    rpc::client client;
    std::mutex mutex;
    std::condition_variable cv;
    std::map<std::string, bool> pending;
    bool quit = false;
};

void worker(Syncopy &syncopy)
{
    while (true) {
        std::unique_lock<std::mutex> locker(syncopy.mutex);
        if (syncopy.pending.empty())
            syncopy.cv.wait(locker, [&] { return !syncopy.pending.empty() || syncopy.quit; });
        if (syncopy.quit)
            break;
        if (syncopy.pending.empty())
            continue;

        std::string fn;
        // Try to find available path to upload
        for (auto it = syncopy.pending.begin(); it != syncopy.pending.end(); ++it) {
            // Check if it is not being processed
            if (!it->second) {
                it->second = true;
                fn = it->first;
                break;
            }
        }

        if (fn.empty())
            continue;
        locker.unlock();

        std::cout << fn << ": > signature ..." << std::endl;
        auto rpcsig = syncopy.client.call("signature", fn).as<syncopy::rpc::Msg<syncopy::Signature>>();
        auto sig = rpcsig.unpack();
        std::cout << fn << ": < signature chunks: " << sig.chunks.size() << std::endl;
        syncopy::File cur(fn);
        std::cout << fn << ": creating delta, size: " << cur.size() << std::endl;
        auto delta = cur.delta(sig);
        std::cout << fn << ": delta chunks: " << delta.chunks.size() << std::endl;
        std::cout << fn << ": > patching ..." << std::endl;
        if (syncopy.client.call("patch", fn, syncopy::rpc::Msg<syncopy::Delta>(delta)).as<bool>())
            std::cout << fn << ": < patched" << std::endl;
        else
            std::cerr << fn << ": could not patch" << std::endl;

        while (!locker.try_lock());
        syncopy.pending.erase(fn);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << argv[0] << " SOURCE_DIR [HOST [PORT]]" << std::endl;
        return 0;
    }

    const std::string src_dir = argv[1];
    const std::string host = argc > 2 ? argv[2] : "127.0.0.1";
    const uint16_t port = argc > 3 ? std::stoi(argv[3]) : 4567;

    std::cout << "src dir : " << src_dir << std::endl;
    std::cout << "host    : " << host << std::endl;
    std::cout << "port    : " << port << std::endl;
    syncopy::File::chdir(src_dir);
    std::vector<std::thread> threads;
    Syncopy syncopy(host, port);
    try {
        for (int i = 0; i < 4; ++i)
            threads.push_back(std::thread(worker, std::ref(syncopy)));

        while (true) {
            auto remote_dirs = syncopy.client.call("dirs").as<std::set<std::string>>();
            auto local_dirs = syncopy::rpc::dirs(".");

            for (auto &d : remote_dirs) {
                auto it = local_dirs.find(d);
                if (it == local_dirs.end()) {
                    std::cout << "> removing dir: " << d << " ...";
                    syncopy.client.call("rmdir", d);
                    std::cout << " < ok" << std::endl;
                }
            }

            for (auto &d : local_dirs) {
                auto it = remote_dirs.find(d);
                if (it == remote_dirs.end()) {
                    std::cout << "> creating dir: " << d << " ...";
                    syncopy.client.call("mkdir", d);
                    std::cout << " < ok" << std::endl;
                }
            }

            auto remote_files = syncopy.client.call("files").as<std::map<std::string, syncopy::rpc::Stat>>();
            auto local_files = syncopy::rpc::files(".");

            for (auto &f : remote_files) {
                auto it = local_files.find(f.first);
                if (it == local_files.end()) {
                    std::cout << "> removing file: " << f.first << " ...";
                    syncopy.client.call("rmdir", f.first);
                    std::cout << " < ok" << std::endl;
                }
            }

            for (auto &f : local_files) {
                syncopy::File locf(f.first);
                if (locf.ext() == syncopy_ext)
                    locf.remove();

                auto it = remote_files.find(f.first);
                // If mtime and name are the same
                if (it != remote_files.end() && it->second == f.second)
                    continue;

                {
                    std::lock_guard<std::mutex> locker(syncopy.mutex);
                    auto it = syncopy.pending.find(f.first);
                    // Add to pending only new files and not being processed
                    if (it == syncopy.pending.end() || it->second != true)
                        syncopy.pending[f.first] = false;
                }
                syncopy.cv.notify_all();
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    syncopy.quit = true;
    syncopy.cv.notify_all();
    for (auto &t : threads)
        t.join();

    return EXIT_SUCCESS;
}
