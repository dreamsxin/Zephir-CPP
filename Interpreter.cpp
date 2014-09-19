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
#include "RuntimeError.h"

Interpreter::Interpreter(const Compiler &compiler) {
	this->compiler = compiler;
}

Interpreter::~Interpreter() {
}

bool Interpreter::run(const std::string& filename) {

	try {

		this->statements = this->compiler.parse(filename);

		std::cout << this->statements << std::endl;

		std::cout << "run" << std::endl;

		StatementResult ret = this->executeStatements(this->statements, nullptr);

		return true;
	} catch (RuntimeError e) {
		std::cerr << e.what() << std::endl;
	} catch (std::exception e) {
		std::cerr << e.what() << std::endl;
	}

	return false;
}

StatementResult Interpreter::executeStatements(const Json::Value& statements, LocalEnvironment * const env) {

	StatementResult ret;
	int size = statements.size();

	for (int i = 0; i < size; i++) {
		ret = this->executeStatement(statements[i], env);
	}

	return ret;
}

StatementResult Interpreter::executeStatement(const Json::Value& statement, LocalEnvironment * const env) {

	StatementResult ret;
	if (statement.isMember("type")) {
		std::string type = statement["type"].asString();
		if (type.compare("declare") == 0) {
			ret = this->executeDeclareStatement(statement, env);
		} else if (type.compare("let") == 0) {
			ret = this->executeLetStatement(statement, env);
		} else if (type.compare("echo") == 0) {
			ret = this->executeEchoStatement(statement, env);
		}		
	}

	return ret;
}

StatementResult Interpreter::executeDeclareStatement(const Json::Value& statement, LocalEnvironment * const env) {

	StatementResult ret;

	if (statement.isMember("variables")) {
		Json::Value variables = statement["variables"];
		std::string data_type = statement["data-type"].asString();

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

StatementResult Interpreter::executeLetStatement(const Json::Value& statement, LocalEnvironment * const env) {

	StatementResult ret;

	if (statement.isMember("assignments")) {
		Json::Value assignments = statement["assignments"];

		int size = assignments.size();

		for (int i = 0; i < size; i++) {			
			StatementResult result = this->executeAssignmentStatement(assignments[i], env);
		}
	}

	return ret;
}

StatementResult Interpreter::executeAssignmentStatement(const Json::Value& statement, LocalEnvironment * const env) {

	StatementResult ret;

	std::string assign_type = statement["assign-type"].asString();
	std::string op = statement["operator"].asString();

	if (assign_type.compare("variable") == 0) {
		std::string name = statement["variable"].asString();
		ZephirValue value;
		if (statement.isMember("expr")) {
			StatementResult result = this->executeExpressionStatement(statement["expr"], env);
			value = result.getValue();
		}

		this->addVariable(name, value, env);
	}

	return ret;
}

StatementResult Interpreter::executeEchoStatement(const Json::Value& statement, LocalEnvironment * const env) {

	StatementResult ret;
	if (statement.isMember("expressions")) {
		Json::Value expressions = statement["expressions"];
		int size = expressions.size();

		for (int i = 0; i < size; i++) {
			StatementResult result = this->executeExpressionStatement(expressions[i], env);
			ZephirValue value = result.getValue();
			std::cout << value;
		}
	}

	return ret;
}

StatementResult Interpreter::executeExpressionStatement(const Json::Value& statement, LocalEnvironment * const env) {

	StatementResult ret;
	if (statement.isMember("type")) {
		std::string type = statement["type"].asString();

		if (type.compare("string") == 0) {
			ZephirValue value(ZephirValue::STRING_VALUE, statement["value"].asString());
			ret.setType(StatementResult::NORMAL_RESULT);
			ret.setValue(value);
		} else if (type.compare("variable") == 0) {
			std::string name = statement["value"].asString();
			ZephirValue *value = this->getVariable(name, env);
			if (nullptr == value) {
				throw RuntimeError(RuntimeError::VARIABLE_NOT_FOUND, statement);
			}
			ret.setValue(*value);
		} else if (type.compare("add") == 0) {
			StatementResult left = this->executeExpressionStatement(statement["left"], env);
			StatementResult right = this->executeExpressionStatement(statement["right"], env);
			
			ZephirValue value = left.getValue() + right.getValue();
			
			ret.setValue(value);
		}
	}

	return ret;
}

void Interpreter::addVariable(const std::string& name, const ZephirValue& value, LocalEnvironment * const env) {
	if (env) {
		env->addVariable(name, value);
	} else {
		this->global_variables[name] = value;
	}
}

ZephirValue* Interpreter::getVariable(const std::string& name, LocalEnvironment* env) {
	ZephirValue *value = nullptr;
	if (env) {
		value = env->getVariable(name);
	}

	if (nullptr == value) {
		if (this->global_variables.find(name) != this->global_variables.end()) {
			value = &this->global_variables[name];
		} else {
			value = nullptr;
		}
	}

	return value;
}
