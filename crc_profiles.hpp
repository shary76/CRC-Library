// crc_profiles.hpp
// Pre-defined standard CRC profiles

#pragma once

#include "crc_types.hpp"

namespace crc
{

    //------------------------------------ 8-bit ------------------------------------
    extern const Parameters CRC8;              // CRC-8          (poly 0x07)
    extern const Parameters CRC8_CDMA2000;     // CRC-8/CDMA2000
    extern const Parameters CRC8_DARC;         // CRC-8/DARC
    extern const Parameters CRC8_MAXIM;        // CRC-8/MAXIM (1-Wire)
    extern const Parameters CRC8_WCDMA;        // CRC-8/WCDMA

    //------------------------------------ 16-bit ------------------------------------
    extern const Parameters CRC16_ARC;         // CRC-16/ARC (IBM)
    extern const Parameters CRC16_CCITT_FALSE; // CRC-16/CCITT-FALSE
    extern const Parameters CRC16_CDMA2000;    // CRC-16/CDMA2000
    extern const Parameters CRC16_IBM;         // alias of ARC
    extern const Parameters CRC16_MODBUS;      // CRC-16/MODBUS
    extern const Parameters CRC16_XMODEM;      // CRC-16/XMODEM
    extern const Parameters CRC16_USB;         // CRC-16/USB

    //------------------------------------ 32-bit ------------------------------------
    extern const Parameters CRC32_IEEE;        // CRC-32 (Ethernet, ZIP, PNG...)
    extern const Parameters CRC32_BZIP2;       // CRC-32/BZIP2
    extern const Parameters CRC32_CASTAGNOLI;  // CRC-32C (Castagnoli) - iSCSI, SCTP...
    extern const Parameters CRC32_MPEG2;       // CRC-32/MPEG-2
    extern const Parameters CRC32_POSIX;       // CRC-32/POSIX

    //------------------------------------ 64-bit ------------------------------------
    extern const Parameters CRC64_ECMA182;     // CRC-64/ECMA-182
    extern const Parameters CRC64_ISO;         // CRC-64/ISO

} // namespace crc
