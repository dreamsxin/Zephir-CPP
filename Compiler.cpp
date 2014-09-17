/* 
 * File:   Compiler.cpp
 * Author: Dreamszhu
 * 
 * Created on September 16, 2014, 11:12 AM
 */
#define BOOST_NO_CXX11_SCOPED_ENUMS

#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include "parser.h"
#include "Compiler.h"

Compiler::Compiler() {
}

Compiler::Compiler(const path& app_path, const path& run_path) {
	this->app_path = app_path;
	this->run_path = run_path;
}

Compiler::~Compiler() {
}

bool Compiler::init(const std::string& ns) {
	this->ext_namespace = ns;

	this->run_path /= boost::to_lower_copy(this->ext_namespace);

	if (!exists(this->run_path)) {
		if (!create_directory(this->run_path)) {
			std::cerr << "Failed to create directory " << this->run_path << ". Please check your folder permissions" << std::endl;
			return false;
		} else {
			std::cout << "[OK]Success to create directory " << this->run_path << std::endl;
		}
	} else {
		std::cout << "Already exists directory " << this->run_path << std::endl;
	}

	path ext_namespace_path = this->run_path / boost::to_lower_copy(this->ext_namespace);

	if (!exists(ext_namespace_path)) {
		if (!create_directory(ext_namespace_path)) {
			std::cerr << "Failed to create directory " << ext_namespace_path << ". Please check your folder permissions" << std::endl;
			return false;
		} else {
			std::cout << "[OK]Success to create directory " << ext_namespace_path << std::endl;
		}
	} else {
		std::cout << "Already exists directory " << ext_namespace_path << std::endl;
	}

	path app_run_path = this->app_path / "ext";
	if (!exists(app_run_path)) {
		std::cerr << "Does not exist or is not a directory " << app_run_path << std::endl;
		return false;
	}

	path ext_run_path = this->run_path / "ext";
	if (!exists(ext_run_path)) {
		if (!this->recursiveProcess(app_run_path, ext_run_path)) {
			return false;
		}
		std::cout << "[OK]Success to create directory " << ext_run_path << std::endl;
	} else {
		std::cout << "Already exists directory " << ext_run_path << std::endl;
	}

	Json::Value config;

	config["name"] = this->ext_namespace;
	config["namespace"] = this->ext_namespace;
	config["description"] = "";
	config["author"] = "";
	config["version"] = "0.0.1";

	Json::FastWriter writer;
	std::string config_file = writer.write(config);

	path ext_config_path = this->run_path / "config.json";

	std::ofstream ofs;
	ofs.open(ext_config_path.generic_string());
	ofs << config_file;

	return true;
}

bool Compiler::generate() {

	path ext_config_path = this->run_path / "config.json";
	if (!exists(ext_config_path)) {
		std::cerr << "Does not exist " << ext_config_path << std::endl;
		return false;
	}

	std::ifstream ifs;
	ifs.open(ext_config_path.generic_string());

	Json::Reader reader;
	if (!reader.parse(ifs, this->config, false)) {
		std::cerr << "config.json is not valid or there is no Zephir extension initialized in this directory" << std::endl;
		return false;
	}

	std::cout << this->config << std::endl;

	return true;
}

Json::Value *Compiler::parse(const std::string& filename) {

	path full_path = this->run_path / filename;
	
	if (!exists(full_path)) {
		std::cerr << "Does not exist " << full_path << std::endl;
		return NULL;
	}

	std::ifstream ifs;
	ifs.open(full_path.generic_string());

	std::ostringstream ost;
	ost << ifs.rdbuf();
	
	std::string program = ost.str();

	return xx_parse_program((char *)program.c_str(), program.length(), (char *)filename.c_str());
}

bool Compiler::recursiveProcess(const path& source, const path& dest) {
	try {
		if (!exists(source) || !is_directory(source)) {
			std::cerr << "Source directory " << source.string() << " does not exist or is not a directory." << std::endl;
			return false;
		}
		if (exists(dest)) {
			std::cerr << "Destination directory " << dest.string() << " already exists." << std::endl;
			return false;
		}

		if (!create_directory(dest)) {
			std::cerr << "Unable to create directory" << dest.string() << std::endl;
			return false;
		}
	} catch (filesystem_error const & e) {
		std::cerr << e.what() << std::endl;
		return false;
	}

	for (directory_iterator file(source); file != directory_iterator(); ++file) {
		try {
			path current(file->path());
			if (is_directory(current)) {
				if (!this->recursiveProcess(current, dest / current.filename())) {
					return false;
				}
			} else {
				copy_file(current, dest / current.filename());
			}
		} catch (filesystem_error const & e) {
			std::cerr << e.what() << std::endl;
			return false;
		}
	}

	return true;
}
