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

using namespace boost::filesystem;

class Compiler {
public:
	Compiler(const path& app_path, const path& ext_path);
	virtual ~Compiler();
	
	bool init(const std::string& ns);
	bool generate();

private:
	path app_path;
	path ext_path;
	std::string ext_namespace;

private:
	bool recursiveProcess(const path& source, const path& dest);
};

#endif	/* COMPILER_H */

