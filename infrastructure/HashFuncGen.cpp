#include "infrastructure/HashFuncGen.h"

#include <stdexcept>

HashFuncGen::HashFuncGen(std::uint64_t seed, std::size_t k)
    : rng_(seed) {
    if (k == 0) throw std::invalid_argument("k must be >= 1");
    coeffs_.resize(k);

    std::uniform_int_distribution<std::uint64_t> dist(0, P - 1);
    for (std::size_t i = 0; i < k; ++i) {
        coeffs_[i] = dist(rng_);
    }
}

std::uint32_t HashFuncGen::operator()(std::string_view s) const {
    const std::uint64_t x = fnv1a64(s) % P;
    const std::uint64_t y = poly_eval_modP(x);
    return static_cast<std::uint32_t>(y);
}

std::uint64_t HashFuncGen::fnv1a64(std::string_view s) {
    std::uint64_t h = 14695981039346656037ULL;
    for (unsigned char c: s) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ULL;
    }
    return h;
}

std::uint64_t HashFuncGen::mul_modP(std::uint64_t a, std::uint64_t b) {
    const unsigned __int128 prod = static_cast<unsigned __int128>(a) * b;
    return static_cast<std::uint64_t>(prod % P);
}

std::uint64_t HashFuncGen::poly_eval_modP(std::uint64_t x) const {
    // (((a_{k-1})*x + a_{k-2})*x + ... + a0) mod P
    std::uint64_t res = 0;
    for (std::size_t i = coeffs_.size(); i-- > 0;) {
        res = (mul_modP(res, x) + coeffs_[i]) % P;
    }
    return res;
}
