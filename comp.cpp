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

using Num = std::variant<int, float>;

struct Token {
	enum struct TYPE : size_t { OPINC = 0, OPDEC, OPSUM, OPSUB, OPMUL, OPDIV, OPREM, OPASGN, OPEQ, OPNEQ, PAOPEN, PACLOSE, NUMREAL, NUMINT, BOOLTRUE, BOOLFALSE, INVALID, COUNT };
	enum struct CATEGORY : size_t { NUMERIC = 0, BOOLEAN, OPERATION, PARENTHESIS, INVALID, COUNT };
	inline static const std::array<TYPE, static_cast<size_t>(TYPE::COUNT)> TYPES = {
		TYPE::OPINC,
		TYPE::OPDEC,
		TYPE::OPSUM,
		TYPE::OPSUB,
		TYPE::OPMUL,
		TYPE::OPDIV,
		TYPE::OPREM,
		TYPE::OPASGN,
		TYPE::OPEQ,
		TYPE::OPNEQ,
		TYPE::PAOPEN,
		TYPE::PACLOSE,
		TYPE::NUMREAL,
		TYPE::NUMINT,
		TYPE::BOOLTRUE,
		TYPE::BOOLFALSE,
		TYPE::INVALID,
	};
	struct Type {
		CATEGORY category;
		std::string_view name, regex_str;
		bool includeVal;
		TYPE type;
		std::regex regex;
	};

	inline static const std::array<Type, static_cast<size_t>(TYPE::COUNT)> types = [] {
		std::array<Type, static_cast<size_t>(TYPE::COUNT)> ts {
			Type{ CATEGORY::OPERATION, "OpInc", R"([+]{2})", false, TYPE::OPINC, {} },
			Type{ CATEGORY::OPERATION, "OpDec", R"([-]{2})", false, TYPE::OPDEC, {} },
			Type{ CATEGORY::OPERATION, "OpSum", R"([+])", false, TYPE::OPSUM, {} },
			Type{ CATEGORY::OPERATION, "OpSub", R"([-])", false, TYPE::OPSUB, {} },
			Type{ CATEGORY::OPERATION, "OpMul", R"([*])", false, TYPE::OPMUL, {} },
			Type{ CATEGORY::OPERATION, "OpDiv", R"([/])", false, TYPE::OPDIV, {} },
			Type{ CATEGORY::OPERATION, "OpRem", R"([%])", false, TYPE::OPREM, {} },
			Type{ CATEGORY::OPERATION, "OpAssign", R"([=])", false, TYPE::OPASGN, {} },
			Type{ CATEGORY::OPERATION, "OpEquals", R"([=]{2})", false, TYPE::OPEQ, {} },
			Type{ CATEGORY::OPERATION, "OpNotEquals", R"([!][=])", false, TYPE::OPNEQ, {} },
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
			if (tp.type != TP) throw std::runtime_error("Mismatched Token::TYPE on Token::types");
			tp.regex = std::regex{static_cast<std::string>(tp.regex_str)};
		}
		return ts;
	}();
	inline static const std::string regex_str_all = types
		| std::views::filter([](auto tp) { return tp.category != CATEGORY::INVALID; })
		| std::views::transform([](auto tp) { return std::format("({})", tp.regex_str); })
		| std::views::join_with('|')
		| std::ranges::to<std::string>();
	inline static const std::regex regex_all{regex_str_all};

	TYPE type;
	std::string str;
	Num val;

	Token() : type{TYPE::INVALID} {}
	Token(TYPE t) : type{t} { _val(); }
	Token(TYPE t, std::string s) : type{t}, str{s} { _val(); }
	Token(std::string s) : str{s} {
		std::smatch match;
		std::regex_match(str, match, regex_all);
		size_t j = 1;
		for (size_t i = 0; i < static_cast<size_t>(TYPE::COUNT); i++) {
			if (types[i].category == CATEGORY::INVALID) continue;
			if (match[j++].matched) type = static_cast<TYPE>(i);
		}
		_val();
	}
	Token(Num v) : val{v} {
		std::visit([this](auto&& arg) {
			this->str = std::format("{}", arg);
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, int>) this->type = TYPE::NUMINT;
			else if constexpr (std::is_same_v<T, float>) this->type = TYPE::NUMREAL;
			else this->type = TYPE::INVALID;
		}, v);
	}
	const Type& getType() const { return types[static_cast<size_t>(type)]; }
	std::string name() const {
		auto tp = getType();
		return tp.includeVal ? std::format("{} ({})", std::string{tp.name}, str) : std::string{tp.name};
	}
	std::string_view regex_str() const { return getType().regex_str; }
	std::regex regex() const { return getType().regex; }

	inline static const Type& getType(TYPE t) { return types[static_cast<size_t>(t)]; }
	inline static std::string_view name(TYPE t) { return getType(t).name; }
	inline static std::string_view regex_str(TYPE t) { return getType(t).regex_str; }
	inline static std::regex regex(TYPE t) { return getType(t).regex; }

	static std::vector<Token> tokenize(const std::string& s) {
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
private:
	void _val() {
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
};

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

struct BNF {
	using TYPE = enum { NUMBER, BOOLEAN, LITERAL, PARENTHESIS, OP1, OP2, OP3, OP4, OP5, OP6, OP7, OP8, OP9, OP10, OP11, OP12, OP13, OP14, OP15, EXPRESSION, COUNT };
	inline static const std::array<TYPE, COUNT> TYPES = { NUMBER, BOOLEAN, LITERAL, PARENTHESIS, OP1, OP2, OP3, OP4, OP5, OP6, OP7, OP8, OP9, OP10, OP11, OP12, OP13, OP14, OP15, EXPRESSION };

	using Pattern = std::vector<std::variant<Token::TYPE, TYPE>>;
	struct Type {
		TYPE type;
		std::string name;
		std::vector<Pattern> patterns;
	};
	inline static const std::array<Type, COUNT> types = [] {
		std::array<Type, COUNT> ts;
		ts[NUMBER] = Type{NUMBER, "Number", {
			Pattern{Token::TYPE::NUMINT,},
			Pattern{Token::TYPE::NUMREAL,},
		}};
		ts[BOOLEAN] = Type{BOOLEAN, "Boolean", {
			Pattern{Token::TYPE::BOOLTRUE,},
			Pattern{Token::TYPE::BOOLFALSE,},
		}};
		ts[LITERAL] = Type{LITERAL, "Literal", {
			Pattern{NUMBER,},
			Pattern{BOOLEAN,},
		}};
		ts[PARENTHESIS] = Type{PARENTHESIS, "PARENTHESIS", {
			Pattern{LITERAL,},
			Pattern{Token::TYPE::PAOPEN, EXPRESSION, Token::TYPE::PACLOSE},
		}};
		ts[OP1] = Type{OP1, "Operator | 1", {
			Pattern{PARENTHESIS,},
			Pattern{OP1, Token::TYPE::OPINC},
			Pattern{OP1, Token::TYPE::OPDEC},
			// TODO:
			// Function call
			// Array subscripting
			// Structure and union member access
			// Structure and union member access through pointer
			// Compound literal
		}};
		ts[OP2] = Type{OP2, "Operator | 2", {
			Pattern{OP1,},
			Pattern{Token::TYPE::OPINC, OP2},
			Pattern{Token::TYPE::OPDEC, OP2},
			Pattern{Token::TYPE::OPSUM, OP2},
			Pattern{Token::TYPE::OPSUB, OP2},
			// TODO:
			// Logical NOT and bitwise NOT
			// Cast
			// Indirection (dereference)
			// Address-of
			// Size-of
			// Alignment requirement
		}};
		ts[OP3] = Type{OP3, "Operator | 3", {
			Pattern{OP2,},
			Pattern{OP3, Token::TYPE::OPMUL, OP2},
			Pattern{OP3, Token::TYPE::OPDIV, OP2},
			Pattern{OP3, Token::TYPE::OPREM, OP2},
		}};
		ts[OP4] = Type{OP4, "Operator | 4", {
			Pattern{OP3,},
			Pattern{OP4, Token::TYPE::OPSUM, OP3},
			Pattern{OP4, Token::TYPE::OPSUB, OP3},
		}};
		ts[OP5] = Type{OP5, "Operator | 5", {
			Pattern{OP4,},
			// TODO:
			// Bitwise left shift and right shift
		}};
		ts[OP6] = Type{OP6, "Operator | 6", {
			Pattern{OP5,},
			// Relational operators < and <=
			// Relational operators > and >=
		}};
		ts[OP7] = Type{OP7, "Operator | 7", {
			Pattern{OP6,},
			// TODO:
			// Relational = and !=
		}};
		ts[OP8] = Type{OP8, "Operator | 8", {
			Pattern{OP7,},
			// TODO:
			// Bitwise AND
		}};
		ts[OP9] = Type{OP9, "Operator | 9", {
			Pattern{OP8,},
			// TODO:
			// Bitwise XOR (exclusive or)
		}};
		ts[OP10] = Type{OP10, "Operator | 10", {
			Pattern{OP9,},
			// TODO:
			// Bitwise OR (inclusive or)
		}};
		ts[OP11] = Type{OP11, "Operator | 11", {
			Pattern{OP10,},
			// TODO:
			// Logical AND
		}};
		ts[OP12] = Type{OP12, "Operator | 12", {
			Pattern{OP11,},
			// TODO:
			// Logical OR
		}};
		ts[OP13] = Type{OP13, "Operator | 13", {
			Pattern{OP12,},
			// TODO:
			// Ternary conditional
		}};
		ts[OP14] = Type{OP14, "Operator | 14", {
			Pattern{OP13,},
			// TODO:
			// Simple assignment
			// Assignment by sum and difference
			// Assignment by product, quotient, and remainder
			// Assignment by bitwise left shift and right shift
			// Assignment by bitwise AND, XOR, and OR
		}};
		ts[OP15] = Type{OP15, "Operator | 15", {
			Pattern{OP14,},
			// TODO:
			// Comma
		}};

		return ts;
	}();

	using Match = std::pair<int, BNF>;
	using Memoization = std::map<std::tuple<int,int,bool,TYPE>, std::optional<Match>>;

	static std::optional<BNF> from_tokens(std::vector<Token> tks) {
		Memoization mem;
		auto mtch = match(tks, EXPRESSION, true, mem, 0, tks.size());
		if (mtch) return mtch->second;
		return std::nullopt;
	}

	static std::optional<Match> match(std::span<Token> tks, TYPE type, bool full, Memoization& mem, int globalStart, int globalEnd, int depth=0) {
		if (mem.contains({globalStart, globalEnd, full, type})) return mem[{globalStart, globalEnd, full, type}];
		auto& result = mem[{globalStart, globalEnd, full, type}];

		for(int i=0;i<depth;i++)std::print("\t");
		std::print("Attempting {} match of type {}, tokens: ", full ? "full" : "free", types[type].name);
		printTokens(tks);
		std::cout.flush();

		auto& pats = types[type].patterns;
		for (auto pat : pats | std::views::reverse) {
			if (tks.size() < pat.size()) continue;
			int cnt = 0;
			for(int i=0;i<depth;i++)std::print("\t");
			std::print("Attempting pattern: [ ");
			for (auto el : pat) {
				if (std::holds_alternative<Token::TYPE>(el)) std::print("{} ", Token::name(std::get<Token::TYPE>(el)));
				else if (std::holds_alternative<TYPE>(el)) {
					std::print("{} ", types[std::get<TYPE>(el)].name);
					cnt++;
				}
			}
			std::println("]");
			std::cout.flush();

			int consumed = 0;
			int idx = tks.size() - 1;
			int pat_els_rem = pat.size();
			bool pat_matches = true;
			BNF node{type};
			for (auto el : pat | std::views::reverse) {
				pat_els_rem--;
				if (idx < pat_els_rem) {
					pat_matches = false;
					break;
				}
				auto& tk = tks[idx];
				std::visit([&](auto&& arg){
					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T, Token::TYPE>) {
						if (tk.type == arg) {
							for(int i=-1;i<depth;i++)std::print("\t");
							std::println("Found token {} on idx {}", tk.str, idx); std::cout.flush();
							node.children.push_back(tk);
							idx--;
							consumed++;
						} else pat_matches = false;
					} else if constexpr (std::is_same_v<T, TYPE>) {
						if (arg == type && idx == (std::ssize(tks) - 1)) {
							pat_matches = false;
							return;
						}
						int remaining = pat.size() - (node.children.size() + 1);
						int start = (cnt == 1) ? remaining : 0;
						bool fll = full && ((cnt == 1) || (remaining == 0));
						auto res = match(tks.subspan(start, (idx+1) - start), arg, fll, mem, globalStart+start, globalStart + ((idx+1) - start), depth+1);
						if (!res) pat_matches = false;
						else {
							auto [cnsmd, bnf] = *res;
							for(int i=-1;i<depth;i++)std::print("\t");
							std::println("Found {} on range {}-{}", types[arg].name, (idx-cnsmd)+1, idx);
							for(int i=-1;i<depth;i++)std::print("\t");
							std::println("{}", bnf.to_string());
							std::cout.flush();
							node.children.push_back(std::move(bnf));
							idx -= cnsmd;
							consumed += cnsmd;
						}
					}
				}, el);
				if (!pat_matches) break;
			}
			if (pat_matches) {
				if (full && (idx > -1)) continue;
				return result = Match{consumed, node};
			}
		}
		return result = std::nullopt;
	}

	std::string to_string() const {
		std::string str;
		str.reserve(string_sz());
		for (const auto& child : children | std::views::reverse) {
			if (!str.empty()) str += " ";
			if (std::holds_alternative<BNF>(child)) str += std::get<BNF>(child).to_string();
			else if (std::holds_alternative<Token>(child)) str += std::get<Token>(child).str;
		}
		return str;
	}

	int string_sz() const {
		int sz = 0;
		for (const auto& child : children) {
			if (sz) sz++;
			if (std::holds_alternative<BNF>(child)) sz += std::get<BNF>(child).string_sz();
			else if (std::holds_alternative<Token>(child)) sz += std::get<Token>(child).str.size();
		}
		return sz;
	}

	TYPE type;
	std::vector<std::variant<BNF,Token>> children{};


	// ------------------------------------------------------ //

	struct DrawNode {
		std::string text;
		int x = 0, y = 0;
		int width = 0;
		std::vector<DrawNode*> children{};
	};

	void render() const {
		// 1. Convert BNF tree to a simplified DrawNode tree for layout
		auto root = build_draw_tree();
		
		// 2. Calculate coordinates (depth-first)
		int max_depth = 0;
		calculate_layout(root, 0, 0, max_depth);

		// 3. Create a canvas
		int total_width = root->width;
		std::vector<std::string> canvas( (max_depth + 1) * 2, std::string(total_width, ' '));

		// 4. Draw to canvas
		draw_to_canvas(root, canvas, 0);

		// 5. Print canvas
		for (const auto& line : canvas) {
			if (line.find_first_not_of(' ') != std::string::npos) {
				std::println("{}", line);
			}
		}
	}

	private:
	DrawNode* build_draw_tree() const {
		DrawNode* d = new DrawNode();

		// Find the primary value/op for this node
		// In your BNF, children are stored in reverse (RTL)
		auto ordered_children = children | std::views::reverse;
		
		// Filter out structural tokens like parentheses for the visual tree
		for (const auto& child : ordered_children) {
			if (std::holds_alternative<Token>(child)) {
				Token t = std::get<Token>(child);
				if (t.type != Token::TYPE::PAOPEN && t.type != Token::TYPE::PACLOSE) {
					if (d->text.empty()) d->text = t.str;
					else d->children.push_back(new DrawNode{t.str});
				} else if (t.type == Token::TYPE::PAOPEN || t.type == Token::TYPE::PACLOSE) {
					// If we want to show parens, we treat them as part of the node text
					d->text = (t.type == Token::TYPE::PAOPEN) ? "()" : d->text;
				}
			} else {
				d->children.push_back(std::get<BNF>(child).build_draw_tree());
			}
		}

		// Heuristic: If node has no text but has 1 child, collapse it (e.g., EXPRESSION -> TERM)
		if (d->text.empty() && d->children.size() == 1) {
			DrawNode* solo = d->children[0];
			delete d;
			return solo;
		}
		if (d->text.empty()) d->text = "?"; 
		return d;
	}

	int calculate_layout(DrawNode* node, int x, int y, int& max_y) const {
		if (!node) return 0;
		max_y = std::max(max_y, y);
		node->y = y;

		if (node->children.empty()) {
			node->width = node->text.length() + 2;
			node->x = x;
			return node->width;
		}

		int current_x = x;
		for (auto* child : node->children) {
			current_x += calculate_layout(child, current_x, y + 1, max_y);
		}

		node->width = std::max((int)node->text.length() + 2, current_x - x);
		node->x = x + (node->width / 2) - (node->text.length() / 2);
		return node->width;
	}

	void draw_to_canvas(DrawNode* node, std::vector<std::string>& canvas, int y_offset) const {
		int row = node->y * 2;
		// Draw text
		for(size_t i=0; i<node->text.length(); ++i) {
			if (node->x + i < canvas[row].size())
				canvas[row][node->x + i] = node->text[i];
		}

		if (node->children.empty()) return;

		// Draw connectors <---|--->
		int row_conn = row + 1;
		int first_x = node->children.front()->x + (node->children.front()->text.length()/2);
		int last_x = node->children.back()->x + (node->children.back()->text.length()/2);
		int mid_x = node->x + (node->text.length()/2);

		if (node->children.size() == 1) canvas[row_conn][first_x] = 'v';
		else { for (int i = first_x; i <= last_x; ++i) {
			if (i < (int)canvas[row_conn].size()) {
				if (i == first_x) canvas[row_conn][i] = '<';
				else if (i == last_x) canvas[row_conn][i] = '>';
				else if (i == mid_x) canvas[row_conn][i] = '|';
				else canvas[row_conn][i] = '-';
			}
		} }

		for (auto* child : node->children) {
			draw_to_canvas(child, canvas, y_offset);
		}
	}
};

struct Node {
	Token token;
	std::vector<int> children{};
};

int main() {
	std::string input, output;

	std::print("digite uma entrada: ");
	while (std::getline(std::cin, input)) {
		auto tokens = Token::tokenize(input);
		if (tokens.empty()) continue;
		printTokens(tokens);
		std::cout.flush();

		auto res = BNF::from_tokens(tokens);
		if (res) {
			std::println("\nParsed Structure:");
			res->render();
			std::println("\nReconstructed: {}", res->to_string());
		}
		std::print("digite uma entrada: ");
	}
}
