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
		Pattern{OP1, Lexic::TYPE::OPBRKOPEN, EXPRESSION, Lexic::TYPE::OPBRKCLOSE}, // Subscript
		Pattern{OP1, Lexic::TYPE::PAOPEN, EXPRESSION, Lexic::TYPE::PACLOSE},       // Call (simplified)
		Pattern{OP1, Lexic::TYPE::OPDOT, Lexic::TYPE::IDENT},                      // Member
		Pattern{OP1, Lexic::TYPE::OPARROW, Lexic::TYPE::IDENT},                    // Pointer Member
	}, Type::Associativity::LTR};

	ts[OP2] = Type{OP2, "Operators | 2", {
		Pattern{OP1,},
		Pattern{Lexic::TYPE::OPINC, OP2},
		Pattern{Lexic::TYPE::OPDEC, OP2},
		Pattern{Lexic::TYPE::OPSUM, OP2},
		Pattern{Lexic::TYPE::OPSUB, OP2},
		Pattern{Lexic::TYPE::OPNOT, OP2},
		Pattern{Lexic::TYPE::OPBNOT, OP2},
		Pattern{Lexic::TYPE::OPAND, OP2},  // Address-of
		Pattern{Lexic::TYPE::OPMUL, OP2},  // Dereference
		Pattern{Lexic::TYPE::SIZEOF, OP2},
		// TODO:
		// Cast
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
		Pattern{OP5, Lexic::TYPE::OPSHL, OP4},
		Pattern{OP5, Lexic::TYPE::OPSHR, OP4},
	}, Type::Associativity::LTR};

	ts[OP6] = Type{OP6, "Operators | 6", {
		Pattern{OP5,},
		Pattern{OP6, Lexic::TYPE::OPLT, OP5},
		Pattern{OP6, Lexic::TYPE::OPLE, OP5},
		Pattern{OP6, Lexic::TYPE::OPGT, OP5},
		Pattern{OP6, Lexic::TYPE::OPGE, OP5},
	}, Type::Associativity::LTR};

	ts[OP7] = Type{OP7, "Operators | 7", {
		Pattern{OP6,},
		Pattern{OP7, Lexic::TYPE::OPEQ, OP6},
		Pattern{OP7, Lexic::TYPE::OPNEQ, OP6},
	}, Type::Associativity::LTR};

	ts[OP8] = Type{OP8, "Operators | 8", {
		Pattern{OP7,},
		Pattern{OP8, Lexic::TYPE::OPAND, OP7},
	}, Type::Associativity::LTR};

	ts[OP9] = Type{OP9, "Operators | 9", {
		Pattern{OP8,},
		Pattern{OP9, Lexic::TYPE::OPXOR, OP8},
	}, Type::Associativity::LTR};

	ts[OP10] = Type{OP10, "Operators | 10", {
		Pattern{OP9,},
		Pattern{OP10, Lexic::TYPE::OPOR, OP9},
	}, Type::Associativity::LTR};

	ts[OP11] = Type{OP11, "Operators | 11", {
		Pattern{OP10,},
		Pattern{OP11, Lexic::TYPE::OPLAND, OP10},
	}, Type::Associativity::LTR};

	ts[OP12] = Type{OP12, "Operators | 12", {
		Pattern{OP11,},
		Pattern{OP12, Lexic::TYPE::OPLOR, OP11},
	}, Type::Associativity::LTR};

	ts[OP13] = Type{OP13, "Operators | 13", {
		Pattern{OP12,},
		Pattern{OP12, Lexic::TYPE::OPCOND, EXPRESSION, Lexic::TYPE::OPCOLON, OP13},
	}, Type::Associativity::RTL};

	ts[OP14] = Type{OP14, "Operators | 14", {
		Pattern{OP13,},
		Pattern{OP1, Lexic::TYPE::OPASGN, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNSUM, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNSUB, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNMUL, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNDIV, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNREM, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNSHL, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNSHR, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNAND, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNXOR, OP14},
		Pattern{OP1, Lexic::TYPE::OPASGNOR, OP14},
	}, Type::Associativity::RTL};

	ts[OP15] = Type{OP15, "Operators | 15", {
		Pattern{OP14,},
		Pattern{OP15, Lexic::TYPE::OPCOMMA, OP14},
	}, Type::Associativity::LTR};

	ts[EXPRESSION] = Type{EXPRESSION, "Expression", {
		Pattern{OP15,},
	}, Type::Associativity::LTR};

	return ts;
}();

std::optional<AST> AST::from_tokens(std::vector<Token> tks) {
	MatchData data{tks, {}, {}};
	for (auto [idx, tk] : tks | std::views::enumerate) data.indexList[tk.type].push_back(idx);

	auto mtch = match(data, 0, tks.size(), EXPRESSION);
	if (mtch) return *mtch;
	return std::nullopt;
}

std::generator<std::vector<int>> gen_static_idxs(AST::MatchData& data, int start, int end, std::span<const AST::PatternElement> els, bool full=true) {
	if (els.empty()) {
		if (start == end || !full) co_yield {};
		co_return;
	}

	const AST::PatternElement& el = els.front();
	if (std::holds_alternative<Lexic::TYPE>(el)) {
		auto tp = std::get<Lexic::TYPE>(el);
		auto iter_nxt = std::lower_bound(data.indexList[tp].begin(), data.indexList[tp].end(), start);
		if (iter_nxt == data.indexList[tp].end()) co_return;
		auto idx = *iter_nxt;
		if (full) {
			if (idx != start) co_return;
			for (auto range : gen_static_idxs(data, start+1, end, els.subspan(1), true)) {
				range.push_back(start);
				co_yield range;
			}
		} else {
			while(idx < end) {
				for (auto range : gen_static_idxs(data, idx+1, end, els.subspan(1), true)) {
					range.push_back(idx);
					co_yield range;
				}
				iter_nxt++;
				if (iter_nxt == data.indexList[tp].end()) break;
				idx = *iter_nxt;
			}
		}
	} else if (std::holds_alternative<TYPE>(el)) {
		co_yield std::ranges::elements_of(gen_static_idxs(data, start, end, els.subspan(1), false));
	}
}

using Range = std::pair<int,int>;
using Ranges = std::vector<Range>;
std::generator<Ranges> gen_ranges(AST::MatchData& data, int start, int end, std::span<const AST::PatternElement> els) {
	if (els.size() == 1 && std::holds_alternative<TYPE>(els.front())) {
		co_yield Ranges{Range{start, end}};
		co_return;
	}

	size_t dyn_els_count = 0;
	for (auto el : els) if (std::holds_alternative<TYPE>(el)) dyn_els_count++;

	for (auto static_idxs : gen_static_idxs(data, start, end, els)) {
		int rg_start = start;
		Ranges out;
		for (auto rg_end : static_idxs | std::views::reverse) {
			if (rg_end != rg_start) out.emplace_back(rg_start, rg_end);
			rg_start = rg_end+1;
		}
		if (rg_start < end) out.emplace_back(rg_start, end);
		if (out.size() != dyn_els_count) continue;
		co_yield out;
	}
}

std::optional<AST> AST::match(MatchData& data, int start, int end, TYPE type, int depth) {
	auto& [tks, memoization, indexList] = data;
	if (memoization.contains({start, end, type})) return memoization[{start, end, type}];
	auto& result = memoization[{start, end, type}];

	for(int i=0;i<depth;i++)std::print("\t");
	std::print("Attempting match of type {}, tokens: ", types[type].name);
	Lexic::tokenizer.print(tks.subspan(start, end-start));
	std::cout.flush();

	const auto& tp = types[type];
	const auto& pats = tp.patterns;
	// bool isRTL = tp.associativity == Type::Associativity::RTL;

	for (auto& pat : pats | std::views::reverse) {
		for(int i=0;i<depth;i++)std::print("\t");
		std::print("Attempting pattern: [ ");
		for (auto el : pat) {
			if (std::holds_alternative<Lexic::TYPE>(el)) std::print("{} ", Lexic::tokenizer.types[static_cast<size_t>(std::get<Lexic::TYPE>(el))].name);
			else if (std::holds_alternative<TYPE>(el)) {
				std::print("{} ", types[std::get<TYPE>(el)].name);
			}
		}
		std::println("]");
		std::cout.flush();

		for (Ranges ranges : gen_ranges(data, start, end, pat)) {
			{
				for(int i=0;i<depth+1;i++)std::print("\t");
				std::println("Ranges sz {}, pat sz {}", ranges.size(), pat.size());
				for(int i=0;i<depth+1;i++)std::print("\t");
				std::print("Attempting ranges: [ ");
				for (auto [idx, el] : pat | std::views::filter(
					[](const auto& el) { return std::holds_alternative<TYPE>(el); }
				) | std::views::enumerate) {
					auto& t = types[static_cast<size_t>(std::get<TYPE>(el))];
					auto& rng = ranges[idx];
					if (idx) std::print(", ");
					std::print("{}: [{}-{})", t.name, rng.first, rng.second);
				}
				std::println(" ]");
				std::cout.flush();
			}

			AST node{type};
			bool failed = false;
			int idx_static = 0, idx_dyn = 0, idx_tkn = start;
			for (auto [idx, el] : std::views::enumerate(pat)) {
				if (idx_tkn < 0 || idx_tkn >= end) {
					failed = true;
					break;
				}
				if (std::holds_alternative<Lexic::TYPE>(el)) {
					const Token& tk = tks[idx_tkn];
					const Lexic::TYPE el_tp = std::get<Lexic::TYPE>(el);
					if (tk.type == el_tp) {
						node.children.push_back(tk);
						idx_static++;
						idx_tkn++;
					} else failed = true;
				} else if (std::holds_alternative<TYPE>(el)) {
					const TYPE el_tp = std::get<TYPE>(el);
					auto& range = ranges[idx_dyn];
					auto ans = match(data, range.first, range.second, el_tp, depth+2);
					if (!ans) failed = true;
					else {
						auto ast = *ans;
						node.children.push_back(ast);
						idx_dyn++;
						idx_tkn = range.second;
					}
				}
				if (failed) break;
			}
			if (idx_tkn != end) failed = true;
			if (!failed) {
				for(int i=0;i<depth;i++)std::print("\t");
				std::print("Successfull match of type {} on range [{}-{}) with pattern [", types[type].name, start, end);
				for (auto el : pat) {
					if (std::holds_alternative<Lexic::TYPE>(el)) std::print("{} ", Lexic::tokenizer.types[static_cast<size_t>(std::get<Lexic::TYPE>(el))].name);
					else if (std::holds_alternative<TYPE>(el)) {
						std::print("{} ", types[std::get<TYPE>(el)].name);
					}
				}
				std::println("]");
				std::cout.flush();
				return (result = {node});
			}
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
    auto root = build_draw_tree();
    if (!root) return;

    int max_depth = 0;
    calculate_layout(root, 0, 0, max_depth);

    // Height is max_depth * 2 (one for text, one for connectors)
    int total_width = root->width;
    std::vector<std::string> canvas((max_depth + 1) * 2, std::string(total_width, ' '));

    draw_to_canvas(root, canvas);

    for (const auto& line : canvas) {
        // Only print lines that aren't purely whitespace
        if (line.find_first_not_of(' ') != std::string::npos) {
            std::println("{}", line);
        }
    }
}

std::unique_ptr<AST::DrawNode> AST::build_draw_tree() const {
    auto d = std::make_unique<DrawNode>();

    // 1. Identify Pivot Index
    std::optional<int> pivot_idx;
    for (int i = 0; i < std::ssize(children); ++i) {
        if (std::holds_alternative<Token>(children[i])) {
            const auto& t = std::get<Token>(children[i]);
            if (t.type != Lexic::TYPE::IDENT && t.type != Lexic::TYPE::NUMINT && 
                t.type != Lexic::TYPE::NUMREAL && t.type != Lexic::TYPE::BOOLTRUE &&
                t.type != Lexic::TYPE::BOOLFALSE && t.type != Lexic::TYPE::PAOPEN && 
                t.type != Lexic::TYPE::PACLOSE) {
                pivot_idx = i;
                break; 
            }
        }
    }

    if (pivot_idx) {
        d->text = std::get<Token>(children[*pivot_idx]).str;
    }

    // 2. Build Children
    for (int i = 0; i < std::ssize(children); ++i) {
        if (pivot_idx && i == *pivot_idx) continue;

        if (std::holds_alternative<Token>(children[i])) {
            const auto& t = std::get<Token>(children[i]);
            if (t.type == Lexic::TYPE::PAOPEN || t.type == Lexic::TYPE::PACLOSE) continue;
            d->children.push_back(std::make_unique<DrawNode>(t.str));
        } else {
            d->children.push_back(std::get<AST>(children[i]).build_draw_tree());
        }
    }

    // 3. Collapse structural nodes
    if (d->text.empty()) {
        if (d->children.size() == 1) {
            return std::move(d->children[0]);
        }
        // If it's a leaf literal/ident that didn't have a pivot
        if (children.size() == 1 && std::holds_alternative<Token>(children[0])) {
            d->text = std::get<Token>(children[0]).str;
            d->children.clear();
        } else {
            d->text = types[type].name; // e.g., "Expression"
        }
    }

    return d;
}

int AST::calculate_layout(const std::unique_ptr<DrawNode>& node, int x, int y, int& max_y) const {
    if (!node) return 0;
    max_y = std::max(max_y, y);
    node->y = y;

    if (node->children.empty()) {
        node->width = static_cast<int>(node->text.length()) + 2;
        node->x = x;
        return node->width;
    }

    int current_x = x;
    for (const auto& child : node->children) {
        current_x += calculate_layout(child, current_x, y + 1, max_y);
    }

    node->width = std::max(static_cast<int>(node->text.length()) + 2, current_x - x);
    node->x = x + (node->width / 2) - (static_cast<int>(node->text.length()) / 2);
    return node->width;
}

void AST::draw_to_canvas(const std::unique_ptr<DrawNode>& node, std::vector<std::string>& canvas) const {
    int row = node->y * 2;
    
    // Draw text with safety bounds
    for (int i = 0; i < static_cast<int>(node->text.length()); ++i) {
        int target_x = node->x + static_cast<int>(i);
        if (target_x >= 0 && target_x < static_cast<int>(canvas[row].size())) {
            canvas[row][target_x] = node->text[i];
        }
    }

    if (node->children.empty()) return;

    // Draw connectors
    int row_conn = row + 1;
    int mid_x = node->x + (static_cast<int>(node->text.length()) / 2);
    int first_x = node->children.front()->x + (static_cast<int>(node->children.front()->text.length()) / 2);
    int last_x = node->children.back()->x + (static_cast<int>(node->children.back()->text.length()) / 2);

    if (node->children.size() == 1) {
        if (first_x < static_cast<int>(canvas[row_conn].size())) canvas[row_conn][first_x] = 'v';
    } else {
        for (int i = first_x; i <= last_x; ++i) {
            if (i < 0 || i >= static_cast<int>(canvas[row_conn].size())) continue;
            if (i == first_x) canvas[row_conn][i] = '<';
            else if (i == last_x) canvas[row_conn][i] = '>';
            else if (i == mid_x) canvas[row_conn][i] = '|';
            else canvas[row_conn][i] = '-';
        }
    }

    for (const auto& child : node->children) {
        draw_to_canvas(child, canvas);
    }
}
