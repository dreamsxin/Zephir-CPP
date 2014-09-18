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

	this->statements = this->compiler.parse(filename);

	std::cout << this->statements << std::endl;



	std::cout << "run" << std::endl;

	return true;
}

StatementResult Interpreter::execute_statements(const Json::Value& statements) {

	StatementResult ret;
	int size = statements.size();

	for (int i = 0; i < size; i++) {
		ret = this->execute_statement(statements[i]);
	}

	return ret;
}

StatementResult Interpreter::execute_statement(const Json::Value& statement) {

	StatementResult ret;
	if (statement.isMember("type")) {

	}

	return ret;
}
