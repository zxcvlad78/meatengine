#pragma once
#include <chrono>
#include <utility>

namespace me {
	template <typename F, typename... Args>
	inline double bench_func(F&& func, Args&&... args) {
		auto start = std::chrono::high_resolution_clock::now();
		std::forward<F>(func)(std::forward<Args>(args)...);
		auto end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double, std::milli> elapsed = end - start;
		return elapsed.count();
	}
}
