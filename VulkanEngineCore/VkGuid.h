#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cstdio>

#ifdef _WIN32
#include <objbase.h>
#include <combaseapi.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <uuid/uuid.h>
#include <Endian.h>
#elif defined(__ANDROID__) || defined(__linux__)
#include <cctype>
#include <cstdlib>
#endif

struct VkGuid
{
    uint32_t Data1 = 0;
    uint16_t Data2 = 0;
    uint16_t Data3 = 0;
    uint8_t  Data4[8] = {};

    VkGuid() = default;
    VkGuid(const VkGuid&) = default;
    VkGuid& operator=(const VkGuid&) = default;

    // From string: {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    explicit VkGuid(const std::string& s)
    {
        std::string str = s;
        if (!str.empty() && str.front() != '{') {
            str = "{" + str + "}";
        }

#ifdef _WIN32
        std::wstring ws(str.begin(), str.end());
        GUID guid{};
        if (FAILED(CLSIDFromString(ws.c_str(), &guid))) {
            throw std::runtime_error("Invalid GUID string");
        }
        Data1 = guid.Data1;
        Data2 = guid.Data2;
        Data3 = guid.Data3;
        std::memcpy(Data4, guid.Data4, 8);

#elif defined(__APPLE__)
        CFStringRef cf = CFStringCreateWithCString(nullptr, str.c_str(), kCFStringEncodingUTF8);
        if (!cf) throw std::runtime_error("CFString creation failed");
        CFUUIDRef uuid = CFUUIDCreateFromString(nullptr, cf);
        CFRelease(cf);
        if (!uuid) throw std::runtime_error("Invalid UUID");

        CFUUIDBytes b = CFUUIDGetUUIDBytes(uuid);
        CFRelease(uuid);

        Data1 = OSSwapBigToHostInt32(*(uint32_t*)&b.byte0);
        Data2 = OSSwapBigToHostInt16(*(uint16_t*)&b.byte4);
        Data3 = OSSwapBigToHostInt16(*(uint16_t*)&b.byte6);
        std::memcpy(Data4, &b.byte8, 8);

#else // Android / Linux
        std::string hex;
        for (char c : str) {
            if (c == '{' || c == '}' || c == '-') continue;
            if (std::isxdigit(c)) hex += std::toupper(c);
        }
        if (hex.size() != 32) throw std::runtime_error("GUID must be 32 hex digits");

        auto p = [&](size_t i) {
            return static_cast<uint8_t>(std::strtol(hex.substr(i, 2).c_str(), nullptr, 16));
            };
        Data1 = (p(0) << 24) | (p(2) << 16) | (p(4) << 8) | p(6);
        Data2 = (p(8) << 8) | p(10);
        Data3 = (p(12) << 8) | p(14);
        for (int i = 0; i < 8; ++i) Data4[i] = p(16 + i * 2);
#endif
    }

    // From Windows GUID (private helper)
#ifdef _WIN32
    explicit VkGuid(const GUID& guid)
    {
        Data1 = guid.Data1;
        Data2 = guid.Data2;
        Data3 = guid.Data3;
        std::memcpy(Data4, guid.Data4, 8);
    }
#endif

    static VkGuid Generate()
    {
        VkGuid g{};
        #ifdef _WIN32
            GUID guid;
            if (FAILED(CoCreateGuid(&guid))) throw std::runtime_error("CoCreateGuid failed");
            std::string guidStr = VkGuid(guid).ToString();
            guidStr.replace(guidStr.find('{'), 1, "");
            guidStr.replace(guidStr.find('}'), 1, "");
            return VkGuid(guid);
        #else
                // 16 random bytes
                uint8_t b[16];
        #if defined(__linux__) || defined(__ANDROID__)
                FILE* f = std::fopen("/dev/urandom", "rb");
                if (!f || std::fread(b, 1, 16, f) != 16)
                    throw std::runtime_error("urandom failed");
                if (f) std::fclose(f);
        #elif defined(__APPLE__)
                arc4random_buf(b, 16);
        #else
        #error "VkGuid::Generate: no CSPRNG"
        #endif
                // UUID v4
                b[6] = static_cast<uint8_t>((b[6] & 0x0F) | 0x40);
                b[8] = static_cast<uint8_t>((b[8] & 0x3F) | 0x80);

                g.Data1 = (uint32_t(b[0]) << 24) | (uint32_t(b[1]) << 16) | (uint32_t(b[2]) << 8) | uint32_t(b[3]);
                g.Data2 = uint16_t((uint16_t(b[4]) << 8) | b[5]);
                g.Data3 = uint16_t((uint16_t(b[6]) << 8) | b[7]);
                std::memcpy(g.Data4, b + 8, 8);
                return g;
        #endif
    }

    static VkGuid Empty() {
        VkGuid g;
        g.Data1 = 0;
        g.Data2 = 0;
        g.Data3 = 0;
        std::memset(g.Data4, 0, 8);
        return g;
    }

    std::string ToString() const
    {
        char buf[39] = {};
        std::snprintf(buf, sizeof(buf),
            "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            Data1, Data2, Data3,
            Data4[0], Data4[1], Data4[2], Data4[3],
            Data4[4], Data4[5], Data4[6], Data4[7]);
        return std::string(buf);
    }

    bool operator==(const VkGuid& o) const
    {
        return Data1 == o.Data1 &&
            Data2 == o.Data2 &&
            Data3 == o.Data3 &&
            std::memcmp(Data4, o.Data4, 8) == 0;
    }

    bool operator!=(const VkGuid& o) const { return !(*this == o); }
    friend struct std::hash<VkGuid>;
};

static_assert(sizeof(VkGuid) == 16, "VkGuid must be exactly 16 bytes");
static_assert(alignof(VkGuid) <= 4, "VkGuid alignment must be <= 4");

namespace std {
    template <>
    struct hash<VkGuid>
    {
        static constexpr uint64_t kFNVPrime = 1099511628211ULL;
        static constexpr uint64_t kFNVOffset = 14695981039346656037ULL;

        inline size_t operator()(const VkGuid& g) const noexcept
        {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&g);
            uint64_t h = kFNVOffset;
            for (int i = 0; i < 16; ++i) {
                h ^= static_cast<uint64_t>(bytes[i]);
                h *= kFNVPrime;
            }
            return static_cast<size_t>(h);
        }
    };
}