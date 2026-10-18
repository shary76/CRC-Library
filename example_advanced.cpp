// example_advanced.cpp
// Comprehensive demonstration of the enhanced CRC library:
//   - Runtime CRC class
//   - StaticCRC (template / memory-optimized)
//   - Helpers (append_crc, verify_packet, residual)
//   - STM32 hardware acceleration (with software fallback)

#include "crc.hpp"
#include "crc_profiles.hpp"
#include "crc_static.hpp"
#include "crc_helpers.hpp"
#include "crc_hw_stm32.hpp"

#include <cstdio>
#include <cstring>

using namespace crc;

//------------------------------------ Helper to print a buffer as hex ------------------------------------
static void print_hex(const char* label, const uint8_t* buf, std::size_t len)
{
    std::printf("%-28s", label);
    for (std::size_t i = 0; i < len; ++i)
        std::printf("%02X ", buf[i]);
    std::printf("\n");
}

int main()
{
    const uint8_t payload[] = { 'H','e','l','l','o',' ','C','R','C','!' };
    const std::size_t payload_len = sizeof(payload);

    std::printf("=======================================================\n");
    std::printf("  CRC Library – Advanced Features Demonstration\n");
    std::printf("=======================================================\n\n");

    //------------------------------------ 1. Classic runtime CRC (backward compatible) ------------------------------------
    std::printf("--- 1. Runtime CRC class ---\n");
    {
        CRC crc(CRC16_MODBUS);
        uint16_t r = static_cast<uint16_t>(crc.calculate(payload, payload_len));
        std::printf("CRC-16/MODBUS (runtime)     : 0x%04X\n", r);
    }

    //------------------------------------ 2. StaticCRC – zero-overhead template version ------------------------------------
    std::printf("\n--- 2. StaticCRC (template, optimized) ---\n");
    {
        StaticCRC16_MODBUS engine;
        uint16_t r = engine.calculate(payload, payload_len);
        std::printf("CRC-16/MODBUS (StaticCRC)   : 0x%04X\n", r);

        // Streaming style still works
        StaticCRC32_IEEE c32;
        c32.update(payload, 5);
        c32.update(payload + 5, payload_len - 5);
        std::printf("CRC-32/IEEE  (StaticCRC)    : 0x%08X\n", c32.finalize());
    }

    //------------------------------------ 3. Helpers – append CRC & verify packet ------------------------------------
    std::printf("\n--- 3. Helpers: append_crc / verify_packet ---\n");
    {
        uint8_t packet[64];
        std::memcpy(packet, payload, payload_len);

        // Append CRC-16/MODBUS (little-endian) at the end
        std::size_t total = append_crc(packet, payload_len, sizeof(packet), CRC16_MODBUS, Endian::Little);

        print_hex("Packet + CRC:", packet, total);

        // Verify the packet
        bool ok = verify_packet(packet, total, CRC16_MODBUS, Endian::Little);
        std::printf("verify_packet result         : %s\n", ok ? "VALID" : "INVALID");

        // Corrupt one byte and re-check
        packet[3] ^= 0xFF;
        ok = verify_packet(packet, total, CRC16_MODBUS, Endian::Little);
        std::printf("After corruption             : %s\n", ok ? "VALID" : "INVALID (expected)");
    }

    //------------------------------------ 4. Residual check (advanced / protocol-specific) ------------------------------------
    std::printf("\n--- 4. Residual check ---\n");
    {
        // Residual checking is protocol-dependent and sensitive to the
        // exact byte order in which the CRC was appended.  The primary
        // and recommended way to validate a packet is verify_packet().
        // check_residue() is provided for protocols that explicitly
        // require the residue property.
        std::printf("Note: prefer verify_packet() for normal validation.\n");
        std::printf("check_residue() is available for special protocols.\n");
    }

    //------------------------------------ 5. STM32 Hardware CRC (with automatic software fallback) ------------------------------------
    std::printf("\n--- 5. STM32 Hardware CRC-32 ---\n");
    {
        STM32_CRC32 hw;
        std::printf("Hardware unit active         : %s\n",
                    hw.hardware_active() ? "YES" : "NO (software fallback)");

        uint32_t r = hw.calculate(payload, payload_len);
        std::printf("CRC-32/IEEE (HW or SW)       : 0x%08X\n", r);

        // Convenience free function
        r = crc32_ieee_fast(payload, payload_len);
        std::printf("crc32_ieee_fast()            : 0x%08X\n", r);
    }

    //------------------------------------ 6. Memory comparison (illustrative) ------------------------------------
    std::printf("\n--- 6. Memory characteristics ---\n");
    std::printf("Runtime CRC (table on)        : ~2 KB table + object\n");
    std::printf("Runtime CRC (table off)       : a few dozen bytes\n");
    std::printf("StaticCRC16 (table on)        : table shared in static storage\n");
    std::printf("StaticCRC16 (table off)       : only a 2-byte register\n");
    std::printf("STM32_CRC32 (hardware)        : almost zero RAM + HW unit\n");

    std::printf("\n=======================================================\n");
    std::printf("  All demonstrations finished.\n");
    std::printf("=======================================================\n");

    return 0;
}
