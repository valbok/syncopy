/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#pragma once

#include "checksum.h"
#include "signature.h"
#include <string>

namespace syncopy
{
    /**
     * File handler to read and write to sync source with destination.
     * There are 3 steps to synchronize a file.
     * 1. Generate a signature for the destination file.
     * 2. Generate a delta for the source file (using the signature).
     * 3. Patch the destination file using the generated delta.
     *
     * @example:
     *  dst.patch(src.delta(dst.signature()))
     */

    class File
    {
    public:
        explicit File(const std::string &fn) : _path(fn)
        {
        }

        File() = default;
        ~File() = default;

        std::string path() const { return _path; }
        std::string parent_path() const;
        std::string ext() const;
        size_t size() const;
        time_t mtime() const;

        void write(const std::vector<uint8_t> &data);
        void append(const std::vector<uint8_t> &data);
        bool exists() const;
        void rename(const std::string &to);
        void remove();
        void touch(time_t ts);
        void chmod(mode_t mode);
        std::vector<uint8_t> readAll() const;

        Signature signature(uint32_t window = 1000) const;
        Delta delta(const Signature &sig) const;
        bool patch(const Delta &delta);

        static std::vector<File> files(const std::string &dir);
        static std::vector<std::string> dirs(const std::string &dir);
        static void mkdir(const std::string &dir);
        static void rmdir(const std::string &dir);
    private:
        std::string _path;
    };
}