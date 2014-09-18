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
	StatementResult executeStatements(const Json::Value& statements, const LocalEnvironment* env);
	StatementResult executeStatement(const Json::Value& statement, const LocalEnvironment* env);
	StatementResult executeDeclareStatement(const Json::Value& statement, const LocalEnvironment* env);
	StatementResult executeEchoStatement(const Json::Value& statement, const LocalEnvironment* env);
	StatementResult executeExpressionStatement(const Json::Value& statement, const LocalEnvironment* env);
	
	void addVariable(const std::string& name, const ZephirValue& value, const LocalEnvironment* env);

private:
	Compiler compiler;
	Json::Value statements;
	boost::unordered_map<std::string, boost::any> global_variables;
	std::stack<StatementResult> eval_stack;

};

#endif	/* INTERPRETER_H */

