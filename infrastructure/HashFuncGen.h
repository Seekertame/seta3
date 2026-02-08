#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <string_view>
#include <vector>

class HashFuncGen {
public:
    explicit HashFuncGen(std::uint64_t seed = 1234567, std::size_t k = 4);

    std::uint32_t operator()(std::string_view s) const;

private:
    static constexpr std::uint64_t P = 4294967311ULL; // простое > 2^32

    static std::uint64_t fnv1a64(std::string_view s);

    static std::uint64_t mul_modP(std::uint64_t a, std::uint64_t b);

    [[nodiscard]] std::uint64_t poly_eval_modP(std::uint64_t x) const;

    std::mt19937_64 rng_;
    std::vector<std::uint64_t> coeffs_;
};
