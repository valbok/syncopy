
#include "checksum.h"
#include <gtest/gtest.h>

TEST(Checksum, adler32)
{
    syncopy::checksum::Adler32 a(9);
    EXPECT_EQ(a.hash(), 0);
    a.eat(87);
    a.eat(105);
    a.eat(107);
    a.eat(105);
    a.eat(112);
    a.eat(101);
    a.eat(100);
    a.eat(105);
    a.eat(97);
    EXPECT_EQ(a.hash(), 0x11E60398);

    a = syncopy::checksum::Adler32(2);
    a.eat(87);
    a.eat(105);
    a.update(107, 87);
    auto h = a.hash();

    a = syncopy::checksum::Adler32(2);
    a.eat(105);
    a.eat(107);

    EXPECT_EQ(a.hash(), h);

    a = syncopy::checksum::Adler32(8);
    a.eat(87);
    a.eat(105);
    a.eat(107);
    a.eat(105);
    a.eat(112);
    a.eat(101);
    a.eat(100);
    a.eat(105);
    a.update(97, 87);
    h = a.hash();

    a = syncopy::checksum::Adler32(8);
    a.eat(105);
    a.eat(107);
    a.eat(105);
    a.eat(112);
    a.eat(101);
    a.eat(100);
    a.eat(105);
    a.eat(97);

    EXPECT_EQ(a.hash(), h);

    a = syncopy::checksum::Adler32(3);
    a.eat(87);
    a.eat(105);
    a.eat(107);
    a.update(105, 87);
    a.update(112, 105);
    a.update(101, 107);
    h = a.hash();

    a = syncopy::checksum::Adler32(3);
    a.eat(105);
    a.eat(112);
    a.eat(101);

    a = syncopy::checksum::Adler32(2);
    a.eat(87);
    a.eat(105);
    a.update(107, 87);
    a.update(105, 105);
    a.update(112, 107);
    a.update(101, 105);
    h = a.hash();

    a = syncopy::checksum::Adler32(2);
    a.eat(112);
    a.eat(101);

    a = syncopy::checksum::Adler32(4);
    a.eat(87);
    a.eat(105);
    a.eat(107);
    a.eat(105);
    a.update(112, 87);
    a.update(101, 105);
    h = a.hash();

    a = syncopy::checksum::Adler32(4);
    a.eat(107);
    a.eat(105);
    a.eat(112);
    a.eat(101);

    EXPECT_EQ(a.hash(), h);

    a = syncopy::checksum::Adler32(200);
    a.eat(255);
    a.eat(1);
    a.update(1, 255);
    EXPECT_EQ(a.hash(), 985399299);

    a = syncopy::checksum::Adler32(500);
    for (int i = 0; i < 500; ++i)
        a.eat(1);

    auto b = syncopy::checksum::Adler32(500);
    for (int i = 0; i < 500; ++i)
        b.eat(1);
    EXPECT_EQ(a.hash(), b.hash());

    a = syncopy::checksum::Adler32(500);
    for (int i = 0; i < 500; ++i)
        a.eat(1);

    b = syncopy::checksum::Adler32(500);
    b.eat(2);
    for (int i = 0; i < 499; ++i)
        b.eat(1);
    b.update(1, 2);
    EXPECT_EQ(a.hash(), b.hash());

    a = syncopy::checksum::Adler32(500);
    for (int i = 0; i < 500; ++i)
        a.eat(0);

    b = syncopy::checksum::Adler32(500);
    b.eat(1);
    for (int i = 0; i < 499; ++i)
        b.eat(0);
    b.update(0, 1);
    EXPECT_EQ(a.hash(), b.hash());

    a = syncopy::checksum::Adler32(10);
    for (int i = 0; i < 10; ++i)
        a.eat(0);
    EXPECT_EQ(a.hash(), 655361);

    b = syncopy::checksum::Adler32(10);
    b.eat(49);
    for (int i = 0; i < 9; ++i)
        b.eat(0);
    b.update(0, 49);
    EXPECT_EQ(a.hash(), b.hash());

    a = syncopy::checksum::Adler32(10);
    a.eat(49);
    EXPECT_EQ(a.hash(), 3276850);
    a.eat(0);
    EXPECT_EQ(a.hash(), 6553650);
    a.eat(0);
    EXPECT_EQ(a.hash(), 9830450);
    a.eat(0);
    EXPECT_EQ(a.hash(), 13107250);
    a.eat(0);
    EXPECT_EQ(a.hash(), 16384050);
    a.eat(0);
    EXPECT_EQ(a.hash(), 19660850);
    a.eat(0);
    EXPECT_EQ(a.hash(), 22937650);
    a.eat(0);
    EXPECT_EQ(a.hash(), 26214450);
    a.eat(0);
    EXPECT_EQ(a.hash(), 29491250);
    a.eat(0);
    EXPECT_EQ(a.hash(), 32768050);
    a.update(0, 49);
    EXPECT_EQ(a.hash(), 655361);
    EXPECT_EQ(a.hash(), b.hash());
}