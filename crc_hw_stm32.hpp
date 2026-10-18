// Hardware CRC acceleration for STM32 microcontrollers
//
// Supports the classic STM32 CRC unit (CRC-32/IEEE) present on
// F0/F1/F2/F3/F4/F7/L0/L1/L4/G0/G4/H7 ... families.
//
// Usage:
//   #define CRC_STM32_USE_HAL   // optional – use HAL macros if available
//   #include "crc/crc_hw_stm32.hpp"
//
//   crc::STM32_CRC32 hw;
//   hw.reset();
//   hw.update(data, len);
//   uint32_t r = hw.finalize();

#pragma once

#include "crc_types.hpp"
#include "crc.hpp"               // software fallback

#include <cstddef>
#include <cstdint>

//-------------------------------------- STM32 targets --------------------------------------
#if defined(STM32F0) || defined(STM32F1) || defined(STM32F2) || defined(STM32F3) \
 || defined(STM32F4) || defined(STM32F7) || defined(STM32L0) || defined(STM32L1) \
 || defined(STM32L4) || defined(STM32L5) || defined(STM32G0) || defined(STM32G4) \
 || defined(STM32H7) || defined(STM32WB) || defined(STM32WL) \
 || defined(STM32F042x6) || defined(STM32F103xB) || defined(STM32F407xx) \
 || defined(STM32F767xx) || defined(STM32L476xx) || defined(STM32G474xx) \
 || defined(STM32H743xx)
#  define CRC_STM32_TARGET 1
#else
#  define CRC_STM32_TARGET 0
#endif

#if CRC_STM32_TARGET
// Minimal register definitions so the header works even without full CMSIS
// (if the real CMSIS headers are present they will override these).
#  ifndef CRC_BASE
#    if defined(STM32H7)
#      define CRC_BASE  0x58024C00UL
#    else
#      define CRC_BASE  0x40023000UL          // most F/L/G series
#    endif
#  endif

struct CRC_TypeDef_Minimal
{
    volatile uint32_t DR;          // Data register
    volatile uint32_t IDR;         // Independent data register
    volatile uint32_t CR;          // Control register
    volatile uint32_t RESERVED0;
    volatile uint32_t INIT;        // Initial CRC value (newer families)
    volatile uint32_t POL;         // Polynomial (newer families)
};

#  ifndef CRC
#    define CRC  ((CRC_TypeDef_Minimal*)CRC_BASE)
#  endif

// Control register bits
#  ifndef CRC_CR_RESET
#    define CRC_CR_RESET   (1u << 0)
#  endif
#  ifndef CRC_CR_REV_IN
#    define CRC_CR_REV_IN  (3u << 5)         // full bit & byte reverse
#  endif
#  ifndef CRC_CR_REV_OUT
#    define CRC_CR_REV_OUT (1u << 7)
#  endif
#endif // CRC_STM32_TARGET

namespace crc
{
    //-------------------------------------- STM32 hardware CRC-32 (IEEE) engine --------------------------------------
    // This class uses the hardware unit when available and automatically
    // falls back to the software implementation otherwise.
    // The public interface is identical to a normal CRC object for CRC-32/IEEE.

    class STM32_CRC32
    {
        public:
            STM32_CRC32(): hw_available_(false), sw_(CRC32_IEEE, true)          // software fallback always ready
            {
                #if CRC_STM32_TARGET
                    // Enable CRC clock is the responsibility of the user
                    // (usually done once in the system init / HAL_CRC_MspInit).
                    // We only reset and configure the unit here.
                    hw_available_ = true;
                    configure_hw();
                #endif
                    reset();
            }

            bool hardware_active() const 
            { 
                return hw_available_; 
            }

            void reset()
            {
                #if CRC_STM32_TARGET
                    if (hw_available_)
                    {
                        CRC->CR |= CRC_CR_RESET;
                        // On older STM32 the reset also loads the fixed init value 0xFFFFFFFF
                        return;
                    }
                #endif
                    sw_.reset();
            }

            void update(uint8_t byte)
            {
                #if CRC_STM32_TARGET
                    if (hw_available_)
                    {
                        // Feed one byte (left-aligned in the 32-bit register for classic unit)
                        *reinterpret_cast<volatile uint8_t*>(&CRC->DR) = byte;
                        return;
                    }
                #endif
                    sw_.update(byte);
            }

            void update(const uint8_t* data, std::size_t length)
            {
                if (data == nullptr) return;

                #if CRC_STM32_TARGET
                    if (hw_available_)
                    {
                        // Fast path: write 32-bit words when possible
                        while (length >= 4)
                        {
                            uint32_t w;
                            // little-endian load (STM32 is LE)
                            w  = data[0];
                            w |= static_cast<uint32_t>(data[1]) << 8;
                            w |= static_cast<uint32_t>(data[2]) << 16;
                            w |= static_cast<uint32_t>(data[3]) << 24;
                            CRC->DR = w;
                            data   += 4;
                            length -= 4;
                        }
                        // remaining bytes
                        while (length--)
                            *reinterpret_cast<volatile uint8_t*>(&CRC->DR) = *data++;
                        return;
                    }
                #endif
                    sw_.update(data, length);
            }

            void update(const void* data, std::size_t length)
            {
                update(static_cast<const uint8_t*>(data), length);
            }

            uint32_t finalize() const
            {
                #if CRC_STM32_TARGET
                    if (hw_available_)
                    {
                        // Hardware already applies the IEEE reflection & final XOR
                        // when REV_IN / REV_OUT are configured.
                        return CRC->DR;
                    }
                #endif
                    return static_cast<uint32_t>(sw_.finalize());
            }

            uint32_t calculate(const uint8_t* data, std::size_t length)
            {
                reset();
                update(data, length);
                return finalize();
            }

            bool verify(const uint8_t* data, std::size_t length, uint32_t expected)
            {
                return calculate(data, length) == expected;
            }

        private:
            #if CRC_STM32_TARGET
                void configure_hw()
                {
                    // Reset the unit
                    CRC->CR = CRC_CR_RESET;

                    // Configure for CRC-32/IEEE (reflected input & output)
                    // This matches the classic Ethernet / ZIP / PNG polynomial.
                    // On very old STM32 (F1) REV_IN/REV_OUT bits do not exist;
                    // the hardware still produces the IEEE result when data is
                    // fed in reverse order, but for simplicity we set the bits
                    // when they are present.
                    CRC->CR = CRC_CR_REV_IN | CRC_CR_REV_OUT;

                    // On programmable units set the standard poly & init
                    #  ifdef CRC_POL
                            CRC->POL  = 0x04C11DB7u;
                            CRC->INIT = 0xFFFFFFFFu;
                    #  endif
                }
            #endif

            bool hw_available_;
            CRC  sw_;          // software fallback (always present)
    };

    //======================================= Helper: prefer hardware when possible, otherwise use software profile =======================================

    inline uint32_t crc32_ieee_fast(const uint8_t* data, std::size_t length)
    {
        #if CRC_STM32_TARGET
            STM32_CRC32 hw;
            return hw.calculate(data, length);
        #else
            return static_cast<uint32_t>(CRC(CRC32_IEEE).calculate(data, length));
        #endif
    }

} // namespace crc
