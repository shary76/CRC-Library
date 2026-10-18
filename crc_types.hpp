// crc_types.hpp
// Core types for the CRC library (Embedded-friendly, C++11+)

#pragma once

#include <cstddef>
#include <cstdint>

namespace crc
{

    //------------------------------------ Byte order helper (currently informational; reserved for future multi-byte helpers) ------------------------------------
    enum class ByteOrder : uint8_t
    {
        BigEndian,
        LittleEndian
    };

    //------------------------------------ Full parameter set following the Rocksoft / Williams model ------------------------------------
    struct Parameters
    {
        uint8_t  width;           // 1 .. 64

        uint64_t polynomial;      // unreflected polynomial
        uint64_t initial;         // initial register value

        bool     reflect_input;   // reflect each input byte (refin)
        bool     reflect_output;  // reflect final register  (refout)

        uint64_t xor_output;      // final XOR

        // Optional verification values (from CRC catalogue)
        // check   = CRC of the ASCII string "123456789"
        // residue = CRC of a valid message with its CRC appended
        uint64_t check;
        uint64_t residue;
    };

    //------------------------------------ Result of parameter validation ------------------------------------
    enum class ValidationError : uint8_t
    {
        None = 0,
        WidthOutOfRange,
        PolynomialOutOfRange,
        InitialOutOfRange,
        XorOutOutOfRange,
        CheckOutOfRange,
        ResidueOutOfRange
    };

} // namespace crc
