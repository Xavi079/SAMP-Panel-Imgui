#pragma once
#include <Windows.h>
#include <cstdint>

class sampBase {
public:
    static std::uint32_t getSampBase() noexcept {
        return reinterpret_cast<std::uint32_t>(GetModuleHandleA("samp.dll"));
    }
};
