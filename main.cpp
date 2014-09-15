/* 
 * File:   main.cpp
 * Author: Dreamszhu
 *
 * Created on September 15, 2014, 9:08 AM
 */

#include<stdlib.h>
#include <stdio.h>
#include "parser.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {

	FILE *fp;
	char ch;
	char *program;
	int i, length;

	if (argc > 0) {

		fp = fopen(argv[1], "r");
		if (!fp) {
			fprintf(stderr, "Cant open file\n");
			exit(1);
		}

		length = 1024;
		program = (char *)malloc(sizeof(char) * length);

		i = 0;
		while (!feof(fp)) {
			ch = fgetc(fp);
			if (i == length) {
				length += 1024;
				program = (char *)realloc(program, sizeof(char) * length);
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

