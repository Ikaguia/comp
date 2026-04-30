#include <input.hpp>
#include <print>
#include <iostream>
#include <fstream>

std::generator<std::string> gen_input() {
	int mode;
	std::string input, line;
	std::println("Digite 1 para entrada manual ou 2 para entrada por arquivo.");
	while(std::cin >> mode) {
		std::cin.get();
		if (mode == 1) {
			std::println("Digite a entrada.");
			std::getline(std::cin, input);
			co_yield input;
		} else if (mode == 2) {
			std::println("Digite o nome do arquivo de entrada.");
			std::getline(std::cin, input);
			std::ifstream file{input};
			if (file) {
				input = "";
				while(std::getline(file, line)) input += line + "\n";
				co_yield input;
			} else std::println("Falha ao ler o arquivo");
		} else co_return;
		std::println("Digite 1 para entrada manual ou 2 para entrada por arquivo.");
	}
}
std::generator<std::string> gen_inputManual() {
	std::string input;
	std::println("Digite a entrada.");
	while(std::getline(std::cin, input)) {
		co_yield input;
		std::println("Digite a entrada.");
	}
}
std::generator<std::string> gen_inputFromFile() {
	std::string input, fileName, line;
	std::println("Digite o nome do arquivo de entrada.");
	while(std::getline(std::cin, fileName)) {
		std::ifstream file{fileName};
		if (file) {
			input = "";
			while(std::getline(file, line)) input += line + "\n";
			co_yield input;
			std::println("Digite o nome do arquivo de entrada.");
		} else throw std::runtime_error("Falha ao ler o arquivo");
	}
}
