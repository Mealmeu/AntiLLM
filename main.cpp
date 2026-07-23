#include "pe_vm/anti_llm.hpp"

#include <Windows.h>
#include <bcrypt.h>

#include <array>
#include <cstdint>
#include <string_view>
#include <iostream>
#include <string>

#pragma comment(lib, "bcrypt.lib")

bool VerifyRealFlag(std::string_view input)
{
    if (input.size() != 13U)
    {
        return false;
    }

    constexpr std::array<std::uint8_t, 32U> kDigestPartA =
    {
        0x71, 0x2C, 0xA9, 0x04, 0xD3, 0x88, 0x5E, 0xB1,
        0x37, 0xC0, 0x62, 0x19, 0xEE, 0x45, 0x9A, 0x0D,
        0x53, 0xF6, 0x20, 0x8B, 0x14, 0xAF, 0x69, 0xD2,
        0x3C, 0x7B, 0x91, 0x06, 0xE8, 0x5D, 0x42, 0xB7
    };

    constexpr std::array<std::uint8_t, 32U> kDigestPartB =
    {
        0xC5, 0xCC, 0x8A, 0x5D, 0xB7, 0x89, 0xCD, 0x46,
        0x55, 0xF6, 0x3D, 0xBB, 0x0A, 0x54, 0xC0, 0x26,
        0x3A, 0x9D, 0xE7, 0xE6, 0xC4, 0xE8, 0xC0, 0x90,
        0x9E, 0xA7, 0x02, 0x30, 0xFA, 0x1E, 0x43, 0x79
    };

    BCRYPT_ALG_HANDLE algorithm = nullptr;

    const NTSTATUS openStatus = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0U);

    if (!BCRYPT_SUCCESS(openStatus))
    {
        return false;
    }

    std::array<std::uint8_t, 32U> digest{};

    const NTSTATUS hashStatus = BCryptHash(algorithm, nullptr, 0U, reinterpret_cast<PUCHAR>(const_cast<char*>(input.data())), static_cast<ULONG>(input.size()), digest.data(), static_cast<ULONG>(digest.size()));

    BCryptCloseAlgorithmProvider(algorithm, 0U);

    if (!BCRYPT_SUCCESS(hashStatus))
    {
        return false;
    }

    std::uint8_t difference = 0U;

    for (std::size_t index = 0U; index < digest.size(); ++index)
    {
        const std::uint8_t expected = kDigestPartA[index] ^ kDigestPartB[index];
        difference |= digest[index] ^ expected;
    }

    return difference == 0U;
}

int main()
{
    std::string input;

    std::cout << enstr("Flag: ");
    std::cin >> input;

    volatile std::uint32_t decoyAnchor = anlm::AnchorDecoys();
    volatile std::uint32_t mazeResult = anlm::MazeMix(decoyAnchor ^ static_cast<std::uint32_t>(input.size()));

    if (anlm::IsAnalysisEnvironment())
    {
        anlm::FakeVerify(input);
        return 0;
    }

    const bool valid = VerifyRealFlag(input);

    if (valid)
    {
        std::cout << enstr("Correct\n");
    }
    else
    {
        std::cout << enstr("Wrong\n");
    }

    static_cast<void>(mazeResult);
    return valid ? 0 : 1;
}
