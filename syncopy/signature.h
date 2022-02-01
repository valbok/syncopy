/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#pragma once

#include "checksum.h"
#include <string>
#include <map>
#include <cstdint>
#include <fstream>
#include <vector>
#include <sys/stat.h>

static const std::string SIGNATURE_HEADER = "syncopy::signature";
static const std::string DELTA_HEADER = "syncopy::delta";

namespace syncopy
{
    class Signature
    {

    public:
        struct Chunk
        {
            size_t pos = 0;
            size_t size = 0;
            uint32_t adler32 = 0;
            std::string md5;

            Chunk() = default;
            Chunk(size_t pos, size_t size, uint32_t adler32, const std::string &md5)
                : pos(pos), size(size), adler32(adler32), md5(md5)
            {}

            bool operator==(const Chunk &other) const
            {
                return pos == other.pos && size == other.size && adler32 == other.adler32 && md5 == other.md5;
            }

            void serialize(std::ostream& os) const
            {
                os.write(reinterpret_cast<const char *>(&pos), sizeof(pos));
                os.write(reinterpret_cast<const char *>(&size), sizeof(size));
                os.write(reinterpret_cast<const char *>(&adler32), sizeof(adler32));
                size_t size = md5.size();
                os.write(reinterpret_cast<const char *>(&size), sizeof(size));
                os.write(md5.c_str(), size);
            }

            void deserialize(std::istream& os)
            {
                os.read(reinterpret_cast<char *>(&pos), sizeof(pos));
                os.read(reinterpret_cast<char *>(&size), sizeof(size));
                os.read(reinterpret_cast<char *>(&adler32), sizeof(adler32));
                size_t size = 0;
                os.read(reinterpret_cast<char *>(&size), sizeof(size));
                md5.resize(size);
                os.read(md5.data(), md5.size());
            }
        };

        void serialize(std::ostream& os) const
        {
            os.write(SIGNATURE_HEADER.c_str(), SIGNATURE_HEADER.size());
            os.write(reinterpret_cast<const char *>(&window), sizeof(window));
            size_t size = chunks.size();
            os.write(reinterpret_cast<const char *>(&size), sizeof(size));
            for (auto &a : chunks)
                a.serialize(os);
        }

        bool deserialize(std::istream& os)
        {
            std::string header;
            header.resize(SIGNATURE_HEADER.size());
            os.read(header.data(), SIGNATURE_HEADER.size());
            if (header != SIGNATURE_HEADER)
                return false;
            os.read(reinterpret_cast<char *>(&window), sizeof(window));
            size_t size = 0;
            os.read(reinterpret_cast<char *>(&size), sizeof(size));
            for (size_t i = 0; i < size; ++i) {
                Chunk c;
                c.deserialize(os);
                chunks.push_back(c);
            }

            return true;
        }

        void save(const std::string &path)
        {
            std::ofstream stream(path);
            serialize(stream);
        }

        bool load(const std::string &path)
        {
            std::ifstream f(path);
            return deserialize(f);
        }

        bool operator==(const Signature &other) const
        {
            return window == other.window && chunks == other.chunks;
        }

        uint32_t window = 0;
        std::vector<Chunk> chunks;
    };

    class Delta
    {
    public:
        struct Chunk
        {
            size_t src_pos = -1;
            size_t dst_pos = 0;
            std::vector<uint8_t> data;
            size_t size = 0;

            Chunk() = default;
            Chunk(size_t src_pos, size_t dst_pos, const std::vector<uint8_t> &data, size_t size)
                : src_pos(src_pos), dst_pos(dst_pos), data(data), size(size)
            {}

            bool operator==(const Chunk &other) const
            {
                return src_pos == other.src_pos && dst_pos == other.dst_pos && data == other.data && size == other.size;
            }

            void serialize(std::ostream& os) const
            {
                os.write(reinterpret_cast<const char *>(&src_pos), sizeof(src_pos));
                os.write(reinterpret_cast<const char *>(&dst_pos), sizeof(dst_pos));
                size_t data_size = data.size();
                os.write(reinterpret_cast<const char *>(&data_size), sizeof(data_size));
                os.write(reinterpret_cast<const char *>(data.data()), data.size());
                os.write(reinterpret_cast<const char *>(&size), sizeof(size));
            }

            void deserialize(std::istream& os)
            {
                os.read(reinterpret_cast<char *>(&src_pos), sizeof(src_pos));
                os.read(reinterpret_cast<char *>(&dst_pos), sizeof(dst_pos));
                size_t data_size = 0;
                os.read(reinterpret_cast<char *>(&data_size), sizeof(data_size));
                data.resize(data_size);
                os.read(reinterpret_cast<char *>(data.data()), data_size);
                os.read(reinterpret_cast<char *>(&size), sizeof(size));
            }
        };

        bool operator==(const Delta &other) const
        {
            return md5 == other.md5 && chunks == other.chunks;
        }

        void serialize(std::ostream& os) const
        {
            os.write(DELTA_HEADER.c_str(), DELTA_HEADER.size());
            os.write(reinterpret_cast<const char *>(&st), sizeof(st));
            size_t size = md5.size();
            os.write(reinterpret_cast<const char *>(&size), sizeof(size));
            os.write(md5.c_str(), size);
            size = chunks.size();
            os.write(reinterpret_cast<const char *>(&size), sizeof(size));
            for (auto &a : chunks)
                a.serialize(os);
        }

        bool deserialize(std::istream& os)
        {
            std::string header;
            header.resize(DELTA_HEADER.size());
            os.read(header.data(), DELTA_HEADER.size());
            if (header != DELTA_HEADER)
                return false;
            os.read(reinterpret_cast<char *>(&st), sizeof(st));
            size_t size = 0;
            os.read(reinterpret_cast<char *>(&size), sizeof(size));
            md5.resize(size);
            os.read(md5.data(), md5.size());
            size = 0;
            os.read(reinterpret_cast<char *>(&size), sizeof(size));
            for (size_t i = 0; i < size; ++i) {
                Chunk c;
                c.deserialize(os);
                chunks.push_back(c);
            }

            return true;
        }

        void save(const std::string &path)
        {
            std::ofstream stream(path);
            serialize(stream);
        }

        bool load(const std::string &path)
        {
            std::ifstream f(path);
            return deserialize(f);
        }

        struct stat st;
        std::string md5;
        std::vector<Chunk> chunks;
    };
}

