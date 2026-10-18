// crc.cpp
// Implementation of the embedded-friendly CRC library

#include "crc.hpp"

namespace crc
{
    //------------------------------------ Internal helpers ------------------------------------
    namespace
    {

        uint64_t width_mask(uint8_t width)
        {
            if (width == 0 || width > 64)
                return 0;

            if (width == 64)
                return UINT64_MAX;

            return (uint64_t{1} << width) - uint64_t{1};
        }

    } // anonymous namespace

    // Public free function (also used by the class)
    ValidationError validate(const Parameters& p)
    {
        if (p.width == 0 || p.width > 64)
            return ValidationError::WidthOutOfRange;

        const uint64_t m = width_mask(p.width);

        if ((p.polynomial  & ~m) != 0) return ValidationError::PolynomialOutOfRange;
        if ((p.initial     & ~m) != 0) return ValidationError::InitialOutOfRange;
        if ((p.xor_output  & ~m) != 0) return ValidationError::XorOutOutOfRange;
        if ((p.check       & ~m) != 0) return ValidationError::CheckOutOfRange;
        if ((p.residue     & ~m) != 0) return ValidationError::ResidueOutOfRange;

        return ValidationError::None;
    }

    //------------------------------------ CRC class – construction ------------------------------------
    CRC::CRC(const Parameters& parameters, bool table_enable)
        : parameters_(parameters)
        , crc_(0)
        , mask_(0)
        , top_bit_(0)
        , reflected_poly_(0)
        , table_enabled_(false)
    {
        if (validate(parameters_) != ValidationError::None)
            return;

        mask_           = width_mask(parameters_.width);
        top_bit_        = uint64_t{1} << (parameters_.width - 1u);
        reflected_poly_ = reflect(parameters_.polynomial, parameters_.width);
        crc_            = parameters_.initial & mask_;

        table_enabled_  = table_enable;
        if (table_enabled_)
            generate_table();
    }

    //------------------------------------ Status ------------------------------------
    bool CRC::is_valid() const
    {
        return validate(parameters_) == ValidationError::None;
    }

    ValidationError CRC::validation_error() const
    {
        return validate(parameters_);
    }

    //------------------------------------ Reset ------------------------------------
    void CRC::reset()
    {
        if (!is_valid())
        {
            crc_ = 0;
            return;
        }
        crc_ = parameters_.initial & mask_;
    }

    //------------------------------------ Bit reflection ------------------------------------
    uint64_t CRC::reflect(uint64_t value, uint8_t bits)
    {
        uint64_t result = 0;
        for (uint8_t i = 0; i < bits; ++i)
        {
            if (value & (uint64_t{1} << i))
                result |= uint64_t{1} << (bits - 1u - i);
        }
        return result;
    }

    uint64_t CRC::width_mask(uint8_t width)
    {
        return ::crc::width_mask(width);
    }

    //------------------------------------ Table generation (called once from constructor when enabled) ------------------------------------
    void CRC::generate_table()
    {
        // Build a 256-entry table using the bit-serial algorithm.
        // This is done only once per CRC instance.
        for (unsigned i = 0; i < 256; ++i)
        {
            table_[i] = process_byte_bit_serial(0, static_cast<uint8_t>(i));
        }
    }

    //------------------------------------ Core byte processing – bit-serial (always correct, used for table gen & fallback) ------------------------------------
    uint64_t CRC::process_byte_bit_serial(uint64_t crc, uint8_t byte) const
    {
        if (!parameters_.reflect_input)
        {
            // MSB-first
            for (uint8_t i = 0; i < 8; ++i)
            {
                const bool input_bit = ((byte >> (7u - i)) & 1u) != 0;
                const bool feedback  = ((crc & top_bit_) != 0) ^ input_bit;

                crc = (crc << 1u) & mask_;
                if (feedback)
                    crc ^= parameters_.polynomial;
            }
        }
        else
        {
            // LSB-first (register is kept in reflected domain)
            for (uint8_t i = 0; i < 8; ++i)
            {
                const bool input_bit = ((byte >> i) & 1u) != 0;
                const bool feedback  = ((crc & uint64_t{1}) != 0) ^ input_bit;

                crc >>= 1u;
                if (feedback)
                    crc ^= reflected_poly_;
            }
            crc &= mask_;
        }
        return crc;
    }

    //------------------------------------ Fast path – table driven ------------------------------------
    uint64_t CRC::process_byte_table(uint64_t crc, uint8_t byte) const
    {
        if (!parameters_.reflect_input)
        {
            // Classic MSB-first table algorithm
            // (crc << 8) ^ table[((crc >> (width-8)) ^ byte) & 0xFF]
            const unsigned shift = parameters_.width >= 8 ? parameters_.width - 8u : 0u;
            const uint8_t  idx   = static_cast<uint8_t>((crc >> shift) ^ byte);
            crc = ((crc << 8) ^ table_[idx]) & mask_;
        }
        else
        {
            // LSB-first table algorithm
            // table[(crc ^ byte) & 0xFF] ^ (crc >> 8)
            const uint8_t idx = static_cast<uint8_t>(crc ^ byte);
            crc = (table_[idx] ^ (crc >> 8)) & mask_;
        }
        return crc;
    }

    //------------------------------------ Public update methods ------------------------------------
    void CRC::update(uint8_t byte)
    {
        if (!is_valid())
            return;

        if (table_enabled_)
            crc_ = process_byte_table(crc_, byte);
        else
            crc_ = process_byte_bit_serial(crc_, byte);
    }

    void CRC::update(const uint8_t* data, std::size_t length)
    {
        if (!is_valid() || data == nullptr)
            return;

        if (table_enabled_)
        {
            for (std::size_t i = 0; i < length; ++i)
                crc_ = process_byte_table(crc_, data[i]);
        }
        else
        {
            for (std::size_t i = 0; i < length; ++i)
                crc_ = process_byte_bit_serial(crc_, data[i]);
        }
    }

    void CRC::update_bit(bool bit)
    {
        if (!is_valid())
            return;

        if (!parameters_.reflect_input)
        {
            const bool feedback = ((crc_ & top_bit_) != 0) ^ bit;
            crc_ = (crc_ << 1u) & mask_;
            if (feedback)
                crc_ ^= parameters_.polynomial;
        }
        else
        {
            const bool feedback = ((crc_ & uint64_t{1}) != 0) ^ bit;
            crc_ >>= 1u;
            if (feedback)
                crc_ ^= reflected_poly_;
            crc_ &= mask_;
        }
    }

    //------------------------------------ Finalize ------------------------------------
    uint64_t CRC::finalize() const
    {
        if (!is_valid())
            return 0;

        uint64_t result = crc_;

        // When the internal register is kept in the reflected domain
        // (reflect_input == true) but the user requested non-reflected output
        // (or vice-versa), we must reflect the register once at the end.
        if (parameters_.reflect_input != parameters_.reflect_output)
            result = reflect(result, parameters_.width);

        result ^= parameters_.xor_output;
        return result & mask_;
    }

    //------------------------------------ One-shot calculate / verify ------------------------------------
    uint64_t CRC::calculate(const uint8_t* data, std::size_t length)
    {
        reset();

        if (!is_valid())
            return 0;

        if (data == nullptr && length > 0)
            return 0;

        update(data, length);
        return finalize();
    }

    bool CRC::verify(const uint8_t* data, std::size_t length, uint64_t expected) const
    {
        if (!is_valid())
            return false;

        if (data == nullptr && length > 0)
            return false;

        CRC tmp(parameters_, table_enabled_);
        tmp.update(data, length);
        return tmp.finalize() == expected;
    }

    //------------------------------------ Metadata ------------------------------------
    std::size_t CRC::width() const
    {
        return parameters_.width;
    }

    std::size_t CRC::size_bytes() const
    {
        return (static_cast<std::size_t>(parameters_.width) + 7u) / 8u;
    }

    const Parameters& CRC::parameters() const
    {
        return parameters_;
    }

} // namespace crc
