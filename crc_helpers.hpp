//  Append CRC to a buffer
//  Verify packet that ends with CRC
//  Residual check
//  Endian-aware CRC read/write

#pragma once

#include "crc.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace crc
{
    // for writing / reading CRC values
    enum class Endian : uint8_t 
    {
        Little,
        Big
    };

    // Write a CRC value into a buffer (up to 8 bytes)
    inline void write_crc(uint8_t* dst, uint64_t value, std::size_t width_bits, Endian endian = Endian::Little)
    {
        const std::size_t nbytes = (width_bits + 7u) / 8u;

        for (std::size_t i = 0; i < nbytes; ++i)
        {
            const std::size_t shift = (endian == Endian::Little) ? (i * 8u) : ((nbytes - 1u - i) * 8u);
            dst[i] = static_cast<uint8_t>((value >> shift) & 0xFFu);
        }
    }

    // Read a CRC value from a buffer
    inline uint64_t read_crc(const uint8_t* src, std::size_t width_bits, Endian endian = Endian::Little)
    {
        const std::size_t nbytes = (width_bits + 7u) / 8u;
        uint64_t value = 0;

        for (std::size_t i = 0; i < nbytes; ++i)
        {
            const std::size_t shift = (endian == Endian::Little) ? (i * 8u) : ((nbytes - 1u - i) * 8u);
            value |= (uint64_t{src[i]} << shift);
        }
        return value;
    }

    // Append CRC to the end of a buffer / Returns the new total length (data_len + crc_bytes), or 0 on failure / buffer must have enough capacity.
    inline std::size_t append_crc(uint8_t* buffer, std::size_t data_len, std::size_t capacity, const Parameters& params, Endian endian = Endian::Little, bool table_enable = true)
    {
        const std::size_t crc_bytes = (params.width + 7u) / 8u;

        if (buffer == nullptr || data_len + crc_bytes > capacity)
            return 0;

        CRC engine(params, table_enable);
        const uint64_t crc_val = engine.calculate(buffer, data_len);

        write_crc(buffer + data_len, crc_val, params.width, endian);
        return data_len + crc_bytes;
    }

    // Overload using an existing CRC engine (avoids re-creating table)
    inline std::size_t append_crc(uint8_t* buffer, std::size_t data_len, std::size_t capacity, CRC& engine, Endian endian = Endian::Little)
    {
        const std::size_t crc_bytes = engine.size_bytes();

        if (buffer == nullptr || data_len + crc_bytes > capacity)
            return 0;

        engine.reset();
        const uint64_t crc_val = engine.calculate(buffer, data_len);

        write_crc(buffer + data_len, crc_val, engine.width(), endian);
        return data_len + crc_bytes;
    }

    // Verify a packet that ends with a CRC field / Returns true if the CRC embedded at the end matches the calculated CRC.
    inline bool verify_packet(const uint8_t* packet, std::size_t total_len, const Parameters& params, Endian endian = Endian::Little, bool table_enable = true)
    {
        const std::size_t crc_bytes = (params.width + 7u) / 8u;

        if (packet == nullptr || total_len < crc_bytes)
            return false;

        const std::size_t data_len = total_len - crc_bytes;
        const uint64_t received = read_crc(packet + data_len, params.width, endian);

        CRC engine(params, table_enable);
        const uint64_t calculated = engine.calculate(packet, data_len);

        return calculated == received;
    }

    inline bool verify_packet(const uint8_t* packet, std::size_t total_len, CRC& engine, Endian endian = Endian::Little)
    {
        const std::size_t crc_bytes = engine.size_bytes();

        if (packet == nullptr || total_len < crc_bytes)
            return false;

        const std::size_t data_len = total_len - crc_bytes;
        const uint64_t received = read_crc(packet + data_len, engine.width(), endian);

        engine.reset();
        const uint64_t calculated = engine.calculate(packet, data_len);

        return calculated == received;
    }

    // Residual check / Some protocols append the CRC and then expect that CRC(message || CRC) == residue / (useful for incremental validation)
    inline bool check_residue(const uint8_t* packet, std::size_t total_len, const Parameters& params, Endian /*endian*/ = Endian::Little, bool table_enable = true)
    {
        if (packet == nullptr || total_len == 0)
            return false;

        CRC engine(params, table_enable);
        const uint64_t result = engine.calculate(packet, total_len);

        return result == params.residue;
    }

    // Convenience: compute CRC and return it as bytes already written in order
    inline std::size_t crc_to_bytes(uint8_t* out, std::size_t out_capacity, const uint8_t* data, std::size_t data_len, const Parameters& params, Endian endian = Endian::Little, bool table_enable = true)
    {
        const std::size_t crc_bytes = (params.width + 7u) / 8u;
        if (out == nullptr || out_capacity < crc_bytes)
            return 0;

        CRC engine(params, table_enable);
        const uint64_t val = engine.calculate(data, data_len);
        write_crc(out, val, params.width, endian);
        return crc_bytes;
    }

} // namespace crc
