# Embedded CRC Library

**Lightweight, efficient and flexible CRC library designed for embedded systems (MCUs).**

کتابخانه سبک، بهینه و انعطاف‌پذیر CRC مخصوص سیستم‌های نهفته (میکروکنترلرها)

### Overview

This is a modern C++ CRC library written with real embedded constraints in mind:

- **Streaming API** – process data chunk-by-chunk (perfect for DMA / UART / SPI)
- **Optional lookup table** – high speed when RAM is available, pure bit-serial when RAM is tight
- **Template version (`StaticCRC`)** – zero runtime parameter overhead, minimal RAM
- **Rich helpers** – `append_crc()`, `verify_packet()`, endian-aware CRC read/write
- **No dynamic allocation, no exceptions**
- Supports CRC widths from **1 to 64 bits**
- Includes common industrial profiles (Modbus, CRC-32/IEEE, Castagnoli, MAXIM, XMODEM, …)

### Features

| Feature                        | Description                                      |
|--------------------------------|--------------------------------------------------|
| Runtime `CRC` class            | Flexible, parameters can be chosen at runtime    |
| `StaticCRC<...>`               | Compile-time / template version, lowest overhead |
| Table-driven & bit-serial      | Choose speed vs RAM                              |
| Streaming (`update`/`finalize`)| Ideal for limited RAM and continuous data        |
| Helper functions               | Append CRC, verify packets, residual support     |
| Standard profiles              | CRC-8/16/32/64 ready to use                      |

### Quick Start

```cpp
#include "crc.hpp"
#include "crc_profiles.hpp"

// One-shot
crc::CRC engine(crc::CRC16_MODBUS);
uint16_t result = static_cast<uint16_t>(engine.calculate(data, length));

// Streaming (recommended for MCU)
crc::CRC crc(crc::CRC32_IEEE);
crc.update(header, header_len);
crc.update(payload, payload_len);
uint32_t crc_val = static_cast<uint32_t>(crc.finalize());
```

#### Template version (lowest RAM)

```cpp
#include "crc_static.hpp"

crc::StaticCRC16_MODBUS eng;
uint16_t result = eng.calculate(data, len);
```

#### Packet helpers

```cpp
#include "crc_helpers.hpp"

// Append CRC at the end of buffer
std::size_t total = crc::append_crc(buffer, data_len, capacity,
                                    crc::CRC16_MODBUS, crc::Endian::Little);

// Verify a received packet
bool ok = crc::verify_packet(packet, total_len, crc::CRC16_MODBUS);
```

### File Structure

```
crc/
├── crc_types.hpp        # Core types & Parameters
├── crc.hpp / crc.cpp    # Main runtime CRC engine
├── crc_profiles.hpp/.cpp# Standard CRC profiles
├── crc_static.hpp       # Template / StaticCRC version
├── crc_helpers.hpp      # append_crc, verify_packet, ...
├── example_usage.cpp    # Simple examples
└── example_advanced.cpp # Advanced usage demonstration
```

### Building

```bash
g++ -std=c++14 -O2 -I. your_app.cpp crc.cpp crc_profiles.cpp -o your_app
```

C++14 or later is required.

### License

This project is released as open source.  
Feel free to use it in commercial and non-commercial embedded projects.

---

این یک کتابخانه CRC مدرن به زبان C++ است که با در نظر گرفتن محدودیت‌های واقعی سیستم‌های نهفته نوشته شده:

- **رابط Streaming** – پردازش داده به‌صورت تکه‌تکه (مناسب DMA، UART، SPI)
- **جدول Lookup اختیاری** – سرعت بالا وقتی RAM کافی است، حالت bit-serial وقتی حافظه محدود است
- **نسخه Template (`StaticCRC`)** – بدون سربار پارامتر در زمان اجرا و کمترین مصرف RAM
- **توابع کمکی قدرتمند** – `append_crc()`، `verify_packet()` و پشتیبانی از endian
- **بدون تخصیص دینامیک و بدون Exception**
- پشتیبانی از عرض CRC بین **۱ تا ۶۴ بیت**
- شامل پروفایل‌های صنعتی رایج (Modbus، CRC-32/IEEE، Castagnoli، MAXIM، XMODEM و ...)

### قابلیت‌ها

| قابلیت                         | توضیح                                            |
|--------------------------------|--------------------------------------------------|
| کلاس `CRC` (Runtime)           | انعطاف‌پذیر، پارامترها در زمان اجرا قابل انتخاب  |
| `StaticCRC<...>`               | نسخه Compile-time با کمترین مصرف منابع           |
| حالت Table و Bit-serial        | انتخاب بین سرعت و مصرف حافظه                     |
| Streaming (`update`/`finalize`)| ایده‌آل برای RAM محدود و داده پیوسته             |
| توابع کمکی                     | اضافه کردن CRC، بررسی صحت بسته و ...             |
| پروفایل‌های استاندارد          | CRC-8/16/32/64 آماده استفاده                     |

```cpp
#include "crc.hpp"
#include "crc_profiles.hpp"

// محاسبه یک‌مرحله‌ای
crc::CRC engine(crc::CRC16_MODBUS);
uint16_t result = static_cast<uint16_t>(engine.calculate(data, length));

// حالت Streaming (توصیه‌شده برای میکروکنترلر)
crc::CRC crc(crc::CRC32_IEEE);
crc.update(header, header_len);
crc.update(payload, payload_len);
uint32_t crc_val = static_cast<uint32_t>(crc.finalize());
```

#### نسخه Template (کمترین مصرف RAM)

```cpp
#include "crc_static.hpp"

crc::StaticCRC16_MODBUS eng;
uint16_t result = eng.calculate(data, len);
```

#### توابع کمکی بسته

```cpp
#include "crc_helpers.hpp"

// اضافه کردن CRC به انتهای بافر
std::size_t total = crc::append_crc(buffer, data_len, capacity,
                                    crc::CRC16_MODBUS, crc::Endian::Little);

// بررسی صحت بسته دریافتی
bool ok = crc::verify_packet(packet, total_len, crc::CRC16_MODBUS);
```

### ساختار فایل‌ها

```
crc/
├── crc_types.hpp        # انواع پایه و ساختار Parameters
├── crc.hpp / crc.cpp    # موتور اصلی Runtime
├── crc_profiles.hpp/.cpp# پروفایل‌های استاندارد
├── crc_static.hpp       # نسخه Template / StaticCRC
├── crc_helpers.hpp      # توابع کمکی (append و verify)
├── example_usage.cpp    # مثال‌های ساده
└── example_advanced.cpp # نمایش قابلیت‌های پیشرفته
```

### کامپایل

```bash
g++ -std=c++14 -O2 -I. your_app.cpp crc.cpp crc_profiles.cpp -o your_app
```

حداقل استاندارد مورد نیاز: **C++14**

### مجوز

این پروژه به‌صورت متن‌باز منتشر شده است.  
می‌توانید آزادانه در پروژه‌های تجاری و غیرتجاری Embedded از آن استفاده کنید.

---

**Designed for real embedded systems.**  
**طراحی‌شده برای سیستم‌های نهفته واقعی.**
