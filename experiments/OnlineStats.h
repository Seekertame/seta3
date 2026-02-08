#pragma once
#include <cstddef>
#include <cmath>

struct OnlineStats {
    std::size_t n = 0;
    double mean = 0.0;
    double m2 = 0.0;

    void add(double x) {
        ++n;
        const double delta = x - mean;
        mean += delta / static_cast<double>(n);
        const double delta2 = x - mean;
        m2 += delta * delta2;
    }

    [[nodiscard]] double variance() const {
        return n > 1 ? m2 / static_cast<double>(n - 1) : 0.0;
    }

    [[nodiscard]] double stddev() const {
        return std::sqrt(variance());
    }
};
