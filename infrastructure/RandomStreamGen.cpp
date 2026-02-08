#include "infrastructure/RandomStreamGen.h"

#include <algorithm>
#include <stdexcept>

RandomStreamGen::RandomStreamGen(Params p)
    : params_(p), rng_(p.seed) {
    if (params_.max_len == 0 || params_.max_len > 30) {
        throw std::invalid_argument("max_len must be in [1..30]");
    }
    data_.reserve(params_.n);
}

void RandomStreamGen::generate() {
    data_.clear();
    data_.reserve(params_.n);

    std::uniform_int_distribution<int> len_dist(1, static_cast<int>(params_.max_len));
    std::uniform_int_distribution<int> char_dist(0, static_cast<int>(alphabet_size() - 1));

    for (std::size_t i = 0; i < params_.n; ++i) {
        const int len = len_dist(rng_);
        std::string s;
        s.resize(static_cast<std::size_t>(len));
        for (int j = 0; j < len; ++j) {
            s[static_cast<std::size_t>(j)] =
                    alphabet()[static_cast<std::size_t>(char_dist(rng_))];
        }
        data_.push_back(std::move(s));
    }
}

const std::vector<std::string> &RandomStreamGen::stream() const {
    return data_;
}

std::size_t RandomStreamGen::size() const {
    return data_.size();
}

std::vector<std::size_t> RandomStreamGen::prefix_sizes_by_step_percent(std::size_t step_percent) const {
    if (step_percent == 0 || step_percent > 100) {
        throw std::invalid_argument("step_percent must be in [1..100]");
    }
    if (data_.empty()) {
        throw std::runtime_error("stream is empty: call generate() first");
    }

    std::vector<std::size_t> res;
    for (std::size_t p = step_percent; p < 100; p += step_percent) {
        res.push_back(prefix_size_for_percent(p));
    }
    res.push_back(prefix_size_for_percent(100));
    return res;
}

std::size_t RandomStreamGen::prefix_size_for_percent(std::size_t percent) const {
    if (percent > 100) {
        throw std::invalid_argument("percent must be in [0..100]");
    }
    const std::size_t n = data_.size();
    return (n * percent) / 100;
}

constexpr std::string_view RandomStreamGen::alphabet() {
    return "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789-";
}

constexpr std::size_t RandomStreamGen::alphabet_size() {
    return alphabet().size();
}
