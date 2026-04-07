#include <lexic.hpp>

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

using Type = Token::Type;
using TYPE = Token::TYPE;
using CATEGORY = Token::CATEGORY;

const std::array<TYPE, static_cast<size_t>(TYPE::COUNT)> Token::TYPES = {
	TYPE::OPINC,
	TYPE::OPDEC,
	TYPE::OPSUM,
	TYPE::OPSUB,
	TYPE::OPMUL,
	TYPE::OPDIV,
	TYPE::OPREM,
	TYPE::OPEQ,
	TYPE::OPNEQ,
	TYPE::OPASGN,
	TYPE::PAOPEN,
	TYPE::PACLOSE,
	TYPE::NUMREAL,
	TYPE::NUMINT,
	TYPE::BOOLTRUE,
	TYPE::BOOLFALSE,
	TYPE::INVALID,
};
const std::array<Type, static_cast<size_t>(TYPE::COUNT)> Token::types = [] {
	std::array<Type, static_cast<size_t>(TYPE::COUNT)> ts {
		Type{ CATEGORY::OPERATION, "OpInc", R"([+]{2})", false, TYPE::OPINC, {} },
		Type{ CATEGORY::OPERATION, "OpDec", R"([-]{2})", false, TYPE::OPDEC, {} },
		Type{ CATEGORY::OPERATION, "OpSum", R"([+])", false, TYPE::OPSUM, {} },
		Type{ CATEGORY::OPERATION, "OpSub", R"([-])", false, TYPE::OPSUB, {} },
		Type{ CATEGORY::OPERATION, "OpMul", R"([*])", false, TYPE::OPMUL, {} },
		Type{ CATEGORY::OPERATION, "OpDiv", R"([/])", false, TYPE::OPDIV, {} },
		Type{ CATEGORY::OPERATION, "OpRem", R"([%])", false, TYPE::OPREM, {} },
		Type{ CATEGORY::OPERATION, "OpEquals", R"([=]{2})", false, TYPE::OPEQ, {} },
		Type{ CATEGORY::OPERATION, "OpNotEquals", R"([!][=])", false, TYPE::OPNEQ, {} },
		Type{ CATEGORY::OPERATION, "OpAssign", R"([=])", false, TYPE::OPASGN, {} },
		Type{ CATEGORY::PARENTHESIS, "ParOpen", R"([(])", false, TYPE::PAOPEN, {} },
		Type{ CATEGORY::PARENTHESIS, "ParClose", R"([)])", false, TYPE::PACLOSE, {} },
		Type{ CATEGORY::NUMERIC, "NumReal", R"(\d+\.\d+)", true, TYPE::NUMREAL, {} },
		Type{ CATEGORY::NUMERIC, "NumInt", R"(\d+)", true, TYPE::NUMINT, {} },
		Type{ CATEGORY::NUMERIC, "BoolTrue", R"([Tt]rue)", false, TYPE::BOOLTRUE, {} },
		Type{ CATEGORY::NUMERIC, "BoolFalse", R"([Ff]alse)", false, TYPE::BOOLFALSE, {} },
		Type{ CATEGORY::INVALID, "Invalid", "", false, TYPE::INVALID, {} },
	};
	for (TYPE TP : TYPES) {
		auto& tp = ts[static_cast<size_t>(TP)];
		if (tp.type != TP) {
			std::println("Mismatched Token::TYPE on Token::types for TP {} tp {}", static_cast<size_t>(TP), tp.name);
			throw std::runtime_error(std::format("Mismatched Token::TYPE on Token::types for TP {} tp {}", static_cast<size_t>(TP), tp.name));
		}
		tp.regex = std::regex{static_cast<std::string>(tp.regex_str)};
	}
	return ts;
}();
const std::string Token::regex_str_all = Token::types
	| std::views::filter([](auto tp) { return tp.category != CATEGORY::INVALID; })
	| std::views::transform([](auto tp) { return std::format("({})", tp.regex_str); })
	| std::views::join_with('|')
	| std::ranges::to<std::string>();
const std::regex Token::regex_all{Token::regex_str_all};

Token::Token() : type{TYPE::INVALID} {}
Token::Token(TYPE t) : type{t} { _val(); }
Token::Token(TYPE t, std::string s) : type{t}, str{s} { _val(); }
Token::Token(std::string s) : str{s} {
	std::smatch match;
	std::regex_match(str, match, regex_all);
	size_t j = 1;
	for (size_t i = 0; i < static_cast<size_t>(TYPE::COUNT); i++) {
		if (types[i].category == CATEGORY::INVALID) continue;
		if (match[j++].matched) type = static_cast<TYPE>(i);
	}
	_val();
}
Token::Token(Num v) : val{v} {
	std::visit([this](auto&& arg) {
		this->str = std::format("{}", arg);
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, int>) this->type = TYPE::NUMINT;
		else if constexpr (std::is_same_v<T, float>) this->type = TYPE::NUMREAL;
		else this->type = TYPE::INVALID;
	}, v);
}
const Type& Token::getType() const { return types[static_cast<size_t>(type)]; }
std::string Token::name() const {
	auto tp = getType();
	return tp.includeVal ? std::format("{} ({})", std::string{tp.name}, str) : std::string{tp.name};
}
std::string_view Token::regex_str() const { return getType().regex_str; }
std::regex Token::regex() const { return getType().regex; }

const Type& Token::getType(TYPE t) { return types[static_cast<size_t>(t)]; }
std::string_view Token::name(TYPE t) { return getType(t).name; }
std::string_view Token::regex_str(TYPE t) { return getType(t).regex_str; }
std::regex Token::regex(TYPE t) { return getType(t).regex; }

std::vector<Token> Token::tokenize(const std::string& s) {
	std::vector<Token> out;
	auto tokensbegin = std::sregex_iterator(s.begin(), s.end(), regex_all);
	auto tokensend = std::sregex_iterator();
	for(auto iter = tokensbegin; iter != tokensend; iter++) {
		auto match = *iter;
		size_t j = 1;
		for (size_t i = 0; i < static_cast<size_t>(TYPE::COUNT); i++) {
			if (types[i].category == CATEGORY::INVALID) continue;
			if (match[j++].matched) {
				if (out.size() > 0 && TYPES[i] == TYPE::INVALID && out.back().type == TYPE::INVALID) out.back().str += match.str();
				else out.emplace_back(TYPES[i], match.str());
				break;
			}
		}
	}
	return out;
}
void Token::_val() {
	if (type == TYPE::NUMREAL) {
		float f;
		auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), f);
		if (ec == std::errc{}) val = f;
	} else if (type == TYPE::NUMINT) {
		int i;
		auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), i);
		if (ec == std::errc{}) val = i;
	}
}

void printTokens(std::span<Token> tks) {
	bool first = true;
	std::print("[ ");
	for (auto token : tks) {
		if (!first) std::print(", ");
		else first = false;
		std::print("{}", token.name());
	}
	std::println(" ]");
}
