#pragma once

#include <regex>
#include <vector>
#include <string>
#include <string_view>

#include <print>
#include <regex>
#include <map>
#include <set>
#include <iostream>
#include <ranges>
#include <algorithm>
#include <span>
#include <limits>
#include <variant>
#include <optional>
#include <format>
#include <functional>
#include <vector>
#include <string>
#include <string_view>
#include <charconv>
#include <array>
#include <stdexcept>


template <typename TYPE, TYPE TYPE_INVALID, size_t TYPE_COUNT>
struct Tokenizer {
	struct Type {
		TYPE type;
		std::string name, regex_str;
		bool includeStr;
		std::regex regex;
	};
	struct Token {
		TYPE type;
		std::string str;
	};
	std::array<Type, TYPE_COUNT> types;
	std::string regex_all_str;
	std::regex regex_all;

	Tokenizer(const std::array<Type, TYPE_COUNT>& tps) : types{tps} { init(); }
	std::vector<Token> tokenize(const std::string& s) const {
		std::vector<Token> out;
		auto matches_view = std::ranges::subrange(std::sregex_iterator(s.begin(), s.end(), regex_all), std::sregex_iterator());
		bool prev_invalid = false;
		for(const auto& match : matches_view) {
			for (const auto& [i, type] : std::views::enumerate(types)) {
				if (match[i+1].matched) {
					// merge consecutive invalid tokens
					bool is_invalid = type.type == TYPE_INVALID;
					if (is_invalid && prev_invalid) out.back().str += match.str();
					else out.emplace_back(type.type, match.str());
					prev_invalid = is_invalid;
					break;
				}
			}
		}
		return out;
	}
	void print(std::span<const Token> tks) const {
		auto formatted_tokens = tks 
			| std::views::transform([this](const auto& tk) { return token_name(tk); })
			| std::views::join_with(std::string_view(", "))
			| std::ranges::to<std::string>();

		std::println("[ {} ]", formatted_tokens);
	}
	std::string token_name(const Token& tk) const {
		const Type& type = types[static_cast<size_t>(tk.type)];
		return type.includeStr ? std::format("{} ({})", std::string{type.name}, tk.str) : std::string{type.name};
	}

private:
	void init() {
		for (auto [i, type] : std::views::enumerate(types)) {
			if (static_cast<long long int>(type.type) != i) {
				std::println("Mismatched Tokenizer::TYPE on Tokenizer::types for i {} type {}", i, type.name);
				throw std::runtime_error(std::format("Mismatched Tokenizer::TYPE on Tokenizer::types for i {} type {}", i, type.name));
			}
			type.regex = std::regex{type.regex_str};
		}
		regex_all_str = types
			| std::views::transform([](auto tp) { return std::format("({})", tp.regex_str); })
			| std::views::join_with('|')
			| std::ranges::to<std::string>();
		regex_all = std::regex{regex_all_str};
	}
};
