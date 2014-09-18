/* 
 * File:   Interpreter.h
 * Author: Dreamszhu
 *
 * Created on September 17, 2014, 10:18 AM
 */

#ifndef INTERPRETER_H
#define	INTERPRETER_H

#include <string>

#include <boost/filesystem.hpp>

#include "json/json.h"
#include "Compiler.h"

using namespace boost::filesystem;

class Interpreter {
public:
	Interpreter(const Compiler &compiler);
	virtual ~Interpreter();
	
	bool run(const std::string& filename);

private:
	Compiler compiler;
	Json::Value statements;

};

#endif	/* INTERPRETER_H */

