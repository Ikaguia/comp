#include <preProcessor.hpp>
#include <lexic.hpp>
#include <syntax.hpp>
// #include <semantic.hpp>
// #include <optimizer.hpp>
// #include <assembler.hpp>

// #include <symbolTable.hpp>
// #include <errorHandler.hpp>

#include <input.hpp>

#include <print>
#include <iostream>
#include <fstream>

int main() {
	for (std::string input : gen_input()) {
		// std::println("input = {}", input);
		// SymbolTable symbols;
		// ErrorHandler eh;
		// input = PreProcessor::process(input, eh);
		input = PreProcessor::process(input);

		auto tokens = Lexic::tokenizer.tokenize(input);
		// auto tokens = Token::tokenize(input, symbols, eh);
		Lexic::tokenizer.print(tokens);

		auto ast = AST::from_tokens(tokens);
		// auto ast = AST::fromTokens(tokens, symbols, eh);
		if (ast) {
			ast->render();
			std::println("\nReconstructed: {}", ast->to_string());
			std::println("Original:      {}", input);
			// printAST(ast);
		} else std::println("Falha em gerar AST");

		// auto intermediaryCode = Semantic::code(ast, symbols, eh);
		// auto optimizedCode = Optimizer::optimize(intermediaryCode, symbols, eh);
		// auto assembled = Assembler::assemble(optimizedCode, symbols, eh);
	}
}
