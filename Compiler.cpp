/* 
 * File:   Compiler.cpp
 * Author: Dreamszhu
 * 
 * Created on September 16, 2014, 11:12 AM
 */
#define BOOST_NO_CXX11_SCOPED_ENUMS

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "Compiler.h"

using namespace boost::filesystem;

Compiler::Compiler(const path& app_path, const path& ext_path) {
	this->app_path = app_path;
	this->ext_path = ext_path;
}

Compiler::~Compiler() {
}

bool Compiler::init(const std::string& ns) {
	this->ext_namespace = ns;

	this->ext_path /= boost::to_lower_copy(this->ext_namespace);

	if (!exists(this->ext_path)) {
		if (!create_directory(this->ext_path)) {
			std::cerr << "Failed to create directory " << this->ext_path << ". Please check your folder permissions" << std::endl;
			return false;
		} else {
			std::cout << "[OK]Success to create directory " << this->ext_path << "." << std::endl;
		}
	} else {
		std::cout << "Already exists directory " << this->ext_path << std::endl;
	}

	path app_ext_path = this->app_path / "ext";
	if (!exists(app_ext_path)) {
		std::cerr << "Does not exist or is not a directory " << app_ext_path << std::endl;
		return false;
	}

	path ext_ext_path = this->ext_path / "ext";
	if (!exists(ext_ext_path)) {
		if (!this->recursiveProcess(app_ext_path, ext_ext_path)) {
			return false;
		}
		std::cout << "[OK]Success to create directory " << ext_ext_path << "." << std::endl;
	} else {
		std::cout << "Already exists directory " << ext_ext_path << std::endl;
	}
}

bool Compiler::recursiveProcess(path const & source, path const & dest) {
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
