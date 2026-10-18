// example_usage.cpp
// Simple demonstration of the CRC library (can be compiled standalone)

#include "crc.hpp"
#include "crc_profiles.hpp"

#include <cstdio>
#include <cstdint>

int main()
{
    using namespace crc;

    const uint8_t msg[] = "Hello Embedded World!";

    // ------------------------------------------------------------------
    // 1. One-shot calculation (most common for short messages)
    // ------------------------------------------------------------------
    CRC crc32(CRC32_IEEE);                     // table enabled by default
    uint32_t result = static_cast<uint32_t>(
        crc32.calculate(msg, sizeof(msg) - 1)  // exclude null terminator
    );

    printf("CRC-32/IEEE of message : 0x%08X\n", result);

    // ------------------------------------------------------------------
    // 2. Streaming (recommended when data arrives in chunks)
    // ------------------------------------------------------------------
    CRC modbus(CRC16_MODBUS);

    modbus.update(msg, 5);                     // first chunk
    modbus.update(msg + 5, 10);                // second chunk
    modbus.update(msg + 15, sizeof(msg) - 16); // rest

    uint16_t crc16 = static_cast<uint16_t>(modbus.finalize());
    printf("CRC-16/MODBUS streaming: 0x%04X\n", crc16);

    // ------------------------------------------------------------------
    // 3. Minimal-RAM mode (no lookup table)
    // ------------------------------------------------------------------
    CRC tiny(CRC8_MAXIM, /*table_enable=*/false);
    uint8_t c8 = static_cast<uint8_t>(tiny.calculate(msg, sizeof(msg) - 1));
    printf("CRC-8/MAXIM (bit-serial): 0x%02X\n", c8);

    // ------------------------------------------------------------------
    // 4. Verify against a known good value
    // ------------------------------------------------------------------
    const uint8_t test[] = {'1','2','3','4','5','6','7','8','9'};
    bool ok = CRC(CRC32_IEEE).verify(test, 9, 0xCBF43926);
    printf("Known-vector verify     : %s\n", ok ? "PASS" : "FAIL");

    return 0;
}
