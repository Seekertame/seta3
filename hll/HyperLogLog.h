#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class HyperLogLog {
public:
    explicit HyperLogLog(std::uint8_t B);

    void reset();

    void add(std::uint32_t hash_value);

    [[nodiscard]] double estimate() const;

    [[nodiscard]] std::uint8_t B() const { return B_; }
    [[nodiscard]] std::size_t m() const { return m_; }

private:
    static double alpha(std::size_t m);

    std::uint8_t B_;
    std::size_t m_;
    std::vector<std::uint8_t> reg_;
};
