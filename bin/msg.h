/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#pragma once

#include "syncopy/file.h"
#include "rpc/msgpack.hpp"
#include <string>
#include <vector>
#include <set>
#include <sstream>

namespace syncopy
{
    namespace rpc
    {
        struct Stat
        {
            size_t size = 0;
            time_t mtime = {0};

            MSGPACK_DEFINE(size, mtime);

            bool operator==(const Stat &other)
            {
                return size == other.size && mtime == other.mtime;
            }
        };

        static std::map<std::string, Stat> files(const std::string &dir)
        {
            std::map<std::string, Stat> result;
            for (auto &f : syncopy::File::files(dir))
                result[f.path()] = {f.size(), f.mtime()};

            return result;
        }

        static std::set<std::string> dirs(const std::string &dir)
        {
            std::set<std::string> result;
            for (auto &f : syncopy::File::dirs(dir))
                result.insert(f);

            return result;
        }

        template<class T>
        struct Msg
        {
            Msg() = default;
            Msg(const T &sig)
            {
                std::ostringstream st;
                sig.serialize(st);
                data = st.str();
            }

            T unpack() const
            {
                T result;
                std::istringstream st;
                st.str(data);
                if (!result.deserialize(st)) {
                    std::cout << "Could not deserialize" << std::endl;
                    return {};
                }

                return result;
            }

            std::string data;
            MSGPACK_DEFINE(data);
        };

        std::string escape(std::string path)
        {
            if (path.substr(0, 2) == "./")
                path.erase(0, 2);
            path = "./" + path;
            std::string p = "../";
            auto n = p.size();
            for (auto i = path.find(p); i != std::string::npos; i = path.find(p))
                path.erase(i, n);
            return path != "./" ? path : std::string{};
        }
    }
}