#include <syntax.hpp>

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
#include <generator>

template <typename T>
auto gen = [](std::span<const T> sp, bool reverse) -> std::generator<T> {
	if (!reverse) for (auto el : sp) co_yield el;
	else for (auto el : sp | std::views::reverse) co_yield el;
	co_return;
};


using TYPE = AST::TYPE;
using Type = AST::Type;

const std::array<TYPE, AST::COUNT> AST::TYPES = { NUMBER, BOOLEAN, LITERAL, PARENTHESIS, OP1, OP2, OP3, OP4, OP5, OP6, OP7, OP8, OP9, OP10, OP11, OP12, OP13, OP14, OP15, EXPRESSION };

const std::array<Type, AST::COUNT> AST::types = [] {
	std::array<Type, COUNT> ts;
	ts[NUMBER] = Type{NUMBER, "Number", {
		Pattern{Lexic::TYPE::NUMINT,},
		Pattern{Lexic::TYPE::NUMREAL,},
	}, Type::Associativity::LTR};
	ts[BOOLEAN] = Type{BOOLEAN, "Boolean", {
		Pattern{Lexic::TYPE::BOOLTRUE,},
		Pattern{Lexic::TYPE::BOOLFALSE,},
	}, Type::Associativity::LTR};
	ts[LITERAL] = Type{LITERAL, "Literal", {
		Pattern{NUMBER,},
		Pattern{BOOLEAN,},
	}, Type::Associativity::LTR};
	ts[PARENTHESIS] = Type{PARENTHESIS, "PARENTHESIS", {
		Pattern{LITERAL,},
		Pattern{Lexic::TYPE::IDENT},
		Pattern{Lexic::TYPE::PAOPEN, EXPRESSION, Lexic::TYPE::PACLOSE},
	}, Type::Associativity::LTR};
	ts[OP1] = Type{OP1, "Operators | 1", {
		Pattern{PARENTHESIS,},
		Pattern{OP1, Lexic::TYPE::OPINC},
		Pattern{OP1, Lexic::TYPE::OPDEC},
		// TODO:
		// Function call
		// Array subscripting
		// Structure and union member access
		// Structure and union member access through pointer
		// Compound literal
	}, Type::Associativity::LTR};
	ts[OP2] = Type{OP2, "Operators | 2", {
		Pattern{OP1,},
		Pattern{Lexic::TYPE::OPINC, OP2},
		Pattern{Lexic::TYPE::OPDEC, OP2},
		Pattern{Lexic::TYPE::OPSUM, OP2},
		Pattern{Lexic::TYPE::OPSUB, OP2},
		// TODO:
		// Logical NOT and bitwise NOT
		// Cast
		// Indirection (dereference)
		// Address-of
		// Size-of
		// Alignment requirement
	}, Type::Associativity::RTL};
	ts[OP3] = Type{OP3, "Operators | 3", {
		Pattern{OP2,},
		Pattern{OP3, Lexic::TYPE::OPMUL, OP2},
		Pattern{OP3, Lexic::TYPE::OPDIV, OP2},
		Pattern{OP3, Lexic::TYPE::OPREM, OP2},
	}, Type::Associativity::LTR};
	ts[OP4] = Type{OP4, "Operators | 4", {
		Pattern{OP3,},
		Pattern{OP4, Lexic::TYPE::OPSUM, OP3},
		Pattern{OP4, Lexic::TYPE::OPSUB, OP3},
	}, Type::Associativity::LTR};
	ts[OP5] = Type{OP5, "Operators | 5", {
		Pattern{OP4,},
		// TODO:
		// Bitwise left shift and right shift
	}, Type::Associativity::LTR};
	ts[OP6] = Type{OP6, "Operators | 6", {
		Pattern{OP5,},
		// Relational operators < and <=
		// Relational operators > and >=
	}, Type::Associativity::LTR};
	ts[OP7] = Type{OP7, "Operators | 7", {
		Pattern{OP6,},
		Pattern{OP7, Lexic::TYPE::OPEQ, OP6},
		Pattern{OP7, Lexic::TYPE::OPNEQ, OP6},
	}, Type::Associativity::LTR};
	ts[OP8] = Type{OP8, "Operators | 8", {
		Pattern{OP7,},
		// TODO:
		// Bitwise AND
	}, Type::Associativity::LTR};
	ts[OP9] = Type{OP9, "Operators | 9", {
		Pattern{OP8,},
		// TODO:
		// Bitwise XOR (exclusive or)
	}, Type::Associativity::LTR};
	ts[OP10] = Type{OP10, "Operators | 10", {
		Pattern{OP9,},
		// TODO:
		// Bitwise OR (inclusive or)
	}, Type::Associativity::LTR};
	ts[OP11] = Type{OP11, "Operators | 11", {
		Pattern{OP10,},
		// TODO:
		// Logical AND
	}, Type::Associativity::LTR};
	ts[OP12] = Type{OP12, "Operators | 12", {
		Pattern{OP11,},
		// TODO:
		// Logical OR
	}, Type::Associativity::LTR};
	ts[OP13] = Type{OP13, "Operators | 13", {
		Pattern{OP12,},
		// TODO:
		// Ternary conditional
	}, Type::Associativity::RTL};
	ts[OP14] = Type{OP14, "Operators | 14", {
		Pattern{OP13,},
		Pattern{Lexic::TYPE::IDENT, Lexic::TYPE::OPASGN, OP14},
		// TODO:
		// Assignment by sum and difference
		// Assignment by product, quotient, and remainder
		// Assignment by bitwise left shift and right shift
		// Assignment by bitwise AND, XOR, and OR
	}, Type::Associativity::RTL};
	ts[OP15] = Type{OP15, "Operators | 15", {
		Pattern{OP14,},
		// TODO:
		// Comma
	}, Type::Associativity::LTR};
	ts[EXPRESSION] = Type{EXPRESSION, "Expression", {
		Pattern{OP15,},
	}, Type::Associativity::LTR};

	return ts;
}();

std::optional<AST> AST::from_tokens(std::vector<Token> tks) {
	Memoization mem;
	auto mtch = match(tks, EXPRESSION, true, mem, 0, tks.size());
	if (mtch) return mtch->second;
	return std::nullopt;
}

std::optional<AST::Match> AST::match(std::span<const Token> tks, TYPE type, bool full, Memoization& mem, int globalStart, int globalEnd, int depth) {
	if (mem.contains({globalStart, globalEnd, full, type})) return mem[{globalStart, globalEnd, full, type}];
	auto& result = mem[{globalStart, globalEnd, full, type}];

	for(int i=0;i<depth;i++)std::print("\t");
	std::print("Attempting {} match of type {}, tokens: ", full ? "full" : "free", types[type].name);
	Lexic::tokenizer.print(tks);
	std::cout.flush();

	const auto& tp = types[type];
	const auto& pats = tp.patterns;
	bool isRTL = tp.associativity == Type::Associativity::RTL;
	for (auto pat : pats | std::views::reverse) {
		if (tks.size() < pat.size()) continue;
		int dynEls = 0;
		for(int i=0;i<depth;i++)std::print("\t");
		std::print("Attempting pattern: [ ");
		for (auto el : pat) {
			if (std::holds_alternative<Lexic::TYPE>(el)) std::print("{} ", Lexic::tokenizer.types[static_cast<size_t>(std::get<Lexic::TYPE>(el))].name);
			else if (std::holds_alternative<TYPE>(el)) {
				std::print("{} ", types[std::get<TYPE>(el)].name);
				dynEls++;
			}
		}
		std::println("]");
		std::cout.flush();

		int consumedTkns = 0, remainingTkns = std::ssize(tks);
		int idxTknStart = isRTL ? 0 : std::ssize(tks) - 1;
		int idxTkn = idxTknStart;
		int step = isRTL ? 1 : -1;
		int elsRem = pat.size();
		bool pat_matches = true;
		AST node{type};
		for (auto el : gen<std::variant<Lexic::TYPE, TYPE>>(pat, !isRTL)) {
			elsRem--;
			if (remainingTkns < elsRem || idxTkn < 0 || idxTkn >= std::ssize(tks)) {
				pat_matches = false;
				break;
			}
			auto& tk = tks[idxTkn];
			std::visit([&](auto&& arg){
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, Lexic::TYPE>) {
					if (tk.type == arg) {
						for(int i=-1;i<depth;i++)std::print("\t");
						std::println("Found token {} on idxTkn {}", tk.str, idxTkn); std::cout.flush();
						node.children.push_back(tk);
						idxTkn += step;
						consumedTkns++;
						remainingTkns--;
					} else pat_matches = false;
				} else if constexpr (std::is_same_v<T, TYPE>) {
					if (arg == type && idxTkn == idxTknStart) {
						pat_matches = false;
						return;
					}
					bool fll = full && ((dynEls == 1) || (elsRem == 0));
					int start = isRTL ? idxTkn : (dynEls == 1 ? elsRem : 0);
					int quant = dynEls == 1 ? remainingTkns - elsRem : remainingTkns;
					auto span = tks.subspan(start, quant);
					auto res = match(span, arg, fll, mem, globalStart+start, globalStart+start+quant, depth+1);
					if (!res) pat_matches = false;
					else {
						auto [cnsmd, ast] = *res;
						for(int i=-1;i<depth;i++)std::print("\t");
						std::println("Found {} on range {}-{}", types[arg].name, isRTL ? start : start+quant-cnsmd, isRTL ? start+cnsmd-1 : start+quant-1);
						for(int i=-1;i<depth;i++)std::print("\t");
						std::println("{}", ast.to_string());
						std::cout.flush();
						node.children.push_back(std::move(ast));
						idxTkn += cnsmd * step;
						consumedTkns += cnsmd;
						remainingTkns -= cnsmd;
					}
					dynEls--;
				}
			}, el);
			if (!pat_matches) break;
		}
		if (pat_matches) {
			if (full && ((isRTL && idxTkn < std::ssize(tks)) || (!isRTL && idxTkn > -1))) continue;
			if (!isRTL) std::ranges::reverse(node.children);
			return result = Match{consumedTkns, node};
		}
	}
	return result = std::nullopt;
}

std::string AST::to_string() const {
	std::string str;
	str.reserve(string_sz());
	for (const auto& child : children) {
		if (!str.empty()) str += " ";
		if (std::holds_alternative<AST>(child)) str += std::get<AST>(child).to_string();
		else if (std::holds_alternative<Token>(child)) str += std::get<Token>(child).str;
	}
	return str;
}

int AST::string_sz() const {
	int sz = 0;
	for (const auto& child : children) {
		if (sz) sz++;
		if (std::holds_alternative<AST>(child)) sz += std::get<AST>(child).string_sz();
		else if (std::holds_alternative<Token>(child)) sz += std::get<Token>(child).str.size();
	}
	return sz;
}

// ------------------------------------------------------ //

void AST::render() const {
	// 1. Convert AST tree to a simplified DrawNode tree for layout
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

AST::DrawNode* AST::build_draw_tree() const {
	DrawNode* d = new DrawNode();

	// 1. Identify the "Pivot" token (the operator)
	// For binary/unary ops, the operator is usually the token that ISN'T 
	// an identifier or a literal. 
	const Token* pivot = nullptr;
	for (const auto& child : children) {
		if (std::holds_alternative<Token>(child)) {
			const Token& t = std::get<Token>(child);
			// Simple heuristic: If it's an operator type, it's our pivot
			if (t.type != Lexic::TYPE::IDENT && 
				t.type != Lexic::TYPE::NUMINT && 
				t.type != Lexic::TYPE::NUMREAL &&
				t.type != Lexic::TYPE::BOOLTRUE &&
				t.type != Lexic::TYPE::BOOLFALSE &&
				t.type != Lexic::TYPE::PAOPEN && 
				t.type != Lexic::TYPE::PACLOSE) {
				pivot = &t;
				break; 
			}
		}
	}

	if (pivot) d->text = pivot->str;

	// 2. Process children
	for (const auto& child : children) {
		if (std::holds_alternative<Token>(child)) {
			Token t = std::get<Token>(child);
			// Skip parentheses and the pivot we already used
			if (&t == pivot || t.type == Lexic::TYPE::PAOPEN || t.type == Lexic::TYPE::PACLOSE) continue;
			
			d->children.push_back(new DrawNode{t.str});
		} else {
			d->children.push_back(std::get<AST>(child).build_draw_tree());
		}
	}

	// 3. Leaf handling (Numbers, Identifiers)
	if (d->text.empty() && children.size() == 1) {
		if (std::holds_alternative<Token>(children[0])) {
			d->text = std::get<Token>(children[0]).str;
		} else {
			DrawNode* solo = std::get<AST>(children[0]).build_draw_tree();
			delete d;
			return solo;
		}
	}

	// 4. Fallback for structural nodes with no operator
	if (d->text.empty()) d->text = types[type].name;

	return d;
}

int AST::calculate_layout(DrawNode* node, int x, int y, int& max_y) const {
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

void AST::draw_to_canvas(DrawNode* node, std::vector<std::string>& canvas, int y_offset) const {
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
