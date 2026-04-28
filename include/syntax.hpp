#pragma once

#include <variant>
#include <optional>
#include <vector>
#include <string>
#include <string_view>

#include <lexic.hpp>

struct AST {
	using TYPE = enum { NUMBER, BOOLEAN, LITERAL, PARENTHESIS, OP1, OP2, OP3, OP4, OP5, OP6, OP7, OP8, OP9, OP10, OP11, OP12, OP13, OP14, OP15, EXPRESSION, COUNT };
	static const std::array<TYPE, COUNT> TYPES;

	using Token = Lexic::LexicTokenizer::Token;
	using PatternElement = std::variant<Lexic::TYPE, TYPE>;
	using Pattern = std::vector<PatternElement>;
	struct Type {
		TYPE type;
		std::string name;
		std::vector<Pattern> patterns;
		enum struct Associativity { LTR, RTL } associativity;
	};
	static const std::array<Type, COUNT> types;

	struct MatchData {
		std::span<const Token> tks;
		std::map<std::tuple<int,int,TYPE>, std::optional<AST>> memoization;
		std::map<Lexic::TYPE,std::vector<int>> indexList;
	};

	static std::optional<AST> from_tokens(std::vector<Token> tks);
	static std::optional<AST> match(MatchData& in, int start, int end, TYPE type, int depth=0);

	std::string to_string() const;

	int string_sz() const;

	TYPE type;
	std::vector<std::variant<AST,Token>> children{};

	struct DrawNode {
		std::string text;
		int x = 0, y = 0;
		int width = 0;
		std::vector<std::unique_ptr<DrawNode>> children{};

		DrawNode(std::string t) : text(std::move(t)) {}
		DrawNode() = default;
	};

	void render() const;

private:
	std::unique_ptr<DrawNode> build_draw_tree() const;
	int calculate_layout(const std::unique_ptr<DrawNode>& node, int x, int y, int& max_y) const;
	void draw_to_canvas(const std::unique_ptr<DrawNode>& node, std::vector<std::string>& canvas) const;
};
