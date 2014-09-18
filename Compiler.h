/* 
 * File:   Compiler.h
 * Author: Dreamszhu
 *
 * Created on September 16, 2014, 11:12 AM
 */

#ifndef COMPILER_H
#define	COMPILER_H

#include <string>

#include <boost/filesystem.hpp>

#include "json/json.h"

using namespace boost::filesystem;

class Compiler {
public:
	Compiler();
	Compiler(const path& app_path, const path& run_path);
	virtual ~Compiler();
	
	bool init(const std::string& ns);
	bool generate();
	Json::Value parse(const std::string& filename);

private:
	bool recursiveProcess(const path& source, const path& dest);

private:
	path app_path;
	path run_path;
	std::string ext_namespace;
	Json::Value config;
};

#endif	/* COMPILER_H */

