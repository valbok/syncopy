/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#include "file.h"
#include <gtest/gtest.h>
#include <fstream>

TEST(File, create)
{
    syncopy::File f("/tmp/create");
    std::vector<uint8_t> v = {1,2,3,4};
    f.write(v);
    EXPECT_TRUE(f.exists());
    EXPECT_EQ(f.size(), 4);
    EXPECT_EQ(f.readAll(), v);
    f.append({5});
    v.push_back(5);
    EXPECT_EQ(f.size(), 5);
    EXPECT_EQ(f.readAll(), v);
    f.write({});
    EXPECT_EQ(f.size(), 0);
    f.remove();
    EXPECT_FALSE(f.exists());
}

TEST(File, sig)
{
    syncopy::File f("/tmp/sig");
    if (f.exists())
        f.remove();

    f.write(std::vector<uint8_t>(1024, 0));
    auto sig = f.signature(1000);

    EXPECT_EQ(sig.chunks.size(), 2);
    EXPECT_EQ(sig.chunks[0].pos, 0);
    EXPECT_EQ(sig.chunks[0].size, 1000);
    EXPECT_EQ(sig.chunks[0].adler32, 65536001);
    EXPECT_EQ(sig.chunks[0].md5, "ede3d3b685b4e137ba4cb2521329a75e");
    EXPECT_EQ(sig.chunks[1].pos, 1000);
    EXPECT_EQ(sig.chunks[1].size, 24);
    EXPECT_EQ(sig.chunks[1].adler32, 1572865);
    EXPECT_EQ(sig.chunks[1].md5, "1681ffc6e046c7af98c9e6c232a3fe0a");

    f.write(std::vector<uint8_t>(11, 0));
    sig = f.signature(5);

    EXPECT_EQ(sig.chunks.size(), 3);

    f.remove();
}

TEST(File, delta_identical)
{
    syncopy::File dst("/tmp/delta_identical1");
    syncopy::File src("/tmp/delta_identical2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 10; ++i)
        bytes.push_back(i % 255);

    dst.append(bytes);
    auto sig = dst.signature(window);
    src.append(bytes);
    auto delta = src.delta(sig);

    EXPECT_EQ(delta.md5, "c56bd5480f6e5413cb62a0ad9666613a");
    EXPECT_EQ(delta.chunks.size(), 2);

    EXPECT_EQ(delta.chunks[0].src_pos, 0);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, window);
    EXPECT_EQ(delta.chunks[0].data.size(), 0);
    EXPECT_EQ(delta.chunks[1].src_pos, 5);
    EXPECT_EQ(delta.chunks[1].dst_pos, 5);
    EXPECT_EQ(delta.chunks[1].size, window);
    EXPECT_EQ(delta.chunks[1].data.size(), 0);

    dst.remove();
    src.remove();
}

TEST(File, delta_identical_plus)
{
    syncopy::File dst("/tmp/delta_identical_plus1");
    syncopy::File src("/tmp/delta_identical_plus2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 10; ++i)
        bytes.push_back(i % 255);

    dst.append(bytes);
    auto sig = dst.signature(window);

    src.append({uint8_t('x')});
    src.append(bytes);
    auto delta = src.delta(sig);

    EXPECT_EQ(delta.md5, "3f6d1006579f41a25165add0c11ef9d2");
    EXPECT_EQ(delta.chunks.size(), 3);

    EXPECT_EQ(delta.chunks[0].src_pos, -1);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, 1);
    EXPECT_EQ(delta.chunks[0].data.size(), 1);
    EXPECT_EQ(delta.chunks[0].data[0], uint8_t('x'));

    EXPECT_EQ(delta.chunks[1].src_pos, 0);
    EXPECT_EQ(delta.chunks[1].dst_pos, 1);
    EXPECT_EQ(delta.chunks[1].size, window);
    EXPECT_EQ(delta.chunks[1].data.size(), 0);

    EXPECT_EQ(delta.chunks[2].src_pos, 5);
    EXPECT_EQ(delta.chunks[2].dst_pos, 6);
    EXPECT_EQ(delta.chunks[2].size, window);
    EXPECT_EQ(delta.chunks[2].data.size(), 0);

    src.append({uint8_t('x')});
    delta = src.delta(sig);

    EXPECT_EQ(delta.md5, "c9bd397b95877ac2598a4cf218a751a6");
    EXPECT_EQ(delta.chunks.size(), 4);

    EXPECT_EQ(delta.chunks[0].src_pos, -1);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, 1);
    EXPECT_EQ(delta.chunks[0].data.size(), 1);
    EXPECT_EQ(delta.chunks[0].data[0], uint8_t('x'));

    EXPECT_EQ(delta.chunks[1].src_pos, 0);
    EXPECT_EQ(delta.chunks[1].dst_pos, 1);
    EXPECT_EQ(delta.chunks[1].size, window);
    EXPECT_EQ(delta.chunks[1].data.size(), 0);

    EXPECT_EQ(delta.chunks[2].src_pos, 5);
    EXPECT_EQ(delta.chunks[2].dst_pos, 6);
    EXPECT_EQ(delta.chunks[2].size, window);
    EXPECT_EQ(delta.chunks[2].data.size(), 0);

    EXPECT_EQ(delta.chunks[3].src_pos, -1);
    EXPECT_EQ(delta.chunks[3].dst_pos, 11);
    EXPECT_EQ(delta.chunks[3].size, 1);
    EXPECT_EQ(delta.chunks[3].data.size(), 1);

    dst.remove();
    src.remove();
}

TEST(File, delta_empty)
{
    syncopy::File dst("/tmp/delta_empty1");
    syncopy::File src("/tmp/delta_empty2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    dst.write({});

    auto sig = dst.signature(window);

    src.write({});
    auto delta = src.delta(sig);

    EXPECT_EQ(delta.md5, "d41d8cd98f00b204e9800998ecf8427e");
    EXPECT_EQ(delta.chunks.size(), 0);

    dst.remove();
    src.remove();
}

TEST(File, delta_src_not_identical)
{
    syncopy::File dst("/tmp/delta_src_not_identical1");
    syncopy::File src("/tmp/delta_src_not_identical2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 10; ++i)
        bytes.push_back(i % 255);

    dst.append(bytes);
    auto sig = dst.signature(window);

    bytes.clear();
    for (int i = 10; i < 30; ++i)
        bytes.push_back(i % 255);

    src.append(bytes);
    auto delta = src.delta(sig);

    EXPECT_EQ(delta.md5, "82e36f4ff37158fef5c68a080bb1a06b");
    EXPECT_EQ(delta.chunks.size(), 1);

    EXPECT_EQ(delta.chunks[0].src_pos, -1);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, 20);
    EXPECT_EQ(delta.chunks[0].data.size(), 20);
    EXPECT_EQ(delta.chunks[0].data[0], uint8_t(10));

    dst.remove();
    src.remove();
}

TEST(File, delta_src_bigger)
{
    syncopy::File dst("/tmp/delta_src_bigger1");
    syncopy::File src("/tmp/delta_src_bigger2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 10; ++i)
        bytes.push_back(i % 255);

    dst.append(bytes);
    auto sig = dst.signature(window);

    bytes.clear();
    for (int i = 0; i < 20; ++i)
        bytes.push_back(i % 255);

    src.append(bytes);
    auto delta = src.delta(sig);

    EXPECT_EQ(delta.md5, "1549d1aae20214e065ab4b76aaac89a8");
    EXPECT_EQ(delta.chunks.size(), 3);

    EXPECT_EQ(delta.chunks[0].src_pos, 0);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, 5);
    EXPECT_EQ(delta.chunks[0].data.size(), 0);

    EXPECT_EQ(delta.chunks[1].src_pos, 5);
    EXPECT_EQ(delta.chunks[1].dst_pos, 5);
    EXPECT_EQ(delta.chunks[1].size, 5);
    EXPECT_EQ(delta.chunks[1].data.size(), 0);

    EXPECT_EQ(delta.chunks[2].src_pos, -1);
    EXPECT_EQ(delta.chunks[2].dst_pos, 10);
    EXPECT_EQ(delta.chunks[2].size, 10);
    EXPECT_EQ(delta.chunks[2].data.size(), 10);
    EXPECT_EQ(delta.chunks[2].data[0], uint8_t(10));

    dst.remove();
    src.remove();
}

TEST(File, delta_first_missed)
{
    syncopy::File dst("/tmp/delta_first_missed1");
    syncopy::File src("/tmp/delta_first_missed2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 10; ++i)
        bytes.push_back(i % 255);


    dst.append(bytes);

    auto sig = dst.signature(window);

    src.append({'x'});
    src.append(bytes);
    src.append({'x','x','x','x','x','x','x','x','x','x'});
    src.append(bytes);
    src.append({'x'});

    auto delta = src.delta(sig);

    EXPECT_EQ(delta.chunks.size(), 7);

    EXPECT_EQ(delta.chunks[0].src_pos, -1);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, 1);
    EXPECT_EQ(delta.chunks[0].data.size(), 1);

    EXPECT_EQ(delta.chunks[1].src_pos, 0);
    EXPECT_EQ(delta.chunks[1].dst_pos, 1);
    EXPECT_EQ(delta.chunks[1].size, 5);
    EXPECT_EQ(delta.chunks[1].data.size(), 0);

    EXPECT_EQ(delta.chunks[2].src_pos, 5);
    EXPECT_EQ(delta.chunks[2].dst_pos, 6);
    EXPECT_EQ(delta.chunks[2].size, 5);
    EXPECT_EQ(delta.chunks[2].data.size(), 0);

    EXPECT_EQ(delta.chunks[3].src_pos, -1);
    EXPECT_EQ(delta.chunks[3].dst_pos, 11);
    EXPECT_EQ(delta.chunks[3].size, 10);
    EXPECT_EQ(delta.chunks[3].data.size(), 10);

    EXPECT_EQ(delta.chunks[4].src_pos, 0);
    EXPECT_EQ(delta.chunks[4].dst_pos, 21);
    EXPECT_EQ(delta.chunks[4].size, 5);
    EXPECT_EQ(delta.chunks[4].data.size(), 0);

    EXPECT_EQ(delta.chunks[5].src_pos, 5);
    EXPECT_EQ(delta.chunks[5].dst_pos, 26);
    EXPECT_EQ(delta.chunks[5].size, 5);
    EXPECT_EQ(delta.chunks[5].data.size(), 0);

    EXPECT_EQ(delta.chunks[6].src_pos, -1);
    EXPECT_EQ(delta.chunks[6].dst_pos, 31);
    EXPECT_EQ(delta.chunks[6].size, 1);
    EXPECT_EQ(delta.chunks[6].data.size(), 1);

    src.append(bytes);

    delta = src.delta(sig);

    EXPECT_EQ(delta.chunks.size(), 9);

    EXPECT_EQ(delta.chunks[8].src_pos, 5);
    EXPECT_EQ(delta.chunks[8].dst_pos, 37);
    EXPECT_EQ(delta.chunks[8].size, 5);
    EXPECT_EQ(delta.chunks[8].data.size(), 0);

    dst.remove();
    src.remove();
}

TEST(File, delta_overlapping)
{
    syncopy::File dst("/tmp/delta_overlapping1");
    syncopy::File src("/tmp/delta_overlapping2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 10; ++i)
        bytes.push_back(i % 255);

    dst.append(bytes);

    auto sig = dst.signature(window);

    bytes.clear();
    for (int i = 5; i < 15; ++i)
        bytes.push_back(i % 255);

    src.append(bytes);

    auto delta = src.delta(sig);

    EXPECT_EQ(delta.chunks.size(), 2);

    EXPECT_EQ(delta.chunks[0].src_pos, 5);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, 5);
    EXPECT_EQ(delta.chunks[0].data.size(), 0);

    EXPECT_EQ(delta.chunks[1].src_pos, -1);
    EXPECT_EQ(delta.chunks[1].dst_pos, 5);
    EXPECT_EQ(delta.chunks[1].size, 5);
    EXPECT_EQ(delta.chunks[1].data.size(), 5);

    dst.remove();
    src.remove();
}

TEST(File, delta_not_even)
{
    syncopy::File dst("/tmp/delta_not_even1");
    syncopy::File src("/tmp/delta_not_even2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 11; ++i)
        bytes.push_back(i % 255);

    dst.append(bytes);
    dst.append(bytes);

    auto sig = dst.signature(window);
    EXPECT_EQ(sig.chunks.size(), 5);

    src.append({'x'});
    src.append(bytes);

    auto delta = src.delta(sig);

    EXPECT_EQ(delta.chunks.size(), 4);

    EXPECT_EQ(delta.chunks[0].src_pos, -1);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, 1);
    EXPECT_EQ(delta.chunks[0].data.size(), 1);

    EXPECT_EQ(delta.chunks[1].src_pos, 0);
    EXPECT_EQ(delta.chunks[1].dst_pos, 1);
    EXPECT_EQ(delta.chunks[1].size, 5);
    EXPECT_EQ(delta.chunks[1].data.size(), 0);

    EXPECT_EQ(delta.chunks[2].src_pos, 5);
    EXPECT_EQ(delta.chunks[2].dst_pos, 6);
    EXPECT_EQ(delta.chunks[2].size, 5);
    EXPECT_EQ(delta.chunks[2].data.size(), 0);

    dst.remove();
    src.remove();
}

static std::string md5(const std::string &path)
{
    uint8_t result[MD5_DIGEST_LENGTH];
    FILE *inFile = fopen (path.c_str(), "rb");
    MD5_CTX mdContext;
    int bytes = 0;
    uint8_t data[1024];

    if (inFile == nullptr) {
        printf ("%s can't be opened.\n", path.c_str());
        return {};
    }

    MD5_Init(&mdContext);
    while ((bytes = fread (data, 1, sizeof(data), inFile)) != 0)
        MD5_Update(&mdContext, data, bytes);
    MD5_Final(result, &mdContext);
    std::ostringstream sout;
    sout << std::hex << std::setfill('0');
    for (auto c: result)
        sout << std::setw(2) << (int)c;

    fclose (inFile);
    return sout.str();
}

TEST(File, patch)
{
    syncopy::File dst("/tmp/patch1");
    syncopy::File src("/tmp/patch2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 11; ++i)
        bytes.push_back(i % 255);

    dst.append(bytes);

    auto sig = dst.signature(window);
    EXPECT_EQ(sig.chunks.size(), 3);

    src.append({'x'});
    src.append(bytes);
    src.append({'x'});

    auto delta = src.delta(sig);

    EXPECT_EQ(delta.chunks.size(), 4);

    EXPECT_EQ(delta.chunks[0].src_pos, -1);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, 1);
    EXPECT_EQ(delta.chunks[0].data.size(), 1);

    EXPECT_EQ(delta.chunks[1].src_pos, 0);
    EXPECT_EQ(delta.chunks[1].dst_pos, 1);
    EXPECT_EQ(delta.chunks[1].size, 5);
    EXPECT_EQ(delta.chunks[1].data.size(), 0);

    EXPECT_EQ(delta.chunks[2].src_pos, 5);
    EXPECT_EQ(delta.chunks[2].dst_pos, 6);
    EXPECT_EQ(delta.chunks[2].size, 5);
    EXPECT_EQ(delta.chunks[2].data.size(), 0);

    EXPECT_EQ(delta.chunks[3].src_pos, -1);
    EXPECT_EQ(delta.chunks[3].dst_pos, 11);
    EXPECT_EQ(delta.chunks[3].size, 2);
    EXPECT_EQ(delta.chunks[3].data.size(), 2);
    EXPECT_EQ(delta.chunks[3].data[1], 'x');

    dst.patch(delta);

    EXPECT_EQ(md5(src.path()), md5(dst.path()));
    EXPECT_EQ(src.mtime(), dst.mtime());

    dst.remove();
    src.remove();
}

TEST(File, patch_empty)
{
    syncopy::File dst("/tmp/patch_empty1");
    syncopy::File src("/tmp/patch_empty2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 11; ++i)
        bytes.push_back(i % 255);

    dst.write({});
    auto sig = dst.signature(window);
    EXPECT_EQ(sig.chunks.size(), 0);

    src.append({'x'});
    src.append(bytes);
    src.append({'x'});

    auto delta = src.delta(sig);

    EXPECT_EQ(delta.chunks.size(), 1);

    EXPECT_EQ(delta.chunks[0].src_pos, -1);
    EXPECT_EQ(delta.chunks[0].dst_pos, 0);
    EXPECT_EQ(delta.chunks[0].size, 13);
    EXPECT_EQ(delta.chunks[0].data.size(), 13);

    dst.patch(delta);

    EXPECT_EQ(md5(src.path()), md5(dst.path()));
    EXPECT_EQ(src.mtime(), dst.mtime());

    dst.remove();
    src.remove();
}

TEST(Signature, serialize)
{
    syncopy::File dst("/tmp/serialize1");
    syncopy::File src("/tmp/serialize2");
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 11; ++i)
        bytes.push_back(i % 255);

    dst.append(bytes);

    auto sig = dst.signature(window);

    std::stringstream out;
    sig.serialize(out);
    out.seekp(0);
    syncopy::Signature sig2;
    sig2.deserialize(out);
    EXPECT_EQ(sig.window, sig2.window);
    EXPECT_EQ(sig.chunks.size(), sig2.chunks.size());
    for (int i = 0; i < sig.chunks.size(); ++i) {
        EXPECT_EQ(sig.chunks[i].pos, sig2.chunks[i].pos);
        EXPECT_EQ(sig.chunks[i].size, sig2.chunks[i].size);
        EXPECT_EQ(sig.chunks[i].adler32, sig2.chunks[i].adler32);
        EXPECT_EQ(sig.chunks[i].md5, sig2.chunks[i].md5);
        EXPECT_EQ(sig.chunks[i], sig2.chunks[i]);
    }
    EXPECT_EQ(sig.chunks, sig2.chunks);
    EXPECT_EQ(sig, sig2);

    src.append({'x'});
    src.append(bytes);

    auto delta = src.delta(sig);

    std::stringstream out2;
    delta.serialize(out2);
    out2.seekp(0);
    syncopy::Delta delta2;
    delta2.deserialize(out2);
    EXPECT_EQ(delta.md5, delta2.md5);
    EXPECT_EQ(delta.chunks.size(), delta2.chunks.size());
    for (int i = 0; i < delta2.chunks.size(); ++i) {
        EXPECT_EQ(delta.chunks[i].src_pos, delta2.chunks[i].src_pos);
        EXPECT_EQ(delta.chunks[i].dst_pos, delta2.chunks[i].dst_pos);
        EXPECT_EQ(delta.chunks[i].data.size(), delta2.chunks[i].data.size());
        EXPECT_EQ(delta.chunks[i].size, delta2.chunks[i].size);
        EXPECT_EQ(delta.chunks[i].data, delta2.chunks[i].data);
        EXPECT_EQ(delta.chunks[i], delta2.chunks[i]);
    }
    EXPECT_EQ(delta, delta2);

    dst.remove();
    src.remove();
}

TEST(Signature, serialize_fstream)
{
    std::string fn1 = "/tmp/serialize1";
    std::string fn2 = "/tmp/serialize2";
    syncopy::File dst(fn1);
    syncopy::File src(fn2);
    if (dst.exists())
        dst.remove();
    if (src.exists())
        src.remove();

    size_t window = 5;
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 11; ++i)
        bytes.push_back(i % 255);

    dst.append(bytes);

    auto sig = dst.signature(window);

    std::ofstream out(fn1 + ".sig");
    sig.serialize(out);
    out.close();

    syncopy::Signature sig2;
    std::ifstream in(fn1 + ".sig");
    sig2.deserialize(in);
    EXPECT_EQ(sig.window, sig2.window);
    EXPECT_EQ(sig.chunks.size(), sig2.chunks.size());
    for (int i = 0; i < sig.chunks.size(); ++i) {
        EXPECT_EQ(sig.chunks[i].pos, sig2.chunks[i].pos);
        EXPECT_EQ(sig.chunks[i].size, sig2.chunks[i].size);
        EXPECT_EQ(sig.chunks[i].adler32, sig2.chunks[i].adler32);
        EXPECT_EQ(sig.chunks[i].md5, sig2.chunks[i].md5);
        EXPECT_EQ(sig.chunks[i], sig2.chunks[i]);
    }
    EXPECT_EQ(sig.chunks, sig2.chunks);
    EXPECT_EQ(sig, sig2);

    src.append({'x'});
    src.append(bytes);

    auto delta = src.delta(sig);

    std::ofstream out2(fn2 + ".delta");
    delta.serialize(out2);
    out2.close();
    syncopy::Delta delta2;
    std::ifstream in2(fn2 + ".delta");
    delta2.deserialize(in2);
    EXPECT_EQ(delta.md5, delta2.md5);
    EXPECT_EQ(delta.chunks.size(), delta2.chunks.size());
    for (int i = 0; i < delta2.chunks.size(); ++i) {
        EXPECT_EQ(delta.chunks[i].src_pos, delta2.chunks[i].src_pos);
        EXPECT_EQ(delta.chunks[i].dst_pos, delta2.chunks[i].dst_pos);
        EXPECT_EQ(delta.chunks[i].data.size(), delta2.chunks[i].data.size());
        EXPECT_EQ(delta.chunks[i].size, delta2.chunks[i].size);
        EXPECT_EQ(delta.chunks[i].data, delta2.chunks[i].data);
        EXPECT_EQ(delta.chunks[i], delta2.chunks[i]);
    }
    EXPECT_EQ(delta, delta2);

    dst.remove();
    src.remove();
}

TEST(File, list)
{
    auto files = syncopy::File::files(".");
    EXPECT_TRUE(files.size() > 0);
}