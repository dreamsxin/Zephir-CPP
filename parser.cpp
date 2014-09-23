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
#define YYNOCODE 220
#define YYACTIONTYPE unsigned short int
#define xx_TOKENTYPE xx_parser_token*
typedef union {
  xx_TOKENTYPE yy0;
  Json::Value* yy396;
  int yy439;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define xx_ARG_SDECL xx_parser_status *status;
#define xx_ARG_PDECL ,xx_parser_status *status
#define xx_ARG_FETCH xx_parser_status *status = yypParser->status
#define xx_ARG_STORE yypParser->status = status
#define YYNSTATE 922
#define YYNRULE 446
#define YYERRORSYMBOL 120
#define YYERRSYMDT yy439
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
 /*     0 */   445,  215,   62,   65,  737,   43,   36,  739,  841,  847,
 /*    10 */   134,  846,  831,  186,  128,   73,   69,  725,  807,  237,
 /*    20 */   245,   47,  565,  702,  825,  603,   50,  139,   59,  144,
 /*    30 */    56,  164,   44,  137,  154,  820,  826,  132,  276,  677,
 /*    40 */   678,  680,  679,  681,  733,   35,  851,  171,  564,  494,
 /*    50 */   670,  133,  153,  184,  108,  214,  237,  245,  150,  565,
 /*    60 */   922,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*    70 */   490,  487,  696,   41,   31,  277,  279,  281,  237,  245,
 /*    80 */   291,  565,  222,  230,  302,  306,  311,  319,   39,  330,
 /*    90 */   684,  685,  337,  639,  705,  294,  715,  376,   34,  147,
 /*   100 */   682,  683,   43,  426,  441,  448,  451,  146,  148,  149,
 /*   110 */   151,  152,  495,  445,   32,   62,   65,   76,   78,   86,
 /*   120 */    80,   82,   84,  237,  245,  352,  188,  128,  298,  351,
 /*   130 */   535,   73,   69,  215,   47,  178,  180,  179,  143,   50,
 /*   140 */   139,   59,  144,   56,  164,   44,  752,  154,  630,  751,
 /*   150 */   132,  276,  677,  678,  680,  679,  681,   38,  747,  380,
 /*   160 */   171,  563,  494,  759,   43,  153,  184,  108,  226,  237,
 /*   170 */   245,  150,  565,  223,  454,  463,  472,  475,  466,  469,
 /*   180 */   478,  484,  481,  490,  487,  211,  212,  216,  277,  279,
 /*   190 */   281,   70, 1342,  291,   37,  547,  541,  302,  306,  311,
 /*   200 */   319,  607,  330,  684,  685,  337,  987,  705,  365,  715,
 /*   210 */   133,  371,  147,  682,  683,   43,  426,  441,  448,  451,
 /*   220 */   146,  148,  149,  151,  152,  495,  445,  806,   62,   65,
 /*   230 */   742,  350,   72,  403,  372,  703,  807,  296,  165,  163,
 /*   240 */   128,  702,  178,  180,  179,  143,  459,   47,  585,  627,
 /*   250 */   181,  517,   50,  139,   59,  144,   56,  164,   44,  523,
 /*   260 */   154,  133, 1095,  132,  276,  677,  678,  680,  679,  681,
 /*   270 */   298,  354,  535,  171,  553,  494,  791,   39,  153,  184,
 /*   280 */   108,  375,  237,  245,  150,  565,  210,  454,  463,  472,
 /*   290 */   475,  466,  469,  478,  484,  481,  490,  487,  742,  327,
 /*   300 */   162,  277,  279,  281,  741,  355,  291,  364,  372,  702,
 /*   310 */   302,  306,  311,  319,   40,  330,  684,  685,  337,  736,
 /*   320 */   810,  386,  715,  392,  372,  147,  682,  683,  174,  426,
 /*   330 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*   340 */   766,   62,   65,  765,  572,   72,  503,  209,  656,  657,
 /*   350 */   661,  662,  761,  128,  577,  178,  180,  179,  143,  859,
 /*   360 */    47,  865,  604,  181,  735,   50,  139,   59,  144,   56,
 /*   370 */   164,   44,  784,  154,  571,  783,  132,  276,  677,  678,
 /*   380 */   680,  679,  681,  645,  779,  823,  171,  285,  494,  885,
 /*   390 */   644,  153,  184,  108,  702,  237,  245,  150,  565,   71,
 /*   400 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*   410 */   487,  687,  241,  129,  277,  279,  281,  238, 1361,  291,
 /*   420 */   286,  552,  702,  302,  306,  311,  319,  126,  330,  684,
 /*   430 */   685,  337,  978,  810,  822,  715,  200,  332,  147,  682,
 /*   440 */   683,  236,  426,  441,  448,  451,  146,  148,  149,  151,
 /*   450 */   152,  495,  445,  170,   62,   65,  899,  340,   72,  729,
 /*   460 */   665,  852,  237,  245,  735,  565,  128,  635,  178,  180,
 /*   470 */   179,  143,  638,   47,  130,  600,  181,  170,   50,  139,
 /*   480 */    59,  144,   56,  164,   44,  798,  154,  169,  797,  132,
 /*   490 */   276,  677,  678,  680,  679,  681,  339,  793,  135,  171,
 /*   500 */   539,  494,  287,  541,  153,  184,  108,  295,  538,  393,
 /*   510 */   150,  176,  371,  454,  463,  472,  475,  466,  469,  478,
 /*   520 */   484,  481,  490,  487,  323,  527,  140,  277,  279,  281,
 /*   530 */   326,  526,  291,  141,  429,  187,  302,  306,  311,  319,
 /*   540 */   166,  330,  684,  685,  337,  982,  808,  404,  715,  417,
 /*   550 */   371,  147,  682,  683,  608,  426,  441,  448,  451,  146,
 /*   560 */   148,  149,  151,  152,  495,  445,  813,   62,   65,  249,
 /*   570 */  1344,   72,  872,  428,  875,  819,  457,  735,  814,  128,
 /*   580 */   172,  178,  180,  179,  143,  457,   47,  650,  168,  181,
 /*   590 */   669,   50,  139,   59,  144,   56,  164,   44, 1343,  154,
 /*   600 */   338,  425,  132,  276,  677,  678,  680,  679,  681,  455,
 /*   610 */   462,  421,  171,  290,  494,  456,  614,  153,  184,  108,
 /*   620 */   464,  462,  672,  150,  465,  675,  454,  463,  472,  475,
 /*   630 */   466,  469,  478,  484,  481,  490,  487,  467,  462, 1341,
 /*   640 */   277,  279,  281,  457,  457,  291,  470,  462,  457,  302,
 /*   650 */   306,  311,  319,  173,  330,  684,  685,  337,  979,  810,
 /*   660 */   185,  715,  473,  462,  147,  682,  683,  190,  426,  441,
 /*   670 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  833,
 /*   680 */    62,   65,  468,  471,   72,  476,  462,  474,  819,  457,
 /*   690 */   457,  834,  128,  200,  178,  180,  179,  143,  457,   47,
 /*   700 */   692,  175,  181,  695,   50,  139,   59,  144,   56,  164,
 /*   710 */    44,  848,  154,  479,  462,  132,  276,  677,  678,  680,
 /*   720 */   679,  681,  482,  462,  191,  171,  534,  494,  477,  480,
 /*   730 */   153,  184,  108,  485,  462,  457,  150,  483,  201,  454,
 /*   740 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*   750 */   342,  711,  343,  277,  279,  281,  488,  462,  291,  735,
 /*   760 */   416,  825,  302,  306,  311,  319,  707,  330,  684,  685,
 /*   770 */   337,  989,  686,  826,  486,  491,  462,  147,  682,  683,
 /*   780 */  1363,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*   790 */   495,  445,  758,   62,   65,  344,  345,  346,  347,  348,
 /*   800 */   349,  754,  457,  457,  651,  128,  654,  668,  657,  661,
 /*   810 */   662, 1362,   47,  698,  412,  414,  701,   50,  139,   59,
 /*   820 */   144,   56,  164,   44,  213,  154,  558,  541,  132,  276,
 /*   830 */   677,  678,  680,  679,  681,   80,   82,   84,  171,  305,
 /*   840 */   494,  489,  492,  153,  184,  108,   73,   69,  220,  150,
 /*   850 */   554,  552,  454,  463,  472,  475,  466,  469,  478,  484,
 /*   860 */   481,  490,  487,  555,  541,  221,  277,  279,  281,  224,
 /*   870 */   732,  291,  720,  735,  170,  302,  306,  311,  319,  772,
 /*   880 */   330,  684,  685,  337,  988,  686,  862,  716,  768,  735,
 /*   890 */   147,  682,  683,  227,  426,  441,  448,  451,  146,  148,
 /*   900 */   149,  151,  152,  495,  445,  170,   62,   65,  575,  225,
 /*   910 */   673,  431,  654,  668,  657,  661,  662,  774,  128,  427,
 /*   920 */   432,  178,  180,  179,  143,   47,  170,  170,  702,  170,
 /*   930 */    50,  139,   59,  144,   56,  164,   44,  811,  154,  582,
 /*   940 */   790,  132,  276,  677,  678,  680,  679,  681,  702,  786,
 /*   950 */   804,  171,  533,  494,  821,  828,  153,  184,  108,  800,
 /*   960 */   591,  601,  150,  605,  228,  454,  463,  472,  475,  466,
 /*   970 */   469,  478,  484,  481,  490,  487,  816,  836,  229,  277,
 /*   980 */   279,  281,  231,  232,  291,  819,  819,  170,  302,  306,
 /*   990 */   311,  319,  235,  330,  684,  685,  337,  981,  773,  853,
 /*  1000 */   863,  851,  851,  147,  682,  683,  459,  426,  441,  448,
 /*  1010 */   451,  146,  148,  149,  151,  152,  495,  445,  170,   62,
 /*  1020 */    65,  612, 1096,   72,  233,  236,  811,  170,  187,  239,
 /*  1030 */   866,  128,  851,  178,  180,  179,  143,  702,   47,  187,
 /*  1040 */   574,  181,  187,   50,  139,   59,  144,   56,  164,   44,
 /*  1050 */   240,  154,  618,  242,  132,  276,  677,  678,  680,  679,
 /*  1060 */   681,  628,  671,  243,  171,  310,  494,  244,  246,  153,
 /*  1070 */   184,  108,  876,  886,  851,  150,  697,  248,  454,  463,
 /*  1080 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  215,
 /*  1090 */   278,  247,  277,  279,  281,  250,  459,  291,  280,  297,
 /*  1100 */   187,  302,  306,  311,  319,  283,  330,  684,  685,  337,
 /*  1110 */   985,  773, 1097,  540,  288,  293,  147,  682,  683,  459,
 /*  1120 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  1130 */   445,  187,   62,   65,  760, 1098,   72,  299,  300,  303,
 /*  1140 */   208,  217,  212,  216,  128,  308,  178,  180,  179,  143,
 /*  1150 */   312,   47,  317,  581,  181,  187,   50,  139,   59,  144,
 /*  1160 */    56,  164,   44,  315,  154,  792,  320,  132,  276,  677,
 /*  1170 */   678,  680,  679,  681,  459,  325,  324,  171,  529,  494,
 /*  1180 */   525,  331,  153,  184,  108,  341,  459,  459,  150,  900,
 /*  1190 */  1099,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  1200 */   490,  487, 1100, 1101,  282,  277,  279,  281,  362,  359,
 /*  1210 */   291,  369,  812,  318,  302,  306,  311,  319,  377,  330,
 /*  1220 */   684,  685,  337,  178,  180,  179,  143,  378,  775,  147,
 /*  1230 */   682,  683,  459,  426,  441,  448,  451,  146,  148,  149,
 /*  1240 */   151,  152,  495,  445,  381,   62,   65,  459, 1102,   72,
 /*  1250 */   382,  385,  459,  390,  397,  401,  413,  128,  408,  178,
 /*  1260 */   180,  179,  143, 1103,   47,  415,  590,  181, 1104,   50,
 /*  1270 */   139,   59,  144,   56,  164,   44,  434,  154,  459,  418,
 /*  1280 */   132,  276,  677,  678,  680,  679,  681,  422,  419,  423,
 /*  1290 */   171,  314,  494,  436, 1105,  153,  184,  108,  438,  440,
 /*  1300 */   459,  150,  460,  458,  454,  463,  472,  475,  466,  469,
 /*  1310 */   478,  484,  481,  490,  487,  461,  504,  292,  277,  279,
 /*  1320 */   281,  518,  505,  291,  519,  832,  318,  302,  306,  311,
 /*  1330 */   319,  524,  330,  684,  685,  337,  178,  180,  179,  143,
 /*  1340 */   532,  743,  147,  682,  683,  543,  426,  441,  448,  451,
 /*  1350 */   146,  148,  149,  151,  152,  495,  445,  530,   62,   65,
 /*  1360 */   536,  548,   72,  556,  573,  557,  559,  567,  578,  579,
 /*  1370 */   128,  586,  178,  180,  179,  143,  580,   47,  587,  611,
 /*  1380 */   181,  588,   50,  139,   59,  144,   56,  164,   44,  594,
 /*  1390 */   154,  609,  610,  132,  276,  677,  678,  680,  679,  681,
 /*  1400 */   615,  616,  631,  171,  528,  494,  633,  634,  153,  184,
 /*  1410 */   108,  636,  637,  638,  150,  641,  676,  454,  463,  472,
 /*  1420 */   475,  466,  469,  478,  484,  481,  490,  487,  647,  648,
 /*  1430 */   652,  277,  279,  281,  655,  658,  291,  664,  666,  667,
 /*  1440 */   302,  306,  311,  319,  674,  330,  684,  685,  337,  983,
 /*  1450 */   844,  689,  690,  694,  700,  147,  682,  683,  708,  426,
 /*  1460 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  1470 */   709,   62,   65,  710,  714,   72,  712,  727,  717,  719,
 /*  1480 */   713,  718,  721,  128,  723,  178,  180,  179,  143,  722,
 /*  1490 */    47,  728,  617,  181,  730,   50,  139,   59,  144,   56,
 /*  1500 */   164,   44,  850,  154,  731,  734,  132,  276,  677,  678,
 /*  1510 */   680,  679,  681,  744,  738,  745,  171,  322,  494,  776,
 /*  1520 */   829,  153,  184,  108,  777,  815,  818,  150,  817, 1000,
 /*  1530 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  1540 */   487, 1001,  824,  827,  277,  279,  281,  830,  839,  291,
 /*  1550 */   835,  838,  837,  302,  306,  311,  319,  840,  330,  684,
 /*  1560 */   685,  337,  986,  773,  854,  856,  849,  857,  147,  682,
 /*  1570 */   683,  858,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  1580 */   152,  495,  445,  860,   62,   65,  869,  870,  861,  640,
 /*  1590 */   864,  642,  873,  724,  867,  645,  128,  871,  735,  874,
 /*  1600 */   725,  877,  704,   47,  879,  882,  702,  880,   50,  139,
 /*  1610 */    59,  144,   56,  164,   44,  887,  154,  892,  893,  132,
 /*  1620 */   276,  677,  678,  680,  679,  681,  894,  896,  667,  171,
 /*  1630 */   516,  494,  901,  667,  153,  184,  108,  667,  667,  667,
 /*  1640 */   150,  667,  667,  454,  463,  472,  475,  466,  469,  478,
 /*  1650 */   484,  481,  490,  487,  667,  667,  667,  277,  279,  281,
 /*  1660 */   667,  667,  291,  667,  667,  667,  302,  306,  311,  319,
 /*  1670 */   667,  330,  684,  685,  337,  984,  773,  667,  667,  667,
 /*  1680 */   667,  147,  682,  683,  667,  426,  441,  448,  451,  146,
 /*  1690 */   148,  149,  151,  152,  495,  445,  667,   62,   65,  667,
 /*  1700 */   667,  667,  667,  667,  805,  740,  667,  809,  831,  128,
 /*  1710 */   667,  667,  667,  725,  807,  667,   47,  667,  667,  702,
 /*  1720 */   667,   50,  139,   59,  144,   56,  164,   44,  667,  154,
 /*  1730 */   667,  667,  132,  276,  677,  678,  680,  679,  681,  667,
 /*  1740 */   667,  667,  171,  329,  494,  667,  667,  153,  184,  108,
 /*  1750 */   667,  667,  667,  150,  667,  667,  454,  463,  472,  475,
 /*  1760 */   466,  469,  478,  484,  481,  490,  487,  667,  667,  667,
 /*  1770 */   277,  279,  281,  667,  667,  291,  667,  667,  667,  302,
 /*  1780 */   306,  311,  319,  667,  330,  684,  685,  337,  980,  773,
 /*  1790 */   667,  667,  667,  667,  147,  682,  683,  667,  426,  441,
 /*  1800 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  667,
 /*  1810 */    62,   65,  667,  667,  667,  667,  842,  667,  845,  667,
 /*  1820 */   846,  831,  128,  667,  667,  667,  703,  807,  667,   47,
 /*  1830 */   667,  667,  702,  667,   50,  139,   59,  144,   56,  164,
 /*  1840 */    44,  667,  154,  667,  667,  132,  276,  677,  678,  680,
 /*  1850 */   679,  681,  667,  667,  667,  171,  497,  494,  667,  667,
 /*  1860 */   153,  184,  108,  667,  667,  667,  150,  667,  667,  454,
 /*  1870 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*  1880 */   667,  667,  307,  277,  279,  281,  667,  667,  291,  667,
 /*  1890 */   667,  318,  302,  306,  311,  319,  667,  330,  684,  685,
 /*  1900 */   337,  178,  180,  179,  143,  667,  646,  147,  682,  683,
 /*  1910 */   667,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*  1920 */   495,  445,  667,   62,   65,  667,  667,   72,  667,  667,
 /*  1930 */   667,  667,  667,  667,  667,  128,  667,  178,  180,  179,
 /*  1940 */   143,  667,   47,  667,  667,  584,  667,   50,  139,   59,
 /*  1950 */   144,   56,  164,   44,  667,  154,  667,  667,  132,  276,
 /*  1960 */   677,  678,  680,  679,  681,  667,  667,  667,  171,  336,
 /*  1970 */   494,  667,  667,  153,  184,  108,  667,  667,  667,  150,
 /*  1980 */   667,  667,  454,  463,  472,  475,  466,  469,  478,  484,
 /*  1990 */   481,  490,  487,  667,  667,  316,  277,  279,  281,  667,
 /*  2000 */   667,  291,  667,  667,  318,  302,  306,  311,  319,  667,
 /*  2010 */   330,  684,  685,  337,  178,  180,  179,  143,  667,  688,
 /*  2020 */   147,  682,  683,  667,  426,  441,  448,  451,  146,  148,
 /*  2030 */   149,  151,  152,  495,  445,  667,   62,   65,  667,  667,
 /*  2040 */   352,  667,  667,  667,  356,  667,  667,  667,  128,  667,
 /*  2050 */   178,  180,  179,  143,  667,   47,  667,  667,  667,  667,
 /*  2060 */    50,  139,   59,  144,   56,  164,   44,  667,  154,  667,
 /*  2070 */   667,  132,  276,  677,  678,  680,  679,  681,  667,  667,
 /*  2080 */   667,  171,  502,  494,  667,  667,  153,  184,  108,  667,
 /*  2090 */   667,  667,  150,  667,  667,  454,  463,  472,  475,  466,
 /*  2100 */   469,  478,  484,  481,  490,  487,  667,  667,  667,  277,
 /*  2110 */   279,  281,  667,  843,  291,  809,  831,  667,  302,  306,
 /*  2120 */   311,  319,  807,  330,  684,  685,  337,  702,  667,  667,
 /*  2130 */   706,  667,  667,  147,  682,  683,  667,  426,  441,  448,
 /*  2140 */   451,  146,  148,  149,  151,  152,  495,  445,  667,   62,
 /*  2150 */    65,  667,  667,  363,  667,  667,  667,  667,  667,  667,
 /*  2160 */   361,  128,  667,  178,  180,  179,  143,  667,   47,  667,
 /*  2170 */   667,  667,  667,   50,  139,   59,  144,   56,  164,   44,
 /*  2180 */   667,  154,  667,  667,  132,  276,  677,  678,  680,  679,
 /*  2190 */   681,  667,  667,  667,  171,  510,  494,  667,  667,  153,
 /*  2200 */   184,  108,  667,  667,  667,  150,  667,  667,  454,  463,
 /*  2210 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  667,
 /*  2220 */   667,  531,  277,  279,  281,  667,  667,  291,  667,  667,
 /*  2230 */   318,  302,  306,  311,  319,  667,  330,  684,  685,  337,
 /*  2240 */   178,  180,  179,  143,  667,  743,  147,  682,  683,  667,
 /*  2250 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  2260 */   445,  667,   62,   65,  667,  667,  352,  667,  667,  667,
 /*  2270 */   360,  667,  667,  667,  128,  667,  178,  180,  179,  143,
 /*  2280 */   667,   47,  667,  667,  667,  667,   50,  139,   59,  144,
 /*  2290 */    56,  164,   44,  667,  154,  667,  667,  132,  276,  677,
 /*  2300 */   678,  680,  679,  681,  667,  667,  667,  171,  509,  494,
 /*  2310 */   667,  667,  153,  184,  108,  667,  667,  667,  150,  667,
 /*  2320 */   667,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  2330 */   490,  487,  667,  667,  542,  277,  279,  281,  667,  667,
 /*  2340 */   291,  667,  667,  318,  302,  306,  311,  319,  667,  330,
 /*  2350 */   684,  685,  337,  178,  180,  179,  143,  667,  775,  147,
 /*  2360 */   682,  683,  667,  426,  441,  448,  451,  146,  148,  149,
 /*  2370 */   151,  152,  495,  445,  667,   62,   65,  667,  667,  352,
 /*  2380 */   667,  667,  667,  366,  667,  667,  667,  128,  667,  178,
 /*  2390 */   180,  179,  143,  667,   47,  667,  667,  667,  667,   50,
 /*  2400 */   139,   59,  144,   56,  164,   44,  667,  154,  667,  667,
 /*  2410 */   132,  276,  677,  678,  680,  679,  681,  667,  667,  667,
 /*  2420 */   171,  515,  494,  667,  667,  153,  184,  108,  667,  667,
 /*  2430 */   667,  150,  667,  667,  454,  463,  472,  475,  466,  469,
 /*  2440 */   478,  484,  481,  490,  487,  667,  667,  667,  277,  279,
 /*  2450 */   281,  667,  667,  291,  667,  667,   45,  302,  306,  311,
 /*  2460 */   319,  667,  330,  684,  685,  337,  178,  180,  179,  143,
 /*  2470 */   667,  667,  147,  682,  683,  667,  426,  441,  448,  451,
 /*  2480 */   146,  148,  149,  151,  152,  495,  445,  667,   62,   65,
 /*  2490 */   667,  667,  693,  667,  654,  668,  657,  661,  662,  667,
 /*  2500 */   128,  667,  667,  125,  667,  667,  667,   47,  667,  667,
 /*  2510 */   667,  667,   50,  139,   59,  144,   56,  164,   44,  123,
 /*  2520 */   154,  667,  667,  132,  276,  667,  667,  667,  667,  178,
 /*  2530 */   180,  179,  143,  171,  522,  494,  667,  667,  153,  184,
 /*  2540 */   108,  667,  667,  667,  150,  667,  667,  454,  463,  472,
 /*  2550 */   475,  466,  469,  478,  484,  481,  490,  487,  667,  667,
 /*  2560 */   667,  277,  279,  281,  667,  667,  291,  667,  667,  667,
 /*  2570 */   302,  306,  311,  319,  667,  330,  667,  699,  337,  654,
 /*  2580 */   668,  657,  661,  662,  667,  147,  667,  667,  667,  426,
 /*  2590 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  2600 */   667,   62,   65,  667,  667,  352,  667,  667,  667,  370,
 /*  2610 */   667,  667,  667,  128,  667,  178,  180,  179,  143,  667,
 /*  2620 */    47,  667,  667,  667,  667,   50,  139,   59,  144,   56,
 /*  2630 */   164,   44,  431,  154,  667,  667,  132,  276,  667,  667,
 /*  2640 */   667,  430,  178,  180,  179,  143,  171,  521,  494,  667,
 /*  2650 */   667,  153,  184,  108,  667,  667,  667,  150,  667,  667,
 /*  2660 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  2670 */   487,  667,  667,  667,  277,  279,  281,  667,  667,  291,
 /*  2680 */   667,  667,  667,  302,  306,  311,  319,  667,  330,  667,
 /*  2690 */   753,  337,  654,  668,  657,  661,  662,  667,  147,  667,
 /*  2700 */   667,  667,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  2710 */   152,  495,  445,  667,   62,   65,  667,  352,  667,  667,
 /*  2720 */   667,  379,  667,  667,  155,  667,  128,  178,  180,  179,
 /*  2730 */   143,  667,  667,   47,  178,  180,  179,  143,   50,  139,
 /*  2740 */    59,  144,   56,  164,   44,  595,  154,  667,  667,  132,
 /*  2750 */   276,  767,  667,  654,  668,  657,  661,  662,  667,  171,
 /*  2760 */   546,  494,  667,  667,  153,  184,  108,  667,  667,  667,
 /*  2770 */   150,  667,  667,  454,  463,  472,  475,  466,  469,  478,
 /*  2780 */   484,  481,  490,  487,  667,  667,  667,  277,  279,  281,
 /*  2790 */   667,  667,  291,  667,  667,  667,  302,  306,  311,  319,
 /*  2800 */   667,  330,  667,  785,  337,  654,  668,  657,  661,  662,
 /*  2810 */   667,  147,  667,  667,  667,  426,  441,  448,  451,  146,
 /*  2820 */   148,  149,  151,  152,  495,  445,  667,   62,   65,  667,
 /*  2830 */   352,  667,  667,  667,  383,  667,  667,  626,  667,  128,
 /*  2840 */   178,  180,  179,  143,  667,  667,   47,  178,  180,  179,
 /*  2850 */   143,   50,  139,   59,  144,   56,  164,   44,  667,  154,
 /*  2860 */   667,  667,  132,  276,  799,  667,  654,  668,  657,  661,
 /*  2870 */   662,  667,  171,  545,  494,  667,  667,  153,  184,  108,
 /*  2880 */   667,  667,  667,  150,  667,  667,  454,  463,  472,  475,
 /*  2890 */   466,  469,  478,  484,  481,  490,  487,  667,  667,  667,
 /*  2900 */   277,  279,  281,  667,  667,  291,  667,  667,   48,  302,
 /*  2910 */   306,  311,  319,  667,  330,  667,  667,  337,  178,  180,
 /*  2920 */   179,  143,  667,  667,  147,  667,  667,  667,  426,  441,
 /*  2930 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  667,
 /*  2940 */    62,   65,  667,  352,  667,  667,  667,  387,  667,  667,
 /*  2950 */   625,  667,  128,  178,  180,  179,  143,  667,  667,   47,
 /*  2960 */   178,  180,  179,  143,   50,  139,   59,  144,   56,  164,
 /*  2970 */    44,   51,  154,  667,  667,  132,  276,  667,  667,  667,
 /*  2980 */   667,  178,  180,  179,  143,  171,  551,  494,  667,  667,
 /*  2990 */   153,  184,  108,  667,  667,  667,  150,  667,  667,  454,
 /*  3000 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*  3010 */   667,  667,  667,  277,  279,  281,  667,  667,  291,  667,
 /*  3020 */   667,  624,  302,  306,  311,  319,  667,  330,  667,  667,
 /*  3030 */   337,  178,  180,  179,  143,  667,  667,  147,  667,  667,
 /*  3040 */   667,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*  3050 */   495,  445,  667,   62,   65,  667,  352,  667,  667,  667,
 /*  3060 */   391,  667,  667,   54,  667,  128,  178,  180,  179,  143,
 /*  3070 */   667,  667,   47,  178,  180,  179,  143,   50,  139,   59,
 /*  3080 */   144,   56,  164,   44,  623,  154,  667,  667,  132,  276,
 /*  3090 */   667,  667,  667,  667,  178,  180,  179,  143,  171,  550,
 /*  3100 */   494,  667,  667,  153,  184,  108,  667,  667,  667,  150,
 /*  3110 */   667,  667,  454,  463,  472,  475,  466,  469,  478,  484,
 /*  3120 */   481,  490,  487,  667,  667,  667,  277,  279,  281,  667,
 /*  3130 */   667,  291,  667,  667,   57,  302,  306,  311,  319,  667,
 /*  3140 */   330,  667,  667,  337,  178,  180,  179,  143,  667,  667,
 /*  3150 */   147,  667,  667,  667,  426,  441,  448,  451,  146,  148,
 /*  3160 */   149,  151,  152,  495,  445,  667,   62,   65,  667,  352,
 /*  3170 */   667,  667,  667,  394,  667,  667,  622,  667,  128,  178,
 /*  3180 */   180,  179,  143,  667,  667,   47,  178,  180,  179,  143,
 /*  3190 */    50,  139,   59,  144,   56,  164,   44,   60,  154,  667,
 /*  3200 */   667,  132,  276,  667,  667,  667,  667,  178,  180,  179,
 /*  3210 */   143,  171,  562,  494,  667,  667,  153,  184,  108,  667,
 /*  3220 */   667,  667,  150,  667,  667,  454,  463,  472,  475,  466,
 /*  3230 */   469,  478,  484,  481,  490,  487,  667,  667,  667,  277,
 /*  3240 */   279,  281,  667,  667,  291,  667,  667,  621,  302,  306,
 /*  3250 */   311,  319,  667,  330,  667,  667,  337,  178,  180,  179,
 /*  3260 */   143,  667,  667,  147,  667,  667,  667,  426,  441,  448,
 /*  3270 */   451,  146,  148,  149,  151,  152,  495,  445,  667,   62,
 /*  3280 */    65,  667,  352,  667,  667,  667,  398,  667,  667,   63,
 /*  3290 */   667,  128,  178,  180,  179,  143,  667,  667,   47,  178,
 /*  3300 */   180,  179,  143,   50,  139,   59,  144,   56,  164,   44,
 /*  3310 */   620,  154,  667,  667,  132,  276,  667,  667,  667,  667,
 /*  3320 */   178,  180,  179,  143,  171,  561,  494,  667,  667,  153,
 /*  3330 */   184,  108,  667,  667,  667,  150,  667,  667,  454,  463,
 /*  3340 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  667,
 /*  3350 */   667,  667,  277,  279,  281,  667,  667,  291,  667,  667,
 /*  3360 */    66,  302,  306,  311,  319,  667,  330,  667,  667,  337,
 /*  3370 */   178,  180,  179,  143,  667,  667,  147,  667,  667,  667,
 /*  3380 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  3390 */   445,  667,   62,   65,  667,  352,  667,  667,  667,  402,
 /*  3400 */   667,  667,   68,  667,  128,  178,  180,  179,  143,  667,
 /*  3410 */   667,   47,  178,  180,  179,  143,   50,  139,   59,  144,
 /*  3420 */    56,  164,   44,   74,  154,  667,  667,  132,  276,  667,
 /*  3430 */   667,  667,  667,  178,  180,  179,  143,  171,  570,  494,
 /*  3440 */   667,  667,  153,  184,  108,  667,  667,  667,  150,  667,
 /*  3450 */   667,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  3460 */   490,  487,  667,  667,  667,  277,  279,  281,  667,  667,
 /*  3470 */   291,  667,  667,   77,  302,  306,  311,  319,  667,  330,
 /*  3480 */   667,  667,  337,  178,  180,  179,  143,  667,  667,  147,
 /*  3490 */   667,  667,  667,  426,  441,  448,  451,  146,  148,  149,
 /*  3500 */   151,  152,  495,  445,  667,   62,   65,  667,  352,  667,
 /*  3510 */   667,  667,  405,  667,  667,   79,  667,  128,  178,  180,
 /*  3520 */   179,  143,  667,  667,   47,  178,  180,  179,  143,   50,
 /*  3530 */   139,   59,  144,   56,  164,   44,   81,  154,  667,  667,
 /*  3540 */   132,  276,  667,  667,  667,  667,  178,  180,  179,  143,
 /*  3550 */   171,  569,  494,  667,  667,  153,  184,  108,  667,  667,
 /*  3560 */   667,  150,  667,  667,  454,  463,  472,  475,  466,  469,
 /*  3570 */   478,  484,  481,  490,  487,  667,  667,  667,  277,  279,
 /*  3580 */   281,  667,  667,  291,  667,  667,   83,  302,  306,  311,
 /*  3590 */   319,  667,  330,  667,  667,  337,  178,  180,  179,  143,
 /*  3600 */   667,  667,  147,  667,  667,  667,  426,  441,  448,  451,
 /*  3610 */   146,  148,  149,  151,  152,  495,  445,  667,   62,   65,
 /*  3620 */   667,  352,  667,  667,  667,  409,  667,  667,   85,  667,
 /*  3630 */   128,  178,  180,  179,  143,  667,  667,   47,  178,  180,
 /*  3640 */   179,  143,   50,  139,   59,  144,   56,  164,   44,   87,
 /*  3650 */   154,  667,  667,  132,  276,  667,  667,  667,  667,  178,
 /*  3660 */   180,  179,  143,  171,  748,  494,  667,  667,  153,  184,
 /*  3670 */   108,  667,  667,  667,  150,  667,  667,  454,  463,  472,
 /*  3680 */   475,  466,  469,  478,  484,  481,  490,  487,  667,  667,
 /*  3690 */   667,  277,  279,  281,  667,  667,  291,  667,  667,   89,
 /*  3700 */   302,  306,  311,  319,  667,  330,  667,  667,  337,  178,
 /*  3710 */   180,  179,  143,  667,  667,  147,  667,  667,  667,  426,
 /*  3720 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  3730 */   667,   62,   65,  667,  352,  667,  667,  667,  420,  667,
 /*  3740 */   667,   91,  667,  128,  178,  180,  179,  143,  667,  667,
 /*  3750 */    47,  178,  180,  179,  143,   50,  139,   59,  144,   56,
 /*  3760 */   164,   44,   93,  154,  667,  667,  132,  276,  667,  667,
 /*  3770 */   667,  667,  178,  180,  179,  143,  171,  750,  494,  667,
 /*  3780 */   667,  153,  184,  108,  667,  667,  667,  150,  667,  667,
 /*  3790 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  3800 */   487,  667,  667,  667,  277,  279,  281,  667,  667,  291,
 /*  3810 */   667,  667,   95,  302,  306,  311,  319,  667,  330,  667,
 /*  3820 */   667,  337,  178,  180,  179,  143,  667,  667,  147,  667,
 /*  3830 */   667,  667,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  3840 */   152,  495,  445,  667,   62,   65,  667,  352,  667,  667,
 /*  3850 */   667,  424,  667,  667,   97,  667,  128,  178,  180,  179,
 /*  3860 */   143,  667,  667,   47,  178,  180,  179,  143,   50,  139,
 /*  3870 */    59,  144,   56,  164,   44,   99,  154,  667,  667,  132,
 /*  3880 */   276,  667,  667,  667,  667,  178,  180,  179,  143,  171,
 /*  3890 */   755,  494,  667,  667,  153,  184,  108,  667,  667,  667,
 /*  3900 */   150,  667,  667,  454,  463,  472,  475,  466,  469,  478,
 /*  3910 */   484,  481,  490,  487,  667,  667,  667,  277,  279,  281,
 /*  3920 */   667,  667,  291,  667,  667,  101,  302,  306,  311,  319,
 /*  3930 */   667,  330,  667,  667,  337,  178,  180,  179,  143,  667,
 /*  3940 */   667,  147,  667,  667,  667,  426,  441,  448,  451,  146,
 /*  3950 */   148,  149,  151,  152,  495,  445,  667,   62,   65,  667,
 /*  3960 */   103,  667,  667,  667,  667,  667,  667,  105,  667,  128,
 /*  3970 */   178,  180,  179,  143,  667,  667,   47,  178,  180,  179,
 /*  3980 */   143,   50,  139,   59,  144,   56,  164,   44,  107,  154,
 /*  3990 */   667,  667,  132,  276,  667,  667,  667,  667,  178,  180,
 /*  4000 */   179,  143,  171,  757,  494,  667,  667,  153,  184,  108,
 /*  4010 */   667,  667,  667,  150,  667,  667,  454,  463,  472,  475,
 /*  4020 */   466,  469,  478,  484,  481,  490,  487,  667,  667,  667,
 /*  4030 */   277,  279,  281,  667,  667,  291,  667,  667,  127,  302,
 /*  4040 */   306,  311,  319,  667,  330,  667,  667,  337,  178,  180,
 /*  4050 */   179,  143,  667,  667,  147,  667,  667,  667,  426,  441,
 /*  4060 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  667,
 /*  4070 */    62,   65,  667,  131,  667,  667,  667,  667,  667,  667,
 /*  4080 */   138,  667,  128,  178,  180,  179,  143,  667,  667,   47,
 /*  4090 */   178,  180,  179,  143,   50,  139,   59,  144,   56,  164,
 /*  4100 */    44,  142,  154,  667,  667,  132,  276,  667,  667,  667,
 /*  4110 */   667,  178,  180,  179,  143,  171,  762,  494,  667,  667,
 /*  4120 */   153,  184,  108,  667,  667,  667,  150,  667,  667,  454,
 /*  4130 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*  4140 */   667,  667,  667,  277,  279,  281,  667,  667,  291,  667,
 /*  4150 */   667,  145,  302,  306,  311,  319,  667,  330,  667,  667,
 /*  4160 */   337,  178,  180,  179,  143,  667,  667,  147,  667,  667,
 /*  4170 */   667,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*  4180 */   495,  445,  667,   62,   65,  667,  183,  667,  667,  667,
 /*  4190 */   667,  667,  667,  333,  667,  128,  178,  180,  179,  143,
 /*  4200 */   667,  667,   47,  178,  180,  179,  143,   50,  139,   59,
 /*  4210 */   144,   56,  164,   44,  442,  154,  667,  667,  132,  276,
 /*  4220 */   667,  667,  667,  667,  178,  180,  179,  143,  171,  764,
 /*  4230 */   494,  667,  667,  153,  184,  108,  667,  667,  667,  150,
 /*  4240 */   667,  667,  454,  463,  472,  475,  466,  469,  478,  484,
 /*  4250 */   481,  490,  487,  667,  667,  667,  277,  279,  281,  667,
 /*  4260 */   667,  291,  667,  667,  446,  302,  306,  311,  319,  667,
 /*  4270 */   330,  667,  667,  337,  178,  180,  179,  143,  667,  667,
 /*  4280 */   147,  667,  667,  667,  426,  441,  448,  451,  146,  148,
 /*  4290 */   149,  151,  152,  495,  445,  667,   62,   65,  667,  449,
 /*  4300 */   667,  667,  667,  667,  667,  667,  452,  667,  128,  178,
 /*  4310 */   180,  179,  143,  667,  667,   47,  178,  180,  179,  143,
 /*  4320 */    50,  139,   59,  144,   56,  164,   44,  499,  154,  667,
 /*  4330 */   667,  132,  276,  667,  667,  667,  667,  178,  180,  179,
 /*  4340 */   143,  171,  769,  494,  667,  667,  153,  184,  108,  667,
 /*  4350 */   667,  667,  150,  667,  667,  454,  463,  472,  475,  466,
 /*  4360 */   469,  478,  484,  481,  490,  487,  667,  667,  667,  277,
 /*  4370 */   279,  281,  667,  667,  291,  667,  667,  506,  302,  306,
 /*  4380 */   311,  319,  667,  330,  667,  667,  337,  178,  180,  179,
 /*  4390 */   143,  667,  667,  147,  667,  667,  667,  426,  441,  448,
 /*  4400 */   451,  146,  148,  149,  151,  152,  495,  445,  667,   62,
 /*  4410 */    65,  667,  512,  667,  667,  667,  667,  667,  667,  599,
 /*  4420 */   667,  128,  178,  180,  179,  143,  667,  667,   47,  178,
 /*  4430 */   180,  179,  143,   50,  139,   59,  144,   56,  164,   44,
 /*  4440 */   920,  154,  667,  667,  132,  276,  667,  667,  667,  667,
 /*  4450 */   178,  180,  179,  143,  171,  771,  494,  667,  667,  153,
 /*  4460 */   184,  108,  667,  667,  667,  150,  667,  667,  454,  463,
 /*  4470 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  667,
 /*  4480 */   667,  667,  277,  279,  281,  667,  667,  291,  643,  667,
 /*  4490 */   645,  302,  306,  311,  319,  703,  330,  704,  667,  337,
 /*  4500 */   667,  702,  667,  667,  667,  667,  147,  667,  667,  667,
 /*  4510 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  4520 */   445,  667,   62,   65,  667,  667,  667,  667,  667,  667,
 /*  4530 */   667,  667,  667,  667,  128,  667,  667,  667,  667,  667,
 /*  4540 */   667,   47,  667,  667,  667,  667,   50,  139,   59,  144,
 /*  4550 */    56,  164,   44,  667,  154,  667,  667,  132,  276,  667,
 /*  4560 */   667,  667,  667,  667,  667,  667,  667,  171,  780,  494,
 /*  4570 */   667,  667,  153,  184,  108,  667,  667,  667,  150,  667,
 /*  4580 */   667,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  4590 */   490,  487,  667,  667,  667,  277,  279,  281,  667,  667,
 /*  4600 */   291,  667,  667,  667,  302,  306,  311,  319,  667,  330,
 /*  4610 */   667,  667,  337,  667,  667,  667,  667,  667,  667,  147,
 /*  4620 */   667,  667,  667,  426,  441,  448,  451,  146,  148,  149,
 /*  4630 */   151,  152,  495,  445,  667,   62,   65,  667,  667,  667,
 /*  4640 */   667,  667,  667,  667,  667,  667,  667,  128,  667,  667,
 /*  4650 */   667,  667,  667,  667,   47,  667,  667,  667,  667,   50,
 /*  4660 */   139,   59,  144,   56,  164,   44,  667,  154,  667,  667,
 /*  4670 */   132,  276,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  4680 */   171,  782,  494,  667,  667,  153,  184,  108,  667,  667,
 /*  4690 */   667,  150,  667,  667,  454,  463,  472,  475,  466,  469,
 /*  4700 */   478,  484,  481,  490,  487,  667,  667,  667,  277,  279,
 /*  4710 */   281,  667,  667,  291,  667,  667,  667,  302,  306,  311,
 /*  4720 */   319,  667,  330,  667,  667,  337,  667,  667,  667,  667,
 /*  4730 */   667,  667,  147,  667,  667,  667,  426,  441,  448,  451,
 /*  4740 */   146,  148,  149,  151,  152,  495,  445,  667,   62,   65,
 /*  4750 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  4760 */   128,  667,  667,  667,  667,  667,  667,   47,  667,  667,
 /*  4770 */   667,  667,   50,  139,   59,  144,   56,  164,   44,  667,
 /*  4780 */   154,  667,  667,  132,  276,  667,  667,  667,  667,  667,
 /*  4790 */   667,  667,  667,  171,  787,  494,  667,  667,  153,  184,
 /*  4800 */   108,  667,  667,  667,  150,  667,  667,  454,  463,  472,
 /*  4810 */   475,  466,  469,  478,  484,  481,  490,  487,  667,  667,
 /*  4820 */   667,  277,  279,  281,  667,  667,  291,  667,  667,  667,
 /*  4830 */   302,  306,  311,  319,  667,  330,  667,  667,  337,  667,
 /*  4840 */   667,  667,  667,  667,  667,  147,  667,  667,  667,  426,
 /*  4850 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  4860 */   667,   62,   65,  667,  667,  667,  667,  667,  667,  667,
 /*  4870 */   667,  667,  667,  128,  667,  667,  667,  667,  667,  667,
 /*  4880 */    47,  667,  667,  667,  667,   50,  139,   59,  144,   56,
 /*  4890 */   164,   44,  667,  154,  667,  667,  132,  276,  667,  667,
 /*  4900 */   667,  667,  667,  667,  667,  667,  171,  789,  494,  667,
 /*  4910 */   667,  153,  184,  108,  667,  667,  667,  150,  667,  667,
 /*  4920 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  4930 */   487,  667,  667,  667,  277,  279,  281,  667,  667,  291,
 /*  4940 */   667,  667,  667,  302,  306,  311,  319,  667,  330,  667,
 /*  4950 */   667,  337,  667,  667,  667,  667,  667,  667,  147,  667,
 /*  4960 */   667,  667,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  4970 */   152,  495,  445,  667,   62,   65,  667,  667,  667,  667,
 /*  4980 */   667,  667,  667,  667,  667,  667,  128,  667,  667,  667,
 /*  4990 */   667,  667,  667,   47,  667,  667,  667,  667,   50,  139,
 /*  5000 */    59,  144,   56,  164,   44,  667,  154,  667,  667,  132,
 /*  5010 */   276,  667,  667,  667,  667,  667,  667,  667,  667,  171,
 /*  5020 */   794,  494,  667,  667,  153,  184,  108,  667,  667,  667,
 /*  5030 */   150,  667,  667,  454,  463,  472,  475,  466,  469,  478,
 /*  5040 */   484,  481,  490,  487,  667,  667,  667,  277,  279,  281,
 /*  5050 */   667,  667,  291,  667,  667,  667,  302,  306,  311,  319,
 /*  5060 */   667,  330,  667,  667,  337,  667,  667,  667,  667,  667,
 /*  5070 */   667,  147,  667,  667,  667,  426,  441,  448,  451,  146,
 /*  5080 */   148,  149,  151,  152,  495,  445,  667,   62,   65,  667,
 /*  5090 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  128,
 /*  5100 */   667,  667,  667,  667,  667,  667,   47,  667,  667,  667,
 /*  5110 */   667,   50,  139,   59,  144,   56,  164,   44,  667,  154,
 /*  5120 */   667,  667,  132,  276,  667,  667,  667,  667,  667,  667,
 /*  5130 */   667,  667,  171,  796,  494,  667,  667,  153,  184,  108,
 /*  5140 */   667,  667,  667,  150,  667,  667,  454,  463,  472,  475,
 /*  5150 */   466,  469,  478,  484,  481,  490,  487,  667,  667,  667,
 /*  5160 */   277,  279,  281,  667,  667,  291,  667,  667,  667,  302,
 /*  5170 */   306,  311,  319,  667,  330,  667,  667,  337,  667,  667,
 /*  5180 */   667,  667,  667,  667,  147,  667,  667,  667,  426,  441,
 /*  5190 */   448,  451,  146,  148,  149,  151,  152,  495,  445,  667,
 /*  5200 */    62,   65,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  5210 */   667,  667,  128,  667,  667,  667,  667,  667,  667,   47,
 /*  5220 */   667,  667,  667,  667,   50,  139,   59,  144,   56,  164,
 /*  5230 */    44,  667,  154,  667,  667,  132,  276,  667,  667,  667,
 /*  5240 */   667,  667,  667,  667,  667,  171,  801,  494,  667,  667,
 /*  5250 */   153,  184,  108,  667,  667,  667,  150,  667,  667,  454,
 /*  5260 */   463,  472,  475,  466,  469,  478,  484,  481,  490,  487,
 /*  5270 */   667,  667,  667,  277,  279,  281,  667,  667,  291,  667,
 /*  5280 */   667,  667,  302,  306,  311,  319,  667,  330,  667,  667,
 /*  5290 */   337,  667,  667,  667,  667,  667,  667,  147,  667,  667,
 /*  5300 */   667,  426,  441,  448,  451,  146,  148,  149,  151,  152,
 /*  5310 */   495,  445,  667,   62,   65,  667,  667,  667,  667,  667,
 /*  5320 */   667,  667,  667,  667,  667,  128,  667,  667,  667,  667,
 /*  5330 */   667,  667,   47,  667,  667,  667,  667,   50,  139,   59,
 /*  5340 */   144,   56,  164,   44,  667,  154,  667,  667,  132,  276,
 /*  5350 */   667,  667,  667,  667,  667,  667,  667,  667,  171,  803,
 /*  5360 */   494,  667,  667,  153,  184,  108,  667,  667,  667,  150,
 /*  5370 */   667,  667,  454,  463,  472,  475,  466,  469,  478,  484,
 /*  5380 */   481,  490,  487,  667,  667,  667,  277,  279,  281,  667,
 /*  5390 */   667,  291,  667,  667,  667,  302,  306,  311,  319,  667,
 /*  5400 */   330,  667,  667,  337,  667,  667,  667,  667,  667,  667,
 /*  5410 */   147,  667,  667,  667,  426,  441,  448,  451,  146,  148,
 /*  5420 */   149,  151,  152,  495,  445,  667,   62,   65,  667,  667,
 /*  5430 */   667,  667,  667,  667,  667,  667,  667,  667,  128,  667,
 /*  5440 */   667,  667,  667,  667,  667,   47,  667,  667,  667,  667,
 /*  5450 */    50,  139,   59,  144,   56,  164,   44,  667,  154,  667,
 /*  5460 */   667,  132,  276,  667,  667,  667,  667,  667,  667,  667,
 /*  5470 */   667,  171,  884,  494,  667,  667,  153,  184,  108,  667,
 /*  5480 */   667,  667,  150,  667,  667,  454,  463,  472,  475,  466,
 /*  5490 */   469,  478,  484,  481,  490,  487,  667,  667,  667,  277,
 /*  5500 */   279,  281,  667,  667,  291,  667,  667,  667,  302,  306,
 /*  5510 */   311,  319,  667,  330,  667,  667,  337,  667,  667,  667,
 /*  5520 */   667,  667,  667,  147,  667,  667,  667,  426,  441,  448,
 /*  5530 */   451,  146,  148,  149,  151,  152,  495,  445,  667,   62,
 /*  5540 */    65,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  5550 */   667,  128,  667,  667,  667,  667,  667,  667,   47,  667,
 /*  5560 */   667,  667,  667,   50,  139,   59,  144,   56,  164,   44,
 /*  5570 */   667,  154,  667,  667,  132,  276,  667,  667,  667,  667,
 /*  5580 */   667,  667,  667,  667,  171,  888,  494,  667,  667,  153,
 /*  5590 */   184,  108,  667,  667,  667,  150,  667,  667,  454,  463,
 /*  5600 */   472,  475,  466,  469,  478,  484,  481,  490,  487,  667,
 /*  5610 */   667,  667,  277,  279,  281,  667,  667,  291,  667,  667,
 /*  5620 */   667,  302,  306,  311,  319,  667,  330,  667,  667,  337,
 /*  5630 */   667,  667,  667,  667,  667,  667,  147,  667,  667,  667,
 /*  5640 */   426,  441,  448,  451,  146,  148,  149,  151,  152,  495,
 /*  5650 */   445,  667,   62,   65,  667,  667,  667,  667,  667,  667,
 /*  5660 */   667,  667,  667,  667,  128,  667,  667,  667,  667,  667,
 /*  5670 */   667,   47,  667,  667,  667,  667,   50,  139,   59,  144,
 /*  5680 */    56,  164,   44,  667,  154,  667,  667,  132,  276,  667,
 /*  5690 */   667,  667,  667,  667,  667,  667,  667,  171,  890,  494,
 /*  5700 */   667,  667,  153,  184,  108,  667,  667,  667,  150,  667,
 /*  5710 */   667,  454,  463,  472,  475,  466,  469,  478,  484,  481,
 /*  5720 */   490,  487,  667,  667,  667,  277,  279,  281,  667,  667,
 /*  5730 */   291,  667,  667,  667,  302,  306,  311,  319,  667,  330,
 /*  5740 */   667,  667,  337,  667,  667,  667,  667,  667,  667,  147,
 /*  5750 */   667,  667,  667,  426,  441,  448,  451,  146,  148,  149,
 /*  5760 */   151,  152,  495,  445,  667,   62,   65,  667,  667,  667,
 /*  5770 */   667,  667,  667,  667,  667,  667,  667,  128,  667,  667,
 /*  5780 */   667,  667,  667,  667,   47,  667,  667,  667,  667,   50,
 /*  5790 */   139,   59,  144,   56,  164,   44,  667,  154,  667,  667,
 /*  5800 */   132,  276,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  5810 */   171,  898,  494,  667,  667,  153,  184,  108,  667,  667,
 /*  5820 */   667,  150,  667,  667,  454,  463,  472,  475,  466,  469,
 /*  5830 */   478,  484,  481,  490,  487,  667,  667,  667,  277,  279,
 /*  5840 */   281,  667,  667,  291,  667,  667,  667,  302,  306,  311,
 /*  5850 */   319,  667,  330,  667,  667,  337,  667,  667,  667,  667,
 /*  5860 */   667,  667,  147,  667,  667,  667,  426,  441,  448,  451,
 /*  5870 */   146,  148,  149,  151,  152,  495,  445,  667,   62,   65,
 /*  5880 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  5890 */   128,  667,  667,  667,  667,  667,  667,   47,  667,  667,
 /*  5900 */   667,  667,   50,  139,   59,  144,   56,  164,   44,  667,
 /*  5910 */   154,  667,  667,  132,  276,  667,  667,  667,  667,  667,
 /*  5920 */   667,  667,  667,  171,  902,  494,  667,  667,  153,  184,
 /*  5930 */   108,  667,  667,  667,  150,  667,  667,  454,  463,  472,
 /*  5940 */   475,  466,  469,  478,  484,  481,  490,  487,  667,  667,
 /*  5950 */   667,  277,  279,  281,  667,  667,  291,  667,  667,  667,
 /*  5960 */   302,  306,  311,  319,  667,  330,  667,  667,  337,  667,
 /*  5970 */   667,  667,  667,  667,  667,  147,  667,  667,  667,  426,
 /*  5980 */   441,  448,  451,  146,  148,  149,  151,  152,  495,  445,
 /*  5990 */   667,   62,   65,  667,  667,  667,  667,  667,  667,  667,
 /*  6000 */   667,  667,  667,  128,  667,  667,  667,  667,  667,  667,
 /*  6010 */    47,  667,  667,  667,  667,   50,  139,   59,  144,   56,
 /*  6020 */   164,   44,  667,  154,  667,  667,  132,  276,  667,  667,
 /*  6030 */   667,  667,  667,  667,  667,  667,  171,  904,  494,  667,
 /*  6040 */   667,  153,  184,  108,  667,  667,  667,  150,  667,  667,
 /*  6050 */   454,  463,  472,  475,  466,  469,  478,  484,  481,  490,
 /*  6060 */   487,  667,  667,  667,  277,  279,  281,  667,  667,  291,
 /*  6070 */   667,  667,  667,  302,  306,  311,  319,  667,  330,  667,
 /*  6080 */   667,  337,  667,  667,  667,  667,  667,  667,  147,  667,
 /*  6090 */   667,  667,  426,  441,  448,  451,  146,  148,  149,  151,
 /*  6100 */   152,  495,  445,  667,   62,   65,  667,  667,  667,  667,
 /*  6110 */   667,  667,  667,  667,  667,  667,  128,  667,  667,  667,
 /*  6120 */   667,  667,  667,   47,  667,  667,  667,  667,   50,  139,
 /*  6130 */    59,  144,   56,  164,   44,  667,  154,  667,  667,  132,
 /*  6140 */   276,  667,  667,  667,  667,  667,  667,  667,  667,  171,
 /*  6150 */   667,  494,  667,  667,  153,  184,  108,  667,  667,  667,
 /*  6160 */   150,  667,  667,  454,  463,  472,  475,  466,  469,  478,
 /*  6170 */   484,  481,  490,  487,  667,  667,  667,  277,  279,  281,
 /*  6180 */   667,  667,  291,  923,  667,  667,  302,  306,  311,  319,
 /*  6190 */   919,  330,  667,  667,  337,  667,  667,  667,  667,  667,
 /*  6200 */   667,  147,  667,  667,  667,  426,  441,  448,  451,  146,
 /*  6210 */   148,  149,  151,  152,  495,  667,  667,  139,  667,  667,
 /*  6220 */   667,  667,  667,  667,  667,  367,   30,   42,  667,   33,
 /*  6230 */   667,  632,  667,  726,  667,  855,  868,  171,  667,  494,
 /*  6240 */   667,  343,  667,  878,  667,  667,  667,  667,  667,  667,
 /*  6250 */   667,  905,  906,  907,  908,  909,  910,  911,  912,  913,
 /*  6260 */   914,  915,  120,  121,  122,  277,  279,  281,  667,  667,
 /*  6270 */   291,  919,  667,  667,  302,  306,  311,  319,  667,  330,
 /*  6280 */   667,  667,  337,  667,  344,  345,  346,  347,  348,  349,
 /*  6290 */   667,  667,  667,  426,  441,  448,  451,  667,  139,  667,
 /*  6300 */   667,  667,  495,  667,  667,  667,  388,   30,   42,  667,
 /*  6310 */    33,  667,  632,  667,  726,  667,  855,  868,  171,  667,
 /*  6320 */   494,  667,  343,  667,  878,  667,  667,  667,  667,  667,
 /*  6330 */   667,  667,  905,  906,  907,  908,  909,  910,  911,  912,
 /*  6340 */   913,  914,  915,  120,  121,  122,  277,  279,  281,  667,
 /*  6350 */   667,  291,  667,  667,  667,  302,  306,  311,  319,   53,
 /*  6360 */   330,   62,   65,  337,  667,  344,  345,  346,  347,  348,
 /*  6370 */   349,  667,  667,  128,  426,  441,  448,  451,  667,  667,
 /*  6380 */    47,  667,  667,  495,  667,   50,  139,   59,  144,   56,
 /*  6390 */   164,   44,  667,  154,  667,  667,  132,  667,  667,  667,
 /*  6400 */   667,  667,  667,  667,  667,  667,  171,  667,  667,  667,
 /*  6410 */   667,  153,  184,  108,  667,  667,  667,  150,  667,  667,
 /*  6420 */   109,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  6430 */   119,  120,  121,  122,  102,   90,   88,   94,   92,   96,
 /*  6440 */    98,  100,   46,   52,   58,   61,   64,   67,   55,   49,
 /*  6450 */    76,   78,   86,   80,   82,   84,  667,  667,  147,  667,
 /*  6460 */   667,  667,  667,  667,   73,   69,  146,  148,  149,  151,
 /*  6470 */   152,  667,  667,  667,  667,  667,  667, 1369,    1,    2,
 /*  6480 */   921,    4,    5,    6,    7,    8,    9,   10,   11,   12,
 /*  6490 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*  6500 */    23,   24,   25,   26,   27,   28,   29,  104,  667,  667,
 /*  6510 */   102,   90,   88,   94,   92,   96,   98,  100,   46,   52,
 /*  6520 */    58,   61,   64,   67,   55,   49,   76,   78,   86,   80,
 /*  6530 */    82,   84,  667,  891,  667,  667,  667,  667,  667,  667,
 /*  6540 */    73,   69,   46,   52,   58,   61,   64,   67,   55,   49,
 /*  6550 */    76,   78,   86,   80,   82,   84,  667,  155,  667,  667,
 /*  6560 */   916,  917,  918,  667,   73,   69,   75,  178,  180,  179,
 /*  6570 */   143,  667,  667,  161,  667,  667,  597,  593,  596,  667,
 /*  6580 */   667,  667,  667,    3,    4,    5,    6,    7,    8,    9,
 /*  6590 */    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*  6600 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*  6610 */   667,  667,  667,  667,  667,  274,  253,  254,  255,  256,
 /*  6620 */   257,  258,  259,  260,  261,  263,  264,  265,  266,  267,
 /*  6630 */   268,  269,  270,  271,  272,  273,  891,  667,  667,  155,
 /*  6640 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  178,
 /*  6650 */   180,  179,  143,  667,  667,  667,  667,  251,  592,  593,
 /*  6660 */   596,  667,  667,  916,  917,  918,  496,  262,  275,  667,
 /*  6670 */   667,  667,  667,  667,  667,  667,  667,  667,  493,  667,
 /*  6680 */   667,  667,  667,  667,  667,  667,  667,  667,  433,  435,
 /*  6690 */   437,  439,  274,  253,  254,  255,  256,  257,  258,  259,
 /*  6700 */   260,  261,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  6710 */   271,  272,  273,   88,   94,   92,   96,   98,  100,   46,
 /*  6720 */    52,   58,   61,   64,   67,   55,   49,   76,   78,   86,
 /*  6730 */    80,   82,   84,  667,  284,  667,  667,  667,  667,  667,
 /*  6740 */   667,   73,   69,  496,  262,  275,  667,  667,  667,  667,
 /*  6750 */   667,  667,  667,  667,  667,  493,  667,  667,  667,  667,
 /*  6760 */   667,  667,  667,  667,  667,  433,  435,  437,  439,  667,
 /*  6770 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  6780 */   667,  667,  667,  667,  667,  667,  667,  667,  274,  253,
 /*  6790 */   254,  255,  256,  257,  258,  259,  260,  261,  263,  264,
 /*  6800 */   265,  266,  267,  268,  269,  270,  271,  272,  273,   94,
 /*  6810 */    92,   96,   98,  100,   46,   52,   58,   61,   64,   67,
 /*  6820 */    55,   49,   76,   78,   86,   80,   82,   84,  667,  667,
 /*  6830 */   289,  399,  353,  667,  667,  203,   73,   69,  199,  496,
 /*  6840 */   262,  275,  395,  667,  667,  667,  667,  343,  667,  667,
 /*  6850 */   667,  493,  667,  202,  667,  667,  667,  667,  343,  196,
 /*  6860 */   667,  433,  435,  437,  439,  274,  253,  254,  255,  256,
 /*  6870 */   257,  258,  259,  260,  261,  263,  264,  265,  266,  267,
 /*  6880 */   268,  269,  270,  271,  272,  273,  667,  667,  667,  667,
 /*  6890 */   344,  345,  346,  347,  348,  349,  234,  384,  410,  411,
 /*  6900 */   194,  344,  345,  346,  347,  348,  349,  301,  192,  193,
 /*  6910 */   195,  198,  197,  667,  667,  566,  496,  262,  275,  189,
 /*  6920 */   667,  667,  667,  667,  667,  667,  667,  667,  493,  667,
 /*  6930 */   667,  667,  667,  218,  667,  667,  667,  234,  433,  435,
 /*  6940 */   437,  439,  667,  109,  110,  111,  112,  113,  114,  115,
 /*  6950 */   116,  117,  118,  119,  120,  121,  122,  667,  667,  667,
 /*  6960 */   219,  274,  253,  254,  255,  256,  257,  258,  259,  260,
 /*  6970 */   261,  263,  264,  265,  266,  267,  268,  269,  270,  271,
 /*  6980 */   272,  273,  667,  667,  109,  110,  111,  112,  113,  114,
 /*  6990 */   115,  116,  117,  118,  119,  120,  121,  122,  667,  667,
 /*  7000 */   667,  667,  203,  304,  667,  206,  667,  667,  667,  667,
 /*  7010 */   667,  667,  496,  262,  275,  667,  667,  667,  667,  667,
 /*  7020 */   202,  667,  667,  667,  493,  667,  196,  667,  207,  667,
 /*  7030 */   667,  667,  667,  667,  433,  435,  437,  439,  274,  253,
 /*  7040 */   254,  255,  256,  257,  258,  259,  260,  261,  263,  264,
 /*  7050 */   265,  266,  267,  268,  269,  270,  271,  272,  273,  667,
 /*  7060 */   667,  667,  667,  667,  667,  667,  667,  205,  667,  234,
 /*  7070 */   667,  667,  667,  667,  667,  204,  193,  195,  198,  197,
 /*  7080 */   309,  667,  667,  667,  667,  667,  667,  667,  649,  496,
 /*  7090 */   262,  275,  189,  667,  667,  667,  667,  667,  667,  357,
 /*  7100 */   667,  493,  667,  667,  667,  667,  218,  667,  667,  667,
 /*  7110 */   667,  433,  435,  437,  439,  343,  109,  110,  111,  112,
 /*  7120 */   113,  114,  115,  116,  117,  118,  119,  120,  121,  122,
 /*  7130 */   667,  667,  667,  667,  274,  253,  254,  255,  256,  257,
 /*  7140 */   258,  259,  260,  261,  263,  264,  265,  266,  267,  268,
 /*  7150 */   269,  270,  271,  272,  273,  667,  667,  667,  344,  345,
 /*  7160 */   346,  347,  348,  349,  667,  667,  373,  374,  667,  667,
 /*  7170 */   667,  667,  667,  667,  667,  667,  313,  406,  667,  667,
 /*  7180 */   667,  203,  667,  667,  206,  496,  262,  275,  667,  667,
 /*  7190 */   667,  667,  667,  343,  667,  667,  667,  493,  667,  202,
 /*  7200 */   667,  667,  667,  667,  667,  196,  667,  433,  435,  437,
 /*  7210 */   439,  274,  253,  254,  255,  256,  257,  258,  259,  260,
 /*  7220 */   261,  263,  264,  265,  266,  267,  268,  269,  270,  271,
 /*  7230 */   272,  273,  667,  667,  667,  667,  344,  345,  346,  347,
 /*  7240 */   348,  349,  667,  667,  667,  663,  205,  667,  667,  667,
 /*  7250 */   667,  667,  667,  321,  204,  193,  195,  198,  197,  667,
 /*  7260 */   667,  667,  496,  262,  275,  667,  667,  667,  667,  667,
 /*  7270 */   667,  667,  667,  667,  493,  667,  667,  667,  667,  667,
 /*  7280 */   667,  667,  667,  667,  433,  435,  437,  439,  653,  659,
 /*  7290 */   660,  667,  109,  110,  111,  112,  113,  114,  115,  116,
 /*  7300 */   117,  118,  119,  120,  121,  122,  667,  274,  253,  254,
 /*  7310 */   255,  256,  257,  258,  259,  260,  261,  263,  264,  265,
 /*  7320 */   266,  267,  268,  269,  270,  271,  272,  273,  667,  667,
 /*  7330 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7340 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  328,
 /*  7350 */   667,  667,  667,  667,  667,  667,  667,  667,  496,  262,
 /*  7360 */   275,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7370 */   493,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7380 */   433,  435,  437,  439,  274,  253,  254,  255,  256,  257,
 /*  7390 */   258,  259,  260,  261,  263,  264,  265,  266,  267,  268,
 /*  7400 */   269,  270,  271,  272,  273,  667,  667,  667,  667,  667,
 /*  7410 */   667,  667,  667,  667,  667,  234,  667,  667,  667,  667,
 /*  7420 */   667,  667,  667,  667,  667,  667,  335,  667,  667,  667,
 /*  7430 */   667,  667,  667,  667,  691,  496,  262,  275,  189,  667,
 /*  7440 */   667,  667,  667,  667,  667,  667,  667,  493,  667,  667,
 /*  7450 */   667,  667,  218,  667,  667,  667,  667,  433,  435,  437,
 /*  7460 */   439,  667,  109,  110,  111,  112,  113,  114,  115,  116,
 /*  7470 */   117,  118,  119,  120,  121,  122,  667,  667,  667,  667,
 /*  7480 */   274,  253,  254,  255,  256,  257,  258,  259,  260,  261,
 /*  7490 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  7500 */   273,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7510 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7520 */   667,  667,  501,  667,  667,  667,  667,  667,  667,  667,
 /*  7530 */   667,  496,  262,  275,  667,  667,  667,  667,  667,  667,
 /*  7540 */   667,  667,  667,  493,  667,  667,  667,  667,  667,  667,
 /*  7550 */   667,  667,  667,  433,  435,  437,  439,  274,  253,  254,
 /*  7560 */   255,  256,  257,  258,  259,  260,  261,  263,  264,  265,
 /*  7570 */   266,  267,  268,  269,  270,  271,  272,  273,  667,  667,
 /*  7580 */   667,  667,  667,  667,  667,  667,  667,  667,  234,  667,
 /*  7590 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  508,
 /*  7600 */   667,  667,  667,  667,  667,  667,  667,  746,  496,  262,
 /*  7610 */   275,  189,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7620 */   493,  667,  667,  667,  667,  218,  667,  667,  667,  667,
 /*  7630 */   433,  435,  437,  439,  667,  109,  110,  111,  112,  113,
 /*  7640 */   114,  115,  116,  117,  118,  119,  120,  121,  122,  667,
 /*  7650 */   667,  667,  667,  274,  253,  254,  255,  256,  257,  258,
 /*  7660 */   259,  260,  261,  263,  264,  265,  266,  267,  268,  269,
 /*  7670 */   270,  271,  272,  273,  667,  667,  667,  667,  667,  667,
 /*  7680 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7690 */   667,  667,  667,  667,  667,  514,  667,  667,  667,  667,
 /*  7700 */   667,  667,  667,  667,  496,  262,  275,  667,  667,  667,
 /*  7710 */   667,  667,  667,  667,  667,  667,  493,  667,  667,  667,
 /*  7720 */   667,  667,  667,  667,  667,  667,  433,  435,  437,  439,
 /*  7730 */   274,  253,  254,  255,  256,  257,  258,  259,  260,  261,
 /*  7740 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  7750 */   273,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7760 */   667,  234,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7770 */   667,  667,  520,  667,  667,  667,  667,  667,  667,  667,
 /*  7780 */   778,  496,  262,  275,  189,  667,  667,  667,  667,  667,
 /*  7790 */   667,  667,  667,  493,  667,  667,  667,  667,  218,  667,
 /*  7800 */   667,  667,  667,  433,  435,  437,  439,  667,  109,  110,
 /*  7810 */   111,  112,  113,  114,  115,  116,  117,  118,  119,  120,
 /*  7820 */   121,  122,  667,  667,  667,  667,  274,  253,  254,  255,
 /*  7830 */   256,  257,  258,  259,  260,  261,  263,  264,  265,  266,
 /*  7840 */   267,  268,  269,  270,  271,  272,  273,  667,  667,  667,
 /*  7850 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  7860 */   667,  667,  667,  667,  667,  667,  667,  667,  537,  667,
 /*  7870 */   667,  667,  667,  667,  667,  667,  667,  496,  262,  275,
 /*  7880 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  493,
 /*  7890 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  433,
 /*  7900 */   435,  437,  439,  274,  253,  254,  255,  256,  257,  258,
 /*  7910 */   259,  260,  261,  263,  264,  265,  266,  267,  268,  269,
 /*  7920 */   270,  271,  272,  273,  667,  667,  667,  667,  667,  667,
 /*  7930 */   667,  667,  667,  667,  234,  667,  667,  667,  667,  667,
 /*  7940 */   667,  667,  667,  667,  667,  544,  667,  667,  667,  667,
 /*  7950 */   667,  667,  667,  881,  496,  262,  275,  189,  667,  667,
 /*  7960 */   667,  667,  667,  667,  667,  667,  493,  667,  667,  667,
 /*  7970 */   667,  218,  667,  667,  667,  667,  433,  435,  437,  439,
 /*  7980 */   667,  109,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  7990 */   118,  119,  120,  121,  122,  667,  667,  667,  667,  274,
 /*  8000 */   253,  254,  255,  256,  257,  258,  259,  260,  261,  263,
 /*  8010 */   264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
 /*  8020 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8030 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8040 */   667,  549,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8050 */   496,  262,  275,  667,  667,  667,  667,  667,  667,  667,
 /*  8060 */   667,  667,  493,  667,  667,  667,  667,  667,  667,  667,
 /*  8070 */   667,  667,  433,  435,  437,  439,  274,  253,  254,  255,
 /*  8080 */   256,  257,  258,  259,  260,  261,  263,  264,  265,  266,
 /*  8090 */   267,  268,  269,  270,  271,  272,  273,  667,  667,  667,
 /*  8100 */   667,  667,  667,  667,  667,  667,  667,  234,  667,  667,
 /*  8110 */   667,  667,  667,  667,  667,  667,  667,  667,  560,  667,
 /*  8120 */   667,  667,  667,  667,  667,  667,  895,  496,  262,  275,
 /*  8130 */   189,  667,  667,  667,  667,  667,  667,  667,  667,  493,
 /*  8140 */   667,  667,  667,  667,  218,  667,  667,  667,  667,  433,
 /*  8150 */   435,  437,  439,  667,  109,  110,  111,  112,  113,  114,
 /*  8160 */   115,  116,  117,  118,  119,  120,  121,  122,  667,  667,
 /*  8170 */   667,  667,  274,  253,  254,  255,  256,  257,  258,  259,
 /*  8180 */   260,  261,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  8190 */   271,  272,  273,  667,  667,  667,  667,  667,  667,  667,
 /*  8200 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8210 */   667,  667,  667,  667,  568,  667,  667,  667,  667,  667,
 /*  8220 */   667,  667,  667,  496,  262,  275,  667,  667,  667,  667,
 /*  8230 */   667,  667,  667,  667,  667,  493,  667,  667,  667,  667,
 /*  8240 */   667,  667,  667,  667,  667,  433,  435,  437,  439,  274,
 /*  8250 */   253,  254,  255,  256,  257,  258,  259,  260,  261,  263,
 /*  8260 */   264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
 /*  8270 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8280 */   234,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8290 */   667,  749,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8300 */   496,  262,  275,  189,  667,  667,  667,  667,  667,  667,
 /*  8310 */   667,  667,  493,  667,  667,  667,  667,  218,  667,  667,
 /*  8320 */   667,  667,  433,  435,  437,  439,  667,  109,  110,  111,
 /*  8330 */   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,
 /*  8340 */   122,  667,  667,  667,  667,  274,  253,  254,  255,  256,
 /*  8350 */   257,  258,  259,  260,  261,  263,  264,  265,  266,  267,
 /*  8360 */   268,  269,  270,  271,  272,  273,  667,  667,  667,  667,
 /*  8370 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8380 */   667,  667,  667,  667,  667,  667,  667,  756,  667,  667,
 /*  8390 */   667,  667,  667,  667,  667,  667,  496,  262,  275,  667,
 /*  8400 */   667,  667,  667,  667,  667,  667,  667,  667,  493,  667,
 /*  8410 */   667,  667,  667,  667,  667,  667,  667,  667,  433,  435,
 /*  8420 */   437,  439,  274,  253,  254,  255,  256,  257,  258,  259,
 /*  8430 */   260,  261,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  8440 */   271,  272,  273,  667,  667,  667,  667,  667,  667,  667,
 /*  8450 */   667,  667,  667,  667,  667,  663,  667,  667,  667,  667,
 /*  8460 */   667,  667,  667,  667,  763,  667,  667,  667,  667,  667,
 /*  8470 */   667,  667,  667,  496,  262,  275,  667,  667,  667,  667,
 /*  8480 */   667,  667,  667,  667,  667,  493,  667,  667,  667,  667,
 /*  8490 */   667,  667,  667,  667,  667,  433,  435,  437,  439,  659,
 /*  8500 */   660,  667,  109,  110,  111,  112,  113,  114,  115,  116,
 /*  8510 */   117,  118,  119,  120,  121,  122,  667,  667,  274,  253,
 /*  8520 */   254,  255,  256,  257,  258,  259,  260,  261,  263,  264,
 /*  8530 */   265,  266,  267,  268,  269,  270,  271,  272,  273,  667,
 /*  8540 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8550 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8560 */   770,  667,  667,  667,  667,  667,  667,  667,  667,  496,
 /*  8570 */   262,  275,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8580 */   667,  493,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8590 */   667,  433,  435,  437,  439,  274,  253,  254,  255,  256,
 /*  8600 */   257,  258,  259,  260,  261,  263,  264,  265,  266,  267,
 /*  8610 */   268,  269,  270,  271,  272,  273,  667,  667,  667,  667,
 /*  8620 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8630 */   667,  667,  667,  667,  667,  667,  667,  781,  667,  667,
 /*  8640 */   667,  667,  667,  667,  667,  667,  496,  262,  275,  667,
 /*  8650 */   667,  667,  667,  667,  667,  667,  667,  667,  493,  667,
 /*  8660 */   667,  667,  667,  667,  667,  667,  667,  667,  433,  435,
 /*  8670 */   437,  439,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8680 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8690 */   667,  274,  253,  254,  255,  256,  257,  258,  259,  260,
 /*  8700 */   261,  263,  264,  265,  266,  267,  268,  269,  270,  271,
 /*  8710 */   272,  273,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8720 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8730 */   667,  667,  667,  788,  667,  667,  667,  667,  667,  667,
 /*  8740 */   667,  667,  496,  262,  275,  667,  667,  667,  667,  667,
 /*  8750 */   667,  667,  667,  667,  493,  667,  667,  667,  667,  667,
 /*  8760 */   667,  667,  667,  667,  433,  435,  437,  439,  274,  253,
 /*  8770 */   254,  255,  256,  257,  258,  259,  260,  261,  263,  264,
 /*  8780 */   265,  266,  267,  268,  269,  270,  271,  272,  273,  667,
 /*  8790 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8800 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8810 */   795,  667,  667,  667,  667,  667,  667,  667,  667,  496,
 /*  8820 */   262,  275,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8830 */   667,  493,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8840 */   667,  433,  435,  437,  439,  667,  667,  667,  667,  667,
 /*  8850 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8860 */   667,  667,  667,  667,  274,  253,  254,  255,  256,  257,
 /*  8870 */   258,  259,  260,  261,  263,  264,  265,  266,  267,  268,
 /*  8880 */   269,  270,  271,  272,  273,  667,  667,  667,  667,  667,
 /*  8890 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8900 */   667,  667,  667,  667,  667,  667,  802,  667,  667,  667,
 /*  8910 */   667,  667,  667,  667,  667,  496,  262,  275,  667,  667,
 /*  8920 */   667,  667,  667,  667,  667,  667,  667,  493,  667,  667,
 /*  8930 */   667,  667,  667,  667,  667,  667,  667,  433,  435,  437,
 /*  8940 */   439,  274,  253,  254,  255,  256,  257,  258,  259,  260,
 /*  8950 */   261,  263,  264,  265,  266,  267,  268,  269,  270,  271,
 /*  8960 */   272,  273,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8970 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  8980 */   667,  667,  667,  883,  667,  667,  667,  667,  667,  667,
 /*  8990 */   667,  667,  496,  262,  275,  667,  667,  667,  667,  667,
 /*  9000 */   667,  667,  667,  667,  493,  667,  667,  667,  667,  667,
 /*  9010 */   667,  667,  667,  667,  433,  435,  437,  439,  667,  667,
 /*  9020 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9030 */   667,  667,  667,  667,  667,  667,  667,  274,  253,  254,
 /*  9040 */   255,  256,  257,  258,  259,  260,  261,  263,  264,  265,
 /*  9050 */   266,  267,  268,  269,  270,  271,  272,  273,  667,  667,
 /*  9060 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9070 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  889,
 /*  9080 */   667,  667,  667,  667,  667,  667,  667,  667,  496,  262,
 /*  9090 */   275,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9100 */   493,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9110 */   433,  435,  437,  439,  274,  253,  254,  255,  256,  257,
 /*  9120 */   258,  259,  260,  261,  263,  264,  265,  266,  267,  268,
 /*  9130 */   269,  270,  271,  272,  273,  667,  667,  667,  667,  667,
 /*  9140 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9150 */   667,  667,  667,  667,  667,  667,  897,  667,  667,  667,
 /*  9160 */   667,  667,  667,  667,  667,  496,  262,  275,  667,  667,
 /*  9170 */   667,  667,  667,  667,  667,  667,  667,  493,  667,  667,
 /*  9180 */   667,  667,  667,  667,  667,  667,  667,  433,  435,  437,
 /*  9190 */   439,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9200 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9210 */   274,  253,  254,  255,  256,  257,  258,  259,  260,  261,
 /*  9220 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  9230 */   273,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9240 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9250 */   667,  667,  903,  667,  667,  667,  667,  667,  667,  667,
 /*  9260 */   667,  496,  262,  275,  667,  667,  667,  667,  667,  667,
 /*  9270 */   667,  667,  667,  493,  667,  667,  667,  667,  667,  667,
 /*  9280 */   667,  667,  667,  433,  435,  437,  439,  274,  253,  254,
 /*  9290 */   255,  256,  257,  258,  259,  260,  261,  263,  264,  265,
 /*  9300 */   266,  267,  268,  269,  270,  271,  272,  273,  667,  104,
 /*  9310 */   667,  667,  102,   90,   88,   94,   92,   96,   98,  100,
 /*  9320 */    46,   52,   58,   61,   64,   67,   55,   49,   76,   78,
 /*  9330 */    86,   80,   82,   84,  667,  667,  667,  667,  252,  262,
 /*  9340 */   275,  667,   73,   69,  667,  667,  667,  667,  667,  667,
 /*  9350 */   493,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9360 */   433,  435,  437,  439,  667,  667,  667,  667,   53,  667,
 /*  9370 */    62,   65,  667,  667,  667,  667,  182,  667,  667,  667,
 /*  9380 */   667,  667,  128,  667,  667,  667,  667,  667,  667,   47,
 /*  9390 */   106,  667,  667,  667,   50,  139,   59,  144,   56,  164,
 /*  9400 */    44,  629,  154,  667,   53,  136,   62,   65,  667,  667,
 /*  9410 */   667,  667,  182,  667,  667,  171,  667,  667,  128,  667,
 /*  9420 */   153,  184,  108,  667,  667,   47,  150,  667,  667,  667,
 /*  9430 */    50,  139,   59,  144,   56,  164,   44,  606,  154,  667,
 /*  9440 */   667,  136,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9450 */   667,  171,  667,  667,  667,  667,  153,  184,  108,  667,
 /*  9460 */   667,  667,  150,  667,  667,  667,  667,  147,  667,  667,
 /*  9470 */   667,  667,  667,  667,  667,  146,  148,  149,  151,  152,
 /*  9480 */    90,   88,   94,   92,   96,   98,  100,   46,   52,   58,
 /*  9490 */    61,   64,   67,   55,   49,   76,   78,   86,   80,   82,
 /*  9500 */    84,  667,  667,  147,  667,  667,  667,  667,  667,   73,
 /*  9510 */    69,  146,  148,  149,  151,  152,  667,  667,  667,   53,
 /*  9520 */   667,   62,   65,  667,  667,  667,  667,  182,  667,  667,
 /*  9530 */   667,  667,  667,  128,  667,  667,  667,  667,  667,  667,
 /*  9540 */    47,  667,  667,  667,  667,   50,  139,   59,  144,   56,
 /*  9550 */   164,   44,  602,  154,  667,   53,  136,   62,   65,  667,
 /*  9560 */   667,  667,  667,  182,  667,  667,  171,  667,  667,  128,
 /*  9570 */   667,  153,  184,  108,  667,  667,   47,  150,  667,  667,
 /*  9580 */   667,   50,  139,   59,  144,   56,  164,   44,  167,  154,
 /*  9590 */   667,  667,  136,  667,  667,  667,  667,  667,  667,  667,
 /*  9600 */   667,  667,  171,  667,  667,  667,  667,  153,  184,  108,
 /*  9610 */   667,  667,  667,  150,  667,  667,  667,  667,  147,  667,
 /*  9620 */   667,  667,  667,  667,  667,  667,  146,  148,  149,  151,
 /*  9630 */   152,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9640 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9650 */   667,  667,  667,  667,  147,  667,  667,  667,  667,  667,
 /*  9660 */   667,  667,  146,  148,  149,  151,  152,  667,  667,  667,
 /*  9670 */    53,  667,   62,   65,  667,  667,  667,  667,  182,  667,
 /*  9680 */   667,  667,  667,  667,  128,  667,  667,  667,  667,  667,
 /*  9690 */   667,   47,  667,  667,  667,  667,   50,  139,   59,  144,
 /*  9700 */    56,  164,   44,  177,  154,  667,   53,  136,   62,   65,
 /*  9710 */   667,  667,  667,  667,  182,  667,  667,  171,  667,  667,
 /*  9720 */   128,  667,  153,  184,  108,  667,  667,   47,  150,  667,
 /*  9730 */   667,  667,   50,  139,   59,  144,   56,  164,   44,  576,
 /*  9740 */   154,  667,  667,  136,  667,  667,  667,  667,  667,  667,
 /*  9750 */   667,  667,  667,  171,  667,  667,  667,  667,  153,  184,
 /*  9760 */   108,  667,  667,  667,  150,  667,  667,  667,  667,  147,
 /*  9770 */   667,  667,  667,  667,  667,  667,  667,  146,  148,  149,
 /*  9780 */   151,  152,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9790 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9800 */   667,  667,  667,  667,  667,  147,  667,  667,  667,  667,
 /*  9810 */   667,  667,  667,  146,  148,  149,  151,  152,  667,  667,
 /*  9820 */   667,   53,  667,   62,   65,  667,  667,  667,  667,  182,
 /*  9830 */   667,  667,  667,  667,  667,  128,  667,  667,  667,  667,
 /*  9840 */   667,  667,   47,  667,  667,  667,  667,   50,  139,   59,
 /*  9850 */   144,   56,  164,   44,  583,  154,  667,   53,  136,   62,
 /*  9860 */    65,  667,  667,  667,  667,  182,  667,  667,  171,  667,
 /*  9870 */   667,  128,  667,  153,  184,  108,  667,  667,   47,  150,
 /*  9880 */   667,  667,  667,   50,  139,   59,  144,   56,  164,   44,
 /*  9890 */   589,  154,  667,  667,  136,  667,  667,  667,  667,  667,
 /*  9900 */   667,  667,  667,  667,  171,  667,  667,  667,  667,  153,
 /*  9910 */   184,  108,  667,  667,  667,  150,  667,  667,  667,  667,
 /*  9920 */   147,  667,  667,  667,  667,  667,  667,  667,  146,  148,
 /*  9930 */   149,  151,  152,  667,  667,  667,  667,  667,  667,  667,
 /*  9940 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /*  9950 */   667,  667,  667,  667,  667,  667,  147,  667,  667,  667,
 /*  9960 */   667,  667,  667,  667,  146,  148,  149,  151,  152,  667,
 /*  9970 */   667,  667,   53,  667,   62,   65,  667,  667,  667,  667,
 /*  9980 */   182,  667,  667,  667,  667,  667,  128,  667,  667,  667,
 /*  9990 */   667,  667,  667,   47,  667,  667,  667,  667,   50,  139,
 /* 10000 */    59,  144,   56,  164,   44,  613,  154,  667,   53,  136,
 /* 10010 */    62,   65,  667,  667,  667,  667,  182,  667,  667,  171,
 /* 10020 */   667,  667,  128,  667,  153,  184,  108,  667,  667,   47,
 /* 10030 */   150,  667,  667,  667,   50,  139,   59,  144,   56,  164,
 /* 10040 */    44,  619,  154,  667,  667,  136,  667,  667,  667,  667,
 /* 10050 */   667,  667,  667,  667,  667,  171,  667,  667,  667,  667,
 /* 10060 */   153,  184,  108,  667,  667,  667,  150,  667,  667,  667,
 /* 10070 */   667,  147,  667,  667,  667,  667,  667,  667,  667,  146,
 /* 10080 */   148,  149,  151,  152,  667,  667,  667,  667,  667,  667,
 /* 10090 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /* 10100 */   667,  667,  667,  667,  667,  667,  667,  147,  667,  667,
 /* 10110 */   667,  667,  667,  667,  667,  146,  148,  149,  151,  152,
 /* 10120 */   667,  667,  667,  667,  104,  667,  667,  102,   90,   88,
 /* 10130 */    94,   92,   96,   98,  100,   46,   52,   58,   61,   64,
 /* 10140 */    67,   55,   49,   76,   78,   86,   80,   82,   84,  667,
 /* 10150 */   667,   53,  667,   62,   65,  667,  124,   73,   69,  598,
 /* 10160 */   667,  667,  667,  667,  667,  128,  667,  667,  667,  667,
 /* 10170 */   667,  667,   47,  667,  667,  667,  667,   50,  139,   59,
 /* 10180 */   144,   56,  164,   44,  667,  154,  667,   53,  132,   62,
 /* 10190 */    65,  667,  667,  667,  667,  667,  667,  667,  171,  667,
 /* 10200 */   667,  128,  667,  153,  184,  108,  667,  667,   47,  150,
 /* 10210 */   667,  667,  667,   50,  139,   59,  144,   56,  164,   44,
 /* 10220 */   667,  154,  667,  667,  156,  667,  667,  667,  667,  667,
 /* 10230 */   667,  667,  667,  667,  171,  667,  667,  667,  667,  159,
 /* 10240 */   184,  108,  667,  667,  667,  150,  667,  160,  667,  667,
 /* 10250 */   147,  667,  667,  667,  667,  667,  667,  667,  146,  148,
 /* 10260 */   149,  151,  152,  667,  667,  667,  667,  667,  667,  667,
 /* 10270 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /* 10280 */   667,  667,  667,  667,  667,  667,  158,  667,  667,  667,
 /* 10290 */   667,  667,  667,  667,  157,  148,  149,  151,  152,  667,
 /* 10300 */   667,  667,   53,  667,   62,   65,  667,  667,  667,  667,
 /* 10310 */   182,  667,  667,  667,  667,  667,  128,  667,  667,  667,
 /* 10320 */   667,  667,  667,   47,  667,  667,  667,  667,   50,  139,
 /* 10330 */    59,  144,   56,  164,   44,  667,  154,  667,   53,  136,
 /* 10340 */    62,   65,  667,  667,  667,  667,  667,  667,  667,  171,
 /* 10350 */   667,  667,  128,  667,  153,  184,  108,  667,  667,   47,
 /* 10360 */   150,  667,  667,  667,   50,  139,   59,  144,   56,  164,
 /* 10370 */    44,  667,  154,  667,  667,  132,  667,  667,  667,  667,
 /* 10380 */   667,  667,  667,  667,  667,  171,  667,  667,  667,  667,
 /* 10390 */   153,  184,  108,  667,  667,  667,  150,  667,  667,  667,
 /* 10400 */   667,  147,  667,  667,  667,  667,  667,  667,  667,  146,
 /* 10410 */   148,  149,  151,  152,  667,  667,  667,  667,  667,  667,
 /* 10420 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  498,
 /* 10430 */   667,  667,  667,  667,  667,  667,  667,  147,  667,  667,
 /* 10440 */   667,  667,  667,  667,  667,  146,  148,  149,  151,  152,
 /* 10450 */   667,  667,  667,  667,  104,  667,  667,  102,   90,   88,
 /* 10460 */    94,   92,   96,   98,  100,   46,   52,   58,   61,   64,
 /* 10470 */    67,   55,   49,   76,   78,   86,   80,   82,   84,  667,
 /* 10480 */    53,  667,   62,   65,  667,  667,  667,   73,   69,  667,
 /* 10490 */   667,  667,  667,  667,  128,  667,  667,  667,  667,  667,
 /* 10500 */   334,   47,  667,  667,  667,  667,   50,  139,   59,  144,
 /* 10510 */    56,  164,   44,  667,  154,  667,  667,  132,  667,  667,
 /* 10520 */   667,  667,  667,  667,  667,  667,  667,  171,  667,  667,
 /* 10530 */   667,  667,  153,  184,  108,  667,  667,  667,  150,  667,
 /* 10540 */   358,  104,  667,  667,  102,   90,   88,   94,   92,   96,
 /* 10550 */    98,  100,   46,   52,   58,   61,   64,   67,   55,   49,
 /* 10560 */    76,   78,   86,   80,   82,   84,  667,  667,  667,  667,
 /* 10570 */   667,  667,  667,  667,   73,   69,  667,  667,  667,  147,
 /* 10580 */   667,  667,   53,  667,   62,   65,  667,  146,  148,  149,
 /* 10590 */   151,  152,  667,  667,  667,  667,  128,  667,  667,  667,
 /* 10600 */  1218,  667,  667,   47,  667,  667,  667,  667,   50,  139,
 /* 10610 */    59,  144,   56,  164,   44,  667,  154,  667,   53,  132,
 /* 10620 */    62,   65,  667,  667,  667,  667,  667,  667,  667,  171,
 /* 10630 */   667,  667,  128,  667,  153,  184,  108,  667,  667,   47,
 /* 10640 */   150,  667,  368,  667,   50,  139,   59,  144,   56,  164,
 /* 10650 */    44,  667,  154,  667,  667,  132,  667,  667,  667,  667,
 /* 10660 */   667,  667,  667,  667,  667,  171,  667,  667,  667,  667,
 /* 10670 */   153,  184,  108,  667,  667,  667,  150,  667,  389,  667,
 /* 10680 */   667,  147,  667,  667,   53,  667,   62,   65,  667,  146,
 /* 10690 */   148,  149,  151,  152,  667,  667,  667,  667,  128,  667,
 /* 10700 */   667,  667,  667,  667,  667,   47,  667,  667,  667,  667,
 /* 10710 */    50,  139,   59,  144,   56,  164,   44,  147,  154,  667,
 /* 10720 */    53,  132,   62,   65,  667,  146,  148,  149,  151,  152,
 /* 10730 */   667,  171,  667,  667,  128,  667,  153,  184,  108,  667,
 /* 10740 */   667,   47,  150,  667,  396,  667,   50,  139,   59,  144,
 /* 10750 */    56,  164,   44,  667,  154,  667,  667,  132,  667,  667,
 /* 10760 */   667,  667,  667,  667,  667,  667,  667,  171,  667,  667,
 /* 10770 */   667,  667,  153,  184,  108,  667,  667,  667,  150,  667,
 /* 10780 */   400,  667,  667,  147,  667,  667,   53,  667,   62,   65,
 /* 10790 */   667,  146,  148,  149,  151,  152,  667,  667,  667,  667,
 /* 10800 */   128,  667,  667,  667,  667,  667,  667,   47,  667,  667,
 /* 10810 */   667,  667,   50,  139,   59,  144,   56,  164,   44,  147,
 /* 10820 */   154,  667,   53,  132,   62,   65,  667,  146,  148,  149,
 /* 10830 */   151,  152,  667,  171,  667,  667,  128,  667,  153,  184,
 /* 10840 */   108,  667,  667,   47,  150,  667,  407,  667,   50,  139,
 /* 10850 */    59,  144,   56,  164,   44,  667,  154,  667,  667,  132,
 /* 10860 */   444,  667,  667,  667,  667,  667,  667,  667,  667,  171,
 /* 10870 */   667,  667,  667,  667,  153,  184,  108,  667,  667,  667,
 /* 10880 */   150,  667,  667,  667,  667,  147,  667,  667,  667,  667,
 /* 10890 */   667,  667,  667,  146,  148,  149,  151,  152,  667,  667,
 /* 10900 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /* 10910 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /* 10920 */   667,  147,  667,  667,  667,  667,  667,  667,  667,  146,
 /* 10930 */   148,  149,  151,  152,  667,  667,  667,  667,  104,  667,
 /* 10940 */   667,  102,   90,   88,   94,   92,   96,   98,  100,   46,
 /* 10950 */    52,   58,   61,   64,   67,   55,   49,   76,   78,   86,
 /* 10960 */    80,   82,   84,  667,  667,  667,  667,  667,  667,  667,
 /* 10970 */   667,   73,   69,  667,  667,  443,  667,  667,  667,  667,
 /* 10980 */   104,  667,  667,  102,   90,   88,   94,   92,   96,   98,
 /* 10990 */   100,   46,   52,   58,   61,   64,   67,   55,   49,   76,
 /* 11000 */    78,   86,   80,   82,   84,  667,  667,  667,  667,  667,
 /* 11010 */   667,  667,  667,   73,   69,  667,  667,  447,  667,  667,
 /* 11020 */   667,  667,  104,  667,  667,  102,   90,   88,   94,   92,
 /* 11030 */    96,   98,  100,   46,   52,   58,   61,   64,   67,   55,
 /* 11040 */    49,   76,   78,   86,   80,   82,   84,  667,  667,  667,
 /* 11050 */   667,  667,  667,  667,  667,   73,   69,  667,  667,  450,
 /* 11060 */   667,  667,  667,  667,  104,  667,  667,  102,   90,   88,
 /* 11070 */    94,   92,   96,   98,  100,   46,   52,   58,   61,   64,
 /* 11080 */    67,   55,   49,   76,   78,   86,   80,   82,   84,  667,
 /* 11090 */   667,  667,  667,  667,  667,  667,  667,   73,   69,  667,
 /* 11100 */   667,  453,  667,  667,  667,  667,  104,  667,  667,  102,
 /* 11110 */    90,   88,   94,   92,   96,   98,  100,   46,   52,   58,
 /* 11120 */    61,   64,   67,   55,   49,   76,   78,   86,   80,   82,
 /* 11130 */    84,  667,   53,  667,   62,   65,  667,  667,  667,   73,
 /* 11140 */    69,  667,  667,  667,  667,  667,  128,  667,  667,  667,
 /* 11150 */   667,  667,  500,   47,  667,  667,  667,  667,   50,  139,
 /* 11160 */    59,  144,   56,  164,   44,  667,  154,  667,  667,  132,
 /* 11170 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  171,
 /* 11180 */   667,  667,  667,  667,  153,  184,  108,  667,  104,  667,
 /* 11190 */   150,  102,   90,   88,   94,   92,   96,   98,  100,   46,
 /* 11200 */    52,   58,   61,   64,   67,   55,   49,   76,   78,   86,
 /* 11210 */    80,   82,   84,  667,  667,  667,  667,  667,  667,  667,
 /* 11220 */   667,   73,   69,  511,  667,  667,  667,  667,  667,  667,
 /* 11230 */   667,  147,  667,  667,  507,  667,  667,  667,  667,  146,
 /* 11240 */   148,  149,  151,  152,  667,  667,  667,  667,  104,  667,
 /* 11250 */   667,  102,   90,   88,   94,   92,   96,   98,  100,   46,
 /* 11260 */    52,   58,   61,   64,   67,   55,   49,   76,   78,   86,
 /* 11270 */    80,   82,   84,  667,   53,  667,   62,   65,  667,  667,
 /* 11280 */   667,   73,   69,  667,  667,  667,  667,  667,  128,  667,
 /* 11290 */   667,  667,  667,  667,  513,   47,  667,  667,  667,  667,
 /* 11300 */    50,  139,   59,  144,   56,  164,   44,  667,  154,  667,
 /* 11310 */   667,  132,  667,  667,  667,  667,  667,  667,  667,  667,
 /* 11320 */   667,  171,  667,  667,  667,  667,  153,  184,  108,  667,
 /* 11330 */   104,  667,  150,  102,   90,   88,   94,   92,   96,   98,
 /* 11340 */   100,   46,   52,   58,   61,   64,   67,   55,   49,   76,
 /* 11350 */    78,   86,   80,   82,   84,  667,  667,  667,  667,  667,
 /* 11360 */   667,  667,  667,   73,   69,  667,  667,  667,  667,  667,
 /* 11370 */   667,  667,   53,  147,   62,   65,  667,  667,  667,  667,
 /* 11380 */   667,  146,  148,  149,  151,  152,  128,  667,  667,  667,
 /* 11390 */   667,  667,  667,   47,  667,  667,  667,  667,   50,  139,
 /* 11400 */    59,  144,   56,  164,   44,  667,  154,  667,  667,  156,
 /* 11410 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  171,
 /* 11420 */   667,  667,  667,  667,  159,  184,  108,  667,  667,  667,
 /* 11430 */   150,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /* 11440 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /* 11450 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /* 11460 */   667,  667,  667,  667,  667,  667,  667,  667,  667,  667,
 /* 11470 */   667,  158,  667,  667,  667,  667,  667,  667,  667,  157,
 /* 11480 */   148,  149,  151,  152,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */     7,  164,    9,   10,  155,   61,    6,  158,  159,  160,
 /*    10 */    44,  162,  163,  171,   21,   41,   42,  168,  169,  177,
 /*    20 */   178,   28,  180,  174,   44,   59,   33,   34,   35,   36,
 /*    30 */    37,   38,   39,   89,   41,   55,   56,   44,   45,    1,
 /*    40 */     2,    3,    4,    5,  154,   45,  156,   54,   55,   56,
 /*    50 */   171,  107,   59,   60,   61,  218,  177,  178,   65,  180,
 /*    60 */     0,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*    70 */    77,   78,  171,  126,   44,   82,   83,   84,  177,  178,
 /*    80 */    87,  180,  177,  178,   91,   92,   93,   94,   47,   96,
 /*    90 */    52,   53,   99,   55,   56,   55,   58,   44,  151,  106,
 /*   100 */    62,   63,   61,  110,  111,  112,  113,  114,  115,  116,
 /*   110 */   117,  118,  119,    7,   45,    9,   10,   27,   28,   29,
 /*   120 */    30,   31,   32,  177,  178,  193,  180,   21,   88,  197,
 /*   130 */    90,   41,   42,  164,   28,  203,  204,  205,  206,   33,
 /*   140 */    34,   35,   36,   37,   38,   39,   42,   41,  107,   45,
 /*   150 */    44,   45,    1,    2,    3,    4,    5,   44,   54,  106,
 /*   160 */    54,   55,   56,  171,   61,   59,   60,   61,   39,  177,
 /*   170 */   178,   65,  180,   44,   68,   69,   70,   71,   72,   73,
 /*   180 */    74,   75,   76,   77,   78,  216,  217,  218,   82,   83,
 /*   190 */    84,   44,   89,   87,  126,   85,   86,   91,   92,   93,
 /*   200 */    94,   54,   96,   52,   53,   99,   55,   56,  196,   58,
 /*   210 */   107,  199,  106,   62,   63,   61,  110,  111,  112,  113,
 /*   220 */   114,  115,  116,  117,  118,  119,    7,  160,    9,   10,
 /*   230 */   163,  196,  193,  198,  199,  168,  169,   55,   44,    6,
 /*   240 */    21,  174,  203,  204,  205,  206,   44,   28,   54,  210,
 /*   250 */   211,    6,   33,   34,   35,   36,   37,   38,   39,   14,
 /*   260 */    41,  107,   60,   44,   45,    1,    2,    3,    4,    5,
 /*   270 */    88,   44,   90,   54,   55,   56,  171,   47,   59,   60,
 /*   280 */    61,   54,  177,  178,   65,  180,    6,   68,   69,   70,
 /*   290 */    71,   72,   73,   74,   75,   76,   77,   78,  163,   54,
 /*   300 */    67,   82,   83,   84,  169,  196,   87,  198,  199,  174,
 /*   310 */    91,   92,   93,   94,   44,   96,   52,   53,   99,   55,
 /*   320 */    56,  196,   58,  198,  199,  106,   62,   63,   61,  110,
 /*   330 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*   340 */    42,    9,   10,   45,   44,  193,    6,   67,  176,  177,
 /*   350 */   178,  179,   54,   21,   54,  203,  204,  205,  206,   49,
 /*   360 */    28,   51,  210,  211,   54,   33,   34,   35,   36,   37,
 /*   370 */    38,   39,   42,   41,  107,   45,   44,   45,    1,    2,
 /*   380 */     3,    4,    5,  163,   54,    6,   54,   55,   56,  171,
 /*   390 */   170,   59,   60,   61,  174,  177,  178,   65,  180,   61,
 /*   400 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*   410 */    78,  163,   39,   44,   82,   83,   84,   44,   89,   87,
 /*   420 */   185,  186,  174,   91,   92,   93,   94,   40,   96,   52,
 /*   430 */    53,   99,   55,   56,   55,   58,  107,   97,  106,   62,
 /*   440 */    63,   22,  110,  111,  112,  113,  114,  115,  116,  117,
 /*   450 */   118,  119,    7,    6,    9,   10,  171,    6,  193,   49,
 /*   460 */    41,   51,  177,  178,   54,  180,   21,   49,  203,  204,
 /*   470 */   205,  206,   54,   28,   22,  210,  211,    6,   33,   34,
 /*   480 */    35,   36,   37,   38,   39,   42,   41,   40,   45,   44,
 /*   490 */    45,    1,    2,    3,    4,    5,   45,   54,   61,   54,
 /*   500 */    55,   56,   85,   86,   59,   60,   61,  187,  188,  196,
 /*   510 */    65,   40,  199,   68,   69,   70,   71,   72,   73,   74,
 /*   520 */    75,   76,   77,   78,  189,  190,   44,   82,   83,   84,
 /*   530 */   191,  192,   87,    6,    6,    6,   91,   92,   93,   94,
 /*   540 */    61,   96,   52,   53,   99,   55,   56,  196,   58,   44,
 /*   550 */   199,  106,   62,   63,   44,  110,  111,  112,  113,  114,
 /*   560 */   115,  116,  117,  118,  119,    7,   45,    9,   10,   40,
 /*   570 */    89,  193,   49,   45,   51,   54,    6,   54,   57,   21,
 /*   580 */    44,  203,  204,  205,  206,    6,   28,   42,  210,  211,
 /*   590 */    45,   33,   34,   35,   36,   37,   38,   39,   89,   41,
 /*   600 */   194,  195,   44,   45,    1,    2,    3,    4,    5,  207,
 /*   610 */   208,  106,   54,   55,   56,   45,  106,   59,   60,   61,
 /*   620 */   207,  208,   42,   65,   45,   45,   68,   69,   70,   71,
 /*   630 */    72,   73,   74,   75,   76,   77,   78,  207,  208,   89,
 /*   640 */    82,   83,   84,    6,    6,   87,  207,  208,    6,   91,
 /*   650 */    92,   93,   94,   55,   96,   52,   53,   99,   55,   56,
 /*   660 */    61,   58,  207,  208,  106,   62,   63,   57,  110,  111,
 /*   670 */   112,  113,  114,  115,  116,  117,  118,  119,    7,   45,
 /*   680 */     9,   10,   45,   45,  193,  207,  208,   45,   54,    6,
 /*   690 */     6,   57,   21,  107,  203,  204,  205,  206,    6,   28,
 /*   700 */    42,  210,  211,   45,   33,   34,   35,   36,   37,   38,
 /*   710 */    39,    6,   41,  207,  208,   44,   45,    1,    2,    3,
 /*   720 */     4,    5,  207,  208,  164,   54,   55,   56,   45,   45,
 /*   730 */    59,   60,   61,  207,  208,    6,   65,   45,   59,   68,
 /*   740 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   750 */    44,   44,   57,   82,   83,   84,  207,  208,   87,   54,
 /*   760 */    54,   44,   91,   92,   93,   94,   59,   96,   52,   53,
 /*   770 */    99,   55,   56,   56,   45,  207,  208,  106,   62,   63,
 /*   780 */    89,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*   790 */   119,    7,   45,    9,   10,  100,  101,  102,  103,  104,
 /*   800 */   105,   54,    6,    6,  173,   21,  175,  176,  177,  178,
 /*   810 */   179,   89,   28,   42,  108,  109,   45,   33,   34,   35,
 /*   820 */    36,   37,   38,   39,   89,   41,   85,   86,   44,   45,
 /*   830 */     1,    2,    3,    4,    5,   30,   31,   32,   54,   55,
 /*   840 */    56,   45,   45,   59,   60,   61,   41,   42,   57,   65,
 /*   850 */   185,  186,   68,   69,   70,   71,   72,   73,   74,   75,
 /*   860 */    76,   77,   78,   85,   86,  164,   82,   83,   84,   57,
 /*   870 */    51,   87,   44,   54,    6,   91,   92,   93,   94,   45,
 /*   880 */    96,   52,   53,   99,   55,   56,   51,   59,   54,   54,
 /*   890 */   106,   62,   63,   44,  110,  111,  112,  113,  114,  115,
 /*   900 */   116,  117,  118,  119,    7,    6,    9,   10,   40,  164,
 /*   910 */   173,  193,  175,  176,  177,  178,  179,  163,   21,  201,
 /*   920 */   202,  203,  204,  205,  206,   28,    6,    6,  174,    6,
 /*   930 */    33,   34,   35,   36,   37,   38,   39,  163,   41,   40,
 /*   940 */    45,   44,   45,    1,    2,    3,    4,    5,  174,   54,
 /*   950 */    45,   54,   55,   56,  166,  167,   59,   60,   61,   54,
 /*   960 */    40,   40,   65,   40,   57,   68,   69,   70,   71,   72,
 /*   970 */    73,   74,   75,   76,   77,   78,   45,   45,  164,   82,
 /*   980 */    83,   84,   44,   57,   87,   54,   54,    6,   91,   92,
 /*   990 */    93,   94,   44,   96,   52,   53,   99,   55,   56,  154,
 /*  1000 */   154,  156,  156,  106,   62,   63,   44,  110,  111,  112,
 /*  1010 */   113,  114,  115,  116,  117,  118,  119,    7,    6,    9,
 /*  1020 */    10,   40,   60,  193,  164,   22,  163,    6,    6,   57,
 /*  1030 */   154,   21,  156,  203,  204,  205,  206,  174,   28,    6,
 /*  1040 */   210,  211,    6,   33,   34,   35,   36,   37,   38,   39,
 /*  1050 */   164,   41,   40,   44,   44,   45,    1,    2,    3,    4,
 /*  1060 */     5,   40,   40,   57,   54,   55,   56,  164,   44,   59,
 /*  1070 */    60,   61,  154,   40,  156,   65,   40,  164,   68,   69,
 /*  1080 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  164,
 /*  1090 */    45,   57,   82,   83,   84,   54,   44,   87,   45,  188,
 /*  1100 */     6,   91,   92,   93,   94,   54,   96,   52,   53,   99,
 /*  1110 */    55,   56,   60,  186,   54,   54,  106,   62,   63,   44,
 /*  1120 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  1130 */     7,    6,    9,   10,   40,   60,  193,  164,   89,   54,
 /*  1140 */   215,  216,  217,  218,   21,   54,  203,  204,  205,  206,
 /*  1150 */    54,   28,   45,  210,  211,    6,   33,   34,   35,   36,
 /*  1160 */    37,   38,   39,   92,   41,   40,   54,   44,   45,    1,
 /*  1170 */     2,    3,    4,    5,   44,   95,  190,   54,   55,   56,
 /*  1180 */    44,   44,   59,   60,   61,  195,   44,   44,   65,   40,
 /*  1190 */    60,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  1200 */    77,   78,   60,   60,  184,   82,   83,   84,   67,  196,
 /*  1210 */    87,  196,   44,  193,   91,   92,   93,   94,   55,   96,
 /*  1220 */    52,   53,   99,  203,  204,  205,  206,  196,   60,  106,
 /*  1230 */    62,   63,   44,  110,  111,  112,  113,  114,  115,  116,
 /*  1240 */   117,  118,  119,    7,   55,    9,   10,   44,   60,  193,
 /*  1250 */   196,   44,   44,  196,  196,  196,   44,   21,  196,  203,
 /*  1260 */   204,  205,  206,   60,   28,   44,  210,  211,   60,   33,
 /*  1270 */    34,   35,   36,   37,   38,   39,   45,   41,   44,   55,
 /*  1280 */    44,   45,    1,    2,    3,    4,    5,   55,  196,  196,
 /*  1290 */    54,   55,   56,   45,   60,   59,   60,   61,   45,   45,
 /*  1300 */    44,   65,   57,  208,   68,   69,   70,   71,   72,   73,
 /*  1310 */    74,   75,   76,   77,   78,  164,   44,  184,   82,   83,
 /*  1320 */    84,   44,   97,   87,   54,   44,  193,   91,   92,   93,
 /*  1330 */    94,  192,   96,   52,   53,   99,  203,  204,  205,  206,
 /*  1340 */    45,   60,  106,   62,   63,   54,  110,  111,  112,  113,
 /*  1350 */   114,  115,  116,  117,  118,  119,    7,   92,    9,   10,
 /*  1360 */    89,   54,  193,   54,   61,   55,   54,   54,   44,   55,
 /*  1370 */    21,   44,  203,  204,  205,  206,   61,   28,   55,  210,
 /*  1380 */   211,   61,   33,   34,   35,   36,   37,   38,   39,   89,
 /*  1390 */    41,   55,   61,   44,   45,    1,    2,    3,    4,    5,
 /*  1400 */    55,   61,   44,   54,   55,   56,   44,  152,   59,   60,
 /*  1410 */    61,   44,  152,   54,   65,   55,  174,   68,   69,   70,
 /*  1420 */    71,   72,   73,   74,   75,   76,   77,   78,   44,   61,
 /*  1430 */    45,   82,   83,   84,   14,   39,   87,   44,   67,   22,
 /*  1440 */    91,   92,   93,   94,   45,   96,   52,   53,   99,   55,
 /*  1450 */    56,   44,   61,   45,   45,  106,   62,   63,   57,  110,
 /*  1460 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  1470 */   164,    9,   10,   45,   45,  193,   57,   44,   57,   45,
 /*  1480 */   164,  164,   57,   21,   45,  203,  204,  205,  206,  164,
 /*  1490 */    28,  153,  210,  211,   44,   33,   34,   35,   36,   37,
 /*  1500 */    38,   39,   44,   41,  153,  153,   44,   45,    1,    2,
 /*  1510 */     3,    4,    5,   44,   55,   61,   54,   55,   56,   44,
 /*  1520 */   165,   59,   60,   61,   61,  164,   45,   65,  165,   45,
 /*  1530 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  1540 */    78,   45,  167,   44,   82,   83,   84,   45,  165,   87,
 /*  1550 */   164,   45,  165,   91,   92,   93,   94,   45,   96,   52,
 /*  1560 */    53,   99,   55,   56,  153,   50,  156,   44,  106,   62,
 /*  1570 */    63,  153,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  1580 */   118,  119,    7,   44,    9,   10,   50,   44,  153,  157,
 /*  1590 */   153,  159,   44,  161,  153,  163,   21,  153,   54,  153,
 /*  1600 */   168,  153,  170,   28,   44,   54,  174,   61,   33,   34,
 /*  1610 */    35,   36,   37,   38,   39,   54,   41,   60,   44,   44,
 /*  1620 */    45,    1,    2,    3,    4,    5,   61,   54,  219,   54,
 /*  1630 */    55,   56,   54,  219,   59,   60,   61,  219,  219,  219,
 /*  1640 */    65,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  1650 */    75,   76,   77,   78,  219,  219,  219,   82,   83,   84,
 /*  1660 */   219,  219,   87,  219,  219,  219,   91,   92,   93,   94,
 /*  1670 */   219,   96,   52,   53,   99,   55,   56,  219,  219,  219,
 /*  1680 */   219,  106,   62,   63,  219,  110,  111,  112,  113,  114,
 /*  1690 */   115,  116,  117,  118,  119,    7,  219,    9,   10,  219,
 /*  1700 */   219,  219,  219,  219,  159,  160,  219,  162,  163,   21,
 /*  1710 */   219,  219,  219,  168,  169,  219,   28,  219,  219,  174,
 /*  1720 */   219,   33,   34,   35,   36,   37,   38,   39,  219,   41,
 /*  1730 */   219,  219,   44,   45,    1,    2,    3,    4,    5,  219,
 /*  1740 */   219,  219,   54,   55,   56,  219,  219,   59,   60,   61,
 /*  1750 */   219,  219,  219,   65,  219,  219,   68,   69,   70,   71,
 /*  1760 */    72,   73,   74,   75,   76,   77,   78,  219,  219,  219,
 /*  1770 */    82,   83,   84,  219,  219,   87,  219,  219,  219,   91,
 /*  1780 */    92,   93,   94,  219,   96,   52,   53,   99,   55,   56,
 /*  1790 */   219,  219,  219,  219,  106,   62,   63,  219,  110,  111,
 /*  1800 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  219,
 /*  1810 */     9,   10,  219,  219,  219,  219,  158,  219,  160,  219,
 /*  1820 */   162,  163,   21,  219,  219,  219,  168,  169,  219,   28,
 /*  1830 */   219,  219,  174,  219,   33,   34,   35,   36,   37,   38,
 /*  1840 */    39,  219,   41,  219,  219,   44,   45,    1,    2,    3,
 /*  1850 */     4,    5,  219,  219,  219,   54,   55,   56,  219,  219,
 /*  1860 */    59,   60,   61,  219,  219,  219,   65,  219,  219,   68,
 /*  1870 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  1880 */   219,  219,  184,   82,   83,   84,  219,  219,   87,  219,
 /*  1890 */   219,  193,   91,   92,   93,   94,  219,   96,   52,   53,
 /*  1900 */    99,  203,  204,  205,  206,  219,   60,  106,   62,   63,
 /*  1910 */   219,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  1920 */   119,    7,  219,    9,   10,  219,  219,  193,  219,  219,
 /*  1930 */   219,  219,  219,  219,  219,   21,  219,  203,  204,  205,
 /*  1940 */   206,  219,   28,  219,  219,  211,  219,   33,   34,   35,
 /*  1950 */    36,   37,   38,   39,  219,   41,  219,  219,   44,   45,
 /*  1960 */     1,    2,    3,    4,    5,  219,  219,  219,   54,   55,
 /*  1970 */    56,  219,  219,   59,   60,   61,  219,  219,  219,   65,
 /*  1980 */   219,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  1990 */    76,   77,   78,  219,  219,  184,   82,   83,   84,  219,
 /*  2000 */   219,   87,  219,  219,  193,   91,   92,   93,   94,  219,
 /*  2010 */    96,   52,   53,   99,  203,  204,  205,  206,  219,   60,
 /*  2020 */   106,   62,   63,  219,  110,  111,  112,  113,  114,  115,
 /*  2030 */   116,  117,  118,  119,    7,  219,    9,   10,  219,  219,
 /*  2040 */   193,  219,  219,  219,  197,  219,  219,  219,   21,  219,
 /*  2050 */   203,  204,  205,  206,  219,   28,  219,  219,  219,  219,
 /*  2060 */    33,   34,   35,   36,   37,   38,   39,  219,   41,  219,
 /*  2070 */   219,   44,   45,    1,    2,    3,    4,    5,  219,  219,
 /*  2080 */   219,   54,   55,   56,  219,  219,   59,   60,   61,  219,
 /*  2090 */   219,  219,   65,  219,  219,   68,   69,   70,   71,   72,
 /*  2100 */    73,   74,   75,   76,   77,   78,  219,  219,  219,   82,
 /*  2110 */    83,   84,  219,  160,   87,  162,  163,  219,   91,   92,
 /*  2120 */    93,   94,  169,   96,   52,   53,   99,  174,  219,  219,
 /*  2130 */    58,  219,  219,  106,   62,   63,  219,  110,  111,  112,
 /*  2140 */   113,  114,  115,  116,  117,  118,  119,    7,  219,    9,
 /*  2150 */    10,  219,  219,  193,  219,  219,  219,  219,  219,  219,
 /*  2160 */   200,   21,  219,  203,  204,  205,  206,  219,   28,  219,
 /*  2170 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /*  2180 */   219,   41,  219,  219,   44,   45,    1,    2,    3,    4,
 /*  2190 */     5,  219,  219,  219,   54,   55,   56,  219,  219,   59,
 /*  2200 */    60,   61,  219,  219,  219,   65,  219,  219,   68,   69,
 /*  2210 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  219,
 /*  2220 */   219,  184,   82,   83,   84,  219,  219,   87,  219,  219,
 /*  2230 */   193,   91,   92,   93,   94,  219,   96,   52,   53,   99,
 /*  2240 */   203,  204,  205,  206,  219,   60,  106,   62,   63,  219,
 /*  2250 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  2260 */     7,  219,    9,   10,  219,  219,  193,  219,  219,  219,
 /*  2270 */   197,  219,  219,  219,   21,  219,  203,  204,  205,  206,
 /*  2280 */   219,   28,  219,  219,  219,  219,   33,   34,   35,   36,
 /*  2290 */    37,   38,   39,  219,   41,  219,  219,   44,   45,    1,
 /*  2300 */     2,    3,    4,    5,  219,  219,  219,   54,   55,   56,
 /*  2310 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /*  2320 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  2330 */    77,   78,  219,  219,  184,   82,   83,   84,  219,  219,
 /*  2340 */    87,  219,  219,  193,   91,   92,   93,   94,  219,   96,
 /*  2350 */    52,   53,   99,  203,  204,  205,  206,  219,   60,  106,
 /*  2360 */    62,   63,  219,  110,  111,  112,  113,  114,  115,  116,
 /*  2370 */   117,  118,  119,    7,  219,    9,   10,  219,  219,  193,
 /*  2380 */   219,  219,  219,  197,  219,  219,  219,   21,  219,  203,
 /*  2390 */   204,  205,  206,  219,   28,  219,  219,  219,  219,   33,
 /*  2400 */    34,   35,   36,   37,   38,   39,  219,   41,  219,  219,
 /*  2410 */    44,   45,    1,    2,    3,    4,    5,  219,  219,  219,
 /*  2420 */    54,   55,   56,  219,  219,   59,   60,   61,  219,  219,
 /*  2430 */   219,   65,  219,  219,   68,   69,   70,   71,   72,   73,
 /*  2440 */    74,   75,   76,   77,   78,  219,  219,  219,   82,   83,
 /*  2450 */    84,  219,  219,   87,  219,  219,  193,   91,   92,   93,
 /*  2460 */    94,  219,   96,   52,   53,   99,  203,  204,  205,  206,
 /*  2470 */   219,  219,  106,   62,   63,  219,  110,  111,  112,  113,
 /*  2480 */   114,  115,  116,  117,  118,  119,    7,  219,    9,   10,
 /*  2490 */   219,  219,  173,  219,  175,  176,  177,  178,  179,  219,
 /*  2500 */    21,  219,  219,  177,  219,  219,  219,   28,  219,  219,
 /*  2510 */   219,  219,   33,   34,   35,   36,   37,   38,   39,  193,
 /*  2520 */    41,  219,  219,   44,   45,  219,  219,  219,  219,  203,
 /*  2530 */   204,  205,  206,   54,   55,   56,  219,  219,   59,   60,
 /*  2540 */    61,  219,  219,  219,   65,  219,  219,   68,   69,   70,
 /*  2550 */    71,   72,   73,   74,   75,   76,   77,   78,  219,  219,
 /*  2560 */   219,   82,   83,   84,  219,  219,   87,  219,  219,  219,
 /*  2570 */    91,   92,   93,   94,  219,   96,  219,  173,   99,  175,
 /*  2580 */   176,  177,  178,  179,  219,  106,  219,  219,  219,  110,
 /*  2590 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  2600 */   219,    9,   10,  219,  219,  193,  219,  219,  219,  197,
 /*  2610 */   219,  219,  219,   21,  219,  203,  204,  205,  206,  219,
 /*  2620 */    28,  219,  219,  219,  219,   33,   34,   35,   36,   37,
 /*  2630 */    38,   39,  193,   41,  219,  219,   44,   45,  219,  219,
 /*  2640 */   219,  202,  203,  204,  205,  206,   54,   55,   56,  219,
 /*  2650 */   219,   59,   60,   61,  219,  219,  219,   65,  219,  219,
 /*  2660 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  2670 */    78,  219,  219,  219,   82,   83,   84,  219,  219,   87,
 /*  2680 */   219,  219,  219,   91,   92,   93,   94,  219,   96,  219,
 /*  2690 */   173,   99,  175,  176,  177,  178,  179,  219,  106,  219,
 /*  2700 */   219,  219,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  2710 */   118,  119,    7,  219,    9,   10,  219,  193,  219,  219,
 /*  2720 */   219,  197,  219,  219,  193,  219,   21,  203,  204,  205,
 /*  2730 */   206,  219,  219,   28,  203,  204,  205,  206,   33,   34,
 /*  2740 */    35,   36,   37,   38,   39,  214,   41,  219,  219,   44,
 /*  2750 */    45,  173,  219,  175,  176,  177,  178,  179,  219,   54,
 /*  2760 */    55,   56,  219,  219,   59,   60,   61,  219,  219,  219,
 /*  2770 */    65,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  2780 */    75,   76,   77,   78,  219,  219,  219,   82,   83,   84,
 /*  2790 */   219,  219,   87,  219,  219,  219,   91,   92,   93,   94,
 /*  2800 */   219,   96,  219,  173,   99,  175,  176,  177,  178,  179,
 /*  2810 */   219,  106,  219,  219,  219,  110,  111,  112,  113,  114,
 /*  2820 */   115,  116,  117,  118,  119,    7,  219,    9,   10,  219,
 /*  2830 */   193,  219,  219,  219,  197,  219,  219,  193,  219,   21,
 /*  2840 */   203,  204,  205,  206,  219,  219,   28,  203,  204,  205,
 /*  2850 */   206,   33,   34,   35,   36,   37,   38,   39,  219,   41,
 /*  2860 */   219,  219,   44,   45,  173,  219,  175,  176,  177,  178,
 /*  2870 */   179,  219,   54,   55,   56,  219,  219,   59,   60,   61,
 /*  2880 */   219,  219,  219,   65,  219,  219,   68,   69,   70,   71,
 /*  2890 */    72,   73,   74,   75,   76,   77,   78,  219,  219,  219,
 /*  2900 */    82,   83,   84,  219,  219,   87,  219,  219,  193,   91,
 /*  2910 */    92,   93,   94,  219,   96,  219,  219,   99,  203,  204,
 /*  2920 */   205,  206,  219,  219,  106,  219,  219,  219,  110,  111,
 /*  2930 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  219,
 /*  2940 */     9,   10,  219,  193,  219,  219,  219,  197,  219,  219,
 /*  2950 */   193,  219,   21,  203,  204,  205,  206,  219,  219,   28,
 /*  2960 */   203,  204,  205,  206,   33,   34,   35,   36,   37,   38,
 /*  2970 */    39,  193,   41,  219,  219,   44,   45,  219,  219,  219,
 /*  2980 */   219,  203,  204,  205,  206,   54,   55,   56,  219,  219,
 /*  2990 */    59,   60,   61,  219,  219,  219,   65,  219,  219,   68,
 /*  3000 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  3010 */   219,  219,  219,   82,   83,   84,  219,  219,   87,  219,
 /*  3020 */   219,  193,   91,   92,   93,   94,  219,   96,  219,  219,
 /*  3030 */    99,  203,  204,  205,  206,  219,  219,  106,  219,  219,
 /*  3040 */   219,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  3050 */   119,    7,  219,    9,   10,  219,  193,  219,  219,  219,
 /*  3060 */   197,  219,  219,  193,  219,   21,  203,  204,  205,  206,
 /*  3070 */   219,  219,   28,  203,  204,  205,  206,   33,   34,   35,
 /*  3080 */    36,   37,   38,   39,  193,   41,  219,  219,   44,   45,
 /*  3090 */   219,  219,  219,  219,  203,  204,  205,  206,   54,   55,
 /*  3100 */    56,  219,  219,   59,   60,   61,  219,  219,  219,   65,
 /*  3110 */   219,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  3120 */    76,   77,   78,  219,  219,  219,   82,   83,   84,  219,
 /*  3130 */   219,   87,  219,  219,  193,   91,   92,   93,   94,  219,
 /*  3140 */    96,  219,  219,   99,  203,  204,  205,  206,  219,  219,
 /*  3150 */   106,  219,  219,  219,  110,  111,  112,  113,  114,  115,
 /*  3160 */   116,  117,  118,  119,    7,  219,    9,   10,  219,  193,
 /*  3170 */   219,  219,  219,  197,  219,  219,  193,  219,   21,  203,
 /*  3180 */   204,  205,  206,  219,  219,   28,  203,  204,  205,  206,
 /*  3190 */    33,   34,   35,   36,   37,   38,   39,  193,   41,  219,
 /*  3200 */   219,   44,   45,  219,  219,  219,  219,  203,  204,  205,
 /*  3210 */   206,   54,   55,   56,  219,  219,   59,   60,   61,  219,
 /*  3220 */   219,  219,   65,  219,  219,   68,   69,   70,   71,   72,
 /*  3230 */    73,   74,   75,   76,   77,   78,  219,  219,  219,   82,
 /*  3240 */    83,   84,  219,  219,   87,  219,  219,  193,   91,   92,
 /*  3250 */    93,   94,  219,   96,  219,  219,   99,  203,  204,  205,
 /*  3260 */   206,  219,  219,  106,  219,  219,  219,  110,  111,  112,
 /*  3270 */   113,  114,  115,  116,  117,  118,  119,    7,  219,    9,
 /*  3280 */    10,  219,  193,  219,  219,  219,  197,  219,  219,  193,
 /*  3290 */   219,   21,  203,  204,  205,  206,  219,  219,   28,  203,
 /*  3300 */   204,  205,  206,   33,   34,   35,   36,   37,   38,   39,
 /*  3310 */   193,   41,  219,  219,   44,   45,  219,  219,  219,  219,
 /*  3320 */   203,  204,  205,  206,   54,   55,   56,  219,  219,   59,
 /*  3330 */    60,   61,  219,  219,  219,   65,  219,  219,   68,   69,
 /*  3340 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  219,
 /*  3350 */   219,  219,   82,   83,   84,  219,  219,   87,  219,  219,
 /*  3360 */   193,   91,   92,   93,   94,  219,   96,  219,  219,   99,
 /*  3370 */   203,  204,  205,  206,  219,  219,  106,  219,  219,  219,
 /*  3380 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  3390 */     7,  219,    9,   10,  219,  193,  219,  219,  219,  197,
 /*  3400 */   219,  219,  193,  219,   21,  203,  204,  205,  206,  219,
 /*  3410 */   219,   28,  203,  204,  205,  206,   33,   34,   35,   36,
 /*  3420 */    37,   38,   39,  193,   41,  219,  219,   44,   45,  219,
 /*  3430 */   219,  219,  219,  203,  204,  205,  206,   54,   55,   56,
 /*  3440 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /*  3450 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  3460 */    77,   78,  219,  219,  219,   82,   83,   84,  219,  219,
 /*  3470 */    87,  219,  219,  193,   91,   92,   93,   94,  219,   96,
 /*  3480 */   219,  219,   99,  203,  204,  205,  206,  219,  219,  106,
 /*  3490 */   219,  219,  219,  110,  111,  112,  113,  114,  115,  116,
 /*  3500 */   117,  118,  119,    7,  219,    9,   10,  219,  193,  219,
 /*  3510 */   219,  219,  197,  219,  219,  193,  219,   21,  203,  204,
 /*  3520 */   205,  206,  219,  219,   28,  203,  204,  205,  206,   33,
 /*  3530 */    34,   35,   36,   37,   38,   39,  193,   41,  219,  219,
 /*  3540 */    44,   45,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  3550 */    54,   55,   56,  219,  219,   59,   60,   61,  219,  219,
 /*  3560 */   219,   65,  219,  219,   68,   69,   70,   71,   72,   73,
 /*  3570 */    74,   75,   76,   77,   78,  219,  219,  219,   82,   83,
 /*  3580 */    84,  219,  219,   87,  219,  219,  193,   91,   92,   93,
 /*  3590 */    94,  219,   96,  219,  219,   99,  203,  204,  205,  206,
 /*  3600 */   219,  219,  106,  219,  219,  219,  110,  111,  112,  113,
 /*  3610 */   114,  115,  116,  117,  118,  119,    7,  219,    9,   10,
 /*  3620 */   219,  193,  219,  219,  219,  197,  219,  219,  193,  219,
 /*  3630 */    21,  203,  204,  205,  206,  219,  219,   28,  203,  204,
 /*  3640 */   205,  206,   33,   34,   35,   36,   37,   38,   39,  193,
 /*  3650 */    41,  219,  219,   44,   45,  219,  219,  219,  219,  203,
 /*  3660 */   204,  205,  206,   54,   55,   56,  219,  219,   59,   60,
 /*  3670 */    61,  219,  219,  219,   65,  219,  219,   68,   69,   70,
 /*  3680 */    71,   72,   73,   74,   75,   76,   77,   78,  219,  219,
 /*  3690 */   219,   82,   83,   84,  219,  219,   87,  219,  219,  193,
 /*  3700 */    91,   92,   93,   94,  219,   96,  219,  219,   99,  203,
 /*  3710 */   204,  205,  206,  219,  219,  106,  219,  219,  219,  110,
 /*  3720 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  3730 */   219,    9,   10,  219,  193,  219,  219,  219,  197,  219,
 /*  3740 */   219,  193,  219,   21,  203,  204,  205,  206,  219,  219,
 /*  3750 */    28,  203,  204,  205,  206,   33,   34,   35,   36,   37,
 /*  3760 */    38,   39,  193,   41,  219,  219,   44,   45,  219,  219,
 /*  3770 */   219,  219,  203,  204,  205,  206,   54,   55,   56,  219,
 /*  3780 */   219,   59,   60,   61,  219,  219,  219,   65,  219,  219,
 /*  3790 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  3800 */    78,  219,  219,  219,   82,   83,   84,  219,  219,   87,
 /*  3810 */   219,  219,  193,   91,   92,   93,   94,  219,   96,  219,
 /*  3820 */   219,   99,  203,  204,  205,  206,  219,  219,  106,  219,
 /*  3830 */   219,  219,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  3840 */   118,  119,    7,  219,    9,   10,  219,  193,  219,  219,
 /*  3850 */   219,  197,  219,  219,  193,  219,   21,  203,  204,  205,
 /*  3860 */   206,  219,  219,   28,  203,  204,  205,  206,   33,   34,
 /*  3870 */    35,   36,   37,   38,   39,  193,   41,  219,  219,   44,
 /*  3880 */    45,  219,  219,  219,  219,  203,  204,  205,  206,   54,
 /*  3890 */    55,   56,  219,  219,   59,   60,   61,  219,  219,  219,
 /*  3900 */    65,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  3910 */    75,   76,   77,   78,  219,  219,  219,   82,   83,   84,
 /*  3920 */   219,  219,   87,  219,  219,  193,   91,   92,   93,   94,
 /*  3930 */   219,   96,  219,  219,   99,  203,  204,  205,  206,  219,
 /*  3940 */   219,  106,  219,  219,  219,  110,  111,  112,  113,  114,
 /*  3950 */   115,  116,  117,  118,  119,    7,  219,    9,   10,  219,
 /*  3960 */   193,  219,  219,  219,  219,  219,  219,  193,  219,   21,
 /*  3970 */   203,  204,  205,  206,  219,  219,   28,  203,  204,  205,
 /*  3980 */   206,   33,   34,   35,   36,   37,   38,   39,  193,   41,
 /*  3990 */   219,  219,   44,   45,  219,  219,  219,  219,  203,  204,
 /*  4000 */   205,  206,   54,   55,   56,  219,  219,   59,   60,   61,
 /*  4010 */   219,  219,  219,   65,  219,  219,   68,   69,   70,   71,
 /*  4020 */    72,   73,   74,   75,   76,   77,   78,  219,  219,  219,
 /*  4030 */    82,   83,   84,  219,  219,   87,  219,  219,  193,   91,
 /*  4040 */    92,   93,   94,  219,   96,  219,  219,   99,  203,  204,
 /*  4050 */   205,  206,  219,  219,  106,  219,  219,  219,  110,  111,
 /*  4060 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  219,
 /*  4070 */     9,   10,  219,  193,  219,  219,  219,  219,  219,  219,
 /*  4080 */   193,  219,   21,  203,  204,  205,  206,  219,  219,   28,
 /*  4090 */   203,  204,  205,  206,   33,   34,   35,   36,   37,   38,
 /*  4100 */    39,  193,   41,  219,  219,   44,   45,  219,  219,  219,
 /*  4110 */   219,  203,  204,  205,  206,   54,   55,   56,  219,  219,
 /*  4120 */    59,   60,   61,  219,  219,  219,   65,  219,  219,   68,
 /*  4130 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  4140 */   219,  219,  219,   82,   83,   84,  219,  219,   87,  219,
 /*  4150 */   219,  193,   91,   92,   93,   94,  219,   96,  219,  219,
 /*  4160 */    99,  203,  204,  205,  206,  219,  219,  106,  219,  219,
 /*  4170 */   219,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  4180 */   119,    7,  219,    9,   10,  219,  193,  219,  219,  219,
 /*  4190 */   219,  219,  219,  193,  219,   21,  203,  204,  205,  206,
 /*  4200 */   219,  219,   28,  203,  204,  205,  206,   33,   34,   35,
 /*  4210 */    36,   37,   38,   39,  193,   41,  219,  219,   44,   45,
 /*  4220 */   219,  219,  219,  219,  203,  204,  205,  206,   54,   55,
 /*  4230 */    56,  219,  219,   59,   60,   61,  219,  219,  219,   65,
 /*  4240 */   219,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  4250 */    76,   77,   78,  219,  219,  219,   82,   83,   84,  219,
 /*  4260 */   219,   87,  219,  219,  193,   91,   92,   93,   94,  219,
 /*  4270 */    96,  219,  219,   99,  203,  204,  205,  206,  219,  219,
 /*  4280 */   106,  219,  219,  219,  110,  111,  112,  113,  114,  115,
 /*  4290 */   116,  117,  118,  119,    7,  219,    9,   10,  219,  193,
 /*  4300 */   219,  219,  219,  219,  219,  219,  193,  219,   21,  203,
 /*  4310 */   204,  205,  206,  219,  219,   28,  203,  204,  205,  206,
 /*  4320 */    33,   34,   35,   36,   37,   38,   39,  193,   41,  219,
 /*  4330 */   219,   44,   45,  219,  219,  219,  219,  203,  204,  205,
 /*  4340 */   206,   54,   55,   56,  219,  219,   59,   60,   61,  219,
 /*  4350 */   219,  219,   65,  219,  219,   68,   69,   70,   71,   72,
 /*  4360 */    73,   74,   75,   76,   77,   78,  219,  219,  219,   82,
 /*  4370 */    83,   84,  219,  219,   87,  219,  219,  193,   91,   92,
 /*  4380 */    93,   94,  219,   96,  219,  219,   99,  203,  204,  205,
 /*  4390 */   206,  219,  219,  106,  219,  219,  219,  110,  111,  112,
 /*  4400 */   113,  114,  115,  116,  117,  118,  119,    7,  219,    9,
 /*  4410 */    10,  219,  193,  219,  219,  219,  219,  219,  219,  193,
 /*  4420 */   219,   21,  203,  204,  205,  206,  219,  219,   28,  203,
 /*  4430 */   204,  205,  206,   33,   34,   35,   36,   37,   38,   39,
 /*  4440 */   193,   41,  219,  219,   44,   45,  219,  219,  219,  219,
 /*  4450 */   203,  204,  205,  206,   54,   55,   56,  219,  219,   59,
 /*  4460 */    60,   61,  219,  219,  219,   65,  219,  219,   68,   69,
 /*  4470 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  219,
 /*  4480 */   219,  219,   82,   83,   84,  219,  219,   87,  161,  219,
 /*  4490 */   163,   91,   92,   93,   94,  168,   96,  170,  219,   99,
 /*  4500 */   219,  174,  219,  219,  219,  219,  106,  219,  219,  219,
 /*  4510 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  4520 */     7,  219,    9,   10,  219,  219,  219,  219,  219,  219,
 /*  4530 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /*  4540 */   219,   28,  219,  219,  219,  219,   33,   34,   35,   36,
 /*  4550 */    37,   38,   39,  219,   41,  219,  219,   44,   45,  219,
 /*  4560 */   219,  219,  219,  219,  219,  219,  219,   54,   55,   56,
 /*  4570 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /*  4580 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  4590 */    77,   78,  219,  219,  219,   82,   83,   84,  219,  219,
 /*  4600 */    87,  219,  219,  219,   91,   92,   93,   94,  219,   96,
 /*  4610 */   219,  219,   99,  219,  219,  219,  219,  219,  219,  106,
 /*  4620 */   219,  219,  219,  110,  111,  112,  113,  114,  115,  116,
 /*  4630 */   117,  118,  119,    7,  219,    9,   10,  219,  219,  219,
 /*  4640 */   219,  219,  219,  219,  219,  219,  219,   21,  219,  219,
 /*  4650 */   219,  219,  219,  219,   28,  219,  219,  219,  219,   33,
 /*  4660 */    34,   35,   36,   37,   38,   39,  219,   41,  219,  219,
 /*  4670 */    44,   45,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  4680 */    54,   55,   56,  219,  219,   59,   60,   61,  219,  219,
 /*  4690 */   219,   65,  219,  219,   68,   69,   70,   71,   72,   73,
 /*  4700 */    74,   75,   76,   77,   78,  219,  219,  219,   82,   83,
 /*  4710 */    84,  219,  219,   87,  219,  219,  219,   91,   92,   93,
 /*  4720 */    94,  219,   96,  219,  219,   99,  219,  219,  219,  219,
 /*  4730 */   219,  219,  106,  219,  219,  219,  110,  111,  112,  113,
 /*  4740 */   114,  115,  116,  117,  118,  119,    7,  219,    9,   10,
 /*  4750 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  4760 */    21,  219,  219,  219,  219,  219,  219,   28,  219,  219,
 /*  4770 */   219,  219,   33,   34,   35,   36,   37,   38,   39,  219,
 /*  4780 */    41,  219,  219,   44,   45,  219,  219,  219,  219,  219,
 /*  4790 */   219,  219,  219,   54,   55,   56,  219,  219,   59,   60,
 /*  4800 */    61,  219,  219,  219,   65,  219,  219,   68,   69,   70,
 /*  4810 */    71,   72,   73,   74,   75,   76,   77,   78,  219,  219,
 /*  4820 */   219,   82,   83,   84,  219,  219,   87,  219,  219,  219,
 /*  4830 */    91,   92,   93,   94,  219,   96,  219,  219,   99,  219,
 /*  4840 */   219,  219,  219,  219,  219,  106,  219,  219,  219,  110,
 /*  4850 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  4860 */   219,    9,   10,  219,  219,  219,  219,  219,  219,  219,
 /*  4870 */   219,  219,  219,   21,  219,  219,  219,  219,  219,  219,
 /*  4880 */    28,  219,  219,  219,  219,   33,   34,   35,   36,   37,
 /*  4890 */    38,   39,  219,   41,  219,  219,   44,   45,  219,  219,
 /*  4900 */   219,  219,  219,  219,  219,  219,   54,   55,   56,  219,
 /*  4910 */   219,   59,   60,   61,  219,  219,  219,   65,  219,  219,
 /*  4920 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  4930 */    78,  219,  219,  219,   82,   83,   84,  219,  219,   87,
 /*  4940 */   219,  219,  219,   91,   92,   93,   94,  219,   96,  219,
 /*  4950 */   219,   99,  219,  219,  219,  219,  219,  219,  106,  219,
 /*  4960 */   219,  219,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  4970 */   118,  119,    7,  219,    9,   10,  219,  219,  219,  219,
 /*  4980 */   219,  219,  219,  219,  219,  219,   21,  219,  219,  219,
 /*  4990 */   219,  219,  219,   28,  219,  219,  219,  219,   33,   34,
 /*  5000 */    35,   36,   37,   38,   39,  219,   41,  219,  219,   44,
 /*  5010 */    45,  219,  219,  219,  219,  219,  219,  219,  219,   54,
 /*  5020 */    55,   56,  219,  219,   59,   60,   61,  219,  219,  219,
 /*  5030 */    65,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  5040 */    75,   76,   77,   78,  219,  219,  219,   82,   83,   84,
 /*  5050 */   219,  219,   87,  219,  219,  219,   91,   92,   93,   94,
 /*  5060 */   219,   96,  219,  219,   99,  219,  219,  219,  219,  219,
 /*  5070 */   219,  106,  219,  219,  219,  110,  111,  112,  113,  114,
 /*  5080 */   115,  116,  117,  118,  119,    7,  219,    9,   10,  219,
 /*  5090 */   219,  219,  219,  219,  219,  219,  219,  219,  219,   21,
 /*  5100 */   219,  219,  219,  219,  219,  219,   28,  219,  219,  219,
 /*  5110 */   219,   33,   34,   35,   36,   37,   38,   39,  219,   41,
 /*  5120 */   219,  219,   44,   45,  219,  219,  219,  219,  219,  219,
 /*  5130 */   219,  219,   54,   55,   56,  219,  219,   59,   60,   61,
 /*  5140 */   219,  219,  219,   65,  219,  219,   68,   69,   70,   71,
 /*  5150 */    72,   73,   74,   75,   76,   77,   78,  219,  219,  219,
 /*  5160 */    82,   83,   84,  219,  219,   87,  219,  219,  219,   91,
 /*  5170 */    92,   93,   94,  219,   96,  219,  219,   99,  219,  219,
 /*  5180 */   219,  219,  219,  219,  106,  219,  219,  219,  110,  111,
 /*  5190 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  219,
 /*  5200 */     9,   10,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  5210 */   219,  219,   21,  219,  219,  219,  219,  219,  219,   28,
 /*  5220 */   219,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /*  5230 */    39,  219,   41,  219,  219,   44,   45,  219,  219,  219,
 /*  5240 */   219,  219,  219,  219,  219,   54,   55,   56,  219,  219,
 /*  5250 */    59,   60,   61,  219,  219,  219,   65,  219,  219,   68,
 /*  5260 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  5270 */   219,  219,  219,   82,   83,   84,  219,  219,   87,  219,
 /*  5280 */   219,  219,   91,   92,   93,   94,  219,   96,  219,  219,
 /*  5290 */    99,  219,  219,  219,  219,  219,  219,  106,  219,  219,
 /*  5300 */   219,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  5310 */   119,    7,  219,    9,   10,  219,  219,  219,  219,  219,
 /*  5320 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /*  5330 */   219,  219,   28,  219,  219,  219,  219,   33,   34,   35,
 /*  5340 */    36,   37,   38,   39,  219,   41,  219,  219,   44,   45,
 /*  5350 */   219,  219,  219,  219,  219,  219,  219,  219,   54,   55,
 /*  5360 */    56,  219,  219,   59,   60,   61,  219,  219,  219,   65,
 /*  5370 */   219,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  5380 */    76,   77,   78,  219,  219,  219,   82,   83,   84,  219,
 /*  5390 */   219,   87,  219,  219,  219,   91,   92,   93,   94,  219,
 /*  5400 */    96,  219,  219,   99,  219,  219,  219,  219,  219,  219,
 /*  5410 */   106,  219,  219,  219,  110,  111,  112,  113,  114,  115,
 /*  5420 */   116,  117,  118,  119,    7,  219,    9,   10,  219,  219,
 /*  5430 */   219,  219,  219,  219,  219,  219,  219,  219,   21,  219,
 /*  5440 */   219,  219,  219,  219,  219,   28,  219,  219,  219,  219,
 /*  5450 */    33,   34,   35,   36,   37,   38,   39,  219,   41,  219,
 /*  5460 */   219,   44,   45,  219,  219,  219,  219,  219,  219,  219,
 /*  5470 */   219,   54,   55,   56,  219,  219,   59,   60,   61,  219,
 /*  5480 */   219,  219,   65,  219,  219,   68,   69,   70,   71,   72,
 /*  5490 */    73,   74,   75,   76,   77,   78,  219,  219,  219,   82,
 /*  5500 */    83,   84,  219,  219,   87,  219,  219,  219,   91,   92,
 /*  5510 */    93,   94,  219,   96,  219,  219,   99,  219,  219,  219,
 /*  5520 */   219,  219,  219,  106,  219,  219,  219,  110,  111,  112,
 /*  5530 */   113,  114,  115,  116,  117,  118,  119,    7,  219,    9,
 /*  5540 */    10,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  5550 */   219,   21,  219,  219,  219,  219,  219,  219,   28,  219,
 /*  5560 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /*  5570 */   219,   41,  219,  219,   44,   45,  219,  219,  219,  219,
 /*  5580 */   219,  219,  219,  219,   54,   55,   56,  219,  219,   59,
 /*  5590 */    60,   61,  219,  219,  219,   65,  219,  219,   68,   69,
 /*  5600 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  219,
 /*  5610 */   219,  219,   82,   83,   84,  219,  219,   87,  219,  219,
 /*  5620 */   219,   91,   92,   93,   94,  219,   96,  219,  219,   99,
 /*  5630 */   219,  219,  219,  219,  219,  219,  106,  219,  219,  219,
 /*  5640 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  5650 */     7,  219,    9,   10,  219,  219,  219,  219,  219,  219,
 /*  5660 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /*  5670 */   219,   28,  219,  219,  219,  219,   33,   34,   35,   36,
 /*  5680 */    37,   38,   39,  219,   41,  219,  219,   44,   45,  219,
 /*  5690 */   219,  219,  219,  219,  219,  219,  219,   54,   55,   56,
 /*  5700 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /*  5710 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  5720 */    77,   78,  219,  219,  219,   82,   83,   84,  219,  219,
 /*  5730 */    87,  219,  219,  219,   91,   92,   93,   94,  219,   96,
 /*  5740 */   219,  219,   99,  219,  219,  219,  219,  219,  219,  106,
 /*  5750 */   219,  219,  219,  110,  111,  112,  113,  114,  115,  116,
 /*  5760 */   117,  118,  119,    7,  219,    9,   10,  219,  219,  219,
 /*  5770 */   219,  219,  219,  219,  219,  219,  219,   21,  219,  219,
 /*  5780 */   219,  219,  219,  219,   28,  219,  219,  219,  219,   33,
 /*  5790 */    34,   35,   36,   37,   38,   39,  219,   41,  219,  219,
 /*  5800 */    44,   45,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  5810 */    54,   55,   56,  219,  219,   59,   60,   61,  219,  219,
 /*  5820 */   219,   65,  219,  219,   68,   69,   70,   71,   72,   73,
 /*  5830 */    74,   75,   76,   77,   78,  219,  219,  219,   82,   83,
 /*  5840 */    84,  219,  219,   87,  219,  219,  219,   91,   92,   93,
 /*  5850 */    94,  219,   96,  219,  219,   99,  219,  219,  219,  219,
 /*  5860 */   219,  219,  106,  219,  219,  219,  110,  111,  112,  113,
 /*  5870 */   114,  115,  116,  117,  118,  119,    7,  219,    9,   10,
 /*  5880 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  5890 */    21,  219,  219,  219,  219,  219,  219,   28,  219,  219,
 /*  5900 */   219,  219,   33,   34,   35,   36,   37,   38,   39,  219,
 /*  5910 */    41,  219,  219,   44,   45,  219,  219,  219,  219,  219,
 /*  5920 */   219,  219,  219,   54,   55,   56,  219,  219,   59,   60,
 /*  5930 */    61,  219,  219,  219,   65,  219,  219,   68,   69,   70,
 /*  5940 */    71,   72,   73,   74,   75,   76,   77,   78,  219,  219,
 /*  5950 */   219,   82,   83,   84,  219,  219,   87,  219,  219,  219,
 /*  5960 */    91,   92,   93,   94,  219,   96,  219,  219,   99,  219,
 /*  5970 */   219,  219,  219,  219,  219,  106,  219,  219,  219,  110,
 /*  5980 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  5990 */   219,    9,   10,  219,  219,  219,  219,  219,  219,  219,
 /*  6000 */   219,  219,  219,   21,  219,  219,  219,  219,  219,  219,
 /*  6010 */    28,  219,  219,  219,  219,   33,   34,   35,   36,   37,
 /*  6020 */    38,   39,  219,   41,  219,  219,   44,   45,  219,  219,
 /*  6030 */   219,  219,  219,  219,  219,  219,   54,   55,   56,  219,
 /*  6040 */   219,   59,   60,   61,  219,  219,  219,   65,  219,  219,
 /*  6050 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  6060 */    78,  219,  219,  219,   82,   83,   84,  219,  219,   87,
 /*  6070 */   219,  219,  219,   91,   92,   93,   94,  219,   96,  219,
 /*  6080 */   219,   99,  219,  219,  219,  219,  219,  219,  106,  219,
 /*  6090 */   219,  219,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  6100 */   118,  119,    7,  219,    9,   10,  219,  219,  219,  219,
 /*  6110 */   219,  219,  219,  219,  219,  219,   21,  219,  219,  219,
 /*  6120 */   219,  219,  219,   28,  219,  219,  219,  219,   33,   34,
 /*  6130 */    35,   36,   37,   38,   39,  219,   41,  219,  219,   44,
 /*  6140 */    45,  219,  219,  219,  219,  219,  219,  219,  219,   54,
 /*  6150 */   219,   56,  219,  219,   59,   60,   61,  219,  219,  219,
 /*  6160 */    65,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  6170 */    75,   76,   77,   78,  219,  219,  219,   82,   83,   84,
 /*  6180 */   219,  219,   87,    0,  219,  219,   91,   92,   93,   94,
 /*  6190 */     7,   96,  219,  219,   99,  219,  219,  219,  219,  219,
 /*  6200 */   219,  106,  219,  219,  219,  110,  111,  112,  113,  114,
 /*  6210 */   115,  116,  117,  118,  119,  219,  219,   34,  219,  219,
 /*  6220 */   219,  219,  219,  219,  219,   41,   43,   44,  219,   46,
 /*  6230 */   219,   48,  219,   50,  219,   52,   53,   54,  219,   56,
 /*  6240 */   219,   57,  219,   60,  219,  219,  219,  219,  219,  219,
 /*  6250 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  6260 */    77,   78,   79,   80,   81,   82,   83,   84,  219,  219,
 /*  6270 */    87,    7,  219,  219,   91,   92,   93,   94,  219,   96,
 /*  6280 */   219,  219,   99,  219,  100,  101,  102,  103,  104,  105,
 /*  6290 */   219,  219,  219,  110,  111,  112,  113,  219,   34,  219,
 /*  6300 */   219,  219,  119,  219,  219,  219,   41,   43,   44,  219,
 /*  6310 */    46,  219,   48,  219,   50,  219,   52,   53,   54,  219,
 /*  6320 */    56,  219,   57,  219,   60,  219,  219,  219,  219,  219,
 /*  6330 */   219,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  6340 */    76,   77,   78,   79,   80,   81,   82,   83,   84,  219,
 /*  6350 */   219,   87,  219,  219,  219,   91,   92,   93,   94,    7,
 /*  6360 */    96,    9,   10,   99,  219,  100,  101,  102,  103,  104,
 /*  6370 */   105,  219,  219,   21,  110,  111,  112,  113,  219,  219,
 /*  6380 */    28,  219,  219,  119,  219,   33,   34,   35,   36,   37,
 /*  6390 */    38,   39,  219,   41,  219,  219,   44,  219,  219,  219,
 /*  6400 */   219,  219,  219,  219,  219,  219,   54,  219,  219,  219,
 /*  6410 */   219,   59,   60,   61,  219,  219,  219,   65,  219,  219,
 /*  6420 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  6430 */    78,   79,   80,   81,   11,   12,   13,   14,   15,   16,
 /*  6440 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /*  6450 */    27,   28,   29,   30,   31,   32,  219,  219,  106,  219,
 /*  6460 */   219,  219,  219,  219,   41,   42,  114,  115,  116,  117,
 /*  6470 */   118,  219,  219,  219,  219,  219,  219,  121,  122,  123,
 /*  6480 */   124,  125,  126,  127,  128,  129,  130,  131,  132,  133,
 /*  6490 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  6500 */   144,  145,  146,  147,  148,  149,  150,    8,  219,  219,
 /*  6510 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*  6520 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /*  6530 */    31,   32,  219,  177,  219,  219,  219,  219,  219,  219,
 /*  6540 */    41,   42,   19,   20,   21,   22,   23,   24,   25,   26,
 /*  6550 */    27,   28,   29,   30,   31,   32,  219,  193,  219,  219,
 /*  6560 */   204,  205,  206,  219,   41,   42,   67,  203,  204,  205,
 /*  6570 */   206,  219,  219,  209,  219,  219,  212,  213,  214,  219,
 /*  6580 */   219,  219,  219,  124,  125,  126,  127,  128,  129,  130,
 /*  6590 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  6600 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  6610 */   219,  219,  219,  219,  219,  130,  131,  132,  133,  134,
 /*  6620 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  6630 */   145,  146,  147,  148,  149,  150,  177,  219,  219,  193,
 /*  6640 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  203,
 /*  6650 */   204,  205,  206,  219,  219,  219,  219,  172,  212,  213,
 /*  6660 */   214,  219,  219,  204,  205,  206,  181,  182,  183,  219,
 /*  6670 */   219,  219,  219,  219,  219,  219,  219,  219,  193,  219,
 /*  6680 */   219,  219,  219,  219,  219,  219,  219,  219,  203,  204,
 /*  6690 */   205,  206,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  6700 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  6710 */   148,  149,  150,   13,   14,   15,   16,   17,   18,   19,
 /*  6720 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*  6730 */    30,   31,   32,  219,  172,  219,  219,  219,  219,  219,
 /*  6740 */   219,   41,   42,  181,  182,  183,  219,  219,  219,  219,
 /*  6750 */   219,  219,  219,  219,  219,  193,  219,  219,  219,  219,
 /*  6760 */   219,  219,  219,  219,  219,  203,  204,  205,  206,  219,
 /*  6770 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  6780 */   219,  219,  219,  219,  219,  219,  219,  219,  130,  131,
 /*  6790 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  6800 */   142,  143,  144,  145,  146,  147,  148,  149,  150,   14,
 /*  6810 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*  6820 */    25,   26,   27,   28,   29,   30,   31,   32,  219,  219,
 /*  6830 */   172,   41,   42,  219,  219,   41,   41,   42,   44,  181,
 /*  6840 */   182,  183,   41,  219,  219,  219,  219,   57,  219,  219,
 /*  6850 */   219,  193,  219,   59,  219,  219,  219,  219,   57,   65,
 /*  6860 */   219,  203,  204,  205,  206,  130,  131,  132,  133,  134,
 /*  6870 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  6880 */   145,  146,  147,  148,  149,  150,  219,  219,  219,  219,
 /*  6890 */   100,  101,  102,  103,  104,  105,   21,  107,  108,  109,
 /*  6900 */   106,  100,  101,  102,  103,  104,  105,  172,  114,  115,
 /*  6910 */   116,  117,  118,  219,  219,   40,  181,  182,  183,   44,
 /*  6920 */   219,  219,  219,  219,  219,  219,  219,  219,  193,  219,
 /*  6930 */   219,  219,  219,   58,  219,  219,  219,   21,  203,  204,
 /*  6940 */   205,  206,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  6950 */    75,   76,   77,   78,   79,   80,   81,  219,  219,  219,
 /*  6960 */    44,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  6970 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  6980 */   149,  150,  219,  219,   68,   69,   70,   71,   72,   73,
 /*  6990 */    74,   75,   76,   77,   78,   79,   80,   81,  219,  219,
 /*  7000 */   219,  219,   41,  172,  219,   44,  219,  219,  219,  219,
 /*  7010 */   219,  219,  181,  182,  183,  219,  219,  219,  219,  219,
 /*  7020 */    59,  219,  219,  219,  193,  219,   65,  219,   67,  219,
 /*  7030 */   219,  219,  219,  219,  203,  204,  205,  206,  130,  131,
 /*  7040 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  7050 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  7060 */   219,  219,  219,  219,  219,  219,  219,  106,  219,   21,
 /*  7070 */   219,  219,  219,  219,  219,  114,  115,  116,  117,  118,
 /*  7080 */   172,  219,  219,  219,  219,  219,  219,  219,   40,  181,
 /*  7090 */   182,  183,   44,  219,  219,  219,  219,  219,  219,   41,
 /*  7100 */   219,  193,  219,  219,  219,  219,   58,  219,  219,  219,
 /*  7110 */   219,  203,  204,  205,  206,   57,   68,   69,   70,   71,
 /*  7120 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*  7130 */   219,  219,  219,  219,  130,  131,  132,  133,  134,  135,
 /*  7140 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  7150 */   146,  147,  148,  149,  150,  219,  219,  219,  100,  101,
 /*  7160 */   102,  103,  104,  105,  219,  219,  108,  109,  219,  219,
 /*  7170 */   219,  219,  219,  219,  219,  219,  172,   41,  219,  219,
 /*  7180 */   219,   41,  219,  219,   44,  181,  182,  183,  219,  219,
 /*  7190 */   219,  219,  219,   57,  219,  219,  219,  193,  219,   59,
 /*  7200 */   219,  219,  219,  219,  219,   65,  219,  203,  204,  205,
 /*  7210 */   206,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  7220 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  7230 */   149,  150,  219,  219,  219,  219,  100,  101,  102,  103,
 /*  7240 */   104,  105,  219,  219,  219,   21,  106,  219,  219,  219,
 /*  7250 */   219,  219,  219,  172,  114,  115,  116,  117,  118,  219,
 /*  7260 */   219,  219,  181,  182,  183,  219,  219,  219,  219,  219,
 /*  7270 */   219,  219,  219,  219,  193,  219,  219,  219,  219,  219,
 /*  7280 */   219,  219,  219,  219,  203,  204,  205,  206,   64,   65,
 /*  7290 */    66,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  7300 */    76,   77,   78,   79,   80,   81,  219,  130,  131,  132,
 /*  7310 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  7320 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /*  7330 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7340 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  172,
 /*  7350 */   219,  219,  219,  219,  219,  219,  219,  219,  181,  182,
 /*  7360 */   183,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7370 */   193,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7380 */   203,  204,  205,  206,  130,  131,  132,  133,  134,  135,
 /*  7390 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  7400 */   146,  147,  148,  149,  150,  219,  219,  219,  219,  219,
 /*  7410 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /*  7420 */   219,  219,  219,  219,  219,  219,  172,  219,  219,  219,
 /*  7430 */   219,  219,  219,  219,   40,  181,  182,  183,   44,  219,
 /*  7440 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  7450 */   219,  219,   58,  219,  219,  219,  219,  203,  204,  205,
 /*  7460 */   206,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  7470 */    76,   77,   78,   79,   80,   81,  219,  219,  219,  219,
 /*  7480 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  7490 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  7500 */   150,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7510 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7520 */   219,  219,  172,  219,  219,  219,  219,  219,  219,  219,
 /*  7530 */   219,  181,  182,  183,  219,  219,  219,  219,  219,  219,
 /*  7540 */   219,  219,  219,  193,  219,  219,  219,  219,  219,  219,
 /*  7550 */   219,  219,  219,  203,  204,  205,  206,  130,  131,  132,
 /*  7560 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  7570 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /*  7580 */   219,  219,  219,  219,  219,  219,  219,  219,   21,  219,
 /*  7590 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  172,
 /*  7600 */   219,  219,  219,  219,  219,  219,  219,   40,  181,  182,
 /*  7610 */   183,   44,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7620 */   193,  219,  219,  219,  219,   58,  219,  219,  219,  219,
 /*  7630 */   203,  204,  205,  206,  219,   68,   69,   70,   71,   72,
 /*  7640 */    73,   74,   75,   76,   77,   78,   79,   80,   81,  219,
 /*  7650 */   219,  219,  219,  130,  131,  132,  133,  134,  135,  136,
 /*  7660 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  7670 */   147,  148,  149,  150,  219,  219,  219,  219,  219,  219,
 /*  7680 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7690 */   219,  219,  219,  219,  219,  172,  219,  219,  219,  219,
 /*  7700 */   219,  219,  219,  219,  181,  182,  183,  219,  219,  219,
 /*  7710 */   219,  219,  219,  219,  219,  219,  193,  219,  219,  219,
 /*  7720 */   219,  219,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  7730 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  7740 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  7750 */   150,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7760 */   219,   21,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7770 */   219,  219,  172,  219,  219,  219,  219,  219,  219,  219,
 /*  7780 */    40,  181,  182,  183,   44,  219,  219,  219,  219,  219,
 /*  7790 */   219,  219,  219,  193,  219,  219,  219,  219,   58,  219,
 /*  7800 */   219,  219,  219,  203,  204,  205,  206,  219,   68,   69,
 /*  7810 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*  7820 */    80,   81,  219,  219,  219,  219,  130,  131,  132,  133,
 /*  7830 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  7840 */   144,  145,  146,  147,  148,  149,  150,  219,  219,  219,
 /*  7850 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7860 */   219,  219,  219,  219,  219,  219,  219,  219,  172,  219,
 /*  7870 */   219,  219,  219,  219,  219,  219,  219,  181,  182,  183,
 /*  7880 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  193,
 /*  7890 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  203,
 /*  7900 */   204,  205,  206,  130,  131,  132,  133,  134,  135,  136,
 /*  7910 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  7920 */   147,  148,  149,  150,  219,  219,  219,  219,  219,  219,
 /*  7930 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /*  7940 */   219,  219,  219,  219,  219,  172,  219,  219,  219,  219,
 /*  7950 */   219,  219,  219,   40,  181,  182,  183,   44,  219,  219,
 /*  7960 */   219,  219,  219,  219,  219,  219,  193,  219,  219,  219,
 /*  7970 */   219,   58,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  7980 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  7990 */    77,   78,   79,   80,   81,  219,  219,  219,  219,  130,
 /*  8000 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  8010 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  8020 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8030 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8040 */   219,  172,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8050 */   181,  182,  183,  219,  219,  219,  219,  219,  219,  219,
 /*  8060 */   219,  219,  193,  219,  219,  219,  219,  219,  219,  219,
 /*  8070 */   219,  219,  203,  204,  205,  206,  130,  131,  132,  133,
 /*  8080 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  8090 */   144,  145,  146,  147,  148,  149,  150,  219,  219,  219,
 /*  8100 */   219,  219,  219,  219,  219,  219,  219,   21,  219,  219,
 /*  8110 */   219,  219,  219,  219,  219,  219,  219,  219,  172,  219,
 /*  8120 */   219,  219,  219,  219,  219,  219,   40,  181,  182,  183,
 /*  8130 */    44,  219,  219,  219,  219,  219,  219,  219,  219,  193,
 /*  8140 */   219,  219,  219,  219,   58,  219,  219,  219,  219,  203,
 /*  8150 */   204,  205,  206,  219,   68,   69,   70,   71,   72,   73,
 /*  8160 */    74,   75,   76,   77,   78,   79,   80,   81,  219,  219,
 /*  8170 */   219,  219,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  8180 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  8190 */   148,  149,  150,  219,  219,  219,  219,  219,  219,  219,
 /*  8200 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8210 */   219,  219,  219,  219,  172,  219,  219,  219,  219,  219,
 /*  8220 */   219,  219,  219,  181,  182,  183,  219,  219,  219,  219,
 /*  8230 */   219,  219,  219,  219,  219,  193,  219,  219,  219,  219,
 /*  8240 */   219,  219,  219,  219,  219,  203,  204,  205,  206,  130,
 /*  8250 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  8260 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  8270 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8280 */    21,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8290 */   219,  172,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8300 */   181,  182,  183,   44,  219,  219,  219,  219,  219,  219,
 /*  8310 */   219,  219,  193,  219,  219,  219,  219,   58,  219,  219,
 /*  8320 */   219,  219,  203,  204,  205,  206,  219,   68,   69,   70,
 /*  8330 */    71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*  8340 */    81,  219,  219,  219,  219,  130,  131,  132,  133,  134,
 /*  8350 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  8360 */   145,  146,  147,  148,  149,  150,  219,  219,  219,  219,
 /*  8370 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8380 */   219,  219,  219,  219,  219,  219,  219,  172,  219,  219,
 /*  8390 */   219,  219,  219,  219,  219,  219,  181,  182,  183,  219,
 /*  8400 */   219,  219,  219,  219,  219,  219,  219,  219,  193,  219,
 /*  8410 */   219,  219,  219,  219,  219,  219,  219,  219,  203,  204,
 /*  8420 */   205,  206,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  8430 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  8440 */   148,  149,  150,  219,  219,  219,  219,  219,  219,  219,
 /*  8450 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /*  8460 */   219,  219,  219,  219,  172,  219,  219,  219,  219,  219,
 /*  8470 */   219,  219,  219,  181,  182,  183,  219,  219,  219,  219,
 /*  8480 */   219,  219,  219,  219,  219,  193,  219,  219,  219,  219,
 /*  8490 */   219,  219,  219,  219,  219,  203,  204,  205,  206,   65,
 /*  8500 */    66,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  8510 */    76,   77,   78,   79,   80,   81,  219,  219,  130,  131,
 /*  8520 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  8530 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  8540 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8550 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8560 */   172,  219,  219,  219,  219,  219,  219,  219,  219,  181,
 /*  8570 */   182,  183,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8580 */   219,  193,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8590 */   219,  203,  204,  205,  206,  130,  131,  132,  133,  134,
 /*  8600 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  8610 */   145,  146,  147,  148,  149,  150,  219,  219,  219,  219,
 /*  8620 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8630 */   219,  219,  219,  219,  219,  219,  219,  172,  219,  219,
 /*  8640 */   219,  219,  219,  219,  219,  219,  181,  182,  183,  219,
 /*  8650 */   219,  219,  219,  219,  219,  219,  219,  219,  193,  219,
 /*  8660 */   219,  219,  219,  219,  219,  219,  219,  219,  203,  204,
 /*  8670 */   205,  206,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8680 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8690 */   219,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  8700 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  8710 */   149,  150,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8720 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8730 */   219,  219,  219,  172,  219,  219,  219,  219,  219,  219,
 /*  8740 */   219,  219,  181,  182,  183,  219,  219,  219,  219,  219,
 /*  8750 */   219,  219,  219,  219,  193,  219,  219,  219,  219,  219,
 /*  8760 */   219,  219,  219,  219,  203,  204,  205,  206,  130,  131,
 /*  8770 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  8780 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  8790 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8800 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8810 */   172,  219,  219,  219,  219,  219,  219,  219,  219,  181,
 /*  8820 */   182,  183,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8830 */   219,  193,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8840 */   219,  203,  204,  205,  206,  219,  219,  219,  219,  219,
 /*  8850 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8860 */   219,  219,  219,  219,  130,  131,  132,  133,  134,  135,
 /*  8870 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  8880 */   146,  147,  148,  149,  150,  219,  219,  219,  219,  219,
 /*  8890 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8900 */   219,  219,  219,  219,  219,  219,  172,  219,  219,  219,
 /*  8910 */   219,  219,  219,  219,  219,  181,  182,  183,  219,  219,
 /*  8920 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  8930 */   219,  219,  219,  219,  219,  219,  219,  203,  204,  205,
 /*  8940 */   206,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  8950 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  8960 */   149,  150,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8970 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8980 */   219,  219,  219,  172,  219,  219,  219,  219,  219,  219,
 /*  8990 */   219,  219,  181,  182,  183,  219,  219,  219,  219,  219,
 /*  9000 */   219,  219,  219,  219,  193,  219,  219,  219,  219,  219,
 /*  9010 */   219,  219,  219,  219,  203,  204,  205,  206,  219,  219,
 /*  9020 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9030 */   219,  219,  219,  219,  219,  219,  219,  130,  131,  132,
 /*  9040 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  9050 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /*  9060 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9070 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  172,
 /*  9080 */   219,  219,  219,  219,  219,  219,  219,  219,  181,  182,
 /*  9090 */   183,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9100 */   193,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9110 */   203,  204,  205,  206,  130,  131,  132,  133,  134,  135,
 /*  9120 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  9130 */   146,  147,  148,  149,  150,  219,  219,  219,  219,  219,
 /*  9140 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9150 */   219,  219,  219,  219,  219,  219,  172,  219,  219,  219,
 /*  9160 */   219,  219,  219,  219,  219,  181,  182,  183,  219,  219,
 /*  9170 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  9180 */   219,  219,  219,  219,  219,  219,  219,  203,  204,  205,
 /*  9190 */   206,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9200 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9210 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  9220 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  9230 */   150,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9240 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9250 */   219,  219,  172,  219,  219,  219,  219,  219,  219,  219,
 /*  9260 */   219,  181,  182,  183,  219,  219,  219,  219,  219,  219,
 /*  9270 */   219,  219,  219,  193,  219,  219,  219,  219,  219,  219,
 /*  9280 */   219,  219,  219,  203,  204,  205,  206,  130,  131,  132,
 /*  9290 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  9300 */   143,  144,  145,  146,  147,  148,  149,  150,  219,    8,
 /*  9310 */   219,  219,   11,   12,   13,   14,   15,   16,   17,   18,
 /*  9320 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /*  9330 */    29,   30,   31,   32,  219,  219,  219,  219,  181,  182,
 /*  9340 */   183,  219,   41,   42,  219,  219,  219,  219,  219,  219,
 /*  9350 */   193,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9360 */   203,  204,  205,  206,  219,  219,  219,  219,    7,  219,
 /*  9370 */     9,   10,  219,  219,  219,  219,   15,  219,  219,  219,
 /*  9380 */   219,  219,   21,  219,  219,  219,  219,  219,  219,   28,
 /*  9390 */    89,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /*  9400 */    39,   40,   41,  219,    7,   44,    9,   10,  219,  219,
 /*  9410 */   219,  219,   15,  219,  219,   54,  219,  219,   21,  219,
 /*  9420 */    59,   60,   61,  219,  219,   28,   65,  219,  219,  219,
 /*  9430 */    33,   34,   35,   36,   37,   38,   39,   40,   41,  219,
 /*  9440 */   219,   44,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9450 */   219,   54,  219,  219,  219,  219,   59,   60,   61,  219,
 /*  9460 */   219,  219,   65,  219,  219,  219,  219,  106,  219,  219,
 /*  9470 */   219,  219,  219,  219,  219,  114,  115,  116,  117,  118,
 /*  9480 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /*  9490 */    22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
 /*  9500 */    32,  219,  219,  106,  219,  219,  219,  219,  219,   41,
 /*  9510 */    42,  114,  115,  116,  117,  118,  219,  219,  219,    7,
 /*  9520 */   219,    9,   10,  219,  219,  219,  219,   15,  219,  219,
 /*  9530 */   219,  219,  219,   21,  219,  219,  219,  219,  219,  219,
 /*  9540 */    28,  219,  219,  219,  219,   33,   34,   35,   36,   37,
 /*  9550 */    38,   39,   40,   41,  219,    7,   44,    9,   10,  219,
 /*  9560 */   219,  219,  219,   15,  219,  219,   54,  219,  219,   21,
 /*  9570 */   219,   59,   60,   61,  219,  219,   28,   65,  219,  219,
 /*  9580 */   219,   33,   34,   35,   36,   37,   38,   39,   40,   41,
 /*  9590 */   219,  219,   44,  219,  219,  219,  219,  219,  219,  219,
 /*  9600 */   219,  219,   54,  219,  219,  219,  219,   59,   60,   61,
 /*  9610 */   219,  219,  219,   65,  219,  219,  219,  219,  106,  219,
 /*  9620 */   219,  219,  219,  219,  219,  219,  114,  115,  116,  117,
 /*  9630 */   118,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9640 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9650 */   219,  219,  219,  219,  106,  219,  219,  219,  219,  219,
 /*  9660 */   219,  219,  114,  115,  116,  117,  118,  219,  219,  219,
 /*  9670 */     7,  219,    9,   10,  219,  219,  219,  219,   15,  219,
 /*  9680 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /*  9690 */   219,   28,  219,  219,  219,  219,   33,   34,   35,   36,
 /*  9700 */    37,   38,   39,   40,   41,  219,    7,   44,    9,   10,
 /*  9710 */   219,  219,  219,  219,   15,  219,  219,   54,  219,  219,
 /*  9720 */    21,  219,   59,   60,   61,  219,  219,   28,   65,  219,
 /*  9730 */   219,  219,   33,   34,   35,   36,   37,   38,   39,   40,
 /*  9740 */    41,  219,  219,   44,  219,  219,  219,  219,  219,  219,
 /*  9750 */   219,  219,  219,   54,  219,  219,  219,  219,   59,   60,
 /*  9760 */    61,  219,  219,  219,   65,  219,  219,  219,  219,  106,
 /*  9770 */   219,  219,  219,  219,  219,  219,  219,  114,  115,  116,
 /*  9780 */   117,  118,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9790 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9800 */   219,  219,  219,  219,  219,  106,  219,  219,  219,  219,
 /*  9810 */   219,  219,  219,  114,  115,  116,  117,  118,  219,  219,
 /*  9820 */   219,    7,  219,    9,   10,  219,  219,  219,  219,   15,
 /*  9830 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /*  9840 */   219,  219,   28,  219,  219,  219,  219,   33,   34,   35,
 /*  9850 */    36,   37,   38,   39,   40,   41,  219,    7,   44,    9,
 /*  9860 */    10,  219,  219,  219,  219,   15,  219,  219,   54,  219,
 /*  9870 */   219,   21,  219,   59,   60,   61,  219,  219,   28,   65,
 /*  9880 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /*  9890 */    40,   41,  219,  219,   44,  219,  219,  219,  219,  219,
 /*  9900 */   219,  219,  219,  219,   54,  219,  219,  219,  219,   59,
 /*  9910 */    60,   61,  219,  219,  219,   65,  219,  219,  219,  219,
 /*  9920 */   106,  219,  219,  219,  219,  219,  219,  219,  114,  115,
 /*  9930 */   116,  117,  118,  219,  219,  219,  219,  219,  219,  219,
 /*  9940 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9950 */   219,  219,  219,  219,  219,  219,  106,  219,  219,  219,
 /*  9960 */   219,  219,  219,  219,  114,  115,  116,  117,  118,  219,
 /*  9970 */   219,  219,    7,  219,    9,   10,  219,  219,  219,  219,
 /*  9980 */    15,  219,  219,  219,  219,  219,   21,  219,  219,  219,
 /*  9990 */   219,  219,  219,   28,  219,  219,  219,  219,   33,   34,
 /* 10000 */    35,   36,   37,   38,   39,   40,   41,  219,    7,   44,
 /* 10010 */     9,   10,  219,  219,  219,  219,   15,  219,  219,   54,
 /* 10020 */   219,  219,   21,  219,   59,   60,   61,  219,  219,   28,
 /* 10030 */    65,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /* 10040 */    39,   40,   41,  219,  219,   44,  219,  219,  219,  219,
 /* 10050 */   219,  219,  219,  219,  219,   54,  219,  219,  219,  219,
 /* 10060 */    59,   60,   61,  219,  219,  219,   65,  219,  219,  219,
 /* 10070 */   219,  106,  219,  219,  219,  219,  219,  219,  219,  114,
 /* 10080 */   115,  116,  117,  118,  219,  219,  219,  219,  219,  219,
 /* 10090 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10100 */   219,  219,  219,  219,  219,  219,  219,  106,  219,  219,
 /* 10110 */   219,  219,  219,  219,  219,  114,  115,  116,  117,  118,
 /* 10120 */   219,  219,  219,  219,    8,  219,  219,   11,   12,   13,
 /* 10130 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /* 10140 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  219,
 /* 10150 */   219,    7,  219,    9,   10,  219,   40,   41,   42,   15,
 /* 10160 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /* 10170 */   219,  219,   28,  219,  219,  219,  219,   33,   34,   35,
 /* 10180 */    36,   37,   38,   39,  219,   41,  219,    7,   44,    9,
 /* 10190 */    10,  219,  219,  219,  219,  219,  219,  219,   54,  219,
 /* 10200 */   219,   21,  219,   59,   60,   61,  219,  219,   28,   65,
 /* 10210 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /* 10220 */   219,   41,  219,  219,   44,  219,  219,  219,  219,  219,
 /* 10230 */   219,  219,  219,  219,   54,  219,  219,  219,  219,   59,
 /* 10240 */    60,   61,  219,  219,  219,   65,  219,   67,  219,  219,
 /* 10250 */   106,  219,  219,  219,  219,  219,  219,  219,  114,  115,
 /* 10260 */   116,  117,  118,  219,  219,  219,  219,  219,  219,  219,
 /* 10270 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10280 */   219,  219,  219,  219,  219,  219,  106,  219,  219,  219,
 /* 10290 */   219,  219,  219,  219,  114,  115,  116,  117,  118,  219,
 /* 10300 */   219,  219,    7,  219,    9,   10,  219,  219,  219,  219,
 /* 10310 */    15,  219,  219,  219,  219,  219,   21,  219,  219,  219,
 /* 10320 */   219,  219,  219,   28,  219,  219,  219,  219,   33,   34,
 /* 10330 */    35,   36,   37,   38,   39,  219,   41,  219,    7,   44,
 /* 10340 */     9,   10,  219,  219,  219,  219,  219,  219,  219,   54,
 /* 10350 */   219,  219,   21,  219,   59,   60,   61,  219,  219,   28,
 /* 10360 */    65,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /* 10370 */    39,  219,   41,  219,  219,   44,  219,  219,  219,  219,
 /* 10380 */   219,  219,  219,  219,  219,   54,  219,  219,  219,  219,
 /* 10390 */    59,   60,   61,  219,  219,  219,   65,  219,  219,  219,
 /* 10400 */   219,  106,  219,  219,  219,  219,  219,  219,  219,  114,
 /* 10410 */   115,  116,  117,  118,  219,  219,  219,  219,  219,  219,
 /* 10420 */   219,  219,  219,  219,  219,  219,  219,  219,  219,   98,
 /* 10430 */   219,  219,  219,  219,  219,  219,  219,  106,  219,  219,
 /* 10440 */   219,  219,  219,  219,  219,  114,  115,  116,  117,  118,
 /* 10450 */   219,  219,  219,  219,    8,  219,  219,   11,   12,   13,
 /* 10460 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /* 10470 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  219,
 /* 10480 */     7,  219,    9,   10,  219,  219,  219,   41,   42,  219,
 /* 10490 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /* 10500 */    54,   28,  219,  219,  219,  219,   33,   34,   35,   36,
 /* 10510 */    37,   38,   39,  219,   41,  219,  219,   44,  219,  219,
 /* 10520 */   219,  219,  219,  219,  219,  219,  219,   54,  219,  219,
 /* 10530 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /* 10540 */    67,    8,  219,  219,   11,   12,   13,   14,   15,   16,
 /* 10550 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /* 10560 */    27,   28,   29,   30,   31,   32,  219,  219,  219,  219,
 /* 10570 */   219,  219,  219,  219,   41,   42,  219,  219,  219,  106,
 /* 10580 */   219,  219,    7,  219,    9,   10,  219,  114,  115,  116,
 /* 10590 */   117,  118,  219,  219,  219,  219,   21,  219,  219,  219,
 /* 10600 */    67,  219,  219,   28,  219,  219,  219,  219,   33,   34,
 /* 10610 */    35,   36,   37,   38,   39,  219,   41,  219,    7,   44,
 /* 10620 */     9,   10,  219,  219,  219,  219,  219,  219,  219,   54,
 /* 10630 */   219,  219,   21,  219,   59,   60,   61,  219,  219,   28,
 /* 10640 */    65,  219,   67,  219,   33,   34,   35,   36,   37,   38,
 /* 10650 */    39,  219,   41,  219,  219,   44,  219,  219,  219,  219,
 /* 10660 */   219,  219,  219,  219,  219,   54,  219,  219,  219,  219,
 /* 10670 */    59,   60,   61,  219,  219,  219,   65,  219,   67,  219,
 /* 10680 */   219,  106,  219,  219,    7,  219,    9,   10,  219,  114,
 /* 10690 */   115,  116,  117,  118,  219,  219,  219,  219,   21,  219,
 /* 10700 */   219,  219,  219,  219,  219,   28,  219,  219,  219,  219,
 /* 10710 */    33,   34,   35,   36,   37,   38,   39,  106,   41,  219,
 /* 10720 */     7,   44,    9,   10,  219,  114,  115,  116,  117,  118,
 /* 10730 */   219,   54,  219,  219,   21,  219,   59,   60,   61,  219,
 /* 10740 */   219,   28,   65,  219,   67,  219,   33,   34,   35,   36,
 /* 10750 */    37,   38,   39,  219,   41,  219,  219,   44,  219,  219,
 /* 10760 */   219,  219,  219,  219,  219,  219,  219,   54,  219,  219,
 /* 10770 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /* 10780 */    67,  219,  219,  106,  219,  219,    7,  219,    9,   10,
 /* 10790 */   219,  114,  115,  116,  117,  118,  219,  219,  219,  219,
 /* 10800 */    21,  219,  219,  219,  219,  219,  219,   28,  219,  219,
 /* 10810 */   219,  219,   33,   34,   35,   36,   37,   38,   39,  106,
 /* 10820 */    41,  219,    7,   44,    9,   10,  219,  114,  115,  116,
 /* 10830 */   117,  118,  219,   54,  219,  219,   21,  219,   59,   60,
 /* 10840 */    61,  219,  219,   28,   65,  219,   67,  219,   33,   34,
 /* 10850 */    35,   36,   37,   38,   39,  219,   41,  219,  219,   44,
 /* 10860 */    45,  219,  219,  219,  219,  219,  219,  219,  219,   54,
 /* 10870 */   219,  219,  219,  219,   59,   60,   61,  219,  219,  219,
 /* 10880 */    65,  219,  219,  219,  219,  106,  219,  219,  219,  219,
 /* 10890 */   219,  219,  219,  114,  115,  116,  117,  118,  219,  219,
 /* 10900 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10910 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10920 */   219,  106,  219,  219,  219,  219,  219,  219,  219,  114,
 /* 10930 */   115,  116,  117,  118,  219,  219,  219,  219,    8,  219,
 /* 10940 */   219,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /* 10950 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /* 10960 */    30,   31,   32,  219,  219,  219,  219,  219,  219,  219,
 /* 10970 */   219,   41,   42,  219,  219,   45,  219,  219,  219,  219,
 /* 10980 */     8,  219,  219,   11,   12,   13,   14,   15,   16,   17,
 /* 10990 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /* 11000 */    28,   29,   30,   31,   32,  219,  219,  219,  219,  219,
 /* 11010 */   219,  219,  219,   41,   42,  219,  219,   45,  219,  219,
 /* 11020 */   219,  219,    8,  219,  219,   11,   12,   13,   14,   15,
 /* 11030 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /* 11040 */    26,   27,   28,   29,   30,   31,   32,  219,  219,  219,
 /* 11050 */   219,  219,  219,  219,  219,   41,   42,  219,  219,   45,
 /* 11060 */   219,  219,  219,  219,    8,  219,  219,   11,   12,   13,
 /* 11070 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /* 11080 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  219,
 /* 11090 */   219,  219,  219,  219,  219,  219,  219,   41,   42,  219,
 /* 11100 */   219,   45,  219,  219,  219,  219,    8,  219,  219,   11,
 /* 11110 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /* 11120 */    22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
 /* 11130 */    32,  219,    7,  219,    9,   10,  219,  219,  219,   41,
 /* 11140 */    42,  219,  219,  219,  219,  219,   21,  219,  219,  219,
 /* 11150 */   219,  219,   54,   28,  219,  219,  219,  219,   33,   34,
 /* 11160 */    35,   36,   37,   38,   39,  219,   41,  219,  219,   44,
 /* 11170 */   219,  219,  219,  219,  219,  219,  219,  219,  219,   54,
 /* 11180 */   219,  219,  219,  219,   59,   60,   61,  219,    8,  219,
 /* 11190 */    65,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /* 11200 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /* 11210 */    30,   31,   32,  219,  219,  219,  219,  219,  219,  219,
 /* 11220 */   219,   41,   42,   98,  219,  219,  219,  219,  219,  219,
 /* 11230 */   219,  106,  219,  219,   54,  219,  219,  219,  219,  114,
 /* 11240 */   115,  116,  117,  118,  219,  219,  219,  219,    8,  219,
 /* 11250 */   219,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /* 11260 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /* 11270 */    30,   31,   32,  219,    7,  219,    9,   10,  219,  219,
 /* 11280 */   219,   41,   42,  219,  219,  219,  219,  219,   21,  219,
 /* 11290 */   219,  219,  219,  219,   54,   28,  219,  219,  219,  219,
 /* 11300 */    33,   34,   35,   36,   37,   38,   39,  219,   41,  219,
 /* 11310 */   219,   44,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11320 */   219,   54,  219,  219,  219,  219,   59,   60,   61,  219,
 /* 11330 */     8,  219,   65,   11,   12,   13,   14,   15,   16,   17,
 /* 11340 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /* 11350 */    28,   29,   30,   31,   32,  219,  219,  219,  219,  219,
 /* 11360 */   219,  219,  219,   41,   42,  219,  219,  219,  219,  219,
 /* 11370 */   219,  219,    7,  106,    9,   10,  219,  219,  219,  219,
 /* 11380 */   219,  114,  115,  116,  117,  118,   21,  219,  219,  219,
 /* 11390 */   219,  219,  219,   28,  219,  219,  219,  219,   33,   34,
 /* 11400 */    35,   36,   37,   38,   39,  219,   41,  219,  219,   44,
 /* 11410 */   219,  219,  219,  219,  219,  219,  219,  219,  219,   54,
 /* 11420 */   219,  219,  219,  219,   59,   60,   61,  219,  219,  219,
 /* 11430 */    65,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11440 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11450 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11460 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11470 */   219,  106,  219,  219,  219,  219,  219,  219,  219,  114,
 /* 11480 */   115,  116,  117,  118,
};
#define YY_SHIFT_USE_DFLT (-57)
static short yy_shift_ofst[] = {
 /*     0 */  6264,   60, 6183,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*    10 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*    20 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*    30 */    30,   69,  -57,  113,    0,  -57,  113,  -57,  230,  270,
 /*    40 */   -57,  -57,   41, 9361, 11267,  -26, 11267, 11267,  805, 11267,
 /*    50 */  11267,  -26, 11267, 11267, 11322, 11267, 11267,  -26, 11267, 11267,
 /*    60 */   -26, 11267, 11267, 6423, 11267, 11267, 6423, 11267,   90,  147,
 /*    70 */   338, 9397, 11322, 11267, 6499,  -57, 11267,  805, 11267,  805,
 /*    80 */  11267,  -26, 11267,  -26, 11267,  -26, 11267,  805, 11267, 6795,
 /*    90 */  11267, 6700, 11267, 6523, 11267, 6523, 11267, 6523, 11267, 6523,
 /*   100 */  11267, 6523, 11267, 9468, 11267, 9301, 11267, 11322, 6352,  -57,
 /*   110 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*   120 */   -57,  -57,  -57, 10116,  -57,  387, 11267,  -26,  369,  452,
 /*   130 */  11267,   90,  154,  -34,  437, 9512,  -56, 10144, 11322,  482,
 /*   140 */   527, 11267,  -26,  -57, 11267,  -26,  -57,  -57,  -57,  -57,
 /*   150 */   -57,  -57,  -57,  -57, 10180, 11322,  103,  481,  509,  550,
 /*   160 */   -57,  233,  -57, 11365,  194,  479, 9548,  -57,  447,  -57,
 /*   170 */  10295,  536,  598,  267, 9663,  471,  -57,  -57,  -57,  -57,
 /*   180 */   -57,  -57, 11267, 11322,  599, 6875,  529, 8259,  -57,  610,
 /*   190 */  6794,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  586,
 /*   200 */   679,  -57,  -57, 6961,  691,  722,  329,  -57,  280,  -57,
 /*   210 */  7140,  -57,  735, 6794,  -57,  -57,  -57,  -57, 6916,  791,
 /*   220 */  6794,  -57,  129,  812, 6794,  -57,  849,  907, 6794,  -57,
 /*   230 */   938,  926, 6794,  -57,  948, 1003,  -57,  373,  972, 6794,
 /*   240 */   -57, 1009, 1006, 6794,  -57, 1024, 1034, 6794,  -57, 1041,
 /*   250 */    -7,  106,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*   260 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*   270 */   -57,  -57,  -57,  -57,  -57,  -57,  -57, 1045,  -57, 1053,
 /*   280 */   -57, 11267, 1051,  219,  332,  110,  417, 1060,  445,  558,
 /*   290 */   -57, 11267, 1061,   40,  -57,  182,  -57,  -57, 6794, 1049,
 /*   300 */  6095, 6095, 1085,  671,  784,  -57, 11267, 1091,  897, 1010,
 /*   310 */   -57, 1096, 1123, 1236, 1071, 11267, 1107,  -57, 11322, 1112,
 /*   320 */  1349, 1462, 1080, 1080,  -57, 1136,  245, 1575, 1688,  -57,
 /*   330 */  1137,  340, 10331, 10446, 1801, 1914,  -57,  706,  451,  -57,
 /*   340 */   706,  -57, 6790,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*   350 */  11267,  -57, 11322,  227, 7058, 11267,  -57, 10473,  695, 11267,
 /*   360 */   -57, 1141,  -57, 10533, 6184, 11267,  -57, 10575,  695, 11267,
 /*   370 */   -57,  -57,  -57,  -57,  -57,   53, 1163,  695, 11267,  -57,
 /*   380 */  1189,  695, 11267,  -57, 1207, 6265, 11267,  -57, 10611,  695,
 /*   390 */  11267,  -57, 6801, 11267,  -57, 10677,  695, 11267,  -57, 10713,
 /*   400 */   695, 11267,  -57, 7136, 11267,  -57, 10779,  695, 11267,  -57,
 /*   410 */   -57,  -57, 1212,  -57, 1221,  -57,  505, 1224,  695, 11267,
 /*   420 */   -57, 1232,  695, 11267,  -57,  -57, 11267,  528,  -57, 11267,
 /*   430 */   -57, 11322,  -57, 1231,  -57, 1248,  -57, 1253,  -57, 1254,
 /*   440 */   -57, 10815, 10930,  -57,  -57, 11267, 10972,  -57, 11267, 11014,
 /*   450 */   -57, 11267, 11056,  -57, 1256,  570,  -57, 1256,  -57, 1245,
 /*   460 */  6794,  -57,  -57, 1256,  579,  -57, 1256,  637,  -57, 1256,
 /*   470 */   638,  -57, 1256,  642,  -57, 1256,  683,  -57, 1256,  684,
 /*   480 */   -57, 1256,  692,  -57, 1256,  729,  -57, 1256,  796,  -57,
 /*   490 */  1256,  797,  -57, 11322,  -57,  -57,  -57,  -57, 11267, 11098,
 /*   500 */  6095, 2027,  -57, 1272, 1225, 11125, 11180, 2140, 2253,  -57,
 /*   510 */   -57, 11267, 11240, 6095, 2366,  -57,  -57, 1277, 1270, 2479,
 /*   520 */  2592,  -57,  -57, 1136,  -57,  -57,  -57,  -57,  -57, 1265,
 /*   530 */  11267, 1295,  -57,  -57,  -57, 1271, 6095, 6095,  -57,  -57,
 /*   540 */   -57, 11267, 1291, 2705, 2818,  -57,  -57, 1307, 2931, 3044,
 /*   550 */   -57,  -57,  -57,  741,  778, 1309, 1310,  -57, 1312, 3157,
 /*   560 */  3270,  -57,  -57,  -57,  -57,  -57, 1313, 3383, 3496,  -57,
 /*   570 */   -57,  300, 1303, 9699,  868,  -57,  -57, 1324, 1314, 1315,
 /*   580 */  9814,  899,  -57,  -57,  -57, 1327, 1323, 1320, 9850,  -57,
 /*   590 */   920,  -57,  -57, 1300, 11267,  -57,  -57,  -57, 11267, 11322,
 /*   600 */   921,  -57,  -57,  -57,  923,  -57,  -57,  510, 1336, 1331,
 /*   610 */  9965,  981,  -57,  -57, 1345, 1340, 10001, 1012,  -57,  -57,
 /*   620 */    90,   90,   90,   90,   90,   90,   90, 1021,  -57,  -57,
 /*   630 */  1358,  437, 1362,  418,  -57, 1367, 1359,  -57,   38,  -57,
 /*   640 */  1360,  -57,  151,  716,  -57, 1846, 1384, 1368, 7048,  545,
 /*   650 */  7224, 1385,  -57,  -57, 1420, 8434,  -57, 1396,  -57,  -57,
 /*   660 */   -57,  -57,  -57, 1393,  419, 1371, 1417,  -57,  -57,  -57,
 /*   670 */  1022,  580, 7224, 1399,  -57,  -57,  -57,  -57,  -57,  -57,
 /*   680 */   -57,  -57,  -57,  -57,  -57,  -57, 2411, 1959, 1407, 1391,
 /*   690 */  7394,  658, 7224, 1408,  -57,  -57, 1036,  771, 7224, 1409,
 /*   700 */   -57,  -57,  -57,  -57,  -57, 2072,  707, 1401, 6794, 1428,
 /*   710 */   -57, 1419, 6794, 1429,  -57,  828, 1421, 6794, 1434,  -57,
 /*   720 */  1425, 6794, 1439,  -57,  829,  -57, 1433,  410,  -57, 1450,
 /*   730 */   819,  -57, 1458,  705,  -57,  264,  -57, 1459,  -57,  377,
 /*   740 */   942,  -57, 2185, 1469, 1454, 7567,  104, 3609,  -57, 3722,
 /*   750 */   -57,  -57, 7224,  747, 3835,  -57, 3948,  -57,  -57, 1094,
 /*   760 */   298, 4061,  -57, 4174,  -57,  -57, 7224,  834, 4287,  -57,
 /*   770 */  4400,  -57,  -57, 2411, 2298, 1475, 1463, 7740,  330, 4513,
 /*   780 */   -57, 4626,  -57,  -57, 7224,  895, 4739,  -57, 4852,  -57,
 /*   790 */   -57, 1125,  443, 4965,  -57, 5078,  -57,  -57, 7224,  905,
 /*   800 */  5191,  -57, 5304,  -57,  -57,  490, 1055,  -57, 2072,  -57,
 /*   810 */  2072, 1168,  521,  -57, 6794,  931,  -57, 1481,  -57,  -20,
 /*   820 */  1484,  379, 1496,  717,  -57,  -57, 1499,  -57,  -57, 1502,
 /*   830 */   -57, 1281,  634,  -57, 6794,  932,  -57, 1506,  -57, 1512,
 /*   840 */   -57,  603, 1394, 1507, 2411, 1620,  -57, 1733, 1458,  -57,
 /*   850 */   -57,  -57, 1458,  705,  -57, 1515, 1523,  310,  -57, 1539,
 /*   860 */   835,  -57, 1458,  705,  -57, 1458,  705,  -57, 1536, 1543,
 /*   870 */   523,  -57, 1548, 1544,  -57, 1458,  705,  -57, 1560, 1546,
 /*   880 */  7913, 1551, 6095, 5417,  -57, 1033, 1561, 5530,  -57, 5643,
 /*   890 */   -57, 1557, 1574, 1565, 8086, 1573, 6095, 5756,  -57, 1149,
 /*   900 */  1578, 5869,  -57, 5982,  -57,  202,  962, 1052, 1075, 1130,
 /*   910 */  1142, 1143, 1188, 1203, 1208, 1234, 1248, 1253, 1254, 11267,
 /*   920 */  10972,  -57,
};
#define YY_REDUCE_USE_DFLT (-164)
static short yy_reduce_ofst[] = {
 /*     0 */  6356, -164, 6459, -164, -164, -164, -164, -164, -164, -164,
 /*    10 */  -164, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*    20 */  -164, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*    30 */  -164, -164, -164,  -53, -164, -164,   68, -164, -164, -164,
 /*    40 */  -164, -164, -164,   39, 2263, -164, 2644, 2715, -164, 2757,
 /*    50 */  2778, -164, 2828, 2870, -164, 2891, 2941, -164, 2983, 3004,
 /*    60 */  -164, 3054, 3096, -164, 3117, 3167, -164, 3209, -164, -164,
 /*    70 */  -164,  152, -164, 3230, -164, -164, 3280, -164, 3322, -164,
 /*    80 */  3343, -164, 3393, -164, 3435, -164, 3456, -164, 3506, -164,
 /*    90 */  3548, -164, 3569, -164, 3619, -164, 3661, -164, 3682, -164,
 /*   100 */  3732, -164, 3767, -164, 3774, -164, 3795, -164, 2326, -164,
 /*   110 */  -164, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*   120 */  -164, -164, -164, -164, -164, -164, 3845, -164, -164, -164,
 /*   130 */  3880, -164, -164, -164, -164,  265, -164, 3887, -164, -164,
 /*   140 */  -164, 3908, -164, -164, 3958, -164, -164, -164, -164, -164,
 /*   150 */  -164, -164, -164, -164, 6364, -164, -164, -164, -164, -164,
 /*   160 */  -164, -164, -164, 6446, -164, -164,  378, -164, -164, -164,
 /*   170 */  1734, -164, -164, -164,  491, -164, -164, -164, -164, -164,
 /*   180 */  -164, -164, 3993, -164, -164, -158, -164,  -54, -164, -164,
 /*   190 */   560, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*   200 */  -164, -164, -164,  925, -164, -164, -164, -164, -164, -164,
 /*   210 */   -31, -164, -164, -163, -164, -164, -164, -164,  -95, -164,
 /*   220 */   701, -164, -164, -164,  745, -164, -164, -164,  814, -164,
 /*   230 */  -164, -164,  860, -164, -164, -164, -164, -164, -164,  886,
 /*   240 */  -164, -164, -164,  903, -164, -164, -164,  913, -164, -164,
 /*   250 */  6485, 9157, -164, -164, -164, -164, -164, -164, -164, -164,
 /*   260 */  -164, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*   270 */  -164, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*   280 */  -164, 1020, -164, 6562, 9157,  235,  927, -164, 6658, 9157,
 /*   290 */  -164, 1133, -164,  320, -164,  911, -164, -164,  973, -164,
 /*   300 */  6735, 9157, -164, 6831, 9157, -164, 1698, -164, 6908, 9157,
 /*   310 */  -164, -164, 7004, 9157, -164, 1811, -164, -164, -164, -164,
 /*   320 */  7081, 9157,  335,  986, -164,  339, -164, 7177, 9157, -164,
 /*   330 */  -164, -164, 4000, -164, 7254, 9157, -164,  406, -164, -164,
 /*   340 */   990, -164,   35, -164, -164, -164, -164, -164, -164, -164,
 /*   350 */   -68, -164, -164, -164,  109, 1847, -164, 1960, 1013, 2073,
 /*   360 */  -164, -164, -164, -164,   12, 2186, -164, 1960, 1015, 2412,
 /*   370 */  -164, -164, -164, -164, -164, -164, -164, 1031, 2524, -164,
 /*   380 */  -164, 1054, 2637, -164, -164,  125, 2750, -164, 1960, 1057,
 /*   390 */  2863, -164,  313, 2976, -164, 1960, 1058, 3089, -164, 1960,
 /*   400 */  1059, 3202, -164,  351, 3315, -164, 1960, 1062, 3428, -164,
 /*   410 */  -164, -164, -164, -164, -164, -164, -164, -164, 1092, 3541,
 /*   420 */  -164, -164, 1093, 3654, -164, -164,  718, -164, -164, 2439,
 /*   430 */  -164, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*   440 */  -164, 4021, -164, -164, -164, 4071, -164, -164, 4106, -164,
 /*   450 */  -164, 4113, -164, -164,  402, -164, -164, 1095, -164, -164,
 /*   460 */  1151, -164, -164,  413, -164, -164,  430, -164, -164,  439,
 /*   470 */  -164, -164,  455, -164, -164,  478, -164, -164,  506, -164,
 /*   480 */  -164,  515, -164, -164,  526, -164, -164,  549, -164, -164,
 /*   490 */   568, -164, -164, -164, -164, -164, -164, -164, 4134, -164,
 /*   500 */  7350, 9157, -164, -164, -164, 4184, -164, 7427, 9157, -164,
 /*   510 */  -164, 4219, -164, 7523, 9157, -164, -164, -164, -164, 7600,
 /*   520 */  9157, -164, -164, 1139, -164, -164, -164, -164, -164, -164,
 /*   530 */  2037, -164, -164, -164, -164, -164, 7696, 9157, -164, -164,
 /*   540 */  -164, 2150, -164, 7773, 9157, -164, -164, -164, 7869, 9157,
 /*   550 */  -164, -164, -164,  665,  927, -164, -164, -164, -164, 7946,
 /*   560 */  9157, -164, -164, -164, -164, -164, -164, 8042, 9157, -164,
 /*   570 */  -164, -164, -164,  830, -164, -164, -164, -164, -164, -164,
 /*   580 */   943, -164, -164, -164, -164, -164, -164, -164, 1056, -164,
 /*   590 */  -164, -164, -164, -164, 2531, -164, -164, -164, 4226, -164,
 /*   600 */  -164, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*   610 */  1169, -164, -164, -164, -164, -164, 1282, -164, -164, -164,
 /*   620 */  -164, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*   630 */  -164, -164, -164, 1255, -164, -164, 1260, -164, 1432, -164,
 /*   640 */  -164, -164, 4327,  220, -164, 1242, -164, -164, -121, -164,
 /*   650 */   631, -164, -164, -164, -164,  172, -164, -164, -164, -164,
 /*   660 */  -164, -164, -164, -164, -164, -164, -164, -164, -164, -164,
 /*   670 */  -164, -164,  737, -164, -164, -164, -164, -164, -164, -164,
 /*   680 */  -164, -164, -164, -164, -164, -164,  248, 1242, -164, -164,
 /*   690 */   -99, -164, 2319, -164, -164, -164, -164, -164, 2404, -164,
 /*   700 */  -164, -164, -164, -164, -164,  248, -164, -164, 1306, -164,
 /*   710 */  -164, -164, 1316, -164, -164, -164, -164, 1317, -164, -164,
 /*   720 */  -164, 1325, -164, -164,  220, -164, -164, 1338, -164, -164,
 /*   730 */  1351, -164, -110, 1352, -164, -151, -164, -164, -164, 1545,
 /*   740 */   135, -164, 1242, -164, -164,   -8, -164, 8119, -164, 9157,
 /*   750 */  -164, -164, 2517, -164, 8215, -164, 9157, -164, -164, -164,
 /*   760 */  -164, 8292, -164, 9157, -164, -164, 2578, -164, 8388, -164,
 /*   770 */  9157, -164, -164,  754, 1242, -164, -164,  105, -164, 8465,
 /*   780 */  -164, 9157, -164, -164, 2630, -164, 8561, -164, 9157, -164,
 /*   790 */  -164, -164, -164, 8638, -164, 9157, -164, -164, 2691, -164,
 /*   800 */  8734, -164, 9157, -164, -164,   67,  135, -164,  754, -164,
 /*   810 */   863, 1242, 1355, -164, 1361, 1363, -164, -164, -164,  788,
 /*   820 */  -164, -164, -164, 1375, -164, -164, -164, -164, -164, -164,
 /*   830 */  -164, 1242, 1383, -164, 1386, 1387, -164, -164, -164, -164,
 /*   840 */  -164, 1658, 1953,  135,  774,  135, -164,  135, 1410, -164,
 /*   850 */  -164, -164,  845, 1411, -164, -164, -164, 1418, -164, -164,
 /*   860 */  1435, -164,  846, 1437, -164,  876, 1441, -164, -164, -164,
 /*   870 */  1444, -164, -164, 1446, -164,  918, 1448, -164, -164, -164,
 /*   880 */   218, -164, 8811, 9157, -164, -164, -164, 8907, -164, 9157,
 /*   890 */  -164, -164, -164, -164,  285, -164, 8984, 9157, -164, -164,
 /*   900 */  -164, 9080, -164, 9157, -164,  402,  413,  455,  478,  430,
 /*   910 */   439,  506,  526,  515,  568,  549, -164, -164, -164, 4247,
 /*   920 */  -164, -164,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */  1368, 1368, 1368,  924,  926,  927,  928,  929,  930,  931,
 /*    10 */   932,  933,  934,  935,  936,  937,  938,  939,  940,  941,
 /*    20 */   942,  943,  944,  945,  946,  947,  948,  949,  950,  951,
 /*    30 */  1368, 1368,  952, 1368, 1368,  953, 1368,  954,  956, 1368,
 /*    40 */   957,  955,  956, 1368, 1368, 1248, 1368, 1368, 1249, 1368,
 /*    50 */  1368, 1250, 1368, 1368, 1251, 1368, 1368, 1252, 1368, 1368,
 /*    60 */  1253, 1368, 1368, 1254, 1368, 1368, 1255, 1368, 1263, 1368,
 /*    70 */  1267, 1368, 1329, 1368, 1368, 1272, 1368, 1273, 1368, 1274,
 /*    80 */  1368, 1275, 1368, 1276, 1368, 1277, 1368, 1278, 1368, 1279,
 /*    90 */  1368, 1280, 1368, 1281, 1368, 1282, 1368, 1283, 1368, 1284,
 /*   100 */  1368, 1285, 1368, 1286, 1368, 1368, 1368, 1326, 1368, 1095,
 /*   110 */  1096, 1097, 1098, 1099, 1100, 1101, 1102, 1103, 1104, 1105,
 /*   120 */  1106, 1107, 1108, 1368, 1264, 1368, 1368, 1265, 1368, 1368,
 /*   130 */  1368, 1266, 1290, 1368, 1270, 1368, 1290, 1368, 1330, 1368,
 /*   140 */  1368, 1368, 1287, 1288, 1368, 1289, 1291, 1292, 1293, 1294,
 /*   150 */  1295, 1296, 1297, 1298, 1368, 1345, 1290, 1291, 1292, 1298,
 /*   160 */  1299, 1368, 1300, 1368, 1368, 1301, 1368, 1302, 1368, 1303,
 /*   170 */  1368, 1368, 1368, 1368, 1368, 1368, 1309, 1310, 1323, 1324,
 /*   180 */  1325, 1328, 1368, 1331, 1368, 1368, 1368, 1368, 1075, 1077,
 /*   190 */  1368, 1085, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1368,
 /*   200 */  1368, 1353, 1354, 1368, 1346, 1348, 1368, 1355, 1368, 1356,
 /*   210 */  1368, 1357, 1368, 1368, 1359, 1364, 1360, 1358, 1368, 1078,
 /*   220 */  1368, 1086, 1368, 1080, 1368, 1088, 1368, 1082, 1368, 1090,
 /*   230 */  1368, 1084, 1368, 1092, 1368, 1368, 1093, 1368, 1079, 1368,
 /*   240 */  1087, 1368, 1081, 1368, 1089, 1368, 1083, 1368, 1091, 1368,
 /*   250 */  1368, 1368, 1109, 1111, 1112, 1113, 1114, 1115, 1116, 1117,
 /*   260 */  1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126, 1127,
 /*   270 */  1128, 1129, 1130, 1131, 1132, 1133, 1134, 1368, 1135, 1368,
 /*   280 */  1136, 1368, 1368, 1368, 1368, 1141, 1142, 1368, 1368, 1368,
 /*   290 */  1144, 1368, 1368, 1368, 1152, 1368, 1153, 1154, 1368, 1368,
 /*   300 */  1156, 1157, 1368, 1368, 1368, 1160, 1368, 1368, 1368, 1368,
 /*   310 */  1162, 1368, 1368, 1368, 1368, 1368, 1368, 1164, 1365, 1368,
 /*   320 */  1368, 1368, 1166, 1167, 1168, 1368, 1368, 1368, 1368, 1170,
 /*   330 */  1368, 1368, 1368, 1368, 1368, 1368, 1177, 1368, 1368, 1183,
 /*   340 */  1368, 1184, 1368, 1186, 1187, 1188, 1189, 1190, 1191, 1192,
 /*   350 */  1368, 1193, 1247, 1368, 1368, 1368, 1194, 1368, 1368, 1368,
 /*   360 */  1197, 1368, 1209, 1368, 1368, 1368, 1198, 1368, 1368, 1368,
 /*   370 */  1199, 1207, 1208, 1210, 1211, 1368, 1368, 1368, 1368, 1195,
 /*   380 */  1368, 1368, 1368, 1196, 1368, 1368, 1368, 1200, 1368, 1368,
 /*   390 */  1368, 1201, 1368, 1368, 1202, 1368, 1368, 1368, 1203, 1368,
 /*   400 */  1368, 1368, 1204, 1368, 1368, 1205, 1368, 1368, 1368, 1206,
 /*   410 */  1212, 1214, 1368, 1213, 1368, 1215, 1368, 1368, 1368, 1368,
 /*   420 */  1216, 1368, 1368, 1368, 1217, 1185, 1368, 1368, 1219, 1368,
 /*   430 */  1220, 1222, 1221, 1323, 1223, 1325, 1224, 1324, 1225, 1288,
 /*   440 */  1226, 1368, 1368, 1227, 1228, 1368, 1368, 1229, 1368, 1368,
 /*   450 */  1230, 1368, 1368, 1231, 1368, 1368, 1232, 1368, 1243, 1245,
 /*   460 */  1368, 1246, 1244, 1368, 1368, 1233, 1368, 1368, 1234, 1368,
 /*   470 */  1368, 1235, 1368, 1368, 1236, 1368, 1368, 1237, 1368, 1368,
 /*   480 */  1238, 1368, 1368, 1239, 1368, 1368, 1240, 1368, 1368, 1241,
 /*   490 */  1368, 1368, 1242, 1368, 1366, 1367, 1110, 1178, 1368, 1368,
 /*   500 */  1368, 1368, 1179, 1368, 1368, 1368, 1368, 1368, 1368, 1180,
 /*   510 */  1181, 1368, 1368, 1368, 1368, 1182, 1171, 1368, 1368, 1368,
 /*   520 */  1368, 1173, 1172, 1368, 1174, 1176, 1175, 1169, 1165, 1368,
 /*   530 */  1368, 1368, 1163, 1161, 1159, 1368, 1368, 1158, 1155, 1146,
 /*   540 */  1148, 1368, 1368, 1368, 1368, 1151, 1150, 1368, 1368, 1368,
 /*   550 */  1143, 1145, 1149, 1137, 1138, 1368, 1368, 1140, 1368, 1368,
 /*   560 */  1368, 1147, 1139, 1336, 1335, 1076, 1368, 1368, 1368, 1334,
 /*   570 */  1333, 1368, 1368, 1368, 1368, 1313, 1314, 1368, 1368, 1368,
 /*   580 */  1368, 1368, 1315, 1316, 1327, 1368, 1368, 1304, 1368, 1305,
 /*   590 */  1368, 1306, 1337, 1368, 1368, 1339, 1340, 1338, 1368, 1332,
 /*   600 */  1368, 1311, 1312, 1271, 1368, 1317, 1318, 1368, 1368, 1268,
 /*   610 */  1368, 1368, 1319, 1320, 1368, 1269, 1368, 1368, 1321, 1322,
 /*   620 */  1262, 1261, 1260, 1259, 1258, 1257, 1256, 1368, 1307, 1308,
 /*   630 */  1368, 1368, 1368, 1368,  958, 1368, 1368,  959, 1368,  976,
 /*   640 */  1368,  977, 1368, 1368, 1010, 1368, 1368, 1368, 1368, 1368,
 /*   650 */  1368, 1368, 1040, 1059, 1060, 1368, 1061, 1063, 1066, 1064,
 /*   660 */  1065, 1067, 1068, 1368, 1368, 1368, 1368, 1094, 1062, 1044,
 /*   670 */  1368, 1368, 1368, 1368, 1041, 1045, 1048, 1050, 1051, 1052,
 /*   680 */  1053, 1054, 1055, 1056, 1057, 1058, 1368, 1368, 1368, 1368,
 /*   690 */  1368, 1368, 1368, 1368, 1042, 1046, 1368, 1368, 1368, 1368,
 /*   700 */  1043, 1047, 1049, 1006, 1011, 1368, 1368, 1368, 1368, 1368,
 /*   710 */  1012, 1368, 1368, 1368, 1014, 1368, 1368, 1368, 1368, 1013,
 /*   720 */  1368, 1368, 1368, 1015, 1368, 1007, 1368, 1368,  960, 1368,
 /*   730 */  1368,  961, 1368, 1368,  963, 1368,  971, 1368,  972, 1368,
 /*   740 */  1368, 1008, 1368, 1368, 1368, 1368, 1368, 1368, 1016, 1368,
 /*   750 */  1020, 1017, 1368, 1368, 1368, 1028, 1368, 1032, 1029, 1368,
 /*   760 */  1368, 1368, 1018, 1368, 1021, 1019, 1368, 1368, 1368, 1030,
 /*   770 */  1368, 1033, 1031, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   780 */  1022, 1368, 1026, 1023, 1368, 1368, 1368, 1034, 1368, 1038,
 /*   790 */  1035, 1368, 1368, 1368, 1024, 1368, 1027, 1025, 1368, 1368,
 /*   800 */  1368, 1036, 1368, 1039, 1037, 1368, 1368, 1009, 1368,  990,
 /*   810 */  1368, 1368, 1368,  992, 1368, 1368,  994, 1368,  998, 1368,
 /*   820 */  1368, 1368, 1368, 1368, 1002, 1004, 1368, 1005, 1003, 1368,
 /*   830 */   996, 1368, 1368,  993, 1368, 1368,  995, 1368,  999, 1368,
 /*   840 */   997, 1368, 1368, 1368, 1368, 1368,  991, 1368, 1368,  973,
 /*   850 */   975,  974, 1368, 1368,  962, 1368, 1368, 1368,  964, 1368,
 /*   860 */  1368,  965, 1368, 1368,  967, 1368, 1368,  966, 1368, 1368,
 /*   870 */  1368,  968, 1368, 1368,  969, 1368, 1368,  970, 1368, 1368,
 /*   880 */  1368, 1368, 1368, 1368, 1069, 1368, 1368, 1368, 1070, 1368,
 /*   890 */  1071, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1072, 1368,
 /*   900 */  1368, 1368, 1073, 1368, 1074, 1368, 1368, 1368, 1368, 1368,
 /*   910 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   920 */  1368,  925,
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
  "xx_interface_def",  "xx_function_def",  "xx_comment",    "xx_cblock",   
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
  "xx_parameter",  "xx_statement",  "xx_mcall_statement",  "xx_empty_statement",
  "xx_eval_expr",  "xx_elseif_statements",  "xx_elseif_statement",  "xx_case_clauses",
  "xx_case_clause",  "xx_catch_statement_list",  "xx_catch_statement",  "xx_catch_classes_list",
  "xx_catch_class",  "xx_common_expr",  "xx_let_assignments",  "xx_let_assignment",
  "xx_assignment_operator",  "xx_assign_expr",  "xx_array_offset_list",  "xx_array_offset",
  "xx_index_expr",  "xx_echo_expressions",  "xx_echo_expression",  "xx_mcall_expr",
  "xx_fcall_expr",  "xx_scall_expr",  "xx_fetch_expr",  "xx_declare_variable_list",
  "xx_declare_variable",  "xx_array_list",  "xx_call_parameters",  "xx_call_parameter",
  "xx_array_item",  "xx_array_key",  "xx_array_value",  "xx_literal_array_list",
  "xx_literal_array_item",  "xx_literal_array_key",  "xx_literal_array_value",
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
 /*   8 */ "xx_top_statement ::= xx_function_def",
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
 /* 147 */ "xx_function_def ::= FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 148 */ "xx_function_def ::= FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 149 */ "xx_function_def ::= FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 150 */ "xx_function_def ::= xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 151 */ "xx_function_def ::= xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 152 */ "xx_function_def ::= xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 153 */ "xx_parameter_list ::= xx_parameter_list COMMA xx_parameter",
 /* 154 */ "xx_parameter_list ::= xx_parameter",
 /* 155 */ "xx_parameter ::= IDENTIFIER",
 /* 156 */ "xx_parameter ::= CONST IDENTIFIER",
 /* 157 */ "xx_parameter ::= xx_parameter_type IDENTIFIER",
 /* 158 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER",
 /* 159 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER",
 /* 160 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER",
 /* 161 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER",
 /* 162 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER",
 /* 163 */ "xx_parameter ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 164 */ "xx_parameter ::= CONST IDENTIFIER ASSIGN xx_literal_expr",
 /* 165 */ "xx_parameter ::= xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 166 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 167 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 168 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 169 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 170 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 171 */ "xx_parameter_cast ::= LESS IDENTIFIER GREATER",
 /* 172 */ "xx_parameter_cast_collection ::= LESS IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE GREATER",
 /* 173 */ "xx_parameter_type ::= TYPE_INTEGER",
 /* 174 */ "xx_parameter_type ::= TYPE_UINTEGER",
 /* 175 */ "xx_parameter_type ::= TYPE_LONG",
 /* 176 */ "xx_parameter_type ::= TYPE_ULONG",
 /* 177 */ "xx_parameter_type ::= TYPE_CHAR",
 /* 178 */ "xx_parameter_type ::= TYPE_UCHAR",
 /* 179 */ "xx_parameter_type ::= TYPE_DOUBLE",
 /* 180 */ "xx_parameter_type ::= TYPE_BOOL",
 /* 181 */ "xx_parameter_type ::= TYPE_STRING",
 /* 182 */ "xx_parameter_type ::= TYPE_ARRAY",
 /* 183 */ "xx_parameter_type ::= TYPE_VAR",
 /* 184 */ "xx_parameter_type ::= TYPE_CALLABLE",
 /* 185 */ "xx_parameter_type ::= TYPE_RESOURCE",
 /* 186 */ "xx_parameter_type ::= TYPE_OBJECT",
 /* 187 */ "xx_statement_list ::= xx_statement_list xx_statement",
 /* 188 */ "xx_statement_list ::= xx_statement",
 /* 189 */ "xx_statement ::= xx_cblock",
 /* 190 */ "xx_statement ::= xx_let_statement",
 /* 191 */ "xx_statement ::= xx_if_statement",
 /* 192 */ "xx_statement ::= xx_loop_statement",
 /* 193 */ "xx_statement ::= xx_echo_statement",
 /* 194 */ "xx_statement ::= xx_return_statement",
 /* 195 */ "xx_statement ::= xx_require_statement",
 /* 196 */ "xx_statement ::= xx_fetch_statement",
 /* 197 */ "xx_statement ::= xx_fcall_statement",
 /* 198 */ "xx_statement ::= xx_mcall_statement",
 /* 199 */ "xx_statement ::= xx_scall_statement",
 /* 200 */ "xx_statement ::= xx_unset_statement",
 /* 201 */ "xx_statement ::= xx_throw_statement",
 /* 202 */ "xx_statement ::= xx_declare_statement",
 /* 203 */ "xx_statement ::= xx_break_statement",
 /* 204 */ "xx_statement ::= xx_continue_statement",
 /* 205 */ "xx_statement ::= xx_while_statement",
 /* 206 */ "xx_statement ::= xx_do_while_statement",
 /* 207 */ "xx_statement ::= xx_try_catch_statement",
 /* 208 */ "xx_statement ::= xx_switch_statement",
 /* 209 */ "xx_statement ::= xx_for_statement",
 /* 210 */ "xx_statement ::= xx_comment",
 /* 211 */ "xx_statement ::= xx_empty_statement",
 /* 212 */ "xx_empty_statement ::= DOTCOMMA",
 /* 213 */ "xx_break_statement ::= BREAK DOTCOMMA",
 /* 214 */ "xx_continue_statement ::= CONTINUE DOTCOMMA",
 /* 215 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 216 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements",
 /* 217 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 218 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 219 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 220 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements",
 /* 221 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 222 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 223 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 224 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 225 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 226 */ "xx_elseif_statements ::= xx_elseif_statements xx_elseif_statement",
 /* 227 */ "xx_elseif_statements ::= xx_elseif_statement",
 /* 228 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 229 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 230 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 231 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN xx_case_clauses BRACKET_CLOSE",
 /* 232 */ "xx_case_clauses ::= xx_case_clauses xx_case_clause",
 /* 233 */ "xx_case_clauses ::= xx_case_clause",
 /* 234 */ "xx_case_clause ::= CASE xx_literal_expr COLON",
 /* 235 */ "xx_case_clause ::= CASE xx_literal_expr COLON xx_statement_list",
 /* 236 */ "xx_case_clause ::= DEFAULT COLON xx_statement_list",
 /* 237 */ "xx_loop_statement ::= LOOP BRACKET_OPEN BRACKET_CLOSE",
 /* 238 */ "xx_loop_statement ::= LOOP BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 239 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 240 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 241 */ "xx_do_while_statement ::= DO BRACKET_OPEN BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 242 */ "xx_do_while_statement ::= DO BRACKET_OPEN xx_statement_list BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 243 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN BRACKET_CLOSE",
 /* 244 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 245 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_catch_statement_list",
 /* 246 */ "xx_catch_statement_list ::= xx_catch_statement_list xx_catch_statement",
 /* 247 */ "xx_catch_statement_list ::= xx_catch_statement",
 /* 248 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 249 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN BRACKET_CLOSE",
 /* 250 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN BRACKET_CLOSE",
 /* 251 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 252 */ "xx_catch_classes_list ::= xx_catch_classes_list BITWISE_OR xx_catch_class",
 /* 253 */ "xx_catch_classes_list ::= xx_catch_class",
 /* 254 */ "xx_catch_class ::= IDENTIFIER",
 /* 255 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 256 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 257 */ "xx_for_statement ::= FOR IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 258 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 259 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 260 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 261 */ "xx_let_statement ::= LET xx_let_assignments DOTCOMMA",
 /* 262 */ "xx_let_assignments ::= xx_let_assignments COMMA xx_let_assignment",
 /* 263 */ "xx_let_assignments ::= xx_let_assignment",
 /* 264 */ "xx_assignment_operator ::= ASSIGN",
 /* 265 */ "xx_assignment_operator ::= ADDASSIGN",
 /* 266 */ "xx_assignment_operator ::= SUBASSIGN",
 /* 267 */ "xx_assignment_operator ::= MULASSIGN",
 /* 268 */ "xx_assignment_operator ::= DIVASSIGN",
 /* 269 */ "xx_assignment_operator ::= CONCATASSIGN",
 /* 270 */ "xx_assignment_operator ::= MODASSIGN",
 /* 271 */ "xx_let_assignment ::= IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 272 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 273 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 274 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 275 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 276 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 277 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 278 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 279 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 280 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 281 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 282 */ "xx_let_assignment ::= IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 283 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 284 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 285 */ "xx_array_offset_list ::= xx_array_offset_list xx_array_offset",
 /* 286 */ "xx_array_offset_list ::= xx_array_offset",
 /* 287 */ "xx_array_offset ::= SBRACKET_OPEN xx_index_expr SBRACKET_CLOSE",
 /* 288 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER INCR",
 /* 289 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER DECR",
 /* 290 */ "xx_let_assignment ::= IDENTIFIER INCR",
 /* 291 */ "xx_let_assignment ::= INCR IDENTIFIER",
 /* 292 */ "xx_let_assignment ::= IDENTIFIER DECR",
 /* 293 */ "xx_let_assignment ::= DECR IDENTIFIER",
 /* 294 */ "xx_let_assignment ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 295 */ "xx_let_assignment ::= BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 296 */ "xx_index_expr ::= xx_common_expr",
 /* 297 */ "xx_echo_statement ::= ECHO xx_echo_expressions DOTCOMMA",
 /* 298 */ "xx_echo_expressions ::= xx_echo_expressions COMMA xx_echo_expression",
 /* 299 */ "xx_echo_expressions ::= xx_echo_expression",
 /* 300 */ "xx_echo_expression ::= xx_common_expr",
 /* 301 */ "xx_mcall_statement ::= xx_mcall_expr DOTCOMMA",
 /* 302 */ "xx_fcall_statement ::= xx_fcall_expr DOTCOMMA",
 /* 303 */ "xx_scall_statement ::= xx_scall_expr DOTCOMMA",
 /* 304 */ "xx_fetch_statement ::= xx_fetch_expr DOTCOMMA",
 /* 305 */ "xx_return_statement ::= RETURN xx_common_expr DOTCOMMA",
 /* 306 */ "xx_return_statement ::= RETURN DOTCOMMA",
 /* 307 */ "xx_require_statement ::= REQUIRE xx_common_expr DOTCOMMA",
 /* 308 */ "xx_unset_statement ::= UNSET xx_common_expr DOTCOMMA",
 /* 309 */ "xx_throw_statement ::= THROW xx_common_expr DOTCOMMA",
 /* 310 */ "xx_declare_statement ::= TYPE_INTEGER xx_declare_variable_list DOTCOMMA",
 /* 311 */ "xx_declare_statement ::= TYPE_UINTEGER xx_declare_variable_list DOTCOMMA",
 /* 312 */ "xx_declare_statement ::= TYPE_CHAR xx_declare_variable_list DOTCOMMA",
 /* 313 */ "xx_declare_statement ::= TYPE_UCHAR xx_declare_variable_list DOTCOMMA",
 /* 314 */ "xx_declare_statement ::= TYPE_LONG xx_declare_variable_list DOTCOMMA",
 /* 315 */ "xx_declare_statement ::= TYPE_ULONG xx_declare_variable_list DOTCOMMA",
 /* 316 */ "xx_declare_statement ::= TYPE_DOUBLE xx_declare_variable_list DOTCOMMA",
 /* 317 */ "xx_declare_statement ::= TYPE_STRING xx_declare_variable_list DOTCOMMA",
 /* 318 */ "xx_declare_statement ::= TYPE_BOOL xx_declare_variable_list DOTCOMMA",
 /* 319 */ "xx_declare_statement ::= TYPE_VAR xx_declare_variable_list DOTCOMMA",
 /* 320 */ "xx_declare_statement ::= TYPE_ARRAY xx_declare_variable_list DOTCOMMA",
 /* 321 */ "xx_declare_variable_list ::= xx_declare_variable_list COMMA xx_declare_variable",
 /* 322 */ "xx_declare_variable_list ::= xx_declare_variable",
 /* 323 */ "xx_declare_variable ::= IDENTIFIER",
 /* 324 */ "xx_declare_variable ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 325 */ "xx_assign_expr ::= xx_common_expr",
 /* 326 */ "xx_common_expr ::= NOT xx_common_expr",
 /* 327 */ "xx_common_expr ::= SUB xx_common_expr",
 /* 328 */ "xx_common_expr ::= ISSET xx_common_expr",
 /* 329 */ "xx_common_expr ::= REQUIRE xx_common_expr",
 /* 330 */ "xx_common_expr ::= CLONE xx_common_expr",
 /* 331 */ "xx_common_expr ::= EMPTY xx_common_expr",
 /* 332 */ "xx_common_expr ::= LIKELY xx_common_expr",
 /* 333 */ "xx_common_expr ::= UNLIKELY xx_common_expr",
 /* 334 */ "xx_common_expr ::= xx_common_expr EQUALS xx_common_expr",
 /* 335 */ "xx_common_expr ::= xx_common_expr NOTEQUALS xx_common_expr",
 /* 336 */ "xx_common_expr ::= xx_common_expr IDENTICAL xx_common_expr",
 /* 337 */ "xx_common_expr ::= xx_common_expr NOTIDENTICAL xx_common_expr",
 /* 338 */ "xx_common_expr ::= xx_common_expr LESS xx_common_expr",
 /* 339 */ "xx_common_expr ::= xx_common_expr GREATER xx_common_expr",
 /* 340 */ "xx_common_expr ::= xx_common_expr LESSEQUAL xx_common_expr",
 /* 341 */ "xx_common_expr ::= xx_common_expr GREATEREQUAL xx_common_expr",
 /* 342 */ "xx_common_expr ::= PARENTHESES_OPEN xx_common_expr PARENTHESES_CLOSE",
 /* 343 */ "xx_common_expr ::= PARENTHESES_OPEN xx_parameter_type PARENTHESES_CLOSE xx_common_expr",
 /* 344 */ "xx_common_expr ::= LESS IDENTIFIER GREATER xx_common_expr",
 /* 345 */ "xx_common_expr ::= xx_common_expr ARROW IDENTIFIER",
 /* 346 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 347 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE",
 /* 348 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER",
 /* 349 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 350 */ "xx_common_expr ::= xx_common_expr SBRACKET_OPEN xx_common_expr SBRACKET_CLOSE",
 /* 351 */ "xx_common_expr ::= xx_common_expr ADD xx_common_expr",
 /* 352 */ "xx_common_expr ::= xx_common_expr SUB xx_common_expr",
 /* 353 */ "xx_common_expr ::= xx_common_expr MUL xx_common_expr",
 /* 354 */ "xx_common_expr ::= xx_common_expr DIV xx_common_expr",
 /* 355 */ "xx_common_expr ::= xx_common_expr MOD xx_common_expr",
 /* 356 */ "xx_common_expr ::= xx_common_expr CONCAT xx_common_expr",
 /* 357 */ "xx_common_expr ::= xx_common_expr AND xx_common_expr",
 /* 358 */ "xx_common_expr ::= xx_common_expr OR xx_common_expr",
 /* 359 */ "xx_common_expr ::= xx_common_expr BITWISE_AND xx_common_expr",
 /* 360 */ "xx_common_expr ::= xx_common_expr BITWISE_OR xx_common_expr",
 /* 361 */ "xx_common_expr ::= xx_common_expr BITWISE_XOR xx_common_expr",
 /* 362 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTLEFT xx_common_expr",
 /* 363 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTRIGHT xx_common_expr",
 /* 364 */ "xx_common_expr ::= xx_common_expr INSTANCEOF xx_common_expr",
 /* 365 */ "xx_fetch_expr ::= FETCH IDENTIFIER COMMA xx_common_expr",
 /* 366 */ "xx_common_expr ::= xx_fetch_expr",
 /* 367 */ "xx_common_expr ::= TYPEOF xx_common_expr",
 /* 368 */ "xx_common_expr ::= IDENTIFIER",
 /* 369 */ "xx_common_expr ::= INTEGER",
 /* 370 */ "xx_common_expr ::= STRING",
 /* 371 */ "xx_common_expr ::= CHAR",
 /* 372 */ "xx_common_expr ::= DOUBLE",
 /* 373 */ "xx_common_expr ::= NULL",
 /* 374 */ "xx_common_expr ::= TRUE",
 /* 375 */ "xx_common_expr ::= FALSE",
 /* 376 */ "xx_common_expr ::= CONSTANT",
 /* 377 */ "xx_common_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 378 */ "xx_common_expr ::= SBRACKET_OPEN xx_array_list SBRACKET_CLOSE",
 /* 379 */ "xx_common_expr ::= NEW IDENTIFIER",
 /* 380 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 381 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 382 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 383 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 384 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 385 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 386 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 387 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 388 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 389 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 390 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 391 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 392 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 393 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 394 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 395 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 396 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 397 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 398 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 399 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 400 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 401 */ "xx_common_expr ::= xx_mcall_expr",
 /* 402 */ "xx_common_expr ::= xx_scall_expr",
 /* 403 */ "xx_common_expr ::= xx_fcall_expr",
 /* 404 */ "xx_common_expr ::= xx_common_expr QUESTION xx_common_expr COLON xx_common_expr",
 /* 405 */ "xx_call_parameters ::= xx_call_parameters COMMA xx_call_parameter",
 /* 406 */ "xx_call_parameters ::= xx_call_parameter",
 /* 407 */ "xx_call_parameter ::= xx_common_expr",
 /* 408 */ "xx_call_parameter ::= IDENTIFIER COLON xx_common_expr",
 /* 409 */ "xx_call_parameter ::= BITWISE_AND xx_common_expr",
 /* 410 */ "xx_call_parameter ::= IDENTIFIER COLON BITWISE_AND xx_common_expr",
 /* 411 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 412 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 413 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 414 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 415 */ "xx_array_list ::= xx_array_list COMMA xx_array_item",
 /* 416 */ "xx_array_list ::= xx_array_item",
 /* 417 */ "xx_array_item ::= xx_array_key COLON xx_array_value",
 /* 418 */ "xx_array_item ::= xx_array_value",
 /* 419 */ "xx_array_key ::= CONSTANT",
 /* 420 */ "xx_array_key ::= IDENTIFIER",
 /* 421 */ "xx_array_key ::= STRING",
 /* 422 */ "xx_array_key ::= INTEGER",
 /* 423 */ "xx_array_value ::= xx_common_expr",
 /* 424 */ "xx_literal_expr ::= INTEGER",
 /* 425 */ "xx_literal_expr ::= CHAR",
 /* 426 */ "xx_literal_expr ::= STRING",
 /* 427 */ "xx_literal_expr ::= DOUBLE",
 /* 428 */ "xx_literal_expr ::= NULL",
 /* 429 */ "xx_literal_expr ::= FALSE",
 /* 430 */ "xx_literal_expr ::= TRUE",
 /* 431 */ "xx_literal_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 432 */ "xx_literal_expr ::= CONSTANT",
 /* 433 */ "xx_literal_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 434 */ "xx_literal_expr ::= SBRACKET_OPEN xx_literal_array_list SBRACKET_CLOSE",
 /* 435 */ "xx_literal_array_list ::= xx_literal_array_list COMMA xx_literal_array_item",
 /* 436 */ "xx_literal_array_list ::= xx_literal_array_item",
 /* 437 */ "xx_literal_array_item ::= xx_literal_array_key COLON xx_literal_array_value",
 /* 438 */ "xx_literal_array_item ::= xx_literal_array_value",
 /* 439 */ "xx_literal_array_key ::= IDENTIFIER",
 /* 440 */ "xx_literal_array_key ::= STRING",
 /* 441 */ "xx_literal_array_key ::= INTEGER",
 /* 442 */ "xx_literal_array_value ::= xx_literal_expr",
 /* 443 */ "xx_eval_expr ::= xx_common_expr",
 /* 444 */ "xx_comment ::= COMMENT",
 /* 445 */ "xx_cblock ::= CBLOCK",
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
// 4740 "parser.cpp"
      break;
    case 122:
// 1384 "parser.lemon"
{ delete (yypminor->yy396); }
// 4745 "parser.cpp"
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
  { 171, 3 },
  { 171, 1 },
  { 180, 1 },
  { 180, 2 },
  { 180, 2 },
  { 180, 3 },
  { 180, 3 },
  { 180, 4 },
  { 180, 2 },
  { 180, 3 },
  { 180, 3 },
  { 180, 4 },
  { 180, 4 },
  { 180, 5 },
  { 180, 5 },
  { 180, 6 },
  { 180, 4 },
  { 180, 5 },
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
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 181, 1 },
  { 183, 1 },
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
  { 185, 2 },
  { 185, 1 },
  { 186, 4 },
  { 186, 5 },
  { 149, 4 },
  { 149, 5 },
  { 187, 2 },
  { 187, 1 },
  { 188, 3 },
  { 188, 4 },
  { 188, 3 },
  { 134, 3 },
  { 134, 4 },
  { 146, 4 },
  { 146, 5 },
  { 147, 6 },
  { 147, 7 },
  { 148, 3 },
  { 148, 4 },
  { 148, 5 },
  { 189, 2 },
  { 189, 1 },
  { 190, 5 },
  { 190, 4 },
  { 190, 6 },
  { 190, 7 },
  { 191, 3 },
  { 191, 1 },
  { 192, 1 },
  { 150, 7 },
  { 150, 6 },
  { 150, 8 },
  { 150, 9 },
  { 150, 8 },
  { 150, 10 },
  { 132, 3 },
  { 194, 3 },
  { 194, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 196, 1 },
  { 195, 3 },
  { 195, 5 },
  { 195, 7 },
  { 195, 7 },
  { 195, 7 },
  { 195, 6 },
  { 195, 8 },
  { 195, 5 },
  { 195, 7 },
  { 195, 6 },
  { 195, 8 },
  { 195, 5 },
  { 195, 4 },
  { 195, 6 },
  { 198, 2 },
  { 198, 1 },
  { 199, 3 },
  { 195, 4 },
  { 195, 4 },
  { 195, 2 },
  { 195, 2 },
  { 195, 2 },
  { 195, 2 },
  { 195, 5 },
  { 195, 5 },
  { 200, 1 },
  { 135, 3 },
  { 201, 3 },
  { 201, 1 },
  { 202, 1 },
  { 182, 2 },
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
  { 207, 3 },
  { 207, 1 },
  { 208, 1 },
  { 208, 3 },
  { 197, 1 },
  { 193, 2 },
  { 193, 2 },
  { 193, 2 },
  { 193, 2 },
  { 193, 2 },
  { 193, 2 },
  { 193, 2 },
  { 193, 2 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 4 },
  { 193, 4 },
  { 193, 3 },
  { 193, 5 },
  { 193, 5 },
  { 193, 3 },
  { 193, 3 },
  { 193, 4 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 193, 3 },
  { 206, 4 },
  { 193, 1 },
  { 193, 2 },
  { 193, 1 },
  { 193, 1 },
  { 193, 1 },
  { 193, 1 },
  { 193, 1 },
  { 193, 1 },
  { 193, 1 },
  { 193, 1 },
  { 193, 1 },
  { 193, 2 },
  { 193, 3 },
  { 193, 2 },
  { 193, 4 },
  { 193, 5 },
  { 193, 4 },
  { 193, 6 },
  { 193, 7 },
  { 204, 4 },
  { 204, 3 },
  { 204, 6 },
  { 204, 5 },
  { 205, 6 },
  { 205, 5 },
  { 205, 8 },
  { 205, 7 },
  { 205, 10 },
  { 205, 9 },
  { 203, 6 },
  { 203, 5 },
  { 203, 8 },
  { 203, 7 },
  { 203, 8 },
  { 203, 7 },
  { 193, 1 },
  { 193, 1 },
  { 193, 1 },
  { 193, 5 },
  { 210, 3 },
  { 210, 1 },
  { 211, 1 },
  { 211, 3 },
  { 211, 2 },
  { 211, 4 },
  { 193, 5 },
  { 193, 6 },
  { 193, 6 },
  { 193, 7 },
  { 209, 3 },
  { 209, 1 },
  { 212, 3 },
  { 212, 1 },
  { 213, 1 },
  { 213, 1 },
  { 213, 1 },
  { 213, 1 },
  { 214, 1 },
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
  { 215, 3 },
  { 215, 1 },
  { 216, 3 },
  { 216, 1 },
  { 217, 1 },
  { 217, 1 },
  { 217, 1 },
  { 218, 1 },
  { 184, 1 },
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
	status->ret = yymsp[0].minor.yy396;
}
// 5408 "parser.cpp"
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
      case 189:
      case 190:
      case 191:
      case 192:
      case 193:
      case 194:
      case 195:
      case 196:
      case 197:
      case 198:
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
      case 296:
      case 300:
      case 325:
      case 366:
      case 401:
      case 402:
      case 403:
      case 423:
      case 442:
      case 443:
// 1386 "parser.lemon"
{
	yygotominor.yy396 = yymsp[0].minor.yy396;
}
// 5474 "parser.cpp"
        break;
      case 2:
      case 68:
      case 84:
      case 86:
      case 88:
      case 126:
      case 187:
      case 226:
      case 232:
      case 246:
      case 285:
// 1390 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(yymsp[-1].minor.yy396, yymsp[0].minor.yy396);
}
// 5491 "parser.cpp"
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
      case 154:
      case 188:
      case 227:
      case 233:
      case 247:
      case 253:
      case 263:
      case 286:
      case 299:
      case 322:
      case 406:
      case 416:
      case 436:
// 1394 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(NULL, yymsp[0].minor.yy396);
}
// 5520 "parser.cpp"
        break;
      case 30:
// 1502 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_namespace(yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(43,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5529 "parser.cpp"
        break;
      case 31:
// 1506 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_use_aliases(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(46,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5538 "parser.cpp"
        break;
      case 32:
      case 51:
      case 80:
      case 153:
      case 262:
      case 298:
      case 321:
      case 405:
      case 415:
      case 435:
// 1510 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(yymsp[-2].minor.yy396, yymsp[0].minor.yy396);
  yy_destructor(6,&yymsp[-1].minor);
}
// 5555 "parser.cpp"
        break;
      case 34:
// 1518 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_use_aliases_item(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 5562 "parser.cpp"
        break;
      case 35:
// 1522 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_use_aliases_item(yymsp[-2].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
  yy_destructor(47,&yymsp[-1].minor);
}
// 5570 "parser.cpp"
        break;
      case 36:
// 1526 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_interface(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(48,&yymsp[-2].minor);
}
// 5578 "parser.cpp"
        break;
      case 37:
// 1530 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_interface(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(48,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5587 "parser.cpp"
        break;
      case 38:
// 1534 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, 0, 0, NULL, NULL, status->scanner_state);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5595 "parser.cpp"
        break;
      case 39:
// 1538 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5604 "parser.cpp"
        break;
      case 40:
// 1542 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 0, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5613 "parser.cpp"
        break;
      case 41:
// 1546 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy396, 0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(50,&yymsp[-6].minor);
  yy_destructor(49,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5623 "parser.cpp"
        break;
      case 42:
// 1550 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, 1, 0, NULL, NULL, status->scanner_state);
  yy_destructor(52,&yymsp[-3].minor);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5632 "parser.cpp"
        break;
      case 43:
// 1554 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 1, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(52,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5642 "parser.cpp"
        break;
      case 44:
// 1558 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 1, 0, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(52,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5652 "parser.cpp"
        break;
      case 45:
// 1562 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy396, 1, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(52,&yymsp[-7].minor);
  yy_destructor(50,&yymsp[-6].minor);
  yy_destructor(49,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5663 "parser.cpp"
        break;
      case 46:
// 1566 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, 0, 1, NULL, NULL, status->scanner_state);
  yy_destructor(53,&yymsp[-3].minor);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5672 "parser.cpp"
        break;
      case 47:
// 1570 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(53,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5682 "parser.cpp"
        break;
      case 48:
// 1574 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 1, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(53,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5692 "parser.cpp"
        break;
      case 49:
      case 78:
// 1578 "parser.lemon"
{
	yygotominor.yy396 = NULL;
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5702 "parser.cpp"
        break;
      case 50:
      case 79:
// 1582 "parser.lemon"
{
	yygotominor.yy396 = yymsp[-1].minor.yy396;
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5712 "parser.cpp"
        break;
      case 53:
      case 254:
      case 368:
      case 420:
      case 439:
// 1594 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state);
}
// 5723 "parser.cpp"
        break;
      case 54:
// 1598 "parser.lemon"
{
  yygotominor.yy396 = NULL;
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5732 "parser.cpp"
        break;
      case 55:
// 1602 "parser.lemon"
{
  yygotominor.yy396 = yymsp[-1].minor.yy396;
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5741 "parser.cpp"
        break;
      case 56:
// 1606 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
}
// 5748 "parser.cpp"
        break;
      case 57:
// 1610 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 5755 "parser.cpp"
        break;
      case 58:
// 1614 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(NULL, yymsp[0].minor.yy396, NULL, status->scanner_state);
}
// 5762 "parser.cpp"
        break;
      case 59:
// 1618 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-1].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
}
// 5769 "parser.cpp"
        break;
      case 60:
// 1622 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-1].minor.yy396, NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 5776 "parser.cpp"
        break;
      case 61:
// 1626 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[0].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
}
// 5783 "parser.cpp"
        break;
      case 62:
// 1630 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(NULL, yymsp[0].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
}
// 5790 "parser.cpp"
        break;
      case 63:
// 1634 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
}
// 5797 "parser.cpp"
        break;
      case 64:
// 1638 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-1].minor.yy396, yymsp[0].minor.yy396, yymsp[-2].minor.yy396, status->scanner_state);
}
// 5804 "parser.cpp"
        break;
      case 65:
// 1642 "parser.lemon"
{
  yygotominor.yy396 = xx_ret_interface_definition(NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 5811 "parser.cpp"
        break;
      case 66:
// 1646 "parser.lemon"
{
  yygotominor.yy396 = xx_ret_interface_definition(yymsp[0].minor.yy396, NULL, status->scanner_state);
}
// 5818 "parser.cpp"
        break;
      case 67:
// 1650 "parser.lemon"
{
  yygotominor.yy396 = xx_ret_interface_definition(yymsp[0].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
}
// 5825 "parser.cpp"
        break;
      case 70:
// 1663 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-2].minor.yy396, yymsp[-1].minor.yy0, NULL, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5833 "parser.cpp"
        break;
      case 71:
// 1667 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-2].minor.yy396, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5841 "parser.cpp"
        break;
      case 72:
// 1671 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-4].minor.yy396, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, NULL, status->scanner_state);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5850 "parser.cpp"
        break;
      case 73:
// 1675 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-4].minor.yy396, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5859 "parser.cpp"
        break;
      case 74:
// 1679 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, NULL, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5867 "parser.cpp"
        break;
      case 75:
// 1683 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5875 "parser.cpp"
        break;
      case 76:
// 1687 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-5].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, yymsp[-6].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(57,&yymsp[-3].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5884 "parser.cpp"
        break;
      case 77:
// 1691 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-5].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(57,&yymsp[-3].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5893 "parser.cpp"
        break;
      case 82:
// 1711 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_property_shortcut(NULL, yymsp[0].minor.yy0, status->scanner_state);
}
// 5900 "parser.cpp"
        break;
      case 83:
// 1715 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_property_shortcut(yymsp[-1].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
}
// 5907 "parser.cpp"
        break;
      case 90:
      case 92:
// 1744 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5918 "parser.cpp"
        break;
      case 91:
      case 93:
// 1748 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5929 "parser.cpp"
        break;
      case 94:
// 1764 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-6].minor.yy396, yymsp[-4].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5941 "parser.cpp"
        break;
      case 95:
      case 122:
// 1769 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-5].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5953 "parser.cpp"
        break;
      case 96:
// 1774 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5965 "parser.cpp"
        break;
      case 97:
      case 123:
// 1779 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-6].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5977 "parser.cpp"
        break;
      case 98:
// 1784 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5989 "parser.cpp"
        break;
      case 99:
// 1788 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6001 "parser.cpp"
        break;
      case 100:
// 1792 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-6].minor.yy396, yymsp[-4].minor.yy0, NULL, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6013 "parser.cpp"
        break;
      case 101:
      case 124:
// 1796 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-5].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, yymsp[-6].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6025 "parser.cpp"
        break;
      case 102:
// 1800 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6037 "parser.cpp"
        break;
      case 103:
      case 125:
// 1804 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-6].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6049 "parser.cpp"
        break;
      case 104:
// 1808 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6061 "parser.cpp"
        break;
      case 105:
// 1812 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, yymsp[-9].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6073 "parser.cpp"
        break;
      case 106:
// 1816 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, NULL, NULL, NULL, yymsp[-2].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6086 "parser.cpp"
        break;
      case 107:
      case 118:
// 1820 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6099 "parser.cpp"
        break;
      case 108:
// 1824 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-9].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy396, NULL, NULL, yymsp[-2].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6112 "parser.cpp"
        break;
      case 109:
      case 119:
// 1828 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6125 "parser.cpp"
        break;
      case 110:
// 1832 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-9].minor.yy396, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy396, NULL, yymsp[-3].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6138 "parser.cpp"
        break;
      case 111:
// 1836 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-10].minor.yy396, yymsp[-8].minor.yy0, yymsp[-6].minor.yy396, yymsp[-1].minor.yy396, NULL, yymsp[-3].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-9].minor);
  yy_destructor(61,&yymsp[-7].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6151 "parser.cpp"
        break;
      case 112:
// 1840 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, NULL, NULL, yymsp[-9].minor.yy0, yymsp[-2].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6164 "parser.cpp"
        break;
      case 113:
      case 120:
// 1844 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, NULL, yymsp[-8].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6177 "parser.cpp"
        break;
      case 114:
// 1848 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-9].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy396, NULL, yymsp[-10].minor.yy0, yymsp[-2].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6190 "parser.cpp"
        break;
      case 115:
      case 121:
// 1852 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, NULL, yymsp[-9].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6203 "parser.cpp"
        break;
      case 116:
// 1856 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-9].minor.yy396, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy396, yymsp[-10].minor.yy0, yymsp[-3].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6216 "parser.cpp"
        break;
      case 117:
// 1860 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-10].minor.yy396, yymsp[-8].minor.yy0, yymsp[-6].minor.yy396, yymsp[-1].minor.yy396, yymsp[-11].minor.yy0, yymsp[-3].minor.yy396, status->scanner_state);
  yy_destructor(60,&yymsp[-9].minor);
  yy_destructor(61,&yymsp[-7].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6229 "parser.cpp"
        break;
      case 128:
// 1906 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("public");
  yy_destructor(1,&yymsp[0].minor);
}
// 6237 "parser.cpp"
        break;
      case 129:
// 1910 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("protected");
  yy_destructor(2,&yymsp[0].minor);
}
// 6245 "parser.cpp"
        break;
      case 130:
// 1914 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("private");
  yy_destructor(4,&yymsp[0].minor);
}
// 6253 "parser.cpp"
        break;
      case 131:
// 1918 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("static");
  yy_destructor(3,&yymsp[0].minor);
}
// 6261 "parser.cpp"
        break;
      case 132:
// 1922 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("scoped");
  yy_destructor(5,&yymsp[0].minor);
}
// 6269 "parser.cpp"
        break;
      case 133:
// 1926 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("inline");
  yy_destructor(62,&yymsp[0].minor);
}
// 6277 "parser.cpp"
        break;
      case 134:
// 1930 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("deprecated");
  yy_destructor(63,&yymsp[0].minor);
}
// 6285 "parser.cpp"
        break;
      case 135:
// 1934 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("abstract");
  yy_destructor(52,&yymsp[0].minor);
}
// 6293 "parser.cpp"
        break;
      case 136:
// 1938 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("final");
  yy_destructor(53,&yymsp[0].minor);
}
// 6301 "parser.cpp"
        break;
      case 137:
// 1943 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type(1, NULL, status->scanner_state);
  yy_destructor(64,&yymsp[0].minor);
}
// 6309 "parser.cpp"
        break;
      case 138:
// 1947 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type(0, yymsp[0].minor.yy396, status->scanner_state);
}
// 6316 "parser.cpp"
        break;
      case 139:
      case 252:
// 1951 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(yymsp[-2].minor.yy396, yymsp[0].minor.yy396);
  yy_destructor(14,&yymsp[-1].minor);
}
// 6325 "parser.cpp"
        break;
      case 141:
// 1959 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(yymsp[0].minor.yy396, NULL, 0, 0, status->scanner_state);
}
// 6332 "parser.cpp"
        break;
      case 142:
// 1963 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_NULL), NULL, 0, 0, status->scanner_state);
  yy_destructor(65,&yymsp[0].minor);
}
// 6340 "parser.cpp"
        break;
      case 143:
// 1967 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_THIS), NULL, 0, 0, status->scanner_state);
  yy_destructor(66,&yymsp[0].minor);
}
// 6348 "parser.cpp"
        break;
      case 144:
// 1971 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(yymsp[-1].minor.yy396, NULL, 1, 0, status->scanner_state);
  yy_destructor(39,&yymsp[0].minor);
}
// 6356 "parser.cpp"
        break;
      case 145:
// 1975 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy396, 0, 0, status->scanner_state);
}
// 6363 "parser.cpp"
        break;
      case 146:
// 1979 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy396, 0, 1, status->scanner_state);
}
// 6370 "parser.cpp"
        break;
      case 147:
// 1986 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(NULL, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6382 "parser.cpp"
        break;
      case 148:
// 1991 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(NULL, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6394 "parser.cpp"
        break;
      case 149:
// 1996 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(NULL, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6406 "parser.cpp"
        break;
      case 150:
// 2001 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6418 "parser.cpp"
        break;
      case 151:
// 2006 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6430 "parser.cpp"
        break;
      case 152:
// 2011 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6442 "parser.cpp"
        break;
      case 155:
// 2025 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6449 "parser.cpp"
        break;
      case 156:
// 2029 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-1].minor);
}
// 6457 "parser.cpp"
        break;
      case 157:
// 2033 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-1].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6464 "parser.cpp"
        break;
      case 158:
// 2037 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-1].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-2].minor);
}
// 6472 "parser.cpp"
        break;
      case 159:
// 2041 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-2].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(39,&yymsp[-1].minor);
}
// 6480 "parser.cpp"
        break;
      case 160:
// 2045 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-2].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(58,&yymsp[-3].minor);
  yy_destructor(39,&yymsp[-1].minor);
}
// 6489 "parser.cpp"
        break;
      case 161:
// 2049 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, yymsp[-1].minor.yy396, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6496 "parser.cpp"
        break;
      case 162:
// 2053 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, yymsp[-1].minor.yy396, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-2].minor);
}
// 6504 "parser.cpp"
        break;
      case 163:
// 2057 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6512 "parser.cpp"
        break;
      case 164:
// 2061 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6521 "parser.cpp"
        break;
      case 165:
// 2065 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-3].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6529 "parser.cpp"
        break;
      case 166:
// 2069 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-3].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6538 "parser.cpp"
        break;
      case 167:
// 2073 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-4].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 1, status->scanner_state);
  yy_destructor(39,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6547 "parser.cpp"
        break;
      case 168:
// 2077 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-4].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 1, status->scanner_state);
  yy_destructor(58,&yymsp[-5].minor);
  yy_destructor(39,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6557 "parser.cpp"
        break;
      case 169:
// 2081 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6565 "parser.cpp"
        break;
      case 170:
// 2085 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6574 "parser.cpp"
        break;
      case 171:
// 2090 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(21,&yymsp[-2].minor);
  yy_destructor(22,&yymsp[0].minor);
}
// 6583 "parser.cpp"
        break;
      case 172:
// 2094 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state);
  yy_destructor(21,&yymsp[-4].minor);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[-1].minor);
  yy_destructor(22,&yymsp[0].minor);
}
// 6594 "parser.cpp"
        break;
      case 173:
// 2098 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_INTEGER);
  yy_destructor(68,&yymsp[0].minor);
}
// 6602 "parser.cpp"
        break;
      case 174:
// 2102 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_UINTEGER);
  yy_destructor(69,&yymsp[0].minor);
}
// 6610 "parser.cpp"
        break;
      case 175:
// 2106 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_LONG);
  yy_destructor(70,&yymsp[0].minor);
}
// 6618 "parser.cpp"
        break;
      case 176:
// 2110 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_ULONG);
  yy_destructor(71,&yymsp[0].minor);
}
// 6626 "parser.cpp"
        break;
      case 177:
// 2114 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_CHAR);
  yy_destructor(72,&yymsp[0].minor);
}
// 6634 "parser.cpp"
        break;
      case 178:
// 2118 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_UCHAR);
  yy_destructor(73,&yymsp[0].minor);
}
// 6642 "parser.cpp"
        break;
      case 179:
// 2122 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_DOUBLE);
  yy_destructor(74,&yymsp[0].minor);
}
// 6650 "parser.cpp"
        break;
      case 180:
// 2126 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_BOOL);
  yy_destructor(75,&yymsp[0].minor);
}
// 6658 "parser.cpp"
        break;
      case 181:
// 2130 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_STRING);
  yy_destructor(76,&yymsp[0].minor);
}
// 6666 "parser.cpp"
        break;
      case 182:
// 2134 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_ARRAY);
  yy_destructor(77,&yymsp[0].minor);
}
// 6674 "parser.cpp"
        break;
      case 183:
// 2138 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_VAR);
  yy_destructor(78,&yymsp[0].minor);
}
// 6682 "parser.cpp"
        break;
      case 184:
// 2142 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_CALLABLE);
  yy_destructor(79,&yymsp[0].minor);
}
// 6690 "parser.cpp"
        break;
      case 185:
// 2146 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_RESOURCE);
  yy_destructor(80,&yymsp[0].minor);
}
// 6698 "parser.cpp"
        break;
      case 186:
// 2150 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_OBJECT);
  yy_destructor(81,&yymsp[0].minor);
}
// 6706 "parser.cpp"
        break;
      case 212:
// 2254 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_empty_statement(status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 6714 "parser.cpp"
        break;
      case 213:
// 2258 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_break_statement(status->scanner_state);
  yy_destructor(82,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6723 "parser.cpp"
        break;
      case 214:
// 2262 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_continue_statement(status->scanner_state);
  yy_destructor(83,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6732 "parser.cpp"
        break;
      case 215:
// 2267 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-2].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6742 "parser.cpp"
        break;
      case 216:
// 2272 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-3].minor.yy396, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(84,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6752 "parser.cpp"
        break;
      case 217:
// 2277 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-5].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6765 "parser.cpp"
        break;
      case 218:
// 2282 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-6].minor.yy396, NULL, yymsp[-3].minor.yy396, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6778 "parser.cpp"
        break;
      case 219:
// 2287 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6788 "parser.cpp"
        break;
      case 220:
// 2292 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-4].minor.yy396, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-3].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6798 "parser.cpp"
        break;
      case 221:
// 2297 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-7].minor.yy396, yymsp[-5].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(84,&yymsp[-8].minor);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6811 "parser.cpp"
        break;
      case 222:
// 2302 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-8].minor.yy396, yymsp[-6].minor.yy396, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(84,&yymsp[-9].minor);
  yy_destructor(54,&yymsp[-7].minor);
  yy_destructor(55,&yymsp[-5].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6824 "parser.cpp"
        break;
      case 223:
// 2307 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-6].minor.yy396, yymsp[-4].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6837 "parser.cpp"
        break;
      case 224:
// 2312 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-7].minor.yy396, yymsp[-5].minor.yy396, yymsp[-3].minor.yy396, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-8].minor);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6850 "parser.cpp"
        break;
      case 225:
// 2317 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-6].minor.yy396, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6863 "parser.cpp"
        break;
      case 228:
// 2330 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-2].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(86,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6873 "parser.cpp"
        break;
      case 229:
// 2335 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(86,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6883 "parser.cpp"
        break;
      case 230:
// 2339 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_switch_statement(yymsp[-2].minor.yy396, NULL, status->scanner_state);
  yy_destructor(87,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6893 "parser.cpp"
        break;
      case 231:
// 2343 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_switch_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(87,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6903 "parser.cpp"
        break;
      case 234:
// 2355 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_case_clause(yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(88,&yymsp[-2].minor);
  yy_destructor(89,&yymsp[0].minor);
}
// 6912 "parser.cpp"
        break;
      case 235:
// 2359 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_case_clause(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(88,&yymsp[-3].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 6921 "parser.cpp"
        break;
      case 236:
// 2363 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_case_clause(NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(90,&yymsp[-2].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 6930 "parser.cpp"
        break;
      case 237:
// 2367 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_loop_statement(NULL, status->scanner_state);
  yy_destructor(91,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6940 "parser.cpp"
        break;
      case 238:
// 2371 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_loop_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(91,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6950 "parser.cpp"
        break;
      case 239:
// 2375 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_while_statement(yymsp[-2].minor.yy396, NULL, status->scanner_state);
  yy_destructor(92,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6960 "parser.cpp"
        break;
      case 240:
// 2379 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_while_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(92,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6970 "parser.cpp"
        break;
      case 241:
// 2383 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_do_while_statement(yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(93,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(92,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6982 "parser.cpp"
        break;
      case 242:
// 2387 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_do_while_statement(yymsp[-1].minor.yy396, yymsp[-4].minor.yy396, status->scanner_state);
  yy_destructor(93,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(92,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6994 "parser.cpp"
        break;
      case 243:
// 2391 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_try_catch_statement(NULL, NULL, status->scanner_state);
  yy_destructor(94,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7004 "parser.cpp"
        break;
      case 244:
// 2395 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_try_catch_statement(yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(94,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7014 "parser.cpp"
        break;
      case 245:
// 2399 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_try_catch_statement(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(94,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-3].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 7024 "parser.cpp"
        break;
      case 248:
// 2411 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-3].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(95,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7034 "parser.cpp"
        break;
      case 249:
// 2415 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-2].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(95,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7044 "parser.cpp"
        break;
      case 250:
// 2419 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-4].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(95,&yymsp[-5].minor);
  yy_destructor(6,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7055 "parser.cpp"
        break;
      case 251:
// 2423 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-5].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state), yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(95,&yymsp[-6].minor);
  yy_destructor(6,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7066 "parser.cpp"
        break;
      case 255:
// 2439 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, NULL, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(96,&yymsp[-6].minor);
  yy_destructor(97,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7077 "parser.cpp"
        break;
      case 256:
// 2443 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-2].minor.yy396, NULL, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(96,&yymsp[-5].minor);
  yy_destructor(97,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7088 "parser.cpp"
        break;
      case 257:
// 2447 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, NULL, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(96,&yymsp[-7].minor);
  yy_destructor(97,&yymsp[-5].minor);
  yy_destructor(98,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7100 "parser.cpp"
        break;
      case 258:
// 2451 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(96,&yymsp[-8].minor);
  yy_destructor(6,&yymsp[-6].minor);
  yy_destructor(97,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7112 "parser.cpp"
        break;
      case 259:
// 2455 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-2].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(96,&yymsp[-7].minor);
  yy_destructor(6,&yymsp[-5].minor);
  yy_destructor(97,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7124 "parser.cpp"
        break;
      case 260:
// 2459 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, yymsp[-8].minor.yy0, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(96,&yymsp[-9].minor);
  yy_destructor(6,&yymsp[-7].minor);
  yy_destructor(97,&yymsp[-5].minor);
  yy_destructor(98,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7137 "parser.cpp"
        break;
      case 261:
// 2463 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(99,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7146 "parser.cpp"
        break;
      case 264:
// 2476 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("assign");
  yy_destructor(57,&yymsp[0].minor);
}
// 7154 "parser.cpp"
        break;
      case 265:
// 2481 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("add-assign");
  yy_destructor(100,&yymsp[0].minor);
}
// 7162 "parser.cpp"
        break;
      case 266:
// 2486 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("sub-assign");
  yy_destructor(101,&yymsp[0].minor);
}
// 7170 "parser.cpp"
        break;
      case 267:
// 2490 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("mul-assign");
  yy_destructor(102,&yymsp[0].minor);
}
// 7178 "parser.cpp"
        break;
      case 268:
// 2494 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("div-assign");
  yy_destructor(103,&yymsp[0].minor);
}
// 7186 "parser.cpp"
        break;
      case 269:
// 2498 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("concat-assign");
  yy_destructor(104,&yymsp[0].minor);
}
// 7194 "parser.cpp"
        break;
      case 270:
// 2502 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("mod-assign");
  yy_destructor(105,&yymsp[0].minor);
}
// 7202 "parser.cpp"
        break;
      case 271:
// 2507 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("variable", yymsp[-1].minor.yy396, yymsp[-2].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 7209 "parser.cpp"
        break;
      case 272:
// 2512 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property", yymsp[-1].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
}
// 7217 "parser.cpp"
        break;
      case 273:
// 2517 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("variable-dynamic-object-property", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7227 "parser.cpp"
        break;
      case 274:
// 2522 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("string-dynamic-object-property", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7237 "parser.cpp"
        break;
      case 275:
// 2527 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-append", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7247 "parser.cpp"
        break;
      case 276:
// 2532 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-array-index", yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(42,&yymsp[-4].minor);
}
// 7255 "parser.cpp"
        break;
      case 277:
// 2536 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-array-index-append", yymsp[-1].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7265 "parser.cpp"
        break;
      case 278:
// 2541 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property", yymsp[-1].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(107,&yymsp[-3].minor);
}
// 7273 "parser.cpp"
        break;
      case 279:
// 2546 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property-append", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(107,&yymsp[-5].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7283 "parser.cpp"
        break;
      case 280:
// 2551 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property-array-index", yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(107,&yymsp[-4].minor);
}
// 7291 "parser.cpp"
        break;
      case 281:
// 2556 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property-array-index-append", yymsp[-1].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(107,&yymsp[-6].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7301 "parser.cpp"
        break;
      case 282:
// 2561 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("variable-append", yymsp[-1].minor.yy396, yymsp[-4].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7310 "parser.cpp"
        break;
      case 283:
// 2566 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("array-index", yymsp[-1].minor.yy396, yymsp[-3].minor.yy0, NULL, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
}
// 7317 "parser.cpp"
        break;
      case 284:
// 2571 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("array-index-append", yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-4].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7326 "parser.cpp"
        break;
      case 287:
// 2583 "parser.lemon"
{
	yygotominor.yy396 = yymsp[-1].minor.yy396;
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7335 "parser.cpp"
        break;
      case 288:
// 2588 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-incr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(108,&yymsp[0].minor);
}
// 7344 "parser.cpp"
        break;
      case 289:
// 2593 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-decr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(109,&yymsp[0].minor);
}
// 7353 "parser.cpp"
        break;
      case 290:
// 2598 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("incr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(108,&yymsp[0].minor);
}
// 7361 "parser.cpp"
        break;
      case 291:
// 2603 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("incr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(108,&yymsp[-1].minor);
}
// 7369 "parser.cpp"
        break;
      case 292:
// 2608 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("decr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(109,&yymsp[0].minor);
}
// 7377 "parser.cpp"
        break;
      case 293:
// 2613 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("decr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(109,&yymsp[-1].minor);
}
// 7385 "parser.cpp"
        break;
      case 294:
// 2618 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("dynamic-variable", yymsp[-1].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7394 "parser.cpp"
        break;
      case 295:
// 2623 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("dynamic-variable-string", yymsp[-1].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7403 "parser.cpp"
        break;
      case 297:
// 2631 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_echo_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(110,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7412 "parser.cpp"
        break;
      case 301:
// 2648 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7420 "parser.cpp"
        break;
      case 302:
// 2653 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7428 "parser.cpp"
        break;
      case 303:
// 2658 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7436 "parser.cpp"
        break;
      case 304:
// 2663 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fetch_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7444 "parser.cpp"
        break;
      case 305:
// 2668 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(111,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7453 "parser.cpp"
        break;
      case 306:
// 2673 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_statement(NULL, status->scanner_state);
  yy_destructor(111,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7462 "parser.cpp"
        break;
      case 307:
// 2678 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_require_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(7,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7471 "parser.cpp"
        break;
      case 308:
// 2683 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_unset_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(112,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7480 "parser.cpp"
        break;
      case 309:
// 2688 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_throw_exception(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(113,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7489 "parser.cpp"
        break;
      case 310:
// 2692 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_INTEGER, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(68,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7498 "parser.cpp"
        break;
      case 311:
// 2696 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_UINTEGER, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(69,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7507 "parser.cpp"
        break;
      case 312:
// 2700 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_CHAR, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(72,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7516 "parser.cpp"
        break;
      case 313:
// 2704 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_UCHAR, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(73,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7525 "parser.cpp"
        break;
      case 314:
// 2708 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_LONG, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(70,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7534 "parser.cpp"
        break;
      case 315:
// 2712 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_ULONG, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(71,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7543 "parser.cpp"
        break;
      case 316:
// 2716 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_DOUBLE, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(74,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7552 "parser.cpp"
        break;
      case 317:
// 2720 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_STRING, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(76,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7561 "parser.cpp"
        break;
      case 318:
// 2724 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_BOOL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(75,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7570 "parser.cpp"
        break;
      case 319:
// 2728 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_VAR, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(78,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7579 "parser.cpp"
        break;
      case 320:
// 2732 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_ARRAY, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(77,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7588 "parser.cpp"
        break;
      case 323:
// 2744 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_variable(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 7595 "parser.cpp"
        break;
      case 324:
// 2748 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_variable(yymsp[-2].minor.yy0, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 7603 "parser.cpp"
        break;
      case 326:
// 2756 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("not", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(39,&yymsp[-1].minor);
}
// 7611 "parser.cpp"
        break;
      case 327:
// 2760 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("minus", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(28,&yymsp[-1].minor);
}
// 7619 "parser.cpp"
        break;
      case 328:
// 2764 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("isset", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(33,&yymsp[-1].minor);
}
// 7627 "parser.cpp"
        break;
      case 329:
// 2768 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("require", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(7,&yymsp[-1].minor);
}
// 7635 "parser.cpp"
        break;
      case 330:
// 2772 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("clone", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(37,&yymsp[-1].minor);
}
// 7643 "parser.cpp"
        break;
      case 331:
// 2776 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("empty", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(35,&yymsp[-1].minor);
}
// 7651 "parser.cpp"
        break;
      case 332:
// 2780 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("likely", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(9,&yymsp[-1].minor);
}
// 7659 "parser.cpp"
        break;
      case 333:
// 2784 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("unlikely", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(10,&yymsp[-1].minor);
}
// 7667 "parser.cpp"
        break;
      case 334:
// 2788 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("equals", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(19,&yymsp[-1].minor);
}
// 7675 "parser.cpp"
        break;
      case 335:
// 2792 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("not-equals", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(26,&yymsp[-1].minor);
}
// 7683 "parser.cpp"
        break;
      case 336:
// 2796 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("identical", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(20,&yymsp[-1].minor);
}
// 7691 "parser.cpp"
        break;
      case 337:
// 2800 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("not-identical", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(25,&yymsp[-1].minor);
}
// 7699 "parser.cpp"
        break;
      case 338:
// 2804 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("less", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(21,&yymsp[-1].minor);
}
// 7707 "parser.cpp"
        break;
      case 339:
// 2808 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("greater", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(22,&yymsp[-1].minor);
}
// 7715 "parser.cpp"
        break;
      case 340:
// 2812 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("less-equal", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(23,&yymsp[-1].minor);
}
// 7723 "parser.cpp"
        break;
      case 341:
// 2816 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("greater-equal", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(24,&yymsp[-1].minor);
}
// 7731 "parser.cpp"
        break;
      case 342:
// 2820 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("list", yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7740 "parser.cpp"
        break;
      case 343:
// 2824 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("cast", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
}
// 7749 "parser.cpp"
        break;
      case 344:
// 2828 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("type-hint", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(21,&yymsp[-3].minor);
  yy_destructor(22,&yymsp[-1].minor);
}
// 7758 "parser.cpp"
        break;
      case 345:
// 2832 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("property-access", yymsp[-2].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-1].minor);
}
// 7766 "parser.cpp"
        break;
      case 346:
// 2836 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("property-dynamic-access", yymsp[-4].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7776 "parser.cpp"
        break;
      case 347:
// 2840 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("property-string-access", yymsp[-4].minor.yy396, xx_ret_literal(XX_T_STRING, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7786 "parser.cpp"
        break;
      case 348:
// 2844 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("static-property-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-1].minor);
}
// 7794 "parser.cpp"
        break;
      case 349:
      case 431:
// 2848 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("static-constant-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-1].minor);
}
// 7803 "parser.cpp"
        break;
      case 350:
// 2857 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("array-access", yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7812 "parser.cpp"
        break;
      case 351:
// 2862 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("add", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(27,&yymsp[-1].minor);
}
// 7820 "parser.cpp"
        break;
      case 352:
// 2867 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("sub", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(28,&yymsp[-1].minor);
}
// 7828 "parser.cpp"
        break;
      case 353:
// 2872 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("mul", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(30,&yymsp[-1].minor);
}
// 7836 "parser.cpp"
        break;
      case 354:
// 2877 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("div", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(31,&yymsp[-1].minor);
}
// 7844 "parser.cpp"
        break;
      case 355:
// 2882 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("mod", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(32,&yymsp[-1].minor);
}
// 7852 "parser.cpp"
        break;
      case 356:
// 2887 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("concat", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(29,&yymsp[-1].minor);
}
// 7860 "parser.cpp"
        break;
      case 357:
// 2892 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("and", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(13,&yymsp[-1].minor);
}
// 7868 "parser.cpp"
        break;
      case 358:
// 2897 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("or", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(12,&yymsp[-1].minor);
}
// 7876 "parser.cpp"
        break;
      case 359:
// 2902 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_and", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(15,&yymsp[-1].minor);
}
// 7884 "parser.cpp"
        break;
      case 360:
// 2907 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_or", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(14,&yymsp[-1].minor);
}
// 7892 "parser.cpp"
        break;
      case 361:
// 2912 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_xor", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(16,&yymsp[-1].minor);
}
// 7900 "parser.cpp"
        break;
      case 362:
// 2917 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_shiftleft", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(17,&yymsp[-1].minor);
}
// 7908 "parser.cpp"
        break;
      case 363:
// 2922 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_shiftright", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(18,&yymsp[-1].minor);
}
// 7916 "parser.cpp"
        break;
      case 364:
// 2927 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("instanceof", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(11,&yymsp[-1].minor);
}
// 7924 "parser.cpp"
        break;
      case 365:
// 2932 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("fetch", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(34,&yymsp[-3].minor);
  yy_destructor(6,&yymsp[-1].minor);
}
// 7933 "parser.cpp"
        break;
      case 367:
// 2942 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("typeof", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(36,&yymsp[-1].minor);
}
// 7941 "parser.cpp"
        break;
      case 369:
      case 422:
      case 424:
      case 441:
// 2952 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_INTEGER, yymsp[0].minor.yy0, status->scanner_state);
}
// 7951 "parser.cpp"
        break;
      case 370:
      case 421:
      case 426:
      case 440:
// 2957 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_STRING, yymsp[0].minor.yy0, status->scanner_state);
}
// 7961 "parser.cpp"
        break;
      case 371:
      case 425:
// 2962 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_CHAR, yymsp[0].minor.yy0, status->scanner_state);
}
// 7969 "parser.cpp"
        break;
      case 372:
      case 427:
// 2967 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_DOUBLE, yymsp[0].minor.yy0, status->scanner_state);
}
// 7977 "parser.cpp"
        break;
      case 373:
      case 428:
// 2972 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_NULL, NULL, status->scanner_state);
  yy_destructor(65,&yymsp[0].minor);
}
// 7986 "parser.cpp"
        break;
      case 374:
      case 430:
// 2977 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_TRUE, NULL, status->scanner_state);
  yy_destructor(117,&yymsp[0].minor);
}
// 7995 "parser.cpp"
        break;
      case 375:
      case 429:
// 2982 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_FALSE, NULL, status->scanner_state);
  yy_destructor(118,&yymsp[0].minor);
}
// 8004 "parser.cpp"
        break;
      case 376:
      case 419:
      case 432:
// 2987 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_CONSTANT, yymsp[0].minor.yy0, status->scanner_state);
}
// 8013 "parser.cpp"
        break;
      case 377:
      case 433:
// 2992 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("empty-array", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-1].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 8023 "parser.cpp"
        break;
      case 378:
      case 434:
// 2997 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("array", yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 8033 "parser.cpp"
        break;
      case 379:
// 3002 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(0, yymsp[0].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-1].minor);
}
// 8041 "parser.cpp"
        break;
      case 380:
// 3007 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8051 "parser.cpp"
        break;
      case 381:
// 3012 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(38,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8061 "parser.cpp"
        break;
      case 382:
// 3017 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8071 "parser.cpp"
        break;
      case 383:
// 3022 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8083 "parser.cpp"
        break;
      case 384:
// 3027 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(38,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8095 "parser.cpp"
        break;
      case 385:
// 3032 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(1, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8104 "parser.cpp"
        break;
      case 386:
// 3037 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(1, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8113 "parser.cpp"
        break;
      case 387:
// 3042 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(2, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8124 "parser.cpp"
        break;
      case 388:
// 3047 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(2, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8135 "parser.cpp"
        break;
      case 389:
// 3052 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(0, yymsp[-5].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(107,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8145 "parser.cpp"
        break;
      case 390:
// 3057 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(0, yymsp[-4].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8155 "parser.cpp"
        break;
      case 391:
// 3062 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-6].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(54,&yymsp[-7].minor);
  yy_destructor(55,&yymsp[-5].minor);
  yy_destructor(107,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8167 "parser.cpp"
        break;
      case 392:
// 3067 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-5].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(107,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8179 "parser.cpp"
        break;
      case 393:
// 3072 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-8].minor.yy0, 1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(54,&yymsp[-9].minor);
  yy_destructor(55,&yymsp[-7].minor);
  yy_destructor(107,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8193 "parser.cpp"
        break;
      case 394:
// 3077 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-7].minor.yy0, 1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-8].minor);
  yy_destructor(55,&yymsp[-6].minor);
  yy_destructor(107,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8207 "parser.cpp"
        break;
      case 395:
// 3082 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(1, yymsp[-5].minor.yy396, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8217 "parser.cpp"
        break;
      case 396:
// 3087 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(1, yymsp[-4].minor.yy396, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8227 "parser.cpp"
        break;
      case 397:
// 3092 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(2, yymsp[-7].minor.yy396, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8239 "parser.cpp"
        break;
      case 398:
// 3097 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(2, yymsp[-6].minor.yy396, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8251 "parser.cpp"
        break;
      case 399:
// 3102 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(3, yymsp[-7].minor.yy396, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8263 "parser.cpp"
        break;
      case 400:
// 3107 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(3, yymsp[-6].minor.yy396, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 8275 "parser.cpp"
        break;
      case 404:
// 3127 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("ternary", yymsp[-4].minor.yy396, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(8,&yymsp[-3].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 8284 "parser.cpp"
        break;
      case 407:
// 3140 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy396, status->scanner_state, 0);
}
// 8291 "parser.cpp"
        break;
      case 408:
// 3145 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(yymsp[-2].minor.yy0, yymsp[0].minor.yy396, status->scanner_state, 0);
  yy_destructor(89,&yymsp[-1].minor);
}
// 8299 "parser.cpp"
        break;
      case 409:
// 3150 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy396, status->scanner_state, 1);
  yy_destructor(15,&yymsp[-1].minor);
}
// 8307 "parser.cpp"
        break;
      case 410:
// 3155 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, status->scanner_state, 0);
  yy_destructor(89,&yymsp[-2].minor);
  yy_destructor(15,&yymsp[-1].minor);
}
// 8316 "parser.cpp"
        break;
      case 411:
// 3160 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8328 "parser.cpp"
        break;
      case 412:
// 3165 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", NULL, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8340 "parser.cpp"
        break;
      case 413:
// 3170 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", yymsp[-3].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8352 "parser.cpp"
        break;
      case 414:
// 3175 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8364 "parser.cpp"
        break;
      case 417:
      case 437:
// 3187 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_array_item(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(89,&yymsp[-1].minor);
}
// 8373 "parser.cpp"
        break;
      case 418:
      case 438:
// 3191 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_array_item(NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 8381 "parser.cpp"
        break;
      case 444:
// 3296 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_comment(yymsp[0].minor.yy0, status->scanner_state);
}
// 8388 "parser.cpp"
        break;
      case 445:
// 3300 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_cblock(yymsp[0].minor.yy0, status->scanner_state);
}
// 8395 "parser.cpp"
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

// 8505 "parser.cpp"
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
