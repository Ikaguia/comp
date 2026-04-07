#pragma once

#include <regex>
#include <vector>
#include <string>
#include <string_view>

struct Token {
	using Num = std::variant<int, float>;
	enum struct TYPE : size_t { OPINC = 0, OPDEC, OPSUM, OPSUB, OPMUL, OPDIV, OPREM, OPEQ, OPNEQ, OPASGN, PAOPEN, PACLOSE, NUMREAL, NUMINT, BOOLTRUE, BOOLFALSE, INVALID, COUNT };
	enum struct CATEGORY : size_t { NUMERIC = 0, BOOLEAN, OPERATION, PARENTHESIS, INVALID, COUNT };
	static const std::array<TYPE, static_cast<size_t>(TYPE::COUNT)> TYPES;
	struct Type {
		CATEGORY category;
		std::string_view name, regex_str;
		bool includeVal;
		TYPE type;
		std::regex regex;
	};

	static const std::array<Type, static_cast<size_t>(TYPE::COUNT)> types;
	static const std::string regex_str_all;
	static const std::regex regex_all;

	TYPE type;
	std::string str;
	Num val;

	Token();
	Token(TYPE t);
	Token(TYPE t, std::string s);
	Token(std::string s);
	Token(Num v);
	const Type& getType() const;
	std::string name() const;
	std::string_view regex_str() const;
	std::regex regex() const;

	static const Type& getType(TYPE t);
	static std::string_view name(TYPE t);
	static std::string_view regex_str(TYPE t);
	static std::regex regex(TYPE t);

	static std::vector<Token> tokenize(const std::string& s);
private:
	void _val();
};

void printTokens(std::span<Token> tks);
