#pragma once

#include <string>
#include <span>
#include <vector>
#include <ranges>
#include <format>

namespace RegexHelpers {
	inline std::string_view NCG(std::string_view str) { return std::format("(?:{})", str); }
	inline std::string_view CG(std::string_view str) { return std::format("({})", str); }
	inline std::string_view KStar(std::string_view str, bool greedy=true) { return std::format("(?:{})*{}", str, greedy ? "" : "?"); }
	inline std::string_view KPlus(std::string_view str, bool greedy=true) { return std::format("(?:{})+{}", str, greedy ? "" : "?"); }
	inline std::string_view QUANT(std::string_view str, int n) { return std::format("(?:{}){{ {} }}", str, n); }
	inline std::string_view OR(std::span<const std::string_view> strs) {
		if (strs.empty()) return "";
		return NCG(strs
			| std::views::transform(NCG)
			| std::views::join_with(std::string_view("|"))
			| std::ranges::to<std::string>());
	}
	inline std::string_view AND(std::span<const std::string_view> strs) {
		if (strs.empty()) return "";
		return NCG(strs
			| std::views::transform(NCG)
			| std::views::join_with(std::string_view(""))
			| std::ranges::to<std::string>());
	}
};
