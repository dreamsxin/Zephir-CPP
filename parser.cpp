/** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
// 54 "parser.lemon"


#include <cstddef>
#include <string>
#include <iostream>
#include <sstream>

#include "json/json.h"

#include "string.h"
#include "parser.h"
#include "scanner.h"
#include "xx.h"

using namespace std;

static Json::Value* xx_ret_literal(int type, xx_parser_token *T, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	switch (type) {

		case XX_T_CONSTANT:
			(*ret)["type"] = "constant";
			break;

		case XX_T_IDENTIFIER:
			(*ret)["type"] = "variable";
			break;

		case XX_T_INTEGER:
			(*ret)["type"] = "int";
			break;

		case XX_T_DOUBLE:
			(*ret)["type"] = "double";
			break;

		case XX_T_NULL:
			(*ret)["type"] = "null";
			break;

		case XX_T_STRING:
			(*ret)["type"] = "string";
			break;

		case XX_T_CHAR:
			(*ret)["type"] = "char";
			break;

		default:
			if (type == XX_T_TRUE) {
				(*ret)["type"] = "bool";
				(*ret)["value"] = "true";
			} else {
				if (type == XX_T_FALSE) {
					(*ret)["type"] = "bool";
					(*ret)["value"] = "false";
				} else {
					fprintf(stderr, "literal??\n");
				}
			}
	}

	if (T) {
		(*ret)["value"] = T->token;
		delete T;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_expr(const char *type, Json::Value* left, Json::Value* right, Json::Value* extra, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = type;

	if (nullptr != left) {
		(*ret)["left"] = *left;
		delete left;
	}
	if (nullptr != right) {
		(*ret)["right"] = *right;
		delete right;
	}
	if (nullptr != extra) {
		(*ret)["extra"] = *extra;
		delete extra;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_array_item(Json::Value* key, Json::Value* value, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	if (nullptr != key) {
		(*ret)["key"] = key;
		delete key;
	}
	(*ret)["value"] = *value;
	delete value;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_namespace(xx_parser_token *T, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "namespace";
	(*ret)["name"] = T->token;
	delete T;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_use_aliases(Json::Value* use_aliases_list, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "use";
	(*ret)["aliases"] = *use_aliases_list;
	delete use_aliases_list;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_use_aliases_item(xx_parser_token *T, xx_parser_token *A, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["name"] = T->token;
	delete T;
	if (A) {
		(*ret)["alias"] = A->token;
		delete A;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_class(xx_parser_token *T, Json::Value* class_definition, int is_abstract, int is_final,
	xx_parser_token *E, Json::Value* I, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "class";
	(*ret)["name"] = T->token;
	delete T;

	(*ret)["abstract"] = is_abstract;
	(*ret)["final"] = is_final;

	if (E) {
		(*ret)["extends"] = E->token;
		delete E;
	}

	if (nullptr != I) {
		(*ret)["implements"] = *I;
		delete I;
	}

	if (nullptr != class_definition) {
		(*ret)["definition"] = *class_definition;
		delete class_definition;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_interface(xx_parser_token *T, Json::Value* interface_definition, xx_parser_token *E, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "interface";
	(*ret)["name"] = T->token;
	delete T;

	if (E) {
		(*ret)["definition"] = E->token;
		delete E;
	}

	if (nullptr != interface_definition) {
		(*ret)["definition"] = *interface_definition;
		delete interface_definition;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_class_definition(Json::Value* properties, Json::Value* methods, Json::Value* constants, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	if (nullptr != properties) {
		(*ret)["properties"] = *properties;
		delete properties;
	}
	if (nullptr != methods) {
		(*ret)["methods"] = *methods;
		delete methods;
	}
	if (nullptr != constants) {
		(*ret)["constants"] = *constants;
		delete constants;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_interface_definition(Json::Value* methods, Json::Value* constants, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	if (nullptr != methods) {
		(*ret)["methods"] = *methods;
		delete methods;
	}
	if (nullptr != constants) {
		(*ret)["constants"] = *constants;
		delete constants;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_class_property(Json::Value* visibility, xx_parser_token *T,
		Json::Value* default_value, xx_parser_token *D, Json::Value* shortcuts, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["visibility"] = visibility;
	delete visibility;
	(*ret)["property"] = "property";
	(*ret)["name"] = T->token;
	delete T;

	if (nullptr != default_value) {
		(*ret)["default"] = *default_value;
		delete default_value;
	}

	if (D) {
		(*ret)["docblock"] = D->token;
		delete D;
	}

	if (nullptr != shortcuts) {
		(*ret)["shortcuts"] = *shortcuts;
		delete shortcuts;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_property_shortcut(xx_parser_token *C, xx_parser_token *D, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "shortcuts";
	if (C) {
		(*ret)["docblock"] = C->token;
		delete C;
	}
	(*ret)["name"] = D->token;
	delete D;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_class_const(xx_parser_token *T, Json::Value* default_value, xx_parser_token *D, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "const";
	(*ret)["name"] = T->token;
	delete T;
	(*ret)["default"] = default_value;
	delete default_value;

	if (D) {
		(*ret)["docblock"] = D->token;
		delete D;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_class_method(Json::Value* visibility, xx_parser_token *T, Json::Value* parameters,
	Json::Value* statements, xx_parser_token *D, Json::Value* return_type, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["visibility"] = visibility;
	delete visibility;
	(*ret)["type"] = "method";
	(*ret)["name"] = T->token;
	delete T;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
		delete parameters;
	}

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}

	if (D) {
		(*ret)["docblock"] = D->token;
		delete D;
	}

	if (nullptr != return_type) {
		(*ret)["return-type"] = *return_type;
		delete return_type;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_function_definition(Json::Value* return_type, xx_parser_token *T, Json::Value* parameters, Json::Value* statements, xx_parser_token *D, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();
	(*ret)["type"] = "function";
	(*ret)["name"] = T->token;

	if (nullptr != return_type) {
		(*ret)["return-type"] = *return_type;
		delete return_type;
	}

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
		delete parameters;
	}

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}

	if (D) {
		(*ret)["docblock"] = D->token;
		delete D;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_parameter(int const_param, Json::Value* type, Json::Value* cast, xx_parser_token *N, Json::Value* default_value,
	int mandatory, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "parameter";
	(*ret)["name"] = N->token;
	delete N;
	(*ret)["const"] = const_param;

	if (nullptr != type) {
		(*ret)["data-type"] = *type;
		(*ret)["mandatory"] = mandatory;
		delete type;
	} else {
		(*ret)["data-type"] = "variable";
		(*ret)["mandatory"] = 0;
	}

	if (nullptr != cast) {
		(*ret)["cast"] = *cast;
		delete cast;
	}
	if (nullptr != default_value) {
		(*ret)["default"] = *default_value;
		delete default_value;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_return_type(int is_void, Json::Value* return_type_list, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "return-type";

	(*ret)["list"] = *return_type_list;
	delete return_type_list;

	(*ret)["void"] = is_void;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_return_type_item(Json::Value* type, Json::Value* cast, int mandatory, int collection, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "return-type-parameter";

	if (nullptr != type) {
		(*ret)["data-type"] = *type;
		(*ret)["mandatory"] = mandatory;
		delete type;
	}

	if (nullptr != cast) {
		(*ret)["cast"] = *cast;
		(*ret)["collection"] = collection;
		delete cast;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_type(int type)
{
	string name;

	switch (type) {
		case XX_TYPE_INTEGER:
			name = "int";
			break;
		case XX_TYPE_UINTEGER:
			name = "uint";
			break;
		case XX_TYPE_DOUBLE:
			name = "double";
			break;
		case XX_TYPE_BOOL:
			name = "bool";
			break;
		case XX_TYPE_LONG:
			name = "long";
			break;
		case XX_TYPE_ULONG:
			name = "ulong";
			break;
		case XX_TYPE_STRING:
			name = "string";
			break;
		case XX_TYPE_CHAR:
			name = "char";
			break;
		case XX_TYPE_ARRAY:
			name = "array";
			break;
		case XX_TYPE_VAR:
			name = "variable";
			break;
		case XX_TYPE_CALLABLE:
			name = "callable";
			break;
		case XX_TYPE_RESOURCE:
			name = "resource";
			break;
		case XX_TYPE_OBJECT:
			name = "object";
			break;
		case XX_T_TYPE_NULL:
			name = "null";
			break;
		case XX_T_TYPE_THIS:
			name = "this";
			break;
		default:
			fprintf(stderr, "unknown type?\n");
	}

	return new Json::Value(name);
}

static Json::Value* xx_ret_list(Json::Value* list_left, Json::Value* right_list)
{
	Json::Value* ret = new Json::Value();
	int i, array_length;

	if (nullptr != list_left) {
		array_length = list_left->size();
		if (array_length > 0) {
			for (i = 0; i < array_length; i++) {
				ret->append((*list_left)[i]);
			}
		} else {
			ret->append(*list_left);
		}
		delete list_left;
	}

	if (nullptr != right_list) {
		ret->append(*right_list);
		delete right_list;
	}

	return ret;
}

static Json::Value* xx_ret_let_statement(Json::Value* assignments, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "let";
	(*ret)["assignments"] = *assignments;
	delete assignments;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_let_assignment(const char *type, Json::Value* op, xx_parser_token *V, xx_parser_token *P, Json::Value* index_expr, Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["assign-type"] = type;
	if (nullptr != op) {
		(*ret)["operator"] = *op;
		delete op;
	}
	(*ret)["variable"] = V->token;
	delete V;
	if (P) {
		(*ret)["property"] = P->token;
		delete P;
	}
	if (nullptr != index_expr) {
		(*ret)["index-expr"] = *index_expr;
		delete index_expr;
	}
	if (nullptr != expr) {
		(*ret)["expr"] = *expr;
		delete expr;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_if_statement(Json::Value* expr, Json::Value* statements, Json::Value* elseif_statements, Json::Value* else_statements, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "if";
	(*ret)["expr"] = *expr;
	delete expr;

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}

	if (nullptr != elseif_statements) {
		(*ret)["elseif_statements"] = *elseif_statements;
		delete elseif_statements;
	}

	if (nullptr != else_statements) {
		(*ret)["else_statements"] = *else_statements;
		delete else_statements;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_switch_statement(Json::Value* expr, Json::Value* clauses, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "switch";
	(*ret)["expr"] = *expr;
	delete expr;

	if (nullptr != clauses) {
		(*ret)["clauses"] = *clauses;
		delete clauses;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_case_clause(Json::Value* expr, Json::Value* statements, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	if (nullptr != expr) {
		(*ret)["type"] = "case";
		(*ret)["expr"] = *expr;
		delete expr;
	} else {
		(*ret)["type"] = "default";
	}

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_while_statement(Json::Value* expr, Json::Value* statements, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "while";
	(*ret)["expr"] = *expr;
	delete expr;

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_do_while_statement(Json::Value* expr, Json::Value* statements, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "do-while";
	(*ret)["expr"] = *expr;
	delete expr;

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_try_catch_statement(Json::Value* statements, Json::Value* catches, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "try-catch";

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}
	if (nullptr != catches) {
		(*ret)["catches"] = *catches;
		delete catches;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_catch_statement(Json::Value* classes, Json::Value* variable, Json::Value* statements, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	if (nullptr != classes) {
		(*ret)["classes"] = *classes;
		delete classes;
	}

	if (nullptr != variable) {
		(*ret)["variable"] = *variable;
		delete variable;
	}

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_for_statement(Json::Value* expr, xx_parser_token *K, xx_parser_token *V, int reverse, Json::Value* statements, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "for";
	(*ret)["expr"] = *expr;
	delete expr;

	if (K) {
		(*ret)["key"] = K->token;
		delete K;
	}
	if (V) {
		(*ret)["value"] = V->token;
		delete V;
	}

	(*ret)["reverse"] = reverse;

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_loop_statement(Json::Value* statements, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "loop";

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
		delete statements;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_empty_statement(xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "empty";

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_break_statement(xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "break";

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_continue_statement(xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "continue";

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_echo_statement(Json::Value* expressions, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "echo";
	(*ret)["expressions"] = *expressions;
	delete expressions;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_return_statement(Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "return";
	if (nullptr != expr) {
		(*ret)["expr"] = *expr;
		delete expr;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_require_statement(Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "require";
	(*ret)["expr"] = *expr;
	delete expr;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_fetch_statement(Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "fetch";
	(*ret)["expr"] = *expr;
	delete expr;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_fcall_statement(Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "fcall";
	(*ret)["expr"] = *expr;
	delete expr;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_mcall_statement(Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "mcall";
	(*ret)["expr"] = *expr;
	delete expr;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_scall_statement(Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "scall";
	(*ret)["expr"] = *expr;
	delete expr;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_unset_statement(Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "unset";
	(*ret)["expr"] = *expr;
	delete expr;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_declare_statement(int type, Json::Value* variables, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "declare";

	switch (type) {

		case XX_T_TYPE_INTEGER:
			(*ret)["data-type"] = "int";
			break;

		case XX_T_TYPE_UINTEGER:
			(*ret)["data-type"] = "uint";
			break;

		case XX_T_TYPE_LONG:
			(*ret)["data-type"] = "long";
			break;

		case XX_T_TYPE_ULONG:
			(*ret)["data-type"] = "ulong";
			break;

		case XX_T_TYPE_CHAR:
			(*ret)["data-type"] = "char";
			break;

		case XX_T_TYPE_UCHAR:
			(*ret)["data-type"] = "uchar";
			break;

		case XX_T_TYPE_DOUBLE:
			(*ret)["data-type"] = "double";
			break;

		case XX_T_TYPE_BOOL:
			(*ret)["data-type"] = "bool";
			break;

		case XX_T_TYPE_STRING:
			(*ret)["data-type"] = "string";
			break;

		case XX_T_TYPE_ARRAY:
			(*ret)["data-type"] = "array";
			break;

		case XX_T_TYPE_VAR:
			(*ret)["data-type"] = "variable";
			break;

		case XX_T_TYPE_CALLABLE:
			(*ret)["data-type"] = "callable";
			break;

		case XX_T_TYPE_RESOURCE:
			(*ret)["data-type"] = "resource";
			break;

		case XX_T_TYPE_OBJECT:
			(*ret)["data-type"] = "object";
			break;

		default:
			fprintf(stderr, "err 2?\n");
	}

	(*ret)["variables"] = *variables;
	delete variables;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_declare_variable(xx_parser_token *T, Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["variable"] = T->token;
	delete T;
	if (nullptr != expr) {
		(*ret)["expr"] = *expr;
		delete expr;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_new_instance(int dynamic, xx_parser_token *T, Json::Value* parameters, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "new";
	(*ret)["class"] = T->token;
	delete T;
	(*ret)["dynamic"] = dynamic;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
		delete parameters;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_throw_exception(Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "throw";
	if (nullptr != expr) {
		(*ret)["expr"] = *expr;
		delete expr;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_fcall(int type, xx_parser_token *F, Json::Value* parameters, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "fcall";
	(*ret)["name"] = F->token;
	delete F;
	(*ret)["call-type"] = type;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
		delete parameters;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_mcall(int type, Json::Value* O, xx_parser_token *M, Json::Value* parameters, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "mcall";
	(*ret)["variable"] = *O;
	delete O;
	(*ret)["name"] = M->token;
	delete M;
	(*ret)["call-type"] = type;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
		delete parameters;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_scall(int dynamic_class, xx_parser_token *O, int dynamic_method, xx_parser_token *M, Json::Value* parameters, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "scall";
	(*ret)["dynamic-class"] = dynamic_class;
	(*ret)["class"] = O->token;
	delete O;
	(*ret)["dynamic"] = dynamic_method;
	(*ret)["name"] = M->token;
	delete M;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
		delete parameters;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_call_parameter(xx_parser_token *N, Json::Value* parameter, xx_scanner_state *state, int reference)
{
	Json::Value* ret = new Json::Value();

	if (N) {
		(*ret)["name"] = N->token;
		delete N;
	}
	(*ret)["parameter"] = *parameter;
	delete parameter;
	if (reference) {
		(*ret)["reference"] = reference;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_comment(xx_parser_token *T, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "comment";
	(*ret)["value"] = T->token;
	delete T;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_cblock(xx_parser_token *T, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "cblock";
	(*ret)["value"] = T->token;
	delete T;

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}


// 1256 "parser.cpp"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    xx_TOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is xx_TOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.
**    xx_ARG_SDECL     A static variable declaration for the %extra_argument
**    xx_ARG_PDECL     A parameter declaration for the %extra_argument
**    xx_ARG_STORE     Code to store %extra_argument into yypParser
**    xx_ARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 223
#define YYACTIONTYPE unsigned short int
#define xx_TOKENTYPE xx_parser_token*
typedef union {
  xx_TOKENTYPE yy0;
  Json::Value* yy78;
  int yy445;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define xx_ARG_SDECL xx_parser_status *status;
#define xx_ARG_PDECL ,xx_parser_status *status
#define xx_ARG_FETCH xx_parser_status *status = yypParser->status
#define xx_ARG_STORE yypParser->status = status
#define YYNSTATE 933
#define YYNRULE 456
#define YYERRORSYMBOL 120
#define YYERRSYMDT yy445
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* Next are that tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static YYACTIONTYPE yy_action[] = {
 /*     0 */   445,  933,   62,   65,  737,   31,   36,  739,  841,  847,
 /*    10 */    41,  846,  831,  186,  128,   73,   69,  725,  807,  237,
 /*    20 */   245,   47,   39,  702,  294,  565,   50,  139,   59,  144,
 /*    30 */    56,  164,   44,   43,  154,   34,   43,  132,  276,  677,
 /*    40 */   678,  680,  679,  681,  670,   35,   38,  171,  564,  494,
 /*    50 */   237,  245,  153,  184,  108,  226,  565,  298,  150,  535,
 /*    60 */   223,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*    70 */   490,  487,  237,  245,   32,  277,  279,  281,  188,  133,
 /*    80 */   291,  733,  630,  851,  302,  306,  311,  319,   70,  330,
 /*    90 */   684,  685,  337,  639,  705,  296,  715,   37,  607,  147,
 /*   100 */   682,  683,  215,  426,  441,  448,  451,  146,  148,  149,
 /*   110 */   151,  152,  495,  445,  813,   62,   65,   76,   78,   86,
 /*   120 */    80,   82,   84,  819,  163,  352,  814,  128,  298,  351,
 /*   130 */   535,   73,   69,  210,   47,  178,  180,  179,  143,   50,
 /*   140 */   139,   59,  144,   56,  164,   44,  350,  154,  403,  372,
 /*   150 */   132,  276,  677,  678,  680,  679,  681,  211,  212,  216,
 /*   160 */   171,  563,  494,   39,   43,  153,  184,  108,  909,  913,
 /*   170 */   914,  150,   40,  908,  454,  463,  472,  475,  466,  469,
 /*   180 */   478,  484,  481,  490,  487,  162,  342,   43,  277,  279,
 /*   190 */   281,  165,  137,  291,  209,  503,  416,  302,  306,  311,
 /*   200 */   319,  585,  330,  684,  685,  337,  998,  705,  126,  715,
 /*   210 */   133,  170,  147,  682,  683, 1363,  426,  441,  448,  451,
 /*   220 */   146,  148,  149,  151,  152,  495,  445,  752,   62,   65,
 /*   230 */   751,   72,  651,  133,  654,  668,  657,  661,  662,  747,
 /*   240 */   128,  178,  180,  179,  143,  169,  129,   47,  627,  181,
 /*   250 */   412,  414,   50,  139,   59,  144,   56,  164,   44,  355,
 /*   260 */   154,  364,  372,  132,  276,  677,  678,  680,  679,  681,
 /*   270 */   645,  825,   71,  171,  553,  494,  130,  644,  153,  184,
 /*   280 */   108,  702,  820,  826,  150,  134,  332,  454,  463,  472,
 /*   290 */   475,  466,  469,  478,  484,  481,  490,  487,  742,  711,
 /*   300 */   603,  277,  279,  281,  741,  135,  291,  222,  230,  702,
 /*   310 */   302,  306,  311,  319,  707,  330,  684,  685,  337,  736,
 /*   320 */   810,  386,  715,  392,  372,  147,  682,  683,  174,  426,
 /*   330 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*   340 */   365,   62,   65,  371,   72,  673,  140,  654,  668,  657,
 /*   350 */   661,  662,  696,  128,  178,  180,  179,  143,  237,  245,
 /*   360 */    47,  604,  181,  215,  565,   50,  139,   59,  144,   56,
 /*   370 */   164,   44,  766,  154,  571,  765,  132,  276,  677,  678,
 /*   380 */   680,  679,  681,  166,  761,  241,  171,  285,  494,  141,
 /*   390 */   238,  153,  184,  108,  729,  340,  852,  150,  429,  735,
 /*   400 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*   410 */   487,  547,  541, 1365,  277,  279,  281,  354, 1382,  291,
 /*   420 */   214,  173,  170,  302,  306,  311,  319,  375,  330,  684,
 /*   430 */   685,  337,  989,  810,  339,  715,  200,  428,  147,  682,
 /*   440 */   683, 1364,  426,  441,  448,  451,  146,  148,  149,  151,
 /*   450 */   152,  495,  445,  187,   62,   65,  176,   72,  693,  457,
 /*   460 */   654,  668,  657,  661,  662,  759,  128,  178,  180,  179,
 /*   470 */   143,  237,  245,   47,  600,  181, 1362,  565,   50,  139,
 /*   480 */    59,  144,   56,  164,   44,  784,  154,  249,  783,  132,
 /*   490 */   276,  677,  678,  680,  679,  681,  185,  779,  456,  171,
 /*   500 */   539,  494,  286,  552,  153,  184,  108,  859,  393,  865,
 /*   510 */   150,  371,  735,  454,  463,  472,  475,  466,  469,  478,
 /*   520 */   484,  481,  490,  487,  287,  541,  172,  277,  279,  281,
 /*   530 */   295,  538,  291,  190,  323,  527,  302,  306,  311,  319,
 /*   540 */   191,  330,  684,  685,  337,  993,  808,  376,  715,  326,
 /*   550 */   526,  147,  682,  683,  417,  426,  441,  448,  451,  146,
 /*   560 */   148,  149,  151,  152,  495,  445,  404,   62,   65,  371,
 /*   570 */    72,  699,  457,  654,  668,  657,  661,  662,  791,  128,
 /*   580 */   178,  180,  179,  143,  237,  245,   47,  168,  181,  201,
 /*   590 */   565,   50,  139,   59,  144,   56,  164,   44,  798,  154,
 /*   600 */   457,  797,  132,  276,  677,  678,  680,  679,  681,  380,
 /*   610 */   793,  465,  171,  290,  494,  758,  421,  153,  184,  108,
 /*   620 */   872,  457,  875,  150,  754,  735,  454,  463,  472,  475,
 /*   630 */   466,  469,  478,  484,  481,  490,  487,  338,  425,  468,
 /*   640 */   277,  279,  281,  455,  462,  291,  464,  462,  170,  302,
 /*   650 */   306,  311,  319, 1384,  330,  684,  685,  337,  990,  810,
 /*   660 */   471,  715,  467,  462,  147,  682,  683,  200,  426,  441,
 /*   670 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  170,
 /*   680 */    62,   65,  575,   72,  753,  457,  654,  668,  657,  661,
 /*   690 */   662,  885,  128,  178,  180,  179,  143,  237,  245,   47,
 /*   700 */   175,  181,  833,  565,   50,  139,   59,  144,   56,  164,
 /*   710 */    44,  819,  154,  582,  834,  132,  276,  677,  678,  680,
 /*   720 */   679,  681,  470,  462,  474,  171,  534,  494,  473,  462,
 /*   730 */   153,  184,  108,  476,  462, 1383,  150,  558,  541,  454,
 /*   740 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*   750 */   479,  462,  343,  277,  279,  281,  482,  462,  291,  485,
 /*   760 */   462,  635,  302,  306,  311,  319,  638,  330,  684,  685,
 /*   770 */   337, 1000,  686,  656,  657,  661,  662,  147,  682,  683,
 /*   780 */   213,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*   790 */   495,  445,  825,   62,   65,  344,  345,  346,  347,  348,
 /*   800 */   349,  572,  517,  352,  826,  128,  220,  356,  608,  457,
 /*   810 */   523,  577,   47,  178,  180,  179,  143,   50,  139,   59,
 /*   820 */   144,   56,  164,   44,  457,  154,  457,  221,  132,  276,
 /*   830 */   677,  678,  680,  679,  681,  687,  488,  462,  171,  305,
 /*   840 */   494,  491,  462,  153,  184,  108,  702,  457,  477,  150,
 /*   850 */   327,  224,  454,  463,  472,  475,  466,  469,  478,  484,
 /*   860 */   481,  490,  487,  480,  225,  483,  277,  279,  281,  457,
 /*   870 */   614,  291,  457,  554,  552,  302,  306,  311,  319,  772,
 /*   880 */   330,  684,  685,  337,  999,  686,  486,  227,  768,  848,
 /*   890 */   147,  682,  683,  228,  426,  441,  448,  451,  146,  148,
 /*   900 */   149,  151,  152,  495,  445,  229,   62,   65,  489,   72,
 /*   910 */   767,  492,  654,  668,  657,  661,  662,  899,  128,  178,
 /*   920 */   180,  179,  143,  237,  245,   47,  574,  181,  231,  565,
 /*   930 */    50,  139,   59,  144,   56,  164,   44,  735,  154,  555,
 /*   940 */   541,  132,  276,  677,  678,  680,  679,  681,  232,  170,
 /*   950 */   823,  171,  533,  494,  821,  828,  153,  184,  108,  236,
 /*   960 */   650,  672,  150,  669,  675,  454,  463,  472,  475,  466,
 /*   970 */   469,  478,  484,  481,  490,  487,  233,  720,  665,  277,
 /*   980 */   279,  281,  692,  591,  291,  695,  236,  170,  302,  306,
 /*   990 */   311,  319,  716,  330,  684,  685,  337,  992,  773,  822,
 /*  1000 */   235,  240,  170,  147,  682,  683,  239,  426,  441,  448,
 /*  1010 */   451,  146,  148,  149,  151,  152,  495,  445,  170,   62,
 /*  1020 */    65,  601,   72,  785,  243,  654,  668,  657,  661,  662,
 /*  1030 */   774,  128,  178,  180,  179,  143,  605,  242,   47,  581,
 /*  1040 */   181,  702,  170,   50,  139,   59,  144,   56,  164,   44,
 /*  1050 */   698,  154,  612,  701,  132,  276,  677,  678,  680,  679,
 /*  1060 */   681,  247,  170,  790,  171,  310,  494,  907,  244,  153,
 /*  1070 */   184,  108,  786,  246,  732,  150,  618,  735,  454,  463,
 /*  1080 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  804,
 /*  1090 */   816,  248,  277,  279,  281,  250,  628,  291,  800,  819,
 /*  1100 */   187,  302,  306,  311,  319,  836,  330,  684,  685,  337,
 /*  1110 */   996,  773,  278, 1087,  819,  187,  147,  682,  683,  280,
 /*  1120 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  1130 */   445,  187,   62,   65,  671,   72,  799,  283,  654,  668,
 /*  1140 */   657,  661,  662,  811,  128,  178,  180,  179,  143,  697,
 /*  1150 */   540,   47,  590,  181,  702,  187,   50,  139,   59,  144,
 /*  1160 */    56,  164,   44,  862,  154,  760,  735,  132,  276,  677,
 /*  1170 */   678,  680,  679,  681,  853,  187,  851,  171,  529,  494,
 /*  1180 */   288,  293,  153,  184,  108,  863,  297,  851,  150,  792,
 /*  1190 */   299,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  1200 */   490,  487,  300,  315,  282,  277,  279,  281,  303,  886,
 /*  1210 */   291,  308,  812,  318,  302,  306,  311,  319,  312,  330,
 /*  1220 */   684,  685,  337,  178,  180,  179,  143,  317,  775,  147,
 /*  1230 */   682,  683,  320,  426,  441,  448,  451,  146,  148,  149,
 /*  1240 */   151,  152,  495,  445,  187,   62,   65,  866,   72,  851,
 /*  1250 */   325,   80,   82,   84,  876,  324,  851,  128,  178,  180,
 /*  1260 */   179,  143,   73,   69,   47,  611,  181,  525,  331,   50,
 /*  1270 */   139,   59,  144,   56,  164,   44,  341,  154,  900,  359,
 /*  1280 */   132,  276,  677,  678,  680,  679,  681,  362,  369,  377,
 /*  1290 */   171,  314,  494,  385,  378,  153,  184,  108,  381,  413,
 /*  1300 */   382,  150,  390,  397,  454,  463,  472,  475,  466,  469,
 /*  1310 */   478,  484,  481,  490,  487,  401,  408,  292,  277,  279,
 /*  1320 */   281,  415,  418,  291,  422,  832,  318,  302,  306,  311,
 /*  1330 */   319,  419,  330,  684,  685,  337,  178,  180,  179,  143,
 /*  1340 */   423,  743,  147,  682,  683,  434,  426,  441,  448,  451,
 /*  1350 */   146,  148,  149,  151,  152,  495,  445,  436,   62,   65,
 /*  1360 */   438,   72,  440,  458,  459,  460,  843,  461,  809,  831,
 /*  1370 */   128,  178,  180,  179,  143,  807,  504,   47,  617,  181,
 /*  1380 */   702,  505,   50,  139,   59,  144,   56,  164,   44,  518,
 /*  1390 */   154,  519,  524,  132,  276,  677,  678,  680,  679,  681,
 /*  1400 */   532,  530,  536,  171,  528,  494,  543,  548,  153,  184,
 /*  1410 */   108,  556,  573,  557,  150,  559,  567,  454,  463,  472,
 /*  1420 */   475,  466,  469,  478,  484,  481,  490,  487,  578,  579,
 /*  1430 */   580,  277,  279,  281,  586,  587,  291,  588,  609,  594,
 /*  1440 */   302,  306,  311,  319,  615,  330,  684,  685,  337,  994,
 /*  1450 */   844,  631,  610,  616,  633,  147,  682,  683,  634,  426,
 /*  1460 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  1470 */   636,   62,   65,  638,  637,  641,  640,  676,  642,  643,
 /*  1480 */   724,  645,  645,  128,  647,  648,  703,  725,  704,  704,
 /*  1490 */    47,  652,  702,  702,  655,   50,  139,   59,  144,   56,
 /*  1500 */   164,   44,  658,  154,  664,  666,  132,  276,  677,  678,
 /*  1510 */   680,  679,  681,  667,  674,  689,  171,  322,  494,  690,
 /*  1520 */   694,  153,  184,  108,  700,  708,  710,  150,  709,  712,
 /*  1530 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  1540 */   487,  713,  714,  717,  277,  279,  281,  719,  723,  291,
 /*  1550 */   718,  721,  722,  302,  306,  311,  319,  727,  330,  684,
 /*  1560 */   685,  337,  997,  773,  728,  730,  731,  850,  147,  682,
 /*  1570 */   683,  734,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  1580 */   152,  495,  445,  738,   62,   65,  744,  745,  776,  815,
 /*  1590 */   777,  805,  740,  829,  809,  831,  128,  818,  817, 1011,
 /*  1600 */   725,  807, 1012,   47,  824,  827,  702,  830,   50,  139,
 /*  1610 */    59,  144,   56,  164,   44,  839,  154,  835,  838,  132,
 /*  1620 */   276,  677,  678,  680,  679,  681,  837,  840,  856,  171,
 /*  1630 */   516,  494,  849,  857,  153,  184,  108,  854,  858,  860,
 /*  1640 */   150,  861,  864,  454,  463,  472,  475,  466,  469,  478,
 /*  1650 */   484,  481,  490,  487,  867,  869,  870,  277,  279,  281,
 /*  1660 */   873,  871,  291,  735,  874,  877,  302,  306,  311,  319,
 /*  1670 */   879,  330,  684,  685,  337,  995,  773,  882,  887,  880,
 /*  1680 */   894,  147,  682,  683,  893,  426,  441,  448,  451,  146,
 /*  1690 */   148,  149,  151,  152,  495,  445,  892,   62,   65,  896,
 /*  1700 */   901, 1086,  910,  842,  680,  845,  680,  846,  831,  128,
 /*  1710 */   680,  680,  680,  703,  807,  680,   47,  680,  680,  702,
 /*  1720 */   680,   50,  139,   59,  144,   56,  164,   44,  680,  154,
 /*  1730 */   680,  680,  132,  276,  677,  678,  680,  679,  681,  680,
 /*  1740 */   680,  680,  171,  329,  494,  680,  680,  153,  184,  108,
 /*  1750 */   680,  680,  680,  150,  680,  680,  454,  463,  472,  475,
 /*  1760 */   466,  469,  478,  484,  481,  490,  487,  680,  680,  680,
 /*  1770 */   277,  279,  281,  680,  680,  291,  680,  680,  680,  302,
 /*  1780 */   306,  311,  319,  680,  330,  684,  685,  337,  991,  773,
 /*  1790 */   680,  680,  680,  680,  147,  682,  683,  680,  426,  441,
 /*  1800 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  680,
 /*  1810 */    62,   65,  680,   72,  680,  680,  680,  680,  680,  680,
 /*  1820 */   680,  680,  128,  178,  180,  179,  143,  680,  680,   47,
 /*  1830 */   680,  584,  680,  680,   50,  139,   59,  144,   56,  164,
 /*  1840 */    44,  680,  154,  680,  680,  132,  276,  677,  678,  680,
 /*  1850 */   679,  681,  680,  680,  680,  171,  497,  494,  680,  680,
 /*  1860 */   153,  184,  108,  680,  680,  680,  150,  680,  680,  454,
 /*  1870 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*  1880 */   680,  680,  307,  277,  279,  281,  680,  680,  291,  215,
 /*  1890 */   680,  318,  302,  306,  311,  319,  680,  330,  684,  685,
 /*  1900 */   337,  178,  180,  179,  143,  680,  646,  147,  682,  683,
 /*  1910 */   680,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*  1920 */   495,  445,  680,   62,   65,  680,  363,  680,  680,  680,
 /*  1930 */   680,  680,  680,  361,  680,  128,  178,  180,  179,  143,
 /*  1940 */   680,  680,   47,  208,  217,  212,  216,   50,  139,   59,
 /*  1950 */   144,   56,  164,   44,  680,  154,  680,  680,  132,  276,
 /*  1960 */   677,  678,  680,  679,  681,  680,  680,  680,  171,  336,
 /*  1970 */   494,  680,  680,  153,  184,  108,  680,  680,  680,  150,
 /*  1980 */   680,  680,  454,  463,  472,  475,  466,  469,  478,  484,
 /*  1990 */   481,  490,  487,  680,  680,  316,  277,  279,  281,  680,
 /*  2000 */   680,  291,  680,  680,  318,  302,  306,  311,  319,  680,
 /*  2010 */   330,  684,  685,  337,  178,  180,  179,  143,  680,  688,
 /*  2020 */   147,  682,  683,  680,  426,  441,  448,  451,  146,  148,
 /*  2030 */   149,  151,  152,  495,  445,  680,   62,   65,  680,  352,
 /*  2040 */   680,  680,  680,  360,  680,  680,  155,  680,  128,  178,
 /*  2050 */   180,  179,  143,  680,  680,   47,  178,  180,  179,  143,
 /*  2060 */    50,  139,   59,  144,   56,  164,   44,  595,  154,  680,
 /*  2070 */   680,  132,  276,  677,  678,  680,  679,  681,  680,  680,
 /*  2080 */   680,  171,  502,  494,  680,  680,  153,  184,  108,  680,
 /*  2090 */   680,  680,  150,  680,  680,  454,  463,  472,  475,  466,
 /*  2100 */   469,  478,  484,  481,  490,  487,  680,  680,  680,  277,
 /*  2110 */   279,  281,  680,  680,  291,  680,  680,  680,  302,  306,
 /*  2120 */   311,  319,  680,  330,  684,  685,  337,  680,  680,  680,
 /*  2130 */   706,  680,  680,  147,  682,  683,  680,  426,  441,  448,
 /*  2140 */   451,  146,  148,  149,  151,  152,  495,  445,  680,   62,
 /*  2150 */    65,  680,  352,  680,  680,  680,  366,  680,  680,   45,
 /*  2160 */   680,  128,  178,  180,  179,  143,  680,  680,   47,  178,
 /*  2170 */   180,  179,  143,   50,  139,   59,  144,   56,  164,   44,
 /*  2180 */   680,  154,  680,  680,  132,  276,  677,  678,  680,  679,
 /*  2190 */   681,  680,  680,  680,  171,  510,  494,  680,  680,  153,
 /*  2200 */   184,  108,  680,  680,  680,  150,  680,  680,  454,  463,
 /*  2210 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  680,
 /*  2220 */   680,  531,  277,  279,  281,  680,  680,  291,  680,  680,
 /*  2230 */   318,  302,  306,  311,  319,  680,  330,  684,  685,  337,
 /*  2240 */   178,  180,  179,  143,  680,  743,  147,  682,  683,  680,
 /*  2250 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  2260 */   445,  680,   62,   65,  680,  352,  680,  680,  680,  370,
 /*  2270 */   680,  680,  626,  680,  128,  178,  180,  179,  143,  680,
 /*  2280 */   680,   47,  178,  180,  179,  143,   50,  139,   59,  144,
 /*  2290 */    56,  164,   44,  680,  154,  680,  680,  132,  276,  677,
 /*  2300 */   678,  680,  679,  681,  680,  680,  680,  171,  509,  494,
 /*  2310 */   680,  680,  153,  184,  108,  680,  680,  680,  150,  680,
 /*  2320 */   680,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  2330 */   490,  487,  680,  680,  542,  277,  279,  281,  680,  680,
 /*  2340 */   291,  680,  680,  318,  302,  306,  311,  319,  680,  330,
 /*  2350 */   684,  685,  337,  178,  180,  179,  143,  680,  775,  147,
 /*  2360 */   682,  683,  680,  426,  441,  448,  451,  146,  148,  149,
 /*  2370 */   151,  152,  495,  445,  680,   62,   65,  680,  352,  680,
 /*  2380 */   680,  680,  379,  680,  680,   48,  680,  128,  178,  180,
 /*  2390 */   179,  143,  680,  680,   47,  178,  180,  179,  143,   50,
 /*  2400 */   139,   59,  144,   56,  164,   44,  680,  154,  680,  680,
 /*  2410 */   132,  276,  677,  678,  680,  679,  681,  680,  680,  680,
 /*  2420 */   171,  515,  494,  680,  680,  153,  184,  108,  680,  680,
 /*  2430 */   680,  150,  680,  680,  454,  463,  472,  475,  466,  469,
 /*  2440 */   478,  484,  481,  490,  487,  680,  680,  680,  277,  279,
 /*  2450 */   281,  680,  680,  291,  680,  680,  625,  302,  306,  311,
 /*  2460 */   319,  680,  330,  684,  685,  337,  178,  180,  179,  143,
 /*  2470 */   680,  680,  147,  682,  683,  680,  426,  441,  448,  451,
 /*  2480 */   146,  148,  149,  151,  152,  495,  445,  680,   62,   65,
 /*  2490 */   680,  352,  680,  680,  680,  383,  680,  680,   51,  680,
 /*  2500 */   128,  178,  180,  179,  143,  680,  680,   47,  178,  180,
 /*  2510 */   179,  143,   50,  139,   59,  144,   56,  164,   44,  431,
 /*  2520 */   154,  680,  680,  132,  276,  680,  680,  427,  432,  178,
 /*  2530 */   180,  179,  143,  171,  522,  494,  680,  680,  153,  184,
 /*  2540 */   108,  680,  680,  680,  150,  680,  680,  454,  463,  472,
 /*  2550 */   475,  466,  469,  478,  484,  481,  490,  487,  680,  680,
 /*  2560 */   680,  277,  279,  281,  680,  680,  291,  680,  680,  624,
 /*  2570 */   302,  306,  311,  319,  680,  330,  680,  680,  337,  178,
 /*  2580 */   180,  179,  143,  680,  680,  147,  680,  680,  680,  426,
 /*  2590 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  2600 */   680,   62,   65,  680,  352,  680,  680,  680,  387,  680,
 /*  2610 */   680,   54,  680,  128,  178,  180,  179,  143,  680,  680,
 /*  2620 */    47,  178,  180,  179,  143,   50,  139,   59,  144,   56,
 /*  2630 */   164,   44,  431,  154,  680,  680,  132,  276,  680,  680,
 /*  2640 */   680,  430,  178,  180,  179,  143,  171,  521,  494,  680,
 /*  2650 */   680,  153,  184,  108,  680,  680,  680,  150,  680,  680,
 /*  2660 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  2670 */   487,  680,  680,  680,  277,  279,  281,  680,  680,  291,
 /*  2680 */   680,  680,  623,  302,  306,  311,  319,  680,  330,  680,
 /*  2690 */   680,  337,  178,  180,  179,  143,  680,  680,  147,  680,
 /*  2700 */   680,  680,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  2710 */   152,  495,  445,  680,   62,   65,  680,  352,  680,  680,
 /*  2720 */   680,  391,  680,  680,   57,  680,  128,  178,  180,  179,
 /*  2730 */   143,  680,  680,   47,  178,  180,  179,  143,   50,  139,
 /*  2740 */    59,  144,   56,  164,   44,  622,  154,  680,  680,  132,
 /*  2750 */   276,  680,  680,  680,  680,  178,  180,  179,  143,  171,
 /*  2760 */   546,  494,  680,  680,  153,  184,  108,  680,  680,  680,
 /*  2770 */   150,  680,  680,  454,  463,  472,  475,  466,  469,  478,
 /*  2780 */   484,  481,  490,  487,  680,  680,  680,  277,  279,  281,
 /*  2790 */   680,  680,  291,  680,  680,   60,  302,  306,  311,  319,
 /*  2800 */   680,  330,  680,  680,  337,  178,  180,  179,  143,  680,
 /*  2810 */   680,  147,  680,  680,  680,  426,  441,  448,  451,  146,
 /*  2820 */   148,  149,  151,  152,  495,  445,  680,   62,   65,  680,
 /*  2830 */   352,  680,  680,  680,  394,  680,  680,  680,  680,  128,
 /*  2840 */   178,  180,  179,  143,  680,  680,   47,  680,  680,  680,
 /*  2850 */   680,   50,  139,   59,  144,   56,  164,   44,  621,  154,
 /*  2860 */   680,  680,  132,  276,  680,  680,  680,  680,  178,  180,
 /*  2870 */   179,  143,  171,  545,  494,  680,  680,  153,  184,  108,
 /*  2880 */   680,  680,  680,  150,  680,  680,  454,  463,  472,  475,
 /*  2890 */   466,  469,  478,  484,  481,  490,  487,  680,  680,  680,
 /*  2900 */   277,  279,  281,  680,  680,  291,  680,  680,   63,  302,
 /*  2910 */   306,  311,  319,  680,  330,  680,  680,  337,  178,  180,
 /*  2920 */   179,  143,  680,  680,  147,  680,  680,  680,  426,  441,
 /*  2930 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  680,
 /*  2940 */    62,   65,  680,  352,  680,  680,  680,  398,  680,  680,
 /*  2950 */   680,  680,  128,  178,  180,  179,  143,  680,  680,   47,
 /*  2960 */   680,  680,  680,  680,   50,  139,   59,  144,   56,  164,
 /*  2970 */    44,  620,  154,  680,  680,  132,  276,  680,  680,  680,
 /*  2980 */   680,  178,  180,  179,  143,  171,  551,  494,  680,  680,
 /*  2990 */   153,  184,  108,  680,  680,  680,  150,  680,  680,  454,
 /*  3000 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*  3010 */   680,  680,  680,  277,  279,  281,  680,  680,  291,  680,
 /*  3020 */   680,   66,  302,  306,  311,  319,  680,  330,  680,  680,
 /*  3030 */   337,  178,  180,  179,  143,  680,  680,  147,  680,  680,
 /*  3040 */   680,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*  3050 */   495,  445,  680,   62,   65,  680,  352,  680,  680,  680,
 /*  3060 */   402,  680,  680,  680,  680,  128,  178,  180,  179,  143,
 /*  3070 */   680,  680,   47,  680,  680,  680,  680,   50,  139,   59,
 /*  3080 */   144,   56,  164,   44,   68,  154,  680,  680,  132,  276,
 /*  3090 */   680,  680,  680,  680,  178,  180,  179,  143,  171,  550,
 /*  3100 */   494,  680,  680,  153,  184,  108,  680,  680,  680,  150,
 /*  3110 */   680,  680,  454,  463,  472,  475,  466,  469,  478,  484,
 /*  3120 */   481,  490,  487,  680,  680,  680,  277,  279,  281,  680,
 /*  3130 */   680,  291,  680,  680,   74,  302,  306,  311,  319,  680,
 /*  3140 */   330,  680,  680,  337,  178,  180,  179,  143,  680,  680,
 /*  3150 */   147,  680,  680,  680,  426,  441,  448,  451,  146,  148,
 /*  3160 */   149,  151,  152,  495,  445,  680,   62,   65,  680,  352,
 /*  3170 */   680,  680,  680,  405,  680,  680,  680,  680,  128,  178,
 /*  3180 */   180,  179,  143,  680,  680,   47,  680,  680,  680,  680,
 /*  3190 */    50,  139,   59,  144,   56,  164,   44,   77,  154,  680,
 /*  3200 */   680,  132,  276,  680,  680,  680,  680,  178,  180,  179,
 /*  3210 */   143,  171,  562,  494,  680,  680,  153,  184,  108,  680,
 /*  3220 */   680,  680,  150,  680,  680,  454,  463,  472,  475,  466,
 /*  3230 */   469,  478,  484,  481,  490,  487,  680,  680,  680,  277,
 /*  3240 */   279,  281,  680,  680,  291,  680,  680,   79,  302,  306,
 /*  3250 */   311,  319,  680,  330,  680,  680,  337,  178,  180,  179,
 /*  3260 */   143,  680,  680,  147,  680,  680,  680,  426,  441,  448,
 /*  3270 */   451,  146,  148,  149,  151,  152,  495,  445,  680,   62,
 /*  3280 */    65,  680,  352,  680,  680,  680,  409,  680,  680,  680,
 /*  3290 */   680,  128,  178,  180,  179,  143,  680,  680,   47,  680,
 /*  3300 */   680,  680,  680,   50,  139,   59,  144,   56,  164,   44,
 /*  3310 */    81,  154,  680,  680,  132,  276,  680,  680,  680,  680,
 /*  3320 */   178,  180,  179,  143,  171,  561,  494,  680,  680,  153,
 /*  3330 */   184,  108,  680,  680,  680,  150,  680,  680,  454,  463,
 /*  3340 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  680,
 /*  3350 */   680,  680,  277,  279,  281,  680,  680,  291,  680,  680,
 /*  3360 */    83,  302,  306,  311,  319,  680,  330,  680,  680,  337,
 /*  3370 */   178,  180,  179,  143,  680,  680,  147,  680,  680,  680,
 /*  3380 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  3390 */   445,  680,   62,   65,  680,  352,  680,  680,  680,  420,
 /*  3400 */   680,  680,  680,  680,  128,  178,  180,  179,  143,  680,
 /*  3410 */   680,   47,  680,  680,  680,  680,   50,  139,   59,  144,
 /*  3420 */    56,  164,   44,   85,  154,  680,  680,  132,  276,  680,
 /*  3430 */   680,  680,  680,  178,  180,  179,  143,  171,  570,  494,
 /*  3440 */   680,  680,  153,  184,  108,  680,  680,  680,  150,  680,
 /*  3450 */   680,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  3460 */   490,  487,  680,  680,  680,  277,  279,  281,  680,  680,
 /*  3470 */   291,  680,  680,   87,  302,  306,  311,  319,  680,  330,
 /*  3480 */   680,  680,  337,  178,  180,  179,  143,  680,  680,  147,
 /*  3490 */   680,  680,  680,  426,  441,  448,  451,  146,  148,  149,
 /*  3500 */   151,  152,  495,  445,  680,   62,   65,  680,  352,  680,
 /*  3510 */   680,  680,  424,  680,  680,  680,  680,  128,  178,  180,
 /*  3520 */   179,  143,  680,  680,   47,  680,  680,  680,  680,   50,
 /*  3530 */   139,   59,  144,   56,  164,   44,   89,  154,  680,  680,
 /*  3540 */   132,  276,  680,  680,  680,  680,  178,  180,  179,  143,
 /*  3550 */   171,  569,  494,  680,  680,  153,  184,  108,  680,  680,
 /*  3560 */   680,  150,  680,  680,  454,  463,  472,  475,  466,  469,
 /*  3570 */   478,  484,  481,  490,  487,  680,  680,  680,  277,  279,
 /*  3580 */   281,  680,  680,  291,  680,  680,   91,  302,  306,  311,
 /*  3590 */   319,  680,  330,  680,  680,  337,  178,  180,  179,  143,
 /*  3600 */   680,  680,  147,  680,  680,  680,  426,  441,  448,  451,
 /*  3610 */   146,  148,  149,  151,  152,  495,  445,  680,   62,   65,
 /*  3620 */   680,   93,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  3630 */   128,  178,  180,  179,  143,  680,  680,   47,  680,  680,
 /*  3640 */   680,  680,   50,  139,   59,  144,   56,  164,   44,   95,
 /*  3650 */   154,  680,  680,  132,  276,  680,  680,  680,  680,  178,
 /*  3660 */   180,  179,  143,  171,  748,  494,  680,  680,  153,  184,
 /*  3670 */   108,  680,  680,  680,  150,  680,  680,  454,  463,  472,
 /*  3680 */   475,  466,  469,  478,  484,  481,  490,  487,  680,  680,
 /*  3690 */   680,  277,  279,  281,  680,  680,  291,  680,  680,   97,
 /*  3700 */   302,  306,  311,  319,  680,  330,  680,  680,  337,  178,
 /*  3710 */   180,  179,  143,  680,  680,  147,  680,  680,  680,  426,
 /*  3720 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  3730 */   680,   62,   65,  680,   99,  680,  680,  680,  680,  680,
 /*  3740 */   680,  680,  680,  128,  178,  180,  179,  143,  680,  680,
 /*  3750 */    47,  680,  680,  680,  680,   50,  139,   59,  144,   56,
 /*  3760 */   164,   44,  101,  154,  680,  680,  132,  276,  680,  680,
 /*  3770 */   680,  680,  178,  180,  179,  143,  171,  750,  494,  680,
 /*  3780 */   680,  153,  184,  108,  680,  680,  680,  150,  680,  680,
 /*  3790 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  3800 */   487,  680,  680,  680,  277,  279,  281,  680,  680,  291,
 /*  3810 */   680,  680,  103,  302,  306,  311,  319,  680,  330,  680,
 /*  3820 */   680,  337,  178,  180,  179,  143,  680,  680,  147,  680,
 /*  3830 */   680,  680,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  3840 */   152,  495,  445,  680,   62,   65,  680,  105,  680,  680,
 /*  3850 */   680,  680,  680,  680,  680,  680,  128,  178,  180,  179,
 /*  3860 */   143,  680,  680,   47,  680,  680,  680,  680,   50,  139,
 /*  3870 */    59,  144,   56,  164,   44,  107,  154,  680,  680,  132,
 /*  3880 */   276,  680,  680,  680,  680,  178,  180,  179,  143,  171,
 /*  3890 */   755,  494,  680,  680,  153,  184,  108,  680,  680,  680,
 /*  3900 */   150,  680,  680,  454,  463,  472,  475,  466,  469,  478,
 /*  3910 */   484,  481,  490,  487,  680,  680,  680,  277,  279,  281,
 /*  3920 */   680,  680,  291,  680,  680,  127,  302,  306,  311,  319,
 /*  3930 */   680,  330,  680,  680,  337,  178,  180,  179,  143,  680,
 /*  3940 */   680,  147,  680,  680,  680,  426,  441,  448,  451,  146,
 /*  3950 */   148,  149,  151,  152,  495,  445,  680,   62,   65,  680,
 /*  3960 */   131,  680,  680,  680,  680,  680,  680,  680,  680,  128,
 /*  3970 */   178,  180,  179,  143,  680,  680,   47,  680,  680,  680,
 /*  3980 */   680,   50,  139,   59,  144,   56,  164,   44,  138,  154,
 /*  3990 */   680,  680,  132,  276,  680,  680,  680,  680,  178,  180,
 /*  4000 */   179,  143,  171,  757,  494,  680,  680,  153,  184,  108,
 /*  4010 */   680,  680,  680,  150,  680,  680,  454,  463,  472,  475,
 /*  4020 */   466,  469,  478,  484,  481,  490,  487,  680,  680,  680,
 /*  4030 */   277,  279,  281,  680,  680,  291,  680,  680,  142,  302,
 /*  4040 */   306,  311,  319,  680,  330,  680,  680,  337,  178,  180,
 /*  4050 */   179,  143,  680,  680,  147,  680,  680,  680,  426,  441,
 /*  4060 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  680,
 /*  4070 */    62,   65,  680,  145,  680,  680,  680,  680,  680,  680,
 /*  4080 */   680,  680,  128,  178,  180,  179,  143,  680,  680,   47,
 /*  4090 */   680,  680,  680,  680,   50,  139,   59,  144,   56,  164,
 /*  4100 */    44,  183,  154,  680,  680,  132,  276,  680,  680,  680,
 /*  4110 */   680,  178,  180,  179,  143,  171,  762,  494,  680,  680,
 /*  4120 */   153,  184,  108,  680,  680,  680,  150,  680,  680,  454,
 /*  4130 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*  4140 */   680,  680,  680,  277,  279,  281,  680,  680,  291,  680,
 /*  4150 */   680,  333,  302,  306,  311,  319,  680,  330,  680,  680,
 /*  4160 */   337,  178,  180,  179,  143,  680,  680,  147,  680,  680,
 /*  4170 */   680,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*  4180 */   495,  445,  680,   62,   65,  680,  442,  680,  680,  680,
 /*  4190 */   680,  680,  680,  680,  680,  128,  178,  180,  179,  143,
 /*  4200 */   680,  680,   47,  680,  680,  680,  680,   50,  139,   59,
 /*  4210 */   144,   56,  164,   44,  446,  154,  680,  680,  132,  276,
 /*  4220 */   680,  680,  680,  680,  178,  180,  179,  143,  171,  764,
 /*  4230 */   494,  680,  680,  153,  184,  108,  680,  680,  680,  150,
 /*  4240 */   680,  680,  454,  463,  472,  475,  466,  469,  478,  484,
 /*  4250 */   481,  490,  487,  680,  680,  680,  277,  279,  281,  680,
 /*  4260 */   680,  291,  680,  680,  449,  302,  306,  311,  319,  680,
 /*  4270 */   330,  680,  680,  337,  178,  180,  179,  143,  680,  680,
 /*  4280 */   147,  680,  680,  680,  426,  441,  448,  451,  146,  148,
 /*  4290 */   149,  151,  152,  495,  445,  680,   62,   65,  680,  452,
 /*  4300 */   680,  680,  680,  680,  680,  680,  680,  680,  128,  178,
 /*  4310 */   180,  179,  143,  680,  680,   47,  680,  680,  680,  680,
 /*  4320 */    50,  139,   59,  144,   56,  164,   44,  499,  154,  680,
 /*  4330 */   680,  132,  276,  680,  680,  680,  680,  178,  180,  179,
 /*  4340 */   143,  171,  769,  494,  680,  680,  153,  184,  108,  680,
 /*  4350 */   680,  680,  150,  680,  680,  454,  463,  472,  475,  466,
 /*  4360 */   469,  478,  484,  481,  490,  487,  680,  680,  680,  277,
 /*  4370 */   279,  281,  680,  680,  291,  680,  680,  506,  302,  306,
 /*  4380 */   311,  319,  680,  330,  680,  680,  337,  178,  180,  179,
 /*  4390 */   143,  680,  680,  147,  680,  680,  680,  426,  441,  448,
 /*  4400 */   451,  146,  148,  149,  151,  152,  495,  445,  680,   62,
 /*  4410 */    65,  680,  512,  680,  680,  680,  680,  680,  680,  680,
 /*  4420 */   680,  128,  178,  180,  179,  143,  680,  680,   47,  680,
 /*  4430 */   680,  680,  680,   50,  139,   59,  144,   56,  164,   44,
 /*  4440 */   599,  154,  680,  680,  132,  276,  680,  680,  680,  680,
 /*  4450 */   178,  180,  179,  143,  171,  771,  494,  680,  680,  153,
 /*  4460 */   184,  108,  680,  680,  680,  150,  680,  680,  454,  463,
 /*  4470 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  680,
 /*  4480 */   680,  680,  277,  279,  281,  680,  680,  291,  680,  680,
 /*  4490 */   931,  302,  306,  311,  319,  680,  330,  680,  680,  337,
 /*  4500 */   178,  180,  179,  143,  680,  680,  147,  680,  680,  680,
 /*  4510 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  4520 */   445,  806,   62,   65,  742,  680,  680,  680,  680,  703,
 /*  4530 */   807,  680,  680,  680,  128,  702,  680,  680,  680,  680,
 /*  4540 */   680,   47,  680,  680,  680,  680,   50,  139,   59,  144,
 /*  4550 */    56,  164,   44,  680,  154,  680,  680,  132,  276,  680,
 /*  4560 */   680,  680,  680,  680,  680,  680,  680,  171,  780,  494,
 /*  4570 */   680,  680,  153,  184,  108,  680,  680,  680,  150,  680,
 /*  4580 */   680,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  4590 */   490,  487,  680,  680,  680,  277,  279,  281,  680,  680,
 /*  4600 */   291,  680,  680,  680,  302,  306,  311,  319,  680,  330,
 /*  4610 */   680,  680,  337,  680,  680,  680,  680,  680,  680,  147,
 /*  4620 */   680,  680,  680,  426,  441,  448,  451,  146,  148,  149,
 /*  4630 */   151,  152,  495,  445,  680,   62,   65,  680,  680,  680,
 /*  4640 */   680,  680,  680,  680,  680,  680,  680,  128,  680,  680,
 /*  4650 */   680,  680,  680,  680,   47,  680,  680,  680,  680,   50,
 /*  4660 */   139,   59,  144,   56,  164,   44,  680,  154,  680,  680,
 /*  4670 */   132,  276,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  4680 */   171,  782,  494,  680,  680,  153,  184,  108,  680,  680,
 /*  4690 */   680,  150,  680,  680,  454,  463,  472,  475,  466,  469,
 /*  4700 */   478,  484,  481,  490,  487,  680,  680,  680,  277,  279,
 /*  4710 */   281,  680,  680,  291,  680,  680,  680,  302,  306,  311,
 /*  4720 */   319,  680,  330,  680,  680,  337,  680,  680,  680,  680,
 /*  4730 */   680,  680,  147,  680,  680,  680,  426,  441,  448,  451,
 /*  4740 */   146,  148,  149,  151,  152,  495,  445,  680,   62,   65,
 /*  4750 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  4760 */   128,  680,  680,  680,  680,  680,  680,   47,  680,  680,
 /*  4770 */   680,  680,   50,  139,   59,  144,   56,  164,   44,  680,
 /*  4780 */   154,  680,  680,  132,  276,  680,  680,  680,  680,  680,
 /*  4790 */   680,  680,  680,  171,  787,  494,  680,  680,  153,  184,
 /*  4800 */   108,  680,  680,  680,  150,  680,  680,  454,  463,  472,
 /*  4810 */   475,  466,  469,  478,  484,  481,  490,  487,  680,  680,
 /*  4820 */   680,  277,  279,  281,  680,  680,  291,  680,  680,  680,
 /*  4830 */   302,  306,  311,  319,  680,  330,  680,  680,  337,  680,
 /*  4840 */   680,  680,  680,  680,  680,  147,  680,  680,  680,  426,
 /*  4850 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  4860 */   680,   62,   65,  680,  680,  680,  680,  680,  680,  680,
 /*  4870 */   680,  680,  680,  128,  680,  680,  680,  680,  680,  680,
 /*  4880 */    47,  680,  680,  680,  680,   50,  139,   59,  144,   56,
 /*  4890 */   164,   44,  680,  154,  680,  680,  132,  276,  680,  680,
 /*  4900 */   680,  680,  680,  680,  680,  680,  171,  789,  494,  680,
 /*  4910 */   680,  153,  184,  108,  680,  680,  680,  150,  680,  680,
 /*  4920 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  4930 */   487,  680,  680,  680,  277,  279,  281,  680,  680,  291,
 /*  4940 */   680,  680,  680,  302,  306,  311,  319,  680,  330,  680,
 /*  4950 */   680,  337,  680,  680,  680,  680,  680,  680,  147,  680,
 /*  4960 */   680,  680,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  4970 */   152,  495,  445,  680,   62,   65,  680,  680,  680,  680,
 /*  4980 */   680,  680,  680,  680,  680,  680,  128,  680,  680,  680,
 /*  4990 */   680,  680,  680,   47,  680,  680,  680,  680,   50,  139,
 /*  5000 */    59,  144,   56,  164,   44,  680,  154,  680,  680,  132,
 /*  5010 */   276,  680,  680,  680,  680,  680,  680,  680,  680,  171,
 /*  5020 */   794,  494,  680,  680,  153,  184,  108,  680,  680,  680,
 /*  5030 */   150,  680,  680,  454,  463,  472,  475,  466,  469,  478,
 /*  5040 */   484,  481,  490,  487,  680,  680,  680,  277,  279,  281,
 /*  5050 */   680,  680,  291,  680,  680,  680,  302,  306,  311,  319,
 /*  5060 */   680,  330,  680,  680,  337,  680,  680,  680,  680,  680,
 /*  5070 */   680,  147,  680,  680,  680,  426,  441,  448,  451,  146,
 /*  5080 */   148,  149,  151,  152,  495,  445,  680,   62,   65,  680,
 /*  5090 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  128,
 /*  5100 */   680,  680,  680,  680,  680,  680,   47,  680,  680,  680,
 /*  5110 */   680,   50,  139,   59,  144,   56,  164,   44,  680,  154,
 /*  5120 */   680,  680,  132,  276,  680,  680,  680,  680,  680,  680,
 /*  5130 */   680,  680,  171,  796,  494,  680,  680,  153,  184,  108,
 /*  5140 */   680,  680,  680,  150,  680,  680,  454,  463,  472,  475,
 /*  5150 */   466,  469,  478,  484,  481,  490,  487,  680,  680,  680,
 /*  5160 */   277,  279,  281,  680,  680,  291,  680,  680,  680,  302,
 /*  5170 */   306,  311,  319,  680,  330,  680,  680,  337,  680,  680,
 /*  5180 */   680,  680,  680,  680,  147,  680,  680,  680,  426,  441,
 /*  5190 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  680,
 /*  5200 */    62,   65,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  5210 */   680,  680,  128,  680,  680,  680,  680,  680,  680,   47,
 /*  5220 */   680,  680,  680,  680,   50,  139,   59,  144,   56,  164,
 /*  5230 */    44,  680,  154,  680,  680,  132,  276,  680,  680,  680,
 /*  5240 */   680,  680,  680,  680,  680,  171,  801,  494,  680,  680,
 /*  5250 */   153,  184,  108,  680,  680,  680,  150,  680,  680,  454,
 /*  5260 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*  5270 */   680,  680,  680,  277,  279,  281,  680,  680,  291,  680,
 /*  5280 */   680,  680,  302,  306,  311,  319,  680,  330,  680,  680,
 /*  5290 */   337,  680,  680,  680,  680,  680,  680,  147,  680,  680,
 /*  5300 */   680,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*  5310 */   495,  445,  680,   62,   65,  680,  680,  680,  680,  680,
 /*  5320 */   680,  680,  680,  680,  680,  128,  680,  680,  680,  680,
 /*  5330 */   680,  680,   47,  680,  680,  680,  680,   50,  139,   59,
 /*  5340 */   144,   56,  164,   44,  680,  154,  680,  680,  132,  276,
 /*  5350 */   680,  680,  680,  680,  680,  680,  680,  680,  171,  803,
 /*  5360 */   494,  680,  680,  153,  184,  108,  680,  680,  680,  150,
 /*  5370 */   680,  680,  454,  463,  472,  475,  466,  469,  478,  484,
 /*  5380 */   481,  490,  487,  680,  680,  680,  277,  279,  281,  680,
 /*  5390 */   680,  291,  680,  680,  680,  302,  306,  311,  319,  680,
 /*  5400 */   330,  680,  680,  337,  680,  680,  680,  680,  680,  680,
 /*  5410 */   147,  680,  680,  680,  426,  441,  448,  451,  146,  148,
 /*  5420 */   149,  151,  152,  495,  445,  680,   62,   65,  680,  680,
 /*  5430 */   680,  680,  680,  680,  680,  680,  680,  680,  128,  680,
 /*  5440 */   680,  680,  680,  680,  680,   47,  680,  680,  680,  680,
 /*  5450 */    50,  139,   59,  144,   56,  164,   44,  680,  154,  680,
 /*  5460 */   680,  132,  276,  680,  680,  680,  680,  680,  680,  680,
 /*  5470 */   680,  171,  884,  494,  680,  680,  153,  184,  108,  680,
 /*  5480 */   680,  680,  150,  680,  680,  454,  463,  472,  475,  466,
 /*  5490 */   469,  478,  484,  481,  490,  487,  680,  680,  680,  277,
 /*  5500 */   279,  281,  680,  680,  291,  680,  680,  680,  302,  306,
 /*  5510 */   311,  319,  680,  330,  680,  680,  337,  680,  680,  680,
 /*  5520 */   680,  680,  680,  147,  680,  680,  680,  426,  441,  448,
 /*  5530 */   451,  146,  148,  149,  151,  152,  495,  445,  680,   62,
 /*  5540 */    65,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  5550 */   680,  128,  680,  680,  680,  680,  680,  680,   47,  680,
 /*  5560 */   680,  680,  680,   50,  139,   59,  144,   56,  164,   44,
 /*  5570 */   680,  154,  680,  680,  132,  276,  680,  680,  680,  680,
 /*  5580 */   680,  680,  680,  680,  171,  888,  494,  680,  680,  153,
 /*  5590 */   184,  108,  680,  680,  680,  150,  680,  680,  454,  463,
 /*  5600 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  680,
 /*  5610 */   680,  680,  277,  279,  281,  680,  680,  291,  680,  680,
 /*  5620 */   680,  302,  306,  311,  319,  680,  330,  680,  680,  337,
 /*  5630 */   680,  680,  680,  680,  680,  680,  147,  680,  680,  680,
 /*  5640 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  5650 */   445,  680,   62,   65,  680,  680,  680,  680,  680,  680,
 /*  5660 */   680,  680,  680,  680,  128,  680,  680,  680,  680,  680,
 /*  5670 */   680,   47,  680,  680,  680,  680,   50,  139,   59,  144,
 /*  5680 */    56,  164,   44,  680,  154,  680,  680,  132,  276,  680,
 /*  5690 */   680,  680,  680,  680,  680,  680,  680,  171,  890,  494,
 /*  5700 */   680,  680,  153,  184,  108,  680,  680,  680,  150,  680,
 /*  5710 */   680,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  5720 */   490,  487,  680,  680,  680,  277,  279,  281,  680,  680,
 /*  5730 */   291,  680,  680,  680,  302,  306,  311,  319,  680,  330,
 /*  5740 */   680,  680,  337,  680,  680,  680,  680,  680,  680,  147,
 /*  5750 */   680,  680,  680,  426,  441,  448,  451,  146,  148,  149,
 /*  5760 */   151,  152,  495,  445,  680,   62,   65,  680,  680,  680,
 /*  5770 */   680,  680,  680,  680,  680,  680,  680,  128,  680,  680,
 /*  5780 */   680,  680,  680,  680,   47,  680,  680,  680,  680,   50,
 /*  5790 */   139,   59,  144,   56,  164,   44,  680,  154,  680,  680,
 /*  5800 */   132,  276,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  5810 */   171,  898,  494,  680,  680,  153,  184,  108,  680,  680,
 /*  5820 */   680,  150,  680,  680,  454,  463,  472,  475,  466,  469,
 /*  5830 */   478,  484,  481,  490,  487,  680,  680,  680,  277,  279,
 /*  5840 */   281,  680,  680,  291,  680,  680,  680,  302,  306,  311,
 /*  5850 */   319,  680,  330,  680,  680,  337,  680,  680,  680,  680,
 /*  5860 */   680,  680,  147,  680,  680,  680,  426,  441,  448,  451,
 /*  5870 */   146,  148,  149,  151,  152,  495,  445,  680,   62,   65,
 /*  5880 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  5890 */   128,  680,  680,  680,  680,  680,  680,   47,  680,  680,
 /*  5900 */   680,  680,   50,  139,   59,  144,   56,  164,   44,  680,
 /*  5910 */   154,  680,  680,  132,  276,  680,  680,  680,  680,  680,
 /*  5920 */   680,  680,  680,  171,  902,  494,  680,  680,  153,  184,
 /*  5930 */   108,  680,  680,  680,  150,  680,  680,  454,  463,  472,
 /*  5940 */   475,  466,  469,  478,  484,  481,  490,  487,  680,  680,
 /*  5950 */   680,  277,  279,  281,  680,  680,  291,  680,  680,  680,
 /*  5960 */   302,  306,  311,  319,  680,  330,  680,  680,  337,  680,
 /*  5970 */   680,  680,  680,  680,  680,  147,  680,  680,  680,  426,
 /*  5980 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  5990 */   680,   62,   65,  680,  680,  680,  680,  680,  680,  680,
 /*  6000 */   680,  680,  680,  128,  680,  680,  680,  680,  680,  680,
 /*  6010 */    47,  680,  680,  680,  680,   50,  139,   59,  144,   56,
 /*  6020 */   164,   44,  680,  154,  680,  680,  132,  276,  680,  680,
 /*  6030 */   680,  680,  680,  680,  680,  680,  171,  904,  494,  680,
 /*  6040 */   680,  153,  184,  108,  680,  680,  680,  150,  680,  680,
 /*  6050 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  6060 */   487,  680,  680,  680,  277,  279,  281,  680,  680,  291,
 /*  6070 */   680,  680,  680,  302,  306,  311,  319,  680,  330,  680,
 /*  6080 */   680,  337,  680,  680,  680,  680,  680,  680,  147,  680,
 /*  6090 */   680,  680,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  6100 */   152,  495,  445,  680,   62,   65,  680,  680,  680,  680,
 /*  6110 */   680,  680,  680,  680,  680,  680,  128,  680,  680,  680,
 /*  6120 */   680,  680,  680,   47,  680,  680,  680,  680,   50,  139,
 /*  6130 */    59,  144,   56,  164,   44,  680,  154,  680,  680,  132,
 /*  6140 */   276,  680,  680,  680,  680,  680,  680,  680,  680,  171,
 /*  6150 */   680,  494,  680,  680,  153,  184,  108,  680,  680,  680,
 /*  6160 */   150,  680,  680,  454,  463,  472,  475,  466,  469,  478,
 /*  6170 */   484,  481,  490,  487,  680,  680,  680,  277,  279,  281,
 /*  6180 */   680,  680,  291,  934,  680,  680,  302,  306,  311,  319,
 /*  6190 */   930,  330,  680,  680,  337,  680,  680,  680,  680,  680,
 /*  6200 */   680,  147,  680,  680,  663,  426,  441,  448,  451,  146,
 /*  6210 */   148,  149,  151,  152,  495,  680,  680,  139,  680,  680,
 /*  6220 */   680,  680,  680,  680,  680,  680,   30,   42,  367,   33,
 /*  6230 */   680,  632,  680,  726,  680,  855,  868,  171,  680,  494,
 /*  6240 */   680,  680,  680,  878,  343,  680,  680,  905,  911,  912,
 /*  6250 */   680,  916,  917,  918,  919,  920,  921,  922,  923,  924,
 /*  6260 */   925,  926,  120,  121,  122,  277,  279,  281,  680,  680,
 /*  6270 */   291,  930,  680,  680,  302,  306,  311,  319,  680,  330,
 /*  6280 */   680,  680,  337,  680,  680,  663,  680,  344,  345,  346,
 /*  6290 */   347,  348,  349,  426,  441,  448,  451,  680,  139,  680,
 /*  6300 */   680,  680,  495,  680,  680,  680,  388,   30,   42,  680,
 /*  6310 */    33,  680,  632,  680,  726,  680,  855,  868,  171,  680,
 /*  6320 */   494,  680,  343,  680,  878,  680,  680,  680,  905,  911,
 /*  6330 */   912,  680,  916,  917,  918,  919,  920,  921,  922,  923,
 /*  6340 */   924,  925,  926,  120,  121,  122,  277,  279,  281,  680,
 /*  6350 */   680,  291,  680,  680,  680,  302,  306,  311,  319,  680,
 /*  6360 */   330,  680,  680,  337,  680,  344,  345,  346,  347,  348,
 /*  6370 */   349,  680,  680,  680,  426,  441,  448,  451,  680,  680,
 /*  6380 */   680,  680,  680,  495, 1390,    1,    2,  932,    4,    5,
 /*  6390 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*  6400 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*  6410 */    26,   27,   28,   29,  680,  680,  102,   90,   88,   94,
 /*  6420 */    92,   96,   98,  100,   46,   52,   58,   61,   64,   67,
 /*  6430 */    55,   49,   76,   78,   86,   80,   82,   84,  680,  680,
 /*  6440 */   909,  913,  914,  891,  906,  915,   73,   69,   53,  680,
 /*  6450 */    62,   65,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  6460 */   680,  680,  128,  680,  680,  680,  680,  125,  680,   47,
 /*  6470 */   927,  928,  929,  680,   50,  139,   59,  144,   56,  164,
 /*  6480 */    44,  680,  154,  680,  680,  132,  123,  680,  680,  680,
 /*  6490 */   680,  680,  680,  680,  680,  171,  178,  180,  179,  143,
 /*  6500 */   153,  184,  108,  680,  680,  680,  150,  680,  680,  109,
 /*  6510 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  6520 */   120,  121,  122,   90,   88,   94,   92,   96,   98,  100,
 /*  6530 */    46,   52,   58,   61,   64,   67,   55,   49,   76,   78,
 /*  6540 */    86,   80,   82,   84,  680,  680,  680,  147,  680,  680,
 /*  6550 */   663,  680,   73,   69,  680,  146,  148,  149,  151,  152,
 /*  6560 */   680,  680,  680,  680,  680,  680,    3,    4,    5,    6,
 /*  6570 */     7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
 /*  6580 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /*  6590 */    27,   28,   29,  653,  659,  660,  680,  109,  110,  111,
 /*  6600 */   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,
 /*  6610 */   122,  680,  680,  680,  680,  680,  680,  680,  680,  909,
 /*  6620 */   913,  914,  891,  906,  915,  680,  274,  253,  254,  255,
 /*  6630 */   256,  257,  258,  259,  260,  261,  263,  264,  265,  266,
 /*  6640 */   267,  268,  269,  270,  271,  272,  273,  680,  680,  927,
 /*  6650 */   928,  929,   46,   52,   58,   61,   64,   67,   55,   49,
 /*  6660 */    76,   78,   86,   80,   82,   84,  680,  680,  251,  680,
 /*  6670 */   680,  680,  680,  680,   73,   69,  680,  155,  680,  680,
 /*  6680 */   496,  262,  275,  680,  680,  680,  680,  178,  180,  179,
 /*  6690 */   143,  680,  493,  161,  680,  680,  597,  593,  596,  680,
 /*  6700 */   680,  680,  433,  435,  437,  439,  274,  253,  254,  255,
 /*  6710 */   256,  257,  258,  259,  260,  261,  263,  264,  265,  266,
 /*  6720 */   267,  268,  269,  270,  271,  272,  273,   88,   94,   92,
 /*  6730 */    96,   98,  100,   46,   52,   58,   61,   64,   67,   55,
 /*  6740 */    49,   76,   78,   86,   80,   82,   84,  680,  284,  680,
 /*  6750 */   680,  680,  680,  680,  155,   73,   69,  680,  680,  680,
 /*  6760 */   496,  262,  275,  680,  178,  180,  179,  143,  680,  680,
 /*  6770 */   680,  680,  493,  592,  593,  596,  680,  680,  680,  680,
 /*  6780 */   680,  680,  433,  435,  437,  439,  680,  680,  680,  680,
 /*  6790 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  274,
 /*  6800 */   253,  254,  255,  256,  257,  258,  259,  260,  261,  263,
 /*  6810 */   264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
 /*  6820 */    94,   92,   96,   98,  100,   46,   52,   58,   61,   64,
 /*  6830 */    67,   55,   49,   76,   78,   86,   80,   82,   84,  680,
 /*  6840 */   680,  289,  680,  680,  680,  680,  680,   73,   69,  680,
 /*  6850 */   680,  680,  680,  496,  262,  275,  663,  680,  680,  680,
 /*  6860 */   680,  680,  680,  680,  680,  493,  680,  680,  680,  680,
 /*  6870 */   680,  680,  680,  680,  680,  433,  435,  437,  439,  274,
 /*  6880 */   253,  254,  255,  256,  257,  258,  259,  260,  261,  263,
 /*  6890 */   264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
 /*  6900 */   659,  660,  680,  109,  110,  111,  112,  113,  114,  115,
 /*  6910 */   116,  117,  118,  119,  120,  121,  122,  680,  680,  680,
 /*  6920 */   680,  301,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  6930 */   680,  680,  680,  496,  262,  275,  399,  353,  680,  680,
 /*  6940 */   680,  680,  680,  203,  680,  493,  206,  680,  680,  680,
 /*  6950 */   680,  680,  343,  680,  680,  433,  435,  437,  439,  680,
 /*  6960 */   680,  202,  680,  680,  680,  680,  680,  196,  680,  207,
 /*  6970 */   680,  680,  274,  253,  254,  255,  256,  257,  258,  259,
 /*  6980 */   260,  261,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  6990 */   271,  272,  273,  680,  680,  344,  345,  346,  347,  348,
 /*  7000 */   349,  680,  384,  410,  411,  680,  680,  680,  205,  680,
 /*  7010 */   680,  680,  680,  680,  304,  357,  204,  193,  195,  198,
 /*  7020 */   197,  680,  203,  680,  680,  199,  496,  262,  275,  395,
 /*  7030 */   680,  343,  680,  680,  680,  680,  680,  680,  493,  680,
 /*  7040 */   202,  680,  680,  680,  680,  343,  196,  680,  433,  435,
 /*  7050 */   437,  439,  274,  253,  254,  255,  256,  257,  258,  259,
 /*  7060 */   260,  261,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  7070 */   271,  272,  273,  680,  344,  345,  346,  347,  348,  349,
 /*  7080 */   680,  680,  373,  374,  680,  680,  680,  194,  344,  345,
 /*  7090 */   346,  347,  348,  349,  309,  192,  193,  195,  198,  197,
 /*  7100 */   680,  203,  680,  680,  206,  680,  496,  262,  275,  680,
 /*  7110 */   680,  680,  680,  680,  680,  680,  680,  680,  493,  202,
 /*  7120 */   406,  680,  680,  680,  680,  196,  680,  680,  433,  435,
 /*  7130 */   437,  439,  680,  680,  680,  680,  343,  680,  680,  680,
 /*  7140 */   680,  680,  680,  680,  680,  274,  253,  254,  255,  256,
 /*  7150 */   257,  258,  259,  260,  261,  263,  264,  265,  266,  267,
 /*  7160 */   268,  269,  270,  271,  272,  273,  205,  680,  680,  680,
 /*  7170 */   680,  680,  680,  680,  204,  193,  195,  198,  197,  344,
 /*  7180 */   345,  346,  347,  348,  349,  680,  680,  313,  680,  680,
 /*  7190 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  496,
 /*  7200 */   262,  275,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7210 */   680,  493,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7220 */   680,  433,  435,  437,  439,  274,  253,  254,  255,  256,
 /*  7230 */   257,  258,  259,  260,  261,  263,  264,  265,  266,  267,
 /*  7240 */   268,  269,  270,  271,  272,  273,  680,  680,  680,  680,
 /*  7250 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7260 */   680,  680,  680,  680,  680,  680,  680,  321,  680,  680,
 /*  7270 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  496,
 /*  7280 */   262,  275,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7290 */   680,  493,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7300 */   680,  433,  435,  437,  439,  680,  680,  680,  680,  680,
 /*  7310 */   680,  680,  680,  680,  680,  680,  680,  680,  274,  253,
 /*  7320 */   254,  255,  256,  257,  258,  259,  260,  261,  263,  264,
 /*  7330 */   265,  266,  267,  268,  269,  270,  271,  272,  273,  680,
 /*  7340 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7350 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7360 */   328,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7370 */   680,  680,  496,  262,  275,  680,  680,  680,  680,  680,
 /*  7380 */   680,  680,  680,  680,  493,  680,  680,  680,  680,  680,
 /*  7390 */   680,  680,  680,  680,  433,  435,  437,  439,  274,  253,
 /*  7400 */   254,  255,  256,  257,  258,  259,  260,  261,  263,  264,
 /*  7410 */   265,  266,  267,  268,  269,  270,  271,  272,  273,  680,
 /*  7420 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7430 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7440 */   335,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7450 */   680,  680,  496,  262,  275,  680,  680,  680,  680,  680,
 /*  7460 */   680,  680,  680,  680,  493,  680,  680,  680,  680,  680,
 /*  7470 */   680,  680,  680,  680,  433,  435,  437,  439,  680,  680,
 /*  7480 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7490 */   680,  274,  253,  254,  255,  256,  257,  258,  259,  260,
 /*  7500 */   261,  263,  264,  265,  266,  267,  268,  269,  270,  271,
 /*  7510 */   272,  273,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7520 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7530 */   680,  680,  680,  501,  680,  680,  680,  680,  680,  680,
 /*  7540 */   680,  680,  680,  680,  680,  496,  262,  275,  680,  680,
 /*  7550 */   680,  680,  680,  680,  680,  680,  680,  493,  680,  680,
 /*  7560 */   680,  680,  680,  680,  680,  680,  680,  433,  435,  437,
 /*  7570 */   439,  274,  253,  254,  255,  256,  257,  258,  259,  260,
 /*  7580 */   261,  263,  264,  265,  266,  267,  268,  269,  270,  271,
 /*  7590 */   272,  273,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7600 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7610 */   680,  680,  680,  508,  680,  680,  680,  680,  680,  680,
 /*  7620 */   680,  680,  680,  680,  680,  496,  262,  275,  680,  680,
 /*  7630 */   680,  680,  680,  680,  680,  680,  680,  493,  680,  680,
 /*  7640 */   680,  680,  680,  680,  680,  680,  680,  433,  435,  437,
 /*  7650 */   439,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7660 */   680,  680,  680,  680,  274,  253,  254,  255,  256,  257,
 /*  7670 */   258,  259,  260,  261,  263,  264,  265,  266,  267,  268,
 /*  7680 */   269,  270,  271,  272,  273,  680,  680,  680,  680,  680,
 /*  7690 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7700 */   680,  680,  680,  680,  680,  680,  514,  680,  680,  680,
 /*  7710 */   680,  680,  680,  680,  680,  680,  680,  680,  496,  262,
 /*  7720 */   275,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7730 */   493,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7740 */   433,  435,  437,  439,  274,  253,  254,  255,  256,  257,
 /*  7750 */   258,  259,  260,  261,  263,  264,  265,  266,  267,  268,
 /*  7760 */   269,  270,  271,  272,  273,  680,  680,  680,  680,  680,
 /*  7770 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7780 */   680,  680,  680,  680,  680,  680,  520,  680,  680,  680,
 /*  7790 */   680,  680,  680,  680,  680,  680,  680,  680,  496,  262,
 /*  7800 */   275,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7810 */   493,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7820 */   433,  435,  437,  439,  680,  680,  680,  680,  680,  680,
 /*  7830 */   680,  680,  680,  680,  680,  680,  680,  274,  253,  254,
 /*  7840 */   255,  256,  257,  258,  259,  260,  261,  263,  264,  265,
 /*  7850 */   266,  267,  268,  269,  270,  271,  272,  273,  680,  680,
 /*  7860 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7870 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  537,
 /*  7880 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7890 */   680,  496,  262,  275,  680,  680,  680,  680,  680,  680,
 /*  7900 */   680,  680,  680,  493,  680,  680,  680,  680,  680,  680,
 /*  7910 */   680,  680,  680,  433,  435,  437,  439,  274,  253,  254,
 /*  7920 */   255,  256,  257,  258,  259,  260,  261,  263,  264,  265,
 /*  7930 */   266,  267,  268,  269,  270,  271,  272,  273,  680,  680,
 /*  7940 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7950 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  544,
 /*  7960 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  7970 */   680,  496,  262,  275,  680,  680,  680,  680,  680,  680,
 /*  7980 */   680,  680,  680,  493,  680,  680,  680,  680,  680,  680,
 /*  7990 */   680,  680,  680,  433,  435,  437,  439,  680,  680,  680,
 /*  8000 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8010 */   274,  253,  254,  255,  256,  257,  258,  259,  260,  261,
 /*  8020 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  8030 */   273,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8040 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8050 */   680,  680,  549,  680,  680,  680,  680,  680,  680,  680,
 /*  8060 */   680,  680,  680,  680,  496,  262,  275,  680,  680,  680,
 /*  8070 */   680,  680,  680,  680,  680,  680,  493,  680,  680,  680,
 /*  8080 */   680,  680,  680,  680,  680,  680,  433,  435,  437,  439,
 /*  8090 */   274,  253,  254,  255,  256,  257,  258,  259,  260,  261,
 /*  8100 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  8110 */   273,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8120 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8130 */   680,  680,  560,  680,  680,  680,  680,  680,  680,  680,
 /*  8140 */   680,  680,  680,  680,  496,  262,  275,  680,  680,  680,
 /*  8150 */   680,  680,  680,  680,  680,  680,  493,  680,  680,  680,
 /*  8160 */   680,  680,  680,  680,  680,  680,  433,  435,  437,  439,
 /*  8170 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8180 */   680,  680,  680,  274,  253,  254,  255,  256,  257,  258,
 /*  8190 */   259,  260,  261,  263,  264,  265,  266,  267,  268,  269,
 /*  8200 */   270,  271,  272,  273,  680,  680,  680,  680,  680,  680,
 /*  8210 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8220 */   680,  680,  680,  680,  680,  568,  680,  680,  680,  680,
 /*  8230 */   680,  680,  680,  680,  680,  680,  680,  496,  262,  275,
 /*  8240 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  493,
 /*  8250 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  433,
 /*  8260 */   435,  437,  439,  274,  253,  254,  255,  256,  257,  258,
 /*  8270 */   259,  260,  261,  263,  264,  265,  266,  267,  268,  269,
 /*  8280 */   270,  271,  272,  273,  680,  680,  680,  680,  680,  680,
 /*  8290 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8300 */   680,  680,  680,  680,  680,  749,  680,  680,  680,  680,
 /*  8310 */   680,  680,  680,  680,  680,  680,  680,  496,  262,  275,
 /*  8320 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  493,
 /*  8330 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  433,
 /*  8340 */   435,  437,  439,  680,  680,  680,  680,  680,  680,  680,
 /*  8350 */   680,  680,  680,  680,  680,  680,  274,  253,  254,  255,
 /*  8360 */   256,  257,  258,  259,  260,  261,  263,  264,  265,  266,
 /*  8370 */   267,  268,  269,  270,  271,  272,  273,  680,  680,  680,
 /*  8380 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8390 */   680,  680,  680,  680,  680,  680,  680,  680,  756,  680,
 /*  8400 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8410 */   496,  262,  275,  680,  680,  680,  680,  680,  680,  680,
 /*  8420 */   680,  680,  493,  680,  680,  680,  680,  680,  680,  680,
 /*  8430 */   680,  680,  433,  435,  437,  439,  274,  253,  254,  255,
 /*  8440 */   256,  257,  258,  259,  260,  261,  263,  264,  265,  266,
 /*  8450 */   267,  268,  269,  270,  271,  272,  273,  680,  680,  680,
 /*  8460 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8470 */   680,  680,  680,  680,  680,  680,  680,  680,  763,  680,
 /*  8480 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8490 */   496,  262,  275,  680,  680,  680,  680,  680,  680,  680,
 /*  8500 */   680,  680,  493,  680,  680,  680,  680,  680,  680,  680,
 /*  8510 */   680,  680,  433,  435,  437,  439,  680,  680,  680,  680,
 /*  8520 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  274,
 /*  8530 */   253,  254,  255,  256,  257,  258,  259,  260,  261,  263,
 /*  8540 */   264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
 /*  8550 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8560 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8570 */   680,  770,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8580 */   680,  680,  680,  496,  262,  275,  680,  680,  680,  680,
 /*  8590 */   680,  680,  680,  680,  680,  493,  680,  680,  680,  680,
 /*  8600 */   680,  680,  680,  680,  680,  433,  435,  437,  439,  274,
 /*  8610 */   253,  254,  255,  256,  257,  258,  259,  260,  261,  263,
 /*  8620 */   264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
 /*  8630 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8640 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8650 */   680,  781,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8660 */   680,  680,  680,  496,  262,  275,  680,  680,  680,  680,
 /*  8670 */   680,  680,  680,  680,  680,  493,  680,  680,  680,  680,
 /*  8680 */   680,  680,  680,  680,  680,  433,  435,  437,  439,  680,
 /*  8690 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8700 */   680,  680,  274,  253,  254,  255,  256,  257,  258,  259,
 /*  8710 */   260,  261,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  8720 */   271,  272,  273,  680,  680,  680,  680,  680,  680,  680,
 /*  8730 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8740 */   680,  680,  680,  680,  788,  680,  680,  680,  680,  680,
 /*  8750 */   680,  680,  680,  680,  680,  680,  496,  262,  275,  680,
 /*  8760 */   680,  680,  680,  680,  680,  680,  680,  680,  493,  680,
 /*  8770 */   680,  680,  680,  680,  680,  680,  680,  680,  433,  435,
 /*  8780 */   437,  439,  274,  253,  254,  255,  256,  257,  258,  259,
 /*  8790 */   260,  261,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  8800 */   271,  272,  273,  680,  680,  680,  680,  680,  680,  680,
 /*  8810 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8820 */   680,  680,  680,  680,  795,  680,  680,  680,  680,  680,
 /*  8830 */   680,  680,  680,  680,  680,  680,  496,  262,  275,  680,
 /*  8840 */   680,  680,  680,  680,  680,  680,  680,  680,  493,  680,
 /*  8850 */   680,  680,  680,  680,  680,  680,  680,  680,  433,  435,
 /*  8860 */   437,  439,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8870 */   680,  680,  680,  680,  680,  274,  253,  254,  255,  256,
 /*  8880 */   257,  258,  259,  260,  261,  263,  264,  265,  266,  267,
 /*  8890 */   268,  269,  270,  271,  272,  273,  680,  680,  680,  680,
 /*  8900 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8910 */   680,  680,  680,  680,  680,  680,  680,  802,  680,  680,
 /*  8920 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  496,
 /*  8930 */   262,  275,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8940 */   680,  493,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8950 */   680,  433,  435,  437,  439,  274,  253,  254,  255,  256,
 /*  8960 */   257,  258,  259,  260,  261,  263,  264,  265,  266,  267,
 /*  8970 */   268,  269,  270,  271,  272,  273,  680,  680,  680,  680,
 /*  8980 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  8990 */   680,  680,  680,  680,  680,  680,  680,  883,  680,  680,
 /*  9000 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  496,
 /*  9010 */   262,  275,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9020 */   680,  493,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9030 */   680,  433,  435,  437,  439,  680,  680,  680,  680,  680,
 /*  9040 */   680,  680,  680,  680,  680,  680,  680,  680,  274,  253,
 /*  9050 */   254,  255,  256,  257,  258,  259,  260,  261,  263,  264,
 /*  9060 */   265,  266,  267,  268,  269,  270,  271,  272,  273,  680,
 /*  9070 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9080 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9090 */   889,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9100 */   680,  680,  496,  262,  275,  680,  680,  680,  680,  680,
 /*  9110 */   680,  680,  680,  680,  493,  680,  680,  680,  680,  680,
 /*  9120 */   680,  680,  680,  680,  433,  435,  437,  439,  274,  253,
 /*  9130 */   254,  255,  256,  257,  258,  259,  260,  261,  263,  264,
 /*  9140 */   265,  266,  267,  268,  269,  270,  271,  272,  273,  680,
 /*  9150 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9160 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9170 */   897,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9180 */   680,  680,  496,  262,  275,  680,  680,  680,  680,  680,
 /*  9190 */   680,  680,  680,  680,  493,  680,  680,  680,  680,  680,
 /*  9200 */   680,  680,  680,  680,  433,  435,  437,  439,  680,  680,
 /*  9210 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9220 */   680,  274,  253,  254,  255,  256,  257,  258,  259,  260,
 /*  9230 */   261,  263,  264,  265,  266,  267,  268,  269,  270,  271,
 /*  9240 */   272,  273,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9250 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9260 */   680,  680,  680,  903,  680,  680,  680,  680,  680,  680,
 /*  9270 */   680,  680,  680,  680,  680,  496,  262,  275,  680,  680,
 /*  9280 */   680,  680,  680,  680,  680,  680,  680,  493,  680,  680,
 /*  9290 */   680,  680,  680,  680,  680,  680,  680,  433,  435,  437,
 /*  9300 */   439,  274,  253,  254,  255,  256,  257,  258,  259,  260,
 /*  9310 */   261,  263,  264,  265,  266,  267,  268,  269,  270,  271,
 /*  9320 */   272,  273,  680,  680,  680,  104,  680,  680,  102,   90,
 /*  9330 */    88,   94,   92,   96,   98,  100,   46,   52,   58,   61,
 /*  9340 */    64,   67,   55,   49,   76,   78,   86,   80,   82,   84,
 /*  9350 */   680,  680,  680,  680,  680,  252,  262,  275,   73,   69,
 /*  9360 */   680,  680,  680,  680,  680,  680,  680,  493,  680,  680,
 /*  9370 */   680,  680,  680,  680,  680,  680,  680,  433,  435,  437,
 /*  9380 */   439,  680,  680,  680,   75,   53,  680,   62,   65,  680,
 /*  9390 */   680,  680,  680,  182,  680,  680,  680,  680,  680,  128,
 /*  9400 */   680,  680,  680,  680,  680,  680,   47,  680,  680,  680,
 /*  9410 */   680,   50,  139,   59,  144,   56,  164,   44,  629,  154,
 /*  9420 */   680,   53,  136,   62,   65,  680,  680,  680,  680,  182,
 /*  9430 */   680,  680,  171,  680,  680,  128,  680,  153,  184,  108,
 /*  9440 */   680,  680,   47,  150,  680,  680,  680,   50,  139,   59,
 /*  9450 */   144,   56,  164,   44,  606,  154,  680,  663,  136,  680,
 /*  9460 */   680,  680,  680,  680,  680,  680,  680,  680,  171,  680,
 /*  9470 */   680,  680,  680,  153,  184,  108,  680,  680,  680,  150,
 /*  9480 */   680,  680,  680,  680,  147,  680,  680,  680,  680,  680,
 /*  9490 */   680,  680,  146,  148,  149,  151,  152,  680,  680,  680,
 /*  9500 */   680,  911,  912,  680,  109,  110,  111,  112,  113,  114,
 /*  9510 */   115,  116,  117,  118,  119,  120,  121,  122,  680,  680,
 /*  9520 */   147,  680,  680,  680,  680,  680,  680,  680,  146,  148,
 /*  9530 */   149,  151,  152,  680,  680,  680,   53,  680,   62,   65,
 /*  9540 */   680,  680,  680,  680,  182,  680,  680,  680,  680,  680,
 /*  9550 */   128,  680,  680,  680,  680,  680,  680,   47,  680,  680,
 /*  9560 */   680,  680,   50,  139,   59,  144,   56,  164,   44,  602,
 /*  9570 */   154,  680,   53,  136,   62,   65,  680,  680,  680,  680,
 /*  9580 */   182,  680,  680,  171,  680,  680,  128,  680,  153,  184,
 /*  9590 */   108,  680,  680,   47,  150,  680,  680,  680,   50,  139,
 /*  9600 */    59,  144,   56,  164,   44,  167,  154,  680,  234,  136,
 /*  9610 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  171,
 /*  9620 */   680,  680,  680,  680,  153,  184,  108,  680,  680,  680,
 /*  9630 */   150,  219,  680,  680,  680,  147,  680,  680,  680,  680,
 /*  9640 */   680,  680,  680,  146,  148,  149,  151,  152,  680,  680,
 /*  9650 */   680,  680,  680,  680,  680,  109,  110,  111,  112,  113,
 /*  9660 */   114,  115,  116,  117,  118,  119,  120,  121,  122,  680,
 /*  9670 */   680,  147,  680,  680,  680,  680,  680,  680,  680,  146,
 /*  9680 */   148,  149,  151,  152,  680,  680,  680,   53,  680,   62,
 /*  9690 */    65,  680,  680,  680,  680,  182,  680,  680,  680,  680,
 /*  9700 */   680,  128,  680,  680,  680,  680,  680,  680,   47,  680,
 /*  9710 */   680,  680,  680,   50,  139,   59,  144,   56,  164,   44,
 /*  9720 */   177,  154,  680,   53,  136,   62,   65,  680,  680,  680,
 /*  9730 */   680,  182,  680,  680,  171,  680,  680,  128,  680,  153,
 /*  9740 */   184,  108,  680,  680,   47,  150,  680,  680,  680,   50,
 /*  9750 */   139,   59,  144,   56,  164,   44,  576,  154,  680,  680,
 /*  9760 */   136,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9770 */   171,  680,  680,  680,  680,  153,  184,  108,  680,  680,
 /*  9780 */   680,  150,  680,  680,  680,  680,  147,  680,  680,  680,
 /*  9790 */   680,  680,  680,  680,  146,  148,  149,  151,  152,  680,
 /*  9800 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9810 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9820 */   680,  680,  147,  680,  680,  680,  680,  680,  680,  680,
 /*  9830 */   146,  148,  149,  151,  152,  680,  680,  680,   53,  680,
 /*  9840 */    62,   65,  680,  680,  680,  680,  182,  680,  680,  680,
 /*  9850 */   680,  680,  128,  680,  680,  680,  680,  680,  680,   47,
 /*  9860 */   680,  680,  680,  680,   50,  139,   59,  144,   56,  164,
 /*  9870 */    44,  583,  154,  680,   53,  136,   62,   65,  680,  680,
 /*  9880 */   680,  680,  182,  680,  680,  171,  680,  680,  128,  680,
 /*  9890 */   153,  184,  108,  680,  680,   47,  150,  680,  680,  680,
 /*  9900 */    50,  139,   59,  144,   56,  164,   44,  589,  154,  680,
 /*  9910 */   680,  136,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9920 */   680,  171,  680,  680,  680,  680,  153,  184,  108,  680,
 /*  9930 */   680,  680,  150,  680,  680,  680,  680,  147,  680,  680,
 /*  9940 */   680,  680,  680,  680,  680,  146,  148,  149,  151,  152,
 /*  9950 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9960 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /*  9970 */   680,  680,  680,  147,  680,  680,  680,  680,  680,  680,
 /*  9980 */   680,  146,  148,  149,  151,  152,  680,  680,  680,   53,
 /*  9990 */   680,   62,   65,  680,  680,  680,  680,  182,  680,  680,
 /* 10000 */   680,  680,  680,  128,  680,  680,  680,  680,  680,  680,
 /* 10010 */    47,  680,  680,  680,  680,   50,  139,   59,  144,   56,
 /* 10020 */   164,   44,  613,  154,  680,   53,  136,   62,   65,  680,
 /* 10030 */   680,  680,  680,  182,  680,  680,  171,  680,  680,  128,
 /* 10040 */   680,  153,  184,  108,  680,  680,   47,  150,  680,  680,
 /* 10050 */   680,   50,  139,   59,  144,   56,  164,   44,  619,  154,
 /* 10060 */   680,  680,  136,  680,  680,  680,  680,  680,  680,  680,
 /* 10070 */   680,  680,  171,  680,  680,  680,  680,  153,  184,  108,
 /* 10080 */   680,  680,  680,  150,  680,  680,  680,  680,  147,  680,
 /* 10090 */   680,  680,  680,  680,  680,  680,  146,  148,  149,  151,
 /* 10100 */   152,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 10110 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 10120 */   680,  680,  680,  680,  147,  680,  680,  680,  680,  680,
 /* 10130 */   680,  680,  146,  148,  149,  151,  152,  680,  680,  680,
 /* 10140 */   680,  104,  680,  680,  102,   90,   88,   94,   92,   96,
 /* 10150 */    98,  100,   46,   52,   58,   61,   64,   67,   55,   49,
 /* 10160 */    76,   78,   86,   80,   82,   84,  680,  680,  680,  680,
 /* 10170 */   680,  680,  680,  104,   73,   69,  102,   90,   88,   94,
 /* 10180 */    92,   96,   98,  100,   46,   52,   58,   61,   64,   67,
 /* 10190 */    55,   49,   76,   78,   86,   80,   82,   84,  680,  680,
 /* 10200 */    53,  680,   62,   65,  680,  124,   73,   69,  598,  680,
 /* 10210 */   680,  680,  680,  680,  128,  680,  680,  680,  680,  680,
 /* 10220 */   680,   47,  106,  680,  680,  680,   50,  139,   59,  144,
 /* 10230 */    56,  164,   44,  680,  154,  680,   53,  132,   62,   65,
 /* 10240 */   680,  680,  680,  680,  680,  680,  680,  171,  680,  680,
 /* 10250 */   128,  680,  153,  184,  108,  680,  680,   47,  150,  680,
 /* 10260 */   680,  680,   50,  139,   59,  144,   56,  164,   44,  234,
 /* 10270 */   154,  680,  680,  156,  680,  680,  680,  680,  680,  680,
 /* 10280 */   680,  680,  680,  171,  680,  680,  680,  680,  159,  184,
 /* 10290 */   108,  680,  189,  680,  150,  680,  160,  680,  680,  147,
 /* 10300 */   680,  680,  680,  680,  680,  680,  218,  146,  148,  149,
 /* 10310 */   151,  152,  680,  680,  680,  680,  109,  110,  111,  112,
 /* 10320 */   113,  114,  115,  116,  117,  118,  119,  120,  121,  122,
 /* 10330 */   680,  680,  680,  680,  680,  158,  680,  680,  680,  680,
 /* 10340 */   680,  680,  680,  157,  148,  149,  151,  152,  680,  680,
 /* 10350 */   680,   53,  680,   62,   65,  680,  680,  680,  680,  182,
 /* 10360 */   680,  680,  680,  680,  680,  128,  680,  680,  680,  680,
 /* 10370 */   680,  680,   47,  680,  680,  680,  680,   50,  139,   59,
 /* 10380 */   144,   56,  164,   44,  680,  154,  680,   53,  136,   62,
 /* 10390 */    65,  680,  680,  680,  680,  680,  680,  680,  171,  680,
 /* 10400 */   680,  128,  680,  153,  184,  108,  680,  680,   47,  150,
 /* 10410 */   680,  680,  680,   50,  139,   59,  144,   56,  164,   44,
 /* 10420 */   680,  154,  680,  680,  132,  680,  680,  680,  680,  680,
 /* 10430 */   680,  680,  680,  680,  171,  680,  680,  680,  680,  153,
 /* 10440 */   184,  108,  680,  680,  680,  150,  680,  680,  680,  680,
 /* 10450 */   147,  680,  680,  680,  680,  680,  680,  680,  146,  148,
 /* 10460 */   149,  151,  152,  680,  680,  680,  680,  680,  680,  680,
 /* 10470 */   680,  680,  680,  680,  680,  680,  680,  680,  498,  680,
 /* 10480 */   680,  680,  680,  680,  680,  680,  147,  680,  680,  680,
 /* 10490 */   680,  680,  680,  680,  146,  148,  149,  151,  152,  680,
 /* 10500 */   680,  680,  680,  104,  680,  680,  102,   90,   88,   94,
 /* 10510 */    92,   96,   98,  100,   46,   52,   58,   61,   64,   67,
 /* 10520 */    55,   49,   76,   78,   86,   80,   82,   84,  680,   53,
 /* 10530 */   680,   62,   65,  680,  680,  680,   73,   69,  680,  680,
 /* 10540 */   680,  680,  680,  128,  680,  680,  680,  680,  680,  334,
 /* 10550 */    47,  680,  680,  680,  680,   50,  139,   59,  144,   56,
 /* 10560 */   164,   44,  680,  154,  680,  680,  132,  680,  680,  680,
 /* 10570 */   680,  680,  680,  680,  680,  680,  171,  680,  680,  680,
 /* 10580 */   680,  153,  184,  108,  680,  680,  680,  150,  680,  358,
 /* 10590 */   104,  680,  680,  102,   90,   88,   94,   92,   96,   98,
 /* 10600 */   100,   46,   52,   58,   61,   64,   67,   55,   49,   76,
 /* 10610 */    78,   86,   80,   82,   84,  680,  680,  680,  680,  680,
 /* 10620 */   680,  680,  680,   73,   69,  680,  680,  680,  147,  680,
 /* 10630 */   680,   53,  680,   62,   65,  680,  146,  148,  149,  151,
 /* 10640 */   152,  680,  680,  680,  680,  128,  680,  680,  680, 1239,
 /* 10650 */   680,  680,   47,  680,  680,  680,  680,   50,  139,   59,
 /* 10660 */   144,   56,  164,   44,  680,  154,  680,   53,  132,   62,
 /* 10670 */    65,  680,  680,  680,  680,  680,  680,  680,  171,  680,
 /* 10680 */   680,  128,  680,  153,  184,  108,  680,  680,   47,  150,
 /* 10690 */   680,  368,  680,   50,  139,   59,  144,   56,  164,   44,
 /* 10700 */   680,  154,  680,  680,  132,  680,  680,  680,  680,  680,
 /* 10710 */   680,  680,  680,  680,  171,  680,  680,  680,  680,  153,
 /* 10720 */   184,  108,  680,  680,  680,  150,  680,  389,  680,  680,
 /* 10730 */   147,  680,  680,   53,  680,   62,   65,  680,  146,  148,
 /* 10740 */   149,  151,  152,  680,  680,  680,  680,  128,  680,  680,
 /* 10750 */   680,  680,  680,  680,   47,  680,  680,  680,  680,   50,
 /* 10760 */   139,   59,  144,   56,  164,   44,  147,  154,  680,   53,
 /* 10770 */   132,   62,   65,  680,  146,  148,  149,  151,  152,  680,
 /* 10780 */   171,  680,  680,  128,  680,  153,  184,  108,  680,  680,
 /* 10790 */    47,  150,  680,  396,  680,   50,  139,   59,  144,   56,
 /* 10800 */   164,   44,  680,  154,  680,  680,  132,  680,  680,  680,
 /* 10810 */   680,  680,  680,  680,  680,  680,  171,  680,  680,  680,
 /* 10820 */   680,  153,  184,  108,  680,  680,  680,  150,  680,  400,
 /* 10830 */   680,  680,  147,  680,  680,   53,  680,   62,   65,  680,
 /* 10840 */   146,  148,  149,  151,  152,  680,  680,  680,  680,  128,
 /* 10850 */   680,  680,  680,  680,  680,  680,   47,  680,  680,  680,
 /* 10860 */   680,   50,  139,   59,  144,   56,  164,   44,  147,  154,
 /* 10870 */   680,   53,  132,   62,   65,  680,  146,  148,  149,  151,
 /* 10880 */   152,  680,  171,  680,  680,  128,  680,  153,  184,  108,
 /* 10890 */   680,  680,   47,  150,  680,  407,  680,   50,  139,   59,
 /* 10900 */   144,   56,  164,   44,  680,  154,  680,  680,  132,  444,
 /* 10910 */   680,  680,  680,  680,  680,  680,  680,  680,  171,  680,
 /* 10920 */   680,  680,  680,  153,  184,  108,  680,  680,  680,  150,
 /* 10930 */   680,  680,  680,  680,  147,  680,  680,  680,  680,  680,
 /* 10940 */   680,  680,  146,  148,  149,  151,  152,  680,  680,  680,
 /* 10950 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 10960 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 10970 */   147,  680,  680,  680,  680,  680,  680,  680,  146,  148,
 /* 10980 */   149,  151,  152,  680,  680,  680,  680,  104,  680,  680,
 /* 10990 */   102,   90,   88,   94,   92,   96,   98,  100,   46,   52,
 /* 11000 */    58,   61,   64,   67,   55,   49,   76,   78,   86,   80,
 /* 11010 */    82,   84,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11020 */    73,   69,  680,  680,  443,  680,  680,  680,  680,  104,
 /* 11030 */   680,  680,  102,   90,   88,   94,   92,   96,   98,  100,
 /* 11040 */    46,   52,   58,   61,   64,   67,   55,   49,   76,   78,
 /* 11050 */    86,   80,   82,   84,  680,  680,  680,  680,  680,  680,
 /* 11060 */   680,  680,   73,   69,  680,  680,  447,  680,  680,  680,
 /* 11070 */   680,  104,  680,  680,  102,   90,   88,   94,   92,   96,
 /* 11080 */    98,  100,   46,   52,   58,   61,   64,   67,   55,   49,
 /* 11090 */    76,   78,   86,   80,   82,   84,  680,  680,  680,  680,
 /* 11100 */   680,  680,  680,  680,   73,   69,  680,  680,  450,  680,
 /* 11110 */   680,  680,  680,  104,  680,  680,  102,   90,   88,   94,
 /* 11120 */    92,   96,   98,  100,   46,   52,   58,   61,   64,   67,
 /* 11130 */    55,   49,   76,   78,   86,   80,   82,   84,  680,  680,
 /* 11140 */   680,  680,  680,  680,  680,  680,   73,   69,  680,  680,
 /* 11150 */   453,  680,  680,  680,  680,  104,  680,  680,  102,   90,
 /* 11160 */    88,   94,   92,   96,   98,  100,   46,   52,   58,   61,
 /* 11170 */    64,   67,   55,   49,   76,   78,   86,   80,   82,   84,
 /* 11180 */   680,   53,  680,   62,   65,  680,  680,  680,   73,   69,
 /* 11190 */   680,  680,  680,  680,  680,  128,  680,  680,  680,  680,
 /* 11200 */   680,  500,   47,  680,  680,  680,  680,   50,  139,   59,
 /* 11210 */   144,   56,  164,   44,  680,  154,  680,  680,  132,  680,
 /* 11220 */   680,  680,  680,  680,  680,  680,  680,  680,  171,  680,
 /* 11230 */   680,  680,  680,  153,  184,  108,  680,  104,  680,  150,
 /* 11240 */   102,   90,   88,   94,   92,   96,   98,  100,   46,   52,
 /* 11250 */    58,   61,   64,   67,   55,   49,   76,   78,   86,   80,
 /* 11260 */    82,   84,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11270 */    73,   69,  511,  680,  680,  680,  680,  680,  680,  680,
 /* 11280 */   147,  680,  680,  507,  680,  680,  680,  680,  146,  148,
 /* 11290 */   149,  151,  152,  680,  680,  680,  680,  104,  680,  680,
 /* 11300 */   102,   90,   88,   94,   92,   96,   98,  100,   46,   52,
 /* 11310 */    58,   61,   64,   67,   55,   49,   76,   78,   86,   80,
 /* 11320 */    82,   84,  680,   53,  680,   62,   65,  680,  680,  680,
 /* 11330 */    73,   69,  680,  680,  680,  680,  680,  128,  680,  680,
 /* 11340 */   680,  680,  680,  513,   47,  680,  680,  680,  680,   50,
 /* 11350 */   139,   59,  144,   56,  164,   44,  680,  154,  680,  680,
 /* 11360 */   132,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11370 */   171,  680,  680,  680,  680,  153,  184,  108,  680,  104,
 /* 11380 */   680,  150,  102,   90,   88,   94,   92,   96,   98,  100,
 /* 11390 */    46,   52,   58,   61,   64,   67,   55,   49,   76,   78,
 /* 11400 */    86,   80,   82,   84,  680,  680,  680,  680,  680,  680,
 /* 11410 */   680,  680,   73,   69,  680,  680,  680,  680,  680,  680,
 /* 11420 */   680,   53,  147,   62,   65,  680,  680,  680,  680,  680,
 /* 11430 */   146,  148,  149,  151,  152,  128,  680,  680,  680,  680,
 /* 11440 */   680,  680,   47,  234,  680,  680,  680,   50,  139,   59,
 /* 11450 */   144,   56,  164,   44,  680,  154,  680,  680,  156,  680,
 /* 11460 */   680,  680,  566,  680,  680,  680,  189,  680,  171,  680,
 /* 11470 */   680,  680,  680,  159,  184,  108,  680,  680,  680,  150,
 /* 11480 */   218,  680,  680,  680,  680,  680,  680,  680,  234,  680,
 /* 11490 */   109,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /* 11500 */   119,  120,  121,  122,  680,  680,  680,  649,  680,  680,
 /* 11510 */   680,  189,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11520 */   158,  680,  680,  680,  680,  218,  680,  680,  157,  148,
 /* 11530 */   149,  151,  152,  234,  680,  109,  110,  111,  112,  113,
 /* 11540 */   114,  115,  116,  117,  118,  119,  120,  121,  122,  680,
 /* 11550 */   680,  680,  691,  680,  680,  680,  189,  680,  680,  680,
 /* 11560 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11570 */   218,  680,  680,  680,  680,  680,  680,  680,  234,  680,
 /* 11580 */   109,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /* 11590 */   119,  120,  121,  122,  680,  680,  680,  746,  680,  680,
 /* 11600 */   680,  189,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11610 */   680,  680,  680,  680,  680,  218,  680,  680,  680,  680,
 /* 11620 */   680,  680,  680,  234,  680,  109,  110,  111,  112,  113,
 /* 11630 */   114,  115,  116,  117,  118,  119,  120,  121,  122,  680,
 /* 11640 */   680,  680,  778,  680,  680,  680,  189,  680,  680,  680,
 /* 11650 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11660 */   218,  680,  680,  680,  680,  680,  680,  680,  234,  680,
 /* 11670 */   109,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /* 11680 */   119,  120,  121,  122,  680,  680,  680,  881,  680,  680,
 /* 11690 */   680,  189,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11700 */   680,  680,  680,  680,  680,  218,  680,  680,  680,  680,
 /* 11710 */   680,  680,  680,  234,  680,  109,  110,  111,  112,  113,
 /* 11720 */   114,  115,  116,  117,  118,  119,  120,  121,  122,  680,
 /* 11730 */   680,  680,  895,  680,  680,  680,  189,  680,  680,  680,
 /* 11740 */   680,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11750 */   218,  680,  680,  680,  680,  680,  680,  680,  680,  680,
 /* 11760 */   109,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /* 11770 */   119,  120,  121,  122,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */     7,    0,    9,   10,  155,   44,    6,  158,  159,  160,
 /*    10 */   126,  162,  163,  171,   21,   41,   42,  168,  169,  177,
 /*    20 */   178,   28,   47,  174,   55,  183,   33,   34,   35,   36,
 /*    30 */    37,   38,   39,   61,   41,  151,   61,   44,   45,    1,
 /*    40 */     2,    3,    4,    5,  171,   45,   44,   54,   55,   56,
 /*    50 */   177,  178,   59,   60,   61,   39,  183,   88,   65,   90,
 /*    60 */    44,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*    70 */    77,   78,  177,  178,   45,   82,   83,   84,  183,  107,
 /*    80 */    87,  154,  107,  156,   91,   92,   93,   94,   44,   96,
 /*    90 */    52,   53,   99,   55,   56,   55,   58,  126,   54,  106,
 /*   100 */    62,   63,  164,  110,  111,  112,  113,  114,  115,  116,
 /*   110 */   117,  118,  119,    7,   45,    9,   10,   27,   28,   29,
 /*   120 */    30,   31,   32,   54,    6,  196,   57,   21,   88,  200,
 /*   130 */    90,   41,   42,    6,   28,  206,  207,  208,  209,   33,
 /*   140 */    34,   35,   36,   37,   38,   39,  199,   41,  201,  202,
 /*   150 */    44,   45,    1,    2,    3,    4,    5,  219,  220,  221,
 /*   160 */    54,   55,   56,   47,   61,   59,   60,   61,  177,  178,
 /*   170 */   179,   65,   44,  182,   68,   69,   70,   71,   72,   73,
 /*   180 */    74,   75,   76,   77,   78,   67,   44,   61,   82,   83,
 /*   190 */    84,   44,   89,   87,   67,    6,   54,   91,   92,   93,
 /*   200 */    94,   54,   96,   52,   53,   99,   55,   56,   40,   58,
 /*   210 */   107,    6,  106,   62,   63,   89,  110,  111,  112,  113,
 /*   220 */   114,  115,  116,  117,  118,  119,    7,   42,    9,   10,
 /*   230 */    45,  196,  173,  107,  175,  176,  177,  178,  179,   54,
 /*   240 */    21,  206,  207,  208,  209,   40,   44,   28,  213,  214,
 /*   250 */   108,  109,   33,   34,   35,   36,   37,   38,   39,  199,
 /*   260 */    41,  201,  202,   44,   45,    1,    2,    3,    4,    5,
 /*   270 */   163,   44,   61,   54,   55,   56,   22,  170,   59,   60,
 /*   280 */    61,  174,   55,   56,   65,   44,   97,   68,   69,   70,
 /*   290 */    71,   72,   73,   74,   75,   76,   77,   78,  163,   44,
 /*   300 */    59,   82,   83,   84,  169,   61,   87,  177,  178,  174,
 /*   310 */    91,   92,   93,   94,   59,   96,   52,   53,   99,   55,
 /*   320 */    56,  199,   58,  201,  202,  106,   62,   63,   61,  110,
 /*   330 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*   340 */   199,    9,   10,  202,  196,  173,   44,  175,  176,  177,
 /*   350 */   178,  179,  171,   21,  206,  207,  208,  209,  177,  178,
 /*   360 */    28,  213,  214,  164,  183,   33,   34,   35,   36,   37,
 /*   370 */    38,   39,   42,   41,  107,   45,   44,   45,    1,    2,
 /*   380 */     3,    4,    5,   61,   54,   39,   54,   55,   56,    6,
 /*   390 */    44,   59,   60,   61,   49,    6,   51,   65,    6,   54,
 /*   400 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*   410 */    78,   85,   86,   89,   82,   83,   84,   44,   89,   87,
 /*   420 */   221,   55,    6,   91,   92,   93,   94,   54,   96,   52,
 /*   430 */    53,   99,   55,   56,   45,   58,  107,   45,  106,   62,
 /*   440 */    63,   89,  110,  111,  112,  113,  114,  115,  116,  117,
 /*   450 */   118,  119,    7,    6,    9,   10,   40,  196,  173,    6,
 /*   460 */   175,  176,  177,  178,  179,  171,   21,  206,  207,  208,
 /*   470 */   209,  177,  178,   28,  213,  214,   89,  183,   33,   34,
 /*   480 */    35,   36,   37,   38,   39,   42,   41,   40,   45,   44,
 /*   490 */    45,    1,    2,    3,    4,    5,   61,   54,   45,   54,
 /*   500 */    55,   56,  188,  189,   59,   60,   61,   49,  199,   51,
 /*   510 */    65,  202,   54,   68,   69,   70,   71,   72,   73,   74,
 /*   520 */    75,   76,   77,   78,   85,   86,   44,   82,   83,   84,
 /*   530 */   190,  191,   87,   57,  192,  193,   91,   92,   93,   94,
 /*   540 */   164,   96,   52,   53,   99,   55,   56,   44,   58,  194,
 /*   550 */   195,  106,   62,   63,   44,  110,  111,  112,  113,  114,
 /*   560 */   115,  116,  117,  118,  119,    7,  199,    9,   10,  202,
 /*   570 */   196,  173,    6,  175,  176,  177,  178,  179,  171,   21,
 /*   580 */   206,  207,  208,  209,  177,  178,   28,  213,  214,   59,
 /*   590 */   183,   33,   34,   35,   36,   37,   38,   39,   42,   41,
 /*   600 */     6,   45,   44,   45,    1,    2,    3,    4,    5,  106,
 /*   610 */    54,   45,   54,   55,   56,   45,  106,   59,   60,   61,
 /*   620 */    49,    6,   51,   65,   54,   54,   68,   69,   70,   71,
 /*   630 */    72,   73,   74,   75,   76,   77,   78,  197,  198,   45,
 /*   640 */    82,   83,   84,  210,  211,   87,  210,  211,    6,   91,
 /*   650 */    92,   93,   94,   89,   96,   52,   53,   99,   55,   56,
 /*   660 */    45,   58,  210,  211,  106,   62,   63,  107,  110,  111,
 /*   670 */   112,  113,  114,  115,  116,  117,  118,  119,    7,    6,
 /*   680 */     9,   10,   40,  196,  173,    6,  175,  176,  177,  178,
 /*   690 */   179,  171,   21,  206,  207,  208,  209,  177,  178,   28,
 /*   700 */   213,  214,   45,  183,   33,   34,   35,   36,   37,   38,
 /*   710 */    39,   54,   41,   40,   57,   44,   45,    1,    2,    3,
 /*   720 */     4,    5,  210,  211,   45,   54,   55,   56,  210,  211,
 /*   730 */    59,   60,   61,  210,  211,   89,   65,   85,   86,   68,
 /*   740 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   750 */   210,  211,   57,   82,   83,   84,  210,  211,   87,  210,
 /*   760 */   211,   49,   91,   92,   93,   94,   54,   96,   52,   53,
 /*   770 */    99,   55,   56,  176,  177,  178,  179,  106,   62,   63,
 /*   780 */    89,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*   790 */   119,    7,   44,    9,   10,  100,  101,  102,  103,  104,
 /*   800 */   105,   44,    6,  196,   56,   21,   57,  200,   44,    6,
 /*   810 */    14,   54,   28,  206,  207,  208,  209,   33,   34,   35,
 /*   820 */    36,   37,   38,   39,    6,   41,    6,  164,   44,   45,
 /*   830 */     1,    2,    3,    4,    5,  163,  210,  211,   54,   55,
 /*   840 */    56,  210,  211,   59,   60,   61,  174,    6,   45,   65,
 /*   850 */    54,   57,   68,   69,   70,   71,   72,   73,   74,   75,
 /*   860 */    76,   77,   78,   45,  164,   45,   82,   83,   84,    6,
 /*   870 */   106,   87,    6,  188,  189,   91,   92,   93,   94,   45,
 /*   880 */    96,   52,   53,   99,   55,   56,   45,   44,   54,    6,
 /*   890 */   106,   62,   63,   57,  110,  111,  112,  113,  114,  115,
 /*   900 */   116,  117,  118,  119,    7,  164,    9,   10,   45,  196,
 /*   910 */   173,   45,  175,  176,  177,  178,  179,  171,   21,  206,
 /*   920 */   207,  208,  209,  177,  178,   28,  213,  214,   44,  183,
 /*   930 */    33,   34,   35,   36,   37,   38,   39,   54,   41,   85,
 /*   940 */    86,   44,   45,    1,    2,    3,    4,    5,   57,    6,
 /*   950 */     6,   54,   55,   56,  166,  167,   59,   60,   61,   22,
 /*   960 */    42,   42,   65,   45,   45,   68,   69,   70,   71,   72,
 /*   970 */    73,   74,   75,   76,   77,   78,  164,   44,   41,   82,
 /*   980 */    83,   84,   42,   40,   87,   45,   22,    6,   91,   92,
 /*   990 */    93,   94,   59,   96,   52,   53,   99,   55,   56,   55,
 /*  1000 */    44,  164,    6,  106,   62,   63,   57,  110,  111,  112,
 /*  1010 */   113,  114,  115,  116,  117,  118,  119,    7,    6,    9,
 /*  1020 */    10,   40,  196,  173,   57,  175,  176,  177,  178,  179,
 /*  1030 */   163,   21,  206,  207,  208,  209,   40,   44,   28,  213,
 /*  1040 */   214,  174,    6,   33,   34,   35,   36,   37,   38,   39,
 /*  1050 */    42,   41,   40,   45,   44,   45,    1,    2,    3,    4,
 /*  1060 */     5,   57,    6,   45,   54,   55,   56,   14,  164,   59,
 /*  1070 */    60,   61,   54,   44,   51,   65,   40,   54,   68,   69,
 /*  1080 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   45,
 /*  1090 */    45,  164,   82,   83,   84,   54,   40,   87,   54,   54,
 /*  1100 */     6,   91,   92,   93,   94,   45,   96,   52,   53,   99,
 /*  1110 */    55,   56,   45,   60,   54,    6,  106,   62,   63,   45,
 /*  1120 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  1130 */     7,    6,    9,   10,   40,  196,  173,   54,  175,  176,
 /*  1140 */   177,  178,  179,  163,   21,  206,  207,  208,  209,   40,
 /*  1150 */   189,   28,  213,  214,  174,    6,   33,   34,   35,   36,
 /*  1160 */    37,   38,   39,   51,   41,   40,   54,   44,   45,    1,
 /*  1170 */     2,    3,    4,    5,  154,    6,  156,   54,   55,   56,
 /*  1180 */    54,   54,   59,   60,   61,  154,  191,  156,   65,   40,
 /*  1190 */   164,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  1200 */    77,   78,   89,   92,  187,   82,   83,   84,   54,   40,
 /*  1210 */    87,   54,   44,  196,   91,   92,   93,   94,   54,   96,
 /*  1220 */    52,   53,   99,  206,  207,  208,  209,   45,   60,  106,
 /*  1230 */    62,   63,   54,  110,  111,  112,  113,  114,  115,  116,
 /*  1240 */   117,  118,  119,    7,    6,    9,   10,  154,  196,  156,
 /*  1250 */    95,   30,   31,   32,  154,  193,  156,   21,  206,  207,
 /*  1260 */   208,  209,   41,   42,   28,  213,  214,   44,   44,   33,
 /*  1270 */    34,   35,   36,   37,   38,   39,  198,   41,   40,  199,
 /*  1280 */    44,   45,    1,    2,    3,    4,    5,   67,  199,   55,
 /*  1290 */    54,   55,   56,   44,  199,   59,   60,   61,   55,   44,
 /*  1300 */   199,   65,  199,  199,   68,   69,   70,   71,   72,   73,
 /*  1310 */    74,   75,   76,   77,   78,  199,  199,  187,   82,   83,
 /*  1320 */    84,   44,   55,   87,   55,   44,  196,   91,   92,   93,
 /*  1330 */    94,  199,   96,   52,   53,   99,  206,  207,  208,  209,
 /*  1340 */   199,   60,  106,   62,   63,   45,  110,  111,  112,  113,
 /*  1350 */   114,  115,  116,  117,  118,  119,    7,   45,    9,   10,
 /*  1360 */    45,  196,   45,  211,   44,   57,  160,  164,  162,  163,
 /*  1370 */    21,  206,  207,  208,  209,  169,   44,   28,  213,  214,
 /*  1380 */   174,   97,   33,   34,   35,   36,   37,   38,   39,   44,
 /*  1390 */    41,   54,  195,   44,   45,    1,    2,    3,    4,    5,
 /*  1400 */    45,   92,   89,   54,   55,   56,   54,   54,   59,   60,
 /*  1410 */    61,   54,   61,   55,   65,   54,   54,   68,   69,   70,
 /*  1420 */    71,   72,   73,   74,   75,   76,   77,   78,   44,   55,
 /*  1430 */    61,   82,   83,   84,   44,   55,   87,   61,   55,   89,
 /*  1440 */    91,   92,   93,   94,   55,   96,   52,   53,   99,   55,
 /*  1450 */    56,   44,   61,   61,   44,  106,   62,   63,  152,  110,
 /*  1460 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  1470 */    44,    9,   10,   54,  152,   55,  157,  174,  159,  161,
 /*  1480 */   161,  163,  163,   21,   44,   61,  168,  168,  170,  170,
 /*  1490 */    28,   45,  174,  174,   14,   33,   34,   35,   36,   37,
 /*  1500 */    38,   39,   39,   41,   44,   67,   44,   45,    1,    2,
 /*  1510 */     3,    4,    5,   22,   45,   44,   54,   55,   56,   61,
 /*  1520 */    45,   59,   60,   61,   45,   57,   45,   65,  164,   57,
 /*  1530 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  1540 */    78,  164,   45,   57,   82,   83,   84,   45,   45,   87,
 /*  1550 */   164,   57,  164,   91,   92,   93,   94,   44,   96,   52,
 /*  1560 */    53,   99,   55,   56,  153,   44,  153,   44,  106,   62,
 /*  1570 */    63,  153,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  1580 */   118,  119,    7,   55,    9,   10,   44,   61,   44,  164,
 /*  1590 */    61,  159,  160,  165,  162,  163,   21,   45,  165,   45,
 /*  1600 */   168,  169,   45,   28,  167,   44,  174,   45,   33,   34,
 /*  1610 */    35,   36,   37,   38,   39,  165,   41,  164,   45,   44,
 /*  1620 */    45,    1,    2,    3,    4,    5,  165,   45,   50,   54,
 /*  1630 */    55,   56,  156,   44,   59,   60,   61,  153,  153,   44,
 /*  1640 */    65,  153,  153,   68,   69,   70,   71,   72,   73,   74,
 /*  1650 */    75,   76,   77,   78,  153,   50,   44,   82,   83,   84,
 /*  1660 */    44,  153,   87,   54,  153,  153,   91,   92,   93,   94,
 /*  1670 */    44,   96,   52,   53,   99,   55,   56,   54,   54,   61,
 /*  1680 */    61,  106,   62,   63,   44,  110,  111,  112,  113,  114,
 /*  1690 */   115,  116,  117,  118,  119,    7,   60,    9,   10,   54,
 /*  1700 */    54,   60,   39,  158,  222,  160,  222,  162,  163,   21,
 /*  1710 */   222,  222,  222,  168,  169,  222,   28,  222,  222,  174,
 /*  1720 */   222,   33,   34,   35,   36,   37,   38,   39,  222,   41,
 /*  1730 */   222,  222,   44,   45,    1,    2,    3,    4,    5,  222,
 /*  1740 */   222,  222,   54,   55,   56,  222,  222,   59,   60,   61,
 /*  1750 */   222,  222,  222,   65,  222,  222,   68,   69,   70,   71,
 /*  1760 */    72,   73,   74,   75,   76,   77,   78,  222,  222,  222,
 /*  1770 */    82,   83,   84,  222,  222,   87,  222,  222,  222,   91,
 /*  1780 */    92,   93,   94,  222,   96,   52,   53,   99,   55,   56,
 /*  1790 */   222,  222,  222,  222,  106,   62,   63,  222,  110,  111,
 /*  1800 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  222,
 /*  1810 */     9,   10,  222,  196,  222,  222,  222,  222,  222,  222,
 /*  1820 */   222,  222,   21,  206,  207,  208,  209,  222,  222,   28,
 /*  1830 */   222,  214,  222,  222,   33,   34,   35,   36,   37,   38,
 /*  1840 */    39,  222,   41,  222,  222,   44,   45,    1,    2,    3,
 /*  1850 */     4,    5,  222,  222,  222,   54,   55,   56,  222,  222,
 /*  1860 */    59,   60,   61,  222,  222,  222,   65,  222,  222,   68,
 /*  1870 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  1880 */   222,  222,  187,   82,   83,   84,  222,  222,   87,  164,
 /*  1890 */   222,  196,   91,   92,   93,   94,  222,   96,   52,   53,
 /*  1900 */    99,  206,  207,  208,  209,  222,   60,  106,   62,   63,
 /*  1910 */   222,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  1920 */   119,    7,  222,    9,   10,  222,  196,  222,  222,  222,
 /*  1930 */   222,  222,  222,  203,  222,   21,  206,  207,  208,  209,
 /*  1940 */   222,  222,   28,  218,  219,  220,  221,   33,   34,   35,
 /*  1950 */    36,   37,   38,   39,  222,   41,  222,  222,   44,   45,
 /*  1960 */     1,    2,    3,    4,    5,  222,  222,  222,   54,   55,
 /*  1970 */    56,  222,  222,   59,   60,   61,  222,  222,  222,   65,
 /*  1980 */   222,  222,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  1990 */    76,   77,   78,  222,  222,  187,   82,   83,   84,  222,
 /*  2000 */   222,   87,  222,  222,  196,   91,   92,   93,   94,  222,
 /*  2010 */    96,   52,   53,   99,  206,  207,  208,  209,  222,   60,
 /*  2020 */   106,   62,   63,  222,  110,  111,  112,  113,  114,  115,
 /*  2030 */   116,  117,  118,  119,    7,  222,    9,   10,  222,  196,
 /*  2040 */   222,  222,  222,  200,  222,  222,  196,  222,   21,  206,
 /*  2050 */   207,  208,  209,  222,  222,   28,  206,  207,  208,  209,
 /*  2060 */    33,   34,   35,   36,   37,   38,   39,  217,   41,  222,
 /*  2070 */   222,   44,   45,    1,    2,    3,    4,    5,  222,  222,
 /*  2080 */   222,   54,   55,   56,  222,  222,   59,   60,   61,  222,
 /*  2090 */   222,  222,   65,  222,  222,   68,   69,   70,   71,   72,
 /*  2100 */    73,   74,   75,   76,   77,   78,  222,  222,  222,   82,
 /*  2110 */    83,   84,  222,  222,   87,  222,  222,  222,   91,   92,
 /*  2120 */    93,   94,  222,   96,   52,   53,   99,  222,  222,  222,
 /*  2130 */    58,  222,  222,  106,   62,   63,  222,  110,  111,  112,
 /*  2140 */   113,  114,  115,  116,  117,  118,  119,    7,  222,    9,
 /*  2150 */    10,  222,  196,  222,  222,  222,  200,  222,  222,  196,
 /*  2160 */   222,   21,  206,  207,  208,  209,  222,  222,   28,  206,
 /*  2170 */   207,  208,  209,   33,   34,   35,   36,   37,   38,   39,
 /*  2180 */   222,   41,  222,  222,   44,   45,    1,    2,    3,    4,
 /*  2190 */     5,  222,  222,  222,   54,   55,   56,  222,  222,   59,
 /*  2200 */    60,   61,  222,  222,  222,   65,  222,  222,   68,   69,
 /*  2210 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  222,
 /*  2220 */   222,  187,   82,   83,   84,  222,  222,   87,  222,  222,
 /*  2230 */   196,   91,   92,   93,   94,  222,   96,   52,   53,   99,
 /*  2240 */   206,  207,  208,  209,  222,   60,  106,   62,   63,  222,
 /*  2250 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  2260 */     7,  222,    9,   10,  222,  196,  222,  222,  222,  200,
 /*  2270 */   222,  222,  196,  222,   21,  206,  207,  208,  209,  222,
 /*  2280 */   222,   28,  206,  207,  208,  209,   33,   34,   35,   36,
 /*  2290 */    37,   38,   39,  222,   41,  222,  222,   44,   45,    1,
 /*  2300 */     2,    3,    4,    5,  222,  222,  222,   54,   55,   56,
 /*  2310 */   222,  222,   59,   60,   61,  222,  222,  222,   65,  222,
 /*  2320 */   222,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  2330 */    77,   78,  222,  222,  187,   82,   83,   84,  222,  222,
 /*  2340 */    87,  222,  222,  196,   91,   92,   93,   94,  222,   96,
 /*  2350 */    52,   53,   99,  206,  207,  208,  209,  222,   60,  106,
 /*  2360 */    62,   63,  222,  110,  111,  112,  113,  114,  115,  116,
 /*  2370 */   117,  118,  119,    7,  222,    9,   10,  222,  196,  222,
 /*  2380 */   222,  222,  200,  222,  222,  196,  222,   21,  206,  207,
 /*  2390 */   208,  209,  222,  222,   28,  206,  207,  208,  209,   33,
 /*  2400 */    34,   35,   36,   37,   38,   39,  222,   41,  222,  222,
 /*  2410 */    44,   45,    1,    2,    3,    4,    5,  222,  222,  222,
 /*  2420 */    54,   55,   56,  222,  222,   59,   60,   61,  222,  222,
 /*  2430 */   222,   65,  222,  222,   68,   69,   70,   71,   72,   73,
 /*  2440 */    74,   75,   76,   77,   78,  222,  222,  222,   82,   83,
 /*  2450 */    84,  222,  222,   87,  222,  222,  196,   91,   92,   93,
 /*  2460 */    94,  222,   96,   52,   53,   99,  206,  207,  208,  209,
 /*  2470 */   222,  222,  106,   62,   63,  222,  110,  111,  112,  113,
 /*  2480 */   114,  115,  116,  117,  118,  119,    7,  222,    9,   10,
 /*  2490 */   222,  196,  222,  222,  222,  200,  222,  222,  196,  222,
 /*  2500 */    21,  206,  207,  208,  209,  222,  222,   28,  206,  207,
 /*  2510 */   208,  209,   33,   34,   35,   36,   37,   38,   39,  196,
 /*  2520 */    41,  222,  222,   44,   45,  222,  222,  204,  205,  206,
 /*  2530 */   207,  208,  209,   54,   55,   56,  222,  222,   59,   60,
 /*  2540 */    61,  222,  222,  222,   65,  222,  222,   68,   69,   70,
 /*  2550 */    71,   72,   73,   74,   75,   76,   77,   78,  222,  222,
 /*  2560 */   222,   82,   83,   84,  222,  222,   87,  222,  222,  196,
 /*  2570 */    91,   92,   93,   94,  222,   96,  222,  222,   99,  206,
 /*  2580 */   207,  208,  209,  222,  222,  106,  222,  222,  222,  110,
 /*  2590 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  2600 */   222,    9,   10,  222,  196,  222,  222,  222,  200,  222,
 /*  2610 */   222,  196,  222,   21,  206,  207,  208,  209,  222,  222,
 /*  2620 */    28,  206,  207,  208,  209,   33,   34,   35,   36,   37,
 /*  2630 */    38,   39,  196,   41,  222,  222,   44,   45,  222,  222,
 /*  2640 */   222,  205,  206,  207,  208,  209,   54,   55,   56,  222,
 /*  2650 */   222,   59,   60,   61,  222,  222,  222,   65,  222,  222,
 /*  2660 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  2670 */    78,  222,  222,  222,   82,   83,   84,  222,  222,   87,
 /*  2680 */   222,  222,  196,   91,   92,   93,   94,  222,   96,  222,
 /*  2690 */   222,   99,  206,  207,  208,  209,  222,  222,  106,  222,
 /*  2700 */   222,  222,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  2710 */   118,  119,    7,  222,    9,   10,  222,  196,  222,  222,
 /*  2720 */   222,  200,  222,  222,  196,  222,   21,  206,  207,  208,
 /*  2730 */   209,  222,  222,   28,  206,  207,  208,  209,   33,   34,
 /*  2740 */    35,   36,   37,   38,   39,  196,   41,  222,  222,   44,
 /*  2750 */    45,  222,  222,  222,  222,  206,  207,  208,  209,   54,
 /*  2760 */    55,   56,  222,  222,   59,   60,   61,  222,  222,  222,
 /*  2770 */    65,  222,  222,   68,   69,   70,   71,   72,   73,   74,
 /*  2780 */    75,   76,   77,   78,  222,  222,  222,   82,   83,   84,
 /*  2790 */   222,  222,   87,  222,  222,  196,   91,   92,   93,   94,
 /*  2800 */   222,   96,  222,  222,   99,  206,  207,  208,  209,  222,
 /*  2810 */   222,  106,  222,  222,  222,  110,  111,  112,  113,  114,
 /*  2820 */   115,  116,  117,  118,  119,    7,  222,    9,   10,  222,
 /*  2830 */   196,  222,  222,  222,  200,  222,  222,  222,  222,   21,
 /*  2840 */   206,  207,  208,  209,  222,  222,   28,  222,  222,  222,
 /*  2850 */   222,   33,   34,   35,   36,   37,   38,   39,  196,   41,
 /*  2860 */   222,  222,   44,   45,  222,  222,  222,  222,  206,  207,
 /*  2870 */   208,  209,   54,   55,   56,  222,  222,   59,   60,   61,
 /*  2880 */   222,  222,  222,   65,  222,  222,   68,   69,   70,   71,
 /*  2890 */    72,   73,   74,   75,   76,   77,   78,  222,  222,  222,
 /*  2900 */    82,   83,   84,  222,  222,   87,  222,  222,  196,   91,
 /*  2910 */    92,   93,   94,  222,   96,  222,  222,   99,  206,  207,
 /*  2920 */   208,  209,  222,  222,  106,  222,  222,  222,  110,  111,
 /*  2930 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  222,
 /*  2940 */     9,   10,  222,  196,  222,  222,  222,  200,  222,  222,
 /*  2950 */   222,  222,   21,  206,  207,  208,  209,  222,  222,   28,
 /*  2960 */   222,  222,  222,  222,   33,   34,   35,   36,   37,   38,
 /*  2970 */    39,  196,   41,  222,  222,   44,   45,  222,  222,  222,
 /*  2980 */   222,  206,  207,  208,  209,   54,   55,   56,  222,  222,
 /*  2990 */    59,   60,   61,  222,  222,  222,   65,  222,  222,   68,
 /*  3000 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  3010 */   222,  222,  222,   82,   83,   84,  222,  222,   87,  222,
 /*  3020 */   222,  196,   91,   92,   93,   94,  222,   96,  222,  222,
 /*  3030 */    99,  206,  207,  208,  209,  222,  222,  106,  222,  222,
 /*  3040 */   222,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  3050 */   119,    7,  222,    9,   10,  222,  196,  222,  222,  222,
 /*  3060 */   200,  222,  222,  222,  222,   21,  206,  207,  208,  209,
 /*  3070 */   222,  222,   28,  222,  222,  222,  222,   33,   34,   35,
 /*  3080 */    36,   37,   38,   39,  196,   41,  222,  222,   44,   45,
 /*  3090 */   222,  222,  222,  222,  206,  207,  208,  209,   54,   55,
 /*  3100 */    56,  222,  222,   59,   60,   61,  222,  222,  222,   65,
 /*  3110 */   222,  222,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  3120 */    76,   77,   78,  222,  222,  222,   82,   83,   84,  222,
 /*  3130 */   222,   87,  222,  222,  196,   91,   92,   93,   94,  222,
 /*  3140 */    96,  222,  222,   99,  206,  207,  208,  209,  222,  222,
 /*  3150 */   106,  222,  222,  222,  110,  111,  112,  113,  114,  115,
 /*  3160 */   116,  117,  118,  119,    7,  222,    9,   10,  222,  196,
 /*  3170 */   222,  222,  222,  200,  222,  222,  222,  222,   21,  206,
 /*  3180 */   207,  208,  209,  222,  222,   28,  222,  222,  222,  222,
 /*  3190 */    33,   34,   35,   36,   37,   38,   39,  196,   41,  222,
 /*  3200 */   222,   44,   45,  222,  222,  222,  222,  206,  207,  208,
 /*  3210 */   209,   54,   55,   56,  222,  222,   59,   60,   61,  222,
 /*  3220 */   222,  222,   65,  222,  222,   68,   69,   70,   71,   72,
 /*  3230 */    73,   74,   75,   76,   77,   78,  222,  222,  222,   82,
 /*  3240 */    83,   84,  222,  222,   87,  222,  222,  196,   91,   92,
 /*  3250 */    93,   94,  222,   96,  222,  222,   99,  206,  207,  208,
 /*  3260 */   209,  222,  222,  106,  222,  222,  222,  110,  111,  112,
 /*  3270 */   113,  114,  115,  116,  117,  118,  119,    7,  222,    9,
 /*  3280 */    10,  222,  196,  222,  222,  222,  200,  222,  222,  222,
 /*  3290 */   222,   21,  206,  207,  208,  209,  222,  222,   28,  222,
 /*  3300 */   222,  222,  222,   33,   34,   35,   36,   37,   38,   39,
 /*  3310 */   196,   41,  222,  222,   44,   45,  222,  222,  222,  222,
 /*  3320 */   206,  207,  208,  209,   54,   55,   56,  222,  222,   59,
 /*  3330 */    60,   61,  222,  222,  222,   65,  222,  222,   68,   69,
 /*  3340 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  222,
 /*  3350 */   222,  222,   82,   83,   84,  222,  222,   87,  222,  222,
 /*  3360 */   196,   91,   92,   93,   94,  222,   96,  222,  222,   99,
 /*  3370 */   206,  207,  208,  209,  222,  222,  106,  222,  222,  222,
 /*  3380 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  3390 */     7,  222,    9,   10,  222,  196,  222,  222,  222,  200,
 /*  3400 */   222,  222,  222,  222,   21,  206,  207,  208,  209,  222,
 /*  3410 */   222,   28,  222,  222,  222,  222,   33,   34,   35,   36,
 /*  3420 */    37,   38,   39,  196,   41,  222,  222,   44,   45,  222,
 /*  3430 */   222,  222,  222,  206,  207,  208,  209,   54,   55,   56,
 /*  3440 */   222,  222,   59,   60,   61,  222,  222,  222,   65,  222,
 /*  3450 */   222,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  3460 */    77,   78,  222,  222,  222,   82,   83,   84,  222,  222,
 /*  3470 */    87,  222,  222,  196,   91,   92,   93,   94,  222,   96,
 /*  3480 */   222,  222,   99,  206,  207,  208,  209,  222,  222,  106,
 /*  3490 */   222,  222,  222,  110,  111,  112,  113,  114,  115,  116,
 /*  3500 */   117,  118,  119,    7,  222,    9,   10,  222,  196,  222,
 /*  3510 */   222,  222,  200,  222,  222,  222,  222,   21,  206,  207,
 /*  3520 */   208,  209,  222,  222,   28,  222,  222,  222,  222,   33,
 /*  3530 */    34,   35,   36,   37,   38,   39,  196,   41,  222,  222,
 /*  3540 */    44,   45,  222,  222,  222,  222,  206,  207,  208,  209,
 /*  3550 */    54,   55,   56,  222,  222,   59,   60,   61,  222,  222,
 /*  3560 */   222,   65,  222,  222,   68,   69,   70,   71,   72,   73,
 /*  3570 */    74,   75,   76,   77,   78,  222,  222,  222,   82,   83,
 /*  3580 */    84,  222,  222,   87,  222,  222,  196,   91,   92,   93,
 /*  3590 */    94,  222,   96,  222,  222,   99,  206,  207,  208,  209,
 /*  3600 */   222,  222,  106,  222,  222,  222,  110,  111,  112,  113,
 /*  3610 */   114,  115,  116,  117,  118,  119,    7,  222,    9,   10,
 /*  3620 */   222,  196,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  3630 */    21,  206,  207,  208,  209,  222,  222,   28,  222,  222,
 /*  3640 */   222,  222,   33,   34,   35,   36,   37,   38,   39,  196,
 /*  3650 */    41,  222,  222,   44,   45,  222,  222,  222,  222,  206,
 /*  3660 */   207,  208,  209,   54,   55,   56,  222,  222,   59,   60,
 /*  3670 */    61,  222,  222,  222,   65,  222,  222,   68,   69,   70,
 /*  3680 */    71,   72,   73,   74,   75,   76,   77,   78,  222,  222,
 /*  3690 */   222,   82,   83,   84,  222,  222,   87,  222,  222,  196,
 /*  3700 */    91,   92,   93,   94,  222,   96,  222,  222,   99,  206,
 /*  3710 */   207,  208,  209,  222,  222,  106,  222,  222,  222,  110,
 /*  3720 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  3730 */   222,    9,   10,  222,  196,  222,  222,  222,  222,  222,
 /*  3740 */   222,  222,  222,   21,  206,  207,  208,  209,  222,  222,
 /*  3750 */    28,  222,  222,  222,  222,   33,   34,   35,   36,   37,
 /*  3760 */    38,   39,  196,   41,  222,  222,   44,   45,  222,  222,
 /*  3770 */   222,  222,  206,  207,  208,  209,   54,   55,   56,  222,
 /*  3780 */   222,   59,   60,   61,  222,  222,  222,   65,  222,  222,
 /*  3790 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  3800 */    78,  222,  222,  222,   82,   83,   84,  222,  222,   87,
 /*  3810 */   222,  222,  196,   91,   92,   93,   94,  222,   96,  222,
 /*  3820 */   222,   99,  206,  207,  208,  209,  222,  222,  106,  222,
 /*  3830 */   222,  222,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  3840 */   118,  119,    7,  222,    9,   10,  222,  196,  222,  222,
 /*  3850 */   222,  222,  222,  222,  222,  222,   21,  206,  207,  208,
 /*  3860 */   209,  222,  222,   28,  222,  222,  222,  222,   33,   34,
 /*  3870 */    35,   36,   37,   38,   39,  196,   41,  222,  222,   44,
 /*  3880 */    45,  222,  222,  222,  222,  206,  207,  208,  209,   54,
 /*  3890 */    55,   56,  222,  222,   59,   60,   61,  222,  222,  222,
 /*  3900 */    65,  222,  222,   68,   69,   70,   71,   72,   73,   74,
 /*  3910 */    75,   76,   77,   78,  222,  222,  222,   82,   83,   84,
 /*  3920 */   222,  222,   87,  222,  222,  196,   91,   92,   93,   94,
 /*  3930 */   222,   96,  222,  222,   99,  206,  207,  208,  209,  222,
 /*  3940 */   222,  106,  222,  222,  222,  110,  111,  112,  113,  114,
 /*  3950 */   115,  116,  117,  118,  119,    7,  222,    9,   10,  222,
 /*  3960 */   196,  222,  222,  222,  222,  222,  222,  222,  222,   21,
 /*  3970 */   206,  207,  208,  209,  222,  222,   28,  222,  222,  222,
 /*  3980 */   222,   33,   34,   35,   36,   37,   38,   39,  196,   41,
 /*  3990 */   222,  222,   44,   45,  222,  222,  222,  222,  206,  207,
 /*  4000 */   208,  209,   54,   55,   56,  222,  222,   59,   60,   61,
 /*  4010 */   222,  222,  222,   65,  222,  222,   68,   69,   70,   71,
 /*  4020 */    72,   73,   74,   75,   76,   77,   78,  222,  222,  222,
 /*  4030 */    82,   83,   84,  222,  222,   87,  222,  222,  196,   91,
 /*  4040 */    92,   93,   94,  222,   96,  222,  222,   99,  206,  207,
 /*  4050 */   208,  209,  222,  222,  106,  222,  222,  222,  110,  111,
 /*  4060 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  222,
 /*  4070 */     9,   10,  222,  196,  222,  222,  222,  222,  222,  222,
 /*  4080 */   222,  222,   21,  206,  207,  208,  209,  222,  222,   28,
 /*  4090 */   222,  222,  222,  222,   33,   34,   35,   36,   37,   38,
 /*  4100 */    39,  196,   41,  222,  222,   44,   45,  222,  222,  222,
 /*  4110 */   222,  206,  207,  208,  209,   54,   55,   56,  222,  222,
 /*  4120 */    59,   60,   61,  222,  222,  222,   65,  222,  222,   68,
 /*  4130 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  4140 */   222,  222,  222,   82,   83,   84,  222,  222,   87,  222,
 /*  4150 */   222,  196,   91,   92,   93,   94,  222,   96,  222,  222,
 /*  4160 */    99,  206,  207,  208,  209,  222,  222,  106,  222,  222,
 /*  4170 */   222,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  4180 */   119,    7,  222,    9,   10,  222,  196,  222,  222,  222,
 /*  4190 */   222,  222,  222,  222,  222,   21,  206,  207,  208,  209,
 /*  4200 */   222,  222,   28,  222,  222,  222,  222,   33,   34,   35,
 /*  4210 */    36,   37,   38,   39,  196,   41,  222,  222,   44,   45,
 /*  4220 */   222,  222,  222,  222,  206,  207,  208,  209,   54,   55,
 /*  4230 */    56,  222,  222,   59,   60,   61,  222,  222,  222,   65,
 /*  4240 */   222,  222,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  4250 */    76,   77,   78,  222,  222,  222,   82,   83,   84,  222,
 /*  4260 */   222,   87,  222,  222,  196,   91,   92,   93,   94,  222,
 /*  4270 */    96,  222,  222,   99,  206,  207,  208,  209,  222,  222,
 /*  4280 */   106,  222,  222,  222,  110,  111,  112,  113,  114,  115,
 /*  4290 */   116,  117,  118,  119,    7,  222,    9,   10,  222,  196,
 /*  4300 */   222,  222,  222,  222,  222,  222,  222,  222,   21,  206,
 /*  4310 */   207,  208,  209,  222,  222,   28,  222,  222,  222,  222,
 /*  4320 */    33,   34,   35,   36,   37,   38,   39,  196,   41,  222,
 /*  4330 */   222,   44,   45,  222,  222,  222,  222,  206,  207,  208,
 /*  4340 */   209,   54,   55,   56,  222,  222,   59,   60,   61,  222,
 /*  4350 */   222,  222,   65,  222,  222,   68,   69,   70,   71,   72,
 /*  4360 */    73,   74,   75,   76,   77,   78,  222,  222,  222,   82,
 /*  4370 */    83,   84,  222,  222,   87,  222,  222,  196,   91,   92,
 /*  4380 */    93,   94,  222,   96,  222,  222,   99,  206,  207,  208,
 /*  4390 */   209,  222,  222,  106,  222,  222,  222,  110,  111,  112,
 /*  4400 */   113,  114,  115,  116,  117,  118,  119,    7,  222,    9,
 /*  4410 */    10,  222,  196,  222,  222,  222,  222,  222,  222,  222,
 /*  4420 */   222,   21,  206,  207,  208,  209,  222,  222,   28,  222,
 /*  4430 */   222,  222,  222,   33,   34,   35,   36,   37,   38,   39,
 /*  4440 */   196,   41,  222,  222,   44,   45,  222,  222,  222,  222,
 /*  4450 */   206,  207,  208,  209,   54,   55,   56,  222,  222,   59,
 /*  4460 */    60,   61,  222,  222,  222,   65,  222,  222,   68,   69,
 /*  4470 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  222,
 /*  4480 */   222,  222,   82,   83,   84,  222,  222,   87,  222,  222,
 /*  4490 */   196,   91,   92,   93,   94,  222,   96,  222,  222,   99,
 /*  4500 */   206,  207,  208,  209,  222,  222,  106,  222,  222,  222,
 /*  4510 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  4520 */     7,  160,    9,   10,  163,  222,  222,  222,  222,  168,
 /*  4530 */   169,  222,  222,  222,   21,  174,  222,  222,  222,  222,
 /*  4540 */   222,   28,  222,  222,  222,  222,   33,   34,   35,   36,
 /*  4550 */    37,   38,   39,  222,   41,  222,  222,   44,   45,  222,
 /*  4560 */   222,  222,  222,  222,  222,  222,  222,   54,   55,   56,
 /*  4570 */   222,  222,   59,   60,   61,  222,  222,  222,   65,  222,
 /*  4580 */   222,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  4590 */    77,   78,  222,  222,  222,   82,   83,   84,  222,  222,
 /*  4600 */    87,  222,  222,  222,   91,   92,   93,   94,  222,   96,
 /*  4610 */   222,  222,   99,  222,  222,  222,  222,  222,  222,  106,
 /*  4620 */   222,  222,  222,  110,  111,  112,  113,  114,  115,  116,
 /*  4630 */   117,  118,  119,    7,  222,    9,   10,  222,  222,  222,
 /*  4640 */   222,  222,  222,  222,  222,  222,  222,   21,  222,  222,
 /*  4650 */   222,  222,  222,  222,   28,  222,  222,  222,  222,   33,
 /*  4660 */    34,   35,   36,   37,   38,   39,  222,   41,  222,  222,
 /*  4670 */    44,   45,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  4680 */    54,   55,   56,  222,  222,   59,   60,   61,  222,  222,
 /*  4690 */   222,   65,  222,  222,   68,   69,   70,   71,   72,   73,
 /*  4700 */    74,   75,   76,   77,   78,  222,  222,  222,   82,   83,
 /*  4710 */    84,  222,  222,   87,  222,  222,  222,   91,   92,   93,
 /*  4720 */    94,  222,   96,  222,  222,   99,  222,  222,  222,  222,
 /*  4730 */   222,  222,  106,  222,  222,  222,  110,  111,  112,  113,
 /*  4740 */   114,  115,  116,  117,  118,  119,    7,  222,    9,   10,
 /*  4750 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  4760 */    21,  222,  222,  222,  222,  222,  222,   28,  222,  222,
 /*  4770 */   222,  222,   33,   34,   35,   36,   37,   38,   39,  222,
 /*  4780 */    41,  222,  222,   44,   45,  222,  222,  222,  222,  222,
 /*  4790 */   222,  222,  222,   54,   55,   56,  222,  222,   59,   60,
 /*  4800 */    61,  222,  222,  222,   65,  222,  222,   68,   69,   70,
 /*  4810 */    71,   72,   73,   74,   75,   76,   77,   78,  222,  222,
 /*  4820 */   222,   82,   83,   84,  222,  222,   87,  222,  222,  222,
 /*  4830 */    91,   92,   93,   94,  222,   96,  222,  222,   99,  222,
 /*  4840 */   222,  222,  222,  222,  222,  106,  222,  222,  222,  110,
 /*  4850 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  4860 */   222,    9,   10,  222,  222,  222,  222,  222,  222,  222,
 /*  4870 */   222,  222,  222,   21,  222,  222,  222,  222,  222,  222,
 /*  4880 */    28,  222,  222,  222,  222,   33,   34,   35,   36,   37,
 /*  4890 */    38,   39,  222,   41,  222,  222,   44,   45,  222,  222,
 /*  4900 */   222,  222,  222,  222,  222,  222,   54,   55,   56,  222,
 /*  4910 */   222,   59,   60,   61,  222,  222,  222,   65,  222,  222,
 /*  4920 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  4930 */    78,  222,  222,  222,   82,   83,   84,  222,  222,   87,
 /*  4940 */   222,  222,  222,   91,   92,   93,   94,  222,   96,  222,
 /*  4950 */   222,   99,  222,  222,  222,  222,  222,  222,  106,  222,
 /*  4960 */   222,  222,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  4970 */   118,  119,    7,  222,    9,   10,  222,  222,  222,  222,
 /*  4980 */   222,  222,  222,  222,  222,  222,   21,  222,  222,  222,
 /*  4990 */   222,  222,  222,   28,  222,  222,  222,  222,   33,   34,
 /*  5000 */    35,   36,   37,   38,   39,  222,   41,  222,  222,   44,
 /*  5010 */    45,  222,  222,  222,  222,  222,  222,  222,  222,   54,
 /*  5020 */    55,   56,  222,  222,   59,   60,   61,  222,  222,  222,
 /*  5030 */    65,  222,  222,   68,   69,   70,   71,   72,   73,   74,
 /*  5040 */    75,   76,   77,   78,  222,  222,  222,   82,   83,   84,
 /*  5050 */   222,  222,   87,  222,  222,  222,   91,   92,   93,   94,
 /*  5060 */   222,   96,  222,  222,   99,  222,  222,  222,  222,  222,
 /*  5070 */   222,  106,  222,  222,  222,  110,  111,  112,  113,  114,
 /*  5080 */   115,  116,  117,  118,  119,    7,  222,    9,   10,  222,
 /*  5090 */   222,  222,  222,  222,  222,  222,  222,  222,  222,   21,
 /*  5100 */   222,  222,  222,  222,  222,  222,   28,  222,  222,  222,
 /*  5110 */   222,   33,   34,   35,   36,   37,   38,   39,  222,   41,
 /*  5120 */   222,  222,   44,   45,  222,  222,  222,  222,  222,  222,
 /*  5130 */   222,  222,   54,   55,   56,  222,  222,   59,   60,   61,
 /*  5140 */   222,  222,  222,   65,  222,  222,   68,   69,   70,   71,
 /*  5150 */    72,   73,   74,   75,   76,   77,   78,  222,  222,  222,
 /*  5160 */    82,   83,   84,  222,  222,   87,  222,  222,  222,   91,
 /*  5170 */    92,   93,   94,  222,   96,  222,  222,   99,  222,  222,
 /*  5180 */   222,  222,  222,  222,  106,  222,  222,  222,  110,  111,
 /*  5190 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  222,
 /*  5200 */     9,   10,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  5210 */   222,  222,   21,  222,  222,  222,  222,  222,  222,   28,
 /*  5220 */   222,  222,  222,  222,   33,   34,   35,   36,   37,   38,
 /*  5230 */    39,  222,   41,  222,  222,   44,   45,  222,  222,  222,
 /*  5240 */   222,  222,  222,  222,  222,   54,   55,   56,  222,  222,
 /*  5250 */    59,   60,   61,  222,  222,  222,   65,  222,  222,   68,
 /*  5260 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  5270 */   222,  222,  222,   82,   83,   84,  222,  222,   87,  222,
 /*  5280 */   222,  222,   91,   92,   93,   94,  222,   96,  222,  222,
 /*  5290 */    99,  222,  222,  222,  222,  222,  222,  106,  222,  222,
 /*  5300 */   222,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  5310 */   119,    7,  222,    9,   10,  222,  222,  222,  222,  222,
 /*  5320 */   222,  222,  222,  222,  222,   21,  222,  222,  222,  222,
 /*  5330 */   222,  222,   28,  222,  222,  222,  222,   33,   34,   35,
 /*  5340 */    36,   37,   38,   39,  222,   41,  222,  222,   44,   45,
 /*  5350 */   222,  222,  222,  222,  222,  222,  222,  222,   54,   55,
 /*  5360 */    56,  222,  222,   59,   60,   61,  222,  222,  222,   65,
 /*  5370 */   222,  222,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  5380 */    76,   77,   78,  222,  222,  222,   82,   83,   84,  222,
 /*  5390 */   222,   87,  222,  222,  222,   91,   92,   93,   94,  222,
 /*  5400 */    96,  222,  222,   99,  222,  222,  222,  222,  222,  222,
 /*  5410 */   106,  222,  222,  222,  110,  111,  112,  113,  114,  115,
 /*  5420 */   116,  117,  118,  119,    7,  222,    9,   10,  222,  222,
 /*  5430 */   222,  222,  222,  222,  222,  222,  222,  222,   21,  222,
 /*  5440 */   222,  222,  222,  222,  222,   28,  222,  222,  222,  222,
 /*  5450 */    33,   34,   35,   36,   37,   38,   39,  222,   41,  222,
 /*  5460 */   222,   44,   45,  222,  222,  222,  222,  222,  222,  222,
 /*  5470 */   222,   54,   55,   56,  222,  222,   59,   60,   61,  222,
 /*  5480 */   222,  222,   65,  222,  222,   68,   69,   70,   71,   72,
 /*  5490 */    73,   74,   75,   76,   77,   78,  222,  222,  222,   82,
 /*  5500 */    83,   84,  222,  222,   87,  222,  222,  222,   91,   92,
 /*  5510 */    93,   94,  222,   96,  222,  222,   99,  222,  222,  222,
 /*  5520 */   222,  222,  222,  106,  222,  222,  222,  110,  111,  112,
 /*  5530 */   113,  114,  115,  116,  117,  118,  119,    7,  222,    9,
 /*  5540 */    10,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  5550 */   222,   21,  222,  222,  222,  222,  222,  222,   28,  222,
 /*  5560 */   222,  222,  222,   33,   34,   35,   36,   37,   38,   39,
 /*  5570 */   222,   41,  222,  222,   44,   45,  222,  222,  222,  222,
 /*  5580 */   222,  222,  222,  222,   54,   55,   56,  222,  222,   59,
 /*  5590 */    60,   61,  222,  222,  222,   65,  222,  222,   68,   69,
 /*  5600 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  222,
 /*  5610 */   222,  222,   82,   83,   84,  222,  222,   87,  222,  222,
 /*  5620 */   222,   91,   92,   93,   94,  222,   96,  222,  222,   99,
 /*  5630 */   222,  222,  222,  222,  222,  222,  106,  222,  222,  222,
 /*  5640 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  5650 */     7,  222,    9,   10,  222,  222,  222,  222,  222,  222,
 /*  5660 */   222,  222,  222,  222,   21,  222,  222,  222,  222,  222,
 /*  5670 */   222,   28,  222,  222,  222,  222,   33,   34,   35,   36,
 /*  5680 */    37,   38,   39,  222,   41,  222,  222,   44,   45,  222,
 /*  5690 */   222,  222,  222,  222,  222,  222,  222,   54,   55,   56,
 /*  5700 */   222,  222,   59,   60,   61,  222,  222,  222,   65,  222,
 /*  5710 */   222,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  5720 */    77,   78,  222,  222,  222,   82,   83,   84,  222,  222,
 /*  5730 */    87,  222,  222,  222,   91,   92,   93,   94,  222,   96,
 /*  5740 */   222,  222,   99,  222,  222,  222,  222,  222,  222,  106,
 /*  5750 */   222,  222,  222,  110,  111,  112,  113,  114,  115,  116,
 /*  5760 */   117,  118,  119,    7,  222,    9,   10,  222,  222,  222,
 /*  5770 */   222,  222,  222,  222,  222,  222,  222,   21,  222,  222,
 /*  5780 */   222,  222,  222,  222,   28,  222,  222,  222,  222,   33,
 /*  5790 */    34,   35,   36,   37,   38,   39,  222,   41,  222,  222,
 /*  5800 */    44,   45,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  5810 */    54,   55,   56,  222,  222,   59,   60,   61,  222,  222,
 /*  5820 */   222,   65,  222,  222,   68,   69,   70,   71,   72,   73,
 /*  5830 */    74,   75,   76,   77,   78,  222,  222,  222,   82,   83,
 /*  5840 */    84,  222,  222,   87,  222,  222,  222,   91,   92,   93,
 /*  5850 */    94,  222,   96,  222,  222,   99,  222,  222,  222,  222,
 /*  5860 */   222,  222,  106,  222,  222,  222,  110,  111,  112,  113,
 /*  5870 */   114,  115,  116,  117,  118,  119,    7,  222,    9,   10,
 /*  5880 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  5890 */    21,  222,  222,  222,  222,  222,  222,   28,  222,  222,
 /*  5900 */   222,  222,   33,   34,   35,   36,   37,   38,   39,  222,
 /*  5910 */    41,  222,  222,   44,   45,  222,  222,  222,  222,  222,
 /*  5920 */   222,  222,  222,   54,   55,   56,  222,  222,   59,   60,
 /*  5930 */    61,  222,  222,  222,   65,  222,  222,   68,   69,   70,
 /*  5940 */    71,   72,   73,   74,   75,   76,   77,   78,  222,  222,
 /*  5950 */   222,   82,   83,   84,  222,  222,   87,  222,  222,  222,
 /*  5960 */    91,   92,   93,   94,  222,   96,  222,  222,   99,  222,
 /*  5970 */   222,  222,  222,  222,  222,  106,  222,  222,  222,  110,
 /*  5980 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  5990 */   222,    9,   10,  222,  222,  222,  222,  222,  222,  222,
 /*  6000 */   222,  222,  222,   21,  222,  222,  222,  222,  222,  222,
 /*  6010 */    28,  222,  222,  222,  222,   33,   34,   35,   36,   37,
 /*  6020 */    38,   39,  222,   41,  222,  222,   44,   45,  222,  222,
 /*  6030 */   222,  222,  222,  222,  222,  222,   54,   55,   56,  222,
 /*  6040 */   222,   59,   60,   61,  222,  222,  222,   65,  222,  222,
 /*  6050 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  6060 */    78,  222,  222,  222,   82,   83,   84,  222,  222,   87,
 /*  6070 */   222,  222,  222,   91,   92,   93,   94,  222,   96,  222,
 /*  6080 */   222,   99,  222,  222,  222,  222,  222,  222,  106,  222,
 /*  6090 */   222,  222,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  6100 */   118,  119,    7,  222,    9,   10,  222,  222,  222,  222,
 /*  6110 */   222,  222,  222,  222,  222,  222,   21,  222,  222,  222,
 /*  6120 */   222,  222,  222,   28,  222,  222,  222,  222,   33,   34,
 /*  6130 */    35,   36,   37,   38,   39,  222,   41,  222,  222,   44,
 /*  6140 */    45,  222,  222,  222,  222,  222,  222,  222,  222,   54,
 /*  6150 */   222,   56,  222,  222,   59,   60,   61,  222,  222,  222,
 /*  6160 */    65,  222,  222,   68,   69,   70,   71,   72,   73,   74,
 /*  6170 */    75,   76,   77,   78,  222,  222,  222,   82,   83,   84,
 /*  6180 */   222,  222,   87,    0,  222,  222,   91,   92,   93,   94,
 /*  6190 */     7,   96,  222,  222,   99,  222,  222,  222,  222,  222,
 /*  6200 */   222,  106,  222,  222,   21,  110,  111,  112,  113,  114,
 /*  6210 */   115,  116,  117,  118,  119,  222,  222,   34,  222,  222,
 /*  6220 */   222,  222,  222,  222,  222,  222,   43,   44,   41,   46,
 /*  6230 */   222,   48,  222,   50,  222,   52,   53,   54,  222,   56,
 /*  6240 */   222,  222,  222,   60,   57,  222,  222,   64,   65,   66,
 /*  6250 */   222,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  6260 */    77,   78,   79,   80,   81,   82,   83,   84,  222,  222,
 /*  6270 */    87,    7,  222,  222,   91,   92,   93,   94,  222,   96,
 /*  6280 */   222,  222,   99,  222,  222,   21,  222,  100,  101,  102,
 /*  6290 */   103,  104,  105,  110,  111,  112,  113,  222,   34,  222,
 /*  6300 */   222,  222,  119,  222,  222,  222,   41,   43,   44,  222,
 /*  6310 */    46,  222,   48,  222,   50,  222,   52,   53,   54,  222,
 /*  6320 */    56,  222,   57,  222,   60,  222,  222,  222,   64,   65,
 /*  6330 */    66,  222,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  6340 */    76,   77,   78,   79,   80,   81,   82,   83,   84,  222,
 /*  6350 */   222,   87,  222,  222,  222,   91,   92,   93,   94,  222,
 /*  6360 */    96,  222,  222,   99,  222,  100,  101,  102,  103,  104,
 /*  6370 */   105,  222,  222,  222,  110,  111,  112,  113,  222,  222,
 /*  6380 */   222,  222,  222,  119,  121,  122,  123,  124,  125,  126,
 /*  6390 */   127,  128,  129,  130,  131,  132,  133,  134,  135,  136,
 /*  6400 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  6410 */   147,  148,  149,  150,  222,  222,   11,   12,   13,   14,
 /*  6420 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*  6430 */    25,   26,   27,   28,   29,   30,   31,   32,  222,  222,
 /*  6440 */   177,  178,  179,  180,  181,  182,   41,   42,    7,  222,
 /*  6450 */     9,   10,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  6460 */   222,  222,   21,  222,  222,  222,  222,  177,  222,   28,
 /*  6470 */   207,  208,  209,  222,   33,   34,   35,   36,   37,   38,
 /*  6480 */    39,  222,   41,  222,  222,   44,  196,  222,  222,  222,
 /*  6490 */   222,  222,  222,  222,  222,   54,  206,  207,  208,  209,
 /*  6500 */    59,   60,   61,  222,  222,  222,   65,  222,  222,   68,
 /*  6510 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  6520 */    79,   80,   81,   12,   13,   14,   15,   16,   17,   18,
 /*  6530 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /*  6540 */    29,   30,   31,   32,  222,  222,  222,  106,  222,  222,
 /*  6550 */    21,  222,   41,   42,  222,  114,  115,  116,  117,  118,
 /*  6560 */   222,  222,  222,  222,  222,  222,  124,  125,  126,  127,
 /*  6570 */   128,  129,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  6580 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  6590 */   148,  149,  150,   64,   65,   66,  222,   68,   69,   70,
 /*  6600 */    71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*  6610 */    81,  222,  222,  222,  222,  222,  222,  222,  222,  177,
 /*  6620 */   178,  179,  180,  181,  182,  222,  130,  131,  132,  133,
 /*  6630 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  6640 */   144,  145,  146,  147,  148,  149,  150,  222,  222,  207,
 /*  6650 */   208,  209,   19,   20,   21,   22,   23,   24,   25,   26,
 /*  6660 */    27,   28,   29,   30,   31,   32,  222,  222,  172,  222,
 /*  6670 */   222,  222,  222,  222,   41,   42,  222,  196,  222,  222,
 /*  6680 */   184,  185,  186,  222,  222,  222,  222,  206,  207,  208,
 /*  6690 */   209,  222,  196,  212,  222,  222,  215,  216,  217,  222,
 /*  6700 */   222,  222,  206,  207,  208,  209,  130,  131,  132,  133,
 /*  6710 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  6720 */   144,  145,  146,  147,  148,  149,  150,   13,   14,   15,
 /*  6730 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*  6740 */    26,   27,   28,   29,   30,   31,   32,  222,  172,  222,
 /*  6750 */   222,  222,  222,  222,  196,   41,   42,  222,  222,  222,
 /*  6760 */   184,  185,  186,  222,  206,  207,  208,  209,  222,  222,
 /*  6770 */   222,  222,  196,  215,  216,  217,  222,  222,  222,  222,
 /*  6780 */   222,  222,  206,  207,  208,  209,  222,  222,  222,  222,
 /*  6790 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  130,
 /*  6800 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  6810 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  6820 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*  6830 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  222,
 /*  6840 */   222,  172,  222,  222,  222,  222,  222,   41,   42,  222,
 /*  6850 */   222,  222,  222,  184,  185,  186,   21,  222,  222,  222,
 /*  6860 */   222,  222,  222,  222,  222,  196,  222,  222,  222,  222,
 /*  6870 */   222,  222,  222,  222,  222,  206,  207,  208,  209,  130,
 /*  6880 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  6890 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  6900 */    65,   66,  222,   68,   69,   70,   71,   72,   73,   74,
 /*  6910 */    75,   76,   77,   78,   79,   80,   81,  222,  222,  222,
 /*  6920 */   222,  172,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  6930 */   222,  222,  222,  184,  185,  186,   41,   42,  222,  222,
 /*  6940 */   222,  222,  222,   41,  222,  196,   44,  222,  222,  222,
 /*  6950 */   222,  222,   57,  222,  222,  206,  207,  208,  209,  222,
 /*  6960 */   222,   59,  222,  222,  222,  222,  222,   65,  222,   67,
 /*  6970 */   222,  222,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  6980 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  6990 */   148,  149,  150,  222,  222,  100,  101,  102,  103,  104,
 /*  7000 */   105,  222,  107,  108,  109,  222,  222,  222,  106,  222,
 /*  7010 */   222,  222,  222,  222,  172,   41,  114,  115,  116,  117,
 /*  7020 */   118,  222,   41,  222,  222,   44,  184,  185,  186,   41,
 /*  7030 */   222,   57,  222,  222,  222,  222,  222,  222,  196,  222,
 /*  7040 */    59,  222,  222,  222,  222,   57,   65,  222,  206,  207,
 /*  7050 */   208,  209,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  7060 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  7070 */   148,  149,  150,  222,  100,  101,  102,  103,  104,  105,
 /*  7080 */   222,  222,  108,  109,  222,  222,  222,  106,  100,  101,
 /*  7090 */   102,  103,  104,  105,  172,  114,  115,  116,  117,  118,
 /*  7100 */   222,   41,  222,  222,   44,  222,  184,  185,  186,  222,
 /*  7110 */   222,  222,  222,  222,  222,  222,  222,  222,  196,   59,
 /*  7120 */    41,  222,  222,  222,  222,   65,  222,  222,  206,  207,
 /*  7130 */   208,  209,  222,  222,  222,  222,   57,  222,  222,  222,
 /*  7140 */   222,  222,  222,  222,  222,  130,  131,  132,  133,  134,
 /*  7150 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  7160 */   145,  146,  147,  148,  149,  150,  106,  222,  222,  222,
 /*  7170 */   222,  222,  222,  222,  114,  115,  116,  117,  118,  100,
 /*  7180 */   101,  102,  103,  104,  105,  222,  222,  172,  222,  222,
 /*  7190 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  184,
 /*  7200 */   185,  186,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7210 */   222,  196,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7220 */   222,  206,  207,  208,  209,  130,  131,  132,  133,  134,
 /*  7230 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  7240 */   145,  146,  147,  148,  149,  150,  222,  222,  222,  222,
 /*  7250 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7260 */   222,  222,  222,  222,  222,  222,  222,  172,  222,  222,
 /*  7270 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  184,
 /*  7280 */   185,  186,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7290 */   222,  196,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7300 */   222,  206,  207,  208,  209,  222,  222,  222,  222,  222,
 /*  7310 */   222,  222,  222,  222,  222,  222,  222,  222,  130,  131,
 /*  7320 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  7330 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  222,
 /*  7340 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7350 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7360 */   172,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7370 */   222,  222,  184,  185,  186,  222,  222,  222,  222,  222,
 /*  7380 */   222,  222,  222,  222,  196,  222,  222,  222,  222,  222,
 /*  7390 */   222,  222,  222,  222,  206,  207,  208,  209,  130,  131,
 /*  7400 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  7410 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  222,
 /*  7420 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7430 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7440 */   172,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7450 */   222,  222,  184,  185,  186,  222,  222,  222,  222,  222,
 /*  7460 */   222,  222,  222,  222,  196,  222,  222,  222,  222,  222,
 /*  7470 */   222,  222,  222,  222,  206,  207,  208,  209,  222,  222,
 /*  7480 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7490 */   222,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  7500 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  7510 */   149,  150,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7520 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7530 */   222,  222,  222,  172,  222,  222,  222,  222,  222,  222,
 /*  7540 */   222,  222,  222,  222,  222,  184,  185,  186,  222,  222,
 /*  7550 */   222,  222,  222,  222,  222,  222,  222,  196,  222,  222,
 /*  7560 */   222,  222,  222,  222,  222,  222,  222,  206,  207,  208,
 /*  7570 */   209,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  7580 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  7590 */   149,  150,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7600 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7610 */   222,  222,  222,  172,  222,  222,  222,  222,  222,  222,
 /*  7620 */   222,  222,  222,  222,  222,  184,  185,  186,  222,  222,
 /*  7630 */   222,  222,  222,  222,  222,  222,  222,  196,  222,  222,
 /*  7640 */   222,  222,  222,  222,  222,  222,  222,  206,  207,  208,
 /*  7650 */   209,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7660 */   222,  222,  222,  222,  130,  131,  132,  133,  134,  135,
 /*  7670 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  7680 */   146,  147,  148,  149,  150,  222,  222,  222,  222,  222,
 /*  7690 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7700 */   222,  222,  222,  222,  222,  222,  172,  222,  222,  222,
 /*  7710 */   222,  222,  222,  222,  222,  222,  222,  222,  184,  185,
 /*  7720 */   186,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7730 */   196,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7740 */   206,  207,  208,  209,  130,  131,  132,  133,  134,  135,
 /*  7750 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  7760 */   146,  147,  148,  149,  150,  222,  222,  222,  222,  222,
 /*  7770 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7780 */   222,  222,  222,  222,  222,  222,  172,  222,  222,  222,
 /*  7790 */   222,  222,  222,  222,  222,  222,  222,  222,  184,  185,
 /*  7800 */   186,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7810 */   196,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7820 */   206,  207,  208,  209,  222,  222,  222,  222,  222,  222,
 /*  7830 */   222,  222,  222,  222,  222,  222,  222,  130,  131,  132,
 /*  7840 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  7850 */   143,  144,  145,  146,  147,  148,  149,  150,  222,  222,
 /*  7860 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7870 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  172,
 /*  7880 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7890 */   222,  184,  185,  186,  222,  222,  222,  222,  222,  222,
 /*  7900 */   222,  222,  222,  196,  222,  222,  222,  222,  222,  222,
 /*  7910 */   222,  222,  222,  206,  207,  208,  209,  130,  131,  132,
 /*  7920 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  7930 */   143,  144,  145,  146,  147,  148,  149,  150,  222,  222,
 /*  7940 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7950 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  172,
 /*  7960 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  7970 */   222,  184,  185,  186,  222,  222,  222,  222,  222,  222,
 /*  7980 */   222,  222,  222,  196,  222,  222,  222,  222,  222,  222,
 /*  7990 */   222,  222,  222,  206,  207,  208,  209,  222,  222,  222,
 /*  8000 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8010 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  8020 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  8030 */   150,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8040 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8050 */   222,  222,  172,  222,  222,  222,  222,  222,  222,  222,
 /*  8060 */   222,  222,  222,  222,  184,  185,  186,  222,  222,  222,
 /*  8070 */   222,  222,  222,  222,  222,  222,  196,  222,  222,  222,
 /*  8080 */   222,  222,  222,  222,  222,  222,  206,  207,  208,  209,
 /*  8090 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  8100 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  8110 */   150,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8120 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8130 */   222,  222,  172,  222,  222,  222,  222,  222,  222,  222,
 /*  8140 */   222,  222,  222,  222,  184,  185,  186,  222,  222,  222,
 /*  8150 */   222,  222,  222,  222,  222,  222,  196,  222,  222,  222,
 /*  8160 */   222,  222,  222,  222,  222,  222,  206,  207,  208,  209,
 /*  8170 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8180 */   222,  222,  222,  130,  131,  132,  133,  134,  135,  136,
 /*  8190 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  8200 */   147,  148,  149,  150,  222,  222,  222,  222,  222,  222,
 /*  8210 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8220 */   222,  222,  222,  222,  222,  172,  222,  222,  222,  222,
 /*  8230 */   222,  222,  222,  222,  222,  222,  222,  184,  185,  186,
 /*  8240 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  196,
 /*  8250 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  206,
 /*  8260 */   207,  208,  209,  130,  131,  132,  133,  134,  135,  136,
 /*  8270 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  8280 */   147,  148,  149,  150,  222,  222,  222,  222,  222,  222,
 /*  8290 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8300 */   222,  222,  222,  222,  222,  172,  222,  222,  222,  222,
 /*  8310 */   222,  222,  222,  222,  222,  222,  222,  184,  185,  186,
 /*  8320 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  196,
 /*  8330 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  206,
 /*  8340 */   207,  208,  209,  222,  222,  222,  222,  222,  222,  222,
 /*  8350 */   222,  222,  222,  222,  222,  222,  130,  131,  132,  133,
 /*  8360 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  8370 */   144,  145,  146,  147,  148,  149,  150,  222,  222,  222,
 /*  8380 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8390 */   222,  222,  222,  222,  222,  222,  222,  222,  172,  222,
 /*  8400 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8410 */   184,  185,  186,  222,  222,  222,  222,  222,  222,  222,
 /*  8420 */   222,  222,  196,  222,  222,  222,  222,  222,  222,  222,
 /*  8430 */   222,  222,  206,  207,  208,  209,  130,  131,  132,  133,
 /*  8440 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  8450 */   144,  145,  146,  147,  148,  149,  150,  222,  222,  222,
 /*  8460 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8470 */   222,  222,  222,  222,  222,  222,  222,  222,  172,  222,
 /*  8480 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8490 */   184,  185,  186,  222,  222,  222,  222,  222,  222,  222,
 /*  8500 */   222,  222,  196,  222,  222,  222,  222,  222,  222,  222,
 /*  8510 */   222,  222,  206,  207,  208,  209,  222,  222,  222,  222,
 /*  8520 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  130,
 /*  8530 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  8540 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  8550 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8560 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8570 */   222,  172,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8580 */   222,  222,  222,  184,  185,  186,  222,  222,  222,  222,
 /*  8590 */   222,  222,  222,  222,  222,  196,  222,  222,  222,  222,
 /*  8600 */   222,  222,  222,  222,  222,  206,  207,  208,  209,  130,
 /*  8610 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  8620 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  8630 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8640 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8650 */   222,  172,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8660 */   222,  222,  222,  184,  185,  186,  222,  222,  222,  222,
 /*  8670 */   222,  222,  222,  222,  222,  196,  222,  222,  222,  222,
 /*  8680 */   222,  222,  222,  222,  222,  206,  207,  208,  209,  222,
 /*  8690 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8700 */   222,  222,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  8710 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  8720 */   148,  149,  150,  222,  222,  222,  222,  222,  222,  222,
 /*  8730 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8740 */   222,  222,  222,  222,  172,  222,  222,  222,  222,  222,
 /*  8750 */   222,  222,  222,  222,  222,  222,  184,  185,  186,  222,
 /*  8760 */   222,  222,  222,  222,  222,  222,  222,  222,  196,  222,
 /*  8770 */   222,  222,  222,  222,  222,  222,  222,  222,  206,  207,
 /*  8780 */   208,  209,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  8790 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  8800 */   148,  149,  150,  222,  222,  222,  222,  222,  222,  222,
 /*  8810 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8820 */   222,  222,  222,  222,  172,  222,  222,  222,  222,  222,
 /*  8830 */   222,  222,  222,  222,  222,  222,  184,  185,  186,  222,
 /*  8840 */   222,  222,  222,  222,  222,  222,  222,  222,  196,  222,
 /*  8850 */   222,  222,  222,  222,  222,  222,  222,  222,  206,  207,
 /*  8860 */   208,  209,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8870 */   222,  222,  222,  222,  222,  130,  131,  132,  133,  134,
 /*  8880 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  8890 */   145,  146,  147,  148,  149,  150,  222,  222,  222,  222,
 /*  8900 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8910 */   222,  222,  222,  222,  222,  222,  222,  172,  222,  222,
 /*  8920 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  184,
 /*  8930 */   185,  186,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8940 */   222,  196,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8950 */   222,  206,  207,  208,  209,  130,  131,  132,  133,  134,
 /*  8960 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  8970 */   145,  146,  147,  148,  149,  150,  222,  222,  222,  222,
 /*  8980 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  8990 */   222,  222,  222,  222,  222,  222,  222,  172,  222,  222,
 /*  9000 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  184,
 /*  9010 */   185,  186,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9020 */   222,  196,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9030 */   222,  206,  207,  208,  209,  222,  222,  222,  222,  222,
 /*  9040 */   222,  222,  222,  222,  222,  222,  222,  222,  130,  131,
 /*  9050 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  9060 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  222,
 /*  9070 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9080 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9090 */   172,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9100 */   222,  222,  184,  185,  186,  222,  222,  222,  222,  222,
 /*  9110 */   222,  222,  222,  222,  196,  222,  222,  222,  222,  222,
 /*  9120 */   222,  222,  222,  222,  206,  207,  208,  209,  130,  131,
 /*  9130 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  9140 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  222,
 /*  9150 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9160 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9170 */   172,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9180 */   222,  222,  184,  185,  186,  222,  222,  222,  222,  222,
 /*  9190 */   222,  222,  222,  222,  196,  222,  222,  222,  222,  222,
 /*  9200 */   222,  222,  222,  222,  206,  207,  208,  209,  222,  222,
 /*  9210 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9220 */   222,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  9230 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  9240 */   149,  150,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9250 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9260 */   222,  222,  222,  172,  222,  222,  222,  222,  222,  222,
 /*  9270 */   222,  222,  222,  222,  222,  184,  185,  186,  222,  222,
 /*  9280 */   222,  222,  222,  222,  222,  222,  222,  196,  222,  222,
 /*  9290 */   222,  222,  222,  222,  222,  222,  222,  206,  207,  208,
 /*  9300 */   209,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  9310 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  9320 */   149,  150,  222,  222,  222,    8,  222,  222,   11,   12,
 /*  9330 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*  9340 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /*  9350 */   222,  222,  222,  222,  222,  184,  185,  186,   41,   42,
 /*  9360 */   222,  222,  222,  222,  222,  222,  222,  196,  222,  222,
 /*  9370 */   222,  222,  222,  222,  222,  222,  222,  206,  207,  208,
 /*  9380 */   209,  222,  222,  222,   67,    7,  222,    9,   10,  222,
 /*  9390 */   222,  222,  222,   15,  222,  222,  222,  222,  222,   21,
 /*  9400 */   222,  222,  222,  222,  222,  222,   28,  222,  222,  222,
 /*  9410 */   222,   33,   34,   35,   36,   37,   38,   39,   40,   41,
 /*  9420 */   222,    7,   44,    9,   10,  222,  222,  222,  222,   15,
 /*  9430 */   222,  222,   54,  222,  222,   21,  222,   59,   60,   61,
 /*  9440 */   222,  222,   28,   65,  222,  222,  222,   33,   34,   35,
 /*  9450 */    36,   37,   38,   39,   40,   41,  222,   21,   44,  222,
 /*  9460 */   222,  222,  222,  222,  222,  222,  222,  222,   54,  222,
 /*  9470 */   222,  222,  222,   59,   60,   61,  222,  222,  222,   65,
 /*  9480 */   222,  222,  222,  222,  106,  222,  222,  222,  222,  222,
 /*  9490 */   222,  222,  114,  115,  116,  117,  118,  222,  222,  222,
 /*  9500 */   222,   65,   66,  222,   68,   69,   70,   71,   72,   73,
 /*  9510 */    74,   75,   76,   77,   78,   79,   80,   81,  222,  222,
 /*  9520 */   106,  222,  222,  222,  222,  222,  222,  222,  114,  115,
 /*  9530 */   116,  117,  118,  222,  222,  222,    7,  222,    9,   10,
 /*  9540 */   222,  222,  222,  222,   15,  222,  222,  222,  222,  222,
 /*  9550 */    21,  222,  222,  222,  222,  222,  222,   28,  222,  222,
 /*  9560 */   222,  222,   33,   34,   35,   36,   37,   38,   39,   40,
 /*  9570 */    41,  222,    7,   44,    9,   10,  222,  222,  222,  222,
 /*  9580 */    15,  222,  222,   54,  222,  222,   21,  222,   59,   60,
 /*  9590 */    61,  222,  222,   28,   65,  222,  222,  222,   33,   34,
 /*  9600 */    35,   36,   37,   38,   39,   40,   41,  222,   21,   44,
 /*  9610 */   222,  222,  222,  222,  222,  222,  222,  222,  222,   54,
 /*  9620 */   222,  222,  222,  222,   59,   60,   61,  222,  222,  222,
 /*  9630 */    65,   44,  222,  222,  222,  106,  222,  222,  222,  222,
 /*  9640 */   222,  222,  222,  114,  115,  116,  117,  118,  222,  222,
 /*  9650 */   222,  222,  222,  222,  222,   68,   69,   70,   71,   72,
 /*  9660 */    73,   74,   75,   76,   77,   78,   79,   80,   81,  222,
 /*  9670 */   222,  106,  222,  222,  222,  222,  222,  222,  222,  114,
 /*  9680 */   115,  116,  117,  118,  222,  222,  222,    7,  222,    9,
 /*  9690 */    10,  222,  222,  222,  222,   15,  222,  222,  222,  222,
 /*  9700 */   222,   21,  222,  222,  222,  222,  222,  222,   28,  222,
 /*  9710 */   222,  222,  222,   33,   34,   35,   36,   37,   38,   39,
 /*  9720 */    40,   41,  222,    7,   44,    9,   10,  222,  222,  222,
 /*  9730 */   222,   15,  222,  222,   54,  222,  222,   21,  222,   59,
 /*  9740 */    60,   61,  222,  222,   28,   65,  222,  222,  222,   33,
 /*  9750 */    34,   35,   36,   37,   38,   39,   40,   41,  222,  222,
 /*  9760 */    44,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9770 */    54,  222,  222,  222,  222,   59,   60,   61,  222,  222,
 /*  9780 */   222,   65,  222,  222,  222,  222,  106,  222,  222,  222,
 /*  9790 */   222,  222,  222,  222,  114,  115,  116,  117,  118,  222,
 /*  9800 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9810 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9820 */   222,  222,  106,  222,  222,  222,  222,  222,  222,  222,
 /*  9830 */   114,  115,  116,  117,  118,  222,  222,  222,    7,  222,
 /*  9840 */     9,   10,  222,  222,  222,  222,   15,  222,  222,  222,
 /*  9850 */   222,  222,   21,  222,  222,  222,  222,  222,  222,   28,
 /*  9860 */   222,  222,  222,  222,   33,   34,   35,   36,   37,   38,
 /*  9870 */    39,   40,   41,  222,    7,   44,    9,   10,  222,  222,
 /*  9880 */   222,  222,   15,  222,  222,   54,  222,  222,   21,  222,
 /*  9890 */    59,   60,   61,  222,  222,   28,   65,  222,  222,  222,
 /*  9900 */    33,   34,   35,   36,   37,   38,   39,   40,   41,  222,
 /*  9910 */   222,   44,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9920 */   222,   54,  222,  222,  222,  222,   59,   60,   61,  222,
 /*  9930 */   222,  222,   65,  222,  222,  222,  222,  106,  222,  222,
 /*  9940 */   222,  222,  222,  222,  222,  114,  115,  116,  117,  118,
 /*  9950 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9960 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /*  9970 */   222,  222,  222,  106,  222,  222,  222,  222,  222,  222,
 /*  9980 */   222,  114,  115,  116,  117,  118,  222,  222,  222,    7,
 /*  9990 */   222,    9,   10,  222,  222,  222,  222,   15,  222,  222,
 /* 10000 */   222,  222,  222,   21,  222,  222,  222,  222,  222,  222,
 /* 10010 */    28,  222,  222,  222,  222,   33,   34,   35,   36,   37,
 /* 10020 */    38,   39,   40,   41,  222,    7,   44,    9,   10,  222,
 /* 10030 */   222,  222,  222,   15,  222,  222,   54,  222,  222,   21,
 /* 10040 */   222,   59,   60,   61,  222,  222,   28,   65,  222,  222,
 /* 10050 */   222,   33,   34,   35,   36,   37,   38,   39,   40,   41,
 /* 10060 */   222,  222,   44,  222,  222,  222,  222,  222,  222,  222,
 /* 10070 */   222,  222,   54,  222,  222,  222,  222,   59,   60,   61,
 /* 10080 */   222,  222,  222,   65,  222,  222,  222,  222,  106,  222,
 /* 10090 */   222,  222,  222,  222,  222,  222,  114,  115,  116,  117,
 /* 10100 */   118,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 10110 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 10120 */   222,  222,  222,  222,  106,  222,  222,  222,  222,  222,
 /* 10130 */   222,  222,  114,  115,  116,  117,  118,  222,  222,  222,
 /* 10140 */   222,    8,  222,  222,   11,   12,   13,   14,   15,   16,
 /* 10150 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /* 10160 */    27,   28,   29,   30,   31,   32,  222,  222,  222,  222,
 /* 10170 */   222,  222,  222,    8,   41,   42,   11,   12,   13,   14,
 /* 10180 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /* 10190 */    25,   26,   27,   28,   29,   30,   31,   32,  222,  222,
 /* 10200 */     7,  222,    9,   10,  222,   40,   41,   42,   15,  222,
 /* 10210 */   222,  222,  222,  222,   21,  222,  222,  222,  222,  222,
 /* 10220 */   222,   28,   89,  222,  222,  222,   33,   34,   35,   36,
 /* 10230 */    37,   38,   39,  222,   41,  222,    7,   44,    9,   10,
 /* 10240 */   222,  222,  222,  222,  222,  222,  222,   54,  222,  222,
 /* 10250 */    21,  222,   59,   60,   61,  222,  222,   28,   65,  222,
 /* 10260 */   222,  222,   33,   34,   35,   36,   37,   38,   39,   21,
 /* 10270 */    41,  222,  222,   44,  222,  222,  222,  222,  222,  222,
 /* 10280 */   222,  222,  222,   54,  222,  222,  222,  222,   59,   60,
 /* 10290 */    61,  222,   44,  222,   65,  222,   67,  222,  222,  106,
 /* 10300 */   222,  222,  222,  222,  222,  222,   58,  114,  115,  116,
 /* 10310 */   117,  118,  222,  222,  222,  222,   68,   69,   70,   71,
 /* 10320 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /* 10330 */   222,  222,  222,  222,  222,  106,  222,  222,  222,  222,
 /* 10340 */   222,  222,  222,  114,  115,  116,  117,  118,  222,  222,
 /* 10350 */   222,    7,  222,    9,   10,  222,  222,  222,  222,   15,
 /* 10360 */   222,  222,  222,  222,  222,   21,  222,  222,  222,  222,
 /* 10370 */   222,  222,   28,  222,  222,  222,  222,   33,   34,   35,
 /* 10380 */    36,   37,   38,   39,  222,   41,  222,    7,   44,    9,
 /* 10390 */    10,  222,  222,  222,  222,  222,  222,  222,   54,  222,
 /* 10400 */   222,   21,  222,   59,   60,   61,  222,  222,   28,   65,
 /* 10410 */   222,  222,  222,   33,   34,   35,   36,   37,   38,   39,
 /* 10420 */   222,   41,  222,  222,   44,  222,  222,  222,  222,  222,
 /* 10430 */   222,  222,  222,  222,   54,  222,  222,  222,  222,   59,
 /* 10440 */    60,   61,  222,  222,  222,   65,  222,  222,  222,  222,
 /* 10450 */   106,  222,  222,  222,  222,  222,  222,  222,  114,  115,
 /* 10460 */   116,  117,  118,  222,  222,  222,  222,  222,  222,  222,
 /* 10470 */   222,  222,  222,  222,  222,  222,  222,  222,   98,  222,
 /* 10480 */   222,  222,  222,  222,  222,  222,  106,  222,  222,  222,
 /* 10490 */   222,  222,  222,  222,  114,  115,  116,  117,  118,  222,
 /* 10500 */   222,  222,  222,    8,  222,  222,   11,   12,   13,   14,
 /* 10510 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /* 10520 */    25,   26,   27,   28,   29,   30,   31,   32,  222,    7,
 /* 10530 */   222,    9,   10,  222,  222,  222,   41,   42,  222,  222,
 /* 10540 */   222,  222,  222,   21,  222,  222,  222,  222,  222,   54,
 /* 10550 */    28,  222,  222,  222,  222,   33,   34,   35,   36,   37,
 /* 10560 */    38,   39,  222,   41,  222,  222,   44,  222,  222,  222,
 /* 10570 */   222,  222,  222,  222,  222,  222,   54,  222,  222,  222,
 /* 10580 */   222,   59,   60,   61,  222,  222,  222,   65,  222,   67,
 /* 10590 */     8,  222,  222,   11,   12,   13,   14,   15,   16,   17,
 /* 10600 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /* 10610 */    28,   29,   30,   31,   32,  222,  222,  222,  222,  222,
 /* 10620 */   222,  222,  222,   41,   42,  222,  222,  222,  106,  222,
 /* 10630 */   222,    7,  222,    9,   10,  222,  114,  115,  116,  117,
 /* 10640 */   118,  222,  222,  222,  222,   21,  222,  222,  222,   67,
 /* 10650 */   222,  222,   28,  222,  222,  222,  222,   33,   34,   35,
 /* 10660 */    36,   37,   38,   39,  222,   41,  222,    7,   44,    9,
 /* 10670 */    10,  222,  222,  222,  222,  222,  222,  222,   54,  222,
 /* 10680 */   222,   21,  222,   59,   60,   61,  222,  222,   28,   65,
 /* 10690 */   222,   67,  222,   33,   34,   35,   36,   37,   38,   39,
 /* 10700 */   222,   41,  222,  222,   44,  222,  222,  222,  222,  222,
 /* 10710 */   222,  222,  222,  222,   54,  222,  222,  222,  222,   59,
 /* 10720 */    60,   61,  222,  222,  222,   65,  222,   67,  222,  222,
 /* 10730 */   106,  222,  222,    7,  222,    9,   10,  222,  114,  115,
 /* 10740 */   116,  117,  118,  222,  222,  222,  222,   21,  222,  222,
 /* 10750 */   222,  222,  222,  222,   28,  222,  222,  222,  222,   33,
 /* 10760 */    34,   35,   36,   37,   38,   39,  106,   41,  222,    7,
 /* 10770 */    44,    9,   10,  222,  114,  115,  116,  117,  118,  222,
 /* 10780 */    54,  222,  222,   21,  222,   59,   60,   61,  222,  222,
 /* 10790 */    28,   65,  222,   67,  222,   33,   34,   35,   36,   37,
 /* 10800 */    38,   39,  222,   41,  222,  222,   44,  222,  222,  222,
 /* 10810 */   222,  222,  222,  222,  222,  222,   54,  222,  222,  222,
 /* 10820 */   222,   59,   60,   61,  222,  222,  222,   65,  222,   67,
 /* 10830 */   222,  222,  106,  222,  222,    7,  222,    9,   10,  222,
 /* 10840 */   114,  115,  116,  117,  118,  222,  222,  222,  222,   21,
 /* 10850 */   222,  222,  222,  222,  222,  222,   28,  222,  222,  222,
 /* 10860 */   222,   33,   34,   35,   36,   37,   38,   39,  106,   41,
 /* 10870 */   222,    7,   44,    9,   10,  222,  114,  115,  116,  117,
 /* 10880 */   118,  222,   54,  222,  222,   21,  222,   59,   60,   61,
 /* 10890 */   222,  222,   28,   65,  222,   67,  222,   33,   34,   35,
 /* 10900 */    36,   37,   38,   39,  222,   41,  222,  222,   44,   45,
 /* 10910 */   222,  222,  222,  222,  222,  222,  222,  222,   54,  222,
 /* 10920 */   222,  222,  222,   59,   60,   61,  222,  222,  222,   65,
 /* 10930 */   222,  222,  222,  222,  106,  222,  222,  222,  222,  222,
 /* 10940 */   222,  222,  114,  115,  116,  117,  118,  222,  222,  222,
 /* 10950 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 10960 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 10970 */   106,  222,  222,  222,  222,  222,  222,  222,  114,  115,
 /* 10980 */   116,  117,  118,  222,  222,  222,  222,    8,  222,  222,
 /* 10990 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /* 11000 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /* 11010 */    31,   32,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11020 */    41,   42,  222,  222,   45,  222,  222,  222,  222,    8,
 /* 11030 */   222,  222,   11,   12,   13,   14,   15,   16,   17,   18,
 /* 11040 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /* 11050 */    29,   30,   31,   32,  222,  222,  222,  222,  222,  222,
 /* 11060 */   222,  222,   41,   42,  222,  222,   45,  222,  222,  222,
 /* 11070 */   222,    8,  222,  222,   11,   12,   13,   14,   15,   16,
 /* 11080 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /* 11090 */    27,   28,   29,   30,   31,   32,  222,  222,  222,  222,
 /* 11100 */   222,  222,  222,  222,   41,   42,  222,  222,   45,  222,
 /* 11110 */   222,  222,  222,    8,  222,  222,   11,   12,   13,   14,
 /* 11120 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /* 11130 */    25,   26,   27,   28,   29,   30,   31,   32,  222,  222,
 /* 11140 */   222,  222,  222,  222,  222,  222,   41,   42,  222,  222,
 /* 11150 */    45,  222,  222,  222,  222,    8,  222,  222,   11,   12,
 /* 11160 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /* 11170 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /* 11180 */   222,    7,  222,    9,   10,  222,  222,  222,   41,   42,
 /* 11190 */   222,  222,  222,  222,  222,   21,  222,  222,  222,  222,
 /* 11200 */   222,   54,   28,  222,  222,  222,  222,   33,   34,   35,
 /* 11210 */    36,   37,   38,   39,  222,   41,  222,  222,   44,  222,
 /* 11220 */   222,  222,  222,  222,  222,  222,  222,  222,   54,  222,
 /* 11230 */   222,  222,  222,   59,   60,   61,  222,    8,  222,   65,
 /* 11240 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /* 11250 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /* 11260 */    31,   32,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11270 */    41,   42,   98,  222,  222,  222,  222,  222,  222,  222,
 /* 11280 */   106,  222,  222,   54,  222,  222,  222,  222,  114,  115,
 /* 11290 */   116,  117,  118,  222,  222,  222,  222,    8,  222,  222,
 /* 11300 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /* 11310 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /* 11320 */    31,   32,  222,    7,  222,    9,   10,  222,  222,  222,
 /* 11330 */    41,   42,  222,  222,  222,  222,  222,   21,  222,  222,
 /* 11340 */   222,  222,  222,   54,   28,  222,  222,  222,  222,   33,
 /* 11350 */    34,   35,   36,   37,   38,   39,  222,   41,  222,  222,
 /* 11360 */    44,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11370 */    54,  222,  222,  222,  222,   59,   60,   61,  222,    8,
 /* 11380 */   222,   65,   11,   12,   13,   14,   15,   16,   17,   18,
 /* 11390 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /* 11400 */    29,   30,   31,   32,  222,  222,  222,  222,  222,  222,
 /* 11410 */   222,  222,   41,   42,  222,  222,  222,  222,  222,  222,
 /* 11420 */   222,    7,  106,    9,   10,  222,  222,  222,  222,  222,
 /* 11430 */   114,  115,  116,  117,  118,   21,  222,  222,  222,  222,
 /* 11440 */   222,  222,   28,   21,  222,  222,  222,   33,   34,   35,
 /* 11450 */    36,   37,   38,   39,  222,   41,  222,  222,   44,  222,
 /* 11460 */   222,  222,   40,  222,  222,  222,   44,  222,   54,  222,
 /* 11470 */   222,  222,  222,   59,   60,   61,  222,  222,  222,   65,
 /* 11480 */    58,  222,  222,  222,  222,  222,  222,  222,   21,  222,
 /* 11490 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /* 11500 */    78,   79,   80,   81,  222,  222,  222,   40,  222,  222,
 /* 11510 */   222,   44,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11520 */   106,  222,  222,  222,  222,   58,  222,  222,  114,  115,
 /* 11530 */   116,  117,  118,   21,  222,   68,   69,   70,   71,   72,
 /* 11540 */    73,   74,   75,   76,   77,   78,   79,   80,   81,  222,
 /* 11550 */   222,  222,   40,  222,  222,  222,   44,  222,  222,  222,
 /* 11560 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11570 */    58,  222,  222,  222,  222,  222,  222,  222,   21,  222,
 /* 11580 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /* 11590 */    78,   79,   80,   81,  222,  222,  222,   40,  222,  222,
 /* 11600 */   222,   44,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11610 */   222,  222,  222,  222,  222,   58,  222,  222,  222,  222,
 /* 11620 */   222,  222,  222,   21,  222,   68,   69,   70,   71,   72,
 /* 11630 */    73,   74,   75,   76,   77,   78,   79,   80,   81,  222,
 /* 11640 */   222,  222,   40,  222,  222,  222,   44,  222,  222,  222,
 /* 11650 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11660 */    58,  222,  222,  222,  222,  222,  222,  222,   21,  222,
 /* 11670 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /* 11680 */    78,   79,   80,   81,  222,  222,  222,   40,  222,  222,
 /* 11690 */   222,   44,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11700 */   222,  222,  222,  222,  222,   58,  222,  222,  222,  222,
 /* 11710 */   222,  222,  222,   21,  222,   68,   69,   70,   71,   72,
 /* 11720 */    73,   74,   75,   76,   77,   78,   79,   80,   81,  222,
 /* 11730 */   222,  222,   40,  222,  222,  222,   44,  222,  222,  222,
 /* 11740 */   222,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11750 */    58,  222,  222,  222,  222,  222,  222,  222,  222,  222,
 /* 11760 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /* 11770 */    78,   79,   80,   81,
};
#define YY_SHIFT_USE_DFLT (-40)
static short yy_shift_ofst[] = {
 /*     0 */  6264,    1, 6183,  -40,  -40,  -40,  -40,  -40,  -40,  -40,
 /*    10 */   -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,
 /*    20 */   -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,
 /*    30 */   -39,   29,  -40,    2,    0,  -40,    2,  -40,  116,  128,
 /*    40 */   -40,  -40,  -25, 9378, 11316,  -26, 11316, 11316, 1221, 11316,
 /*    50 */  11316,  -26, 11316, 11316, 11371, 11316, 11316,  -26, 11316, 11316,
 /*    60 */   -26, 11316, 11316, 6405, 11316, 11316, 6405, 11316,   90,   44,
 /*    70 */   211, 9414, 11371, 11316, 9317,  -40, 11316, 1221, 11316, 1221,
 /*    80 */  11316,  -26, 11316,  -26, 11316,  -26, 11316, 1221, 11316, 6806,
 /*    90 */  11316, 6714, 11316, 6633, 11316, 6633, 11316, 6633, 11316, 6633,
 /*   100 */  11316, 6633, 11316, 6511, 11316, 10133, 11316, 11371, 6441,  -40,
 /*   110 */   -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,
 /*   120 */   -40,  -40,  -40, 10165,  -40,  168, 11316,  -26,  202,  254,
 /*   130 */  11316,   90,  -28,  241,  244, 9529,  103, 10193, 11371,  302,
 /*   140 */   383, 11316,  -26,  -40, 11316,  -26,  -40,  -40,  -40,  -40,
 /*   150 */   -40,  -40,  -40,  -40, 10229, 11371,  126,  324,  352,  387,
 /*   160 */   -40,  118,  -40, 11414,  147,  322, 9565,  -40,  205,  -40,
 /*   170 */  10344,  482,  366,  267, 9680,  416,  -40,  -40,  -40,  -40,
 /*   180 */   -40,  -40, 11316, 11371,  435, 11422,  447, 10248,  -40,  476,
 /*   190 */  6981,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  560,
 /*   200 */   530,  -40,  -40, 6902,  564,  646,  329,  -40,  127,  -40,
 /*   210 */  7060,  -40,  691, 6981,  -40,  -40,  -40,  -40, 9587,  749,
 /*   220 */  6981,  -40,   16,  794, 6981,  -40,  843,  836, 6981,  -40,
 /*   230 */   884,  891, 6981,  -40,  956,  964,  -40,  346,  949, 6981,
 /*   240 */   -40,  993,  967, 6981,  -40, 1029, 1004, 6981,  -40, 1041,
 /*   250 */    -7,  106,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,
 /*   260 */   -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40,
 /*   270 */   -40,  -40,  -40,  -40,  -40,  -40,  -40, 1067,  -40, 1074,
 /*   280 */   -40, 11316, 1083,  219,  332,  326,  439, 1126,  445,  558,
 /*   290 */   -40, 11316, 1127,  -31,  -40,   40,  -40,  -40, 6981, 1113,
 /*   300 */  6095, 6095, 1154,  671,  784,  -40, 11316, 1157,  897, 1010,
 /*   310 */   -40, 1164, 1123, 1236, 1111, 11316, 1182,  -40, 11371, 1178,
 /*   320 */  1349, 1462, 1155, 1155,  -40, 1223,  796, 1575, 1688,  -40,
 /*   330 */  1224,  189, 10380, 10495, 1801, 1914,  -40,  142,  389,  -40,
 /*   340 */   142,  -40, 6895,  -40,  -40,  -40,  -40,  -40,  -40,  -40,
 /*   350 */  11316,  -40, 11371,  373, 6974, 11316,  -40, 10522,  695, 11316,
 /*   360 */   -40, 1220,  -40, 10582, 6187, 11316,  -40, 10624,  695, 11316,
 /*   370 */   -40,  -40,  -40,  -40,  -40,  503, 1234,  695, 11316,  -40,
 /*   380 */  1243,  695, 11316,  -40, 1249, 6265, 11316,  -40, 10660,  695,
 /*   390 */  11316,  -40, 6988, 11316,  -40, 10726,  695, 11316,  -40, 10762,
 /*   400 */   695, 11316,  -40, 7079, 11316,  -40, 10828,  695, 11316,  -40,
 /*   410 */   -40,  -40, 1255,  -40, 1277,  -40,  510, 1267,  695, 11316,
 /*   420 */   -40, 1269,  695, 11316,  -40,  -40, 11316,  392,  -40, 11316,
 /*   430 */   -40, 11371,  -40, 1300,  -40, 1312,  -40, 1315,  -40, 1317,
 /*   440 */   -40, 10864, 10979,  -40,  -40, 11316, 11021,  -40, 11316, 11063,
 /*   450 */   -40, 11316, 11105,  -40, 1320,  453,  -40, 1320,  -40, 1308,
 /*   460 */  6981,  -40,  -40, 1320,  566,  -40, 1320,  594,  -40, 1320,
 /*   470 */   615,  -40, 1320,  679,  -40, 1320,  803,  -40, 1320,  818,
 /*   480 */   -40, 1320,  820,  -40, 1320,  841,  -40, 1320,  863,  -40,
 /*   490 */  1320,  866,  -40, 11371,  -40,  -40,  -40,  -40, 11316, 11147,
 /*   500 */  6095, 2027,  -40, 1332, 1284, 11174, 11229, 2140, 2253,  -40,
 /*   510 */   -40, 11316, 11289, 6095, 2366,  -40,  -40, 1345, 1337, 2479,
 /*   520 */  2592,  -40,  -40, 1223,  -40,  -40,  -40,  -40,  -40, 1309,
 /*   530 */  11316, 1355,  -40,  -40,  -40, 1313, 6095, 6095,  -40,  -40,
 /*   540 */   -40, 11316, 1352, 2705, 2818,  -40,  -40, 1353, 2931, 3044,
 /*   550 */   -40,  -40,  -40,  652,  854, 1357, 1358,  -40, 1361, 3157,
 /*   560 */  3270,  -40,  -40,  -40,  -40,  -40, 1362, 3383, 3496,  -40,
 /*   570 */   -40,  757, 1351, 9716,  642,  -40,  -40, 1384, 1374, 1369,
 /*   580 */  9831,  673,  -40,  -40,  -40, 1390, 1380, 1376, 9867,  -40,
 /*   590 */   943,  -40,  -40, 1350, 11316,  -40,  -40,  -40, 11316, 11371,
 /*   600 */   981,  -40,  -40,  -40,  996,  -40,  -40,  764, 1383, 1391,
 /*   610 */  9982, 1012,  -40,  -40, 1389, 1392, 10018, 1036,  -40,  -40,
 /*   620 */    90,   90,   90,   90,   90,   90,   90, 1056,  -40,  -40,
 /*   630 */  1407,  244, 1410,  712,  -40, 1426, 1419,  -40,   38,  -40,
 /*   640 */  1420,  -40,  151,  716,  -40, 1846, 1440, 1424, 11467,  918,
 /*   650 */  6529, 1446,  -40,  -40, 1480, 6835,  -40, 1463,  -40,  -40,
 /*   660 */   -40,  -40,  -40, 1460,  937, 1438, 1491,  -40,  -40,  -40,
 /*   670 */  1094,  919, 6529, 1469,  -40,  -40,  -40,  -40,  -40,  -40,
 /*   680 */   -40,  -40,  -40,  -40,  -40,  -40, 2411, 1959, 1471, 1458,
 /*   690 */  11512,  940, 6529, 1475,  -40,  -40, 1109, 1008, 6529, 1479,
 /*   700 */   -40,  -40,  -40,  -40,  -40, 2072,  255, 1468, 6981, 1481,
 /*   710 */   -40, 1472, 6981, 1497,  -40,  933, 1486, 6981, 1502,  -40,
 /*   720 */  1494, 6981, 1503,  -40,  829,  -40, 1513,  345,  -40, 1521,
 /*   730 */  1023,  -40, 1523,  883,  -40,  264,  -40, 1528,  -40,  377,
 /*   740 */   942,  -40, 2185, 1542, 1526, 11557,  185, 3609,  -40, 3722,
 /*   750 */   -40,  -40, 6529,  570, 3835,  -40, 3948,  -40,  -40, 1125,
 /*   760 */   330, 4061,  -40, 4174,  -40,  -40, 6529,  834, 4287,  -40,
 /*   770 */  4400,  -40,  -40, 2411, 2298, 1544, 1529, 11602,  443, 4513,
 /*   780 */   -40, 4626,  -40,  -40, 6529, 1018, 4739,  -40, 4852,  -40,
 /*   790 */   -40, 1149,  556, 4965,  -40, 5078,  -40,  -40, 6529, 1044,
 /*   800 */  5191,  -40, 5304,  -40,  -40,  490, 1055,  -40, 2072,  -40,
 /*   810 */  2072, 1168,   69,  -40, 6981, 1045,  -40, 1552,  -40,  227,
 /*   820 */  1554,  944, 1557,  748,  -40,  -40, 1561,  -40,  -40, 1562,
 /*   830 */   -40, 1281,  657,  -40, 6981, 1060,  -40, 1573,  -40, 1582,
 /*   840 */   -40,  603, 1394, 1507, 2411, 1620,  -40, 1733, 1523,  -40,
 /*   850 */   -40,  -40, 1523,  883,  -40, 1578, 1589,  458,  -40, 1595,
 /*   860 */  1112,  -40, 1523,  883,  -40, 1523,  883,  -40, 1605, 1612,
 /*   870 */   571,  -40, 1616, 1609,  -40, 1523,  883,  -40, 1626, 1618,
 /*   880 */  11647, 1623, 6095, 5417,  -40, 1169, 1624, 5530,  -40, 5643,
 /*   890 */   -40, 1636, 1640, 1619, 11692, 1645, 6095, 5756,  -40, 1238,
 /*   900 */  1646, 5869,  -40, 5982,  -40, 1641, 1053, 9436,  -40, 1663,
 /*   910 */   -40,  -40,  -40,  -40,  -40,  -40, 1320, 1320, 1320, 1320,
 /*   920 */  1320, 1320, 1320, 1320, 1320, 1320, 1320, 1312, 1315, 1317,
 /*   930 */  11316, 11021,  -40,
};
#define YY_REDUCE_USE_DFLT (-159)
static short yy_reduce_ofst[] = {
 /*     0 */  6263, -159, 6442, -159, -159, -159, -159, -159, -159, -159,
 /*    10 */  -159, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*    20 */  -159, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*    30 */  -159, -159, -159, -116, -159, -159,  -29, -159, -159, -159,
 /*    40 */  -159, -159, -159,   35, 1963, -159, 2076, 2189, -159, 2260,
 /*    50 */  2302, -159, 2373, 2415, -159, 2486, 2528, -159, 2549, 2599,
 /*    60 */  -159, 2662, 2712, -159, 2775, 2825, -159, 2888, -159, -159,
 /*    70 */  -159,  148, -159, 2938, -159, -159, 3001, -159, 3051, -159,
 /*    80 */  3114, -159, 3164, -159, 3227, -159, 3277, -159, 3340, -159,
 /*    90 */  3390, -159, 3425, -159, 3453, -159, 3503, -159, 3538, -159,
 /*   100 */  3566, -159, 3616, -159, 3651, -159, 3679, -159, 6290, -159,
 /*   110 */  -159, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*   120 */  -159, -159, -159, -159, -159, -159, 3729, -159, -159, -159,
 /*   130 */  3764, -159, -159, -159, -159,  261, -159, 3792, -159, -159,
 /*   140 */  -159, 3842, -159, -159, 3877, -159, -159, -159, -159, -159,
 /*   150 */  -159, -159, -159, -159, 6481, -159, -159, -159, -159, -159,
 /*   160 */  -159, -159, -159, 6558, -159, -159,  374, -159, -159, -159,
 /*   170 */  1617, -159, -159, -159,  487, -159, -159, -159, -159, -159,
 /*   180 */  -159, -159, 3905, -159, -159, -158, -159, -105, -159, -159,
 /*   190 */   376, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*   200 */  -159, -159, -159, 1725, -159, -159, -159, -159, -159, -159,
 /*   210 */   -62, -159, -159,  199, -159, -159, -159, -159,  130, -159,
 /*   220 */   663, -159, -159, -159,  700, -159, -159, -159,  741, -159,
 /*   230 */  -159, -159,  812, -159, -159, -159, -159, -159, -159,  837,
 /*   240 */  -159, -159, -159,  904, -159, -159, -159,  927, -159, -159,
 /*   250 */  6496, 9171, -159, -159, -159, -159, -159, -159, -159, -159,
 /*   260 */  -159, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*   270 */  -159, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*   280 */  -159, 1017, -159, 6576, 9171,  314,  961, -159, 6669, 9171,
 /*   290 */  -159, 1130, -159,  340, -159,  995, -159, -159, 1026, -159,
 /*   300 */  6749, 9171, -159, 6842, 9171, -159, 1695, -159, 6922, 9171,
 /*   310 */  -159, -159, 7015, 9171, -159, 1808, -159, -159, -159, -159,
 /*   320 */  7095, 9171,  342, 1062, -159,  355, -159, 7188, 9171, -159,
 /*   330 */  -159, -159, 3955, -159, 7268, 9171, -159,  440, -159, -159,
 /*   340 */  1078, -159,  -53, -159, -159, -159, -159, -159, -159, -159,
 /*   350 */   -71, -159, -159, -159,   60,  607, -159, 1730, 1080, 1843,
 /*   360 */  -159, -159, -159, -159,  141, 1956, -159, 1730, 1089, 2069,
 /*   370 */  -159, -159, -159, -159, -159, -159, -159, 1095, 2182, -159,
 /*   380 */  -159, 1101, 2295, -159, -159,  122, 2408, -159, 1730, 1103,
 /*   390 */  2521, -159,  309, 2634, -159, 1730, 1104, 2747, -159, 1730,
 /*   400 */  1116, 2860, -159,  367, 2973, -159, 1730, 1117, 3086, -159,
 /*   410 */  -159, -159, -159, -159, -159, -159, -159, -159, 1132, 3199,
 /*   420 */  -159, -159, 1141, 3312, -159, -159, 2323, -159, -159, 2436,
 /*   430 */  -159, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*   440 */  -159, 3990, -159, -159, -159, 4018, -159, -159, 4068, -159,
 /*   450 */  -159, 4103, -159, -159,  433, -159, -159, 1152, -159, -159,
 /*   460 */  1203, -159, -159,  436, -159, -159,  452, -159, -159,  512,
 /*   470 */  -159, -159,  518, -159, -159,  523, -159, -159,  540, -159,
 /*   480 */  -159,  546, -159, -159,  549, -159, -159,  626, -159, -159,
 /*   490 */   631, -159, -159, -159, -159, -159, -159, -159, 4131, -159,
 /*   500 */  7361, 9171, -159, -159, -159, 4181, -159, 7441, 9171, -159,
 /*   510 */  -159, 4216, -159, 7534, 9171, -159, -159, -159, -159, 7614,
 /*   520 */  9171, -159, -159, 1197, -159, -159, -159, -159, -159, -159,
 /*   530 */  2034, -159, -159, -159, -159, -159, 7707, 9171, -159, -159,
 /*   540 */  -159, 2147, -159, 7787, 9171, -159, -159, -159, 7880, 9171,
 /*   550 */  -159, -159, -159,  685,  961, -159, -159, -159, -159, 7960,
 /*   560 */  9171, -159, -159, -159, -159, -159, -159, 8053, 9171, -159,
 /*   570 */  -159, -159, -159,  713, -159, -159, -159, -159, -159, -159,
 /*   580 */   826, -159, -159, -159, -159, -159, -159, -159,  939, -159,
 /*   590 */  -159, -159, -159, -159, 1850, -159, -159, -159, 4244, -159,
 /*   600 */  -159, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*   610 */  1052, -159, -159, -159, -159, -159, 1165, -159, -159, -159,
 /*   620 */  -159, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*   630 */  -159, -159, -159, 1306, -159, -159, 1322, -159, 1319, -159,
 /*   640 */  -159, -159, 1318,  107, -159, 1303, -159, -159, -127, -159,
 /*   650 */    59, -159, -159, -159, -159,  597, -159, -159, -159, -159,
 /*   660 */  -159, -159, -159, -159, -159, -159, -159, -159, -159, -159,
 /*   670 */  -159, -159,  172, -159, -159, -159, -159, -159, -159, -159,
 /*   680 */  -159, -159, -159, -159, -159, -159,  672, 1303, -159, -159,
 /*   690 */   181, -159,  285, -159, -159, -159, -159, -159,  398, -159,
 /*   700 */  -159, -159, -159, -159, -159,  672, -159, -159, 1364, -159,
 /*   710 */  -159, -159, 1377, -159, -159, -159, -159, 1386, -159, -159,
 /*   720 */  -159, 1388, -159, -159,  107, -159, -159, 1411, -159, -159,
 /*   730 */  1413, -159,  -73, 1418, -159, -151, -159, -159, -159, 1432,
 /*   740 */   135, -159, 1303, -159, -159,  294, -159, 8133, -159, 9171,
 /*   750 */  -159, -159,  511, -159, 8226, -159, 9171, -159, -159, -159,
 /*   760 */  -159, 8306, -159, 9171, -159, -159,  737, -159, 8399, -159,
 /*   770 */  9171, -159, -159,  867, 1303, -159, -159,  407, -159, 8479,
 /*   780 */  -159, 9171, -159, -159,  850, -159, 8572, -159, 9171, -159,
 /*   790 */  -159, -159, -159, 8652, -159, 9171, -159, -159,  963, -159,
 /*   800 */  8745, -159, 9171, -159, -159, 4361,  135, -159,  867, -159,
 /*   810 */   980, 1303, 1428, -159, 1425, 1433, -159, -159, -159,  788,
 /*   820 */  -159, -159, -159, 1437, -159, -159, -159, -159, -159, -159,
 /*   830 */  -159, 1303, 1450, -159, 1453, 1461, -159, -159, -159, -159,
 /*   840 */  -159, 1545, 1206,  135,  980,  135, -159,  135, 1476, -159,
 /*   850 */  -159, -159, 1020, 1484, -159, -159, -159, 1485, -159, -159,
 /*   860 */  1488, -159, 1031, 1489, -159, 1093, 1501, -159, -159, -159,
 /*   870 */  1508, -159, -159, 1511, -159, 1100, 1512, -159, -159, -159,
 /*   880 */   520, -159, 8825, 9171, -159, -159, -159, 8918, -159, 9171,
 /*   890 */  -159, -159, -159, -159,  746, -159, 8998, 9171, -159, -159,
 /*   900 */  -159, 9091, -159, 9171, -159, -159, -159,   -9, -159, -159,
 /*   910 */  -159, -159, -159, -159, -159, -159,  433,  436,  518,  523,
 /*   920 */   452,  512,  540,  549,  546,  631,  626, -159, -159, -159,
 /*   930 */  4294, -159, -159,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */  1389, 1389, 1389,  935,  937,  938,  939,  940,  941,  942,
 /*    10 */   943,  944,  945,  946,  947,  948,  949,  950,  951,  952,
 /*    20 */   953,  954,  955,  956,  957,  958,  959,  960,  961,  962,
 /*    30 */  1389, 1389,  963, 1389, 1389,  964, 1389,  965,  967, 1389,
 /*    40 */   968,  966,  967, 1389, 1389, 1269, 1389, 1389, 1270, 1389,
 /*    50 */  1389, 1271, 1389, 1389, 1272, 1389, 1389, 1273, 1389, 1389,
 /*    60 */  1274, 1389, 1389, 1275, 1389, 1389, 1276, 1389, 1284, 1389,
 /*    70 */  1288, 1389, 1350, 1389, 1389, 1293, 1389, 1294, 1389, 1295,
 /*    80 */  1389, 1296, 1389, 1297, 1389, 1298, 1389, 1299, 1389, 1300,
 /*    90 */  1389, 1301, 1389, 1302, 1389, 1303, 1389, 1304, 1389, 1305,
 /*   100 */  1389, 1306, 1389, 1307, 1389, 1389, 1389, 1347, 1389, 1116,
 /*   110 */  1117, 1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126,
 /*   120 */  1127, 1128, 1129, 1389, 1285, 1389, 1389, 1286, 1389, 1389,
 /*   130 */  1389, 1287, 1311, 1389, 1291, 1389, 1311, 1389, 1351, 1389,
 /*   140 */  1389, 1389, 1308, 1309, 1389, 1310, 1312, 1313, 1314, 1315,
 /*   150 */  1316, 1317, 1318, 1319, 1389, 1366, 1311, 1312, 1313, 1319,
 /*   160 */  1320, 1389, 1321, 1389, 1389, 1322, 1389, 1323, 1389, 1324,
 /*   170 */  1389, 1389, 1389, 1389, 1389, 1389, 1330, 1331, 1344, 1345,
 /*   180 */  1346, 1349, 1389, 1352, 1389, 1389, 1389, 1389, 1096, 1098,
 /*   190 */  1389, 1106, 1367, 1368, 1369, 1370, 1371, 1372, 1373, 1389,
 /*   200 */  1389, 1374, 1375, 1389, 1367, 1369, 1389, 1376, 1389, 1377,
 /*   210 */  1389, 1378, 1389, 1389, 1380, 1385, 1381, 1379, 1389, 1099,
 /*   220 */  1389, 1107, 1389, 1101, 1389, 1109, 1389, 1103, 1389, 1111,
 /*   230 */  1389, 1105, 1389, 1113, 1389, 1389, 1114, 1389, 1100, 1389,
 /*   240 */  1108, 1389, 1102, 1389, 1110, 1389, 1104, 1389, 1112, 1389,
 /*   250 */  1389, 1389, 1130, 1132, 1133, 1134, 1135, 1136, 1137, 1138,
 /*   260 */  1139, 1140, 1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148,
 /*   270 */  1149, 1150, 1151, 1152, 1153, 1154, 1155, 1389, 1156, 1389,
 /*   280 */  1157, 1389, 1389, 1389, 1389, 1162, 1163, 1389, 1389, 1389,
 /*   290 */  1165, 1389, 1389, 1389, 1173, 1389, 1174, 1175, 1389, 1389,
 /*   300 */  1177, 1178, 1389, 1389, 1389, 1181, 1389, 1389, 1389, 1389,
 /*   310 */  1183, 1389, 1389, 1389, 1389, 1389, 1389, 1185, 1386, 1389,
 /*   320 */  1389, 1389, 1187, 1188, 1189, 1389, 1389, 1389, 1389, 1191,
 /*   330 */  1389, 1389, 1389, 1389, 1389, 1389, 1198, 1389, 1389, 1204,
 /*   340 */  1389, 1205, 1389, 1207, 1208, 1209, 1210, 1211, 1212, 1213,
 /*   350 */  1389, 1214, 1268, 1389, 1389, 1389, 1215, 1389, 1389, 1389,
 /*   360 */  1218, 1389, 1230, 1389, 1389, 1389, 1219, 1389, 1389, 1389,
 /*   370 */  1220, 1228, 1229, 1231, 1232, 1389, 1389, 1389, 1389, 1216,
 /*   380 */  1389, 1389, 1389, 1217, 1389, 1389, 1389, 1221, 1389, 1389,
 /*   390 */  1389, 1222, 1389, 1389, 1223, 1389, 1389, 1389, 1224, 1389,
 /*   400 */  1389, 1389, 1225, 1389, 1389, 1226, 1389, 1389, 1389, 1227,
 /*   410 */  1233, 1235, 1389, 1234, 1389, 1236, 1389, 1389, 1389, 1389,
 /*   420 */  1237, 1389, 1389, 1389, 1238, 1206, 1389, 1389, 1240, 1389,
 /*   430 */  1241, 1243, 1242, 1344, 1244, 1346, 1245, 1345, 1246, 1309,
 /*   440 */  1247, 1389, 1389, 1248, 1249, 1389, 1389, 1250, 1389, 1389,
 /*   450 */  1251, 1389, 1389, 1252, 1389, 1389, 1253, 1389, 1264, 1266,
 /*   460 */  1389, 1267, 1265, 1389, 1389, 1254, 1389, 1389, 1255, 1389,
 /*   470 */  1389, 1256, 1389, 1389, 1257, 1389, 1389, 1258, 1389, 1389,
 /*   480 */  1259, 1389, 1389, 1260, 1389, 1389, 1261, 1389, 1389, 1262,
 /*   490 */  1389, 1389, 1263, 1389, 1387, 1388, 1131, 1199, 1389, 1389,
 /*   500 */  1389, 1389, 1200, 1389, 1389, 1389, 1389, 1389, 1389, 1201,
 /*   510 */  1202, 1389, 1389, 1389, 1389, 1203, 1192, 1389, 1389, 1389,
 /*   520 */  1389, 1194, 1193, 1389, 1195, 1197, 1196, 1190, 1186, 1389,
 /*   530 */  1389, 1389, 1184, 1182, 1180, 1389, 1389, 1179, 1176, 1167,
 /*   540 */  1169, 1389, 1389, 1389, 1389, 1172, 1171, 1389, 1389, 1389,
 /*   550 */  1164, 1166, 1170, 1158, 1159, 1389, 1389, 1161, 1389, 1389,
 /*   560 */  1389, 1168, 1160, 1357, 1356, 1097, 1389, 1389, 1389, 1355,
 /*   570 */  1354, 1389, 1389, 1389, 1389, 1334, 1335, 1389, 1389, 1389,
 /*   580 */  1389, 1389, 1336, 1337, 1348, 1389, 1389, 1325, 1389, 1326,
 /*   590 */  1389, 1327, 1358, 1389, 1389, 1360, 1361, 1359, 1389, 1353,
 /*   600 */  1389, 1332, 1333, 1292, 1389, 1338, 1339, 1389, 1389, 1289,
 /*   610 */  1389, 1389, 1340, 1341, 1389, 1290, 1389, 1389, 1342, 1343,
 /*   620 */  1283, 1282, 1281, 1280, 1279, 1278, 1277, 1389, 1328, 1329,
 /*   630 */  1389, 1389, 1389, 1389,  969, 1389, 1389,  970, 1389,  987,
 /*   640 */  1389,  988, 1389, 1389, 1021, 1389, 1389, 1389, 1389, 1389,
 /*   650 */  1389, 1389, 1051, 1070, 1071, 1389, 1072, 1074, 1077, 1075,
 /*   660 */  1076, 1078, 1079, 1389, 1389, 1389, 1389, 1115, 1073, 1055,
 /*   670 */  1389, 1389, 1389, 1389, 1052, 1056, 1059, 1061, 1062, 1063,
 /*   680 */  1064, 1065, 1066, 1067, 1068, 1069, 1389, 1389, 1389, 1389,
 /*   690 */  1389, 1389, 1389, 1389, 1053, 1057, 1389, 1389, 1389, 1389,
 /*   700 */  1054, 1058, 1060, 1017, 1022, 1389, 1389, 1389, 1389, 1389,
 /*   710 */  1023, 1389, 1389, 1389, 1025, 1389, 1389, 1389, 1389, 1024,
 /*   720 */  1389, 1389, 1389, 1026, 1389, 1018, 1389, 1389,  971, 1389,
 /*   730 */  1389,  972, 1389, 1389,  974, 1389,  982, 1389,  983, 1389,
 /*   740 */  1389, 1019, 1389, 1389, 1389, 1389, 1389, 1389, 1027, 1389,
 /*   750 */  1031, 1028, 1389, 1389, 1389, 1039, 1389, 1043, 1040, 1389,
 /*   760 */  1389, 1389, 1029, 1389, 1032, 1030, 1389, 1389, 1389, 1041,
 /*   770 */  1389, 1044, 1042, 1389, 1389, 1389, 1389, 1389, 1389, 1389,
 /*   780 */  1033, 1389, 1037, 1034, 1389, 1389, 1389, 1045, 1389, 1049,
 /*   790 */  1046, 1389, 1389, 1389, 1035, 1389, 1038, 1036, 1389, 1389,
 /*   800 */  1389, 1047, 1389, 1050, 1048, 1389, 1389, 1020, 1389, 1001,
 /*   810 */  1389, 1389, 1389, 1003, 1389, 1389, 1005, 1389, 1009, 1389,
 /*   820 */  1389, 1389, 1389, 1389, 1013, 1015, 1389, 1016, 1014, 1389,
 /*   830 */  1007, 1389, 1389, 1004, 1389, 1389, 1006, 1389, 1010, 1389,
 /*   840 */  1008, 1389, 1389, 1389, 1389, 1389, 1002, 1389, 1389,  984,
 /*   850 */   986,  985, 1389, 1389,  973, 1389, 1389, 1389,  975, 1389,
 /*   860 */  1389,  976, 1389, 1389,  978, 1389, 1389,  977, 1389, 1389,
 /*   870 */  1389,  979, 1389, 1389,  980, 1389, 1389,  981, 1389, 1389,
 /*   880 */  1389, 1389, 1389, 1389, 1080, 1389, 1389, 1389, 1081, 1389,
 /*   890 */  1082, 1389, 1389, 1389, 1389, 1389, 1389, 1389, 1083, 1389,
 /*   900 */  1389, 1389, 1084, 1389, 1085, 1389, 1389, 1389, 1088, 1090,
 /*   910 */  1093, 1091, 1092, 1094, 1095, 1089, 1116, 1117, 1118, 1119,
 /*   920 */  1120, 1121, 1122, 1123, 1124, 1125, 1126, 1389, 1389, 1389,
 /*   930 */  1389, 1389,  936,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammer, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  int stateno;       /* The state-number */
  int major;         /* The major token value.  This is the code
                     ** number for the token at this stack level */
  YYMINORTYPE minor; /* The user-supplied minor token value.  This
                     ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
  int yyerrcnt;                 /* Shifts left before out of the error */
  xx_ARG_SDECL                /* A place to hold %extra_argument */
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void xx_Trace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *yyTokenName[] = { 
  "$",             "PUBLIC",        "PROTECTED",     "STATIC",      
  "PRIVATE",       "SCOPED",        "COMMA",         "REQUIRE",     
  "QUESTION",      "LIKELY",        "UNLIKELY",      "INSTANCEOF",  
  "OR",            "AND",           "BITWISE_OR",    "BITWISE_AND", 
  "BITWISE_XOR",   "BITWISE_SHIFTLEFT",  "BITWISE_SHIFTRIGHT",  "EQUALS",      
  "IDENTICAL",     "LESS",          "GREATER",       "LESSEQUAL",   
  "GREATEREQUAL",  "NOTIDENTICAL",  "NOTEQUALS",     "ADD",         
  "SUB",           "CONCAT",        "MUL",           "DIV",         
  "MOD",           "ISSET",         "FETCH",         "EMPTY",       
  "TYPEOF",        "CLONE",         "NEW",           "NOT",         
  "PARENTHESES_CLOSE",  "SBRACKET_OPEN",  "ARROW",         "NAMESPACE",   
  "IDENTIFIER",    "DOTCOMMA",      "USE",           "AS",          
  "INTERFACE",     "EXTENDS",       "CLASS",         "IMPLEMENTS",  
  "ABSTRACT",      "FINAL",         "BRACKET_OPEN",  "BRACKET_CLOSE",
  "COMMENT",       "ASSIGN",        "CONST",         "CONSTANT",    
  "FUNCTION",      "PARENTHESES_OPEN",  "INLINE",        "DEPRECATED",  
  "VOID",          "NULL",          "THIS",          "SBRACKET_CLOSE",
  "TYPE_INTEGER",  "TYPE_UINTEGER",  "TYPE_LONG",     "TYPE_ULONG",  
  "TYPE_CHAR",     "TYPE_UCHAR",    "TYPE_DOUBLE",   "TYPE_BOOL",   
  "TYPE_STRING",   "TYPE_ARRAY",    "TYPE_VAR",      "TYPE_CALLABLE",
  "TYPE_RESOURCE",  "TYPE_OBJECT",   "BREAK",         "CONTINUE",    
  "IF",            "ELSE",          "ELSEIF",        "SWITCH",      
  "CASE",          "COLON",         "DEFAULT",       "LOOP",        
  "WHILE",         "DO",            "TRY",           "CATCH",       
  "FOR",           "IN",            "REVERSE",       "LET",         
  "ADDASSIGN",     "SUBASSIGN",     "MULASSIGN",     "DIVASSIGN",   
  "CONCATASSIGN",  "MODASSIGN",     "STRING",        "DOUBLECOLON", 
  "INCR",          "DECR",          "ECHO",          "RETURN",      
  "UNSET",         "THROW",         "INTEGER",       "CHAR",        
  "DOUBLE",        "TRUE",          "FALSE",         "CBLOCK",      
  "error",         "program",       "xx_language",   "xx_top_statement_list",
  "xx_top_statement",  "xx_namespace_def",  "xx_use_aliases",  "xx_class_def",
  "xx_interface_def",  "xx_function_definition",  "xx_comment",    "xx_cblock",   
  "xx_let_statement",  "xx_if_statement",  "xx_loop_statement",  "xx_echo_statement",
  "xx_return_statement",  "xx_require_statement",  "xx_fetch_statement",  "xx_fcall_statement",
  "xx_scall_statement",  "xx_unset_statement",  "xx_throw_statement",  "xx_declare_statement",
  "xx_break_statement",  "xx_continue_statement",  "xx_while_statement",  "xx_do_while_statement",
  "xx_try_catch_statement",  "xx_switch_statement",  "xx_for_statement",  "xx_use_aliases_list",
  "xx_interface_body",  "xx_class_body",  "xx_implements_list",  "xx_class_definition",
  "xx_implements",  "xx_interface_definition",  "xx_class_properties_definition",  "xx_class_consts_definition",
  "xx_class_methods_definition",  "xx_interface_methods_definition",  "xx_class_property_definition",  "xx_visibility_list",
  "xx_literal_expr",  "xx_class_property_shortcuts",  "xx_class_property_shortcuts_list",  "xx_class_property_shortcut",
  "xx_class_const_definition",  "xx_class_method_definition",  "xx_interface_method_definition",  "xx_parameter_list",
  "xx_statement_list",  "xx_method_return_type",  "xx_visibility",  "xx_method_return_type_list",
  "xx_method_return_type_item",  "xx_parameter_type",  "xx_parameter_cast",  "xx_parameter_cast_collection",
  "xx_function_return_type",  "xx_function_return_type_list",  "xx_function_return_type_item",  "xx_parameter",
  "xx_statement",  "xx_mcall_statement",  "xx_empty_statement",  "xx_eval_expr",
  "xx_elseif_statements",  "xx_elseif_statement",  "xx_case_clauses",  "xx_case_clause",
  "xx_catch_statement_list",  "xx_catch_statement",  "xx_catch_classes_list",  "xx_catch_class",
  "xx_common_expr",  "xx_let_assignments",  "xx_let_assignment",  "xx_assignment_operator",
  "xx_assign_expr",  "xx_array_offset_list",  "xx_array_offset",  "xx_index_expr",
  "xx_echo_expressions",  "xx_echo_expression",  "xx_mcall_expr",  "xx_fcall_expr",
  "xx_scall_expr",  "xx_fetch_expr",  "xx_declare_variable_list",  "xx_declare_variable",
  "xx_array_list",  "xx_call_parameters",  "xx_call_parameter",  "xx_array_item",
  "xx_array_key",  "xx_array_value",  "xx_literal_array_list",  "xx_literal_array_item",
  "xx_literal_array_key",  "xx_literal_array_value",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *yyRuleName[] = {
 /*   0 */ "program ::= xx_language",
 /*   1 */ "xx_language ::= xx_top_statement_list",
 /*   2 */ "xx_top_statement_list ::= xx_top_statement_list xx_top_statement",
 /*   3 */ "xx_top_statement_list ::= xx_top_statement",
 /*   4 */ "xx_top_statement ::= xx_namespace_def",
 /*   5 */ "xx_top_statement ::= xx_use_aliases",
 /*   6 */ "xx_top_statement ::= xx_class_def",
 /*   7 */ "xx_top_statement ::= xx_interface_def",
 /*   8 */ "xx_top_statement ::= xx_function_definition",
 /*   9 */ "xx_top_statement ::= xx_comment",
 /*  10 */ "xx_top_statement ::= xx_cblock",
 /*  11 */ "xx_top_statement ::= xx_let_statement",
 /*  12 */ "xx_top_statement ::= xx_if_statement",
 /*  13 */ "xx_top_statement ::= xx_loop_statement",
 /*  14 */ "xx_top_statement ::= xx_echo_statement",
 /*  15 */ "xx_top_statement ::= xx_return_statement",
 /*  16 */ "xx_top_statement ::= xx_require_statement",
 /*  17 */ "xx_top_statement ::= xx_fetch_statement",
 /*  18 */ "xx_top_statement ::= xx_fcall_statement",
 /*  19 */ "xx_top_statement ::= xx_scall_statement",
 /*  20 */ "xx_top_statement ::= xx_unset_statement",
 /*  21 */ "xx_top_statement ::= xx_throw_statement",
 /*  22 */ "xx_top_statement ::= xx_declare_statement",
 /*  23 */ "xx_top_statement ::= xx_break_statement",
 /*  24 */ "xx_top_statement ::= xx_continue_statement",
 /*  25 */ "xx_top_statement ::= xx_while_statement",
 /*  26 */ "xx_top_statement ::= xx_do_while_statement",
 /*  27 */ "xx_top_statement ::= xx_try_catch_statement",
 /*  28 */ "xx_top_statement ::= xx_switch_statement",
 /*  29 */ "xx_top_statement ::= xx_for_statement",
 /*  30 */ "xx_namespace_def ::= NAMESPACE IDENTIFIER DOTCOMMA",
 /*  31 */ "xx_namespace_def ::= USE xx_use_aliases_list DOTCOMMA",
 /*  32 */ "xx_use_aliases_list ::= xx_use_aliases_list COMMA xx_use_aliases",
 /*  33 */ "xx_use_aliases_list ::= xx_use_aliases",
 /*  34 */ "xx_use_aliases ::= IDENTIFIER",
 /*  35 */ "xx_use_aliases ::= IDENTIFIER AS IDENTIFIER",
 /*  36 */ "xx_interface_def ::= INTERFACE IDENTIFIER xx_interface_body",
 /*  37 */ "xx_interface_def ::= INTERFACE IDENTIFIER EXTENDS IDENTIFIER xx_interface_body",
 /*  38 */ "xx_class_def ::= CLASS IDENTIFIER xx_class_body",
 /*  39 */ "xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body",
 /*  40 */ "xx_class_def ::= CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  41 */ "xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  42 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER xx_class_body",
 /*  43 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body",
 /*  44 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  45 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER EXTENDS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  46 */ "xx_class_def ::= FINAL CLASS IDENTIFIER xx_class_body",
 /*  47 */ "xx_class_def ::= FINAL CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body",
 /*  48 */ "xx_class_def ::= FINAL CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  49 */ "xx_class_body ::= BRACKET_OPEN BRACKET_CLOSE",
 /*  50 */ "xx_class_body ::= BRACKET_OPEN xx_class_definition BRACKET_CLOSE",
 /*  51 */ "xx_implements_list ::= xx_implements_list COMMA xx_implements",
 /*  52 */ "xx_implements_list ::= xx_implements",
 /*  53 */ "xx_implements ::= IDENTIFIER",
 /*  54 */ "xx_interface_body ::= BRACKET_OPEN BRACKET_CLOSE",
 /*  55 */ "xx_interface_body ::= BRACKET_OPEN xx_interface_definition BRACKET_CLOSE",
 /*  56 */ "xx_class_definition ::= xx_class_properties_definition",
 /*  57 */ "xx_class_definition ::= xx_class_consts_definition",
 /*  58 */ "xx_class_definition ::= xx_class_methods_definition",
 /*  59 */ "xx_class_definition ::= xx_class_properties_definition xx_class_methods_definition",
 /*  60 */ "xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition",
 /*  61 */ "xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition",
 /*  62 */ "xx_class_definition ::= xx_class_consts_definition xx_class_methods_definition",
 /*  63 */ "xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition xx_class_methods_definition",
 /*  64 */ "xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition xx_class_methods_definition",
 /*  65 */ "xx_interface_definition ::= xx_class_consts_definition",
 /*  66 */ "xx_interface_definition ::= xx_interface_methods_definition",
 /*  67 */ "xx_interface_definition ::= xx_class_consts_definition xx_interface_methods_definition",
 /*  68 */ "xx_class_properties_definition ::= xx_class_properties_definition xx_class_property_definition",
 /*  69 */ "xx_class_properties_definition ::= xx_class_property_definition",
 /*  70 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER DOTCOMMA",
 /*  71 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER DOTCOMMA",
 /*  72 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  73 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  74 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA",
 /*  75 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA",
 /*  76 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA",
 /*  77 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA",
 /*  78 */ "xx_class_property_shortcuts ::= BRACKET_OPEN BRACKET_CLOSE",
 /*  79 */ "xx_class_property_shortcuts ::= BRACKET_OPEN xx_class_property_shortcuts_list BRACKET_CLOSE",
 /*  80 */ "xx_class_property_shortcuts_list ::= xx_class_property_shortcuts_list COMMA xx_class_property_shortcut",
 /*  81 */ "xx_class_property_shortcuts_list ::= xx_class_property_shortcut",
 /*  82 */ "xx_class_property_shortcut ::= IDENTIFIER",
 /*  83 */ "xx_class_property_shortcut ::= COMMENT IDENTIFIER",
 /*  84 */ "xx_class_consts_definition ::= xx_class_consts_definition xx_class_const_definition",
 /*  85 */ "xx_class_consts_definition ::= xx_class_const_definition",
 /*  86 */ "xx_class_methods_definition ::= xx_class_methods_definition xx_class_method_definition",
 /*  87 */ "xx_class_methods_definition ::= xx_class_method_definition",
 /*  88 */ "xx_interface_methods_definition ::= xx_interface_methods_definition xx_interface_method_definition",
 /*  89 */ "xx_interface_methods_definition ::= xx_interface_method_definition",
 /*  90 */ "xx_class_const_definition ::= COMMENT CONST CONSTANT ASSIGN xx_literal_expr DOTCOMMA",
 /*  91 */ "xx_class_const_definition ::= CONST CONSTANT ASSIGN xx_literal_expr DOTCOMMA",
 /*  92 */ "xx_class_const_definition ::= COMMENT CONST IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  93 */ "xx_class_const_definition ::= CONST IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  94 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /*  95 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /*  96 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /*  97 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /*  98 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  99 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 100 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 101 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /* 102 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 103 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /* 104 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 105 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 106 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /* 107 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 108 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /* 109 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 110 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 111 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 112 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /* 113 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 114 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /* 115 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 116 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 117 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 118 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 119 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 120 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 121 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 122 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /* 123 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /* 124 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /* 125 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /* 126 */ "xx_visibility_list ::= xx_visibility_list xx_visibility",
 /* 127 */ "xx_visibility_list ::= xx_visibility",
 /* 128 */ "xx_visibility ::= PUBLIC",
 /* 129 */ "xx_visibility ::= PROTECTED",
 /* 130 */ "xx_visibility ::= PRIVATE",
 /* 131 */ "xx_visibility ::= STATIC",
 /* 132 */ "xx_visibility ::= SCOPED",
 /* 133 */ "xx_visibility ::= INLINE",
 /* 134 */ "xx_visibility ::= DEPRECATED",
 /* 135 */ "xx_visibility ::= ABSTRACT",
 /* 136 */ "xx_visibility ::= FINAL",
 /* 137 */ "xx_method_return_type ::= VOID",
 /* 138 */ "xx_method_return_type ::= xx_method_return_type_list",
 /* 139 */ "xx_method_return_type_list ::= xx_method_return_type_list BITWISE_OR xx_method_return_type_item",
 /* 140 */ "xx_method_return_type_list ::= xx_method_return_type_item",
 /* 141 */ "xx_method_return_type_item ::= xx_parameter_type",
 /* 142 */ "xx_method_return_type_item ::= NULL",
 /* 143 */ "xx_method_return_type_item ::= THIS",
 /* 144 */ "xx_method_return_type_item ::= xx_parameter_type NOT",
 /* 145 */ "xx_method_return_type_item ::= xx_parameter_cast",
 /* 146 */ "xx_method_return_type_item ::= xx_parameter_cast_collection",
 /* 147 */ "xx_function_definition ::= FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 148 */ "xx_function_definition ::= FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 149 */ "xx_function_definition ::= FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 150 */ "xx_function_definition ::= xx_function_return_type FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 151 */ "xx_function_definition ::= xx_function_return_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 152 */ "xx_function_definition ::= xx_function_return_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 153 */ "xx_function_return_type ::= VOID",
 /* 154 */ "xx_function_return_type ::= xx_function_return_type_list",
 /* 155 */ "xx_function_return_type_list ::= xx_function_return_type_list BITWISE_OR xx_function_return_type_item",
 /* 156 */ "xx_function_return_type_list ::= xx_function_return_type_item",
 /* 157 */ "xx_function_return_type_item ::= xx_parameter_type",
 /* 158 */ "xx_function_return_type_item ::= NULL",
 /* 159 */ "xx_function_return_type_item ::= THIS",
 /* 160 */ "xx_function_return_type_item ::= xx_parameter_type NOT",
 /* 161 */ "xx_function_return_type_item ::= xx_parameter_cast",
 /* 162 */ "xx_function_return_type_item ::= xx_parameter_cast_collection",
 /* 163 */ "xx_parameter_list ::= xx_parameter_list COMMA xx_parameter",
 /* 164 */ "xx_parameter_list ::= xx_parameter",
 /* 165 */ "xx_parameter ::= IDENTIFIER",
 /* 166 */ "xx_parameter ::= CONST IDENTIFIER",
 /* 167 */ "xx_parameter ::= xx_parameter_type IDENTIFIER",
 /* 168 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER",
 /* 169 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER",
 /* 170 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER",
 /* 171 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER",
 /* 172 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER",
 /* 173 */ "xx_parameter ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 174 */ "xx_parameter ::= CONST IDENTIFIER ASSIGN xx_literal_expr",
 /* 175 */ "xx_parameter ::= xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 176 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 177 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 178 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 179 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 180 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 181 */ "xx_parameter_cast ::= LESS IDENTIFIER GREATER",
 /* 182 */ "xx_parameter_cast_collection ::= LESS IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE GREATER",
 /* 183 */ "xx_parameter_type ::= TYPE_INTEGER",
 /* 184 */ "xx_parameter_type ::= TYPE_UINTEGER",
 /* 185 */ "xx_parameter_type ::= TYPE_LONG",
 /* 186 */ "xx_parameter_type ::= TYPE_ULONG",
 /* 187 */ "xx_parameter_type ::= TYPE_CHAR",
 /* 188 */ "xx_parameter_type ::= TYPE_UCHAR",
 /* 189 */ "xx_parameter_type ::= TYPE_DOUBLE",
 /* 190 */ "xx_parameter_type ::= TYPE_BOOL",
 /* 191 */ "xx_parameter_type ::= TYPE_STRING",
 /* 192 */ "xx_parameter_type ::= TYPE_ARRAY",
 /* 193 */ "xx_parameter_type ::= TYPE_VAR",
 /* 194 */ "xx_parameter_type ::= TYPE_CALLABLE",
 /* 195 */ "xx_parameter_type ::= TYPE_RESOURCE",
 /* 196 */ "xx_parameter_type ::= TYPE_OBJECT",
 /* 197 */ "xx_statement_list ::= xx_statement_list xx_statement",
 /* 198 */ "xx_statement_list ::= xx_statement",
 /* 199 */ "xx_statement ::= xx_cblock",
 /* 200 */ "xx_statement ::= xx_let_statement",
 /* 201 */ "xx_statement ::= xx_if_statement",
 /* 202 */ "xx_statement ::= xx_loop_statement",
 /* 203 */ "xx_statement ::= xx_echo_statement",
 /* 204 */ "xx_statement ::= xx_return_statement",
 /* 205 */ "xx_statement ::= xx_require_statement",
 /* 206 */ "xx_statement ::= xx_fetch_statement",
 /* 207 */ "xx_statement ::= xx_fcall_statement",
 /* 208 */ "xx_statement ::= xx_mcall_statement",
 /* 209 */ "xx_statement ::= xx_scall_statement",
 /* 210 */ "xx_statement ::= xx_unset_statement",
 /* 211 */ "xx_statement ::= xx_throw_statement",
 /* 212 */ "xx_statement ::= xx_declare_statement",
 /* 213 */ "xx_statement ::= xx_break_statement",
 /* 214 */ "xx_statement ::= xx_continue_statement",
 /* 215 */ "xx_statement ::= xx_while_statement",
 /* 216 */ "xx_statement ::= xx_do_while_statement",
 /* 217 */ "xx_statement ::= xx_try_catch_statement",
 /* 218 */ "xx_statement ::= xx_switch_statement",
 /* 219 */ "xx_statement ::= xx_for_statement",
 /* 220 */ "xx_statement ::= xx_comment",
 /* 221 */ "xx_statement ::= xx_empty_statement",
 /* 222 */ "xx_empty_statement ::= DOTCOMMA",
 /* 223 */ "xx_break_statement ::= BREAK DOTCOMMA",
 /* 224 */ "xx_continue_statement ::= CONTINUE DOTCOMMA",
 /* 225 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 226 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements",
 /* 227 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 228 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 229 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 230 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements",
 /* 231 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 232 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 233 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 234 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 235 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 236 */ "xx_elseif_statements ::= xx_elseif_statements xx_elseif_statement",
 /* 237 */ "xx_elseif_statements ::= xx_elseif_statement",
 /* 238 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 239 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 240 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 241 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN xx_case_clauses BRACKET_CLOSE",
 /* 242 */ "xx_case_clauses ::= xx_case_clauses xx_case_clause",
 /* 243 */ "xx_case_clauses ::= xx_case_clause",
 /* 244 */ "xx_case_clause ::= CASE xx_literal_expr COLON",
 /* 245 */ "xx_case_clause ::= CASE xx_literal_expr COLON xx_statement_list",
 /* 246 */ "xx_case_clause ::= DEFAULT COLON xx_statement_list",
 /* 247 */ "xx_loop_statement ::= LOOP BRACKET_OPEN BRACKET_CLOSE",
 /* 248 */ "xx_loop_statement ::= LOOP BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 249 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 250 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 251 */ "xx_do_while_statement ::= DO BRACKET_OPEN BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 252 */ "xx_do_while_statement ::= DO BRACKET_OPEN xx_statement_list BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 253 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN BRACKET_CLOSE",
 /* 254 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 255 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_catch_statement_list",
 /* 256 */ "xx_catch_statement_list ::= xx_catch_statement_list xx_catch_statement",
 /* 257 */ "xx_catch_statement_list ::= xx_catch_statement",
 /* 258 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 259 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN BRACKET_CLOSE",
 /* 260 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN BRACKET_CLOSE",
 /* 261 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 262 */ "xx_catch_classes_list ::= xx_catch_classes_list BITWISE_OR xx_catch_class",
 /* 263 */ "xx_catch_classes_list ::= xx_catch_class",
 /* 264 */ "xx_catch_class ::= IDENTIFIER",
 /* 265 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 266 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 267 */ "xx_for_statement ::= FOR IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 268 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 269 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 270 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 271 */ "xx_let_statement ::= LET xx_let_assignments DOTCOMMA",
 /* 272 */ "xx_let_assignments ::= xx_let_assignments COMMA xx_let_assignment",
 /* 273 */ "xx_let_assignments ::= xx_let_assignment",
 /* 274 */ "xx_assignment_operator ::= ASSIGN",
 /* 275 */ "xx_assignment_operator ::= ADDASSIGN",
 /* 276 */ "xx_assignment_operator ::= SUBASSIGN",
 /* 277 */ "xx_assignment_operator ::= MULASSIGN",
 /* 278 */ "xx_assignment_operator ::= DIVASSIGN",
 /* 279 */ "xx_assignment_operator ::= CONCATASSIGN",
 /* 280 */ "xx_assignment_operator ::= MODASSIGN",
 /* 281 */ "xx_let_assignment ::= IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 282 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 283 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 284 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 285 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 286 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 287 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 288 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 289 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 290 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 291 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 292 */ "xx_let_assignment ::= IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 293 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 294 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 295 */ "xx_array_offset_list ::= xx_array_offset_list xx_array_offset",
 /* 296 */ "xx_array_offset_list ::= xx_array_offset",
 /* 297 */ "xx_array_offset ::= SBRACKET_OPEN xx_index_expr SBRACKET_CLOSE",
 /* 298 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER INCR",
 /* 299 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER DECR",
 /* 300 */ "xx_let_assignment ::= IDENTIFIER INCR",
 /* 301 */ "xx_let_assignment ::= INCR IDENTIFIER",
 /* 302 */ "xx_let_assignment ::= IDENTIFIER DECR",
 /* 303 */ "xx_let_assignment ::= DECR IDENTIFIER",
 /* 304 */ "xx_let_assignment ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 305 */ "xx_let_assignment ::= BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 306 */ "xx_index_expr ::= xx_common_expr",
 /* 307 */ "xx_echo_statement ::= ECHO xx_echo_expressions DOTCOMMA",
 /* 308 */ "xx_echo_expressions ::= xx_echo_expressions COMMA xx_echo_expression",
 /* 309 */ "xx_echo_expressions ::= xx_echo_expression",
 /* 310 */ "xx_echo_expression ::= xx_common_expr",
 /* 311 */ "xx_mcall_statement ::= xx_mcall_expr DOTCOMMA",
 /* 312 */ "xx_fcall_statement ::= xx_fcall_expr DOTCOMMA",
 /* 313 */ "xx_scall_statement ::= xx_scall_expr DOTCOMMA",
 /* 314 */ "xx_fetch_statement ::= xx_fetch_expr DOTCOMMA",
 /* 315 */ "xx_return_statement ::= RETURN xx_common_expr DOTCOMMA",
 /* 316 */ "xx_return_statement ::= RETURN DOTCOMMA",
 /* 317 */ "xx_require_statement ::= REQUIRE xx_common_expr DOTCOMMA",
 /* 318 */ "xx_unset_statement ::= UNSET xx_common_expr DOTCOMMA",
 /* 319 */ "xx_throw_statement ::= THROW xx_common_expr DOTCOMMA",
 /* 320 */ "xx_declare_statement ::= TYPE_INTEGER xx_declare_variable_list DOTCOMMA",
 /* 321 */ "xx_declare_statement ::= TYPE_UINTEGER xx_declare_variable_list DOTCOMMA",
 /* 322 */ "xx_declare_statement ::= TYPE_CHAR xx_declare_variable_list DOTCOMMA",
 /* 323 */ "xx_declare_statement ::= TYPE_UCHAR xx_declare_variable_list DOTCOMMA",
 /* 324 */ "xx_declare_statement ::= TYPE_LONG xx_declare_variable_list DOTCOMMA",
 /* 325 */ "xx_declare_statement ::= TYPE_ULONG xx_declare_variable_list DOTCOMMA",
 /* 326 */ "xx_declare_statement ::= TYPE_DOUBLE xx_declare_variable_list DOTCOMMA",
 /* 327 */ "xx_declare_statement ::= TYPE_STRING xx_declare_variable_list DOTCOMMA",
 /* 328 */ "xx_declare_statement ::= TYPE_BOOL xx_declare_variable_list DOTCOMMA",
 /* 329 */ "xx_declare_statement ::= TYPE_VAR xx_declare_variable_list DOTCOMMA",
 /* 330 */ "xx_declare_statement ::= TYPE_ARRAY xx_declare_variable_list DOTCOMMA",
 /* 331 */ "xx_declare_variable_list ::= xx_declare_variable_list COMMA xx_declare_variable",
 /* 332 */ "xx_declare_variable_list ::= xx_declare_variable",
 /* 333 */ "xx_declare_variable ::= IDENTIFIER",
 /* 334 */ "xx_declare_variable ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 335 */ "xx_assign_expr ::= xx_common_expr",
 /* 336 */ "xx_common_expr ::= NOT xx_common_expr",
 /* 337 */ "xx_common_expr ::= SUB xx_common_expr",
 /* 338 */ "xx_common_expr ::= ISSET xx_common_expr",
 /* 339 */ "xx_common_expr ::= REQUIRE xx_common_expr",
 /* 340 */ "xx_common_expr ::= CLONE xx_common_expr",
 /* 341 */ "xx_common_expr ::= EMPTY xx_common_expr",
 /* 342 */ "xx_common_expr ::= LIKELY xx_common_expr",
 /* 343 */ "xx_common_expr ::= UNLIKELY xx_common_expr",
 /* 344 */ "xx_common_expr ::= xx_common_expr EQUALS xx_common_expr",
 /* 345 */ "xx_common_expr ::= xx_common_expr NOTEQUALS xx_common_expr",
 /* 346 */ "xx_common_expr ::= xx_common_expr IDENTICAL xx_common_expr",
 /* 347 */ "xx_common_expr ::= xx_common_expr NOTIDENTICAL xx_common_expr",
 /* 348 */ "xx_common_expr ::= xx_common_expr LESS xx_common_expr",
 /* 349 */ "xx_common_expr ::= xx_common_expr GREATER xx_common_expr",
 /* 350 */ "xx_common_expr ::= xx_common_expr LESSEQUAL xx_common_expr",
 /* 351 */ "xx_common_expr ::= xx_common_expr GREATEREQUAL xx_common_expr",
 /* 352 */ "xx_common_expr ::= PARENTHESES_OPEN xx_common_expr PARENTHESES_CLOSE",
 /* 353 */ "xx_common_expr ::= PARENTHESES_OPEN xx_parameter_type PARENTHESES_CLOSE xx_common_expr",
 /* 354 */ "xx_common_expr ::= LESS IDENTIFIER GREATER xx_common_expr",
 /* 355 */ "xx_common_expr ::= xx_common_expr ARROW IDENTIFIER",
 /* 356 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 357 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE",
 /* 358 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER",
 /* 359 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 360 */ "xx_common_expr ::= xx_common_expr SBRACKET_OPEN xx_common_expr SBRACKET_CLOSE",
 /* 361 */ "xx_common_expr ::= xx_common_expr ADD xx_common_expr",
 /* 362 */ "xx_common_expr ::= xx_common_expr SUB xx_common_expr",
 /* 363 */ "xx_common_expr ::= xx_common_expr MUL xx_common_expr",
 /* 364 */ "xx_common_expr ::= xx_common_expr DIV xx_common_expr",
 /* 365 */ "xx_common_expr ::= xx_common_expr MOD xx_common_expr",
 /* 366 */ "xx_common_expr ::= xx_common_expr CONCAT xx_common_expr",
 /* 367 */ "xx_common_expr ::= xx_common_expr AND xx_common_expr",
 /* 368 */ "xx_common_expr ::= xx_common_expr OR xx_common_expr",
 /* 369 */ "xx_common_expr ::= xx_common_expr BITWISE_AND xx_common_expr",
 /* 370 */ "xx_common_expr ::= xx_common_expr BITWISE_OR xx_common_expr",
 /* 371 */ "xx_common_expr ::= xx_common_expr BITWISE_XOR xx_common_expr",
 /* 372 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTLEFT xx_common_expr",
 /* 373 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTRIGHT xx_common_expr",
 /* 374 */ "xx_common_expr ::= xx_common_expr INSTANCEOF xx_common_expr",
 /* 375 */ "xx_fetch_expr ::= FETCH IDENTIFIER COMMA xx_common_expr",
 /* 376 */ "xx_common_expr ::= xx_fetch_expr",
 /* 377 */ "xx_common_expr ::= TYPEOF xx_common_expr",
 /* 378 */ "xx_common_expr ::= IDENTIFIER",
 /* 379 */ "xx_common_expr ::= INTEGER",
 /* 380 */ "xx_common_expr ::= STRING",
 /* 381 */ "xx_common_expr ::= CHAR",
 /* 382 */ "xx_common_expr ::= DOUBLE",
 /* 383 */ "xx_common_expr ::= NULL",
 /* 384 */ "xx_common_expr ::= TRUE",
 /* 385 */ "xx_common_expr ::= FALSE",
 /* 386 */ "xx_common_expr ::= CONSTANT",
 /* 387 */ "xx_common_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 388 */ "xx_common_expr ::= SBRACKET_OPEN xx_array_list SBRACKET_CLOSE",
 /* 389 */ "xx_common_expr ::= NEW IDENTIFIER",
 /* 390 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 391 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 392 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 393 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 394 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 395 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 396 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 397 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 398 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 399 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 400 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 401 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 402 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 403 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 404 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 405 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 406 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 407 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 408 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 409 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 410 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 411 */ "xx_common_expr ::= xx_mcall_expr",
 /* 412 */ "xx_common_expr ::= xx_scall_expr",
 /* 413 */ "xx_common_expr ::= xx_fcall_expr",
 /* 414 */ "xx_common_expr ::= xx_common_expr QUESTION xx_common_expr COLON xx_common_expr",
 /* 415 */ "xx_call_parameters ::= xx_call_parameters COMMA xx_call_parameter",
 /* 416 */ "xx_call_parameters ::= xx_call_parameter",
 /* 417 */ "xx_call_parameter ::= xx_common_expr",
 /* 418 */ "xx_call_parameter ::= IDENTIFIER COLON xx_common_expr",
 /* 419 */ "xx_call_parameter ::= BITWISE_AND xx_common_expr",
 /* 420 */ "xx_call_parameter ::= IDENTIFIER COLON BITWISE_AND xx_common_expr",
 /* 421 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 422 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 423 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 424 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 425 */ "xx_array_list ::= xx_array_list COMMA xx_array_item",
 /* 426 */ "xx_array_list ::= xx_array_item",
 /* 427 */ "xx_array_item ::= xx_array_key COLON xx_array_value",
 /* 428 */ "xx_array_item ::= xx_array_value",
 /* 429 */ "xx_array_key ::= CONSTANT",
 /* 430 */ "xx_array_key ::= IDENTIFIER",
 /* 431 */ "xx_array_key ::= STRING",
 /* 432 */ "xx_array_key ::= INTEGER",
 /* 433 */ "xx_array_value ::= xx_common_expr",
 /* 434 */ "xx_literal_expr ::= INTEGER",
 /* 435 */ "xx_literal_expr ::= CHAR",
 /* 436 */ "xx_literal_expr ::= STRING",
 /* 437 */ "xx_literal_expr ::= DOUBLE",
 /* 438 */ "xx_literal_expr ::= NULL",
 /* 439 */ "xx_literal_expr ::= FALSE",
 /* 440 */ "xx_literal_expr ::= TRUE",
 /* 441 */ "xx_literal_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 442 */ "xx_literal_expr ::= CONSTANT",
 /* 443 */ "xx_literal_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 444 */ "xx_literal_expr ::= SBRACKET_OPEN xx_literal_array_list SBRACKET_CLOSE",
 /* 445 */ "xx_literal_array_list ::= xx_literal_array_list COMMA xx_literal_array_item",
 /* 446 */ "xx_literal_array_list ::= xx_literal_array_item",
 /* 447 */ "xx_literal_array_item ::= xx_literal_array_key COLON xx_literal_array_value",
 /* 448 */ "xx_literal_array_item ::= xx_literal_array_value",
 /* 449 */ "xx_literal_array_key ::= IDENTIFIER",
 /* 450 */ "xx_literal_array_key ::= STRING",
 /* 451 */ "xx_literal_array_key ::= INTEGER",
 /* 452 */ "xx_literal_array_value ::= xx_literal_expr",
 /* 453 */ "xx_eval_expr ::= xx_common_expr",
 /* 454 */ "xx_comment ::= COMMENT",
 /* 455 */ "xx_cblock ::= CBLOCK",
};
#endif /* NDEBUG */

/*
** This function returns the symbolic name associated with a token
** value.
*/
const char *xx_TokenName(int tokenType){
#ifndef NDEBUG
  if( tokenType>0 && tokenType<(int)(sizeof(yyTokenName)/sizeof(yyTokenName[0])) ){
    return yyTokenName[tokenType];
  }else{
    return "Unknown";
  }
#else
  return "";
#endif
}

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to xx_ and xx_Free.
*/
void *xx_Alloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(YYCODETYPE yymajor, YYMINORTYPE *yypminor){
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
// 1371 "parser.lemon"
{
	if ((yypminor->yy0)) {
		if ((yypminor->yy0)->free_flag) {
			
		}
		delete (yypminor->yy0);
	}
}
// 4812 "parser.cpp"
      break;
    case 122:
// 1384 "parser.lemon"
{ delete (yypminor->yy78); }
// 4817 "parser.cpp"
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor( yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from xx_Alloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void xx_Free(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  /* if( pParser->yyidx<0 ) return YY_NO_ACTION;  */
  i = yy_shift_ofst[stateno];
  if( i==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
    int iFallback;            /* Fallback token */
    if( iLookAhead<(int)(sizeof(yyFallback)/sizeof(yyFallback[0]))
           && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
           yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
      }
#endif
      return yy_find_shift_action(pParser, iFallback);
    }
#endif
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  i = yy_reduce_ofst[stateno];
  if( i==YY_REDUCE_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer ot the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
  if( yypParser->yyidx>=YYSTACKDEPTH ){
     xx_ARG_FETCH;
     yypParser->yyidx--;
#ifndef NDEBUG
     if( yyTraceFILE ){
       fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
     }
#endif
     while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
     /* Here code is inserted which will execute if the parser
     ** stack every overflows */
     xx_ARG_STORE; /* Suppress warning about unused %extra_argument var */
     return;
  }
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 121, 1 },
  { 122, 1 },
  { 123, 2 },
  { 123, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 124, 1 },
  { 125, 3 },
  { 125, 3 },
  { 151, 3 },
  { 151, 1 },
  { 126, 1 },
  { 126, 3 },
  { 128, 3 },
  { 128, 5 },
  { 127, 3 },
  { 127, 5 },
  { 127, 5 },
  { 127, 7 },
  { 127, 4 },
  { 127, 6 },
  { 127, 6 },
  { 127, 8 },
  { 127, 4 },
  { 127, 6 },
  { 127, 6 },
  { 153, 2 },
  { 153, 3 },
  { 154, 3 },
  { 154, 1 },
  { 156, 1 },
  { 152, 2 },
  { 152, 3 },
  { 155, 1 },
  { 155, 1 },
  { 155, 1 },
  { 155, 2 },
  { 155, 2 },
  { 155, 2 },
  { 155, 2 },
  { 155, 3 },
  { 155, 3 },
  { 157, 1 },
  { 157, 1 },
  { 157, 2 },
  { 158, 2 },
  { 158, 1 },
  { 162, 4 },
  { 162, 3 },
  { 162, 6 },
  { 162, 5 },
  { 162, 5 },
  { 162, 4 },
  { 162, 7 },
  { 162, 6 },
  { 165, 2 },
  { 165, 3 },
  { 166, 3 },
  { 166, 1 },
  { 167, 1 },
  { 167, 2 },
  { 159, 2 },
  { 159, 1 },
  { 160, 2 },
  { 160, 1 },
  { 161, 2 },
  { 161, 1 },
  { 168, 6 },
  { 168, 5 },
  { 168, 6 },
  { 168, 5 },
  { 169, 7 },
  { 169, 6 },
  { 169, 8 },
  { 169, 7 },
  { 169, 8 },
  { 169, 9 },
  { 169, 8 },
  { 169, 7 },
  { 169, 9 },
  { 169, 8 },
  { 169, 9 },
  { 169, 10 },
  { 169, 9 },
  { 169, 8 },
  { 169, 10 },
  { 169, 9 },
  { 169, 10 },
  { 169, 11 },
  { 169, 10 },
  { 169, 9 },
  { 169, 11 },
  { 169, 10 },
  { 169, 11 },
  { 169, 12 },
  { 170, 8 },
  { 170, 9 },
  { 170, 9 },
  { 170, 10 },
  { 170, 6 },
  { 170, 7 },
  { 170, 7 },
  { 170, 8 },
  { 163, 2 },
  { 163, 1 },
  { 174, 1 },
  { 174, 1 },
  { 174, 1 },
  { 174, 1 },
  { 174, 1 },
  { 174, 1 },
  { 174, 1 },
  { 174, 1 },
  { 174, 1 },
  { 173, 1 },
  { 173, 1 },
  { 175, 3 },
  { 175, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 2 },
  { 176, 1 },
  { 176, 1 },
  { 129, 7 },
  { 129, 7 },
  { 129, 8 },
  { 129, 8 },
  { 129, 8 },
  { 129, 9 },
  { 180, 1 },
  { 180, 1 },
  { 181, 3 },
  { 181, 1 },
  { 182, 1 },
  { 182, 1 },
  { 182, 1 },
  { 182, 2 },
  { 182, 1 },
  { 182, 1 },
  { 171, 3 },
  { 171, 1 },
  { 183, 1 },
  { 183, 2 },
  { 183, 2 },
  { 183, 3 },
  { 183, 3 },
  { 183, 4 },
  { 183, 2 },
  { 183, 3 },
  { 183, 3 },
  { 183, 4 },
  { 183, 4 },
  { 183, 5 },
  { 183, 5 },
  { 183, 6 },
  { 183, 4 },
  { 183, 5 },
  { 178, 3 },
  { 179, 5 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 177, 1 },
  { 172, 2 },
  { 172, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 184, 1 },
  { 186, 1 },
  { 144, 2 },
  { 145, 2 },
  { 133, 4 },
  { 133, 5 },
  { 133, 7 },
  { 133, 8 },
  { 133, 5 },
  { 133, 6 },
  { 133, 9 },
  { 133, 10 },
  { 133, 8 },
  { 133, 9 },
  { 133, 8 },
  { 188, 2 },
  { 188, 1 },
  { 189, 4 },
  { 189, 5 },
  { 149, 4 },
  { 149, 5 },
  { 190, 2 },
  { 190, 1 },
  { 191, 3 },
  { 191, 4 },
  { 191, 3 },
  { 134, 3 },
  { 134, 4 },
  { 146, 4 },
  { 146, 5 },
  { 147, 6 },
  { 147, 7 },
  { 148, 3 },
  { 148, 4 },
  { 148, 5 },
  { 192, 2 },
  { 192, 1 },
  { 193, 5 },
  { 193, 4 },
  { 193, 6 },
  { 193, 7 },
  { 194, 3 },
  { 194, 1 },
  { 195, 1 },
  { 150, 7 },
  { 150, 6 },
  { 150, 8 },
  { 150, 9 },
  { 150, 8 },
  { 150, 10 },
  { 132, 3 },
  { 197, 3 },
  { 197, 1 },
  { 199, 1 },
  { 199, 1 },
  { 199, 1 },
  { 199, 1 },
  { 199, 1 },
  { 199, 1 },
  { 199, 1 },
  { 198, 3 },
  { 198, 5 },
  { 198, 7 },
  { 198, 7 },
  { 198, 7 },
  { 198, 6 },
  { 198, 8 },
  { 198, 5 },
  { 198, 7 },
  { 198, 6 },
  { 198, 8 },
  { 198, 5 },
  { 198, 4 },
  { 198, 6 },
  { 201, 2 },
  { 201, 1 },
  { 202, 3 },
  { 198, 4 },
  { 198, 4 },
  { 198, 2 },
  { 198, 2 },
  { 198, 2 },
  { 198, 2 },
  { 198, 5 },
  { 198, 5 },
  { 203, 1 },
  { 135, 3 },
  { 204, 3 },
  { 204, 1 },
  { 205, 1 },
  { 185, 2 },
  { 139, 2 },
  { 140, 2 },
  { 138, 2 },
  { 136, 3 },
  { 136, 2 },
  { 137, 3 },
  { 141, 3 },
  { 142, 3 },
  { 143, 3 },
  { 143, 3 },
  { 143, 3 },
  { 143, 3 },
  { 143, 3 },
  { 143, 3 },
  { 143, 3 },
  { 143, 3 },
  { 143, 3 },
  { 143, 3 },
  { 143, 3 },
  { 210, 3 },
  { 210, 1 },
  { 211, 1 },
  { 211, 3 },
  { 200, 1 },
  { 196, 2 },
  { 196, 2 },
  { 196, 2 },
  { 196, 2 },
  { 196, 2 },
  { 196, 2 },
  { 196, 2 },
  { 196, 2 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 4 },
  { 196, 4 },
  { 196, 3 },
  { 196, 5 },
  { 196, 5 },
  { 196, 3 },
  { 196, 3 },
  { 196, 4 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 196, 3 },
  { 209, 4 },
  { 196, 1 },
  { 196, 2 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 2 },
  { 196, 3 },
  { 196, 2 },
  { 196, 4 },
  { 196, 5 },
  { 196, 4 },
  { 196, 6 },
  { 196, 7 },
  { 207, 4 },
  { 207, 3 },
  { 207, 6 },
  { 207, 5 },
  { 208, 6 },
  { 208, 5 },
  { 208, 8 },
  { 208, 7 },
  { 208, 10 },
  { 208, 9 },
  { 206, 6 },
  { 206, 5 },
  { 206, 8 },
  { 206, 7 },
  { 206, 8 },
  { 206, 7 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 5 },
  { 213, 3 },
  { 213, 1 },
  { 214, 1 },
  { 214, 3 },
  { 214, 2 },
  { 214, 4 },
  { 196, 5 },
  { 196, 6 },
  { 196, 6 },
  { 196, 7 },
  { 212, 3 },
  { 212, 1 },
  { 215, 3 },
  { 215, 1 },
  { 216, 1 },
  { 216, 1 },
  { 216, 1 },
  { 216, 1 },
  { 217, 1 },
  { 164, 1 },
  { 164, 1 },
  { 164, 1 },
  { 164, 1 },
  { 164, 1 },
  { 164, 1 },
  { 164, 1 },
  { 164, 3 },
  { 164, 1 },
  { 164, 2 },
  { 164, 3 },
  { 218, 3 },
  { 218, 1 },
  { 219, 3 },
  { 219, 1 },
  { 220, 1 },
  { 220, 1 },
  { 220, 1 },
  { 221, 1 },
  { 187, 1 },
  { 130, 1 },
  { 131, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  xx_ARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  // <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  // <lineno> <thisfile>
  **     break;
  */
      case 0:
// 1380 "parser.lemon"
{
	status->ret = yymsp[0].minor.yy78;
}
// 5490 "parser.cpp"
        break;
      case 1:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
      case 199:
      case 200:
      case 201:
      case 202:
      case 203:
      case 204:
      case 205:
      case 206:
      case 207:
      case 208:
      case 209:
      case 210:
      case 211:
      case 212:
      case 213:
      case 214:
      case 215:
      case 216:
      case 217:
      case 218:
      case 219:
      case 220:
      case 221:
      case 306:
      case 310:
      case 335:
      case 376:
      case 411:
      case 412:
      case 413:
      case 433:
      case 452:
      case 453:
// 1386 "parser.lemon"
{
	yygotominor.yy78 = yymsp[0].minor.yy78;
}
// 5556 "parser.cpp"
        break;
      case 2:
      case 68:
      case 84:
      case 86:
      case 88:
      case 126:
      case 197:
      case 236:
      case 242:
      case 256:
      case 295:
// 1390 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_list(yymsp[-1].minor.yy78, yymsp[0].minor.yy78);
}
// 5573 "parser.cpp"
        break;
      case 3:
      case 33:
      case 52:
      case 69:
      case 81:
      case 85:
      case 87:
      case 89:
      case 127:
      case 140:
      case 156:
      case 164:
      case 198:
      case 237:
      case 243:
      case 257:
      case 263:
      case 273:
      case 296:
      case 309:
      case 332:
      case 416:
      case 426:
      case 446:
// 1394 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_list(NULL, yymsp[0].minor.yy78);
}
// 5603 "parser.cpp"
        break;
      case 30:
// 1502 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_namespace(yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(43,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5612 "parser.cpp"
        break;
      case 31:
// 1506 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_use_aliases(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(46,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5621 "parser.cpp"
        break;
      case 32:
      case 51:
      case 80:
      case 163:
      case 272:
      case 308:
      case 331:
      case 415:
      case 425:
      case 445:
// 1510 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_list(yymsp[-2].minor.yy78, yymsp[0].minor.yy78);
  yy_destructor(6,&yymsp[-1].minor);
}
// 5638 "parser.cpp"
        break;
      case 34:
// 1518 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_use_aliases_item(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 5645 "parser.cpp"
        break;
      case 35:
// 1522 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_use_aliases_item(yymsp[-2].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
  yy_destructor(47,&yymsp[-1].minor);
}
// 5653 "parser.cpp"
        break;
      case 36:
// 1526 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_interface(yymsp[-1].minor.yy0, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(48,&yymsp[-2].minor);
}
// 5661 "parser.cpp"
        break;
      case 37:
// 1530 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_interface(yymsp[-3].minor.yy0, yymsp[0].minor.yy78, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(48,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5670 "parser.cpp"
        break;
      case 38:
// 1534 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy78, 0, 0, NULL, NULL, status->scanner_state);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5678 "parser.cpp"
        break;
      case 39:
// 1538 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy78, 0, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5687 "parser.cpp"
        break;
      case 40:
// 1542 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy78, 0, 0, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5696 "parser.cpp"
        break;
      case 41:
// 1546 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy78, 0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(50,&yymsp[-6].minor);
  yy_destructor(49,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5706 "parser.cpp"
        break;
      case 42:
// 1550 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy78, 1, 0, NULL, NULL, status->scanner_state);
  yy_destructor(52,&yymsp[-3].minor);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5715 "parser.cpp"
        break;
      case 43:
// 1554 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy78, 1, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(52,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5725 "parser.cpp"
        break;
      case 44:
// 1558 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy78, 1, 0, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(52,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5735 "parser.cpp"
        break;
      case 45:
// 1562 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy78, 1, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(52,&yymsp[-7].minor);
  yy_destructor(50,&yymsp[-6].minor);
  yy_destructor(49,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5746 "parser.cpp"
        break;
      case 46:
// 1566 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy78, 0, 1, NULL, NULL, status->scanner_state);
  yy_destructor(53,&yymsp[-3].minor);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5755 "parser.cpp"
        break;
      case 47:
// 1570 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy78, 0, 1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(53,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5765 "parser.cpp"
        break;
      case 48:
// 1574 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy78, 0, 1, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(53,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5775 "parser.cpp"
        break;
      case 49:
      case 78:
// 1578 "parser.lemon"
{
	yygotominor.yy78 = NULL;
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5785 "parser.cpp"
        break;
      case 50:
      case 79:
// 1582 "parser.lemon"
{
	yygotominor.yy78 = yymsp[-1].minor.yy78;
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5795 "parser.cpp"
        break;
      case 53:
      case 264:
      case 378:
      case 430:
      case 449:
// 1594 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state);
}
// 5806 "parser.cpp"
        break;
      case 54:
// 1598 "parser.lemon"
{
  yygotominor.yy78 = NULL;
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5815 "parser.cpp"
        break;
      case 55:
// 1602 "parser.lemon"
{
  yygotominor.yy78 = yymsp[-1].minor.yy78;
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5824 "parser.cpp"
        break;
      case 56:
// 1606 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_definition(yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
}
// 5831 "parser.cpp"
        break;
      case 57:
// 1610 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_definition(NULL, NULL, yymsp[0].minor.yy78, status->scanner_state);
}
// 5838 "parser.cpp"
        break;
      case 58:
// 1614 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_definition(NULL, yymsp[0].minor.yy78, NULL, status->scanner_state);
}
// 5845 "parser.cpp"
        break;
      case 59:
// 1618 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_definition(yymsp[-1].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
}
// 5852 "parser.cpp"
        break;
      case 60:
// 1622 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_definition(yymsp[-1].minor.yy78, NULL, yymsp[0].minor.yy78, status->scanner_state);
}
// 5859 "parser.cpp"
        break;
      case 61:
// 1626 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_definition(yymsp[0].minor.yy78, NULL, yymsp[-1].minor.yy78, status->scanner_state);
}
// 5866 "parser.cpp"
        break;
      case 62:
// 1630 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_definition(NULL, yymsp[0].minor.yy78, yymsp[-1].minor.yy78, status->scanner_state);
}
// 5873 "parser.cpp"
        break;
      case 63:
// 1634 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_definition(yymsp[-2].minor.yy78, yymsp[0].minor.yy78, yymsp[-1].minor.yy78, status->scanner_state);
}
// 5880 "parser.cpp"
        break;
      case 64:
// 1638 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_definition(yymsp[-1].minor.yy78, yymsp[0].minor.yy78, yymsp[-2].minor.yy78, status->scanner_state);
}
// 5887 "parser.cpp"
        break;
      case 65:
// 1642 "parser.lemon"
{
  yygotominor.yy78 = xx_ret_interface_definition(NULL, yymsp[0].minor.yy78, status->scanner_state);
}
// 5894 "parser.cpp"
        break;
      case 66:
// 1646 "parser.lemon"
{
  yygotominor.yy78 = xx_ret_interface_definition(yymsp[0].minor.yy78, NULL, status->scanner_state);
}
// 5901 "parser.cpp"
        break;
      case 67:
// 1650 "parser.lemon"
{
  yygotominor.yy78 = xx_ret_interface_definition(yymsp[0].minor.yy78, yymsp[-1].minor.yy78, status->scanner_state);
}
// 5908 "parser.cpp"
        break;
      case 70:
// 1663 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_property(yymsp[-2].minor.yy78, yymsp[-1].minor.yy0, NULL, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5916 "parser.cpp"
        break;
      case 71:
// 1667 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_property(yymsp[-2].minor.yy78, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5924 "parser.cpp"
        break;
      case 72:
// 1671 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_property(yymsp[-4].minor.yy78, yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, yymsp[-5].minor.yy0, NULL, status->scanner_state);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5933 "parser.cpp"
        break;
      case 73:
// 1675 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_property(yymsp[-4].minor.yy78, yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5942 "parser.cpp"
        break;
      case 74:
// 1679 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_property(yymsp[-3].minor.yy78, yymsp[-2].minor.yy0, NULL, yymsp[-4].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5950 "parser.cpp"
        break;
      case 75:
// 1683 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_property(yymsp[-3].minor.yy78, yymsp[-2].minor.yy0, NULL, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5958 "parser.cpp"
        break;
      case 76:
// 1687 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_property(yymsp[-5].minor.yy78, yymsp[-4].minor.yy0, yymsp[-2].minor.yy78, yymsp[-6].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(57,&yymsp[-3].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5967 "parser.cpp"
        break;
      case 77:
// 1691 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_property(yymsp[-5].minor.yy78, yymsp[-4].minor.yy0, yymsp[-2].minor.yy78, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(57,&yymsp[-3].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5976 "parser.cpp"
        break;
      case 82:
// 1711 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_property_shortcut(NULL, yymsp[0].minor.yy0, status->scanner_state);
}
// 5983 "parser.cpp"
        break;
      case 83:
// 1715 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_property_shortcut(yymsp[-1].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
}
// 5990 "parser.cpp"
        break;
      case 90:
      case 92:
// 1744 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, yymsp[-5].minor.yy0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6001 "parser.cpp"
        break;
      case 91:
      case 93:
// 1748 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6012 "parser.cpp"
        break;
      case 94:
// 1764 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-6].minor.yy78, yymsp[-4].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6024 "parser.cpp"
        break;
      case 95:
      case 122:
// 1769 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-5].minor.yy78, yymsp[-3].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6036 "parser.cpp"
        break;
      case 96:
// 1774 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-7].minor.yy78, yymsp[-5].minor.yy0, yymsp[-3].minor.yy78, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6048 "parser.cpp"
        break;
      case 97:
      case 123:
// 1779 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-6].minor.yy78, yymsp[-4].minor.yy0, yymsp[-2].minor.yy78, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6060 "parser.cpp"
        break;
      case 98:
// 1784 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-7].minor.yy78, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6072 "parser.cpp"
        break;
      case 99:
// 1788 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-8].minor.yy78, yymsp[-6].minor.yy0, yymsp[-4].minor.yy78, yymsp[-1].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6084 "parser.cpp"
        break;
      case 100:
// 1792 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-6].minor.yy78, yymsp[-4].minor.yy0, NULL, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6096 "parser.cpp"
        break;
      case 101:
      case 124:
// 1796 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-5].minor.yy78, yymsp[-3].minor.yy0, NULL, NULL, yymsp[-6].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6108 "parser.cpp"
        break;
      case 102:
// 1800 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-7].minor.yy78, yymsp[-5].minor.yy0, yymsp[-3].minor.yy78, NULL, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6120 "parser.cpp"
        break;
      case 103:
      case 125:
// 1804 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-6].minor.yy78, yymsp[-4].minor.yy0, yymsp[-2].minor.yy78, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6132 "parser.cpp"
        break;
      case 104:
// 1808 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-7].minor.yy78, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy78, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6144 "parser.cpp"
        break;
      case 105:
// 1812 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-8].minor.yy78, yymsp[-6].minor.yy0, yymsp[-4].minor.yy78, yymsp[-1].minor.yy78, yymsp[-9].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6156 "parser.cpp"
        break;
      case 106:
// 1816 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-8].minor.yy78, yymsp[-6].minor.yy0, NULL, NULL, NULL, yymsp[-2].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6169 "parser.cpp"
        break;
      case 107:
      case 118:
// 1820 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-7].minor.yy78, yymsp[-5].minor.yy0, NULL, NULL, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6182 "parser.cpp"
        break;
      case 108:
// 1824 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-9].minor.yy78, yymsp[-7].minor.yy0, yymsp[-5].minor.yy78, NULL, NULL, yymsp[-2].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6195 "parser.cpp"
        break;
      case 109:
      case 119:
// 1828 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-8].minor.yy78, yymsp[-6].minor.yy0, yymsp[-4].minor.yy78, NULL, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6208 "parser.cpp"
        break;
      case 110:
// 1832 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-9].minor.yy78, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy78, NULL, yymsp[-3].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6221 "parser.cpp"
        break;
      case 111:
// 1836 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-10].minor.yy78, yymsp[-8].minor.yy0, yymsp[-6].minor.yy78, yymsp[-1].minor.yy78, NULL, yymsp[-3].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-9].minor);
  yy_destructor(61,&yymsp[-7].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6234 "parser.cpp"
        break;
      case 112:
// 1840 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-8].minor.yy78, yymsp[-6].minor.yy0, NULL, NULL, yymsp[-9].minor.yy0, yymsp[-2].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6247 "parser.cpp"
        break;
      case 113:
      case 120:
// 1844 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-7].minor.yy78, yymsp[-5].minor.yy0, NULL, NULL, yymsp[-8].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6260 "parser.cpp"
        break;
      case 114:
// 1848 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-9].minor.yy78, yymsp[-7].minor.yy0, yymsp[-5].minor.yy78, NULL, yymsp[-10].minor.yy0, yymsp[-2].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6273 "parser.cpp"
        break;
      case 115:
      case 121:
// 1852 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-8].minor.yy78, yymsp[-6].minor.yy0, yymsp[-4].minor.yy78, NULL, yymsp[-9].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6286 "parser.cpp"
        break;
      case 116:
// 1856 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-9].minor.yy78, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy78, yymsp[-10].minor.yy0, yymsp[-3].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6299 "parser.cpp"
        break;
      case 117:
// 1860 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_class_method(yymsp[-10].minor.yy78, yymsp[-8].minor.yy0, yymsp[-6].minor.yy78, yymsp[-1].minor.yy78, yymsp[-11].minor.yy0, yymsp[-3].minor.yy78, status->scanner_state);
  yy_destructor(60,&yymsp[-9].minor);
  yy_destructor(61,&yymsp[-7].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6312 "parser.cpp"
        break;
      case 128:
// 1906 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("public");
  yy_destructor(1,&yymsp[0].minor);
}
// 6320 "parser.cpp"
        break;
      case 129:
// 1910 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("protected");
  yy_destructor(2,&yymsp[0].minor);
}
// 6328 "parser.cpp"
        break;
      case 130:
// 1914 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("private");
  yy_destructor(4,&yymsp[0].minor);
}
// 6336 "parser.cpp"
        break;
      case 131:
// 1918 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("static");
  yy_destructor(3,&yymsp[0].minor);
}
// 6344 "parser.cpp"
        break;
      case 132:
// 1922 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("scoped");
  yy_destructor(5,&yymsp[0].minor);
}
// 6352 "parser.cpp"
        break;
      case 133:
// 1926 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("inline");
  yy_destructor(62,&yymsp[0].minor);
}
// 6360 "parser.cpp"
        break;
      case 134:
// 1930 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("deprecated");
  yy_destructor(63,&yymsp[0].minor);
}
// 6368 "parser.cpp"
        break;
      case 135:
// 1934 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("abstract");
  yy_destructor(52,&yymsp[0].minor);
}
// 6376 "parser.cpp"
        break;
      case 136:
// 1938 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("final");
  yy_destructor(53,&yymsp[0].minor);
}
// 6384 "parser.cpp"
        break;
      case 137:
      case 153:
// 1943 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_type(1, NULL, status->scanner_state);
  yy_destructor(64,&yymsp[0].minor);
}
// 6393 "parser.cpp"
        break;
      case 138:
      case 154:
// 1947 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_type(0, yymsp[0].minor.yy78, status->scanner_state);
}
// 6401 "parser.cpp"
        break;
      case 139:
      case 155:
      case 262:
// 1951 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_list(yymsp[-2].minor.yy78, yymsp[0].minor.yy78);
  yy_destructor(14,&yymsp[-1].minor);
}
// 6411 "parser.cpp"
        break;
      case 141:
      case 157:
// 1959 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_type_item(yymsp[0].minor.yy78, NULL, 0, 0, status->scanner_state);
}
// 6419 "parser.cpp"
        break;
      case 142:
      case 158:
// 1963 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_NULL), NULL, 0, 0, status->scanner_state);
  yy_destructor(65,&yymsp[0].minor);
}
// 6428 "parser.cpp"
        break;
      case 143:
      case 159:
// 1967 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_THIS), NULL, 0, 0, status->scanner_state);
  yy_destructor(66,&yymsp[0].minor);
}
// 6437 "parser.cpp"
        break;
      case 144:
      case 160:
// 1971 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_type_item(yymsp[-1].minor.yy78, NULL, 1, 0, status->scanner_state);
  yy_destructor(39,&yymsp[0].minor);
}
// 6446 "parser.cpp"
        break;
      case 145:
      case 161:
// 1975 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy78, 0, 0, status->scanner_state);
}
// 6454 "parser.cpp"
        break;
      case 146:
      case 162:
// 1979 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy78, 0, 1, status->scanner_state);
}
// 6462 "parser.cpp"
        break;
      case 147:
// 1986 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_function_definition(NULL, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6474 "parser.cpp"
        break;
      case 148:
// 1991 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_function_definition(NULL, yymsp[-5].minor.yy0, yymsp[-3].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6486 "parser.cpp"
        break;
      case 149:
// 1996 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_function_definition(NULL, yymsp[-6].minor.yy0, yymsp[-4].minor.yy78, yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6498 "parser.cpp"
        break;
      case 150:
// 2001 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_function_definition(yymsp[-7].minor.yy78, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6510 "parser.cpp"
        break;
      case 151:
// 2006 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_function_definition(yymsp[-7].minor.yy78, yymsp[-5].minor.yy0, yymsp[-3].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6522 "parser.cpp"
        break;
      case 152:
// 2011 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_function_definition(yymsp[-8].minor.yy78, yymsp[-6].minor.yy0, yymsp[-4].minor.yy78, yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6534 "parser.cpp"
        break;
      case 165:
// 2065 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(0, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6541 "parser.cpp"
        break;
      case 166:
// 2069 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(1, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-1].minor);
}
// 6549 "parser.cpp"
        break;
      case 167:
// 2073 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(0, yymsp[-1].minor.yy78, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6556 "parser.cpp"
        break;
      case 168:
// 2077 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(1, yymsp[-1].minor.yy78, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-2].minor);
}
// 6564 "parser.cpp"
        break;
      case 169:
// 2081 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(0, yymsp[-2].minor.yy78, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(39,&yymsp[-1].minor);
}
// 6572 "parser.cpp"
        break;
      case 170:
// 2085 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(1, yymsp[-2].minor.yy78, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(58,&yymsp[-3].minor);
  yy_destructor(39,&yymsp[-1].minor);
}
// 6581 "parser.cpp"
        break;
      case 171:
// 2089 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(0, NULL, yymsp[-1].minor.yy78, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6588 "parser.cpp"
        break;
      case 172:
// 2093 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(1, NULL, yymsp[-1].minor.yy78, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-2].minor);
}
// 6596 "parser.cpp"
        break;
      case 173:
// 2097 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(0, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy78, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6604 "parser.cpp"
        break;
      case 174:
// 2101 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(1, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy78, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6613 "parser.cpp"
        break;
      case 175:
// 2105 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(0, yymsp[-3].minor.yy78, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy78, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6621 "parser.cpp"
        break;
      case 176:
// 2109 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(1, yymsp[-3].minor.yy78, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy78, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6630 "parser.cpp"
        break;
      case 177:
// 2113 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(0, yymsp[-4].minor.yy78, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy78, 1, status->scanner_state);
  yy_destructor(39,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6639 "parser.cpp"
        break;
      case 178:
// 2117 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(1, yymsp[-4].minor.yy78, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy78, 1, status->scanner_state);
  yy_destructor(58,&yymsp[-5].minor);
  yy_destructor(39,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6649 "parser.cpp"
        break;
      case 179:
// 2121 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(0, NULL, yymsp[-3].minor.yy78, yymsp[-2].minor.yy0, yymsp[0].minor.yy78, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6657 "parser.cpp"
        break;
      case 180:
// 2125 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_parameter(1, NULL, yymsp[-3].minor.yy78, yymsp[-2].minor.yy0, yymsp[0].minor.yy78, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6666 "parser.cpp"
        break;
      case 181:
// 2130 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(21,&yymsp[-2].minor);
  yy_destructor(22,&yymsp[0].minor);
}
// 6675 "parser.cpp"
        break;
      case 182:
// 2134 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state);
  yy_destructor(21,&yymsp[-4].minor);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[-1].minor);
  yy_destructor(22,&yymsp[0].minor);
}
// 6686 "parser.cpp"
        break;
      case 183:
// 2138 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_INTEGER);
  yy_destructor(68,&yymsp[0].minor);
}
// 6694 "parser.cpp"
        break;
      case 184:
// 2142 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_UINTEGER);
  yy_destructor(69,&yymsp[0].minor);
}
// 6702 "parser.cpp"
        break;
      case 185:
// 2146 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_LONG);
  yy_destructor(70,&yymsp[0].minor);
}
// 6710 "parser.cpp"
        break;
      case 186:
// 2150 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_ULONG);
  yy_destructor(71,&yymsp[0].minor);
}
// 6718 "parser.cpp"
        break;
      case 187:
// 2154 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_CHAR);
  yy_destructor(72,&yymsp[0].minor);
}
// 6726 "parser.cpp"
        break;
      case 188:
// 2158 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_UCHAR);
  yy_destructor(73,&yymsp[0].minor);
}
// 6734 "parser.cpp"
        break;
      case 189:
// 2162 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_DOUBLE);
  yy_destructor(74,&yymsp[0].minor);
}
// 6742 "parser.cpp"
        break;
      case 190:
// 2166 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_BOOL);
  yy_destructor(75,&yymsp[0].minor);
}
// 6750 "parser.cpp"
        break;
      case 191:
// 2170 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_STRING);
  yy_destructor(76,&yymsp[0].minor);
}
// 6758 "parser.cpp"
        break;
      case 192:
// 2174 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_ARRAY);
  yy_destructor(77,&yymsp[0].minor);
}
// 6766 "parser.cpp"
        break;
      case 193:
// 2178 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_VAR);
  yy_destructor(78,&yymsp[0].minor);
}
// 6774 "parser.cpp"
        break;
      case 194:
// 2182 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_CALLABLE);
  yy_destructor(79,&yymsp[0].minor);
}
// 6782 "parser.cpp"
        break;
      case 195:
// 2186 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_RESOURCE);
  yy_destructor(80,&yymsp[0].minor);
}
// 6790 "parser.cpp"
        break;
      case 196:
// 2190 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_type(XX_TYPE_OBJECT);
  yy_destructor(81,&yymsp[0].minor);
}
// 6798 "parser.cpp"
        break;
      case 222:
// 2294 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_empty_statement(status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 6806 "parser.cpp"
        break;
      case 223:
// 2298 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_break_statement(status->scanner_state);
  yy_destructor(82,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6815 "parser.cpp"
        break;
      case 224:
// 2302 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_continue_statement(status->scanner_state);
  yy_destructor(83,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6824 "parser.cpp"
        break;
      case 225:
// 2307 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-2].minor.yy78, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6834 "parser.cpp"
        break;
      case 226:
// 2312 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-3].minor.yy78, NULL, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(84,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6844 "parser.cpp"
        break;
      case 227:
// 2317 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-5].minor.yy78, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6857 "parser.cpp"
        break;
      case 228:
// 2322 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-6].minor.yy78, NULL, yymsp[-3].minor.yy78, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6870 "parser.cpp"
        break;
      case 229:
// 2327 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-3].minor.yy78, yymsp[-1].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6880 "parser.cpp"
        break;
      case 230:
// 2332 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-4].minor.yy78, yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-3].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6890 "parser.cpp"
        break;
      case 231:
// 2337 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-7].minor.yy78, yymsp[-5].minor.yy78, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(84,&yymsp[-8].minor);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6903 "parser.cpp"
        break;
      case 232:
// 2342 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-8].minor.yy78, yymsp[-6].minor.yy78, yymsp[-4].minor.yy78, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(84,&yymsp[-9].minor);
  yy_destructor(54,&yymsp[-7].minor);
  yy_destructor(55,&yymsp[-5].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6916 "parser.cpp"
        break;
      case 233:
// 2347 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-6].minor.yy78, yymsp[-4].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6929 "parser.cpp"
        break;
      case 234:
// 2352 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-7].minor.yy78, yymsp[-5].minor.yy78, yymsp[-3].minor.yy78, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-8].minor);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6942 "parser.cpp"
        break;
      case 235:
// 2357 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-6].minor.yy78, NULL, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6955 "parser.cpp"
        break;
      case 238:
// 2370 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-2].minor.yy78, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(86,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6965 "parser.cpp"
        break;
      case 239:
// 2375 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_if_statement(yymsp[-3].minor.yy78, yymsp[-1].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(86,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6975 "parser.cpp"
        break;
      case 240:
// 2379 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_switch_statement(yymsp[-2].minor.yy78, NULL, status->scanner_state);
  yy_destructor(87,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6985 "parser.cpp"
        break;
      case 241:
// 2383 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_switch_statement(yymsp[-3].minor.yy78, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(87,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6995 "parser.cpp"
        break;
      case 244:
// 2395 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_case_clause(yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(88,&yymsp[-2].minor);
  yy_destructor(89,&yymsp[0].minor);
}
// 7004 "parser.cpp"
        break;
      case 245:
// 2399 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_case_clause(yymsp[-2].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(88,&yymsp[-3].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 7013 "parser.cpp"
        break;
      case 246:
// 2403 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_case_clause(NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(90,&yymsp[-2].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 7022 "parser.cpp"
        break;
      case 247:
// 2407 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_loop_statement(NULL, status->scanner_state);
  yy_destructor(91,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7032 "parser.cpp"
        break;
      case 248:
// 2411 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_loop_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(91,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7042 "parser.cpp"
        break;
      case 249:
// 2415 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_while_statement(yymsp[-2].minor.yy78, NULL, status->scanner_state);
  yy_destructor(92,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7052 "parser.cpp"
        break;
      case 250:
// 2419 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_while_statement(yymsp[-3].minor.yy78, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(92,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7062 "parser.cpp"
        break;
      case 251:
// 2423 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_do_while_statement(yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(93,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(92,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7074 "parser.cpp"
        break;
      case 252:
// 2427 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_do_while_statement(yymsp[-1].minor.yy78, yymsp[-4].minor.yy78, status->scanner_state);
  yy_destructor(93,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(92,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7086 "parser.cpp"
        break;
      case 253:
// 2431 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_try_catch_statement(NULL, NULL, status->scanner_state);
  yy_destructor(94,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7096 "parser.cpp"
        break;
      case 254:
// 2435 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_try_catch_statement(yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(94,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7106 "parser.cpp"
        break;
      case 255:
// 2439 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_try_catch_statement(yymsp[-2].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(94,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-3].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 7116 "parser.cpp"
        break;
      case 258:
// 2451 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_catch_statement(yymsp[-3].minor.yy78, NULL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(95,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7126 "parser.cpp"
        break;
      case 259:
// 2455 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_catch_statement(yymsp[-2].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(95,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7136 "parser.cpp"
        break;
      case 260:
// 2459 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_catch_statement(yymsp[-4].minor.yy78, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(95,&yymsp[-5].minor);
  yy_destructor(6,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7147 "parser.cpp"
        break;
      case 261:
// 2463 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_catch_statement(yymsp[-5].minor.yy78, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state), yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(95,&yymsp[-6].minor);
  yy_destructor(6,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7158 "parser.cpp"
        break;
      case 265:
// 2479 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_for_statement(yymsp[-3].minor.yy78, NULL, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(96,&yymsp[-6].minor);
  yy_destructor(97,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7169 "parser.cpp"
        break;
      case 266:
// 2483 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_for_statement(yymsp[-2].minor.yy78, NULL, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(96,&yymsp[-5].minor);
  yy_destructor(97,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7180 "parser.cpp"
        break;
      case 267:
// 2487 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_for_statement(yymsp[-3].minor.yy78, NULL, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(96,&yymsp[-7].minor);
  yy_destructor(97,&yymsp[-5].minor);
  yy_destructor(98,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7192 "parser.cpp"
        break;
      case 268:
// 2491 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_for_statement(yymsp[-3].minor.yy78, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(96,&yymsp[-8].minor);
  yy_destructor(6,&yymsp[-6].minor);
  yy_destructor(97,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7204 "parser.cpp"
        break;
      case 269:
// 2495 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_for_statement(yymsp[-2].minor.yy78, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(96,&yymsp[-7].minor);
  yy_destructor(6,&yymsp[-5].minor);
  yy_destructor(97,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7216 "parser.cpp"
        break;
      case 270:
// 2499 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_for_statement(yymsp[-3].minor.yy78, yymsp[-8].minor.yy0, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(96,&yymsp[-9].minor);
  yy_destructor(6,&yymsp[-7].minor);
  yy_destructor(97,&yymsp[-5].minor);
  yy_destructor(98,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7229 "parser.cpp"
        break;
      case 271:
// 2503 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(99,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7238 "parser.cpp"
        break;
      case 274:
// 2516 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("assign");
  yy_destructor(57,&yymsp[0].minor);
}
// 7246 "parser.cpp"
        break;
      case 275:
// 2521 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("add-assign");
  yy_destructor(100,&yymsp[0].minor);
}
// 7254 "parser.cpp"
        break;
      case 276:
// 2526 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("sub-assign");
  yy_destructor(101,&yymsp[0].minor);
}
// 7262 "parser.cpp"
        break;
      case 277:
// 2530 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("mul-assign");
  yy_destructor(102,&yymsp[0].minor);
}
// 7270 "parser.cpp"
        break;
      case 278:
// 2534 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("div-assign");
  yy_destructor(103,&yymsp[0].minor);
}
// 7278 "parser.cpp"
        break;
      case 279:
// 2538 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("concat-assign");
  yy_destructor(104,&yymsp[0].minor);
}
// 7286 "parser.cpp"
        break;
      case 280:
// 2542 "parser.lemon"
{
	yygotominor.yy78 = new Json::Value("mod-assign");
  yy_destructor(105,&yymsp[0].minor);
}
// 7294 "parser.cpp"
        break;
      case 281:
// 2547 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("variable", yymsp[-1].minor.yy78, yymsp[-2].minor.yy0, NULL, NULL, yymsp[0].minor.yy78, status->scanner_state);
}
// 7301 "parser.cpp"
        break;
      case 282:
// 2552 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("object-property", yymsp[-1].minor.yy78, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
}
// 7309 "parser.cpp"
        break;
      case 283:
// 2557 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("variable-dynamic-object-property", yymsp[-1].minor.yy78, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7319 "parser.cpp"
        break;
      case 284:
// 2562 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("string-dynamic-object-property", yymsp[-1].minor.yy78, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7329 "parser.cpp"
        break;
      case 285:
// 2567 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("object-property-append", yymsp[-1].minor.yy78, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7339 "parser.cpp"
        break;
      case 286:
// 2572 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("object-property-array-index", yymsp[-1].minor.yy78, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(42,&yymsp[-4].minor);
}
// 7347 "parser.cpp"
        break;
      case 287:
// 2576 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("object-property-array-index-append", yymsp[-1].minor.yy78, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7357 "parser.cpp"
        break;
      case 288:
// 2581 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("static-property", yymsp[-1].minor.yy78, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(107,&yymsp[-3].minor);
}
// 7365 "parser.cpp"
        break;
      case 289:
// 2586 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("static-property-append", yymsp[-1].minor.yy78, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(107,&yymsp[-5].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7375 "parser.cpp"
        break;
      case 290:
// 2591 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("static-property-array-index", yymsp[-1].minor.yy78, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(107,&yymsp[-4].minor);
}
// 7383 "parser.cpp"
        break;
      case 291:
// 2596 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("static-property-array-index-append", yymsp[-1].minor.yy78, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(107,&yymsp[-6].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7393 "parser.cpp"
        break;
      case 292:
// 2601 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("variable-append", yymsp[-1].minor.yy78, yymsp[-4].minor.yy0, NULL, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7402 "parser.cpp"
        break;
      case 293:
// 2606 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("array-index", yymsp[-1].minor.yy78, yymsp[-3].minor.yy0, NULL, yymsp[-2].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
}
// 7409 "parser.cpp"
        break;
      case 294:
// 2611 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("array-index-append", yymsp[-1].minor.yy78, yymsp[-5].minor.yy0, NULL, yymsp[-4].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7418 "parser.cpp"
        break;
      case 297:
// 2623 "parser.lemon"
{
	yygotominor.yy78 = yymsp[-1].minor.yy78;
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7427 "parser.cpp"
        break;
      case 298:
// 2628 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("object-property-incr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(108,&yymsp[0].minor);
}
// 7436 "parser.cpp"
        break;
      case 299:
// 2633 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("object-property-decr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(109,&yymsp[0].minor);
}
// 7445 "parser.cpp"
        break;
      case 300:
// 2638 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("incr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(108,&yymsp[0].minor);
}
// 7453 "parser.cpp"
        break;
      case 301:
// 2643 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("incr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(108,&yymsp[-1].minor);
}
// 7461 "parser.cpp"
        break;
      case 302:
// 2648 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("decr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(109,&yymsp[0].minor);
}
// 7469 "parser.cpp"
        break;
      case 303:
// 2653 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("decr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(109,&yymsp[-1].minor);
}
// 7477 "parser.cpp"
        break;
      case 304:
// 2658 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("dynamic-variable", yymsp[-1].minor.yy78, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7486 "parser.cpp"
        break;
      case 305:
// 2663 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_let_assignment("dynamic-variable-string", yymsp[-1].minor.yy78, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7495 "parser.cpp"
        break;
      case 307:
// 2671 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_echo_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(110,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7504 "parser.cpp"
        break;
      case 311:
// 2688 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_mcall_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7512 "parser.cpp"
        break;
      case 312:
// 2693 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_fcall_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7520 "parser.cpp"
        break;
      case 313:
// 2698 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_scall_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7528 "parser.cpp"
        break;
      case 314:
// 2703 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_fetch_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7536 "parser.cpp"
        break;
      case 315:
// 2708 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(111,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7545 "parser.cpp"
        break;
      case 316:
// 2713 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_return_statement(NULL, status->scanner_state);
  yy_destructor(111,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7554 "parser.cpp"
        break;
      case 317:
// 2718 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_require_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(7,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7563 "parser.cpp"
        break;
      case 318:
// 2723 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_unset_statement(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(112,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7572 "parser.cpp"
        break;
      case 319:
// 2728 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_throw_exception(yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(113,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7581 "parser.cpp"
        break;
      case 320:
// 2732 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_INTEGER, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(68,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7590 "parser.cpp"
        break;
      case 321:
// 2736 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_UINTEGER, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(69,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7599 "parser.cpp"
        break;
      case 322:
// 2740 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_CHAR, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(72,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7608 "parser.cpp"
        break;
      case 323:
// 2744 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_UCHAR, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(73,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7617 "parser.cpp"
        break;
      case 324:
// 2748 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_LONG, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(70,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7626 "parser.cpp"
        break;
      case 325:
// 2752 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_ULONG, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(71,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7635 "parser.cpp"
        break;
      case 326:
// 2756 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_DOUBLE, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(74,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7644 "parser.cpp"
        break;
      case 327:
// 2760 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_STRING, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(76,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7653 "parser.cpp"
        break;
      case 328:
// 2764 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_BOOL, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(75,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7662 "parser.cpp"
        break;
      case 329:
// 2768 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_VAR, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(78,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7671 "parser.cpp"
        break;
      case 330:
// 2772 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_statement(XX_T_TYPE_ARRAY, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(77,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7680 "parser.cpp"
        break;
      case 333:
// 2784 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_variable(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 7687 "parser.cpp"
        break;
      case 334:
// 2788 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_declare_variable(yymsp[-2].minor.yy0, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 7695 "parser.cpp"
        break;
      case 336:
// 2796 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("not", yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(39,&yymsp[-1].minor);
}
// 7703 "parser.cpp"
        break;
      case 337:
// 2800 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("minus", yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(28,&yymsp[-1].minor);
}
// 7711 "parser.cpp"
        break;
      case 338:
// 2804 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("isset", yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(33,&yymsp[-1].minor);
}
// 7719 "parser.cpp"
        break;
      case 339:
// 2808 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("require", yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(7,&yymsp[-1].minor);
}
// 7727 "parser.cpp"
        break;
      case 340:
// 2812 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("clone", yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(37,&yymsp[-1].minor);
}
// 7735 "parser.cpp"
        break;
      case 341:
// 2816 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("empty", yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(35,&yymsp[-1].minor);
}
// 7743 "parser.cpp"
        break;
      case 342:
// 2820 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("likely", yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(9,&yymsp[-1].minor);
}
// 7751 "parser.cpp"
        break;
      case 343:
// 2824 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("unlikely", yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(10,&yymsp[-1].minor);
}
// 7759 "parser.cpp"
        break;
      case 344:
// 2828 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("equals", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(19,&yymsp[-1].minor);
}
// 7767 "parser.cpp"
        break;
      case 345:
// 2832 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("not-equals", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(26,&yymsp[-1].minor);
}
// 7775 "parser.cpp"
        break;
      case 346:
// 2836 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("identical", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(20,&yymsp[-1].minor);
}
// 7783 "parser.cpp"
        break;
      case 347:
// 2840 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("not-identical", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(25,&yymsp[-1].minor);
}
// 7791 "parser.cpp"
        break;
      case 348:
// 2844 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("less", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(21,&yymsp[-1].minor);
}
// 7799 "parser.cpp"
        break;
      case 349:
// 2848 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("greater", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(22,&yymsp[-1].minor);
}
// 7807 "parser.cpp"
        break;
      case 350:
// 2852 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("less-equal", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(23,&yymsp[-1].minor);
}
// 7815 "parser.cpp"
        break;
      case 351:
// 2856 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("greater-equal", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(24,&yymsp[-1].minor);
}
// 7823 "parser.cpp"
        break;
      case 352:
// 2860 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("list", yymsp[-1].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7832 "parser.cpp"
        break;
      case 353:
// 2864 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("cast", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
}
// 7841 "parser.cpp"
        break;
      case 354:
// 2868 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("type-hint", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(21,&yymsp[-3].minor);
  yy_destructor(22,&yymsp[-1].minor);
}
// 7850 "parser.cpp"
        break;
      case 355:
// 2872 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("property-access", yymsp[-2].minor.yy78, xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-1].minor);
}
// 7858 "parser.cpp"
        break;
      case 356:
// 2876 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("property-dynamic-access", yymsp[-4].minor.yy78, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7868 "parser.cpp"
        break;
      case 357:
// 2880 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("property-string-access", yymsp[-4].minor.yy78, xx_ret_literal(XX_T_STRING, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7878 "parser.cpp"
        break;
      case 358:
// 2884 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("static-property-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-1].minor);
}
// 7886 "parser.cpp"
        break;
      case 359:
      case 441:
// 2888 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("static-constant-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-1].minor);
}
// 7895 "parser.cpp"
        break;
      case 360:
// 2897 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("array-access", yymsp[-3].minor.yy78, yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7904 "parser.cpp"
        break;
      case 361:
// 2902 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("add", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(27,&yymsp[-1].minor);
}
// 7912 "parser.cpp"
        break;
      case 362:
// 2907 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("sub", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(28,&yymsp[-1].minor);
}
// 7920 "parser.cpp"
        break;
      case 363:
// 2912 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("mul", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(30,&yymsp[-1].minor);
}
// 7928 "parser.cpp"
        break;
      case 364:
// 2917 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("div", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(31,&yymsp[-1].minor);
}
// 7936 "parser.cpp"
        break;
      case 365:
// 2922 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("mod", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(32,&yymsp[-1].minor);
}
// 7944 "parser.cpp"
        break;
      case 366:
// 2927 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("concat", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(29,&yymsp[-1].minor);
}
// 7952 "parser.cpp"
        break;
      case 367:
// 2932 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("and", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(13,&yymsp[-1].minor);
}
// 7960 "parser.cpp"
        break;
      case 368:
// 2937 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("or", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(12,&yymsp[-1].minor);
}
// 7968 "parser.cpp"
        break;
      case 369:
// 2942 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("bitwise_and", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(15,&yymsp[-1].minor);
}
// 7976 "parser.cpp"
        break;
      case 370:
// 2947 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("bitwise_or", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(14,&yymsp[-1].minor);
}
// 7984 "parser.cpp"
        break;
      case 371:
// 2952 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("bitwise_xor", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(16,&yymsp[-1].minor);
}
// 7992 "parser.cpp"
        break;
      case 372:
// 2957 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("bitwise_shiftleft", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(17,&yymsp[-1].minor);
}
// 8000 "parser.cpp"
        break;
      case 373:
// 2962 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("bitwise_shiftright", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(18,&yymsp[-1].minor);
}
// 8008 "parser.cpp"
        break;
      case 374:
// 2967 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("instanceof", yymsp[-2].minor.yy78, yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(11,&yymsp[-1].minor);
}
// 8016 "parser.cpp"
        break;
      case 375:
// 2972 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("fetch", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy78, NULL, status->scanner_state);
  yy_destructor(34,&yymsp[-3].minor);
  yy_destructor(6,&yymsp[-1].minor);
}
// 8025 "parser.cpp"
        break;
      case 377:
// 2982 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("typeof", yymsp[0].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(36,&yymsp[-1].minor);
}
// 8033 "parser.cpp"
        break;
      case 379:
      case 432:
      case 434:
      case 451:
// 2992 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_INTEGER, yymsp[0].minor.yy0, status->scanner_state);
}
// 8043 "parser.cpp"
        break;
      case 380:
      case 431:
      case 436:
      case 450:
// 2997 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_STRING, yymsp[0].minor.yy0, status->scanner_state);
}
// 8053 "parser.cpp"
        break;
      case 381:
      case 435:
// 3002 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_CHAR, yymsp[0].minor.yy0, status->scanner_state);
}
// 8061 "parser.cpp"
        break;
      case 382:
      case 437:
// 3007 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_DOUBLE, yymsp[0].minor.yy0, status->scanner_state);
}
// 8069 "parser.cpp"
        break;
      case 383:
      case 438:
// 3012 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_NULL, NULL, status->scanner_state);
  yy_destructor(65,&yymsp[0].minor);
}
// 8078 "parser.cpp"
        break;
      case 384:
      case 440:
// 3017 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_TRUE, NULL, status->scanner_state);
  yy_destructor(117,&yymsp[0].minor);
}
// 8087 "parser.cpp"
        break;
      case 385:
      case 439:
// 3022 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_FALSE, NULL, status->scanner_state);
  yy_destructor(118,&yymsp[0].minor);
}
// 8096 "parser.cpp"
        break;
      case 386:
      case 429:
      case 442:
// 3027 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_literal(XX_T_CONSTANT, yymsp[0].minor.yy0, status->scanner_state);
}
// 8105 "parser.cpp"
        break;
      case 387:
      case 443:
// 3032 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("empty-array", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-1].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 8115 "parser.cpp"
        break;
      case 388:
      case 444:
// 3037 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("array", yymsp[-1].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 8125 "parser.cpp"
        break;
      case 389:
// 3042 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_new_instance(0, yymsp[0].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-1].minor);
}
// 8133 "parser.cpp"
        break;
      case 390:
// 3047 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_new_instance(0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8143 "parser.cpp"
        break;
      case 391:
// 3052 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_new_instance(0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(38,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8153 "parser.cpp"
        break;
      case 392:
// 3057 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_new_instance(1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8163 "parser.cpp"
        break;
      case 393:
// 3062 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_new_instance(1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8175 "parser.cpp"
        break;
      case 394:
// 3067 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_new_instance(1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(38,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8187 "parser.cpp"
        break;
      case 395:
// 3072 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_fcall(1, yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8196 "parser.cpp"
        break;
      case 396:
// 3077 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_fcall(1, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8205 "parser.cpp"
        break;
      case 397:
// 3082 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_fcall(2, yymsp[-4].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8216 "parser.cpp"
        break;
      case 398:
// 3087 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_fcall(2, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8227 "parser.cpp"
        break;
      case 399:
// 3092 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_scall(0, yymsp[-5].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(107,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8237 "parser.cpp"
        break;
      case 400:
// 3097 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_scall(0, yymsp[-4].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8247 "parser.cpp"
        break;
      case 401:
// 3102 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_scall(1, yymsp[-6].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(54,&yymsp[-7].minor);
  yy_destructor(55,&yymsp[-5].minor);
  yy_destructor(107,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8259 "parser.cpp"
        break;
      case 402:
// 3107 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_scall(1, yymsp[-5].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(107,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8271 "parser.cpp"
        break;
      case 403:
// 3112 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_scall(1, yymsp[-8].minor.yy0, 1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(54,&yymsp[-9].minor);
  yy_destructor(55,&yymsp[-7].minor);
  yy_destructor(107,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8285 "parser.cpp"
        break;
      case 404:
// 3117 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_scall(1, yymsp[-7].minor.yy0, 1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-8].minor);
  yy_destructor(55,&yymsp[-6].minor);
  yy_destructor(107,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8299 "parser.cpp"
        break;
      case 405:
// 3122 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_mcall(1, yymsp[-5].minor.yy78, yymsp[-3].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8309 "parser.cpp"
        break;
      case 406:
// 3127 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_mcall(1, yymsp[-4].minor.yy78, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8319 "parser.cpp"
        break;
      case 407:
// 3132 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_mcall(2, yymsp[-7].minor.yy78, yymsp[-4].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8331 "parser.cpp"
        break;
      case 408:
// 3137 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_mcall(2, yymsp[-6].minor.yy78, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8343 "parser.cpp"
        break;
      case 409:
// 3142 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_mcall(3, yymsp[-7].minor.yy78, yymsp[-4].minor.yy0, yymsp[-1].minor.yy78, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8355 "parser.cpp"
        break;
      case 410:
// 3147 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_mcall(3, yymsp[-6].minor.yy78, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8367 "parser.cpp"
        break;
      case 414:
// 3167 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("ternary", yymsp[-4].minor.yy78, yymsp[-2].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(8,&yymsp[-3].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 8376 "parser.cpp"
        break;
      case 417:
// 3180 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy78, status->scanner_state, 0);
}
// 8383 "parser.cpp"
        break;
      case 418:
// 3185 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_call_parameter(yymsp[-2].minor.yy0, yymsp[0].minor.yy78, status->scanner_state, 0);
  yy_destructor(89,&yymsp[-1].minor);
}
// 8391 "parser.cpp"
        break;
      case 419:
// 3190 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy78, status->scanner_state, 1);
  yy_destructor(15,&yymsp[-1].minor);
}
// 8399 "parser.cpp"
        break;
      case 420:
// 3195 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_call_parameter(yymsp[-3].minor.yy0, yymsp[0].minor.yy78, status->scanner_state, 0);
  yy_destructor(89,&yymsp[-2].minor);
  yy_destructor(15,&yymsp[-1].minor);
}
// 8408 "parser.cpp"
        break;
      case 421:
// 3200 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("closure", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8420 "parser.cpp"
        break;
      case 422:
// 3205 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("closure", NULL, yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8432 "parser.cpp"
        break;
      case 423:
// 3210 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("closure", yymsp[-3].minor.yy78, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8444 "parser.cpp"
        break;
      case 424:
// 3215 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_expr("closure", yymsp[-4].minor.yy78, yymsp[-1].minor.yy78, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8456 "parser.cpp"
        break;
      case 427:
      case 447:
// 3227 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_array_item(yymsp[-2].minor.yy78, yymsp[0].minor.yy78, status->scanner_state);
  yy_destructor(89,&yymsp[-1].minor);
}
// 8465 "parser.cpp"
        break;
      case 428:
      case 448:
// 3231 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_array_item(NULL, yymsp[0].minor.yy78, status->scanner_state);
}
// 8473 "parser.cpp"
        break;
      case 454:
// 3336 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_comment(yymsp[0].minor.yy0, status->scanner_state);
}
// 8480 "parser.cpp"
        break;
      case 455:
// 3340 "parser.lemon"
{
	yygotominor.yy78 = xx_ret_cblock(yymsp[0].minor.yy0, status->scanner_state);
}
// 8487 "parser.cpp"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yypParser,yygoto);
  if( yyact < YYNSTATE ){
    yy_shift(yypParser,yyact,yygoto,&yygotominor);
  }else if( yyact == YYNSTATE + YYNRULE + 1 ){
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  xx_ARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  xx_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  xx_ARG_FETCH;
#define TOKEN (yyminor.yy0)
// 1303 "parser.lemon"

	Json::Value* syntax_error = new Json::Value();
	(*syntax_error)["type"] = "error";

	if (status->scanner_state->start_length) {
		(*syntax_error)["message"] = "Syntax error";

		const xx_token_names *tokens = xx_tokens;
		std::string token_name;

		if (status->scanner_state->active_token) {
			do {
				if (tokens->code == status->scanner_state->active_token) {
					token_name = tokens->name;
					break;
				}
				++tokens;
			} while (tokens[0].code != 0);
		}

		if (token_name.empty()) {
			token_name = "UNKNOWN";
		}

		std::ostringstream oss;
		oss << "Syntax error, ";

		if (status->scanner_state->start_length > 0) {
			oss << "unexpected token " << token_name;
			if (!status->token->value.empty()) {
				oss << "(" << status->token->value << "), near to '" << status->scanner_state->start;
				oss << "' in " << status->scanner_state->active_file << " on line " << status->scanner_state->active_line;
			} else {
				oss << ", near to '" << status->scanner_state->start << "' in " << status->scanner_state->active_file;
				oss << " on line " << status->scanner_state->active_line;
			}
		} else {
			if (status->scanner_state->active_token != XX_T_IGNORE) {
				if (!status->token->value.empty()) {
					oss << "unexpected token %s" << token_name << "(" << status->token->value << "), ";
					oss << "at the end of docblock in " << status->scanner_state->active_file << " on line " << status->scanner_state->active_line;
				} else {
					oss << "unexpected token %s" << token_name << "), ";
					oss << "at the end of docblock in " << status->scanner_state->active_file << " on line " << status->scanner_state->active_line;
				}
			} else {
				oss << "unexpected EOF, at the end of docblock in " << status->scanner_state->active_file << " on line " << status->scanner_state->active_line;
			}
		}

		status->syntax_error = oss.str();
		status->syntax_error_len = status->syntax_error.length();
	} else {
		(*syntax_error)["message"] = "Unexpected EOF";

		status->syntax_error = "Syntax error, unexpected EOF in ";
		status->syntax_error.append(status->scanner_state->active_file);
		status->syntax_error_len = status->syntax_error.length();
	}

	(*syntax_error)["file"] = status->scanner_state->active_file;
	(*syntax_error)["line"] = status->scanner_state->active_line;
	(*syntax_error)["char"] = status->scanner_state->active_char;

	status->status = XX_PARSING_FAILED;
	status->ret = syntax_error;

// 8597 "parser.cpp"
  xx_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  xx_ARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  xx_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "xx_Alloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void xx_(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  xx_TOKENTYPE yyminor       /* The value for the token */
  xx_ARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
    if( yymajor==0 ) return;
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  xx_ARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      if( yyendofinput && yypParser->yyidx>=0 ){
        yymajor = 0;
      }else{
        yymajor = YYNOCODE;
      }
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else if( yyact == YY_ERROR_ACTION ){
      int yymx;
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_shift_action(yypParser,YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }else{
      yy_accept(yypParser);
      yymajor = YYNOCODE;
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}

/*
 +--------------------------------------------------------------------------+
 | Zephir Language                                                          |
 +--------------------------------------------------------------------------+
 | Copyright (c) 2013-2014 Zephir Team and contributors                          |
 +--------------------------------------------------------------------------+
 | This source file is subject the MIT license, that is bundled with        |
 | this package in the file LICENSE, and is available through the           |
 | world-wide-web at the following url:                                     |
 | http://zephir-lang.com/license.html                                      |
 |                                                                          |
 | If you did not receive a copy of the MIT license and are unable          |
 | to obtain it through the world-wide-web, please send a note to           |
 | license@zephir-lang.com so we can mail you a copy immediately.           |
 +--------------------------------------------------------------------------+
*/

#define SUCCESS 1
#define FAILURE 0

const xx_token_names xx_tokens[] =
{
	{ XX_T_INTEGER,             "INTEGER" },
	{ XX_T_DOUBLE,              "DOUBLE" },
	{ XX_T_STRING,              "STRING" },
	{ XX_T_IDENTIFIER,    		"IDENTIFIER" },
	{ XX_T_AT,                  "@" },
	{ XX_T_COMMA,               "," },
	{ XX_T_ASSIGN,              "=" },
	{ XX_T_COLON,               ":" },
	{ XX_T_PARENTHESES_OPEN,    "(" },
	{ XX_T_PARENTHESES_CLOSE,   ")" },
	{ XX_T_BRACKET_OPEN,        "{" },
	{ XX_T_BRACKET_CLOSE,       "}" },
 	{ XX_T_SBRACKET_OPEN,       "[" },
	{ XX_T_SBRACKET_CLOSE,      "]" },
	{  0, NULL }
};

/**
 * Wrapper to alloc memory within the parser
 */
static void *xx_wrapper_alloc(size_t bytes){
	return malloc(bytes);
}

/**
 * Wrapper to free memory within the parser
 */
static void xx_wrapper_free(void *pointer){
	free(pointer);
}

/**
 * Creates a parser_token to be passed to the parser
 */
static void xx_parse_with_token(void* xx_parser, int opcode, int parsercode, xx_scanner_token *token, xx_parser_status *parser_status){

	xx_parser_token *pToken;

	pToken = new xx_parser_token();
	pToken->opcode = opcode;;
	pToken->token = token->value;
	pToken->token_len = token->len;
	pToken->free_flag = 1;

	xx_(xx_parser, parsercode, pToken, parser_status);

	token->value.clear();
	token->len = 0;
}

/**
 * Parses a comment returning an intermediate array representation
 */
Json::Value xx_parse_program(char *program, unsigned int program_length, char *file_path) {

	Json::Value ret;
	xx_scanner_state *state;
	xx_scanner_token token;
	int scanner_status, status = SUCCESS;
	xx_parser_status *parser_status = NULL;
	void* xx_parser;

	/**
	 * Check if the program has any length
	 */
	if (!program_length || program_length < 2) {
		return ret;
	}

	/**
	 * Start the reentrant parser
	 */
	xx_parser = xx_Alloc(xx_wrapper_alloc);

	parser_status = new xx_parser_status();
	state = new xx_scanner_state();

	parser_status->status = XX_PARSING_OK;
	parser_status->scanner_state = state;
	parser_status->ret = NULL;
	parser_status->token = &token;
	parser_status->syntax_error.clear();
	parser_status->number_brackets = 0;

	/**
	 * Initialize the scanner state
	 */
	state->active_token = 0;
	state->start = program;
	state->start_length = 0;
	state->active_file = file_path;
	state->active_line = 1;
	state->active_char = 1;

	state->end = state->start;

	while (0 <= (scanner_status = xx_get_token(state, &token))) {

		state->active_token = token.opcode;

		state->start_length = (program + program_length - state->start);

		switch (token.opcode) {

			case XX_T_IGNORE:
				break;

			case XX_T_NAMESPACE:
				xx_(xx_parser, XX_NAMESPACE, NULL, parser_status);
				break;
			case XX_T_ABSTRACT:
				xx_(xx_parser, XX_ABSTRACT, NULL, parser_status);
				break;
			case XX_T_CLASS:
				xx_(xx_parser, XX_CLASS, NULL, parser_status);
				break;
			case XX_T_INTERFACE:
				xx_(xx_parser, XX_INTERFACE, NULL, parser_status);
				break;
			case XX_T_EXTENDS:
				xx_(xx_parser, XX_EXTENDS, NULL, parser_status);
				break;
			case XX_T_IMPLEMENTS:
				xx_(xx_parser, XX_IMPLEMENTS, NULL, parser_status);
				break;
			case XX_T_PUBLIC:
				xx_(xx_parser, XX_PUBLIC, NULL, parser_status);
				break;
			case XX_T_PROTECTED:
				xx_(xx_parser, XX_PROTECTED, NULL, parser_status);
				break;
			case XX_T_PRIVATE:
				xx_(xx_parser, XX_PRIVATE, NULL, parser_status);
				break;
			case XX_T_STATIC:
				xx_(xx_parser, XX_STATIC, NULL, parser_status);
				break;
			case XX_T_INLINE:
				xx_(xx_parser, XX_INLINE, NULL, parser_status);
				break;
			case XX_T_DEPRECATED:
				xx_(xx_parser, XX_DEPRECATED, NULL, parser_status);
				break;
			case XX_T_FINAL:
				xx_(xx_parser, XX_FINAL, NULL, parser_status);
				break;
			case XX_T_FUNCTION:
				xx_(xx_parser, XX_FUNCTION, NULL, parser_status);
				break;
			case XX_T_LET:
				xx_(xx_parser, XX_LET, NULL, parser_status);
				break;
			case XX_T_ECHO:
				xx_(xx_parser, XX_ECHO, NULL, parser_status);
				break;
			case XX_T_RETURN:
				xx_(xx_parser, XX_RETURN, NULL, parser_status);
				break;
			case XX_T_REQUIRE:
				xx_(xx_parser, XX_REQUIRE, NULL, parser_status);
				break;
			case XX_T_CLONE:
				xx_(xx_parser, XX_CLONE, NULL, parser_status);
				break;
			case XX_T_EMPTY:
				xx_(xx_parser, XX_EMPTY, NULL, parser_status);
				break;
			case XX_T_IF:
				xx_(xx_parser, XX_IF, NULL, parser_status);
				break;
			case XX_T_ELSE:
				xx_(xx_parser, XX_ELSE, NULL, parser_status);
				break;
			case XX_T_ELSEIF:
				xx_(xx_parser, XX_ELSEIF, NULL, parser_status);
				break;
			case XX_T_LOOP:
				xx_(xx_parser, XX_LOOP, NULL, parser_status);
				break;
			case XX_T_CONTINUE:
				xx_(xx_parser, XX_CONTINUE, NULL, parser_status);
				break;
			case XX_T_BREAK:
				xx_(xx_parser, XX_BREAK, NULL, parser_status);
				break;
			case XX_T_WHILE:
				xx_(xx_parser, XX_WHILE, NULL, parser_status);
				break;
			case XX_T_DO:
				xx_(xx_parser, XX_DO, NULL, parser_status);
				break;
			case XX_T_NEW:
				xx_(xx_parser, XX_NEW, NULL, parser_status);
				break;
			case XX_T_CONST:
				xx_(xx_parser, XX_CONST, NULL, parser_status);
				break;
			case XX_T_TYPEOF:
				xx_(xx_parser, XX_TYPEOF, NULL, parser_status);
				break;
			case XX_T_INSTANCEOF:
				xx_(xx_parser, XX_INSTANCEOF, NULL, parser_status);
				break;
			case XX_T_ISSET:
				xx_(xx_parser, XX_ISSET, NULL, parser_status);
				break;
			case XX_T_UNSET:
				xx_(xx_parser, XX_UNSET, NULL, parser_status);
				break;
			case XX_T_THROW:
				xx_(xx_parser, XX_THROW, NULL, parser_status);
				break;
			case XX_T_FOR:
				xx_(xx_parser, XX_FOR, NULL, parser_status);
				break;
			case XX_T_IN:
				xx_(xx_parser, XX_IN, NULL, parser_status);
				break;
			case XX_T_REVERSE:
				xx_(xx_parser, XX_REVERSE, NULL, parser_status);
				break;
			case XX_T_USE:
				xx_(xx_parser, XX_USE, NULL, parser_status);
				break;
			case XX_T_AS:
				xx_(xx_parser, XX_AS, NULL, parser_status);
				break;
			case XX_T_TRY:
				xx_(xx_parser, XX_TRY, NULL, parser_status);
				break;
			case XX_T_CATCH:
				xx_(xx_parser, XX_CATCH, NULL, parser_status);
				break;

			case XX_T_DOTCOMMA:
				xx_(xx_parser, XX_DOTCOMMA, NULL, parser_status);
				break;
			case XX_T_COMMA:
				xx_(xx_parser, XX_COMMA, NULL, parser_status);
				break;
			case XX_T_ASSIGN:
				xx_(xx_parser, XX_ASSIGN, NULL, parser_status);
				break;
			case XX_T_ADDASSIGN:
				xx_(xx_parser, XX_ADDASSIGN, NULL, parser_status);
				break;
			case XX_T_SUBASSIGN:
				xx_(xx_parser, XX_SUBASSIGN, NULL, parser_status);
				break;
			case XX_T_DIVASSIGN:
				xx_(xx_parser, XX_DIVASSIGN, NULL, parser_status);
				break;
			case XX_T_MULASSIGN:
				xx_(xx_parser, XX_MULASSIGN, NULL, parser_status);
				break;
			case XX_T_CONCATASSIGN:
				xx_(xx_parser, XX_CONCATASSIGN, NULL, parser_status);
				break;
			case XX_T_MODASSIGN:
				xx_(xx_parser, XX_MODASSIGN, NULL, parser_status);
				break;
			case XX_T_EQUALS:
				xx_(xx_parser, XX_EQUALS, NULL, parser_status);
				break;
			case XX_T_NOTEQUALS:
				xx_(xx_parser, XX_NOTEQUALS, NULL, parser_status);
				break;
			case XX_T_IDENTICAL:
				xx_(xx_parser, XX_IDENTICAL, NULL, parser_status);
				break;
			case XX_T_NOTIDENTICAL:
				xx_(xx_parser, XX_NOTIDENTICAL, NULL, parser_status);
				break;
			case XX_T_LESS:
				xx_(xx_parser, XX_LESS, NULL, parser_status);
				break;
			case XX_T_GREATER:
				xx_(xx_parser, XX_GREATER, NULL, parser_status);
				break;
			case XX_T_LESSEQUAL:
				xx_(xx_parser, XX_LESSEQUAL, NULL, parser_status);
				break;
			case XX_T_GREATEREQUAL:
				xx_(xx_parser, XX_GREATEREQUAL, NULL, parser_status);
				break;
			case XX_T_QUESTION:
				xx_(xx_parser, XX_QUESTION, NULL, parser_status);
				break;
			case XX_T_COLON:
				xx_(xx_parser, XX_COLON, NULL, parser_status);
				break;
			case XX_T_ARROW:
				xx_(xx_parser, XX_ARROW, NULL, parser_status);
				break;
			case XX_T_DOUBLECOLON:
				xx_(xx_parser, XX_DOUBLECOLON, NULL, parser_status);
				break;
			case XX_T_NOT:
				xx_(xx_parser, XX_NOT, NULL, parser_status);
				break;
			case XX_T_FETCH:
				xx_(xx_parser, XX_FETCH, NULL, parser_status);
				break;
			case XX_T_SWITCH:
				xx_(xx_parser, XX_SWITCH, NULL, parser_status);
				break;
			case XX_T_CASE:
				xx_(xx_parser, XX_CASE, NULL, parser_status);
				break;
			case XX_T_DEFAULT:
				xx_(xx_parser, XX_DEFAULT, NULL, parser_status);
				break;

			case XX_T_PARENTHESES_OPEN:
				xx_(xx_parser, XX_PARENTHESES_OPEN, NULL, parser_status);
				break;
			case XX_T_PARENTHESES_CLOSE:
				xx_(xx_parser, XX_PARENTHESES_CLOSE, NULL, parser_status);
				break;

			case XX_T_BRACKET_OPEN:
				parser_status->number_brackets++;
				xx_(xx_parser, XX_BRACKET_OPEN, NULL, parser_status);
				break;
			case XX_T_BRACKET_CLOSE:
				parser_status->number_brackets--;
				xx_(xx_parser, XX_BRACKET_CLOSE, NULL, parser_status);
				break;

			case XX_T_SBRACKET_OPEN:
				xx_(xx_parser, XX_SBRACKET_OPEN, NULL, parser_status);
				break;
			case XX_T_SBRACKET_CLOSE:
				xx_(xx_parser, XX_SBRACKET_CLOSE, NULL, parser_status);
				break;

			case XX_T_NULL:
				xx_(xx_parser, XX_NULL, NULL, parser_status);
				break;
			case XX_T_TRUE:
				xx_(xx_parser, XX_TRUE, NULL, parser_status);
				break;
			case XX_T_FALSE:
				xx_(xx_parser, XX_FALSE, NULL, parser_status);
				break;
			case XX_T_COMMENT:
				if (parser_status->number_brackets <= 1) {
					xx_parse_with_token(xx_parser, XX_T_COMMENT, XX_COMMENT, &token, parser_status);
				}
				break;
			case XX_T_CBLOCK:
				xx_parse_with_token(xx_parser, XX_T_CBLOCK, XX_CBLOCK, &token, parser_status);
				break;
			case XX_T_TYPE_INTEGER:
				xx_(xx_parser, XX_TYPE_INTEGER, NULL, parser_status);
				break;
			case XX_T_TYPE_UINTEGER:
				xx_(xx_parser, XX_TYPE_UINTEGER, NULL, parser_status);
				break;
			case XX_T_TYPE_CHAR:
				xx_(xx_parser, XX_TYPE_CHAR, NULL, parser_status);
				break;
			case XX_T_TYPE_UCHAR:
				xx_(xx_parser, XX_TYPE_UCHAR, NULL, parser_status);
				break;
			case XX_T_TYPE_LONG:
				xx_(xx_parser, XX_TYPE_LONG, NULL, parser_status);
				break;
			case XX_T_TYPE_ULONG:
				xx_(xx_parser, XX_TYPE_ULONG, NULL, parser_status);
				break;
			case XX_T_TYPE_DOUBLE:
				xx_(xx_parser, XX_TYPE_DOUBLE, NULL, parser_status);
				break;
			case XX_T_TYPE_STRING:
				xx_(xx_parser, XX_TYPE_STRING, NULL, parser_status);
				break;
			case XX_T_TYPE_BOOL:
				xx_(xx_parser, XX_TYPE_BOOL, NULL, parser_status);
				break;
			case XX_T_TYPE_ARRAY:
				xx_(xx_parser, XX_TYPE_ARRAY, NULL, parser_status);
				break;
			case XX_T_TYPE_VAR:
				xx_(xx_parser, XX_TYPE_VAR, NULL, parser_status);
				break;
			case XX_T_TYPE_OBJECT:
				xx_(xx_parser, XX_TYPE_OBJECT, NULL, parser_status);
				break;
			case XX_T_TYPE_RESOURCE:
				xx_(xx_parser, XX_TYPE_RESOURCE, NULL, parser_status);
				break;
			case XX_T_TYPE_CALLABLE:
				xx_(xx_parser, XX_TYPE_CALLABLE, NULL, parser_status);
				break;

			case XX_T_ADD:
				xx_(xx_parser, XX_ADD, NULL, parser_status);
				break;
			case XX_T_SUB:
				xx_(xx_parser, XX_SUB, NULL, parser_status);
				break;
			case XX_T_MUL:
				xx_(xx_parser, XX_MUL, NULL, parser_status);
				break;
			case XX_T_DIV:
				xx_(xx_parser, XX_DIV, NULL, parser_status);
				break;
			case XX_T_MOD:
				xx_(xx_parser, XX_MOD, NULL, parser_status);
				break;
			case XX_T_DOT:
				xx_(xx_parser, XX_CONCAT, NULL, parser_status);
				break;
			case XX_T_INCR:
				xx_(xx_parser, XX_INCR, NULL, parser_status);
				break;
			case XX_T_DECR:
				xx_(xx_parser, XX_DECR, NULL, parser_status);
				break;
			case XX_T_AND:
				xx_(xx_parser, XX_AND, NULL, parser_status);
				break;
			case XX_T_OR:
				xx_(xx_parser, XX_OR, NULL, parser_status);
				break;
			case XX_T_BITWISE_AND:
				xx_(xx_parser, XX_BITWISE_AND, NULL, parser_status);
				break;
			case XX_T_BITWISE_OR:
				xx_(xx_parser, XX_BITWISE_OR, NULL, parser_status);
				break;
			case XX_T_BITWISE_XOR:
				xx_(xx_parser, XX_BITWISE_XOR, NULL, parser_status);
				break;
			case XX_T_BITWISE_SHIFTLEFT:
				xx_(xx_parser, XX_BITWISE_SHIFTLEFT, NULL, parser_status);
				break;
			case XX_T_BITWISE_SHIFTRIGHT:
				xx_(xx_parser, XX_BITWISE_SHIFTRIGHT, NULL, parser_status);
				break;
			case XX_T_INTEGER:
				xx_parse_with_token(xx_parser, XX_T_INTEGER, XX_INTEGER, &token, parser_status);
				break;
			case XX_T_DOUBLE:
				xx_parse_with_token(xx_parser, XX_T_DOUBLE, XX_DOUBLE, &token, parser_status);
				break;
			case XX_T_STRING:
				xx_parse_with_token(xx_parser, XX_T_STRING, XX_STRING, &token, parser_status);
				break;
			case XX_T_CHAR:
				xx_parse_with_token(xx_parser, XX_T_CHAR, XX_CHAR, &token, parser_status);
				break;
			case XX_T_IDENTIFIER:
				xx_parse_with_token(xx_parser, XX_T_IDENTIFIER, XX_IDENTIFIER, &token, parser_status);
				break;
			case XX_T_CONSTANT:
				xx_parse_with_token(xx_parser, XX_T_CONSTANT, XX_CONSTANT, &token, parser_status);
				break;

			case XX_T_VOID:
				xx_(xx_parser, XX_VOID, NULL, parser_status);
				break;
			case XX_T_LIKELY:
				xx_(xx_parser, XX_LIKELY, NULL, parser_status);
				break;
			case XX_T_UNLIKELY:
				xx_(xx_parser, XX_UNLIKELY, NULL, parser_status);
				break;

			default:
				parser_status->status = XX_PARSING_FAILED;
				std::cerr << "Scanner: unknown opcode " << token.opcode << std::endl;
				break;
		}

		if (parser_status->status != XX_PARSING_OK) {
			status = FAILURE;
			break;
		}

		state->end = state->start;
	}

	if (status != FAILURE) {
		switch (scanner_status) {
			case XX_SCANNER_RETCODE_ERR:
			case XX_SCANNER_RETCODE_IMPOSSIBLE:
				{
					if (state->start) {
						std::cerr << "Scanner error: " << scanner_status << " " << state->start << std::endl;
					} else {
						std::cerr << "Scanner error: " << scanner_status << std::endl;
					}
					status = FAILURE;
				}
				break;
			default:
				xx_(xx_parser, 0, NULL, parser_status);
		}
	}

	state->active_token = 0;
	state->start = NULL;

	if (parser_status->status == XX_PARSING_FAILED) {
		status = FAILURE;
		if (!parser_status->syntax_error.empty()) {
			std::cerr << "Error: " << parser_status->syntax_error << std::endl;
		} else {
			std::cerr << "Error" <<                                                                                                                                                             std::endl;
		}
	}

	xx_Free(xx_parser, xx_wrapper_free);

	if (status != FAILURE) {
		if (parser_status->status == XX_PARSING_OK) {
			if (parser_status->ret) {
				ret = *(parser_status->ret);
			}
		}
	}

	if (parser_status->ret) {
		delete parser_status->ret;
	}

	delete parser_status;
	delete state;

	return ret;
}
