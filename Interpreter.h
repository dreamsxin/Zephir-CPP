/* 
 * File:   Interpreter.h
 * Author: Dreamszhu
 *
 * Created on September 17, 2014, 10:18 AM
 */

#ifndef INTERPRETER_H
#define	INTERPRETER_H

#include <string>
#include <stack>

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp> 

#include "json/json.h"
#include "Compiler.h"
#include "interpreter/StatementResult.h"
#include "interpreter/ZephirValue.h"
#include "interpreter/LocalEnvironment.h"

using namespace boost::filesystem;

class Interpreter {
public:
	Interpreter(const Compiler &compiler);
	virtual ~Interpreter();
	
	bool run(const std::string& filename);

private:
	StatementResult executeStatements(const Json::Value& statements, LocalEnvironment* const env);
	StatementResult executeStatement(const Json::Value& statement, LocalEnvironment* const env);
	StatementResult executeDeclareStatement(const Json::Value& statement, LocalEnvironment* const env);
	StatementResult executeLetStatement(const Json::Value& statement, LocalEnvironment* const env);
	StatementResult executeAssignmentStatement(const Json::Value& statement, LocalEnvironment* const env);
	StatementResult executeEchoStatement(const Json::Value& statement, LocalEnvironment* const env);
	StatementResult executeIfStatement(const Json::Value& statement, LocalEnvironment* const env);
	StatementResult executeWhileStatement(const Json::Value& statement, LocalEnvironment* const env);
	StatementResult executeExpressionStatement(const Json::Value& statement, LocalEnvironment* const env);

	void addVariable(const std::string& name, const ZephirValue& value, LocalEnvironment* const env);
	ZephirValue getVariable(const std::string& name, LocalEnvironment* const env);
	ZephirValue callMethod(const ZephirValue& value, const std::string& method, LocalEnvironment* const env);
	ZephirValue callStringMethod(const ZephirValue& value, const std::string& method, LocalEnvironment* const env);

private:
	Compiler compiler;
	Json::Value statements;
	boost::unordered_map<std::string, ZephirValue> global_variables;
	std::stack<ZephirValue> eval_stack;

};

#endif	/* INTERPRETER_H */

