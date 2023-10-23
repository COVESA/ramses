//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "gtest/gtest.h"

#include <city.h>

// test for different data sizes inspired by internal cityhash test (city-test.cc)
namespace ramses::internal
{
    static const uint32_t numTests = 20;
    static const uint32_t dataSize = 1 << numTests;
    static char testData[dataSize];
    static const cityhash::uint128 expectedHashs[numTests] = {
        {0x132a6150371ba800, 0xf37a02a3a52349fb},
        {0x01b87602e96ed410, 0xa23d361672c9df70},
        {0xa368efbdc937a498, 0x61d6b5a232f6f739},
        {0xd3582ceed4b09ab5, 0xfc76d52f6341101d},
        {0x141a2128a55204ac, 0x1b5cf25f42ddae07},
        {0x5a0bf27f6732e977, 0x9379b769aaddc02c},
        {0x7d96a85b976b63be, 0x09d45b45626a5cf7},
        {0x6b1303648efe2f48, 0xfa77d357975cd11a},
        {0x4f60b45cd1bb3c25, 0x40d697ed11923698},
        {0x81b7600bafa2a2a0, 0x5ee67f2fcb4ff797},
        {0x39a86eb98cda9d99, 0xcf4104d1b35bc493},
        {0x61aa07fddf078230, 0xb893b2e0158e4704},
        {0xcadd7cde3d42546c, 0x0d6aeb970736f8ad},
        {0x6decd72a3b46affc, 0x05cde8fd043504ac},
        {0x3e1b6bd7550ca76a, 0x4c16ef2dd6644dcf},
        {0x88b75f69ae8097b1, 0x4c9516874e538fc0},
        {0x4243245f24893037, 0xefd557fc505dd126},
        {0x717c78178375ae41, 0xd5310e0e5c631080},
        {0xd801d11006fe3249, 0x8be55b8f58f3af2c},
        {0x6cc09e60700563e9, 0xd18f23221e964791}
    };

    // Initialize data to pseudorandom values.
    static void setupTestData()
    {
        const uint64_t k0 = 0xc3a5c85c97cb3127ULL;
        uint64_t a = 9;
        uint64_t b = 777;
        for (uint32_t i = 0; i < dataSize; i++)
        {
            a += b;
            b += a;
            a = (a ^ (a >> 41u)) * k0;
            b = (b ^ (b >> 41u)) * k0 + i;
            const auto u = static_cast<uint8_t>((b >> 37u) & 0xffu);
            memcpy(testData + i, &u, 1);
        }
    }

    TEST(Cityhash, CalculateCorrectHashForDifferentDataSizes)
    {
        setupTestData();

        uint32_t offset = 0;
        uint32_t l = 0;
        for (; l < numTests - 1; l++)
        {
            const uint32_t length = 1 << l;
            SCOPED_TRACE(testing::Message("hash error with data size of ") << length << " byte");
            {
                EXPECT_EQ(expectedHashs[l], cityhash::CityHash128(testData + offset, length));
            }
            offset += length;
        }
        EXPECT_EQ(expectedHashs[l], cityhash::CityHash128(testData + 0, dataSize));
    }
}
