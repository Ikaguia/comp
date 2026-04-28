#pragma once

#include <string>
#include <tokenizer.hpp>

namespace PreProcessor {
	enum struct TYPE : size_t {
		STRING,
		SCOMM, LCOMM,
		OPDHT, OPHT, COMMA,
		KWDEFINE, KWUNDEFINE,
		PAOPEN, PACLOSE,
		IDENT,
		LBRK, WSPC, OTHER, COUNT
	};
	using PreProcessorTokenizer = Tokenizer<TYPE, TYPE::OTHER, static_cast<size_t>(TYPE::COUNT)>;
	extern const PreProcessorTokenizer tokenizer;

	std::string process(const std::string& input);
}
