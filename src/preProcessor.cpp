#include <preProcessor.hpp>
#include <regexHelpers.hpp>
#include <string>
#include <string_view>
#include <regex>

namespace PreProcessor {
	using Type = PreProcessorTokenizer::Type;
	using namespace RegexHelpers;
	using namespace std::string_view_literals;

	const PreProcessorTokenizer tokenizer{
		std::array {
			Type{ TYPE::STRING,		"String",
				std::string{OR(std::vector{
					// empty string
					R"("")"sv,
					AND(std::vector{
						// open "
						R"(")"sv,
						// anything other than a solo \ or a \n
						KStar(OR(std::vector{
							R"([^"\\\n])"sv,
							R"(\\[^\n])"sv,
						})),
						// close "
						R"(")"sv,
					})
				})},
				false, {} },
			Type{ TYPE::SCOMM,		"ShortComment",			R"(//[^\n]*)",		false, {} },
			Type{ TYPE::LCOMM,		"LongComment",			R"(/\*.*\*/)",		false, {} },
			Type{ TYPE::OPDHT,		"DoubleHashtag",		R"([#]{2})",		false, {} },
			Type{ TYPE::OPHT,		"Hashtag",				R"([#])",			false, {} },
			Type{ TYPE::COMMA,		"Comma",				R"([,])",			false, {} },
			Type{ TYPE::KWDEFINE,	"Define",				R"(define)",		false, {} },
			Type{ TYPE::KWUNDEFINE,	"Undefine",				R"(undef)",			false, {} },
			Type{ TYPE::PAOPEN,		"ParOpen",				R"(\()",			false, {} },
			Type{ TYPE::PACLOSE,	"ParClose",				R"(\))",			false, {} },
			Type{ TYPE::IDENT,		"Identifier",			R"([_A-Za-z]\w*)",	true,  {} },
			Type{ TYPE::LBRK,		"LineBreak",			R"(\n)",			false, {} },
			Type{ TYPE::WSPC,		"WhiteSpace",			R"(\s)",			true,  {} },
			Type{ TYPE::OTHER,		"Other",				R"(.)",				true,  {} },
		}
	};

	struct Defined {
		std::string name;
		std::vector<std::string> args;
		std::string val;
	};

	#define NXT do {																							\
			idx2++;																								\
			if (idx2 >= static_cast<int>(tks.size())) throw std::runtime_error("Unfinished macro definition");	\
			tk2 = tks[idx2];																					\
			std::println("NXT called, tk2 = {}, {}", tk2.str, static_cast<size_t>(tk2.type));					\
		} while (tk2.type == TYPE::WSPC);

	std::string process(const std::string& input) {
		std::string input2 = std::regex_replace(input, std::regex{R"(\\\n)"}, ""), output;

		std::map<std::string, Defined> defined;

		auto tks = tokenizer.tokenize(input2);
		tokenizer.print(tks);

		int line=0, col=0, skip=0;
		// enum struct STATE { DEFAULT, DEFINING_NAME, DEFINING_CONTENT, INVALID } state=STATE::DEFAULT;
		for (auto [idx, tk] : tks | std::views::enumerate) {
			if (skip) { skip--; continue; }

			if (tk.type == TYPE::SCOMM || tk.type == TYPE::LCOMM ) continue;
			else if (tk.type == TYPE::OPHT) {
				Defined d;

				int idx2 = idx;
				auto& tk2 = tks[idx2];
				NXT

				if (tk2.type != TYPE::KWDEFINE) throw std::runtime_error("Invalid macro definition");
				NXT

				if (tk2.type != TYPE::IDENT) throw std::runtime_error("Invalid macro definition");
				d.name = tk2.str;
				NXT

				if (tk2.type == TYPE::PAOPEN) {
					NXT
					while(true) {
						if (!d.args.empty()) {
							if (tk2.type != TYPE::COMMA) throw std::runtime_error("Invalid macro definition");
							NXT
						}

						if (tk2.type == TYPE::PACLOSE) {
							NXT
							break;
						}
						else if (tk2.type == TYPE::IDENT) d.args.push_back(tk2.str);
						else throw std::runtime_error("Invalid macro definition");

						NXT
					}
				}

				while(tk2.type != TYPE::LBRK) {
					d.val += tk2.str;
					NXT
				}

				skip = idx2-idx-1;
				if (defined.contains(d.name)) throw std::runtime_error("Duplicated definition of macro " + d.name);
				defined[d.name] = d;
			} else if (tk.type == TYPE::LBRK) {
				output += "\n";
				line++;
				col = 0;
			} else if (tk.type == TYPE::IDENT && defined.contains(tk.str)) {
				Defined& d = defined[tk.str];
				if (!d.args.empty()) {
					int idx2 = idx;
					auto& tk2 = tks[idx2];
					NXT

					if (tk2.type != TYPE::PAOPEN) throw std::runtime_error("Invalid macro invocation");
					NXT

					std::vector<std::string> args; args.resize(d.args.size());

					for (int i = 0; i < static_cast<int>(d.args.size()); i++) {
						auto& arg = args[i];
						int depth = 0;
						bool last = i == static_cast<int>(d.args.size()) - 1;
						while(true) {
							if (tk2.type == TYPE::PAOPEN) depth++;
							if (tk2.type == TYPE::PACLOSE) {
								depth--;
								if (last && depth < 0) {
									NXT
									break;
								}
								if (depth < 0) throw std::runtime_error("Invalid macro invocation");
							}
							if (tk2.type == TYPE::COMMA && depth == 0) {
								NXT
								break;
							}
							arg += tk2.str;
							NXT
						}
					}
					// TODO: Use d.val and args to expand the macro properly
					output += d.val;

					skip = idx2-idx-1;
				} else output += d.val;
			} else {
				output += tk.str;
				col++;
			}
		}

		return output;
	}
}
