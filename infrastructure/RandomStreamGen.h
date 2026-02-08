#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <string_view>
#include <vector>

class RandomStreamGen {
public:
    struct Params {
        std::size_t n = 100000;
        std::size_t max_len = 30;
        std::uint64_t seed = 1234567;
    };

    explicit RandomStreamGen(Params p);

    void generate();

    [[nodiscard]] const std::vector<std::string> &stream() const;

    [[nodiscard]] std::size_t size() const;

    [[nodiscard]] std::vector<std::size_t> prefix_sizes_by_step_percent(std::size_t step_percent) const;

    [[nodiscard]] std::size_t prefix_size_for_percent(std::size_t percent) const;

private:
    static constexpr std::string_view alphabet();

    static constexpr std::size_t alphabet_size();

    Params params_;
    std::mt19937_64 rng_;
    std::vector<std::string> data_;
};
