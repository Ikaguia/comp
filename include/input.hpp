#pragma once

#include <generator>
#include <string>

std::generator<std::string> gen_input();
std::generator<std::string> gen_inputManual();
std::generator<std::string> gen_inputFromFile();
