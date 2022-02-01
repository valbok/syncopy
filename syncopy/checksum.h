/*********************************************************
 * Copyright (C) 2022, Val Doroshchuk <valbok@gmail.com> *
 *********************************************************/

#pragma once

#include <openssl/md5.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace syncopy
{
    namespace checksum
    {
        class Adler32
        {
        public:
            explicit Adler32(uint32_t window) : _window(window)
            {
            }

            void eat(uint8_t in)
            {
                _sum1 = (_sum1 + in) % _base;
                _sum2 = (_sum2 + _sum1) % _base;

                _hash = (_sum2 << 16) | _sum1;
            }

            void update(uint8_t in, uint8_t out)
            {
                int sum2 = (_hash >> 16) & 0xffff;
                int sum1 = _hash & 0xffff;

                sum1 += in - out;
                if (sum1 >= _base)
                    sum1 -= _base;
                else if (sum1 < 0)
                    sum1 += _base;

                sum2 = ((int)(sum2 - _window * out + sum1 - 1) % (int)_base);
                if (sum2 < 0)
                    sum2 += _base;
                _hash = (sum2 << 16) | sum1;
            }

            constexpr uint32_t hash() const
            {
                return _hash;
            }

            void reset()
            {
                _sum1 = 1;
                _sum2 = 0;
                _hash = 0;
            }

        private:
            uint32_t _window = 1;
            uint32_t _sum1 = 1;
            uint32_t _sum2 = 0;
            uint32_t _hash = 0;
            static const uint32_t _base = 65521;
        };
    }
}