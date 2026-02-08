#include "hll/HyperLogLog.h"

#include <bit>
#include <cmath>
#include <stdexcept>

HyperLogLog::HyperLogLog(std::uint8_t B)
    : B_(B) {
    if (B_ == 0 || B_ >= 32) {
        throw std::invalid_argument("B must be in [1..31]");
    }
    m_ = static_cast<std::size_t>(1ULL) << B_;
    reg_.assign(m_, 0);
}

void HyperLogLog::reset() {
    std::fill(reg_.begin(), reg_.end(), 0);
}

void HyperLogLog::add(std::uint32_t hv) {
    const std::uint32_t j = B_ == 32 ? 0u : hv >> (32 - B_);

    const std::uint32_t w = B_ == 32 ? 0u : hv << B_;

    std::uint8_t rho = 0;
    if (w == 0) {
        rho = static_cast<std::uint8_t>(32 - B_ + 1);
    } else {
        rho = static_cast<std::uint8_t>(std::countl_zero(w) + 1);
    }

    std::uint8_t &cell = reg_[static_cast<std::size_t>(j)];
    if (rho > cell) cell = rho;
}

double HyperLogLog::alpha(std::size_t m) {
    if (m == 16) return 0.673;
    if (m == 32) return 0.697;
    if (m == 64) return 0.709;
    return 0.7213 / (1.0 + 1.079 / static_cast<double>(m));
}

double HyperLogLog::estimate() const {
    double Z = 0.0;
    std::size_t V = 0;

    for (std::uint8_t v: reg_) {
        if (v == 0) ++V;
        Z += std::ldexp(1.0, -static_cast<int>(v));
    }

    const auto m = static_cast<double>(m_);
    double E = alpha(m_) * (m * m) / Z;

    if (E <= 2.5 * m && V > 0) {
        E = m * std::log(m / static_cast<double>(V));
        return E;
    }

    constexpr double TWO_32 = 4294967296.0;
    if (E > (TWO_32 / 30.0)) {
        E = -TWO_32 * std::log(1.0 - (E / TWO_32));
    }

    return E;
}
