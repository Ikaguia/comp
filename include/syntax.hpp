#include <variant>
#include <optional>
#include <vector>
#include <string>
#include <string_view>

#include <lexic.hpp>

struct AST {
	using TYPE = enum { NUMBER, BOOLEAN, LITERAL, PARENTHESIS, OP1, OP2, OP3, OP4, OP5, OP6, OP7, OP8, OP9, OP10, OP11, OP12, OP13, OP14, OP15, EXPRESSION, COUNT };
	static const std::array<TYPE, COUNT> TYPES;

	using Pattern = std::vector<std::variant<Token::TYPE, TYPE>>;
	struct Type {
		TYPE type;
		std::string name;
		std::vector<Pattern> patterns;
	};
	static const std::array<Type, COUNT> types;

	using Match = std::pair<int, AST>;
	using Memoization = std::map<std::tuple<int,int,bool,TYPE>, std::optional<Match>>;

	static std::optional<AST> from_tokens(std::vector<Token> tks);

	static std::optional<Match> match(std::span<Token> tks, TYPE type, bool full, Memoization& mem, int globalStart, int globalEnd, int depth=0);

	std::string to_string() const;

	int string_sz() const;

	TYPE type;
	std::vector<std::variant<AST,Token>> children{};


	// ------------------------------------------------------ //

	struct DrawNode {
		std::string text;
		int x = 0, y = 0;
		int width = 0;
		std::vector<DrawNode*> children{};
	};

	void render() const;
private:
	DrawNode* build_draw_tree() const;
	int calculate_layout(DrawNode* node, int x, int y, int& max_y) const;
	void draw_to_canvas(DrawNode* node, std::vector<std::string>& canvas, int y_offset) const;
};
