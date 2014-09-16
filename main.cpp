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

/*
 * 
 */
int main(int argc, char** argv) {

	std::string appname = boost::filesystem::basename(argv[0]);
	
	boost::program_options::options_description desc("Zephir-CPP version 0.1\n\nUsage");
	desc.add_options()
			("help,h", "describe arguments")
			("init", boost::program_options::value<std::string>(), "Initializes a Zephir extension")
			("generate", "Generates C code from the Zephir code")
			("compile", "Compile a Zephir extension")
			("install", "Installs the extension (requires root password)")
			("build", "Generate/Compile/Install a Zephir extension")
			("fullclean", "Cleans the generated object files in compilation")
			("clean", "Cleans the generated object files in compilation");

	boost::program_options::variables_map vm;
	
	try {
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);

		if (vm.count("generate")) {
			std::cout << "generate" << "\n";
		} else if (vm.count("compile")) {
			std::cout << "compile" << "\n";
		} else if (vm.count("help")) {
			std::cout << desc << "\n";
		} else {
			std::cout << desc << "\n";
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
