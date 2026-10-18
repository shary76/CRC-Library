// crc.hpp
// High-quality, embedded-friendly CRC library
//
// Features:
//   - Streaming API (update / finalize) – ideal for limited RAM
//   - Optional 256-entry lookup table (fast path)
//   - Bit-serial fallback (minimal RAM)
//   - Full Rocksoft parameter model (refin / refout)
//   - Width 1..64
//   - Pre-defined standard profiles
//   - No dynamic allocation, no exceptions
//
// Typical usage (streaming):
//
//   crc::CRC crc(crc::CRC32_IEEE);          // table enabled by default
//   crc.update(header, header_len);
//   crc.update(payload, payload_len);
//   uint32_t result = static_cast<uint32_t>(crc.finalize());
//
// One-shot:
//
//   uint32_t result = crc.calculate(data, len);

#pragma once

#include "crc_types.hpp"

#include <cstddef>
#include <cstdint>

namespace crc
{

    // Validate a parameter set. Returns ValidationError::None on success.
    ValidationError validate(const Parameters& parameters);

    class CRC
    {
    public:

        //------------------------------------ Construction ------------------------------------
        // table_enable = true  → generate 256-entry table (recommended, ~2 KB RAM)
        // table_enable = false → pure bit-serial (minimal RAM, slower)
        explicit CRC(const Parameters& parameters, bool table_enable = true);

        //------------------------------------ Status ------------------------------------
        bool            is_valid()        const;   // parameters are legal
        ValidationError validation_error() const;

        //------------------------------------ Streaming interface (primary API for embedded) ------------------------------------
        void reset();                              // back to initial value

        void update(const uint8_t* data, std::size_t length);
        void update(uint8_t byte);
        void update_bit(bool bit);                 // single-bit processing

        // Convenience overloads
        void update(const void* data, std::size_t length)
        {
            update(static_cast<const uint8_t*>(data), length);
        }

        // Final result (does not modify internal state)
        uint64_t finalize() const;

        // Same as finalize() – kept for readability
        uint64_t value() const { return finalize(); }

        //------------------------------------ One-shot helpers ------------------------------------
        // Resets, feeds data, returns finalize()
        uint64_t calculate(const uint8_t* data, std::size_t length);

        // Returns true if CRC of data matches expected
        bool verify(const uint8_t* data, std::size_t length, uint64_t expected) const;

        //------------------------------------ Information ------------------------------------
        std::size_t       width()      const;
        std::size_t       size_bytes() const;   // ceil(width / 8)
        const Parameters& parameters() const;

        bool table_enabled() const { return table_enabled_; }

        // Expected check value for "123456789" (from catalogue)
        uint64_t check_value() const { return parameters_.check; }

        // Residue value (CRC of a correct message+CRC)
        uint64_t residue_value() const { return parameters_.residue; }

    private:
        Parameters parameters_;

        uint64_t crc_;
        uint64_t mask_;
        uint64_t top_bit_;
        uint64_t reflected_poly_;   // cached
        bool     table_enabled_;

        // 256-entry lookup table (only used when table_enabled_ == true)
        // For width <= 32 this wastes a little RAM, but keeps code simple
        // and is still acceptable on virtually all modern MCUs.
        uint64_t table_[256];

        // Helpers
        static uint64_t reflect(uint64_t value, uint8_t bits);
        static uint64_t width_mask(uint8_t width);

        void generate_table();

        uint64_t process_byte_bit_serial(uint64_t crc, uint8_t byte) const;
        uint64_t process_byte_table     (uint64_t crc, uint8_t byte) const;
    };

} // namespace crc
