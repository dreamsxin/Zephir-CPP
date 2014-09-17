/* 
 * File:   Interpreter.cpp
 * Author: Dreamszhu
 * 
 * Created on September 17, 2014, 10:18 AM
 */

#include <fstream>
#include <sstream>

#include "parser.h"
#include "Interpreter.h"
#include "Compiler.h"

Interpreter::Interpreter(const Compiler &compiler) {
	this->compiler = compiler;
}

Interpreter::~Interpreter() {
}

bool Interpreter::run(const std::string& filename) {

	Json::Value *value = this->compiler.parse(filename);

	std::cout << *(value) << std::endl;
	std::cout << "run" << std::endl;
}