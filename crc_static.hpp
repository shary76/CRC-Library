// crc_static.hpp
// Compile-time / template-based CRC engine
//
// Advantages over the runtime CRC class:
//   - Zero runtime parameter storage
//   - Register width optimized (uint8/16/32/64)
//   - Table can be static (shared / in flash)
//   - Aggressive compiler inlining & constant folding
//   - Ideal when the CRC algorithm is known at compile time
//
// Example:
//   using MyCRC = crc::StaticCRC<32, 0x04C11DB7, 0xFFFFFFFF, true, true, 0xFFFFFFFF>;
//   MyCRC engine;
//   engine.update(data, len);
//   uint32_t r = engine.finalize();

#pragma once

#include "crc_types.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace crc
{
    namespace detail
    {

        // Smallest unsigned type that can hold 'bits' bits
        template<uint8_t Bits>
        using uint_least_t = typename std::conditional< (Bits <= 8),  uint8_t, typename std::conditional< (Bits <= 16), uint16_t, typename std::conditional< (Bits <= 32), uint32_t, uint64_t >::type >::type >::type;

        template<uint8_t Bits>
        constexpr uint_least_t<Bits> make_mask()
        {
            using T = uint_least_t<Bits>;
            return (Bits == sizeof(T) * 8) ? static_cast<T>(~T{0}) : static_cast<T>((T{1} << Bits) - T{1});
        }

        template<typename T>
        constexpr T reflect_bits(T value, uint8_t bits)
        {
            T result = 0;
            for (uint8_t i = 0; i < bits; ++i)
            {
                if (value & (T{1} << i))
                    result |= T{1} << (bits - 1u - i);
            }
            return result;
        }

    } // namespace detail

//------------------------------------ StaticCRC – fully constexpr-friendly, zero-overhead abstract ------------------------------------
    template<
        uint8_t  Width,
        uint64_t Poly,
        uint64_t Init,
        bool     RefIn,
        bool     RefOut,
        uint64_t XorOut,
        bool     UseTable = true
    >

    class StaticCRC
    {
        static_assert(Width >= 1 && Width <= 64, "Width must be in 1..64");

        public:
            using value_type = detail::uint_least_t<Width>;

            static constexpr uint8_t  width        = Width;
            static constexpr value_type mask       = detail::make_mask<Width>();
            static constexpr value_type polynomial = static_cast<value_type>(Poly) & mask;
            static constexpr value_type initial    = static_cast<value_type>(Init) & mask;
            static constexpr value_type xor_out    = static_cast<value_type>(XorOut) & mask;
            static constexpr bool reflect_in       = RefIn;
            static constexpr bool reflect_out      = RefOut;
            static constexpr bool table_enabled    = UseTable;

            StaticCRC() : crc_(initial) {}

            void reset() { crc_ = initial; }

        //------------------------------------ Streaming ------------------------------------
        void update(uint8_t byte)
        {
            if (UseTable)
                crc_ = process_table(crc_, byte);
            else
                crc_ = process_bit_serial(crc_, byte);
        }

        void update(const uint8_t* data, std::size_t length)
        {
            if (data == nullptr) return;
            for (std::size_t i = 0; i < length; ++i)
                update(data[i]);
        }

        void update(const void* data, std::size_t length)
        {
            update(static_cast<const uint8_t*>(data), length);
        }

        void update_bit(bool bit)
        {
            if (!RefIn)
            {
                const value_type top = value_type{1} << (Width - 1);
                const bool feedback = ((crc_ & top) != 0) ^ bit;
                crc_ = static_cast<value_type>((crc_ << 1) & mask);
                if (feedback) crc_ ^= polynomial;
            }
            else
            {
                const bool feedback = ((crc_ & value_type{1}) != 0) ^ bit;
                crc_ >>= 1;
                if (feedback) crc_ ^= reflected_poly();
                crc_ &= mask;
            }
        }

        //------------------------------------ Result ------------------------------------
        value_type finalize() const
        {
            value_type result = crc_;
            if (RefIn != RefOut)
                result = detail::reflect_bits(result, Width);
            result ^= xor_out;
            return static_cast<value_type>(result & mask);
        }

        value_type value() const { return finalize(); }

        //------------------------------------ One-shot ------------------------------------
        value_type calculate(const uint8_t* data, std::size_t length)
        {
            reset();
            update(data, length);
            return finalize();
        }

        bool verify(const uint8_t* data, std::size_t length, value_type expected) const
        {
            StaticCRC tmp;
            return tmp.calculate(data, length) == expected;
        }

        static constexpr std::size_t size_bytes()
        {
            return (static_cast<std::size_t>(Width) + 7u) / 8u;
        }

        private:
            value_type crc_;

            static constexpr value_type reflected_poly()
            {
                return detail::reflect_bits(polynomial, Width);
            }

            // Bit-serial byte processing
            static value_type process_bit_serial(value_type crc, uint8_t byte)
            {
                if (!RefIn)
                {
                    const value_type top = value_type{1} << (Width - 1);
                    for (uint8_t i = 0; i < 8; ++i)
                    {
                        const bool input_bit = ((byte >> (7u - i)) & 1u) != 0;
                        const bool feedback  = ((crc & top) != 0) ^ input_bit;
                        crc = static_cast<value_type>((crc << 1) & mask);
                        if (feedback) crc ^= polynomial;
                    }
                }
                else
                {
                    for (uint8_t i = 0; i < 8; ++i)
                    {
                        const bool input_bit = ((byte >> i) & 1u) != 0;
                        const bool feedback  = ((crc & value_type{1}) != 0) ^ input_bit;
                        crc >>= 1;
                        if (feedback) crc ^= reflected_poly();
                    }
                    crc &= mask;
                }
                return crc;
            }

            // Table-driven (table lives in static storage → flash on MCU)
            static value_type process_table(value_type crc, uint8_t byte)
            {
                if (!RefIn)
                {
                    const unsigned shift = Width >= 8 ? Width - 8u : 0u;
                    const uint8_t  idx   = static_cast<uint8_t>((crc >> shift) ^ byte);
                    crc = static_cast<value_type>(((crc << 8) ^ table()[idx]) & mask);
                }
                else
                {
                    const uint8_t idx = static_cast<uint8_t>(crc ^ byte);
                    crc = static_cast<value_type>((table()[idx] ^ (crc >> 8)) & mask);
                }
                return crc;
            }

            // Generate table once (function-local static → shared, typically in .data/.bss
            // or can be moved to flash with compiler sections if desired)
            static const value_type* table()
            {
                static value_type tbl[256];
                static bool ready = false;
                if (!ready)
                {
                    for (unsigned i = 0; i < 256; ++i)
                        tbl[i] = process_bit_serial(value_type{0}, static_cast<uint8_t>(i));
                    ready = true;
                }
                return tbl;
            }
    };

    //------------------------------------ Convenient aliases matching the profiles ------------------------------------
    using StaticCRC8              = StaticCRC<8,  0x07, 0x00, false, false, 0x00>;
    using StaticCRC8_MAXIM        = StaticCRC<8,  0x31, 0x00, true,  true,  0x00>;

    using StaticCRC16_MODBUS      = StaticCRC<16, 0x8005, 0xFFFF, true,  true,  0x0000>;
    using StaticCRC16_CCITT_FALSE = StaticCRC<16, 0x1021, 0xFFFF, false, false, 0x0000>;
    using StaticCRC16_XMODEM      = StaticCRC<16, 0x1021, 0x0000, false, false, 0x0000>;
    using StaticCRC16_ARC         = StaticCRC<16, 0x8005, 0x0000, true,  true,  0x0000>;

    using StaticCRC32_IEEE        = StaticCRC<32, 0x04C11DB7, 0xFFFFFFFF, true, true, 0xFFFFFFFF>;
    using StaticCRC32_CASTAGNOLI  = StaticCRC<32, 0x1EDC6F41, 0xFFFFFFFF, true, true, 0xFFFFFFFF>;
    using StaticCRC32_BZIP2       = StaticCRC<32, 0x04C11DB7, 0xFFFFFFFF, false,false,0xFFFFFFFF>;

    using StaticCRC64_ECMA182     = StaticCRC<64, 0x42F0E1EBA9EA3693ULL, 0x0, false, false, 0x0>;

} // namespace crc
