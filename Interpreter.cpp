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
	}
	return false;
}

StatementResult Interpreter::executeStatements(const Json::Value& statements, LocalEnvironment * const env) {


	int size = statements.size();

	for (int i = 0; i < size; i++) {
		StatementResult result = this->executeStatement(statements[i], env);
		switch (result.getType()) {
			case StatementResult::BREAK_RESULT:
			case StatementResult::RETURN_RESULT:
			case StatementResult::CONTINUE_RESULT:
				return result;
			default:
				break;
		}
	}
	StatementResult ret;
	ret.setType(StatementResult::NORMAL_RESULT);
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
		} else if (type.compare("if") == 0) {
			ret = this->executeIfStatement(statement, env);
		} else if (type.compare("while") == 0) {
			ret = this->executeWhileStatement(statement, env);
		} else if (type.compare("return") == 0) {
			ret = this->executeExpressionStatement(statement["expr"], env);
		} else if (type.compare("break") == 0) {
			ret.setType(StatementResult::BREAK_RESULT);
		} else if (type.compare("continue") == 0) {
			ret.setType(StatementResult::CONTINUE_RESULT);
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

	if (assign_type.compare("decr") == 0) {
		std::string name = statement["variable"].asString();
		ZephirValue value = this->getVariable(name, env);
		value--;
		this->addVariable(name, value, env);
		ret.setValue(value);
	} else if (assign_type.compare("variable") == 0) {
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

StatementResult Interpreter::executeIfStatement(const Json::Value& statement, LocalEnvironment * const env) {

	StatementResult ret;
	if (statement.isMember("expr")) {
		StatementResult result = this->executeExpressionStatement(statement["expr"], env);
		ZephirValue value = result.getValue();

		if (value.asBool()) {
			return this->executeStatements(statement["statements"], env);
		}
	}

	return ret;
}

StatementResult Interpreter::executeWhileStatement(const Json::Value& statement, LocalEnvironment * const env) {

	StatementResult ret;
	if (statement.isMember("expr")) {
		while (1) {
			StatementResult result = this->executeExpressionStatement(statement["expr"], env);
			ZephirValue value = result.getValue();
			if (value.asBool()) {
				StatementResult result = this->executeStatements(statement["statements"], env);

				switch (result.getType()) {
					case StatementResult::BREAK_RESULT:
					case StatementResult::RETURN_RESULT:
						return result;
					default:
						continue;
				}
			} else {
				break;
			}
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
		} else if (type.compare("int") == 0) {
			ZephirValue value(ZephirValue::INT_VALUE, std::stoi(statement["value"].asString()));
			ret.setType(StatementResult::NORMAL_RESULT);
			ret.setValue(value);
		} else if (type.compare("variable") == 0) {
			std::string name = statement["value"].asString();
			ZephirValue value = this->getVariable(name, env);
			if (value.getType() == ZephirValue::UNDEFINED_VALUE) {
				throw RuntimeError(RuntimeError::VARIABLE_NOT_FOUND, statement);
			} else {
				ret.setValue(value);
			}
		} else if (type.compare("greater") == 0) {
			StatementResult left = this->executeExpressionStatement(statement["left"], env);
			StatementResult right = this->executeExpressionStatement(statement["right"], env);

			ZephirValue value;
			value.setType(ZephirValue::BOOLEAN_VALUE);
			value.setValue(left.getValue() > right.getValue());
			ret.setValue(value);
		} else if (type.compare("add") == 0) {
			StatementResult left = this->executeExpressionStatement(statement["left"], env);
			StatementResult right = this->executeExpressionStatement(statement["right"], env);

			ZephirValue value = left.getValue() + right.getValue();

			ret.setValue(value);
		} else if (type.compare("mod") == 0) {
			StatementResult left = this->executeExpressionStatement(statement["left"], env);
			StatementResult right = this->executeExpressionStatement(statement["right"], env);

			ZephirValue value = (left.getValue() % right.getValue());

			ret.setValue(value);
		} else if (type.compare("equals") == 0) {
			StatementResult left = this->executeExpressionStatement(statement["left"], env);
			StatementResult right = this->executeExpressionStatement(statement["right"], env);

			ZephirValue value(ZephirValue::BOOLEAN_VALUE, left.getValue() == right.getValue());
			ret.setValue(value);
		} else if (type.compare("mcall") == 0) {
			StatementResult result = this->executeExpressionStatement(statement["variable"], env);

			ZephirValue value = this->callMethod(result.getValue(), statement["name"].asString(), env);

			ret.setValue(value);
		} else if (type.compare("fcall") == 0) {
			std::string name = statement["name"].asString();
			std::cout << "fcall " << name << std::endl;
			this->callFunction(name, env);
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

ZephirValue Interpreter::getVariable(const std::string& name, LocalEnvironment* env) {
	ZephirValue value;
	if (env) {
		value = env->getVariable(name);
	}

	if (value.getType() == ZephirValue::UNDEFINED_VALUE) {
		if (this->global_variables.find(name) != this->global_variables.end()) {
			value = this->global_variables[name];
		} else {
			value.setType(ZephirValue::UNDEFINED_VALUE);
		}
	}

	return value;
}

ZephirValue Interpreter::callMethod(const ZephirValue& value, const std::string& method, LocalEnvironment * const env) {

	ZephirValue ret;

	if (value.isString()) {
		ret = this->callStringMethod(value, method, env);
	}

	return ret;
}

ZephirValue Interpreter::callStringMethod(const ZephirValue& value, const std::string& method, LocalEnvironment * const env) {

	ZephirValue ret;

	if (method.compare("length") == 0) {
		ret.setType(ZephirValue::INT_VALUE);
		ret.setValue((int) value.asString().length());
	}

	return ret;
}

ZephirValue Interpreter::callFunction(const std::string& method, LocalEnvironment * const env) {

	ZephirValue ret;

	std::cout << "callFunction" << this->statements.size() << std::endl;
	
	int length = this->statements.size();
	
	for (int i = 0; i < length; i++) {
		Json::Value statement = this->statements[i];
		if (statement.isMember("type")) {
			std::string type = statement["type"].asString();
			if (type.compare("function") == 0) {
				std::string name = statement["name"].asString();
				if (name.compare(method) == 0) {
					break;
				}
			}
			
		}
		
	}

	return ret;
}
