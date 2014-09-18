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

	StatementResult ret = this->executeStatements(this->statements, nullptr);

	return true;
}

StatementResult Interpreter::executeStatements(const Json::Value& statements, const LocalEnvironment* env) {
	
	StatementResult ret;
	int size = statements.size();

	for (int i = 0; i < size; i++) {
		ret = this->executeStatement(statements[i], env);
	}

	return ret;
}

StatementResult Interpreter::executeStatement(const Json::Value& statement, const LocalEnvironment* env) {

	StatementResult ret;
	if (statement.isMember("type")) {
		std::string type = statement["type"].asString();
		if (type.compare("declare") == 0) {
			ret = this->executeDeclareStatement(statement, env);
		} else if (type.compare("echo") == 0) {
			ret = this->executeEchoStatement(statement, env);
		}
	}

	return ret;
}

StatementResult Interpreter::executeDeclareStatement(const Json::Value& statement, const LocalEnvironment* env) {

	StatementResult ret;

	if (statement.isMember("variables")) {
		Json::Value variables = statement["variables"];
		std::string type = statement["data-type"].asString();

		int size = variables.size();

		for (int i = 0; i < size; i++) {
			Json::Value variable = variables[i];
			ZephirValue value;
			if (variable.isMember("expr")) {
				StatementResult result = this->executeExpressionStatement(variable["expr"], env);
				value = result.getValue();
			}

			this->addVariable(variable["variable"].asString(), value, env);
		}
	}

	return ret;
}

StatementResult Interpreter::executeEchoStatement(const Json::Value& statement, const LocalEnvironment* env) {

	StatementResult ret;
	if (statement.isMember("expressions")) {
		Json::Value expressions = statement["expressions"];
	}

	return ret;
}

StatementResult Interpreter::executeExpressionStatement(const Json::Value& statement, const LocalEnvironment* env) {
	
	StatementResult ret;
	if (statement.isMember("type")) {
		std::string type = statement["type"].asString();
		
		if (type.compare("string") == 0) {
			ZephirValue value(ZephirValue::TYPE::STRING_VALUE, statement["value"].asString());
			ret.setType(StatementResult::TYPE::NORMAL_RESULT);
			ret.setValue(value);
		}
	}

	return ret;
}

void Interpreter::addVariable(const std::string& name, const ZephirValue& value, const LocalEnvironment* env) {
	std::cout << "addVariable name:" << name << ", value: " << value << std::endl;
	if (env) {
	} else {
		this->global_variables[name] = value;
	}
}
