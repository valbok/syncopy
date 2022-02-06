/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#include "file.h"
#include <fstream>
#include <cstdio>
#include <time.h>
#include <utime.h>
#include <sys/stat.h>
#include <memory>

#if __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

static std::string toString(uint8_t md5[])
{
    std::ostringstream sout;
    sout << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        sout << std::setw(2) << int(md5[i]);

    return sout.str();
}

static std::string md5(const unsigned char *data, size_t len)
{
    uint8_t result[MD5_DIGEST_LENGTH];
    MD5(data, len, result);
    return toString(result);
}

static struct stat stat(const std::string &path)
{
    struct stat st;
    stat(path.c_str(), &st);
    return st;
}

namespace syncopy
{
    size_t File::size() const
    {
        std::ifstream in(_path, std::ifstream::ate | std::ifstream::binary);
        return in.tellg();
    }

    time_t File::mtime() const
    {
        return stat(_path).st_mtime;
    }

    std::string File::parent_path() const
    {
        fs::path p = _path;
        return p.parent_path();
    }

    std::string File::ext() const
    {
        fs::path p = _path;
        return p.extension();
    }

    void File::write(const std::vector<uint8_t> &data)
    {
        mkdir(parent_path());
        std::ofstream f(_path, std::ios::out | std::ios::binary);
        f.write((const char *)data.data(), data.size());
    }

    void File::append(const std::vector<uint8_t> &data)
    {
        std::fstream f(_path, std::ios::out | std::ios::binary | std::ios_base::app);
        f.write((const char *)data.data(), data.size());
    }

    bool File::exists() const
    {
        std::ifstream f(_path.c_str());
        return f.good();
    }

    void File::rename(const std::string &to)
    {
        std::rename(_path.c_str(), to.c_str());
        _path = to;
    }

    void File::remove()
    {
        fs::remove(_path);
    }

    void File::touch(time_t ts)
    {
        struct utimbuf buf;
        buf.modtime = ts;
        if (utime(_path.c_str(), &buf) != 0)
            std::cout << "Could not change mtime" << std::endl;
    }

    void File::chmod(mode_t mode)
    {
        if (::chmod(_path.c_str(), mode) != 0)
            std::cout << "Could not change mode" << std::endl;
    }

    std::vector<uint8_t> File::readAll() const
    {
        std::ifstream input(_path, std::ios::binary);
        std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
        return buffer;
    }

    Signature File::signature(uint32_t window) const
    {
        Signature result;
        result.window = window;
        std::unique_ptr<FILE, int(*)(FILE*)> f(fopen(_path.c_str(), "r"), &fclose);
        if (!f)
            return result;

        uint8_t buf[window];
        size_t bytesRead = 0;
        size_t pos = 0;
        checksum::Adler32 a(window);
        while ((bytesRead = fread(buf, 1, sizeof(buf), f.get())) > 0) {
            a.reset();
            for (int i = 0; i < bytesRead; ++i)
                a.eat(buf[i]);
            result.chunks.push_back({pos, bytesRead, a.hash(), md5(buf, bytesRead)});
            pos += bytesRead;
        }

        return result;
    }

    static Signature::Chunk query(uint32_t hash, const std::map<uint32_t, std::vector<Signature::Chunk>> &m,
        uint8_t *data, size_t size)
    {
        auto it = m.find(hash);
        if (it != m.end()) {
            auto md5sum = md5(data, size);
            for (auto &a : it->second) {
                if (a.md5 == md5sum)
                    return a;
            }
        }

        return {};
    }

    Delta File::delta(const Signature &sig) const
    {
        Delta result;
        std::unique_ptr<FILE, int(*)(FILE*)> f(fopen(_path.c_str(), "r"), &fclose);
        if (!f)
            return result;

        std::map<uint32_t, std::vector<Signature::Chunk>> m;
        for (auto a : sig.chunks)
            m[a.adler32].push_back(a);

        uint8_t buf[sig.window];
        size_t bytesRead = 0;
        checksum::Adler32 a(sig.window);
        std::vector<uint8_t> data;
        size_t i = 0;
        size_t bytes_count = 0;
        int start = 0;

        uint8_t md5[MD5_DIGEST_LENGTH];
        MD5_CTX mdContext;
        MD5_Init(&mdContext);

        while ((bytesRead = fread(buf, 1, sizeof(buf), f.get())) > 0) {
            MD5_Update(&mdContext, buf, bytesRead);
            data.insert(data.end(), buf, buf + bytesRead);
            while (!m.empty() && i < data.size()) {
                start = i - sig.window;

                if (start >= 0)
                    a.update(data[i], data[start]);
                else
                    a.eat(data[i]);

                if (start + 1 >= 0) {
                    auto matched = query(a.hash(), m, &data.data()[start + 1], sig.window);
                    if (matched.size > 0) {
                        a.reset();
                        auto missed_pos = bytes_count - i;
                        std::vector<uint8_t> missed_data(data.begin(), data.begin() + start + 1);
                        if (!missed_data.empty())
                            result.chunks.push_back({size_t(-1), missed_pos, missed_data, missed_data.size()});

                        data.erase(data.begin(), data.begin() + i + 1);
                        result.chunks.push_back({matched.pos, missed_pos + missed_data.size(), {}, matched.size});
                        i = -1;
                    }
                }

                ++i;
                ++bytes_count;
            }
        }

        if (!data.empty()) {
            auto matched = query(a.hash(), m, data.data(), data.size());
            result.chunks.push_back(
                matched.size > 0 ? Delta::Chunk{matched.pos, bytes_count - i, {}, matched.size}
                                 : Delta::Chunk{size_t(-1), bytes_count - i, data, data.size()}
            );
        }

        MD5_Final(md5, &mdContext);
        result.md5 = toString(md5);
        result.st = stat(_path);

        return result;
    }

    bool File::patch(const Delta &delta)
    {
        std::unique_ptr<FILE, int(*)(FILE*)> f(fopen(_path.c_str(), "r"), &fclose);
        if (!f)
            return false;

        uint8_t md5[MD5_DIGEST_LENGTH];
        MD5_CTX mdContext;
        MD5_Init(&mdContext);

        std::string fn = _path + ".syncopy";
        std::unique_ptr<FILE, int(*)(FILE*)> syncopy(fopen(fn.c_str(), "w"), &fclose);
        if (!syncopy) {
            std::cerr << "Could not open file: " << fn << std::endl;
            return false;
        }

        for (auto &chunk : delta.chunks) {
            if (!chunk.data.empty()) {
                MD5_Update(&mdContext, chunk.data.data(), chunk.data.size());
                fwrite(chunk.data.data(), sizeof(char), chunk.data.size(), syncopy.get());
                continue;
            }

            fseek(f.get(), chunk.src_pos, SEEK_SET);
            uint8_t buf[chunk.size];
            int bytesRead = 0;
            while ((bytesRead = fread(buf, 1, sizeof(buf), f.get())) > 0) {
                if (bytesRead == chunk.size) {
                    MD5_Update(&mdContext, buf, bytesRead);
                    fwrite(buf, sizeof(char), bytesRead, syncopy.get());
                } else {
                    std::cerr << "Size mismatch, size: " << chunk.size << " bytesRead:" << bytesRead << std::endl;
                    File(fn).remove();
                    return false;
                }
                break;
            }
        }

        MD5_Final(md5, &mdContext);
        auto md5sum = toString(md5);
        if (delta.md5 != md5sum) {
            std::cerr << "Cound not patch, md5 mismatch: '" << delta.md5 <<"' != '" << md5sum << "'" << std::endl;
            File(fn).remove();
            return false;
        }

        syncopy.reset();
        File result(fn);
        result.touch(delta.st.st_mtime);
        result.chmod(delta.st.st_mode);
        result.rename(_path);
        return true;
    }

    std::vector<File> File::files(const std::string &dir)
    {
        std::vector<File> result;
        for (auto &entry : fs::recursive_directory_iterator(dir)) {
            if (!fs::is_directory(entry)) {
                result.emplace_back(entry.path());
            }
        }

        return result;
    }

    std::vector<std::string> File::dirs(const std::string &dir)
    {
        std::vector<std::string> result;
        for (auto &entry : fs::recursive_directory_iterator(dir)) {
            if (fs::is_directory(entry))
                result.push_back(entry.path());
        }

        return result;
    }

    void File::mkdir(const std::string &dir)
    {
        fs::create_directories(dir);
    }

    void File::rmdir(const std::string &dir)
    {
        fs::remove_all(dir);
    }

    void File::chdir(const std::string &dir)
    {
        fs::current_path(dir);
    }
}