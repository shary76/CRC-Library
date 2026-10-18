// Standard CRC parameter sets (Rocksoft model)

#include "crc_profiles.hpp"

namespace crc
{
    //------------------------------------ 8-bit ------------------------------------
    const Parameters CRC8 =
    {
        8, 0x07, 0x00,
        false, false,
        0x00,
        0xF4, 0x00
    };

    const Parameters CRC8_CDMA2000 =
    {
        8, 0x9B, 0xFF,
        false, false,
        0x00,
        0xDA, 0x00
    };

    const Parameters CRC8_DARC =
    {
        8, 0x39, 0x00,
        true, true,
        0x00,
        0x15, 0x00
    };

    const Parameters CRC8_MAXIM =
    {
        8, 0x31, 0x00,
        true, true,
        0x00,
        0xA1, 0x00
    };

    const Parameters CRC8_WCDMA =
    {
        8, 0x9B, 0x00,
        true, true,
        0x00,
        0x25, 0x00
    };


    //------------------------------------ 16-bit ------------------------------------
    const Parameters CRC16_ARC =
    {
        16, 0x8005, 0x0000,
        true, true,
        0x0000,
        0xBB3D, 0x0000
    };

    const Parameters CRC16_CCITT_FALSE =
    {
        16, 0x1021, 0xFFFF,
        false, false,
        0x0000,
        0x29B1, 0x0000
    };

    const Parameters CRC16_CDMA2000 =
    {
        16, 0xC867, 0xFFFF,
        false, false,
        0x0000,
        0x4C06, 0x0000
    };

    const Parameters CRC16_IBM = CRC16_ARC;   // common alias

    const Parameters CRC16_MODBUS =
    {
        16, 0x8005, 0xFFFF,
        true, true,
        0x0000,
        0x4B37, 0x0000
    };

    const Parameters CRC16_XMODEM =
    {
        16, 0x1021, 0x0000,
        false, false,
        0x0000,
        0x31C3, 0x0000
    };

    const Parameters CRC16_USB =
    {
        16, 0x8005, 0xFFFF,
        true, true,
        0xFFFF,
        0xB4C8, 0x0000
    };

    //------------------------------------ 32-bit ------------------------------------
    const Parameters CRC32_IEEE =
    {
        32, 0x04C11DB7, 0xFFFFFFFF,
        true, true,
        0xFFFFFFFF,
        0xCBF43926, 0xDEBB20E3
    };

    const Parameters CRC32_BZIP2 =
    {
        32, 0x04C11DB7, 0xFFFFFFFF,
        false, false,
        0xFFFFFFFF,
        0xFC891918, 0xC704DD7B
    };

    const Parameters CRC32_CASTAGNOLI =
    {
        32, 0x1EDC6F41, 0xFFFFFFFF,
        true, true,
        0xFFFFFFFF,
        0xE3069283, 0xB798B438
    };

    const Parameters CRC32_MPEG2 =
    {
        32, 0x04C11DB7, 0xFFFFFFFF,
        false, false,
        0x00000000,
        0x0376E6E7, 0x00000000
    };

    const Parameters CRC32_POSIX =
    {
        32, 0x04C11DB7, 0x00000000,
        false, false,
        0xFFFFFFFF,
        0x765E7680, 0xC704DD7B
    };

//------------------------------------ 64-bit ------------------------------------
    const Parameters CRC64_ECMA182 =
    {
        64,
        0x42F0E1EBA9EA3693ULL,
        0x0000000000000000ULL,
        false, false,
        0x0000000000000000ULL,
        0x6C40DF5F0B497347ULL,
        0x0000000000000000ULL
    };

    const Parameters CRC64_ISO =
    {
        64,
        0x000000000000001BULL,
        0xFFFFFFFFFFFFFFFFULL,
        true, true,
        0xFFFFFFFFFFFFFFFFULL,
        0xB90956C775A41001ULL,
        0x5300000000000000ULL
    };

} // namespace crc
