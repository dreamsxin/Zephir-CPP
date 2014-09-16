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

#include "parser.h"

using namespace std;

static void show_usage(std::string name) {
	std::cerr << "Zephir version 0.5\n\n"
			<< "Usage: "
			<< "\tcommand [options]\n\n"
			<< boost::format("\t%-20s%s\n") % "help" % "Displays this help"
			<< boost::format("\t%-20s%s\n") % "init [namespace]" % "Initializes a Zephir extension"
			<< boost::format("\t%-20s%s\n") % "generate" % "Generates C code from the Zephir code"
			<< boost::format("\t%-20s%s\n") % "compile" % "Compile a Zephir extension"
			<< boost::format("\t%-20s%s\n") % "install" % "Installs the extension (requires root password)"
			<< boost::format("\t%-20s%s\n") % "build" % "Generate/Compile/Install a Zephir extension"
			<< boost::format("\t%-20s%s\n") % "fullclean" % "Cleans the generated object files in compilation"
			<< boost::format("\t%-20s%s\n") % "clean" % "Cleans the generated object files in compilation"
			<< std::endl;
}

/*
 * 
 */
int main(int argc, char** argv) {

	FILE *fp;
	char ch;
	char *program;
	int i, length;

	if (argc < 2) {
		show_usage(argv[0]);
		return 1;
	}

	if (argc > 0) {

		fp = fopen(argv[1], "r");
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

		xx_parse_program(program, i - 1, argv[1]);

		free(program);
	}

	return 0;
}
