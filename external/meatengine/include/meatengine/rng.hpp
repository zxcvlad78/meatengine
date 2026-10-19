#pragma once
#include <random>

namespace me::rng {
    inline std::mt19937& rng() {
        static std::mt19937 gen(std::random_device{}());
        return gen;
    }

    template<typename T>
    T random(T min, T max) {
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_same_v<T, bool>) {
                std::bernoulli_distribution dist(0.5);
                return dist(rng());
            } else {
                using DistributionType = std::conditional_t<
                    sizeof(T) == 1,
                    std::conditional_t<std::is_signed_v<T>, int16_t, uint16_t>,
                    T
                >;

                std::uniform_int_distribution<DistributionType> dist(min, max);
                return static_cast<T>(dist(rng()));
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(rng());
        } else {
            static_assert(std::is_arithmetic_v<T>, "T must be arithmetic");
        }
    }
};