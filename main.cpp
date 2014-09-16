/* 
 * File:   main.cpp
 * Author: Dreamszhu
 *
 * Created on September 15, 2014, 9:08 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "parser.h"
#include "Compiler.h"

/*
 * 
 */
int main(int argc, char** argv) {

	std::string ns;
	
	boost::program_options::options_description generic_desc("GENERIC");
	generic_desc.add_options()
		("help,H", "Describe arguments")
		("version,V", "Print version and exit");
	
	boost::program_options::options_description command_desc("COMMANDS");
	command_desc.add_options()
			("init", boost::program_options::value<std::string>(&ns), "Initializes a Zephir extension")
			("generate", "Generates C code from the Zephir code")
			("compile", "Compile a Zephir extension")
			("install", "Installs the extension (requires root password)")
			("build", "Generate/Compile/Install a Zephir extension")
			("fullclean", "Cleans the generated object files in compilation")
			("clean", "Cleans the generated object files in compilation");
	
	boost::program_options::options_description desc;
	desc.add(generic_desc).add(command_desc);

	boost::program_options::variables_map vm;
	
	try {
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);
		
		boost::filesystem::path app_path( argv[0] );
		boost::filesystem::path run_path( boost::filesystem::current_path() );

		if (vm.count("init")) {
			std::cout << "app_path: " << app_path.branch_path().branch_path() << std::endl;
			std::cout << "run_path: " << run_path << std::endl;
			Compiler compiler(app_path.branch_path().branch_path(), run_path);
			compiler.init(ns);
		} else if (vm.count("generate")) {
			std::cout << "generate" << std::endl;
		} else if (vm.count("compile")) {
			std::cout << "compile" << std::endl;
		} else if (vm.count("version")) {
			std::cout << "Zephir-CPP version 0.1" << std::endl;
		} else {
			std::cout << "Zephir-CPP version 0.1\n\nUsage" << std::endl;
			std::cout << desc << std::endl;
		}
	} catch (boost::program_options::required_option& e) {
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		return 1;
	} catch (boost::program_options::error& e) {
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		return 1;
	}

	return 0;
}

int parser(char* file) {
	FILE *fp;
	char ch;
	char *program;
	int i, length;

	fp = fopen(file, "r");
	if (!fp) {
		fprintf(stderr, "Cant open file\n");
		exit(1);
	}

	length = 1024;
	program = (char *) malloc(sizeof (char) * length);

	i = 0;
	while (!feof(fp)) {
		ch = fgetc(fp);
		if (i == length) {
			length += 1024;
			program = (char *) realloc(program, sizeof (char) * length);
		}
		program[i++] = ch;
	}
	program[i - 1] = '\0';
	fclose(fp);

	xx_parse_program(program, i - 1, file);

	free(program);

	return 0;
}
