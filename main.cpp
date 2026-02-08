#include "infrastructure/HashFuncGen.h"
#include "infrastructure/RandomStreamGen.h"
#include "hll/HyperLogLog.h"
#include "experiments/OnlineStats.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <unordered_set>
#include <vector>

struct StringViewHash {
    std::size_t operator()(std::string_view s) const noexcept {
        std::uint64_t h = 14695981039346656037ULL;
        for (unsigned char c: s) {
            h ^= static_cast<std::uint64_t>(c);
            h *= 1099511628211ULL;
        }
        return h;
    }
};

static void write_single_csv(const std::string &path,
                             const std::vector<std::size_t> &t,
                             const std::vector<std::size_t> &F0,
                             const std::vector<double> &Nt) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot open file: " + path);
    out << "t,F0,Nt\n";
    out << std::fixed << std::setprecision(6);
    for (std::size_t i = 0; i < t.size(); ++i) {
        out << t[i] << "," << F0[i] << "," << Nt[i] << "\n";
    }
}

static void write_multi_csv(const std::string &path,
                            const std::vector<std::size_t> &t,
                            const std::vector<OnlineStats> &stats_F0,
                            const std::vector<OnlineStats> &stats_Nt) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot open file: " + path);
    out << "t,EF0,ENt,sigmaNt,relErrPercent\n";
    out << std::fixed << std::setprecision(6);
    for (std::size_t i = 0; i < t.size(); ++i) {
        const double EF0 = stats_F0[i].mean;
        const double ENt = stats_Nt[i].mean;
        const double sNt = stats_Nt[i].stddev();
        const double rel = EF0 > 0.0 ? 100.0 * (ENt - EF0) / EF0 : 0.0;
        out << t[i] << "," << EF0 << "," << ENt << "," << sNt << "," << rel << "\n";
    }
}

int main() {
    constexpr std::size_t n = 60000;
    constexpr std::size_t step_percent = 5;
    constexpr std::uint8_t B = 12;
    constexpr std::size_t trials = 30;

    std::vector<std::size_t> prefix_sizes;
    for (std::size_t p = step_percent; p < 100; p += step_percent) {
        prefix_sizes.push_back((n * p) / 100);
    }
    prefix_sizes.push_back(n);

    std::vector<OnlineStats> stats_F0(prefix_sizes.size());
    std::vector<OnlineStats> stats_Nt(prefix_sizes.size());

    constexpr std::uint64_t hash_seed = 1337;

    std::filesystem::create_directories("results");

    std::vector<std::size_t> demo_t;
    std::vector<std::size_t> demo_F0;
    std::vector<double> demo_Nt;
    demo_t.reserve(prefix_sizes.size());
    demo_F0.reserve(prefix_sizes.size());
    demo_Nt.reserve(prefix_sizes.size()); {
        RandomStreamGen gen({.n = n, .max_len = 30, .seed = 42});
        gen.generate();

        HashFuncGen h(hash_seed, /*k=*/4);
        HyperLogLog hll(B);

        std::unordered_set<std::string_view, StringViewHash> seen;
        seen.reserve(n);

        const auto &s = gen.stream();

        std::cout << "SINGLE RUN (demo)\n";
        std::cout << "n=" << n << ", step=" << step_percent << "%, B=" << int(B)
                << ", m=" << hll.m() << "\n\n";

        std::cout << std::left
                << std::setw(10) << "t"
                << std::setw(15) << "F0(t)"
                << std::setw(15) << "N_t(HLL)"
                << "\n";
        std::cout << std::string(40, '-') << "\n";

        std::size_t next_idx = 0;
        std::size_t next_t = prefix_sizes[next_idx];

        for (std::size_t i = 0; i < s.size(); ++i) {
            std::string_view sv(s[i]);
            seen.insert(sv);
            hll.add(h(sv));

            const std::size_t t = i + 1;
            if (t == next_t) {
                const std::size_t F0 = seen.size();
                const double Nt = hll.estimate();

                demo_t.push_back(t);
                demo_F0.push_back(F0);
                demo_Nt.push_back(Nt);

                std::cout << std::left
                        << std::setw(10) << t
                        << std::setw(15) << F0
                        << std::setw(15) << static_cast<std::uint64_t>(Nt)
                        << "\n";

                ++next_idx;
                if (next_idx >= prefix_sizes.size()) break;
                next_t = prefix_sizes[next_idx];
            }
        }
        std::cout << "\n";
    }

    write_single_csv("results/single_run.csv", demo_t, demo_F0, demo_Nt);

    for (std::size_t trial = 0; trial < trials; ++trial) {
        RandomStreamGen gen({.n = n, .max_len = 30, .seed = 1000 + trial});
        gen.generate();

        HashFuncGen h(hash_seed, /*k=*/4);
        HyperLogLog hll(B);

        std::unordered_set<std::string_view, StringViewHash> seen;
        seen.reserve(n);

        const auto &s = gen.stream();

        std::size_t next_idx = 0;
        std::size_t next_t = prefix_sizes[next_idx];

        for (std::size_t i = 0; i < s.size(); ++i) {
            std::string_view sv(s[i]);
            seen.insert(sv);
            hll.add(h(sv));

            const std::size_t t = i + 1;
            if (t == next_t) {
                stats_F0[next_idx].add(static_cast<double>(seen.size()));
                stats_Nt[next_idx].add(hll.estimate());

                ++next_idx;
                if (next_idx >= prefix_sizes.size()) break;
                next_t = prefix_sizes[next_idx];
            }
        }
    }

    write_multi_csv("results/multi_run.csv", prefix_sizes, stats_F0, stats_Nt);

    std::cout << "CSV saved:\n"
            << "  results/single_run.csv\n"
            << "  results/multi_run.csv\n";

    return 0;
}
