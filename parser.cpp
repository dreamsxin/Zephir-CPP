/** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
// 54 "parser.lemon"


#include <iostream>
#include <cstddef>
#include <string>

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
	}
	if (nullptr != right) {
		(*ret)["right"] = *right;
	}
	if (nullptr != extra) {
		(*ret)["extra"] = *extra;
	}

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	std::cout << *ret << std::endl;

	return ret;
}

static Json::Value* xx_ret_array_item(Json::Value* key, Json::Value* value, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	if (nullptr != key) {
		(*ret)["key"] = key;
	}
	(*ret)["value"] = *value;

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

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_use_aliases_item(xx_parser_token *T, xx_parser_token *A, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["name"] = T->token;
	if (A) {
		(*ret)["alias"] = A->token;
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

	(*ret)["abstract"] = is_abstract;
	(*ret)["final"] = is_final;

	if (E) {
		(*ret)["extends"] = E->token;
	}

	if (nullptr != I) {
		(*ret)["implements"] = *I;
	}

	if (nullptr != class_definition) {
		(*ret)["definition"] = *class_definition;
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

	if (E) {
		(*ret)["definition"] = E->token;
	}

	if (nullptr != interface_definition) {
		(*ret)["definition"] = *interface_definition;
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
	}
	if (nullptr != methods) {
		(*ret)["methods"] = *methods;
	}
	if (nullptr != constants) {
		(*ret)["constants"] = *constants;
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
	}
	if (nullptr != constants) {
		(*ret)["constants"] = *constants;
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
	(*ret)["property"] = "property";
	(*ret)["name"] = T->token;

	if (nullptr != default_value) {
		(*ret)["default"] = *default_value;
	}

	if (D) {
		(*ret)["docblock"] = D->token;
	}

	if (nullptr != shortcuts) {
		(*ret)["shortcuts"] = *shortcuts;
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
	}
	(*ret)["name"] = D->token;

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
	(*ret)["default"] = default_value;

	if (D) {
		(*ret)["docblock"] = D->token;
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
	(*ret)["type"] = "method";
	(*ret)["name"] = T->token;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
	}

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
	}

	if (D) {
		(*ret)["docblock"] = D->token;
	}

	if (nullptr != return_type) {
		(*ret)["return-type"] = *return_type;
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
	(*ret)["const"] = const_param;

	if (nullptr != type) {
		(*ret)["data-type"] = *type;
		(*ret)["mandatory"] = mandatory;
	} else {
		(*ret)["data-type"] = "variable";
		(*ret)["mandatory"] = 0;
	}

	if (nullptr != cast) {
		(*ret)["cast"] = *cast;
	}
	if (nullptr != default_value) {
		(*ret)["default"] = *default_value;
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
	}

	if (nullptr != cast) {
		(*ret)["cast"] = *cast;
		(*ret)["collection"] = collection;
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
		ret->append(*list_left);
	}

	if (nullptr != right_list) {
		ret->append(*right_list);
	}

	return ret;
}

static Json::Value* xx_ret_let_statement(Json::Value* assignments, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["type"] = "let";
	(*ret)["assignments"] = *assignments;

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
	}
	(*ret)["variable"] = V->token;
	if (P) {
		(*ret)["property"] = P->token;
	}
	if (nullptr != index_expr) {
		(*ret)["index-expr"] = *index_expr;
	}
	if (nullptr != expr) {
		(*ret)["expr"] = *expr;
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

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
	}

	if (nullptr != elseif_statements) {
		(*ret)["elseif_statements"] = *elseif_statements;
	}

	if (nullptr != else_statements) {
		(*ret)["else_statements"] = *else_statements;
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

	if (nullptr != clauses) {
		(*ret)["clauses"] = *clauses;
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
	} else {
		(*ret)["type"] = "default";
	}

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
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

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
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

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
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
	}
	if (nullptr != catches) {
		(*ret)["catches"] = *catches;
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
	}

	if (nullptr != variable) {
		(*ret)["variable"] = *variable;
	}

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
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

	if (K) {
		(*ret)["key"] = K->token;
	}
	if (V) {
		(*ret)["value"] = V->token;
	}

	(*ret)["reverse"] = reverse;

	if (nullptr != statements) {
		(*ret)["statements"] = *statements;
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

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}

static Json::Value* xx_ret_declare_variable(xx_parser_token *T, Json::Value* expr, xx_scanner_state *state)
{
	Json::Value* ret = new Json::Value();

	(*ret)["variable"] = T->token;
	if (nullptr != expr) {
		(*ret)["expr"] = *expr;
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
	(*ret)["dynamic"] = dynamic;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
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
	(*ret)["call-type"] = type;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
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
	(*ret)["variable"] = O;
	(*ret)["name"] = M->token;
	(*ret)["call-type"] = type;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
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
	(*ret)["dynamic"] = dynamic_method;
	(*ret)["name"] = M->token;

	if (nullptr != parameters) {
		(*ret)["parameters"] = *parameters;
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
	}
	(*ret)["parameter"] = *parameter;
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

	(*ret)["file"] = state->active_file;
	(*ret)["line"] = state->active_line;
	(*ret)["char"] = state->active_char;

	return ret;
}


// 1117 "parser.c"
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
#define YYNOCODE 219
#define YYACTIONTYPE unsigned short int
#define xx_TOKENTYPE xx_parser_token*
typedef union {
  xx_TOKENTYPE yy0;
  Json::Value* yy262;
  int yy437;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define xx_ARG_SDECL xx_parser_status *status;
#define xx_ARG_PDECL ,xx_parser_status *status
#define xx_ARG_FETCH xx_parser_status *status = yypParser->status
#define xx_ARG_STORE yypParser->status = status
#define YYNSTATE 852
#define YYNRULE 418
#define YYERRORSYMBOL 120
#define YYERRSYMDT yy437
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
 /*     0 */   537,  109,  302,  305, 1271,    1,    2,  851,    4,    5,
 /*    10 */     6,    7,    8,    9,  354,  852,   75,   46,   47,   51,
 /*    20 */    52,  287,  130,  138,  260,  142,  290,  364,  299,  369,
 /*    30 */   296,  389,  284,  446,  379,  499,  468,  358,  242,  144,
 /*    40 */   145,  147,  146,  148,  320,  322,  324,  396,  215,  586,
 /*    50 */   116,  124,  378,  409,  348,  313,  309,  264,  375,  696,
 /*    60 */   109,  546,  555,  564,  567,  558,  561,  570,  576,  573,
 /*    70 */   582,  579,  105,  106,  110,  243,  245,  247,  130,  138,
 /*    80 */   257,   82,   12,  821,  268,  272,  277,  415,  262,  426,
 /*    90 */   151,  152,  433,   29,  172,  451,  182,  460,  468,  372,
 /*   100 */   149,  150,  363,  518,  533,  540,  543,  371,  373,  374,
 /*   110 */   376,  377,  587,  537,   40,  302,  305,   74,   16,   10,
 /*   120 */    18,  264,   13,  696,   22,  380,  193,  354,  828,  841,
 /*   130 */   658,  202,  586,  108,  287,  403,  405,  404,  368,  290,
 /*   140 */   364,  299,  369,  296,  389,  284,  652,  379,  359,   55,
 /*   150 */   358,  242,  144,  145,  147,  146,  148,   15,  178,   25,
 /*   160 */   396,  217,  586,  163,   28,  378,  409,  348,   56,  130,
 /*   170 */   138,  375,  142,  174,  546,  555,  564,  567,  558,  561,
 /*   180 */   570,  576,  573,  582,  579,   35,   21,   17,  243,  245,
 /*   190 */   247,   14,   34,  257,   11,  587,  169,  268,  272,  277,
 /*   200 */   415,   18,  426,  151,  152,  433,  897,  172,  482,  182,
 /*   210 */   488,  468,  372,  149,  150,   19,  518,  533,  540,  543,
 /*   220 */   371,  373,  374,  376,  377,  587,  537,   81,  302,  305,
 /*   230 */   204,  120, 1263,  206,  814,  820,  117,  819,  804,  732,
 /*   240 */   354,  708,  702,  192,  780,  130,  138,  287,  142,  169,
 /*   250 */    94,  796,  290,  364,  299,  369,  296,  389,  284,  725,
 /*   260 */   379,   76,  724,  358,  242,  144,  145,  147,  146,  148,
 /*   270 */    20,  214,  134,  396,  714,  586,  411,  131,  378,  409,
 /*   280 */   348,   23,  130,  138,  375,  142,   24,  546,  555,  564,
 /*   290 */   567,  558,  561,  570,  576,  573,  582,  579,  764,  187,
 /*   300 */   795,  243,  245,  247,  130,  138,  257,  142,  252,  713,
 /*   310 */   268,  272,  277,  415,  183,  426,  151,  152,  433,  203,
 /*   320 */   783,   77,  182,  104,   80,  372,  149,  150,   81,  518,
 /*   330 */   533,  540,  543,  371,  373,  374,  376,  377,  587,  537,
 /*   340 */   154,  302,  305,  316,  318,  326,  320,  322,  324,  739,
 /*   350 */    26,  169,  738,  354,  196,   28,  825,  313,  309,  202,
 /*   360 */   287,  734,  164,  798,  609,  290,  364,  299,  369,  296,
 /*   370 */   389,  284,  615,  379,  793,  799,  358,  242,  144,  145,
 /*   380 */   147,  146,  148,  209,  103,   27,  396,  251,  586,  208,
 /*   390 */    31,  378,  409,  348,  169,  159,  165,  375,  162,  168,
 /*   400 */   546,  555,  564,  567,  558,  561,  570,  576,  573,  582,
 /*   410 */   579,  200,  423,  824,  243,  245,  247,  199,  731,  257,
 /*   420 */   202,  253,  702,  268,  272,  277,  415,  727,  426,  151,
 /*   430 */   152,  433,  888,  783,  143,  182,  261,  699,  372,  149,
 /*   440 */   150,  363,  518,  533,  540,  543,  371,  373,  374,  376,
 /*   450 */   377,  587,  537,   37,  302,  305,  399,    3,    4,    5,
 /*   460 */     6,    7,    8,    9,  310,   41,  354,   44,   73,   47,
 /*   470 */    51,   52,  388,  287,  669,  313,  309,   38,  290,  364,
 /*   480 */   299,  369,  296,  389,  284,  757,  379,  359,  756,  358,
 /*   490 */   242,  144,  145,  147,  146,  148,  360,  752,  595,  396,
 /*   500 */   700,  586,  628,   45,  378,  409,  348,  832,  436,  838,
 /*   510 */   375,  665,  202,  546,  555,  564,  567,  558,  561,  570,
 /*   520 */   576,  573,  582,  579,  390,  419,  619,  243,  245,  247,
 /*   530 */   422,  618,  257,  387,  642,  395,  268,  272,  277,  415,
 /*   540 */   438,  426,  151,  152,  433,  892,  781,  435,  182,  472,
 /*   550 */   508,  372,  149,  150,  363,  518,  533,  540,  543,  371,
 /*   560 */   373,  374,  376,  377,  587,  537,   42,  302,  305,  394,
 /*   570 */    30,   33,   32,   35,  191,  395,   35,  509,  170,  354,
 /*   580 */   171,  192, 1244,  171,  169,  461,  287,  169,  467,  428,
 /*   590 */    81,  290,  364,  299,  369,  296,  389,  284,  771,  379,
 /*   600 */   359,  770,  358,  242,  144,  145,  147,  146,  148,  401,
 /*   610 */   766,  476,  396,  256,  586,  434,  517,  378,  409,  348,
 /*   620 */   845,   48,  848,  375,  412,  202,  546,  555,  564,  567,
 /*   630 */   558,  561,  570,  576,  573,  582,  579,  450,  629,  513,
 /*   640 */   243,  245,  247,  489,  521,  257,  467,  471,  634,  268,
 /*   650 */   272,  277,  415,  395,  426,  151,  152,  433,  889,  783,
 /*   660 */   500,  182,  549,  467,  372,  149,  150,  798,  518,  533,
 /*   670 */   540,  543,  371,  373,  374,  376,  377,  587,  537,  799,
 /*   680 */   302,  305,  747,  520,  549,  778,  207,  632,  782,  804,
 /*   690 */   547,  554,  354,  169,  192,  780,  549,  556,  554,  287,
 /*   700 */   169,  548,  786,  109,  290,  364,  299,  369,  296,  389,
 /*   710 */   284,  792,  379,  549,  787,  358,  242,  144,  145,  147,
 /*   720 */   146,  148,  826,  557,  824,  396,  695,  586,  559,  554,
 /*   730 */   378,  409,  348,  562,  554,  560,  375,  565,  554,  546,
 /*   740 */   555,  564,  567,  558,  561,  570,  576,  573,  582,  579,
 /*   750 */   568,  554,  563,  243,  245,  247,  571,  554,  257,  574,
 /*   760 */   554,   57,  268,  272,  277,  415,   54,  426,  151,  152,
 /*   770 */   433,  899,  153,  102,  111,  106,  110,  372,  149,  150,
 /*   780 */    58,  518,  533,  540,  543,  371,  373,  374,  376,  377,
 /*   790 */   587,  537,   79,  302,  305,   78,  312,   44,   73,   47,
 /*   800 */    51,   52,  549,  549,   85,  354,  403,  405,  404,  368,
 /*   810 */   670,   84,  287,  666,  406,  806,   94,  290,  364,  299,
 /*   820 */   369,  296,  389,  284,  792,  379,  549,  807,  358,  242,
 /*   830 */   144,  145,  147,  146,  148,  577,  554,  745,  396,  271,
 /*   840 */   586,  566,  569,  378,  409,  348,  741,  549,  549,  375,
 /*   850 */   580,  554,  546,  555,  564,  567,  558,  561,  570,  576,
 /*   860 */   573,  582,  579,  719,  702,  572,  243,  245,  247,  583,
 /*   870 */   554,  257,  676,  715,  713,  268,  272,  277,  415,  763,
 /*   880 */   426,  151,  152,  433,  898,  153,  575,  578,  759,  549,
 /*   890 */   372,  149,  150, 1265,  518,  533,  540,  543,  371,  373,
 /*   900 */   374,  376,  377,  587,  537,  395,  302,  305,  160,  312,
 /*   910 */    44,   73,   47,   51,   52,  549,  395,   95,  354,  403,
 /*   920 */   405,  404,  368,  794,  801,  287,  662,  406,  581,  395,
 /*   930 */   290,  364,  299,  369,  296,  389,  284,  835,  379,  639,
 /*   940 */   202,  358,  242,  144,  145,  147,  146,  148,  716,  702,
 /*   950 */   648,  396,  694,  586,  584, 1264,  378,  409,  348,  836,
 /*   960 */   107,  824,  375,  656,  114,  546,  555,  564,  567,  558,
 /*   970 */   561,  570,  576,  573,  582,  579,  777,  789,  809,  243,
 /*   980 */   245,  247,  115,  118,  257,  773,  792,  792,  268,  272,
 /*   990 */   277,  415,  119,  426,  151,  152,  433,  891,  746,  839,
 /*  1000 */   121,  824,  395,  372,  149,  150,  122,  518,  533,  540,
 /*  1010 */   543,  371,  373,  374,  376,  377,  587,  537,  395,  302,
 /*  1020 */   305,  166,  312,   44,   73,   47,   51,   52,  395,  395,
 /*  1030 */   784,  354,  403,  405,  404,  368,  663,  123,  287,  655,
 /*  1040 */   406,  169,   81,  290,  364,  299,  369,  296,  389,  284,
 /*  1050 */   125,  379,  667,  129,  358,  242,  144,  145,  147,  146,
 /*  1060 */   148,  126,  674,  680,  396,  276,  586,   55,  127,  378,
 /*  1070 */   409,  348,  849,  132,  824,  375,  733,  135,  546,  555,
 /*  1080 */   564,  567,  558,  561,  570,  576,  573,  582,  579,  133,
 /*  1090 */   137,  136,  243,  245,  247,  139,  140,  257,  141,  157,
 /*  1100 */   175,  268,  272,  277,  415,  156,  426,  151,  152,  433,
 /*  1110 */   895,  746,  161,  167,  176,   81,  372,  149,  150,  177,
 /*  1120 */   518,  533,  540,  543,  371,  373,  374,  376,  377,  587,
 /*  1130 */   537,  180,  302,  305,  726,  312,   44,   73,   47,   51,
 /*  1140 */    52,  179,  181,  184,  354,  403,  405,  404,  368,  765,
 /*  1150 */   186,  287,  393,  406,  194,  185,  290,  364,  299,  369,
 /*  1160 */   296,  389,  284,  188,  379,  190,  197,  358,  242,  144,
 /*  1170 */   145,  147,  146,  148,  189,  195,  823,  396,  690,  586,
 /*  1180 */   198,  201,  378,  409,  348,  205,  211,  212,  375,  244,
 /*  1190 */   246,  546,  555,  564,  567,  558,  561,  570,  576,  573,
 /*  1200 */   582,  579,  249,  701,  248,  243,  245,  247,  254,  259,
 /*  1210 */   257,  263,  785,  689,  268,  272,  277,  415,  266,  426,
 /*  1220 */   151,  152,  433,  403,  405,  404,  368,  265,  748,  372,
 /*  1230 */   149,  150,  269,  518,  533,  540,  543,  371,  373,  374,
 /*  1240 */   376,  377,  587,  537,  274,  302,  305,  740,  312,   44,
 /*  1250 */    73,   47,   51,   52,  278,  281,  283,  354,  403,  405,
 /*  1260 */   404,  368,  311,  352,  287,  400,  406,  356,  355,  290,
 /*  1270 */   364,  299,  369,  296,  389,  284,  361,  379,  365,  366,
 /*  1280 */   358,  242,  144,  145,  147,  146,  148, 1246, 1245,  391,
 /*  1290 */   396,  280,  586, 1243,  397,  378,  409,  348,  398,  410,
 /*  1300 */   413,  375,  416,  421,  546,  555,  564,  567,  558,  561,
 /*  1310 */   570,  576,  573,  582,  579,  617,  420,  439,  243,  245,
 /*  1320 */   247,  427,  437,  257,  458,  805,  285,  268,  272,  277,
 /*  1330 */   415,  455,  426,  151,  152,  433,  403,  405,  404,  368,
 /*  1340 */   473,  210,  372,  149,  150,  465,  518,  533,  540,  543,
 /*  1350 */   371,  373,  374,  376,  377,  587,  537,  474,  302,  305,
 /*  1360 */   440,  441,  442,  443,  444,  445,  477,  478,  481,  758,
 /*  1370 */   354,   44,   73,   47,   51,   52,  486,  287,  493,  497,
 /*  1380 */   504,  510,  290,  364,  299,  369,  296,  389,  284,  511,
 /*  1390 */   379,  514,  515,  358,  242,  144,  145,  147,  146,  148,
 /*  1400 */   526,  528,  530,  396,  622,  586,  532,  551,  378,  409,
 /*  1410 */   348,  552,  550,  553,  375,  596,  597,  546,  555,  564,
 /*  1420 */   567,  558,  561,  570,  576,  573,  582,  579,  610,  611,
 /*  1430 */   616,  243,  245,  247,  624,  630,  257,  635,  636,  637,
 /*  1440 */   268,  272,  277,  415,  643,  426,  151,  152,  433,  893,
 /*  1450 */   817,  644,  645,  651,  671,  372,  149,  150,  672,  518,
 /*  1460 */   533,  540,  543,  371,  373,  374,  376,  377,  587,  537,
 /*  1470 */   677,  302,  305,  678,  691,  693,  523,  697,  704,  709,
 /*  1480 */   717,  750,  718,  354,  519,  524,  403,  405,  404,  368,
 /*  1490 */   287,  720,  749,  802,  788,  290,  364,  299,  369,  296,
 /*  1500 */   389,  284,  791,  379,  790,  910,  358,  242,  144,  145,
 /*  1510 */   147,  146,  148,  911,  797,  800,  396,  621,  586,  803,
 /*  1520 */   808,  378,  409,  348,  812,  811,  810,  375,  813,  829,
 /*  1530 */   546,  555,  564,  567,  558,  561,  570,  576,  573,  582,
 /*  1540 */   579,  822,  827,  830,  243,  245,  247,  831,  833,  257,
 /*  1550 */   834,  837,  840,  268,  272,  277,  415,  843,  426,  151,
 /*  1560 */   152,  433,  896,  746,  842,  844,  846,  847,  372,  149,
 /*  1570 */   150,  202,  518,  533,  540,  543,  371,  373,  374,  376,
 /*  1580 */   377,  587,  537,  850,  302,  305,  772,  312,   44,   73,
 /*  1590 */    47,   51,   52,  638,  638,  638,  354,  403,  405,  404,
 /*  1600 */   368,  638,  638,  287,  631,  406,  638,  638,  290,  364,
 /*  1610 */   299,  369,  296,  389,  284,  638,  379,  638,  638,  358,
 /*  1620 */   242,  144,  145,  147,  146,  148,  638,  638,  638,  396,
 /*  1630 */   620,  586,  638,  638,  378,  409,  348,  638,  638,  638,
 /*  1640 */   375,  638,  638,  546,  555,  564,  567,  558,  561,  570,
 /*  1650 */   576,  573,  582,  579,  638,  638,  638,  243,  245,  247,
 /*  1660 */   638,  638,  257,  638,  638,  638,  268,  272,  277,  415,
 /*  1670 */   638,  426,  151,  152,  433,  894,  746,  638,  638,  638,
 /*  1680 */   638,  372,  149,  150,  638,  518,  533,  540,  543,  371,
 /*  1690 */   373,  374,  376,  377,  587,  537,  638,  302,  305,  638,
 /*  1700 */   312,  638,  638,  638,  638,  638,  638,  638,  638,  354,
 /*  1710 */   403,  405,  404,  368,  638,  638,  287,  638,  406,  638,
 /*  1720 */   638,  290,  364,  299,  369,  296,  389,  284,  638,  379,
 /*  1730 */   638,  638,  358,  242,  144,  145,  147,  146,  148,  638,
 /*  1740 */   638,  638,  396,  418,  586,  638,  638,  378,  409,  348,
 /*  1750 */   638,  638,  638,  375,  638,  638,  546,  555,  564,  567,
 /*  1760 */   558,  561,  570,  576,  573,  582,  579,  638,  638,  638,
 /*  1770 */   243,  245,  247,  638,  638,  257,  638,  638,  638,  268,
 /*  1780 */   272,  277,  415,  638,  426,  151,  152,  433,  890,  746,
 /*  1790 */   638,  638,  638,  638,  372,  149,  150,  638,  518,  533,
 /*  1800 */   540,  543,  371,  373,  374,  376,  377,  587,  537,  638,
 /*  1810 */   302,  305,  638,  312,  638,  638,  638,  638,  638,  638,
 /*  1820 */   638,  638,  354,  403,  405,  404,  368,  638,  638,  287,
 /*  1830 */   647,  406,  638,  638,  290,  364,  299,  369,  296,  389,
 /*  1840 */   284,  638,  379,  638,  638,  358,  242,  144,  145,  147,
 /*  1850 */   146,  148,  638,  638,  638,  396,  608,  586,  638,  638,
 /*  1860 */   378,  409,  348,  638,  638,  638,  375,  638,  638,  546,
 /*  1870 */   555,  564,  567,  558,  561,  570,  576,  573,  582,  579,
 /*  1880 */   638,  638,  258,  243,  245,  247,  638,  638,  257,  638,
 /*  1890 */   638,  689,  268,  272,  277,  415,  638,  426,  151,  152,
 /*  1900 */   433,  403,  405,  404,  368,  638,   36,  372,  149,  150,
 /*  1910 */   638,  518,  533,  540,  543,  371,  373,  374,  376,  377,
 /*  1920 */   587,  537,  638,  302,  305,  638,  312,  638,  638,  638,
 /*  1930 */   638,  638,  638,  638,  638,  354,  403,  405,  404,  368,
 /*  1940 */   638,  638,  287,  673,  406,  638,  638,  290,  364,  299,
 /*  1950 */   369,  296,  389,  284,  638,  379,  638,  638,  358,  242,
 /*  1960 */   144,  145,  147,  146,  148,  638,  638,  638,  396,  425,
 /*  1970 */   586,  638,  638,  378,  409,  348,  638,  638,  638,  375,
 /*  1980 */   638,  638,  546,  555,  564,  567,  558,  561,  570,  576,
 /*  1990 */   573,  582,  579,  638,  638,  273,  243,  245,  247,  638,
 /*  2000 */   638,  257,  638,  638,  689,  268,  272,  277,  415,  638,
 /*  2010 */   426,  151,  152,  433,  403,  405,  404,  368,  638,  155,
 /*  2020 */   372,  149,  150,  638,  518,  533,  540,  543,  371,  373,
 /*  2030 */   374,  376,  377,  587,  537,  638,  302,  305,  638,  312,
 /*  2040 */   638,  638,  638,  638,  816,  638,  782,  804,  354,  403,
 /*  2050 */   405,  404,  368,  780,  638,  287,  679,  406,  169,  638,
 /*  2060 */   290,  364,  299,  369,  296,  389,  284,  638,  379,  638,
 /*  2070 */   638,  358,  242,  144,  145,  147,  146,  148,  638,  638,
 /*  2080 */   638,  396,  589,  586,  638,  638,  378,  409,  348,  638,
 /*  2090 */   638,  638,  375,  638,  638,  546,  555,  564,  567,  558,
 /*  2100 */   561,  570,  576,  573,  582,  579,  638,  638,  638,  243,
 /*  2110 */   245,  247,  638,  638,  257,  638,  638,  351,  268,  272,
 /*  2120 */   277,  415,  638,  426,  151,  152,  433,  638,  638,  638,
 /*  2130 */   173,  638,  638,  372,  149,  150,  638,  518,  533,  540,
 /*  2140 */   543,  371,  373,  374,  376,  377,  587,  537,  638,  302,
 /*  2150 */   305,  638,  349,  638,  638,  638,  638,  638,  638,  638,
 /*  2160 */   638,  354,  403,  405,  404,  368,  638,  638,  287,  638,
 /*  2170 */   638,  638,  638,  290,  364,  299,  369,  296,  389,  284,
 /*  2180 */   638,  379,  638,  638,  358,  242,  144,  145,  147,  146,
 /*  2190 */   148,  638,  638,  638,  396,  432,  586,  638,  638,  378,
 /*  2200 */   409,  348,  638,  638,  638,  375,  638,  638,  546,  555,
 /*  2210 */   564,  567,  558,  561,  570,  576,  573,  582,  579,  638,
 /*  2220 */   638,  282,  243,  245,  247,  638,  638,  257,  638,  638,
 /*  2230 */   689,  268,  272,  277,  415,  638,  426,  151,  152,  433,
 /*  2240 */   403,  405,  404,  368,  638,  210,  372,  149,  150,  638,
 /*  2250 */   518,  533,  540,  543,  371,  373,  374,  376,  377,  587,
 /*  2260 */   537,  638,  302,  305,  638,  312,  638,  638,  638,  638,
 /*  2270 */   638,  638,  638,  638,  354,  403,  405,  404,  368,  638,
 /*  2280 */   638,  287,  638,  641,  638,  638,  290,  364,  299,  369,
 /*  2290 */   296,  389,  284,  638,  379,  638,  638,  358,  242,  144,
 /*  2300 */   145,  147,  146,  148,  638,  638,  638,  396,  594,  586,
 /*  2310 */   638,  638,  378,  409,  348,  638,  638,  638,  375,  638,
 /*  2320 */   638,  546,  555,  564,  567,  558,  561,  570,  576,  573,
 /*  2330 */   582,  579,  638,  638,  692,  243,  245,  247,  638,  638,
 /*  2340 */   257,  638,  638,  689,  268,  272,  277,  415,  638,  426,
 /*  2350 */   151,  152,  433,  403,  405,  404,  368,  638,  748,  372,
 /*  2360 */   149,  150,  638,  518,  533,  540,  543,  371,  373,  374,
 /*  2370 */   376,  377,  587,  537,  638,  302,  305,  638,  448,  638,
 /*  2380 */   638,  638,  447,  638,  638,  638,  638,  354,  403,  405,
 /*  2390 */   404,  368,  638,  638,  287,  638,  638,  638,  638,  290,
 /*  2400 */   364,  299,  369,  296,  389,  284,  638,  379,  638,  638,
 /*  2410 */   358,  242,  144,  145,  147,  146,  148,  638,  638,  638,
 /*  2420 */   396,  602,  586,  638,  638,  378,  409,  348,  638,  638,
 /*  2430 */   638,  375,  638,  638,  546,  555,  564,  567,  558,  561,
 /*  2440 */   570,  576,  573,  582,  579,  638,  638,  703,  243,  245,
 /*  2450 */   247,  638,  638,  257,  638,  638,  689,  268,  272,  277,
 /*  2460 */   415,  638,  426,  151,  152,  433,  403,  405,  404,  368,
 /*  2470 */   638,  638,  372,  149,  150,  638,  518,  533,  540,  543,
 /*  2480 */   371,  373,  374,  376,  377,  587,  537,  638,  302,  305,
 /*  2490 */   638,  448,  638,  638,  638,  452,  638,  638,  638,  638,
 /*  2500 */   354,  403,  405,  404,  368,  638,  638,  287,  638,  638,
 /*  2510 */   638,  638,  290,  364,  299,  369,  296,  389,  284,  638,
 /*  2520 */   379,  638,  638,  358,  242,  638,  815,  638,  818,  638,
 /*  2530 */   819,  804,  638,  396,  601,  586,  170,  780,  378,  409,
 /*  2540 */   348,  638,  169,  638,  375,  638,  638,  546,  555,  564,
 /*  2550 */   567,  558,  561,  570,  576,  573,  582,  579,  638,  638,
 /*  2560 */   638,  243,  245,  247,  638,  638,  257,  638,  638,  459,
 /*  2570 */   268,  272,  277,  415,  638,  426,  457,  638,  433,  403,
 /*  2580 */   405,  404,  368,  638,  638,  372,  638,  638,  638,  518,
 /*  2590 */   533,  540,  543,  371,  373,  374,  376,  377,  587,  537,
 /*  2600 */   638,  302,  305,  638,  448,  638,  638,  638,  456,  638,
 /*  2610 */   638,  638,  638,  354,  403,  405,  404,  368,  638,  638,
 /*  2620 */   287,  638,  638,  638,  638,  290,  364,  299,  369,  296,
 /*  2630 */   389,  284,  523,  379,  638,  638,  358,  242,  638,  638,
 /*  2640 */   638,  522,  403,  405,  404,  368,  396,  607,  586,  638,
 /*  2650 */   638,  378,  409,  348,  638,  638,  638,  375,  638,  638,
 /*  2660 */   546,  555,  564,  567,  558,  561,  570,  576,  573,  582,
 /*  2670 */   579,  638,  638,  638,  243,  245,  247,  638,  638,  257,
 /*  2680 */   638,  638,  688,  268,  272,  277,  415,  638,  426,  638,
 /*  2690 */   638,  433,  403,  405,  404,  368,  638,  638,  372,  638,
 /*  2700 */   638,  638,  518,  533,  540,  543,  371,  373,  374,  376,
 /*  2710 */   377,  587,  537,  638,  302,  305,  638,  448,  638,  638,
 /*  2720 */   638,  462,  638,  638,  288,  638,  354,  403,  405,  404,
 /*  2730 */   368,  638,  638,  287,  403,  405,  404,  368,  290,  364,
 /*  2740 */   299,  369,  296,  389,  284,  687,  379,  638,  638,  358,
 /*  2750 */   242,  638,  638,  638,  638,  403,  405,  404,  368,  396,
 /*  2760 */   614,  586,  638,  638,  378,  409,  348,  638,  638,  638,
 /*  2770 */   375,  638,  638,  546,  555,  564,  567,  558,  561,  570,
 /*  2780 */   576,  573,  582,  579,  638,  638,  638,  243,  245,  247,
 /*  2790 */   638,  638,  257,  638,  638,  291,  268,  272,  277,  415,
 /*  2800 */   638,  426,  638,  638,  433,  403,  405,  404,  368,  638,
 /*  2810 */   638,  372,  638,  638,  638,  518,  533,  540,  543,  371,
 /*  2820 */   373,  374,  376,  377,  587,  537,  638,  302,  305,  638,
 /*  2830 */   448,  638,  638,  638,  466,  638,  638,  686,  638,  354,
 /*  2840 */   403,  405,  404,  368,  638,  638,  287,  403,  405,  404,
 /*  2850 */   368,  290,  364,  299,  369,  296,  389,  284,  294,  379,
 /*  2860 */   638,  638,  358,  242,  638,  638,  638,  638,  403,  405,
 /*  2870 */   404,  368,  396,  613,  586,  638,  638,  378,  409,  348,
 /*  2880 */   638,  638,  638,  375,  638,  638,  546,  555,  564,  567,
 /*  2890 */   558,  561,  570,  576,  573,  582,  579,  638,  638,  638,
 /*  2900 */   243,  245,  247,  638,  638,  257,  638,  638,  685,  268,
 /*  2910 */   272,  277,  415,  638,  426,  638,  638,  433,  403,  405,
 /*  2920 */   404,  368,  638,  638,  372,  638,  638,  638,  518,  533,
 /*  2930 */   540,  543,  371,  373,  374,  376,  377,  587,  537,  638,
 /*  2940 */   302,  305,  638,  448,  638,  638,  638,  475,  638,  638,
 /*  2950 */   297,  638,  354,  403,  405,  404,  368,  638,  638,  287,
 /*  2960 */   403,  405,  404,  368,  290,  364,  299,  369,  296,  389,
 /*  2970 */   284,  684,  379,  638,  638,  358,  242,  638,  638,  638,
 /*  2980 */   638,  403,  405,  404,  368,  396,  627,  586,  638,  638,
 /*  2990 */   378,  409,  348,  638,  638,  638,  375,  638,  638,  546,
 /*  3000 */   555,  564,  567,  558,  561,  570,  576,  573,  582,  579,
 /*  3010 */   638,  638,  638,  243,  245,  247,  638,  638,  257,  638,
 /*  3020 */   638,  300,  268,  272,  277,  415,  638,  426,  638,  638,
 /*  3030 */   433,  403,  405,  404,  368,  638,  638,  372,  638,  638,
 /*  3040 */   638,  518,  533,  540,  543,  371,  373,  374,  376,  377,
 /*  3050 */   587,  537,  638,  302,  305,  638,  448,  638,  638,  638,
 /*  3060 */   479,  638,  638,  683,  638,  354,  403,  405,  404,  368,
 /*  3070 */   638,  638,  287,  403,  405,  404,  368,  290,  364,  299,
 /*  3080 */   369,  296,  389,  284,  303,  379,  638,  638,  358,  242,
 /*  3090 */   638,  638,  638,  638,  403,  405,  404,  368,  396,  626,
 /*  3100 */   586,  638,  638,  378,  409,  348,  638,  638,  638,  375,
 /*  3110 */   638,  638,  546,  555,  564,  567,  558,  561,  570,  576,
 /*  3120 */   573,  582,  579,  638,  638,  638,  243,  245,  247,  638,
 /*  3130 */   638,  257,  638,  638,  682,  268,  272,  277,  415,  638,
 /*  3140 */   426,  638,  638,  433,  403,  405,  404,  368,  638,  638,
 /*  3150 */   372,  638,  638,  638,  518,  533,  540,  543,  371,  373,
 /*  3160 */   374,  376,  377,  587,  537,  638,  302,  305,  638,  448,
 /*  3170 */   638,  638,  638,  483,  638,  638,  306,  638,  354,  403,
 /*  3180 */   405,  404,  368,  638,  638,  287,  403,  405,  404,  368,
 /*  3190 */   290,  364,  299,  369,  296,  389,  284,  308,  379,  638,
 /*  3200 */   638,  358,  242,  638,  638,  638,  638,  403,  405,  404,
 /*  3210 */   368,  396,  707,  586,  638,  638,  378,  409,  348,  638,
 /*  3220 */   638,  638,  375,  638,  638,  546,  555,  564,  567,  558,
 /*  3230 */   561,  570,  576,  573,  582,  579,  638,  638,  638,  243,
 /*  3240 */   245,  247,  638,  638,  257,  638,  638,  314,  268,  272,
 /*  3250 */   277,  415,  638,  426,  638,  638,  433,  403,  405,  404,
 /*  3260 */   368,  638,  638,  372,  638,  638,  638,  518,  533,  540,
 /*  3270 */   543,  371,  373,  374,  376,  377,  587,  537,  638,  302,
 /*  3280 */   305,  638,  448,  638,  638,  638,  487,  638,  638,  317,
 /*  3290 */   638,  354,  403,  405,  404,  368,  638,  638,  287,  403,
 /*  3300 */   405,  404,  368,  290,  364,  299,  369,  296,  389,  284,
 /*  3310 */   319,  379,  638,  638,  358,  242,  638,  638,  638,  638,
 /*  3320 */   403,  405,  404,  368,  396,  706,  586,  638,  638,  378,
 /*  3330 */   409,  348,  638,  638,  638,  375,  638,  638,  546,  555,
 /*  3340 */   564,  567,  558,  561,  570,  576,  573,  582,  579,  638,
 /*  3350 */   638,  638,  243,  245,  247,  638,  638,  257,  638,  638,
 /*  3360 */   321,  268,  272,  277,  415,  638,  426,  638,  638,  433,
 /*  3370 */   403,  405,  404,  368,  638,  638,  372,  638,  638,  638,
 /*  3380 */   518,  533,  540,  543,  371,  373,  374,  376,  377,  587,
 /*  3390 */   537,  638,  302,  305,  638,  448,  638,  638,  638,  490,
 /*  3400 */   638,  638,  323,  638,  354,  403,  405,  404,  368,  638,
 /*  3410 */   638,  287,  403,  405,  404,  368,  290,  364,  299,  369,
 /*  3420 */   296,  389,  284,  325,  379,  638,  638,  358,  242,  638,
 /*  3430 */   638,  638,  638,  403,  405,  404,  368,  396,  712,  586,
 /*  3440 */   638,  638,  378,  409,  348,  638,  638,  638,  375,  638,
 /*  3450 */   638,  546,  555,  564,  567,  558,  561,  570,  576,  573,
 /*  3460 */   582,  579,  638,  638,  638,  243,  245,  247,  638,  638,
 /*  3470 */   257,  638,  638,  327,  268,  272,  277,  415,  638,  426,
 /*  3480 */   638,  638,  433,  403,  405,  404,  368,  638,  638,  372,
 /*  3490 */   638,  638,  638,  518,  533,  540,  543,  371,  373,  374,
 /*  3500 */   376,  377,  587,  537,  638,  302,  305,  638,  448,  638,
 /*  3510 */   638,  638,  494,  638,  638,  329,  638,  354,  403,  405,
 /*  3520 */   404,  368,  638,  638,  287,  403,  405,  404,  368,  290,
 /*  3530 */   364,  299,  369,  296,  389,  284,  331,  379,  638,  638,
 /*  3540 */   358,  242,  638,  638,  638,  638,  403,  405,  404,  368,
 /*  3550 */   396,  711,  586,  638,  638,  378,  409,  348,  638,  638,
 /*  3560 */   638,  375,  638,  638,  546,  555,  564,  567,  558,  561,
 /*  3570 */   570,  576,  573,  582,  579,  638,  638,  638,  243,  245,
 /*  3580 */   247,  638,  638,  257,  638,  638,  333,  268,  272,  277,
 /*  3590 */   415,  638,  426,  638,  638,  433,  403,  405,  404,  368,
 /*  3600 */   638,  638,  372,  638,  638,  638,  518,  533,  540,  543,
 /*  3610 */   371,  373,  374,  376,  377,  587,  537,  638,  302,  305,
 /*  3620 */   638,  448,  638,  638,  638,  498,  638,  638,  335,  638,
 /*  3630 */   354,  403,  405,  404,  368,  638,  638,  287,  403,  405,
 /*  3640 */   404,  368,  290,  364,  299,  369,  296,  389,  284,  337,
 /*  3650 */   379,  638,  638,  358,  242,  638,  638,  638,  638,  403,
 /*  3660 */   405,  404,  368,  396,  723,  586,  638,  638,  378,  409,
 /*  3670 */   348,  638,  638,  638,  375,  638,  638,  546,  555,  564,
 /*  3680 */   567,  558,  561,  570,  576,  573,  582,  579,  638,  638,
 /*  3690 */   638,  243,  245,  247,  638,  638,  257,  638,  638,  339,
 /*  3700 */   268,  272,  277,  415,  638,  426,  638,  638,  433,  403,
 /*  3710 */   405,  404,  368,  638,  638,  372,  638,  638,  638,  518,
 /*  3720 */   533,  540,  543,  371,  373,  374,  376,  377,  587,  537,
 /*  3730 */   638,  302,  305,  638,  448,  638,  638,  638,  501,  638,
 /*  3740 */   638,  341,  638,  354,  403,  405,  404,  368,  638,  638,
 /*  3750 */   287,  403,  405,  404,  368,  290,  364,  299,  369,  296,
 /*  3760 */   389,  284,  343,  379,  638,  638,  358,  242,  638,  638,
 /*  3770 */   638,  638,  403,  405,  404,  368,  396,  722,  586,  638,
 /*  3780 */   638,  378,  409,  348,  638,  638,  638,  375,  638,  638,
 /*  3790 */   546,  555,  564,  567,  558,  561,  570,  576,  573,  582,
 /*  3800 */   579,  638,  638,  638,  243,  245,  247,  638,  638,  257,
 /*  3810 */   638,  638,  345,  268,  272,  277,  415,  638,  426,  638,
 /*  3820 */   638,  433,  403,  405,  404,  368,  638,  638,  372,  638,
 /*  3830 */   638,  638,  518,  533,  540,  543,  371,  373,  374,  376,
 /*  3840 */   377,  587,  537,  638,  302,  305,  638,  448,  638,  638,
 /*  3850 */   638,  505,  638,  638,  347,  638,  354,  403,  405,  404,
 /*  3860 */   368,  638,  638,  287,  403,  405,  404,  368,  290,  364,
 /*  3870 */   299,  369,  296,  389,  284,  353,  379,  638,  638,  358,
 /*  3880 */   242,  638,  638,  638,  638,  403,  405,  404,  368,  396,
 /*  3890 */   728,  586,  638,  638,  378,  409,  348,  638,  638,  638,
 /*  3900 */   375,  638,  638,  546,  555,  564,  567,  558,  561,  570,
 /*  3910 */   576,  573,  582,  579,  638,  638,  638,  243,  245,  247,
 /*  3920 */   638,  638,  257,  638,  638,  357,  268,  272,  277,  415,
 /*  3930 */   638,  426,  638,  638,  433,  403,  405,  404,  368,  638,
 /*  3940 */   638,  372,  638,  638,  638,  518,  533,  540,  543,  371,
 /*  3950 */   373,  374,  376,  377,  587,  537,  638,  302,  305,  638,
 /*  3960 */   448,  638,  638,  638,  512,  638,  638,  367,  638,  354,
 /*  3970 */   403,  405,  404,  368,  638,  638,  287,  403,  405,  404,
 /*  3980 */   368,  290,  364,  299,  369,  296,  389,  284,  370,  379,
 /*  3990 */   638,  638,  358,  242,  638,  638,  638,  638,  403,  405,
 /*  4000 */   404,  368,  396,  730,  586,  638,  638,  378,  409,  348,
 /*  4010 */   638,  638,  638,  375,  638,  638,  546,  555,  564,  567,
 /*  4020 */   558,  561,  570,  576,  573,  582,  579,  638,  638,  638,
 /*  4030 */   243,  245,  247,  638,  638,  257,  638,  638,  408,  268,
 /*  4040 */   272,  277,  415,  638,  426,  638,  638,  433,  403,  405,
 /*  4050 */   404,  368,  638,  638,  372,  638,  638,  638,  518,  533,
 /*  4060 */   540,  543,  371,  373,  374,  376,  377,  587,  537,  638,
 /*  4070 */   302,  305,  638,  448,  638,  638,  638,  516,  638,  638,
 /*  4080 */   429,  638,  354,  403,  405,  404,  368,  638,  638,  287,
 /*  4090 */   403,  405,  404,  368,  290,  364,  299,  369,  296,  389,
 /*  4100 */   284,  534,  379,  638,  638,  358,  242,  638,  638,  638,
 /*  4110 */   638,  403,  405,  404,  368,  396,  735,  586,  638,  638,
 /*  4120 */   378,  409,  348,  638,  638,  638,  375,  638,  638,  546,
 /*  4130 */   555,  564,  567,  558,  561,  570,  576,  573,  582,  579,
 /*  4140 */   638,  638,  638,  243,  245,  247,  638,  638,  257,  638,
 /*  4150 */   638,  538,  268,  272,  277,  415,  638,  426,  638,  638,
 /*  4160 */   433,  403,  405,  404,  368,  638,  638,  372,  638,  638,
 /*  4170 */   638,  518,  533,  540,  543,  371,  373,  374,  376,  377,
 /*  4180 */   587,  537,  638,  302,  305,  638,  541,  638,  638,  638,
 /*  4190 */   638,  638,  638,  544,  638,  354,  403,  405,  404,  368,
 /*  4200 */   638,  638,  287,  403,  405,  404,  368,  290,  364,  299,
 /*  4210 */   369,  296,  389,  284,  591,  379,  638,  638,  358,  242,
 /*  4220 */   638,  638,  638,  638,  403,  405,  404,  368,  396,  737,
 /*  4230 */   586,  638,  638,  378,  409,  348,  638,  638,  638,  375,
 /*  4240 */   638,  638,  546,  555,  564,  567,  558,  561,  570,  576,
 /*  4250 */   573,  582,  579,  638,  638,  638,  243,  245,  247,  638,
 /*  4260 */   638,  257,  638,  638,  598,  268,  272,  277,  415,  638,
 /*  4270 */   426,  638,  638,  433,  403,  405,  404,  368,  638,  638,
 /*  4280 */   372,  638,  638,  638,  518,  533,  540,  543,  371,  373,
 /*  4290 */   374,  376,  377,  587,  537,  638,  302,  305,  638,  604,
 /*  4300 */   638,  638,  638,  638,  638,  638,  659,  638,  354,  403,
 /*  4310 */   405,  404,  368,  638,  638,  287,  403,  405,  404,  368,
 /*  4320 */   290,  364,  299,  369,  296,  389,  284,  661,  379,  638,
 /*  4330 */   638,  358,  242,  638,  638,  638,  638,  403,  405,  404,
 /*  4340 */   368,  396,  742,  586,  638,  638,  378,  409,  348,  638,
 /*  4350 */   638,  638,  375,  638,  638,  546,  555,  564,  567,  558,
 /*  4360 */   561,  570,  576,  573,  582,  579,  638,  638,  638,  243,
 /*  4370 */   245,  247,  638,  638,  257,  638,  638,  638,  268,  272,
 /*  4380 */   277,  415,  638,  426,  638,  638,  433,  638,  638,  638,
 /*  4390 */   638,  638,  638,  372,  638,  638,  638,  518,  533,  540,
 /*  4400 */   543,  371,  373,  374,  376,  377,  587,  537,  779,  302,
 /*  4410 */   305,  209,  638,  638,  638,  638,  170,  780,  638,  638,
 /*  4420 */   638,  354,  169,  638,  638,  638,  638,  638,  287,  638,
 /*  4430 */   638,  638,  638,  290,  364,  299,  369,  296,  389,  284,
 /*  4440 */   638,  379,  638,  638,  358,  242,  638,  638,  638,  638,
 /*  4450 */   638,  638,  638,  638,  396,  744,  586,  638,  638,  378,
 /*  4460 */   409,  348,  638,  638,  638,  375,  638,  638,  546,  555,
 /*  4470 */   564,  567,  558,  561,  570,  576,  573,  582,  579,  638,
 /*  4480 */   638,  638,  243,  245,  247,  638,  638,  257,  638,  638,
 /*  4490 */   638,  268,  272,  277,  415,  638,  426,  638,  638,  433,
 /*  4500 */   638,  638,  638,  638,  638,  638,  372,  638,  638,  638,
 /*  4510 */   518,  533,  540,  543,  371,  373,  374,  376,  377,  587,
 /*  4520 */   537,  638,  302,  305,  638,  638,  638,  638,  638,  638,
 /*  4530 */   638,  638,  638,  638,  354,  638,  638,  638,  638,  638,
 /*  4540 */   638,  287,  638,  638,  638,  638,  290,  364,  299,  369,
 /*  4550 */   296,  389,  284,  638,  379,  638,  638,  358,  242,  638,
 /*  4560 */   638,  638,  638,  638,  638,  638,  638,  396,  753,  586,
 /*  4570 */   638,  638,  378,  409,  348,  638,  638,  638,  375,  638,
 /*  4580 */   638,  546,  555,  564,  567,  558,  561,  570,  576,  573,
 /*  4590 */   582,  579,  638,  638,  638,  243,  245,  247,  638,  638,
 /*  4600 */   257,  638,  638,  638,  268,  272,  277,  415,  638,  426,
 /*  4610 */   638,  638,  433,  638,  638,  638,  638,  638,  638,  372,
 /*  4620 */   638,  638,  638,  518,  533,  540,  543,  371,  373,  374,
 /*  4630 */   376,  377,  587,  537,  638,  302,  305,  638,  638,  638,
 /*  4640 */   638,  638,  638,  638,  638,  638,  638,  354,  638,  638,
 /*  4650 */   638,  638,  638,  638,  287,  638,  638,  638,  638,  290,
 /*  4660 */   364,  299,  369,  296,  389,  284,  638,  379,  638,  638,
 /*  4670 */   358,  242,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  4680 */   396,  755,  586,  638,  638,  378,  409,  348,  638,  638,
 /*  4690 */   638,  375,  638,  638,  546,  555,  564,  567,  558,  561,
 /*  4700 */   570,  576,  573,  582,  579,  638,  638,  638,  243,  245,
 /*  4710 */   247,  638,  638,  257,  638,  638,  638,  268,  272,  277,
 /*  4720 */   415,  638,  426,  638,  638,  433,  638,  638,  638,  638,
 /*  4730 */   638,  638,  372,  638,  638,  638,  518,  533,  540,  543,
 /*  4740 */   371,  373,  374,  376,  377,  587,  537,  638,  302,  305,
 /*  4750 */   638,  638,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  4760 */   354,  638,  638,  638,  638,  638,  638,  287,  638,  638,
 /*  4770 */   638,  638,  290,  364,  299,  369,  296,  389,  284,  638,
 /*  4780 */   379,  638,  638,  358,  242,  638,  638,  638,  638,  638,
 /*  4790 */   638,  638,  638,  396,  760,  586,  638,  638,  378,  409,
 /*  4800 */   348,  638,  638,  638,  375,  638,  638,  546,  555,  564,
 /*  4810 */   567,  558,  561,  570,  576,  573,  582,  579,  638,  638,
 /*  4820 */   638,  243,  245,  247,  638,  638,  257,  638,  638,  638,
 /*  4830 */   268,  272,  277,  415,  638,  426,  638,  638,  433,  638,
 /*  4840 */   638,  638,  638,  638,  638,  372,  638,  638,  638,  518,
 /*  4850 */   533,  540,  543,  371,  373,  374,  376,  377,  587,  537,
 /*  4860 */   638,  302,  305,  638,  638,  638,  638,  638,  638,  638,
 /*  4870 */   638,  638,  638,  354,  638,  638,  638,  638,  638,  638,
 /*  4880 */   287,  638,  638,  638,  638,  290,  364,  299,  369,  296,
 /*  4890 */   389,  284,  638,  379,  638,  638,  358,  242,  638,  638,
 /*  4900 */   638,  638,  638,  638,  638,  638,  396,  762,  586,  638,
 /*  4910 */   638,  378,  409,  348,  638,  638,  638,  375,  638,  638,
 /*  4920 */   546,  555,  564,  567,  558,  561,  570,  576,  573,  582,
 /*  4930 */   579,  638,  638,  638,  243,  245,  247,  638,  638,  257,
 /*  4940 */   638,  638,  638,  268,  272,  277,  415,  638,  426,  638,
 /*  4950 */   638,  433,  638,  638,  638,  638,  638,  638,  372,  638,
 /*  4960 */   638,  638,  518,  533,  540,  543,  371,  373,  374,  376,
 /*  4970 */   377,  587,  537,  638,  302,  305,  638,  638,  638,  638,
 /*  4980 */   638,  638,  638,  638,  638,  638,  354,  638,  638,  638,
 /*  4990 */   638,  638,  638,  287,  638,  638,  638,  638,  290,  364,
 /*  5000 */   299,  369,  296,  389,  284,  638,  379,  638,  638,  358,
 /*  5010 */   242,  638,  638,  638,  638,  638,  638,  638,  638,  396,
 /*  5020 */   767,  586,  638,  638,  378,  409,  348,  638,  638,  638,
 /*  5030 */   375,  638,  638,  546,  555,  564,  567,  558,  561,  570,
 /*  5040 */   576,  573,  582,  579,  638,  638,  638,  243,  245,  247,
 /*  5050 */   638,  638,  257,  638,  638,  638,  268,  272,  277,  415,
 /*  5060 */   638,  426,  638,  638,  433,  638,  638,  638,  638,  638,
 /*  5070 */   638,  372,  638,  638,  638,  518,  533,  540,  543,  371,
 /*  5080 */   373,  374,  376,  377,  587,  537,  638,  302,  305,  638,
 /*  5090 */   638,  638,  638,  638,  638,  638,  638,  638,  638,  354,
 /*  5100 */   638,  638,  638,  638,  638,  638,  287,  638,  638,  638,
 /*  5110 */   638,  290,  364,  299,  369,  296,  389,  284,  638,  379,
 /*  5120 */   638,  638,  358,  242,  638,  638,  638,  638,  638,  638,
 /*  5130 */   638,  638,  396,  769,  586,  638,  638,  378,  409,  348,
 /*  5140 */   638,  638,  638,  375,  638,  638,  546,  555,  564,  567,
 /*  5150 */   558,  561,  570,  576,  573,  582,  579,  638,  638,  638,
 /*  5160 */   243,  245,  247,  638,  638,  257,  638,  638,  638,  268,
 /*  5170 */   272,  277,  415,  638,  426,  638,  638,  433,  638,  638,
 /*  5180 */   638,  638,  638,  638,  372,  638,  638,  638,  518,  533,
 /*  5190 */   540,  543,  371,  373,  374,  376,  377,  587,  537,  638,
 /*  5200 */   302,  305,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  5210 */   638,  638,  354,  638,  638,  638,  638,  638,  638,  287,
 /*  5220 */   638,  638,  638,  638,  290,  364,  299,  369,  296,  389,
 /*  5230 */   284,  638,  379,  638,  638,  358,  242,  638,  638,  638,
 /*  5240 */   638,  638,  638,  638,  638,  396,  774,  586,  638,  638,
 /*  5250 */   378,  409,  348,  638,  638,  638,  375,  638,  638,  546,
 /*  5260 */   555,  564,  567,  558,  561,  570,  576,  573,  582,  579,
 /*  5270 */   638,  638,  638,  243,  245,  247,  638,  638,  257,  638,
 /*  5280 */   638,  638,  268,  272,  277,  415,  638,  426,  638,  638,
 /*  5290 */   433,  638,  638,  638,  638,  638,  638,  372,  638,  638,
 /*  5300 */   638,  518,  533,  540,  543,  371,  373,  374,  376,  377,
 /*  5310 */   587,  537,  638,  302,  305,  638,  638,  638,  638,  638,
 /*  5320 */   638,  638,  638,  638,  638,  354,  638,  638,  638,  638,
 /*  5330 */   638,  638,  287,  638,  638,  638,  638,  290,  364,  299,
 /*  5340 */   369,  296,  389,  284,  638,  379,  638,  638,  358,  242,
 /*  5350 */   638,  638,  638,  638,  638,  638,  638,  638,  396,  776,
 /*  5360 */   586,  638,  638,  378,  409,  348,  638,  638,  638,  375,
 /*  5370 */   638,  638,  546,  555,  564,  567,  558,  561,  570,  576,
 /*  5380 */   573,  582,  579,  638,  638,  638,  243,  245,  247,  638,
 /*  5390 */   638,  257,  638,  638,  638,  268,  272,  277,  415,  638,
 /*  5400 */   426,  638,  638,  433,  638,  638,  638,  638,  638,  638,
 /*  5410 */   372,  638,  638,  638,  518,  533,  540,  543,  371,  373,
 /*  5420 */   374,  376,  377,  587,  537,  638,  302,  305,  638,  638,
 /*  5430 */   638,  638,  638,  638,  638,  638,  638,  638,  354,  638,
 /*  5440 */   638,  638,  638,  638,  638,  287,  638,  638,  638,  638,
 /*  5450 */   290,  364,  299,  369,  296,  389,  284,  638,  379,  638,
 /*  5460 */   638,  358,  242,  638,  638,  638,  638,  638,  638,  638,
 /*  5470 */   638,  396,  638,  586,  638,  638,  378,  409,  348,  638,
 /*  5480 */   638,  638,  375,  638,  638,  546,  555,  564,  567,  558,
 /*  5490 */   561,  570,  576,  573,  582,  579,  638,  638,  638,  243,
 /*  5500 */   245,  247,  638,  638,  257,  638,  638,  638,  268,  272,
 /*  5510 */   277,  415,  638,  426,  638,  638,  433,  638,  638,  638,
 /*  5520 */   638,  638,  638,  372,  638,  638,  638,  518,  533,  540,
 /*  5530 */   543,  371,  373,  374,  376,  377,  587,  293,  638,  302,
 /*  5540 */   305,  638,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  5550 */   638,  354,  638,  638,  638,  638,  638,  638,  287,  638,
 /*  5560 */   638,  638,  638,  290,  364,  299,  369,  296,  389,  284,
 /*  5570 */   638,  379,  638,  638,  358,  638,  638,  638,  638,  638,
 /*  5580 */   638,  638,  638,  638,  396,  638,  638,  638,  638,  378,
 /*  5590 */   409,  348,  638,  638,  638,  375,  638,  638,   59,   60,
 /*  5600 */    61,   62,   63,   64,   65,   66,   67,   68,   69,   70,
 /*  5610 */    71,   72,  638,  638,  638,  638,  638,  240,  219,  286,
 /*  5620 */   292,  298,  301,  304,  307,  295,  289,  316,  318,  326,
 /*  5630 */   320,  322,  324,  638,  638,  638,  372,  638,  638,  638,
 /*  5640 */   216,  313,  309,  638,  371,  373,  374,  376,  377,  588,
 /*  5650 */   220,  221,  222,  223,  224,  225,  226,  227,  228,  229,
 /*  5660 */   230,  231,  232,  233,  234,  235,  236,  237,  238,  239,
 /*  5670 */   241,  638,  638,  638,  638,  638,  638,  638,  453,  638,
 /*  5680 */   585,  240,  219,  638,  638,  638,  380,  638,  638,  638,
 /*  5690 */   525,  527,  529,  531,  439,  638,  403,  405,  404,  368,
 /*  5700 */   638,  638,  386,  638,  250,  654,  650,  653,  638,  638,
 /*  5710 */   638,  638,  638,  588,  220,  221,  222,  223,  224,  225,
 /*  5720 */   226,  227,  228,  229,  230,  231,  232,  233,  234,  235,
 /*  5730 */   236,  237,  238,  239,  241,  240,  219,  440,  441,  442,
 /*  5740 */   443,  444,  445,  638,  585,  469,  470,  638,  638,  638,
 /*  5750 */   638,  638,  638,  638,  525,  527,  529,  531,  255,  638,
 /*  5760 */   638,  638,  638,  638,  638,  638,  638,  588,  220,  221,
 /*  5770 */   222,  223,  224,  225,  226,  227,  228,  229,  230,  231,
 /*  5780 */   232,  233,  234,  235,  236,  237,  238,  239,  241,  240,
 /*  5790 */   219,  638,  638,  638,  638,  638,   97,  638,  585,   93,
 /*  5800 */   638,  638,  638,  638,  638,  638,  638,  638,  525,  527,
 /*  5810 */   529,  531,  267,  638,   96,  638,  638,  638,  638,  638,
 /*  5820 */    90,  588,  220,  221,  222,  223,  224,  225,  226,  227,
 /*  5830 */   228,  229,  230,  231,  232,  233,  234,  235,  236,  237,
 /*  5840 */   238,  239,  241,  240,  219,  638,  638,  638,  638,  638,
 /*  5850 */    97,  638,  585,  100,  638,  638,  638,  638,  638,  638,
 /*  5860 */   638,   88,  525,  527,  529,  531,  270,  638,   96,   86,
 /*  5870 */    87,   89,   92,   91,   90,  588,  220,  221,  222,  223,
 /*  5880 */   224,  225,  226,  227,  228,  229,  230,  231,  232,  233,
 /*  5890 */   234,  235,  236,  237,  238,  239,  241,  240,  219,  638,
 /*  5900 */   638,  638,  638,  638,  638,  463,  585,  638,  638,  638,
 /*  5910 */   638,  638,  638,  638,  638,   99,  525,  527,  529,  531,
 /*  5920 */   275,  439,  638,   98,   87,   89,   92,   91,  638,  588,
 /*  5930 */   220,  221,  222,  223,  224,  225,  226,  227,  228,  229,
 /*  5940 */   230,  231,  232,  233,  234,  235,  236,  237,  238,  239,
 /*  5950 */   241,  240,  219,  638,  638,  638,  638,  638,  638,  484,
 /*  5960 */   585,  638,  638,  638,  440,  441,  442,  443,  444,  445,
 /*  5970 */   525,  527,  529,  531,  279,  439,  638,  638,  638,  638,
 /*  5980 */   638,  638,  638,  588,  220,  221,  222,  223,  224,  225,
 /*  5990 */   226,  227,  228,  229,  230,  231,  232,  233,  234,  235,
 /*  6000 */   236,  237,  238,  239,  241,  240,  219,  638,  638,  638,
 /*  6010 */   638,  638,  638,  638,  585,  638,  638,  638,  440,  441,
 /*  6020 */   442,  443,  444,  445,  525,  527,  529,  531,  414,  638,
 /*  6030 */   638,  638,  638,  638,  638,  638,  638,  588,  220,  221,
 /*  6040 */   222,  223,  224,  225,  226,  227,  228,  229,  230,  231,
 /*  6050 */   232,  233,  234,  235,  236,  237,  238,  239,  241,  240,
 /*  6060 */   219,  638,  638,  638,  380,  638,  638,  638,  585,  638,
 /*  6070 */   638,  638,  638,  638,  403,  405,  404,  368,  525,  527,
 /*  6080 */   529,  531,  417,  649,  650,  653,  638,  638,  638,  638,
 /*  6090 */   638,  588,  220,  221,  222,  223,  224,  225,  226,  227,
 /*  6100 */   228,  229,  230,  231,  232,  233,  234,  235,  236,  237,
 /*  6110 */   238,  239,  241,  240,  219,  638,  638,  638,  638,  638,
 /*  6120 */   638,  491,  585,  638,  638,  638,  638,  638,  638,  638,
 /*  6130 */   638,  638,  525,  527,  529,  531,  424,  439,  638,  638,
 /*  6140 */   638,  638,  638,  638,  638,  588,  220,  221,  222,  223,
 /*  6150 */   224,  225,  226,  227,  228,  229,  230,  231,  232,  233,
 /*  6160 */   234,  235,  236,  237,  238,  239,  241,  240,  219,  638,
 /*  6170 */   638,  638,  638,  638,  638,  502,  585,  638,  638,  638,
 /*  6180 */   440,  441,  442,  443,  444,  445,  525,  527,  529,  531,
 /*  6190 */   431,  439,  638,  638,  638,  638,  638,  638,  638,  588,
 /*  6200 */   220,  221,  222,  223,  224,  225,  226,  227,  228,  229,
 /*  6210 */   230,  231,  232,  233,  234,  235,  236,  237,  238,  239,
 /*  6220 */   241,  240,  219,  638,  638,  638,  638,  638,  638,  638,
 /*  6230 */   585,  638,  638,  638,  440,  441,  442,  443,  444,  445,
 /*  6240 */   525,  527,  529,  531,  593,  638,  638,  638,  638,  638,
 /*  6250 */   638,  638,  638,  588,  220,  221,  222,  223,  224,  225,
 /*  6260 */   226,  227,  228,  229,  230,  231,  232,  233,  234,  235,
 /*  6270 */   236,  237,  238,  239,  241,  240,  219,  638,  638,  638,
 /*  6280 */   638,  638,  638,  638,  585,  638,  638,  638,  638,  638,
 /*  6290 */   638,  638,  638,  638,  525,  527,  529,  531,  600,  638,
 /*  6300 */   638,  638,  638,  638,  638,  638,  638,  588,  220,  221,
 /*  6310 */   222,  223,  224,  225,  226,  227,  228,  229,  230,  231,
 /*  6320 */   232,  233,  234,  235,  236,  237,  238,  239,  241,  240,
 /*  6330 */   219,  638,  638,  638,  638,  638,  638,  638,  585,  638,
 /*  6340 */   638,  638,  638,  638,  638,  638,  638,  638,  525,  527,
 /*  6350 */   529,  531,  606,  638,  638,  638,  638,  638,  638,  638,
 /*  6360 */   638,  588,  220,  221,  222,  223,  224,  225,  226,  227,
 /*  6370 */   228,  229,  230,  231,  232,  233,  234,  235,  236,  237,
 /*  6380 */   238,  239,  241,  240,  219,  638,  638,  638,  638,  638,
 /*  6390 */   638,  638,  585,  638,  638,  638,  638,  638,  638,  638,
 /*  6400 */   638,  638,  525,  527,  529,  531,  612,  638,  638,  638,
 /*  6410 */   638,  638,  638,  638,  638,  588,  220,  221,  222,  223,
 /*  6420 */   224,  225,  226,  227,  228,  229,  230,  231,  232,  233,
 /*  6430 */   234,  235,  236,  237,  238,  239,  241,  240,  219,  638,
 /*  6440 */   638,  638,  638,  638,  638,  638,  585,  638,  638,  638,
 /*  6450 */   638,  638,  638,  638,  638,  638,  525,  527,  529,  531,
 /*  6460 */   625,  638,  638,  638,  638,  638,  638,  638,  638,  588,
 /*  6470 */   220,  221,  222,  223,  224,  225,  226,  227,  228,  229,
 /*  6480 */   230,  231,  232,  233,  234,  235,  236,  237,  238,  239,
 /*  6490 */   241,  240,  219,  638,  638,  638,  638,  638,  638,  638,
 /*  6500 */   585,  638,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  6510 */   525,  527,  529,  531,  698,  638,  638,  638,  638,  638,
 /*  6520 */   638,  638,  638,  588,  220,  221,  222,  223,  224,  225,
 /*  6530 */   226,  227,  228,  229,  230,  231,  232,  233,  234,  235,
 /*  6540 */   236,  237,  238,  239,  241,  240,  219,  638,  638,  638,
 /*  6550 */   638,  638,  638,  638,  585,  638,  638,  638,  638,  638,
 /*  6560 */   638,  638,  638,  638,  525,  527,  529,  531,  705,  638,
 /*  6570 */   638,  638,  638,  638,  638,  638,  638,  588,  220,  221,
 /*  6580 */   222,  223,  224,  225,  226,  227,  228,  229,  230,  231,
 /*  6590 */   232,  233,  234,  235,  236,  237,  238,  239,  241,  240,
 /*  6600 */   219,  638,  638,  638,  638,  638,  638,  638,  585,  638,
 /*  6610 */   638,  638,  638,  638,  638,  638,  638,  638,  525,  527,
 /*  6620 */   529,  531,  710,  638,  638,  638,  638,  638,  638,  638,
 /*  6630 */   638,  588,  220,  221,  222,  223,  224,  225,  226,  227,
 /*  6640 */   228,  229,  230,  231,  232,  233,  234,  235,  236,  237,
 /*  6650 */   238,  239,  241,  240,  219,  638,  638,  638,  638,  638,
 /*  6660 */   638,  638,  585,  638,  638,  638,  638,  638,  638,  638,
 /*  6670 */   638,  638,  525,  527,  529,  531,  721,  638,  638,  638,
 /*  6680 */   638,  638,  638,  638,  638,  588,  220,  221,  222,  223,
 /*  6690 */   224,  225,  226,  227,  228,  229,  230,  231,  232,  233,
 /*  6700 */   234,  235,  236,  237,  238,  239,  241,  240,  219,  638,
 /*  6710 */   638,  638,  638,  638,  638,  638,  585,  638,  638,  638,
 /*  6720 */   638,  638,  638,  638,  638,  638,  525,  527,  529,  531,
 /*  6730 */   729,  638,  638,  638,  638,  638,  638,  638,  638,  588,
 /*  6740 */   220,  221,  222,  223,  224,  225,  226,  227,  228,  229,
 /*  6750 */   230,  231,  232,  233,  234,  235,  236,  237,  238,  239,
 /*  6760 */   241,  240,  219,  638,  638,  638,  638,  638,  638,  638,
 /*  6770 */   585,  638,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  6780 */   525,  527,  529,  531,  736,  638,  638,  638,  638,  638,
 /*  6790 */   638,  638,  638,  588,  220,  221,  222,  223,  224,  225,
 /*  6800 */   226,  227,  228,  229,  230,  231,  232,  233,  234,  235,
 /*  6810 */   236,  237,  238,  239,  241,  240,  219,  638,  638,  638,
 /*  6820 */   638,  638,  638,  638,  585,  638,  638,  638,  638,  638,
 /*  6830 */   638,  638,  638,  638,  525,  527,  529,  531,  743,  638,
 /*  6840 */   638,  638,  638,  638,  638,  638,  638,  588,  220,  221,
 /*  6850 */   222,  223,  224,  225,  226,  227,  228,  229,  230,  231,
 /*  6860 */   232,  233,  234,  235,  236,  237,  238,  239,  241,  240,
 /*  6870 */   219,  638,  638,  638,  638,  638,  638,  638,  585,  638,
 /*  6880 */   638,  638,  638,  638,  638,  638,  638,  638,  525,  527,
 /*  6890 */   529,  531,  754,  638,  638,  638,  638,  638,  638,  638,
 /*  6900 */   638,  588,  220,  221,  222,  223,  224,  225,  226,  227,
 /*  6910 */   228,  229,  230,  231,  232,  233,  234,  235,  236,  237,
 /*  6920 */   238,  239,  241,  240,  219,  638,  638,  638,  638,  638,
 /*  6930 */   638,  638,  585,  638,  638,  638,  638,  638,  638,  638,
 /*  6940 */   638,  638,  525,  527,  529,  531,  761,  638,  638,  638,
 /*  6950 */   638,  638,  638,  638,  638,  588,  220,  221,  222,  223,
 /*  6960 */   224,  225,  226,  227,  228,  229,  230,  231,  232,  233,
 /*  6970 */   234,  235,  236,  237,  238,  239,  241,  240,  219,  638,
 /*  6980 */   638,  638,  638,  638,  638,  638,  585,  638,  638,  638,
 /*  6990 */   638,  638,  638,  638,  638,  638,  525,  527,  529,  531,
 /*  7000 */   768,  638,  638,  638,  638,  638,  638,  638,  638,  588,
 /*  7010 */   220,  221,  222,  223,  224,  225,  226,  227,  228,  229,
 /*  7020 */   230,  231,  232,  233,  234,  235,  236,  237,  238,  239,
 /*  7030 */   241,  240,  219,  638,  638,  638,  638,  638,  638,  638,
 /*  7040 */   585,  638,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  7050 */   525,  527,  529,  531,  775,  638,  638,  638,  638,  638,
 /*  7060 */   638,  638,  638,  588,  220,  221,  222,  223,  224,  225,
 /*  7070 */   226,  227,  228,  229,  230,  231,  232,  233,  234,  235,
 /*  7080 */   236,  237,  238,  239,  241,  240,  219,  638,  638,  638,
 /*  7090 */   638,  638,  638,  638,  585,  638,  638,  638,  638,  638,
 /*  7100 */   638,  638,  638,  638,  525,  527,  529,  531,  638,  638,
 /*  7110 */   638,  638,  638,  638,  638,  638,  638,  218,  220,  221,
 /*  7120 */   222,  223,  224,  225,  226,  227,  228,  229,  230,  231,
 /*  7130 */   232,  233,  234,  235,  236,  237,  238,  239,  241,  638,
 /*  7140 */   638,  293,  638,  302,  305,  638,  638,  638,  585,  407,
 /*  7150 */   638,  638,  638,  638,  638,  354,  638,  638,  525,  527,
 /*  7160 */   529,  531,  287,  638,  638,  638,  638,  290,  364,  299,
 /*  7170 */   369,  296,  389,  284,  668,  379,  638,  293,  362,  302,
 /*  7180 */   305,  638,  638,  638,  638,  407,  638,  638,  396,  638,
 /*  7190 */   638,  354,  638,  378,  409,  348,  638,  638,  287,  375,
 /*  7200 */   638,  638,  638,  290,  364,  299,  369,  296,  389,  284,
 /*  7210 */   664,  379,  638,  638,  362,  638,  638,  638,  638,  638,
 /*  7220 */   638,  638,  638,  638,  396,  638,  638,  638,  638,  378,
 /*  7230 */   409,  348,  638,  638,  638,  375,  638,  638,  638,  638,
 /*  7240 */   372,  638,  638,  638,  638,  638,  638,  638,  371,  373,
 /*  7250 */   374,  376,  377,  330,  328,  334,  332,  336,  338,  340,
 /*  7260 */   286,  292,  298,  301,  304,  307,  295,  289,  316,  318,
 /*  7270 */   326,  320,  322,  324,  638,  638,  372,  638,  638,  638,
 /*  7280 */   638,  638,  313,  309,  371,  373,  374,  376,  377,  638,
 /*  7290 */   638,  638,  293,  638,  302,  305,  638,  638,  638,  638,
 /*  7300 */   407,  638,  638,  638,  638,  638,  354,  638,  638,  638,
 /*  7310 */   638,  638,  638,  287,  638,  638,  638,  638,  290,  364,
 /*  7320 */   299,  369,  296,  389,  284,  657,  379,  638,  293,  362,
 /*  7330 */   302,  305,  638,  638,  638,  638,  407,  638,  638,  396,
 /*  7340 */   638,  638,  354,  638,  378,  409,  348,  638,  638,  287,
 /*  7350 */   375,  638,  638,  638,  290,  364,  299,  369,  296,  389,
 /*  7360 */   284,  392,  379,  638,  638,  362,  638,  638,  638,  638,
 /*  7370 */   638,  638,  638,  638,  638,  396,  638,  638,  638,  638,
 /*  7380 */   378,  409,  348,  638,  638,  638,  375,  638,  638,  638,
 /*  7390 */   638,  372,  638,  638,  638,  638,  638,  638,  638,  371,
 /*  7400 */   373,  374,  376,  377,  638,  328,  334,  332,  336,  338,
 /*  7410 */   340,  286,  292,  298,  301,  304,  307,  295,  289,  316,
 /*  7420 */   318,  326,  320,  322,  324,  638,  638,  372,  638,  638,
 /*  7430 */   638,  638,  638,  313,  309,  371,  373,  374,  376,  377,
 /*  7440 */   638,  638,  638,  293,  638,  302,  305,  638,  638,  638,
 /*  7450 */   638,  407,  638,  638,  638,  638,  638,  354,  638,  638,
 /*  7460 */   638,  638,  638,  638,  287,  638,  638,  638,  638,  290,
 /*  7470 */   364,  299,  369,  296,  389,  284,  402,  379,  638,  293,
 /*  7480 */   362,  302,  305,  638,  638,  638,  638,  407,  638,  638,
 /*  7490 */   396,  638,  638,  354,  638,  378,  409,  348,  638,  638,
 /*  7500 */   287,  375,  638,  638,  638,  290,  364,  299,  369,  296,
 /*  7510 */   389,  284,  633,  379,  638,  638,  362,  638,  638,  638,
 /*  7520 */   638,  638,  638,  638,  638,  638,  396,  638,  638,  638,
 /*  7530 */   638,  378,  409,  348,  638,  638,  638,  375,  638,  638,
 /*  7540 */   638,  638,  372,  638,  638,  638,  638,  638,  638,  638,
 /*  7550 */   371,  373,  374,  376,  377,  638,  638,  334,  332,  336,
 /*  7560 */   338,  340,  286,  292,  298,  301,  304,  307,  295,  289,
 /*  7570 */   316,  318,  326,  320,  322,  324,  638,  638,  372,  638,
 /*  7580 */   638,  638,  638,  638,  313,  309,  371,  373,  374,  376,
 /*  7590 */   377,  638,  638,  638,  293,  638,  302,  305,  638,  638,
 /*  7600 */   638,  638,  407,  638,  638,  638,  638,  638,  354,  638,
 /*  7610 */   638,  638,  638,  638,  638,  287,  638,  638,  638,  638,
 /*  7620 */   290,  364,  299,  369,  296,  389,  284,  640,  379,  638,
 /*  7630 */   293,  362,  302,  305,  638,  638,  638,  638,  407,  638,
 /*  7640 */   638,  396,  638,  638,  354,  638,  378,  409,  348,  638,
 /*  7650 */   638,  287,  375,  638,  638,  638,  290,  364,  299,  369,
 /*  7660 */   296,  389,  284,  646,  379,  638,   53,  362,  638,  638,
 /*  7670 */   638,  638,  638,  638,  638,  638,  638,  396,  638,  638,
 /*  7680 */   638,  638,  378,  409,  348,  638,  638,  638,  375,  638,
 /*  7690 */   638,  638,  638,  372,  638,  638,  638,  638,  638,  638,
 /*  7700 */   638,  371,  373,  374,  376,  377,  638,  638,  638,   43,
 /*  7710 */    49,   50,  853,   59,   60,   61,   62,   63,   64,   65,
 /*  7720 */    66,   67,   68,   69,   70,   71,   72,  638,  638,  372,
 /*  7730 */   638,  638,  638,  638,  638,  638,  638,  371,  373,  374,
 /*  7740 */   376,  377,  638,  638,  638,  293,  638,  302,  305,  638,
 /*  7750 */   638,  638,  638,  407,  638,   10,   18,  638,   13,  354,
 /*  7760 */    22,  638,  193,  638,  828,  841,  287,  638,  586,  638,
 /*  7770 */   638,  290,  364,  299,  369,  296,  389,  284,  675,  379,
 /*  7780 */   638,  293,  362,  302,  305,  638,  638,  638,  638,  407,
 /*  7790 */   638,  638,  396,  638,  638,  354,  638,  378,  409,  348,
 /*  7800 */   638,  638,  287,  375,  638,  638,  638,  290,  364,  299,
 /*  7810 */   369,  296,  389,  284,  681,  379,  638,   53,  362,  638,
 /*  7820 */   638,  638,  638,  638,  638,  638,  638,  638,  396,  638,
 /*  7830 */   638,  587,  638,  378,  409,  348,  638,  638,  638,  375,
 /*  7840 */   638,  638,  638,  638,  372,  638,  638,  638,  638,  638,
 /*  7850 */   638,  638,  371,  373,  374,  376,  377,  638,  638,  638,
 /*  7860 */   638,   49,   50,  638,   59,   60,   61,   62,   63,   64,
 /*  7870 */    65,   66,   67,   68,   69,   70,   71,   72,  638,  638,
 /*  7880 */   372,  638,  638,  638,  638,  638,  638,  638,  371,  373,
 /*  7890 */   374,  376,  377,  638,  638,  638,  638,  344,  638,  638,
 /*  7900 */   342,  330,  328,  334,  332,  336,  338,  340,  286,  292,
 /*  7910 */   298,  301,  304,  307,  295,  289,  316,  318,  326,  320,
 /*  7920 */   322,  324,  638,  638,  638,  638,  638,  638,  638,  344,
 /*  7930 */   313,  309,  342,  330,  328,  334,  332,  336,  338,  340,
 /*  7940 */   286,  292,  298,  301,  304,  307,  295,  289,  316,  318,
 /*  7950 */   326,  320,  322,  324,  638,  638,  315,  638,  638,  638,
 /*  7960 */   638,  344,  313,  309,  342,  330,  328,  334,  332,  336,
 /*  7970 */   338,  340,  286,  292,  298,  301,  304,  307,  295,  289,
 /*  7980 */   316,  318,  326,  320,  322,  324,  293,  638,  302,  305,
 /*  7990 */   638,  638,  638,  350,  313,  309,  638,  638,  638,  638,
 /*  8000 */   354,  638,  638,  638,  638,  638,  638,  287,  638,  638,
 /*  8010 */   346,  638,  290,  364,  299,  369,  296,  389,  284,  638,
 /*  8020 */   379,  638,  293,  381,  302,  305,  638,  638,  638,  638,
 /*  8030 */   407,  638,  638,  396,  638,  638,  354,  638,  384,  409,
 /*  8040 */   348,  638,  638,  287,  375,  638,  385,  638,  290,  364,
 /*  8050 */   299,  369,  296,  389,  284,  638,  379,  638,  638,  362,
 /*  8060 */   638,  638,  638,  638,  638,  638,  638,  638,  638,  396,
 /*  8070 */   638,  638,  638,  638,  378,  409,  348,  638,  638,  638,
 /*  8080 */   375,  638,  638,  638,  638,  383,  638,  638,  293,  638,
 /*  8090 */   302,  305,  638,  382,  373,  374,  376,  377,  638,  638,
 /*  8100 */   638,  638,  354,  638,  638,  638,  638,  638,  638,  287,
 /*  8110 */   638,  638,  638,  638,  290,  364,  299,  369,  296,  389,
 /*  8120 */   284,  372,  379,  638,  638,  358,  638,  638,  638,  371,
 /*  8130 */   373,  374,  376,  377,  638,  396,  638,  638,  638,  638,
 /*  8140 */   378,  409,  348,  638,  344,  638,  375,  342,  330,  328,
 /*  8150 */   334,  332,  336,  338,  340,  286,  292,  298,  301,  304,
 /*  8160 */   307,  295,  289,  316,  318,  326,  320,  322,  324,  638,
 /*  8170 */   638,  638,  638,  638,  638,  638,  638,  313,  309,  590,
 /*  8180 */   638,  638,  638,  638,  638,  638,  293,  372,  302,  305,
 /*  8190 */   430,  638,  638,  638,  638,  371,  373,  374,  376,  377,
 /*  8200 */   354,  638,  638,  638,  638,  638,  638,  287,  638,  638,
 /*  8210 */   638,  638,  290,  364,  299,  369,  296,  389,  284,  638,
 /*  8220 */   379,  638,  638,  358,  638,  638,  638,  638,  638,  638,
 /*  8230 */   638,  638,  638,  396,  638,  638,  638,  638,  378,  409,
 /*  8240 */   348,  638,  638,  638,  375,  638,  454,  344,  638,  638,
 /*  8250 */   342,  330,  328,  334,  332,  336,  338,  340,  286,  292,
 /*  8260 */   298,  301,  304,  307,  295,  289,  316,  318,  326,  320,
 /*  8270 */   322,  324,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  8280 */   313,  309,  638,  638,  638,  372,  638,  638,  293,  638,
 /*  8290 */   302,  305,  638,  371,  373,  374,  376,  377,  638,  638,
 /*  8300 */   638,  638,  354,  638,  638,  638, 1120,  638,  638,  287,
 /*  8310 */   638,  638,  638,  638,  290,  364,  299,  369,  296,  389,
 /*  8320 */   284,  638,  379,  638,  293,  358,  302,  305,  638,  638,
 /*  8330 */   638,  638,  638,  638,  638,  396,  638,  638,  354,  638,
 /*  8340 */   378,  409,  348,  638,  638,  287,  375,  638,  464,  638,
 /*  8350 */   290,  364,  299,  369,  296,  389,  284,  638,  379,  638,
 /*  8360 */   638,  358,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  8370 */   638,  396,  638,  638,  638,  638,  378,  409,  348,  638,
 /*  8380 */   638,  638,  375,  638,  485,  638,  638,  372,  638,  638,
 /*  8390 */   293,  638,  302,  305,  638,  371,  373,  374,  376,  377,
 /*  8400 */   638,  638,  638,  638,  354,  638,  638,  638,  638,  638,
 /*  8410 */   638,  287,  638,  638,  638,  638,  290,  364,  299,  369,
 /*  8420 */   296,  389,  284,  372,  379,  638,  293,  358,  302,  305,
 /*  8430 */   638,  371,  373,  374,  376,  377,  638,  396,  638,  638,
 /*  8440 */   354,  638,  378,  409,  348,  638,  638,  287,  375,  638,
 /*  8450 */   492,  638,  290,  364,  299,  369,  296,  389,  284,  638,
 /*  8460 */   379,  638,  638,  358,  638,  638,  638,  638,  638,  638,
 /*  8470 */   638,  638,  638,  396,  638,  638,  638,  638,  378,  409,
 /*  8480 */   348,  638,  638,  638,  375,  638,  496,  638,  638,  372,
 /*  8490 */   638,  638,  293,  638,  302,  305,  638,  371,  373,  374,
 /*  8500 */   376,  377,  638,  638,  638,  638,  354,  638,  638,  638,
 /*  8510 */   638,  638,  638,  287,  638,  638,  638,  638,  290,  364,
 /*  8520 */   299,  369,  296,  389,  284,  372,  379,  638,  293,  358,
 /*  8530 */   302,  305,  638,  371,  373,  374,  376,  377,  638,  396,
 /*  8540 */   638,  638,  354,  638,  378,  409,  348,  638,  638,  287,
 /*  8550 */   375,  638,  503,  638,  290,  364,  299,  369,  296,  389,
 /*  8560 */   284,  128,  379,  638,  638,  358,  536,  638,  638,  638,
 /*  8570 */   638,  638,  638,  638,  638,  396,  638,  638,  638,  638,
 /*  8580 */   378,  409,  348,  638,   83,  638,  375,  638,  638,  638,
 /*  8590 */   638,  372,  638,  638,  638,  638,  638,  638,  112,  371,
 /*  8600 */   373,  374,  376,  377,  638,  638,  638,  638,   59,   60,
 /*  8610 */    61,   62,   63,   64,   65,   66,   67,   68,   69,   70,
 /*  8620 */    71,   72,  638,  638,  638,  638,  638,  372,  638,  638,
 /*  8630 */   638,  638,  638,  638,  638,  371,  373,  374,  376,  377,
 /*  8640 */   638,  638,  638,  638,  344,  638,  638,  342,  330,  328,
 /*  8650 */   334,  332,  336,  338,  340,  286,  292,  298,  301,  304,
 /*  8660 */   307,  295,  289,  316,  318,  326,  320,  322,  324,  638,
 /*  8670 */   638,  638,  638,  638,  638,  638,  638,  313,  309,  638,
 /*  8680 */   638,  535,  638,  638,  638,  638,  344,  638,  638,  342,
 /*  8690 */   330,  328,  334,  332,  336,  338,  340,  286,  292,  298,
 /*  8700 */   301,  304,  307,  295,  289,  316,  318,  326,  320,  322,
 /*  8710 */   324,  638,  638,  638,  638,  638,  638,  638,  638,  313,
 /*  8720 */   309,  638,  638,  539,  638,  638,  638,  638,  344,  638,
 /*  8730 */   638,  342,  330,  328,  334,  332,  336,  338,  340,  286,
 /*  8740 */   292,  298,  301,  304,  307,  295,  289,  316,  318,  326,
 /*  8750 */   320,  322,  324,  638,  638,  638,  638,  638,  638,  638,
 /*  8760 */   638,  313,  309,  638,  638,  542,  638,  638,  638,  638,
 /*  8770 */   344,  638,  638,  342,  330,  328,  334,  332,  336,  338,
 /*  8780 */   340,  286,  292,  298,  301,  304,  307,  295,  289,  316,
 /*  8790 */   318,  326,  320,  322,  324,  638,  638,  638,  638,  638,
 /*  8800 */   638,  638,  638,  313,  309,  638,  638,  545,  638,  638,
 /*  8810 */   638,  638,  344,  638,  638,  342,  330,  328,  334,  332,
 /*  8820 */   336,  338,  340,  286,  292,  298,  301,  304,  307,  295,
 /*  8830 */   289,  316,  318,  326,  320,  322,  324,  638,  293,  638,
 /*  8840 */   302,  305,  638,  638,  638,  313,  309,  638,  638,  638,
 /*  8850 */   638,  638,  354,  638,  638,  638,  638,  638,  592,  287,
 /*  8860 */   638,  638,  638,  638,  290,  364,  299,  369,  296,  389,
 /*  8870 */   284,  638,  379,  638,  638,  358,  638,  638,  638,  638,
 /*  8880 */   638,  638,  638,  638,  638,  396,  638,  638,  638,  638,
 /*  8890 */   378,  409,  348,  638,  344,  638,  375,  342,  330,  328,
 /*  8900 */   334,  332,  336,  338,  340,  286,  292,  298,  301,  304,
 /*  8910 */   307,  295,  289,  316,  318,  326,  320,  322,  324,  638,
 /*  8920 */   638,  638,  638,  638,  638,  638,  638,  313,  309,  603,
 /*  8930 */   638,  638,  638,  638,  638,  638,  638,  372,  638,  638,
 /*  8940 */   599,  638,  638,  638,  638,  371,  373,  374,  376,  377,
 /*  8950 */   638,  638,  638,  638,  344,  638,  638,  342,  330,  328,
 /*  8960 */   334,  332,  336,  338,  340,  286,  292,  298,  301,  304,
 /*  8970 */   307,  295,  289,  316,  318,  326,  320,  322,  324,  638,
 /*  8980 */   638,  293,  638,  302,  305,  638,  638,  313,  309,  660,
 /*  8990 */   638,  638,  638,  638,  638,  354,  638,  638,  638,  638,
 /*  9000 */   605,  638,  287,  638,  638,  638,  638,  290,  364,  299,
 /*  9010 */   369,  296,  389,  284,  638,  379,  638,  293,  358,  302,
 /*  9020 */   305,  638,  638,  638,  638,  638,  638,  638,  396,  638,
 /*  9030 */   638,  354,  638,  378,  409,  348,  638,  638,  287,  375,
 /*  9040 */   638,  638,  638,  290,  364,  299,  369,  296,  389,  284,
 /*  9050 */   128,  379,  638,  638,  358,  638,  638,  638,  638,  638,
 /*  9060 */   638,  638,  638,  638,  396,  638,  638,  638,  638,  378,
 /*  9070 */   409,  348,  638,  113,  638,  375,  638,  638,  638,  638,
 /*  9080 */   372,  638,  638,  638,  638,  638,  638,  638,  371,  373,
 /*  9090 */   374,  376,  377,  638,  638,  638,  638,   59,   60,   61,
 /*  9100 */    62,   63,   64,   65,   66,   67,   68,   69,   70,   71,
 /*  9110 */    72,  638,  638,  638,  638,  638,  372,  638,  638,  638,
 /*  9120 */   638,  638,  638,  638,  371,  373,  374,  376,  377,  638,
 /*  9130 */   638,  638,  638,  344,  638,  638,  342,  330,  328,  334,
 /*  9140 */   332,  336,  338,  340,  286,  292,  298,  301,  304,  307,
 /*  9150 */   295,  289,  316,  318,  326,  320,  322,  324,  293,  638,
 /*  9160 */   302,  305,  638,  638,  638,  638,  313,  309,  638,  638,
 /*  9170 */   638,  638,  354,  638,  638,  638,  638,  638,   97,  287,
 /*  9180 */   638,  100,  638,  638,  290,  364,  299,  369,  296,  389,
 /*  9190 */   284,  638,  379,  638,  638,  381,   96,  638,  638,  638,
 /*  9200 */   638,  638,   90,  638,  101,  396,  638,  638,  638,  638,
 /*  9210 */   384,  409,  348,  638,  638,  638,  375,  342,  330,  328,
 /*  9220 */   334,  332,  336,  338,  340,  286,  292,  298,  301,  304,
 /*  9230 */   307,  295,  289,  316,  318,  326,  320,  322,  324,  128,
 /*  9240 */   638,  638,  638,   99,  638,  638,  638,  313,  309,  638,
 /*  9250 */   638,   98,   87,   89,   92,   91,  638,  383,   39,  638,
 /*  9260 */   638,  638,   83,  638,  638,  382,  373,  374,  376,  377,
 /*  9270 */   638,  638,  638,  638,  638,  638,  112,  638,  638,  638,
 /*  9280 */   638,  638,  638,  638,  128,  638,   59,   60,   61,   62,
 /*  9290 */    63,   64,   65,   66,   67,   68,   69,   70,   71,   72,
 /*  9300 */   638,  638,  638,  158,  638,  638,  638,   83,  638,  638,
 /*  9310 */   638,  638,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  9320 */   638,  112,  638,  638,  638,  638,  638,  638,  638,  128,
 /*  9330 */   638,   59,   60,   61,   62,   63,   64,   65,   66,   67,
 /*  9340 */    68,   69,   70,   71,   72,  638,  638,  638,  213,  638,
 /*  9350 */   638,  638,   83,  638,  638,  638,  638,  638,  638,  638,
 /*  9360 */   638,  638,  638,  638,  638,  638,  112,  638,  638,  638,
 /*  9370 */   638,  638,  638,  638,  128,  638,   59,   60,   61,   62,
 /*  9380 */    63,   64,   65,   66,   67,   68,   69,   70,   71,   72,
 /*  9390 */   638,  638,  638,  623,  638,  638,  638,   83,  638,  638,
 /*  9400 */   638,  638,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  9410 */   638,  112,  638,  638,  638,  638,  638,  638,  638,  128,
 /*  9420 */   638,   59,   60,   61,   62,   63,   64,   65,   66,   67,
 /*  9430 */    68,   69,   70,   71,   72,  495,  449,  638,  751,  638,
 /*  9440 */   638,  638,   83,  638,  638,  638,  638,  638,  638,  638,
 /*  9450 */   638,  439,  638,  638,  638,  638,  112,  638,  638,  638,
 /*  9460 */   638,  638,  638,  638,  638,  638,   59,   60,   61,   62,
 /*  9470 */    63,   64,   65,   66,   67,   68,   69,   70,   71,   72,
 /*  9480 */   638,  638,  638,  638,  638,  638,  638,  638,  638,  638,
 /*  9490 */   638,  638,  638,  638,  440,  441,  442,  443,  444,  445,
 /*  9500 */   638,  480,  506,  507,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */     7,  144,    9,   10,  121,  122,  123,  124,  125,  126,
 /*    10 */   127,  128,  129,  130,   21,    0,  151,  156,  157,  158,
 /*    20 */   159,   28,  157,  158,   55,  160,   33,   34,   35,   36,
 /*    30 */    37,   38,   39,  195,   41,  197,  198,   44,   45,    1,
 /*    40 */     2,    3,    4,    5,   30,   31,   32,   54,   55,   56,
 /*    50 */   157,  158,   59,   60,   61,   41,   42,   88,   65,   90,
 /*    60 */   144,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*    70 */    77,   78,  215,  216,  217,   82,   83,   84,  157,  158,
 /*    80 */    87,  160,   45,    6,   91,   92,   93,   94,   55,   96,
 /*    90 */    52,   53,   99,   55,   56,  195,   58,  197,  198,  106,
 /*   100 */    62,   63,   61,  110,  111,  112,  113,  114,  115,  116,
 /*   110 */   117,  118,  119,    7,   42,    9,   10,   45,    6,   43,
 /*   120 */    44,   88,   46,   90,   48,  192,   50,   21,   52,   53,
 /*   130 */    89,   54,   56,  217,   28,  202,  203,  204,  205,   33,
 /*   140 */    34,   35,   36,   37,   38,   39,  213,   41,  107,   22,
 /*   150 */    44,   45,    1,    2,    3,    4,    5,   45,   44,   49,
 /*   160 */    54,   55,   56,  151,   54,   59,   60,   61,   41,  157,
 /*   170 */   158,   65,  160,   59,   68,   69,   70,   71,   72,   73,
 /*   180 */    74,   75,   76,   77,   78,  143,  126,  126,   82,   83,
 /*   190 */    84,  131,  150,   87,   44,  119,  154,   91,   92,   93,
 /*   200 */    94,   44,   96,   52,   53,   99,   55,   56,  195,   58,
 /*   210 */   197,  198,  106,   62,   63,   47,  110,  111,  112,  113,
 /*   220 */   114,  115,  116,  117,  118,  119,    7,    6,    9,   10,
 /*   230 */   135,   39,   89,  138,  139,  140,   44,  142,  143,  151,
 /*   240 */    21,   85,   86,  148,  149,  157,  158,   28,  160,  154,
 /*   250 */   107,    6,   33,   34,   35,   36,   37,   38,   39,   42,
 /*   260 */    41,   40,   45,   44,   45,    1,    2,    3,    4,    5,
 /*   270 */    44,   54,   39,   54,   55,   56,  151,   44,   59,   60,
 /*   280 */    61,   44,  157,  158,   65,  160,  132,   68,   69,   70,
 /*   290 */    71,   72,   73,   74,   75,   76,   77,   78,  151,   44,
 /*   300 */    55,   82,   83,   84,  157,  158,   87,  160,  184,  185,
 /*   310 */    91,   92,   93,   94,   59,   96,   52,   53,   99,   55,
 /*   320 */    56,   42,   58,    6,   45,  106,   62,   63,    6,  110,
 /*   330 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*   340 */   143,    9,   10,   27,   28,   29,   30,   31,   32,   42,
 /*   350 */    44,  154,   45,   21,   49,   54,   51,   41,   42,   54,
 /*   360 */    28,   54,   40,   44,    6,   33,   34,   35,   36,   37,
 /*   370 */    38,   39,   14,   41,   55,   56,   44,   45,    1,    2,
 /*   380 */     3,    4,    5,  143,   67,  132,   54,   55,   56,  149,
 /*   390 */    55,   59,   60,   61,  154,   42,   42,   65,   45,   45,
 /*   400 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*   410 */    78,  134,   54,  136,   82,   83,   84,   51,   45,   87,
 /*   420 */    54,   85,   86,   91,   92,   93,   94,   54,   96,   52,
 /*   430 */    53,   99,   55,   56,  154,   58,  186,  187,  106,   62,
 /*   440 */    63,   61,  110,  111,  112,  113,  114,  115,  116,  117,
 /*   450 */   118,  119,    7,   44,    9,   10,   61,  124,  125,  126,
 /*   460 */   127,  128,  129,  130,   44,  153,   21,  155,  156,  157,
 /*   470 */   158,  159,    6,   28,   54,   41,   42,   61,   33,   34,
 /*   480 */    35,   36,   37,   38,   39,   42,   41,  107,   45,   44,
 /*   490 */    45,    1,    2,    3,    4,    5,   44,   54,    6,   54,
 /*   500 */    55,   56,  107,   14,   59,   60,   61,   49,    6,   51,
 /*   510 */    65,   59,   54,   68,   69,   70,   71,   72,   73,   74,
 /*   520 */    75,   76,   77,   78,   44,  188,  189,   82,   83,   84,
 /*   530 */   190,  191,   87,   67,   54,    6,   91,   92,   93,   94,
 /*   540 */    44,   96,   52,   53,   99,   55,   56,   45,   58,   44,
 /*   550 */    54,  106,   62,   63,   61,  110,  111,  112,  113,  114,
 /*   560 */   115,  116,  117,  118,  119,    7,   45,    9,   10,   40,
 /*   570 */   137,  141,  139,  143,  141,    6,  143,   44,  148,   21,
 /*   580 */   150,  148,   89,  150,  154,  195,   28,  154,  198,   97,
 /*   590 */     6,   33,   34,   35,   36,   37,   38,   39,   42,   41,
 /*   600 */   107,   45,   44,   45,    1,    2,    3,    4,    5,   40,
 /*   610 */    54,  106,   54,   55,   56,  193,  194,   59,   60,   61,
 /*   620 */    49,   39,   51,   65,   40,   54,   68,   69,   70,   71,
 /*   630 */    72,   73,   74,   75,   76,   77,   78,   44,   44,  106,
 /*   640 */    82,   83,   84,  195,    6,   87,  198,   54,   54,   91,
 /*   650 */    92,   93,   94,    6,   96,   52,   53,   99,   55,   56,
 /*   660 */   195,   58,    6,  198,  106,   62,   63,   44,  110,  111,
 /*   670 */   112,  113,  114,  115,  116,  117,  118,  119,    7,   56,
 /*   680 */     9,   10,  143,   45,    6,  139,  140,   40,  142,  143,
 /*   690 */   206,  207,   21,  154,  148,  149,    6,  206,  207,   28,
 /*   700 */   154,   45,   45,  144,   33,   34,   35,   36,   37,   38,
 /*   710 */    39,   54,   41,    6,   57,   44,   45,    1,    2,    3,
 /*   720 */     4,    5,  134,   45,  136,   54,   55,   56,  206,  207,
 /*   730 */    59,   60,   61,  206,  207,   45,   65,  206,  207,   68,
 /*   740 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   750 */   206,  207,   45,   82,   83,   84,  206,  207,   87,  206,
 /*   760 */   207,   67,   91,   92,   93,   94,   44,   96,   52,   53,
 /*   770 */    99,   55,   56,  214,  215,  216,  217,  106,   62,   63,
 /*   780 */    22,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*   790 */   119,    7,   45,    9,   10,  153,  192,  155,  156,  157,
 /*   800 */   158,  159,    6,    6,  144,   21,  202,  203,  204,  205,
 /*   810 */    44,   57,   28,  209,  210,   45,  107,   33,   34,   35,
 /*   820 */    36,   37,   38,   39,   54,   41,    6,   57,   44,   45,
 /*   830 */     1,    2,    3,    4,    5,  206,  207,   45,   54,   55,
 /*   840 */    56,   45,   45,   59,   60,   61,   54,    6,    6,   65,
 /*   850 */   206,  207,   68,   69,   70,   71,   72,   73,   74,   75,
 /*   860 */    76,   77,   78,   85,   86,   45,   82,   83,   84,  206,
 /*   870 */   207,   87,  106,  184,  185,   91,   92,   93,   94,   45,
 /*   880 */    96,   52,   53,   99,   55,   56,   45,   45,   54,    6,
 /*   890 */   106,   62,   63,   89,  110,  111,  112,  113,  114,  115,
 /*   900 */   116,  117,  118,  119,    7,    6,    9,   10,  153,  192,
 /*   910 */   155,  156,  157,  158,  159,    6,    6,   59,   21,  202,
 /*   920 */   203,  204,  205,  146,  147,   28,  209,  210,   45,    6,
 /*   930 */    33,   34,   35,   36,   37,   38,   39,   51,   41,   40,
 /*   940 */    54,   44,   45,    1,    2,    3,    4,    5,   85,   86,
 /*   950 */    40,   54,   55,   56,   45,   89,   59,   60,   61,  134,
 /*   960 */    89,  136,   65,   40,   57,   68,   69,   70,   71,   72,
 /*   970 */    73,   74,   75,   76,   77,   78,   45,   45,   45,   82,
 /*   980 */    83,   84,  144,   57,   87,   54,   54,   54,   91,   92,
 /*   990 */    93,   94,  144,   96,   52,   53,   99,   55,   56,  134,
 /*  1000 */    44,  136,    6,  106,   62,   63,   57,  110,  111,  112,
 /*  1010 */   113,  114,  115,  116,  117,  118,  119,    7,    6,    9,
 /*  1020 */    10,  153,  192,  155,  156,  157,  158,  159,    6,    6,
 /*  1030 */   143,   21,  202,  203,  204,  205,   40,  144,   28,  209,
 /*  1040 */   210,  154,    6,   33,   34,   35,   36,   37,   38,   39,
 /*  1050 */    44,   41,   40,   44,   44,   45,    1,    2,    3,    4,
 /*  1060 */     5,   57,   40,   40,   54,   55,   56,   22,  144,   59,
 /*  1070 */    60,   61,  134,   57,  136,   65,   40,   44,   68,   69,
 /*  1080 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  144,
 /*  1090 */   144,   57,   82,   83,   84,   44,   57,   87,  144,   61,
 /*  1100 */    57,   91,   92,   93,   94,   44,   96,   52,   53,   99,
 /*  1110 */    55,   56,   45,   45,  144,    6,  106,   62,   63,   45,
 /*  1120 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  1130 */     7,  144,    9,   10,  153,  192,  155,  156,  157,  158,
 /*  1140 */   159,   57,   45,   57,   21,  202,  203,  204,  205,   40,
 /*  1150 */    45,   28,  209,  210,   44,  144,   33,   34,   35,   36,
 /*  1160 */    37,   38,   39,   57,   41,   45,   44,   44,   45,    1,
 /*  1170 */     2,    3,    4,    5,  144,  133,   44,   54,   55,   56,
 /*  1180 */   133,  133,   59,   60,   61,   55,   44,   61,   65,   45,
 /*  1190 */    45,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  1200 */    77,   78,   54,  185,  183,   82,   83,   84,   54,   54,
 /*  1210 */    87,  187,   44,  192,   91,   92,   93,   94,   89,   96,
 /*  1220 */    52,   53,   99,  202,  203,  204,  205,  144,   60,  106,
 /*  1230 */    62,   63,   54,  110,  111,  112,  113,  114,  115,  116,
 /*  1240 */   117,  118,  119,    7,   54,    9,   10,  153,  192,  155,
 /*  1250 */   156,  157,  158,  159,   54,   92,   45,   21,  202,  203,
 /*  1260 */   204,  205,   61,   40,   28,  209,  210,   22,   44,   33,
 /*  1270 */    34,   35,   36,   37,   38,   39,   61,   41,   44,    6,
 /*  1280 */    44,   45,    1,    2,    3,    4,    5,   89,   89,   61,
 /*  1290 */    54,   55,   56,   89,   44,   59,   60,   61,   55,   61,
 /*  1300 */    54,   65,   54,   95,   68,   69,   70,   71,   72,   73,
 /*  1310 */    74,   75,   76,   77,   78,   44,  189,   57,   82,   83,
 /*  1320 */    84,   44,  194,   87,   67,   44,  192,   91,   92,   93,
 /*  1330 */    94,  195,   96,   52,   53,   99,  202,  203,  204,  205,
 /*  1340 */    55,   60,  106,   62,   63,  195,  110,  111,  112,  113,
 /*  1350 */   114,  115,  116,  117,  118,  119,    7,  195,    9,   10,
 /*  1360 */   100,  101,  102,  103,  104,  105,   55,  195,   44,  153,
 /*  1370 */    21,  155,  156,  157,  158,  159,  195,   28,  195,  195,
 /*  1380 */   195,   55,   33,   34,   35,   36,   37,   38,   39,  195,
 /*  1390 */    41,   55,  195,   44,   45,    1,    2,    3,    4,    5,
 /*  1400 */    45,   45,   45,   54,   55,   56,   45,   44,   59,   60,
 /*  1410 */    61,   57,  207,  144,   65,   44,   97,   68,   69,   70,
 /*  1420 */    71,   72,   73,   74,   75,   76,   77,   78,   44,   54,
 /*  1430 */   191,   82,   83,   84,   54,   61,   87,   44,   55,   61,
 /*  1440 */    91,   92,   93,   94,   44,   96,   52,   53,   99,   55,
 /*  1450 */    56,   55,   61,   89,   55,  106,   62,   63,   61,  110,
 /*  1460 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  1470 */    55,    9,   10,   61,   92,   45,  192,   89,   54,   54,
 /*  1480 */    54,   61,   55,   21,  200,  201,  202,  203,  204,  205,
 /*  1490 */    28,   54,   44,  145,  144,   33,   34,   35,   36,   37,
 /*  1500 */    38,   39,   45,   41,  145,   45,   44,   45,    1,    2,
 /*  1510 */     3,    4,    5,   45,  147,   44,   54,   55,   56,   45,
 /*  1520 */   144,   59,   60,   61,  145,   45,  145,   65,   45,   50,
 /*  1530 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  1540 */    78,  136,  133,   44,   82,   83,   84,  133,   44,   87,
 /*  1550 */   133,  133,  133,   91,   92,   93,   94,   44,   96,   52,
 /*  1560 */    53,   99,   55,   56,   50,  133,   44,  133,  106,   62,
 /*  1570 */    63,   54,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  1580 */   118,  119,    7,  133,    9,   10,  153,  192,  155,  156,
 /*  1590 */   157,  158,  159,  218,  218,  218,   21,  202,  203,  204,
 /*  1600 */   205,  218,  218,   28,  209,  210,  218,  218,   33,   34,
 /*  1610 */    35,   36,   37,   38,   39,  218,   41,  218,  218,   44,
 /*  1620 */    45,    1,    2,    3,    4,    5,  218,  218,  218,   54,
 /*  1630 */    55,   56,  218,  218,   59,   60,   61,  218,  218,  218,
 /*  1640 */    65,  218,  218,   68,   69,   70,   71,   72,   73,   74,
 /*  1650 */    75,   76,   77,   78,  218,  218,  218,   82,   83,   84,
 /*  1660 */   218,  218,   87,  218,  218,  218,   91,   92,   93,   94,
 /*  1670 */   218,   96,   52,   53,   99,   55,   56,  218,  218,  218,
 /*  1680 */   218,  106,   62,   63,  218,  110,  111,  112,  113,  114,
 /*  1690 */   115,  116,  117,  118,  119,    7,  218,    9,   10,  218,
 /*  1700 */   192,  218,  218,  218,  218,  218,  218,  218,  218,   21,
 /*  1710 */   202,  203,  204,  205,  218,  218,   28,  209,  210,  218,
 /*  1720 */   218,   33,   34,   35,   36,   37,   38,   39,  218,   41,
 /*  1730 */   218,  218,   44,   45,    1,    2,    3,    4,    5,  218,
 /*  1740 */   218,  218,   54,   55,   56,  218,  218,   59,   60,   61,
 /*  1750 */   218,  218,  218,   65,  218,  218,   68,   69,   70,   71,
 /*  1760 */    72,   73,   74,   75,   76,   77,   78,  218,  218,  218,
 /*  1770 */    82,   83,   84,  218,  218,   87,  218,  218,  218,   91,
 /*  1780 */    92,   93,   94,  218,   96,   52,   53,   99,   55,   56,
 /*  1790 */   218,  218,  218,  218,  106,   62,   63,  218,  110,  111,
 /*  1800 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  218,
 /*  1810 */     9,   10,  218,  192,  218,  218,  218,  218,  218,  218,
 /*  1820 */   218,  218,   21,  202,  203,  204,  205,  218,  218,   28,
 /*  1830 */   209,  210,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  1840 */    39,  218,   41,  218,  218,   44,   45,    1,    2,    3,
 /*  1850 */     4,    5,  218,  218,  218,   54,   55,   56,  218,  218,
 /*  1860 */    59,   60,   61,  218,  218,  218,   65,  218,  218,   68,
 /*  1870 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  1880 */   218,  218,  183,   82,   83,   84,  218,  218,   87,  218,
 /*  1890 */   218,  192,   91,   92,   93,   94,  218,   96,   52,   53,
 /*  1900 */    99,  202,  203,  204,  205,  218,   60,  106,   62,   63,
 /*  1910 */   218,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  1920 */   119,    7,  218,    9,   10,  218,  192,  218,  218,  218,
 /*  1930 */   218,  218,  218,  218,  218,   21,  202,  203,  204,  205,
 /*  1940 */   218,  218,   28,  209,  210,  218,  218,   33,   34,   35,
 /*  1950 */    36,   37,   38,   39,  218,   41,  218,  218,   44,   45,
 /*  1960 */     1,    2,    3,    4,    5,  218,  218,  218,   54,   55,
 /*  1970 */    56,  218,  218,   59,   60,   61,  218,  218,  218,   65,
 /*  1980 */   218,  218,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  1990 */    76,   77,   78,  218,  218,  183,   82,   83,   84,  218,
 /*  2000 */   218,   87,  218,  218,  192,   91,   92,   93,   94,  218,
 /*  2010 */    96,   52,   53,   99,  202,  203,  204,  205,  218,   60,
 /*  2020 */   106,   62,   63,  218,  110,  111,  112,  113,  114,  115,
 /*  2030 */   116,  117,  118,  119,    7,  218,    9,   10,  218,  192,
 /*  2040 */   218,  218,  218,  218,  140,  218,  142,  143,   21,  202,
 /*  2050 */   203,  204,  205,  149,  218,   28,  209,  210,  154,  218,
 /*  2060 */    33,   34,   35,   36,   37,   38,   39,  218,   41,  218,
 /*  2070 */   218,   44,   45,    1,    2,    3,    4,    5,  218,  218,
 /*  2080 */   218,   54,   55,   56,  218,  218,   59,   60,   61,  218,
 /*  2090 */   218,  218,   65,  218,  218,   68,   69,   70,   71,   72,
 /*  2100 */    73,   74,   75,   76,   77,   78,  218,  218,  218,   82,
 /*  2110 */    83,   84,  218,  218,   87,  218,  218,  157,   91,   92,
 /*  2120 */    93,   94,  218,   96,   52,   53,   99,  218,  218,  218,
 /*  2130 */    58,  218,  218,  106,   62,   63,  218,  110,  111,  112,
 /*  2140 */   113,  114,  115,  116,  117,  118,  119,    7,  218,    9,
 /*  2150 */    10,  218,  192,  218,  218,  218,  218,  218,  218,  218,
 /*  2160 */   218,   21,  202,  203,  204,  205,  218,  218,   28,  218,
 /*  2170 */   218,  218,  218,   33,   34,   35,   36,   37,   38,   39,
 /*  2180 */   218,   41,  218,  218,   44,   45,    1,    2,    3,    4,
 /*  2190 */     5,  218,  218,  218,   54,   55,   56,  218,  218,   59,
 /*  2200 */    60,   61,  218,  218,  218,   65,  218,  218,   68,   69,
 /*  2210 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  218,
 /*  2220 */   218,  183,   82,   83,   84,  218,  218,   87,  218,  218,
 /*  2230 */   192,   91,   92,   93,   94,  218,   96,   52,   53,   99,
 /*  2240 */   202,  203,  204,  205,  218,   60,  106,   62,   63,  218,
 /*  2250 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  2260 */     7,  218,    9,   10,  218,  192,  218,  218,  218,  218,
 /*  2270 */   218,  218,  218,  218,   21,  202,  203,  204,  205,  218,
 /*  2280 */   218,   28,  218,  210,  218,  218,   33,   34,   35,   36,
 /*  2290 */    37,   38,   39,  218,   41,  218,  218,   44,   45,    1,
 /*  2300 */     2,    3,    4,    5,  218,  218,  218,   54,   55,   56,
 /*  2310 */   218,  218,   59,   60,   61,  218,  218,  218,   65,  218,
 /*  2320 */   218,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  2330 */    77,   78,  218,  218,  183,   82,   83,   84,  218,  218,
 /*  2340 */    87,  218,  218,  192,   91,   92,   93,   94,  218,   96,
 /*  2350 */    52,   53,   99,  202,  203,  204,  205,  218,   60,  106,
 /*  2360 */    62,   63,  218,  110,  111,  112,  113,  114,  115,  116,
 /*  2370 */   117,  118,  119,    7,  218,    9,   10,  218,  192,  218,
 /*  2380 */   218,  218,  196,  218,  218,  218,  218,   21,  202,  203,
 /*  2390 */   204,  205,  218,  218,   28,  218,  218,  218,  218,   33,
 /*  2400 */    34,   35,   36,   37,   38,   39,  218,   41,  218,  218,
 /*  2410 */    44,   45,    1,    2,    3,    4,    5,  218,  218,  218,
 /*  2420 */    54,   55,   56,  218,  218,   59,   60,   61,  218,  218,
 /*  2430 */   218,   65,  218,  218,   68,   69,   70,   71,   72,   73,
 /*  2440 */    74,   75,   76,   77,   78,  218,  218,  183,   82,   83,
 /*  2450 */    84,  218,  218,   87,  218,  218,  192,   91,   92,   93,
 /*  2460 */    94,  218,   96,   52,   53,   99,  202,  203,  204,  205,
 /*  2470 */   218,  218,  106,   62,   63,  218,  110,  111,  112,  113,
 /*  2480 */   114,  115,  116,  117,  118,  119,    7,  218,    9,   10,
 /*  2490 */   218,  192,  218,  218,  218,  196,  218,  218,  218,  218,
 /*  2500 */    21,  202,  203,  204,  205,  218,  218,   28,  218,  218,
 /*  2510 */   218,  218,   33,   34,   35,   36,   37,   38,   39,  218,
 /*  2520 */    41,  218,  218,   44,   45,  218,  138,  218,  140,  218,
 /*  2530 */   142,  143,  218,   54,   55,   56,  148,  149,   59,   60,
 /*  2540 */    61,  218,  154,  218,   65,  218,  218,   68,   69,   70,
 /*  2550 */    71,   72,   73,   74,   75,   76,   77,   78,  218,  218,
 /*  2560 */   218,   82,   83,   84,  218,  218,   87,  218,  218,  192,
 /*  2570 */    91,   92,   93,   94,  218,   96,  199,  218,   99,  202,
 /*  2580 */   203,  204,  205,  218,  218,  106,  218,  218,  218,  110,
 /*  2590 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  2600 */   218,    9,   10,  218,  192,  218,  218,  218,  196,  218,
 /*  2610 */   218,  218,  218,   21,  202,  203,  204,  205,  218,  218,
 /*  2620 */    28,  218,  218,  218,  218,   33,   34,   35,   36,   37,
 /*  2630 */    38,   39,  192,   41,  218,  218,   44,   45,  218,  218,
 /*  2640 */   218,  201,  202,  203,  204,  205,   54,   55,   56,  218,
 /*  2650 */   218,   59,   60,   61,  218,  218,  218,   65,  218,  218,
 /*  2660 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  2670 */    78,  218,  218,  218,   82,   83,   84,  218,  218,   87,
 /*  2680 */   218,  218,  192,   91,   92,   93,   94,  218,   96,  218,
 /*  2690 */   218,   99,  202,  203,  204,  205,  218,  218,  106,  218,
 /*  2700 */   218,  218,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  2710 */   118,  119,    7,  218,    9,   10,  218,  192,  218,  218,
 /*  2720 */   218,  196,  218,  218,  192,  218,   21,  202,  203,  204,
 /*  2730 */   205,  218,  218,   28,  202,  203,  204,  205,   33,   34,
 /*  2740 */    35,   36,   37,   38,   39,  192,   41,  218,  218,   44,
 /*  2750 */    45,  218,  218,  218,  218,  202,  203,  204,  205,   54,
 /*  2760 */    55,   56,  218,  218,   59,   60,   61,  218,  218,  218,
 /*  2770 */    65,  218,  218,   68,   69,   70,   71,   72,   73,   74,
 /*  2780 */    75,   76,   77,   78,  218,  218,  218,   82,   83,   84,
 /*  2790 */   218,  218,   87,  218,  218,  192,   91,   92,   93,   94,
 /*  2800 */   218,   96,  218,  218,   99,  202,  203,  204,  205,  218,
 /*  2810 */   218,  106,  218,  218,  218,  110,  111,  112,  113,  114,
 /*  2820 */   115,  116,  117,  118,  119,    7,  218,    9,   10,  218,
 /*  2830 */   192,  218,  218,  218,  196,  218,  218,  192,  218,   21,
 /*  2840 */   202,  203,  204,  205,  218,  218,   28,  202,  203,  204,
 /*  2850 */   205,   33,   34,   35,   36,   37,   38,   39,  192,   41,
 /*  2860 */   218,  218,   44,   45,  218,  218,  218,  218,  202,  203,
 /*  2870 */   204,  205,   54,   55,   56,  218,  218,   59,   60,   61,
 /*  2880 */   218,  218,  218,   65,  218,  218,   68,   69,   70,   71,
 /*  2890 */    72,   73,   74,   75,   76,   77,   78,  218,  218,  218,
 /*  2900 */    82,   83,   84,  218,  218,   87,  218,  218,  192,   91,
 /*  2910 */    92,   93,   94,  218,   96,  218,  218,   99,  202,  203,
 /*  2920 */   204,  205,  218,  218,  106,  218,  218,  218,  110,  111,
 /*  2930 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  218,
 /*  2940 */     9,   10,  218,  192,  218,  218,  218,  196,  218,  218,
 /*  2950 */   192,  218,   21,  202,  203,  204,  205,  218,  218,   28,
 /*  2960 */   202,  203,  204,  205,   33,   34,   35,   36,   37,   38,
 /*  2970 */    39,  192,   41,  218,  218,   44,   45,  218,  218,  218,
 /*  2980 */   218,  202,  203,  204,  205,   54,   55,   56,  218,  218,
 /*  2990 */    59,   60,   61,  218,  218,  218,   65,  218,  218,   68,
 /*  3000 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  3010 */   218,  218,  218,   82,   83,   84,  218,  218,   87,  218,
 /*  3020 */   218,  192,   91,   92,   93,   94,  218,   96,  218,  218,
 /*  3030 */    99,  202,  203,  204,  205,  218,  218,  106,  218,  218,
 /*  3040 */   218,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  3050 */   119,    7,  218,    9,   10,  218,  192,  218,  218,  218,
 /*  3060 */   196,  218,  218,  192,  218,   21,  202,  203,  204,  205,
 /*  3070 */   218,  218,   28,  202,  203,  204,  205,   33,   34,   35,
 /*  3080 */    36,   37,   38,   39,  192,   41,  218,  218,   44,   45,
 /*  3090 */   218,  218,  218,  218,  202,  203,  204,  205,   54,   55,
 /*  3100 */    56,  218,  218,   59,   60,   61,  218,  218,  218,   65,
 /*  3110 */   218,  218,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  3120 */    76,   77,   78,  218,  218,  218,   82,   83,   84,  218,
 /*  3130 */   218,   87,  218,  218,  192,   91,   92,   93,   94,  218,
 /*  3140 */    96,  218,  218,   99,  202,  203,  204,  205,  218,  218,
 /*  3150 */   106,  218,  218,  218,  110,  111,  112,  113,  114,  115,
 /*  3160 */   116,  117,  118,  119,    7,  218,    9,   10,  218,  192,
 /*  3170 */   218,  218,  218,  196,  218,  218,  192,  218,   21,  202,
 /*  3180 */   203,  204,  205,  218,  218,   28,  202,  203,  204,  205,
 /*  3190 */    33,   34,   35,   36,   37,   38,   39,  192,   41,  218,
 /*  3200 */   218,   44,   45,  218,  218,  218,  218,  202,  203,  204,
 /*  3210 */   205,   54,   55,   56,  218,  218,   59,   60,   61,  218,
 /*  3220 */   218,  218,   65,  218,  218,   68,   69,   70,   71,   72,
 /*  3230 */    73,   74,   75,   76,   77,   78,  218,  218,  218,   82,
 /*  3240 */    83,   84,  218,  218,   87,  218,  218,  192,   91,   92,
 /*  3250 */    93,   94,  218,   96,  218,  218,   99,  202,  203,  204,
 /*  3260 */   205,  218,  218,  106,  218,  218,  218,  110,  111,  112,
 /*  3270 */   113,  114,  115,  116,  117,  118,  119,    7,  218,    9,
 /*  3280 */    10,  218,  192,  218,  218,  218,  196,  218,  218,  192,
 /*  3290 */   218,   21,  202,  203,  204,  205,  218,  218,   28,  202,
 /*  3300 */   203,  204,  205,   33,   34,   35,   36,   37,   38,   39,
 /*  3310 */   192,   41,  218,  218,   44,   45,  218,  218,  218,  218,
 /*  3320 */   202,  203,  204,  205,   54,   55,   56,  218,  218,   59,
 /*  3330 */    60,   61,  218,  218,  218,   65,  218,  218,   68,   69,
 /*  3340 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  218,
 /*  3350 */   218,  218,   82,   83,   84,  218,  218,   87,  218,  218,
 /*  3360 */   192,   91,   92,   93,   94,  218,   96,  218,  218,   99,
 /*  3370 */   202,  203,  204,  205,  218,  218,  106,  218,  218,  218,
 /*  3380 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  3390 */     7,  218,    9,   10,  218,  192,  218,  218,  218,  196,
 /*  3400 */   218,  218,  192,  218,   21,  202,  203,  204,  205,  218,
 /*  3410 */   218,   28,  202,  203,  204,  205,   33,   34,   35,   36,
 /*  3420 */    37,   38,   39,  192,   41,  218,  218,   44,   45,  218,
 /*  3430 */   218,  218,  218,  202,  203,  204,  205,   54,   55,   56,
 /*  3440 */   218,  218,   59,   60,   61,  218,  218,  218,   65,  218,
 /*  3450 */   218,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  3460 */    77,   78,  218,  218,  218,   82,   83,   84,  218,  218,
 /*  3470 */    87,  218,  218,  192,   91,   92,   93,   94,  218,   96,
 /*  3480 */   218,  218,   99,  202,  203,  204,  205,  218,  218,  106,
 /*  3490 */   218,  218,  218,  110,  111,  112,  113,  114,  115,  116,
 /*  3500 */   117,  118,  119,    7,  218,    9,   10,  218,  192,  218,
 /*  3510 */   218,  218,  196,  218,  218,  192,  218,   21,  202,  203,
 /*  3520 */   204,  205,  218,  218,   28,  202,  203,  204,  205,   33,
 /*  3530 */    34,   35,   36,   37,   38,   39,  192,   41,  218,  218,
 /*  3540 */    44,   45,  218,  218,  218,  218,  202,  203,  204,  205,
 /*  3550 */    54,   55,   56,  218,  218,   59,   60,   61,  218,  218,
 /*  3560 */   218,   65,  218,  218,   68,   69,   70,   71,   72,   73,
 /*  3570 */    74,   75,   76,   77,   78,  218,  218,  218,   82,   83,
 /*  3580 */    84,  218,  218,   87,  218,  218,  192,   91,   92,   93,
 /*  3590 */    94,  218,   96,  218,  218,   99,  202,  203,  204,  205,
 /*  3600 */   218,  218,  106,  218,  218,  218,  110,  111,  112,  113,
 /*  3610 */   114,  115,  116,  117,  118,  119,    7,  218,    9,   10,
 /*  3620 */   218,  192,  218,  218,  218,  196,  218,  218,  192,  218,
 /*  3630 */    21,  202,  203,  204,  205,  218,  218,   28,  202,  203,
 /*  3640 */   204,  205,   33,   34,   35,   36,   37,   38,   39,  192,
 /*  3650 */    41,  218,  218,   44,   45,  218,  218,  218,  218,  202,
 /*  3660 */   203,  204,  205,   54,   55,   56,  218,  218,   59,   60,
 /*  3670 */    61,  218,  218,  218,   65,  218,  218,   68,   69,   70,
 /*  3680 */    71,   72,   73,   74,   75,   76,   77,   78,  218,  218,
 /*  3690 */   218,   82,   83,   84,  218,  218,   87,  218,  218,  192,
 /*  3700 */    91,   92,   93,   94,  218,   96,  218,  218,   99,  202,
 /*  3710 */   203,  204,  205,  218,  218,  106,  218,  218,  218,  110,
 /*  3720 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  3730 */   218,    9,   10,  218,  192,  218,  218,  218,  196,  218,
 /*  3740 */   218,  192,  218,   21,  202,  203,  204,  205,  218,  218,
 /*  3750 */    28,  202,  203,  204,  205,   33,   34,   35,   36,   37,
 /*  3760 */    38,   39,  192,   41,  218,  218,   44,   45,  218,  218,
 /*  3770 */   218,  218,  202,  203,  204,  205,   54,   55,   56,  218,
 /*  3780 */   218,   59,   60,   61,  218,  218,  218,   65,  218,  218,
 /*  3790 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  3800 */    78,  218,  218,  218,   82,   83,   84,  218,  218,   87,
 /*  3810 */   218,  218,  192,   91,   92,   93,   94,  218,   96,  218,
 /*  3820 */   218,   99,  202,  203,  204,  205,  218,  218,  106,  218,
 /*  3830 */   218,  218,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  3840 */   118,  119,    7,  218,    9,   10,  218,  192,  218,  218,
 /*  3850 */   218,  196,  218,  218,  192,  218,   21,  202,  203,  204,
 /*  3860 */   205,  218,  218,   28,  202,  203,  204,  205,   33,   34,
 /*  3870 */    35,   36,   37,   38,   39,  192,   41,  218,  218,   44,
 /*  3880 */    45,  218,  218,  218,  218,  202,  203,  204,  205,   54,
 /*  3890 */    55,   56,  218,  218,   59,   60,   61,  218,  218,  218,
 /*  3900 */    65,  218,  218,   68,   69,   70,   71,   72,   73,   74,
 /*  3910 */    75,   76,   77,   78,  218,  218,  218,   82,   83,   84,
 /*  3920 */   218,  218,   87,  218,  218,  192,   91,   92,   93,   94,
 /*  3930 */   218,   96,  218,  218,   99,  202,  203,  204,  205,  218,
 /*  3940 */   218,  106,  218,  218,  218,  110,  111,  112,  113,  114,
 /*  3950 */   115,  116,  117,  118,  119,    7,  218,    9,   10,  218,
 /*  3960 */   192,  218,  218,  218,  196,  218,  218,  192,  218,   21,
 /*  3970 */   202,  203,  204,  205,  218,  218,   28,  202,  203,  204,
 /*  3980 */   205,   33,   34,   35,   36,   37,   38,   39,  192,   41,
 /*  3990 */   218,  218,   44,   45,  218,  218,  218,  218,  202,  203,
 /*  4000 */   204,  205,   54,   55,   56,  218,  218,   59,   60,   61,
 /*  4010 */   218,  218,  218,   65,  218,  218,   68,   69,   70,   71,
 /*  4020 */    72,   73,   74,   75,   76,   77,   78,  218,  218,  218,
 /*  4030 */    82,   83,   84,  218,  218,   87,  218,  218,  192,   91,
 /*  4040 */    92,   93,   94,  218,   96,  218,  218,   99,  202,  203,
 /*  4050 */   204,  205,  218,  218,  106,  218,  218,  218,  110,  111,
 /*  4060 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  218,
 /*  4070 */     9,   10,  218,  192,  218,  218,  218,  196,  218,  218,
 /*  4080 */   192,  218,   21,  202,  203,  204,  205,  218,  218,   28,
 /*  4090 */   202,  203,  204,  205,   33,   34,   35,   36,   37,   38,
 /*  4100 */    39,  192,   41,  218,  218,   44,   45,  218,  218,  218,
 /*  4110 */   218,  202,  203,  204,  205,   54,   55,   56,  218,  218,
 /*  4120 */    59,   60,   61,  218,  218,  218,   65,  218,  218,   68,
 /*  4130 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  4140 */   218,  218,  218,   82,   83,   84,  218,  218,   87,  218,
 /*  4150 */   218,  192,   91,   92,   93,   94,  218,   96,  218,  218,
 /*  4160 */    99,  202,  203,  204,  205,  218,  218,  106,  218,  218,
 /*  4170 */   218,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  4180 */   119,    7,  218,    9,   10,  218,  192,  218,  218,  218,
 /*  4190 */   218,  218,  218,  192,  218,   21,  202,  203,  204,  205,
 /*  4200 */   218,  218,   28,  202,  203,  204,  205,   33,   34,   35,
 /*  4210 */    36,   37,   38,   39,  192,   41,  218,  218,   44,   45,
 /*  4220 */   218,  218,  218,  218,  202,  203,  204,  205,   54,   55,
 /*  4230 */    56,  218,  218,   59,   60,   61,  218,  218,  218,   65,
 /*  4240 */   218,  218,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  4250 */    76,   77,   78,  218,  218,  218,   82,   83,   84,  218,
 /*  4260 */   218,   87,  218,  218,  192,   91,   92,   93,   94,  218,
 /*  4270 */    96,  218,  218,   99,  202,  203,  204,  205,  218,  218,
 /*  4280 */   106,  218,  218,  218,  110,  111,  112,  113,  114,  115,
 /*  4290 */   116,  117,  118,  119,    7,  218,    9,   10,  218,  192,
 /*  4300 */   218,  218,  218,  218,  218,  218,  192,  218,   21,  202,
 /*  4310 */   203,  204,  205,  218,  218,   28,  202,  203,  204,  205,
 /*  4320 */    33,   34,   35,   36,   37,   38,   39,  192,   41,  218,
 /*  4330 */   218,   44,   45,  218,  218,  218,  218,  202,  203,  204,
 /*  4340 */   205,   54,   55,   56,  218,  218,   59,   60,   61,  218,
 /*  4350 */   218,  218,   65,  218,  218,   68,   69,   70,   71,   72,
 /*  4360 */    73,   74,   75,   76,   77,   78,  218,  218,  218,   82,
 /*  4370 */    83,   84,  218,  218,   87,  218,  218,  218,   91,   92,
 /*  4380 */    93,   94,  218,   96,  218,  218,   99,  218,  218,  218,
 /*  4390 */   218,  218,  218,  106,  218,  218,  218,  110,  111,  112,
 /*  4400 */   113,  114,  115,  116,  117,  118,  119,    7,  140,    9,
 /*  4410 */    10,  143,  218,  218,  218,  218,  148,  149,  218,  218,
 /*  4420 */   218,   21,  154,  218,  218,  218,  218,  218,   28,  218,
 /*  4430 */   218,  218,  218,   33,   34,   35,   36,   37,   38,   39,
 /*  4440 */   218,   41,  218,  218,   44,   45,  218,  218,  218,  218,
 /*  4450 */   218,  218,  218,  218,   54,   55,   56,  218,  218,   59,
 /*  4460 */    60,   61,  218,  218,  218,   65,  218,  218,   68,   69,
 /*  4470 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  218,
 /*  4480 */   218,  218,   82,   83,   84,  218,  218,   87,  218,  218,
 /*  4490 */   218,   91,   92,   93,   94,  218,   96,  218,  218,   99,
 /*  4500 */   218,  218,  218,  218,  218,  218,  106,  218,  218,  218,
 /*  4510 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  4520 */     7,  218,    9,   10,  218,  218,  218,  218,  218,  218,
 /*  4530 */   218,  218,  218,  218,   21,  218,  218,  218,  218,  218,
 /*  4540 */   218,   28,  218,  218,  218,  218,   33,   34,   35,   36,
 /*  4550 */    37,   38,   39,  218,   41,  218,  218,   44,   45,  218,
 /*  4560 */   218,  218,  218,  218,  218,  218,  218,   54,   55,   56,
 /*  4570 */   218,  218,   59,   60,   61,  218,  218,  218,   65,  218,
 /*  4580 */   218,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  4590 */    77,   78,  218,  218,  218,   82,   83,   84,  218,  218,
 /*  4600 */    87,  218,  218,  218,   91,   92,   93,   94,  218,   96,
 /*  4610 */   218,  218,   99,  218,  218,  218,  218,  218,  218,  106,
 /*  4620 */   218,  218,  218,  110,  111,  112,  113,  114,  115,  116,
 /*  4630 */   117,  118,  119,    7,  218,    9,   10,  218,  218,  218,
 /*  4640 */   218,  218,  218,  218,  218,  218,  218,   21,  218,  218,
 /*  4650 */   218,  218,  218,  218,   28,  218,  218,  218,  218,   33,
 /*  4660 */    34,   35,   36,   37,   38,   39,  218,   41,  218,  218,
 /*  4670 */    44,   45,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  4680 */    54,   55,   56,  218,  218,   59,   60,   61,  218,  218,
 /*  4690 */   218,   65,  218,  218,   68,   69,   70,   71,   72,   73,
 /*  4700 */    74,   75,   76,   77,   78,  218,  218,  218,   82,   83,
 /*  4710 */    84,  218,  218,   87,  218,  218,  218,   91,   92,   93,
 /*  4720 */    94,  218,   96,  218,  218,   99,  218,  218,  218,  218,
 /*  4730 */   218,  218,  106,  218,  218,  218,  110,  111,  112,  113,
 /*  4740 */   114,  115,  116,  117,  118,  119,    7,  218,    9,   10,
 /*  4750 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  4760 */    21,  218,  218,  218,  218,  218,  218,   28,  218,  218,
 /*  4770 */   218,  218,   33,   34,   35,   36,   37,   38,   39,  218,
 /*  4780 */    41,  218,  218,   44,   45,  218,  218,  218,  218,  218,
 /*  4790 */   218,  218,  218,   54,   55,   56,  218,  218,   59,   60,
 /*  4800 */    61,  218,  218,  218,   65,  218,  218,   68,   69,   70,
 /*  4810 */    71,   72,   73,   74,   75,   76,   77,   78,  218,  218,
 /*  4820 */   218,   82,   83,   84,  218,  218,   87,  218,  218,  218,
 /*  4830 */    91,   92,   93,   94,  218,   96,  218,  218,   99,  218,
 /*  4840 */   218,  218,  218,  218,  218,  106,  218,  218,  218,  110,
 /*  4850 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  4860 */   218,    9,   10,  218,  218,  218,  218,  218,  218,  218,
 /*  4870 */   218,  218,  218,   21,  218,  218,  218,  218,  218,  218,
 /*  4880 */    28,  218,  218,  218,  218,   33,   34,   35,   36,   37,
 /*  4890 */    38,   39,  218,   41,  218,  218,   44,   45,  218,  218,
 /*  4900 */   218,  218,  218,  218,  218,  218,   54,   55,   56,  218,
 /*  4910 */   218,   59,   60,   61,  218,  218,  218,   65,  218,  218,
 /*  4920 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  4930 */    78,  218,  218,  218,   82,   83,   84,  218,  218,   87,
 /*  4940 */   218,  218,  218,   91,   92,   93,   94,  218,   96,  218,
 /*  4950 */   218,   99,  218,  218,  218,  218,  218,  218,  106,  218,
 /*  4960 */   218,  218,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  4970 */   118,  119,    7,  218,    9,   10,  218,  218,  218,  218,
 /*  4980 */   218,  218,  218,  218,  218,  218,   21,  218,  218,  218,
 /*  4990 */   218,  218,  218,   28,  218,  218,  218,  218,   33,   34,
 /*  5000 */    35,   36,   37,   38,   39,  218,   41,  218,  218,   44,
 /*  5010 */    45,  218,  218,  218,  218,  218,  218,  218,  218,   54,
 /*  5020 */    55,   56,  218,  218,   59,   60,   61,  218,  218,  218,
 /*  5030 */    65,  218,  218,   68,   69,   70,   71,   72,   73,   74,
 /*  5040 */    75,   76,   77,   78,  218,  218,  218,   82,   83,   84,
 /*  5050 */   218,  218,   87,  218,  218,  218,   91,   92,   93,   94,
 /*  5060 */   218,   96,  218,  218,   99,  218,  218,  218,  218,  218,
 /*  5070 */   218,  106,  218,  218,  218,  110,  111,  112,  113,  114,
 /*  5080 */   115,  116,  117,  118,  119,    7,  218,    9,   10,  218,
 /*  5090 */   218,  218,  218,  218,  218,  218,  218,  218,  218,   21,
 /*  5100 */   218,  218,  218,  218,  218,  218,   28,  218,  218,  218,
 /*  5110 */   218,   33,   34,   35,   36,   37,   38,   39,  218,   41,
 /*  5120 */   218,  218,   44,   45,  218,  218,  218,  218,  218,  218,
 /*  5130 */   218,  218,   54,   55,   56,  218,  218,   59,   60,   61,
 /*  5140 */   218,  218,  218,   65,  218,  218,   68,   69,   70,   71,
 /*  5150 */    72,   73,   74,   75,   76,   77,   78,  218,  218,  218,
 /*  5160 */    82,   83,   84,  218,  218,   87,  218,  218,  218,   91,
 /*  5170 */    92,   93,   94,  218,   96,  218,  218,   99,  218,  218,
 /*  5180 */   218,  218,  218,  218,  106,  218,  218,  218,  110,  111,
 /*  5190 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  218,
 /*  5200 */     9,   10,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  5210 */   218,  218,   21,  218,  218,  218,  218,  218,  218,   28,
 /*  5220 */   218,  218,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  5230 */    39,  218,   41,  218,  218,   44,   45,  218,  218,  218,
 /*  5240 */   218,  218,  218,  218,  218,   54,   55,   56,  218,  218,
 /*  5250 */    59,   60,   61,  218,  218,  218,   65,  218,  218,   68,
 /*  5260 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  5270 */   218,  218,  218,   82,   83,   84,  218,  218,   87,  218,
 /*  5280 */   218,  218,   91,   92,   93,   94,  218,   96,  218,  218,
 /*  5290 */    99,  218,  218,  218,  218,  218,  218,  106,  218,  218,
 /*  5300 */   218,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  5310 */   119,    7,  218,    9,   10,  218,  218,  218,  218,  218,
 /*  5320 */   218,  218,  218,  218,  218,   21,  218,  218,  218,  218,
 /*  5330 */   218,  218,   28,  218,  218,  218,  218,   33,   34,   35,
 /*  5340 */    36,   37,   38,   39,  218,   41,  218,  218,   44,   45,
 /*  5350 */   218,  218,  218,  218,  218,  218,  218,  218,   54,   55,
 /*  5360 */    56,  218,  218,   59,   60,   61,  218,  218,  218,   65,
 /*  5370 */   218,  218,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  5380 */    76,   77,   78,  218,  218,  218,   82,   83,   84,  218,
 /*  5390 */   218,   87,  218,  218,  218,   91,   92,   93,   94,  218,
 /*  5400 */    96,  218,  218,   99,  218,  218,  218,  218,  218,  218,
 /*  5410 */   106,  218,  218,  218,  110,  111,  112,  113,  114,  115,
 /*  5420 */   116,  117,  118,  119,    7,  218,    9,   10,  218,  218,
 /*  5430 */   218,  218,  218,  218,  218,  218,  218,  218,   21,  218,
 /*  5440 */   218,  218,  218,  218,  218,   28,  218,  218,  218,  218,
 /*  5450 */    33,   34,   35,   36,   37,   38,   39,  218,   41,  218,
 /*  5460 */   218,   44,   45,  218,  218,  218,  218,  218,  218,  218,
 /*  5470 */   218,   54,  218,   56,  218,  218,   59,   60,   61,  218,
 /*  5480 */   218,  218,   65,  218,  218,   68,   69,   70,   71,   72,
 /*  5490 */    73,   74,   75,   76,   77,   78,  218,  218,  218,   82,
 /*  5500 */    83,   84,  218,  218,   87,  218,  218,  218,   91,   92,
 /*  5510 */    93,   94,  218,   96,  218,  218,   99,  218,  218,  218,
 /*  5520 */   218,  218,  218,  106,  218,  218,  218,  110,  111,  112,
 /*  5530 */   113,  114,  115,  116,  117,  118,  119,    7,  218,    9,
 /*  5540 */    10,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  5550 */   218,   21,  218,  218,  218,  218,  218,  218,   28,  218,
 /*  5560 */   218,  218,  218,   33,   34,   35,   36,   37,   38,   39,
 /*  5570 */   218,   41,  218,  218,   44,  218,  218,  218,  218,  218,
 /*  5580 */   218,  218,  218,  218,   54,  218,  218,  218,  218,   59,
 /*  5590 */    60,   61,  218,  218,  218,   65,  218,  218,   68,   69,
 /*  5600 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*  5610 */    80,   81,  218,  218,  218,  218,  218,  129,  130,   19,
 /*  5620 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*  5630 */    30,   31,   32,  218,  218,  218,  106,  218,  218,  218,
 /*  5640 */   152,   41,   42,  218,  114,  115,  116,  117,  118,  161,
 /*  5650 */   162,  163,  164,  165,  166,  167,  168,  169,  170,  171,
 /*  5660 */   172,  173,  174,  175,  176,  177,  178,  179,  180,  181,
 /*  5670 */   182,  218,  218,  218,  218,  218,  218,  218,   41,  218,
 /*  5680 */   192,  129,  130,  218,  218,  218,  192,  218,  218,  218,
 /*  5690 */   202,  203,  204,  205,   57,  218,  202,  203,  204,  205,
 /*  5700 */   218,  218,  208,  218,  152,  211,  212,  213,  218,  218,
 /*  5710 */   218,  218,  218,  161,  162,  163,  164,  165,  166,  167,
 /*  5720 */   168,  169,  170,  171,  172,  173,  174,  175,  176,  177,
 /*  5730 */   178,  179,  180,  181,  182,  129,  130,  100,  101,  102,
 /*  5740 */   103,  104,  105,  218,  192,  108,  109,  218,  218,  218,
 /*  5750 */   218,  218,  218,  218,  202,  203,  204,  205,  152,  218,
 /*  5760 */   218,  218,  218,  218,  218,  218,  218,  161,  162,  163,
 /*  5770 */   164,  165,  166,  167,  168,  169,  170,  171,  172,  173,
 /*  5780 */   174,  175,  176,  177,  178,  179,  180,  181,  182,  129,
 /*  5790 */   130,  218,  218,  218,  218,  218,   41,  218,  192,   44,
 /*  5800 */   218,  218,  218,  218,  218,  218,  218,  218,  202,  203,
 /*  5810 */   204,  205,  152,  218,   59,  218,  218,  218,  218,  218,
 /*  5820 */    65,  161,  162,  163,  164,  165,  166,  167,  168,  169,
 /*  5830 */   170,  171,  172,  173,  174,  175,  176,  177,  178,  179,
 /*  5840 */   180,  181,  182,  129,  130,  218,  218,  218,  218,  218,
 /*  5850 */    41,  218,  192,   44,  218,  218,  218,  218,  218,  218,
 /*  5860 */   218,  106,  202,  203,  204,  205,  152,  218,   59,  114,
 /*  5870 */   115,  116,  117,  118,   65,  161,  162,  163,  164,  165,
 /*  5880 */   166,  167,  168,  169,  170,  171,  172,  173,  174,  175,
 /*  5890 */   176,  177,  178,  179,  180,  181,  182,  129,  130,  218,
 /*  5900 */   218,  218,  218,  218,  218,   41,  192,  218,  218,  218,
 /*  5910 */   218,  218,  218,  218,  218,  106,  202,  203,  204,  205,
 /*  5920 */   152,   57,  218,  114,  115,  116,  117,  118,  218,  161,
 /*  5930 */   162,  163,  164,  165,  166,  167,  168,  169,  170,  171,
 /*  5940 */   172,  173,  174,  175,  176,  177,  178,  179,  180,  181,
 /*  5950 */   182,  129,  130,  218,  218,  218,  218,  218,  218,   41,
 /*  5960 */   192,  218,  218,  218,  100,  101,  102,  103,  104,  105,
 /*  5970 */   202,  203,  204,  205,  152,   57,  218,  218,  218,  218,
 /*  5980 */   218,  218,  218,  161,  162,  163,  164,  165,  166,  167,
 /*  5990 */   168,  169,  170,  171,  172,  173,  174,  175,  176,  177,
 /*  6000 */   178,  179,  180,  181,  182,  129,  130,  218,  218,  218,
 /*  6010 */   218,  218,  218,  218,  192,  218,  218,  218,  100,  101,
 /*  6020 */   102,  103,  104,  105,  202,  203,  204,  205,  152,  218,
 /*  6030 */   218,  218,  218,  218,  218,  218,  218,  161,  162,  163,
 /*  6040 */   164,  165,  166,  167,  168,  169,  170,  171,  172,  173,
 /*  6050 */   174,  175,  176,  177,  178,  179,  180,  181,  182,  129,
 /*  6060 */   130,  218,  218,  218,  192,  218,  218,  218,  192,  218,
 /*  6070 */   218,  218,  218,  218,  202,  203,  204,  205,  202,  203,
 /*  6080 */   204,  205,  152,  211,  212,  213,  218,  218,  218,  218,
 /*  6090 */   218,  161,  162,  163,  164,  165,  166,  167,  168,  169,
 /*  6100 */   170,  171,  172,  173,  174,  175,  176,  177,  178,  179,
 /*  6110 */   180,  181,  182,  129,  130,  218,  218,  218,  218,  218,
 /*  6120 */   218,   41,  192,  218,  218,  218,  218,  218,  218,  218,
 /*  6130 */   218,  218,  202,  203,  204,  205,  152,   57,  218,  218,
 /*  6140 */   218,  218,  218,  218,  218,  161,  162,  163,  164,  165,
 /*  6150 */   166,  167,  168,  169,  170,  171,  172,  173,  174,  175,
 /*  6160 */   176,  177,  178,  179,  180,  181,  182,  129,  130,  218,
 /*  6170 */   218,  218,  218,  218,  218,   41,  192,  218,  218,  218,
 /*  6180 */   100,  101,  102,  103,  104,  105,  202,  203,  204,  205,
 /*  6190 */   152,   57,  218,  218,  218,  218,  218,  218,  218,  161,
 /*  6200 */   162,  163,  164,  165,  166,  167,  168,  169,  170,  171,
 /*  6210 */   172,  173,  174,  175,  176,  177,  178,  179,  180,  181,
 /*  6220 */   182,  129,  130,  218,  218,  218,  218,  218,  218,  218,
 /*  6230 */   192,  218,  218,  218,  100,  101,  102,  103,  104,  105,
 /*  6240 */   202,  203,  204,  205,  152,  218,  218,  218,  218,  218,
 /*  6250 */   218,  218,  218,  161,  162,  163,  164,  165,  166,  167,
 /*  6260 */   168,  169,  170,  171,  172,  173,  174,  175,  176,  177,
 /*  6270 */   178,  179,  180,  181,  182,  129,  130,  218,  218,  218,
 /*  6280 */   218,  218,  218,  218,  192,  218,  218,  218,  218,  218,
 /*  6290 */   218,  218,  218,  218,  202,  203,  204,  205,  152,  218,
 /*  6300 */   218,  218,  218,  218,  218,  218,  218,  161,  162,  163,
 /*  6310 */   164,  165,  166,  167,  168,  169,  170,  171,  172,  173,
 /*  6320 */   174,  175,  176,  177,  178,  179,  180,  181,  182,  129,
 /*  6330 */   130,  218,  218,  218,  218,  218,  218,  218,  192,  218,
 /*  6340 */   218,  218,  218,  218,  218,  218,  218,  218,  202,  203,
 /*  6350 */   204,  205,  152,  218,  218,  218,  218,  218,  218,  218,
 /*  6360 */   218,  161,  162,  163,  164,  165,  166,  167,  168,  169,
 /*  6370 */   170,  171,  172,  173,  174,  175,  176,  177,  178,  179,
 /*  6380 */   180,  181,  182,  129,  130,  218,  218,  218,  218,  218,
 /*  6390 */   218,  218,  192,  218,  218,  218,  218,  218,  218,  218,
 /*  6400 */   218,  218,  202,  203,  204,  205,  152,  218,  218,  218,
 /*  6410 */   218,  218,  218,  218,  218,  161,  162,  163,  164,  165,
 /*  6420 */   166,  167,  168,  169,  170,  171,  172,  173,  174,  175,
 /*  6430 */   176,  177,  178,  179,  180,  181,  182,  129,  130,  218,
 /*  6440 */   218,  218,  218,  218,  218,  218,  192,  218,  218,  218,
 /*  6450 */   218,  218,  218,  218,  218,  218,  202,  203,  204,  205,
 /*  6460 */   152,  218,  218,  218,  218,  218,  218,  218,  218,  161,
 /*  6470 */   162,  163,  164,  165,  166,  167,  168,  169,  170,  171,
 /*  6480 */   172,  173,  174,  175,  176,  177,  178,  179,  180,  181,
 /*  6490 */   182,  129,  130,  218,  218,  218,  218,  218,  218,  218,
 /*  6500 */   192,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6510 */   202,  203,  204,  205,  152,  218,  218,  218,  218,  218,
 /*  6520 */   218,  218,  218,  161,  162,  163,  164,  165,  166,  167,
 /*  6530 */   168,  169,  170,  171,  172,  173,  174,  175,  176,  177,
 /*  6540 */   178,  179,  180,  181,  182,  129,  130,  218,  218,  218,
 /*  6550 */   218,  218,  218,  218,  192,  218,  218,  218,  218,  218,
 /*  6560 */   218,  218,  218,  218,  202,  203,  204,  205,  152,  218,
 /*  6570 */   218,  218,  218,  218,  218,  218,  218,  161,  162,  163,
 /*  6580 */   164,  165,  166,  167,  168,  169,  170,  171,  172,  173,
 /*  6590 */   174,  175,  176,  177,  178,  179,  180,  181,  182,  129,
 /*  6600 */   130,  218,  218,  218,  218,  218,  218,  218,  192,  218,
 /*  6610 */   218,  218,  218,  218,  218,  218,  218,  218,  202,  203,
 /*  6620 */   204,  205,  152,  218,  218,  218,  218,  218,  218,  218,
 /*  6630 */   218,  161,  162,  163,  164,  165,  166,  167,  168,  169,
 /*  6640 */   170,  171,  172,  173,  174,  175,  176,  177,  178,  179,
 /*  6650 */   180,  181,  182,  129,  130,  218,  218,  218,  218,  218,
 /*  6660 */   218,  218,  192,  218,  218,  218,  218,  218,  218,  218,
 /*  6670 */   218,  218,  202,  203,  204,  205,  152,  218,  218,  218,
 /*  6680 */   218,  218,  218,  218,  218,  161,  162,  163,  164,  165,
 /*  6690 */   166,  167,  168,  169,  170,  171,  172,  173,  174,  175,
 /*  6700 */   176,  177,  178,  179,  180,  181,  182,  129,  130,  218,
 /*  6710 */   218,  218,  218,  218,  218,  218,  192,  218,  218,  218,
 /*  6720 */   218,  218,  218,  218,  218,  218,  202,  203,  204,  205,
 /*  6730 */   152,  218,  218,  218,  218,  218,  218,  218,  218,  161,
 /*  6740 */   162,  163,  164,  165,  166,  167,  168,  169,  170,  171,
 /*  6750 */   172,  173,  174,  175,  176,  177,  178,  179,  180,  181,
 /*  6760 */   182,  129,  130,  218,  218,  218,  218,  218,  218,  218,
 /*  6770 */   192,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6780 */   202,  203,  204,  205,  152,  218,  218,  218,  218,  218,
 /*  6790 */   218,  218,  218,  161,  162,  163,  164,  165,  166,  167,
 /*  6800 */   168,  169,  170,  171,  172,  173,  174,  175,  176,  177,
 /*  6810 */   178,  179,  180,  181,  182,  129,  130,  218,  218,  218,
 /*  6820 */   218,  218,  218,  218,  192,  218,  218,  218,  218,  218,
 /*  6830 */   218,  218,  218,  218,  202,  203,  204,  205,  152,  218,
 /*  6840 */   218,  218,  218,  218,  218,  218,  218,  161,  162,  163,
 /*  6850 */   164,  165,  166,  167,  168,  169,  170,  171,  172,  173,
 /*  6860 */   174,  175,  176,  177,  178,  179,  180,  181,  182,  129,
 /*  6870 */   130,  218,  218,  218,  218,  218,  218,  218,  192,  218,
 /*  6880 */   218,  218,  218,  218,  218,  218,  218,  218,  202,  203,
 /*  6890 */   204,  205,  152,  218,  218,  218,  218,  218,  218,  218,
 /*  6900 */   218,  161,  162,  163,  164,  165,  166,  167,  168,  169,
 /*  6910 */   170,  171,  172,  173,  174,  175,  176,  177,  178,  179,
 /*  6920 */   180,  181,  182,  129,  130,  218,  218,  218,  218,  218,
 /*  6930 */   218,  218,  192,  218,  218,  218,  218,  218,  218,  218,
 /*  6940 */   218,  218,  202,  203,  204,  205,  152,  218,  218,  218,
 /*  6950 */   218,  218,  218,  218,  218,  161,  162,  163,  164,  165,
 /*  6960 */   166,  167,  168,  169,  170,  171,  172,  173,  174,  175,
 /*  6970 */   176,  177,  178,  179,  180,  181,  182,  129,  130,  218,
 /*  6980 */   218,  218,  218,  218,  218,  218,  192,  218,  218,  218,
 /*  6990 */   218,  218,  218,  218,  218,  218,  202,  203,  204,  205,
 /*  7000 */   152,  218,  218,  218,  218,  218,  218,  218,  218,  161,
 /*  7010 */   162,  163,  164,  165,  166,  167,  168,  169,  170,  171,
 /*  7020 */   172,  173,  174,  175,  176,  177,  178,  179,  180,  181,
 /*  7030 */   182,  129,  130,  218,  218,  218,  218,  218,  218,  218,
 /*  7040 */   192,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7050 */   202,  203,  204,  205,  152,  218,  218,  218,  218,  218,
 /*  7060 */   218,  218,  218,  161,  162,  163,  164,  165,  166,  167,
 /*  7070 */   168,  169,  170,  171,  172,  173,  174,  175,  176,  177,
 /*  7080 */   178,  179,  180,  181,  182,  129,  130,  218,  218,  218,
 /*  7090 */   218,  218,  218,  218,  192,  218,  218,  218,  218,  218,
 /*  7100 */   218,  218,  218,  218,  202,  203,  204,  205,  218,  218,
 /*  7110 */   218,  218,  218,  218,  218,  218,  218,  161,  162,  163,
 /*  7120 */   164,  165,  166,  167,  168,  169,  170,  171,  172,  173,
 /*  7130 */   174,  175,  176,  177,  178,  179,  180,  181,  182,  218,
 /*  7140 */   218,    7,  218,    9,   10,  218,  218,  218,  192,   15,
 /*  7150 */   218,  218,  218,  218,  218,   21,  218,  218,  202,  203,
 /*  7160 */   204,  205,   28,  218,  218,  218,  218,   33,   34,   35,
 /*  7170 */    36,   37,   38,   39,   40,   41,  218,    7,   44,    9,
 /*  7180 */    10,  218,  218,  218,  218,   15,  218,  218,   54,  218,
 /*  7190 */   218,   21,  218,   59,   60,   61,  218,  218,   28,   65,
 /*  7200 */   218,  218,  218,   33,   34,   35,   36,   37,   38,   39,
 /*  7210 */    40,   41,  218,  218,   44,  218,  218,  218,  218,  218,
 /*  7220 */   218,  218,  218,  218,   54,  218,  218,  218,  218,   59,
 /*  7230 */    60,   61,  218,  218,  218,   65,  218,  218,  218,  218,
 /*  7240 */   106,  218,  218,  218,  218,  218,  218,  218,  114,  115,
 /*  7250 */   116,  117,  118,   12,   13,   14,   15,   16,   17,   18,
 /*  7260 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /*  7270 */    29,   30,   31,   32,  218,  218,  106,  218,  218,  218,
 /*  7280 */   218,  218,   41,   42,  114,  115,  116,  117,  118,  218,
 /*  7290 */   218,  218,    7,  218,    9,   10,  218,  218,  218,  218,
 /*  7300 */    15,  218,  218,  218,  218,  218,   21,  218,  218,  218,
 /*  7310 */   218,  218,  218,   28,  218,  218,  218,  218,   33,   34,
 /*  7320 */    35,   36,   37,   38,   39,   40,   41,  218,    7,   44,
 /*  7330 */     9,   10,  218,  218,  218,  218,   15,  218,  218,   54,
 /*  7340 */   218,  218,   21,  218,   59,   60,   61,  218,  218,   28,
 /*  7350 */    65,  218,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  7360 */    39,   40,   41,  218,  218,   44,  218,  218,  218,  218,
 /*  7370 */   218,  218,  218,  218,  218,   54,  218,  218,  218,  218,
 /*  7380 */    59,   60,   61,  218,  218,  218,   65,  218,  218,  218,
 /*  7390 */   218,  106,  218,  218,  218,  218,  218,  218,  218,  114,
 /*  7400 */   115,  116,  117,  118,  218,   13,   14,   15,   16,   17,
 /*  7410 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*  7420 */    28,   29,   30,   31,   32,  218,  218,  106,  218,  218,
 /*  7430 */   218,  218,  218,   41,   42,  114,  115,  116,  117,  118,
 /*  7440 */   218,  218,  218,    7,  218,    9,   10,  218,  218,  218,
 /*  7450 */   218,   15,  218,  218,  218,  218,  218,   21,  218,  218,
 /*  7460 */   218,  218,  218,  218,   28,  218,  218,  218,  218,   33,
 /*  7470 */    34,   35,   36,   37,   38,   39,   40,   41,  218,    7,
 /*  7480 */    44,    9,   10,  218,  218,  218,  218,   15,  218,  218,
 /*  7490 */    54,  218,  218,   21,  218,   59,   60,   61,  218,  218,
 /*  7500 */    28,   65,  218,  218,  218,   33,   34,   35,   36,   37,
 /*  7510 */    38,   39,   40,   41,  218,  218,   44,  218,  218,  218,
 /*  7520 */   218,  218,  218,  218,  218,  218,   54,  218,  218,  218,
 /*  7530 */   218,   59,   60,   61,  218,  218,  218,   65,  218,  218,
 /*  7540 */   218,  218,  106,  218,  218,  218,  218,  218,  218,  218,
 /*  7550 */   114,  115,  116,  117,  118,  218,  218,   14,   15,   16,
 /*  7560 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /*  7570 */    27,   28,   29,   30,   31,   32,  218,  218,  106,  218,
 /*  7580 */   218,  218,  218,  218,   41,   42,  114,  115,  116,  117,
 /*  7590 */   118,  218,  218,  218,    7,  218,    9,   10,  218,  218,
 /*  7600 */   218,  218,   15,  218,  218,  218,  218,  218,   21,  218,
 /*  7610 */   218,  218,  218,  218,  218,   28,  218,  218,  218,  218,
 /*  7620 */    33,   34,   35,   36,   37,   38,   39,   40,   41,  218,
 /*  7630 */     7,   44,    9,   10,  218,  218,  218,  218,   15,  218,
 /*  7640 */   218,   54,  218,  218,   21,  218,   59,   60,   61,  218,
 /*  7650 */   218,   28,   65,  218,  218,  218,   33,   34,   35,   36,
 /*  7660 */    37,   38,   39,   40,   41,  218,   21,   44,  218,  218,
 /*  7670 */   218,  218,  218,  218,  218,  218,  218,   54,  218,  218,
 /*  7680 */   218,  218,   59,   60,   61,  218,  218,  218,   65,  218,
 /*  7690 */   218,  218,  218,  106,  218,  218,  218,  218,  218,  218,
 /*  7700 */   218,  114,  115,  116,  117,  118,  218,  218,  218,   64,
 /*  7710 */    65,   66,    0,   68,   69,   70,   71,   72,   73,   74,
 /*  7720 */    75,   76,   77,   78,   79,   80,   81,  218,  218,  106,
 /*  7730 */   218,  218,  218,  218,  218,  218,  218,  114,  115,  116,
 /*  7740 */   117,  118,  218,  218,  218,    7,  218,    9,   10,  218,
 /*  7750 */   218,  218,  218,   15,  218,   43,   44,  218,   46,   21,
 /*  7760 */    48,  218,   50,  218,   52,   53,   28,  218,   56,  218,
 /*  7770 */   218,   33,   34,   35,   36,   37,   38,   39,   40,   41,
 /*  7780 */   218,    7,   44,    9,   10,  218,  218,  218,  218,   15,
 /*  7790 */   218,  218,   54,  218,  218,   21,  218,   59,   60,   61,
 /*  7800 */   218,  218,   28,   65,  218,  218,  218,   33,   34,   35,
 /*  7810 */    36,   37,   38,   39,   40,   41,  218,   21,   44,  218,
 /*  7820 */   218,  218,  218,  218,  218,  218,  218,  218,   54,  218,
 /*  7830 */   218,  119,  218,   59,   60,   61,  218,  218,  218,   65,
 /*  7840 */   218,  218,  218,  218,  106,  218,  218,  218,  218,  218,
 /*  7850 */   218,  218,  114,  115,  116,  117,  118,  218,  218,  218,
 /*  7860 */   218,   65,   66,  218,   68,   69,   70,   71,   72,   73,
 /*  7870 */    74,   75,   76,   77,   78,   79,   80,   81,  218,  218,
 /*  7880 */   106,  218,  218,  218,  218,  218,  218,  218,  114,  115,
 /*  7890 */   116,  117,  118,  218,  218,  218,  218,    8,  218,  218,
 /*  7900 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*  7910 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /*  7920 */    31,   32,  218,  218,  218,  218,  218,  218,  218,    8,
 /*  7930 */    41,   42,   11,   12,   13,   14,   15,   16,   17,   18,
 /*  7940 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /*  7950 */    29,   30,   31,   32,  218,  218,   67,  218,  218,  218,
 /*  7960 */   218,    8,   41,   42,   11,   12,   13,   14,   15,   16,
 /*  7970 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /*  7980 */    27,   28,   29,   30,   31,   32,    7,  218,    9,   10,
 /*  7990 */   218,  218,  218,   40,   41,   42,  218,  218,  218,  218,
 /*  8000 */    21,  218,  218,  218,  218,  218,  218,   28,  218,  218,
 /*  8010 */    89,  218,   33,   34,   35,   36,   37,   38,   39,  218,
 /*  8020 */    41,  218,    7,   44,    9,   10,  218,  218,  218,  218,
 /*  8030 */    15,  218,  218,   54,  218,  218,   21,  218,   59,   60,
 /*  8040 */    61,  218,  218,   28,   65,  218,   67,  218,   33,   34,
 /*  8050 */    35,   36,   37,   38,   39,  218,   41,  218,  218,   44,
 /*  8060 */   218,  218,  218,  218,  218,  218,  218,  218,  218,   54,
 /*  8070 */   218,  218,  218,  218,   59,   60,   61,  218,  218,  218,
 /*  8080 */    65,  218,  218,  218,  218,  106,  218,  218,    7,  218,
 /*  8090 */     9,   10,  218,  114,  115,  116,  117,  118,  218,  218,
 /*  8100 */   218,  218,   21,  218,  218,  218,  218,  218,  218,   28,
 /*  8110 */   218,  218,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  8120 */    39,  106,   41,  218,  218,   44,  218,  218,  218,  114,
 /*  8130 */   115,  116,  117,  118,  218,   54,  218,  218,  218,  218,
 /*  8140 */    59,   60,   61,  218,    8,  218,   65,   11,   12,   13,
 /*  8150 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*  8160 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  218,
 /*  8170 */   218,  218,  218,  218,  218,  218,  218,   41,   42,   98,
 /*  8180 */   218,  218,  218,  218,  218,  218,    7,  106,    9,   10,
 /*  8190 */    54,  218,  218,  218,  218,  114,  115,  116,  117,  118,
 /*  8200 */    21,  218,  218,  218,  218,  218,  218,   28,  218,  218,
 /*  8210 */   218,  218,   33,   34,   35,   36,   37,   38,   39,  218,
 /*  8220 */    41,  218,  218,   44,  218,  218,  218,  218,  218,  218,
 /*  8230 */   218,  218,  218,   54,  218,  218,  218,  218,   59,   60,
 /*  8240 */    61,  218,  218,  218,   65,  218,   67,    8,  218,  218,
 /*  8250 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*  8260 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /*  8270 */    31,   32,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8280 */    41,   42,  218,  218,  218,  106,  218,  218,    7,  218,
 /*  8290 */     9,   10,  218,  114,  115,  116,  117,  118,  218,  218,
 /*  8300 */   218,  218,   21,  218,  218,  218,   67,  218,  218,   28,
 /*  8310 */   218,  218,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  8320 */    39,  218,   41,  218,    7,   44,    9,   10,  218,  218,
 /*  8330 */   218,  218,  218,  218,  218,   54,  218,  218,   21,  218,
 /*  8340 */    59,   60,   61,  218,  218,   28,   65,  218,   67,  218,
 /*  8350 */    33,   34,   35,   36,   37,   38,   39,  218,   41,  218,
 /*  8360 */   218,   44,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8370 */   218,   54,  218,  218,  218,  218,   59,   60,   61,  218,
 /*  8380 */   218,  218,   65,  218,   67,  218,  218,  106,  218,  218,
 /*  8390 */     7,  218,    9,   10,  218,  114,  115,  116,  117,  118,
 /*  8400 */   218,  218,  218,  218,   21,  218,  218,  218,  218,  218,
 /*  8410 */   218,   28,  218,  218,  218,  218,   33,   34,   35,   36,
 /*  8420 */    37,   38,   39,  106,   41,  218,    7,   44,    9,   10,
 /*  8430 */   218,  114,  115,  116,  117,  118,  218,   54,  218,  218,
 /*  8440 */    21,  218,   59,   60,   61,  218,  218,   28,   65,  218,
 /*  8450 */    67,  218,   33,   34,   35,   36,   37,   38,   39,  218,
 /*  8460 */    41,  218,  218,   44,  218,  218,  218,  218,  218,  218,
 /*  8470 */   218,  218,  218,   54,  218,  218,  218,  218,   59,   60,
 /*  8480 */    61,  218,  218,  218,   65,  218,   67,  218,  218,  106,
 /*  8490 */   218,  218,    7,  218,    9,   10,  218,  114,  115,  116,
 /*  8500 */   117,  118,  218,  218,  218,  218,   21,  218,  218,  218,
 /*  8510 */   218,  218,  218,   28,  218,  218,  218,  218,   33,   34,
 /*  8520 */    35,   36,   37,   38,   39,  106,   41,  218,    7,   44,
 /*  8530 */     9,   10,  218,  114,  115,  116,  117,  118,  218,   54,
 /*  8540 */   218,  218,   21,  218,   59,   60,   61,  218,  218,   28,
 /*  8550 */    65,  218,   67,  218,   33,   34,   35,   36,   37,   38,
 /*  8560 */    39,   21,   41,  218,  218,   44,   45,  218,  218,  218,
 /*  8570 */   218,  218,  218,  218,  218,   54,  218,  218,  218,  218,
 /*  8580 */    59,   60,   61,  218,   44,  218,   65,  218,  218,  218,
 /*  8590 */   218,  106,  218,  218,  218,  218,  218,  218,   58,  114,
 /*  8600 */   115,  116,  117,  118,  218,  218,  218,  218,   68,   69,
 /*  8610 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*  8620 */    80,   81,  218,  218,  218,  218,  218,  106,  218,  218,
 /*  8630 */   218,  218,  218,  218,  218,  114,  115,  116,  117,  118,
 /*  8640 */   218,  218,  218,  218,    8,  218,  218,   11,   12,   13,
 /*  8650 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*  8660 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  218,
 /*  8670 */   218,  218,  218,  218,  218,  218,  218,   41,   42,  218,
 /*  8680 */   218,   45,  218,  218,  218,  218,    8,  218,  218,   11,
 /*  8690 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /*  8700 */    22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
 /*  8710 */    32,  218,  218,  218,  218,  218,  218,  218,  218,   41,
 /*  8720 */    42,  218,  218,   45,  218,  218,  218,  218,    8,  218,
 /*  8730 */   218,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*  8740 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*  8750 */    30,   31,   32,  218,  218,  218,  218,  218,  218,  218,
 /*  8760 */   218,   41,   42,  218,  218,   45,  218,  218,  218,  218,
 /*  8770 */     8,  218,  218,   11,   12,   13,   14,   15,   16,   17,
 /*  8780 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*  8790 */    28,   29,   30,   31,   32,  218,  218,  218,  218,  218,
 /*  8800 */   218,  218,  218,   41,   42,  218,  218,   45,  218,  218,
 /*  8810 */   218,  218,    8,  218,  218,   11,   12,   13,   14,   15,
 /*  8820 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*  8830 */    26,   27,   28,   29,   30,   31,   32,  218,    7,  218,
 /*  8840 */     9,   10,  218,  218,  218,   41,   42,  218,  218,  218,
 /*  8850 */   218,  218,   21,  218,  218,  218,  218,  218,   54,   28,
 /*  8860 */   218,  218,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  8870 */    39,  218,   41,  218,  218,   44,  218,  218,  218,  218,
 /*  8880 */   218,  218,  218,  218,  218,   54,  218,  218,  218,  218,
 /*  8890 */    59,   60,   61,  218,    8,  218,   65,   11,   12,   13,
 /*  8900 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*  8910 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  218,
 /*  8920 */   218,  218,  218,  218,  218,  218,  218,   41,   42,   98,
 /*  8930 */   218,  218,  218,  218,  218,  218,  218,  106,  218,  218,
 /*  8940 */    54,  218,  218,  218,  218,  114,  115,  116,  117,  118,
 /*  8950 */   218,  218,  218,  218,    8,  218,  218,   11,   12,   13,
 /*  8960 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*  8970 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  218,
 /*  8980 */   218,    7,  218,    9,   10,  218,  218,   41,   42,   15,
 /*  8990 */   218,  218,  218,  218,  218,   21,  218,  218,  218,  218,
 /*  9000 */    54,  218,   28,  218,  218,  218,  218,   33,   34,   35,
 /*  9010 */    36,   37,   38,   39,  218,   41,  218,    7,   44,    9,
 /*  9020 */    10,  218,  218,  218,  218,  218,  218,  218,   54,  218,
 /*  9030 */   218,   21,  218,   59,   60,   61,  218,  218,   28,   65,
 /*  9040 */   218,  218,  218,   33,   34,   35,   36,   37,   38,   39,
 /*  9050 */    21,   41,  218,  218,   44,  218,  218,  218,  218,  218,
 /*  9060 */   218,  218,  218,  218,   54,  218,  218,  218,  218,   59,
 /*  9070 */    60,   61,  218,   44,  218,   65,  218,  218,  218,  218,
 /*  9080 */   106,  218,  218,  218,  218,  218,  218,  218,  114,  115,
 /*  9090 */   116,  117,  118,  218,  218,  218,  218,   68,   69,   70,
 /*  9100 */    71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*  9110 */    81,  218,  218,  218,  218,  218,  106,  218,  218,  218,
 /*  9120 */   218,  218,  218,  218,  114,  115,  116,  117,  118,  218,
 /*  9130 */   218,  218,  218,    8,  218,  218,   11,   12,   13,   14,
 /*  9140 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*  9150 */    25,   26,   27,   28,   29,   30,   31,   32,    7,  218,
 /*  9160 */     9,   10,  218,  218,  218,  218,   41,   42,  218,  218,
 /*  9170 */   218,  218,   21,  218,  218,  218,  218,  218,   41,   28,
 /*  9180 */   218,   44,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  9190 */    39,  218,   41,  218,  218,   44,   59,  218,  218,  218,
 /*  9200 */   218,  218,   65,  218,   67,   54,  218,  218,  218,  218,
 /*  9210 */    59,   60,   61,  218,  218,  218,   65,   11,   12,   13,
 /*  9220 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*  9230 */    24,   25,   26,   27,   28,   29,   30,   31,   32,   21,
 /*  9240 */   218,  218,  218,  106,  218,  218,  218,   41,   42,  218,
 /*  9250 */   218,  114,  115,  116,  117,  118,  218,  106,   40,  218,
 /*  9260 */   218,  218,   44,  218,  218,  114,  115,  116,  117,  118,
 /*  9270 */   218,  218,  218,  218,  218,  218,   58,  218,  218,  218,
 /*  9280 */   218,  218,  218,  218,   21,  218,   68,   69,   70,   71,
 /*  9290 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*  9300 */   218,  218,  218,   40,  218,  218,  218,   44,  218,  218,
 /*  9310 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  9320 */   218,   58,  218,  218,  218,  218,  218,  218,  218,   21,
 /*  9330 */   218,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  9340 */    77,   78,   79,   80,   81,  218,  218,  218,   40,  218,
 /*  9350 */   218,  218,   44,  218,  218,  218,  218,  218,  218,  218,
 /*  9360 */   218,  218,  218,  218,  218,  218,   58,  218,  218,  218,
 /*  9370 */   218,  218,  218,  218,   21,  218,   68,   69,   70,   71,
 /*  9380 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*  9390 */   218,  218,  218,   40,  218,  218,  218,   44,  218,  218,
 /*  9400 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  9410 */   218,   58,  218,  218,  218,  218,  218,  218,  218,   21,
 /*  9420 */   218,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  9430 */    77,   78,   79,   80,   81,   41,   42,  218,   40,  218,
 /*  9440 */   218,  218,   44,  218,  218,  218,  218,  218,  218,  218,
 /*  9450 */   218,   57,  218,  218,  218,  218,   58,  218,  218,  218,
 /*  9460 */   218,  218,  218,  218,  218,  218,   68,   69,   70,   71,
 /*  9470 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*  9480 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  9490 */   218,  218,  218,  218,  100,  101,  102,  103,  104,  105,
 /*  9500 */   218,  107,  108,  109,
};
#define YY_SHIFT_USE_DFLT (-32)
static short yy_shift_ofst[] = {
 /*     0 */    76,   15, 7712,  -32,  -32,  -32,  -32,  -32,  -32,  -32,
 /*    10 */   150,   37,  -32,  157,  112,  -32,  157,  -32,  168,  226,
 /*    20 */   -32,  -32,  237,  110,  -32,  306,  301,  -32,   38,  -32,
 /*    30 */   335,  -32,  151,  716,  -32, 1846,  409,  416, 9218,   72,
 /*    40 */  7645,  521,  -32,  -32,  489, 7796,  -32,  582,  -32,  -32,
 /*    50 */   -32,  -32,  -32,  722,  127,  -32,  694,  758,  -32,  -32,
 /*    60 */   -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,
 /*    70 */   -32,  -32,  -32,  -32,  -32,  221,  279, 7645,  747,  -32,
 /*    80 */   -32, 8540,  -32,  754, 5755,  -32,  -32,  -32,  -32,  -32,
 /*    90 */   -32,  -32,  -32,  709,  858,  -32,  -32, 9137,  804,  866,
 /*   100 */   143,  -32,  317,  -32, 5809,  -32,  871, 5755,  -32,  -32,
 /*   110 */   -32,  -32, 9029,  907, 5755,  -32,  192,  926, 5755,  -32,
 /*   120 */   956,  949, 5755,  -32, 1006, 1004, 5755,  -32, 1009, 1045,
 /*   130 */   233, 1016, 5755,  -32, 1033, 1034, 5755,  -32, 1051, 1039,
 /*   140 */  5755,  -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,
 /*   150 */   -32,  -32,  -32, 2411, 1959, 1061, 1038, 9263,  353, 7645,
 /*   160 */  1067,  -32,  -32,  322,  354, 7645, 1068,  -32,  -32,  -32,
 /*   170 */   -32,  -32, 2072,  114, 1043, 5755, 1074,  -32, 1084, 5755,
 /*   180 */  1097,  -32,  255, 1086, 5755, 1105,  -32, 1106, 5755, 1120,
 /*   190 */   -32,  829,  -32, 1110,  305,  -32, 1122,  366,  -32, 1132,
 /*   200 */    77,  -32,  264,  -32, 1130,  -32,  377,  942,  -32, 2185,
 /*   210 */  1142, 1126, 9308,  217,   -7,  -32,  106,  -32,  -32,  -32,
 /*   220 */   -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,
 /*   230 */   -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32,
 /*   240 */   -32,  -32,  -32, 1144,  -32, 1145,  -32, 9010, 1148,  219,
 /*   250 */   332,  156,  336, 1154,  445,  558,  -32, 9010, 1155,  -31,
 /*   260 */   -32,   33,  -32,  -32, 5755, 1129, 5417, 5417, 1178,  671,
 /*   270 */   784,  -32, 9010, 1190,  897, 1010,  -32, 1200, 1123, 1236,
 /*   280 */  1163, 9010, 1211,  -32, 9010,  434, 9010, 9010,   14, 9010,
 /*   290 */  9010,  434, 9010, 9010, 9125, 9010, 9010,  434, 9010, 9010,
 /*   300 */   434, 9010, 9010, 9206, 9010, 9010, 9206, 9010,  316,  420,
 /*   310 */  1201, 7134, 9125, 9010, 7889,  -32, 9010,   14, 9010,   14,
 /*   320 */  9010,  434, 9010,  434, 9010,  434, 9010,   14, 9010, 7543,
 /*   330 */  9010, 7392, 9010, 5600, 9010, 5600, 9010, 5600, 9010, 5600,
 /*   340 */  9010, 5600, 9010, 7241, 9010, 7921, 9010, 9125, 5530, 7953,
 /*   350 */   -32, 1223, 9010,  434, 1224, 1245, 9010,  316,  380,  452,
 /*   360 */  1215, 7170,   41, 7285, 1234, 1273, 9010,  434,  -32, 9010,
 /*   370 */   434,  -32,  -32,  -32,  -32,  -32,  -32,  -32,  -32, 7979,
 /*   380 */  9125,  493, 1198, 1199, 1204,  -32,  466,  -32, 9151,  480,
 /*   390 */  1228, 7321,  -32,  529,  -32, 8015, 1250, 1243,  395, 7436,
 /*   400 */   569,  -32,  -32,  -32,  -32,  -32,  -32, 9010, 9125, 1238,
 /*   410 */  9353,  584, 1246, 1349, 1462, 1248, 1575, 1688, 1208, 1208,
 /*   420 */   -32, 1271,  358, 1801, 1914,  -32, 1277,  492, 8081, 8136,
 /*   430 */  2027, 2140,  -32,  496,  502,  -32,  496,  -32, 9394,  -32,
 /*   440 */   -32,  -32,  -32,  -32,  -32,  -32, 9010,  -32, 9125,  593,
 /*   450 */  5637, 9010,  -32, 8179, 1260, 9010,  -32, 1257,  -32, 8239,
 /*   460 */  5864, 9010,  -32, 8281, 1260, 9010,  -32,  -32,  -32,  -32,
 /*   470 */   -32,  505, 1285, 1260, 9010,  -32, 1311, 1260, 9010,  -32,
 /*   480 */  1324, 5918, 9010,  -32, 8317, 1260, 9010,  -32, 6080, 9010,
 /*   490 */   -32, 8383, 1260, 9010,  -32, 8419, 1260, 9010,  -32, 6134,
 /*   500 */  9010,  -32, 8485, 1260, 9010,  -32,  -32,  -32,  533, 1326,
 /*   510 */  1260, 9010,  -32, 1336, 1260, 9010,  -32,  -32, 9010,  638,
 /*   520 */   -32, 9010,  -32, 9125,  -32, 1355,  -32, 1356,  -32, 1357,
 /*   530 */   -32, 1361,  -32, 8521, 8636,  -32,  -32, 9010, 8678,  -32,
 /*   540 */  9010, 8720,  -32, 9010, 8762,  -32, 1363,  656,  -32, 1363,
 /*   550 */   -32, 1354, 5755,  -32,  -32, 1363,  678,  -32, 1363,  690,
 /*   560 */   -32, 1363,  707,  -32, 1363,  796,  -32, 1363,  797,  -32,
 /*   570 */  1363,  820,  -32, 1363,  841,  -32, 1363,  842,  -32, 1363,
 /*   580 */   883,  -32, 1363,  909,  -32, 9125,  -32,  -32,  -32,  -32,
 /*   590 */  9010, 8804, 5417, 2253,  -32, 1371, 1319, 8831, 8886, 2366,
 /*   600 */  2479,  -32,  -32, 9010, 8946, 5417, 2592,  -32,  -32, 1384,
 /*   610 */  1375, 2705, 2818,  -32,  -32, 1271,  -32,  -32,  -32,  -32,
 /*   620 */   -32,  -32,  -32, 1380, 2931, 3044,  -32,  -32,  594, 1374,
 /*   630 */  7472,  647,  -32,  -32, 1393, 1383, 1378, 7587,  899,  -32,
 /*   640 */   -32,  -32, 1400, 1396, 1391, 7623,  -32,  910,  -32,  -32,
 /*   650 */  1364, 9010,  -32,  -32,  -32,  923,  -32,  -32, 8974, 9125,
 /*   660 */  9010, 9125,  996,  -32,  -32,  -32, 1012,  -32,  -32,  766,
 /*   670 */  1399, 1397, 7738, 1022,  -32,  -32, 1415, 1412, 7774, 1023,
 /*   680 */   -32,  -32,  316,  316,  316,  316,  316,  316,  316, 9125,
 /*   690 */  1382, 9010, 1430,  -32,  -32,  -32, 1388, 5417, 5417,  -32,
 /*   700 */   -32,  -32, 9010, 1424, 3157, 3270,  -32,  -32, 1425, 3383,
 /*   710 */  3496,  -32,  -32,  -32,  778,  863, 1426, 1427,  -32, 1437,
 /*   720 */  3609, 3722,  -32,  -32,  -32, 7645,  373, 3835,  -32, 3948,
 /*   730 */   -32,  -32, 1036,  307, 4061,  -32, 4174,  -32,  -32, 7645,
 /*   740 */   792, 4287,  -32, 4400,  -32,  -32, 2411, 2298, 1448, 1420,
 /*   750 */  9398,  443, 4513,  -32, 4626,  -32,  -32, 7645,  834, 4739,
 /*   760 */   -32, 4852,  -32,  -32, 1109,  556, 4965,  -32, 5078,  -32,
 /*   770 */   -32, 7645,  931, 5191,  -32, 5304,  -32,  -32,  490, 1055,
 /*   780 */   -32, 2072,  -32, 2072, 1168,  657,  -32, 5755,  932,  -32,
 /*   790 */  1457,  -32,  319, 1460,  245, 1468,  623,  -32,  -32, 1471,
 /*   800 */   -32,  -32, 1474,  -32, 1281,  770,  -32, 5755,  933,  -32,
 /*   810 */  1480,  -32, 1483,  -32,  603, 1394, 1507, 2411, 1620,  -32,
 /*   820 */  1733, 1132,  -32,  -32,  -32, 1132,   77,  -32, 1479, 1499,
 /*   830 */   458,  -32, 1504,  886,  -32, 1132,   77,  -32, 1132,   77,
 /*   840 */   -32, 1514, 1513,  571,  -32, 1522, 1517,  -32, 1132,   77,
 /*   850 */   -32,  -32,
};
#define YY_REDUCE_USE_DFLT (-163)
static short yy_reduce_ofst[] = {
 /*     0 */  -117, -163,  333, -163, -163, -163, -163, -163, -163, -163,
 /*    10 */  -163, -163, -163,   60, -163, -163,   61, -163, -163, -163,
 /*    20 */  -163, -163, -163,  154, -163, -163,  253, -163,  433, -163,
 /*    30 */  -163, -163,  430,   42, -163,  280, -163, -163, -135, -163,
 /*    40 */   312, -163, -163, -163, -163, -139, -163, -163, -163, -163,
 /*    50 */  -163, -163, -163, -163, -163, -163, -163, -163, -163, -163,
 /*    60 */  -163, -163, -163, -163, -163, -163, -163, -163, -163, -163,
 /*    70 */  -163, -163, -163, -163, -163, -163, -163,  642, -163, -163,
 /*    80 */  -163,  -79, -163, -163,  660, -163, -163, -163, -163, -163,
 /*    90 */  -163, -163, -163, -163, -163, -163, -163,  559, -163, -163,
 /*   100 */  -163, -163, -163, -163, -143, -163, -163,  -84, -163, -163,
 /*   110 */  -163, -163, -107, -163,  838, -163, -163, -163,  848, -163,
 /*   120 */  -163, -163,  893, -163, -163, -163,  924, -163, -163, -163,
 /*   130 */  -163, -163,  945, -163, -163, -163,  946, -163, -163, -163,
 /*   140 */   954, -163, -163, -163, -163, -163, -163, -163, -163, -163,
 /*   150 */  -163, -163, -163,  197,  280, -163, -163,   12, -163,  755,
 /*   160 */  -163, -163, -163, -163, -163,  868, -163, -163, -163, -163,
 /*   170 */  -163, -163,  197, -163, -163,  970, -163, -163, -163,  987,
 /*   180 */  -163, -163, -163, -163, 1011, -163, -163, -163, 1030, -163,
 /*   190 */  -163,   42, -163, -163, 1042, -163, -163, 1047, -163,  277,
 /*   200 */  1048, -163,   95, -163, -163, -163,  546,  240, -163,  280,
 /*   210 */  -163, -163,   88, -163, 5488, -163, 6956, -163, -163, -163,
 /*   220 */  -163, -163, -163, -163, -163, -163, -163, -163, -163, -163,
 /*   230 */  -163, -163, -163, -163, -163, -163, -163, -163, -163, -163,
 /*   240 */  -163, -163, -163, -163, -163, -163, -163, 1021, -163, 5552,
 /*   250 */  6956,  124, 1018, -163, 5606, 6956, -163, 1699, -163,  250,
 /*   260 */  -163, 1024, -163, -163, 1083, -163, 5660, 6956, -163, 5714,
 /*   270 */  6956, -163, 1812, -163, 5768, 6956, -163, -163, 5822, 6956,
 /*   280 */  -163, 2038, -163, -163, 1134, -163, 2490, 2532, -163, 2553,
 /*   290 */  2603, -163, 2645, 2666, -163, 2716, 2758, -163, 2779, 2829,
 /*   300 */  -163, 2871, 2892, -163, 2942, 2984, -163, 3005, -163, -163,
 /*   310 */  -163,  604, -163, 3055, -163, -163, 3097, -163, 3118, -163,
 /*   320 */  3168, -163, 3210, -163, 3231, -163, 3281, -163, 3323, -163,
 /*   330 */  3344, -163, 3394, -163, 3436, -163, 3457, -163, 3507, -163,
 /*   340 */  3549, -163, 3570, -163, 3620, -163, 3662, -163, 1960, -163,
 /*   350 */  -163, -163, 3683, -163, -163, -163, 3733, -163, -163, -163,
 /*   360 */  -163,  717, -163,  830, -163, -163, 3775, -163, -163, 3796,
 /*   370 */  -163, -163, -163, -163, -163, -163, -163, -163, -163, 5494,
 /*   380 */  -163, -163, -163, -163, -163, -163, -163, -163, 5872, -163,
 /*   390 */  -163,  943, -163, -163, -163, 2073, -163, -163, -163, 1056,
 /*   400 */  -163, -163, -163, -163, -163, -163, -163, 3846, -163, -163,
 /*   410 */   125, -163, -163, 5876, 6956, -163, 5930, 6956,  337, 1127,
 /*   420 */  -163,  340, -163, 5984, 6956, -163, -163, -163, 3888, -163,
 /*   430 */  6038, 6956, -163,  422, -163, -163, 1128, -163, -162, -163,
 /*   440 */  -163, -163, -163, -163, -163, -163, 2186, -163, -163, -163,
 /*   450 */  -100, 2299, -163, 2377, 1136, 2412, -163, -163, -163, -163,
 /*   460 */   390, 2525, -163, 2377, 1150, 2638, -163, -163, -163, -163,
 /*   470 */  -163, -163, -163, 1162, 2751, -163, -163, 1172, 2864, -163,
 /*   480 */  -163,   13, 2977, -163, 2377, 1181, 3090, -163,  448, 3203,
 /*   490 */  -163, 2377, 1183, 3316, -163, 2377, 1184, 3429, -163,  465,
 /*   500 */  3542, -163, 2377, 1185, 3655, -163, -163, -163, -163, -163,
 /*   510 */  1194, 3768, -163, -163, 1197, 3881, -163, -163, 1284, -163,
 /*   520 */  -163, 2440, -163, -163, -163, -163, -163, -163, -163, -163,
 /*   530 */  -163, -163, -163, 3909, -163, -163, -163, 3959, -163, -163,
 /*   540 */  3994, -163, -163, 4001, -163, -163,  484, -163, -163, 1205,
 /*   550 */  -163, -163, 1269, -163, -163,  491, -163, -163,  522, -163,
 /*   560 */  -163,  527, -163, -163,  531, -163, -163,  544, -163, -163,
 /*   570 */   550, -163, -163,  553, -163, -163,  629, -163, -163,  644,
 /*   580 */  -163, -163,  663, -163, -163, -163, -163, -163, -163, -163,
 /*   590 */  4022, -163, 6092, 6956, -163, -163, -163, 4072, -163, 6146,
 /*   600 */  6956, -163, -163, 4107, -163, 6200, 6956, -163, -163, -163,
 /*   610 */  -163, 6254, 6956, -163, -163, 1239, -163, -163, -163, -163,
 /*   620 */  -163, -163, -163, -163, 6308, 6956, -163, -163, -163, -163,
 /*   630 */  1395, -163, -163, -163, -163, -163, -163, 1508, -163, -163,
 /*   640 */  -163, -163, -163, -163, -163, 1621, -163, -163, -163, -163,
 /*   650 */  -163,  -67, -163, -163, -163, -163, -163, -163, 4114, -163,
 /*   660 */  4135, -163, -163, -163, -163, -163, -163, -163, -163, -163,
 /*   670 */  -163, -163, 1734, -163, -163, -163, -163, -163, 1847, -163,
 /*   680 */  -163, -163, -163, -163, -163, -163, -163, -163, -163, -163,
 /*   690 */  -163, 2151, -163, -163, -163, -163, -163, 6362, 6956, -163,
 /*   700 */  -163, -163, 2264, -163, 6416, 6956, -163, -163, -163, 6470,
 /*   710 */  6956, -163, -163, -163,  689, 1018, -163, -163, -163, -163,
 /*   720 */  6524, 6956, -163, -163, -163,  981, -163, 6578, -163, 6956,
 /*   730 */  -163, -163, -163, -163, 6632, -163, 6956, -163, -163, 1094,
 /*   740 */  -163, 6686, -163, 6956, -163, -163,  539,  280, -163, -163,
 /*   750 */   147, -163, 6740, -163, 6956, -163, -163, 1216, -163, 6794,
 /*   760 */  -163, 6956, -163, -163, -163, -163, 6848, -163, 6956, -163,
 /*   770 */  -163, 1433, -163, 6902, -163, 6956, -163, -163, 4268,  240,
 /*   780 */  -163,  539, -163,  887,  280, 1348, -163, 1350, 1359, -163,
 /*   790 */  -163, -163,  777, -163, -163, -163, 1367, -163, -163, -163,
 /*   800 */  -163, -163, -163, -163,  280, 1379, -163, 1376, 1381, -163,
 /*   810 */  -163, -163, -163, -163, 2388, 1904,  240,  887,  240, -163,
 /*   820 */   240, 1405, -163, -163, -163,  588, 1409, -163, -163, -163,
 /*   830 */  1414, -163, -163, 1417, -163,  825, 1418, -163,  865, 1419,
 /*   840 */  -163, -163, -163, 1432, -163, -163, 1434, -163,  938, 1450,
 /*   850 */  -163, -163,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */  1270, 1270, 1270,  854,  856,  857,  858,  859,  860,  861,
 /*    10 */  1270, 1270,  862, 1270, 1270,  863, 1270,  864,  866, 1270,
 /*    20 */   867,  865, 1270, 1270,  868, 1270, 1270,  869, 1270,  886,
 /*    30 */  1270,  887, 1270, 1270,  920, 1270, 1270, 1270, 1270, 1270,
 /*    40 */  1270, 1270,  950,  969,  970, 1270,  971,  973,  976,  974,
 /*    50 */   975,  977,  978, 1270, 1270,  997, 1270, 1270,  998,  999,
 /*    60 */  1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009,
 /*    70 */  1010, 1011, 1012,  972,  954, 1270, 1270, 1270, 1270,  951,
 /*    80 */   955, 1270,  979,  981, 1270,  989, 1248, 1249, 1250, 1251,
 /*    90 */  1252, 1253, 1254, 1270, 1270, 1255, 1256, 1270, 1248, 1250,
 /*   100 */  1270, 1257, 1270, 1258, 1270, 1259, 1270, 1270, 1261, 1266,
 /*   110 */  1262, 1260, 1270,  982, 1270,  990, 1270,  984, 1270,  992,
 /*   120 */  1270,  986, 1270,  994, 1270,  988, 1270,  996, 1270, 1270,
 /*   130 */  1270,  983, 1270,  991, 1270,  985, 1270,  993, 1270,  987,
 /*   140 */  1270,  995,  980,  958,  960,  961,  962,  963,  964,  965,
 /*   150 */   966,  967,  968, 1270, 1270, 1270, 1270, 1270, 1270, 1270,
 /*   160 */  1270,  952,  956, 1270, 1270, 1270, 1270,  953,  957,  959,
 /*   170 */   916,  921, 1270, 1270, 1270, 1270, 1270,  922, 1270, 1270,
 /*   180 */  1270,  924, 1270, 1270, 1270, 1270,  923, 1270, 1270, 1270,
 /*   190 */   925, 1270,  917, 1270, 1270,  870, 1270, 1270,  871, 1270,
 /*   200 */  1270,  873, 1270,  881, 1270,  882, 1270, 1270,  918, 1270,
 /*   210 */  1270, 1270, 1270, 1270, 1270,  926, 1270,  930, 1013, 1015,
 /*   220 */  1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025,
 /*   230 */  1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035,
 /*   240 */  1036, 1037, 1038, 1270, 1039, 1270, 1040, 1270, 1270, 1270,
 /*   250 */  1270, 1045, 1046, 1270, 1270, 1270, 1048, 1270, 1270, 1270,
 /*   260 */  1056, 1270, 1057, 1058, 1270, 1270, 1060, 1061, 1270, 1270,
 /*   270 */  1270, 1064, 1270, 1270, 1270, 1270, 1066, 1270, 1270, 1270,
 /*   280 */  1270, 1270, 1270, 1068, 1270, 1150, 1270, 1270, 1151, 1270,
 /*   290 */  1270, 1152, 1270, 1270, 1153, 1270, 1270, 1154, 1270, 1270,
 /*   300 */  1155, 1270, 1270, 1156, 1270, 1270, 1157, 1270, 1165, 1270,
 /*   310 */  1169, 1270, 1231, 1270, 1270, 1174, 1270, 1175, 1270, 1176,
 /*   320 */  1270, 1177, 1270, 1178, 1270, 1179, 1270, 1180, 1270, 1181,
 /*   330 */  1270, 1182, 1270, 1183, 1270, 1184, 1270, 1185, 1270, 1186,
 /*   340 */  1270, 1187, 1270, 1188, 1270, 1270, 1270, 1228, 1270, 1270,
 /*   350 */  1166, 1270, 1270, 1167, 1270, 1270, 1270, 1168, 1192, 1270,
 /*   360 */  1172, 1270, 1192, 1270, 1270, 1270, 1270, 1189, 1190, 1270,
 /*   370 */  1191, 1193, 1194, 1195, 1196, 1197, 1198, 1199, 1200, 1270,
 /*   380 */  1247, 1192, 1193, 1194, 1200, 1201, 1270, 1202, 1270, 1270,
 /*   390 */  1203, 1270, 1204, 1270, 1205, 1270, 1270, 1270, 1270, 1270,
 /*   400 */  1270, 1211, 1212, 1225, 1226, 1227, 1230, 1270, 1233, 1270,
 /*   410 */  1270, 1270, 1270, 1270, 1270, 1270, 1270, 1270, 1070, 1071,
 /*   420 */  1072, 1270, 1270, 1270, 1270, 1074, 1270, 1270, 1270, 1270,
 /*   430 */  1270, 1270, 1081, 1270, 1270, 1087, 1270, 1088, 1270, 1090,
 /*   440 */  1091, 1092, 1093, 1094, 1095, 1096, 1270, 1097, 1149, 1270,
 /*   450 */  1270, 1270, 1098, 1270, 1270, 1270, 1101, 1270, 1113, 1270,
 /*   460 */  1270, 1270, 1102, 1270, 1270, 1270, 1103, 1111, 1112, 1114,
 /*   470 */  1115, 1270, 1270, 1270, 1270, 1099, 1270, 1270, 1270, 1100,
 /*   480 */  1270, 1270, 1270, 1104, 1270, 1270, 1270, 1105, 1270, 1270,
 /*   490 */  1106, 1270, 1270, 1270, 1107, 1270, 1270, 1270, 1108, 1270,
 /*   500 */  1270, 1109, 1270, 1270, 1270, 1110, 1116, 1117, 1270, 1270,
 /*   510 */  1270, 1270, 1118, 1270, 1270, 1270, 1119, 1089, 1270, 1270,
 /*   520 */  1121, 1270, 1122, 1124, 1123, 1225, 1125, 1227, 1126, 1226,
 /*   530 */  1127, 1190, 1128, 1270, 1270, 1129, 1130, 1270, 1270, 1131,
 /*   540 */  1270, 1270, 1132, 1270, 1270, 1133, 1270, 1270, 1134, 1270,
 /*   550 */  1145, 1147, 1270, 1148, 1146, 1270, 1270, 1135, 1270, 1270,
 /*   560 */  1136, 1270, 1270, 1137, 1270, 1270, 1138, 1270, 1270, 1139,
 /*   570 */  1270, 1270, 1140, 1270, 1270, 1141, 1270, 1270, 1142, 1270,
 /*   580 */  1270, 1143, 1270, 1270, 1144, 1270, 1268, 1269, 1014, 1082,
 /*   590 */  1270, 1270, 1270, 1270, 1083, 1270, 1270, 1270, 1270, 1270,
 /*   600 */  1270, 1084, 1085, 1270, 1270, 1270, 1270, 1086, 1075, 1270,
 /*   610 */  1270, 1270, 1270, 1077, 1076, 1270, 1078, 1080, 1079, 1073,
 /*   620 */  1069, 1238, 1237, 1270, 1270, 1270, 1236, 1235, 1270, 1270,
 /*   630 */  1270, 1270, 1215, 1216, 1270, 1270, 1270, 1270, 1270, 1217,
 /*   640 */  1218, 1229, 1270, 1270, 1206, 1270, 1207, 1270, 1208, 1239,
 /*   650 */  1270, 1270, 1241, 1242, 1240, 1270, 1209, 1210, 1270, 1232,
 /*   660 */  1270, 1234, 1270, 1213, 1214, 1173, 1270, 1219, 1220, 1270,
 /*   670 */  1270, 1170, 1270, 1270, 1221, 1222, 1270, 1171, 1270, 1270,
 /*   680 */  1223, 1224, 1164, 1163, 1162, 1161, 1160, 1159, 1158, 1267,
 /*   690 */  1270, 1270, 1270, 1067, 1065, 1063, 1270, 1270, 1062, 1059,
 /*   700 */  1050, 1052, 1270, 1270, 1270, 1270, 1055, 1054, 1270, 1270,
 /*   710 */  1270, 1047, 1049, 1053, 1041, 1042, 1270, 1270, 1044, 1270,
 /*   720 */  1270, 1270, 1051, 1043,  927, 1270, 1270, 1270,  938, 1270,
 /*   730 */   942,  939, 1270, 1270, 1270,  928, 1270,  931,  929, 1270,
 /*   740 */  1270, 1270,  940, 1270,  943,  941, 1270, 1270, 1270, 1270,
 /*   750 */  1270, 1270, 1270,  932, 1270,  936,  933, 1270, 1270, 1270,
 /*   760 */   944, 1270,  948,  945, 1270, 1270, 1270,  934, 1270,  937,
 /*   770 */   935, 1270, 1270, 1270,  946, 1270,  949,  947, 1270, 1270,
 /*   780 */   919, 1270,  900, 1270, 1270, 1270,  902, 1270, 1270,  904,
 /*   790 */  1270,  908, 1270, 1270, 1270, 1270, 1270,  912,  914, 1270,
 /*   800 */   915,  913, 1270,  906, 1270, 1270,  903, 1270, 1270,  905,
 /*   810 */  1270,  909, 1270,  907, 1270, 1270, 1270, 1270, 1270,  901,
 /*   820 */  1270, 1270,  883,  885,  884, 1270, 1270,  872, 1270, 1270,
 /*   830 */  1270,  874, 1270, 1270,  875, 1270, 1270,  877, 1270, 1270,
 /*   840 */   876, 1270, 1270, 1270,  878, 1270, 1270,  879, 1270, 1270,
 /*   850 */   880,  855,
};
#define YY_SZ_ACTTAB (sizeof(yy_action)/sizeof(yy_action[0]))

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
  "xx_interface_def",  "xx_comment",    "xx_cblock",     "xx_use_aliases_list",
  "xx_interface_body",  "xx_class_body",  "xx_implements_list",  "xx_class_definition",
  "xx_implements",  "xx_interface_definition",  "xx_class_properties_definition",  "xx_class_consts_definition",
  "xx_class_methods_definition",  "xx_interface_methods_definition",  "xx_class_property_definition",  "xx_visibility_list",
  "xx_literal_expr",  "xx_class_property_shortcuts",  "xx_class_property_shortcuts_list",  "xx_class_property_shortcut",
  "xx_class_const_definition",  "xx_class_method_definition",  "xx_interface_method_definition",  "xx_parameter_list",
  "xx_statement_list",  "xx_method_return_type",  "xx_visibility",  "xx_method_return_type_list",
  "xx_method_return_type_item",  "xx_parameter_type",  "xx_parameter_cast",  "xx_parameter_cast_collection",
  "xx_parameter",  "xx_statement",  "xx_let_statement",  "xx_if_statement",
  "xx_loop_statement",  "xx_echo_statement",  "xx_return_statement",  "xx_require_statement",
  "xx_fetch_statement",  "xx_fcall_statement",  "xx_mcall_statement",  "xx_scall_statement",
  "xx_unset_statement",  "xx_throw_statement",  "xx_declare_statement",  "xx_break_statement",
  "xx_continue_statement",  "xx_while_statement",  "xx_do_while_statement",  "xx_try_catch_statement",
  "xx_switch_statement",  "xx_for_statement",  "xx_empty_statement",  "xx_eval_expr",
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
 /*   8 */ "xx_top_statement ::= xx_comment",
 /*   9 */ "xx_top_statement ::= xx_cblock",
 /*  10 */ "xx_namespace_def ::= NAMESPACE IDENTIFIER DOTCOMMA",
 /*  11 */ "xx_namespace_def ::= USE xx_use_aliases_list DOTCOMMA",
 /*  12 */ "xx_use_aliases_list ::= xx_use_aliases_list COMMA xx_use_aliases",
 /*  13 */ "xx_use_aliases_list ::= xx_use_aliases",
 /*  14 */ "xx_use_aliases ::= IDENTIFIER",
 /*  15 */ "xx_use_aliases ::= IDENTIFIER AS IDENTIFIER",
 /*  16 */ "xx_interface_def ::= INTERFACE IDENTIFIER xx_interface_body",
 /*  17 */ "xx_interface_def ::= INTERFACE IDENTIFIER EXTENDS IDENTIFIER xx_interface_body",
 /*  18 */ "xx_class_def ::= CLASS IDENTIFIER xx_class_body",
 /*  19 */ "xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body",
 /*  20 */ "xx_class_def ::= CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  21 */ "xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  22 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER xx_class_body",
 /*  23 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body",
 /*  24 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  25 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER EXTENDS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  26 */ "xx_class_def ::= FINAL CLASS IDENTIFIER xx_class_body",
 /*  27 */ "xx_class_def ::= FINAL CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body",
 /*  28 */ "xx_class_def ::= FINAL CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  29 */ "xx_class_body ::= BRACKET_OPEN BRACKET_CLOSE",
 /*  30 */ "xx_class_body ::= BRACKET_OPEN xx_class_definition BRACKET_CLOSE",
 /*  31 */ "xx_implements_list ::= xx_implements_list COMMA xx_implements",
 /*  32 */ "xx_implements_list ::= xx_implements",
 /*  33 */ "xx_implements ::= IDENTIFIER",
 /*  34 */ "xx_interface_body ::= BRACKET_OPEN BRACKET_CLOSE",
 /*  35 */ "xx_interface_body ::= BRACKET_OPEN xx_interface_definition BRACKET_CLOSE",
 /*  36 */ "xx_class_definition ::= xx_class_properties_definition",
 /*  37 */ "xx_class_definition ::= xx_class_consts_definition",
 /*  38 */ "xx_class_definition ::= xx_class_methods_definition",
 /*  39 */ "xx_class_definition ::= xx_class_properties_definition xx_class_methods_definition",
 /*  40 */ "xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition",
 /*  41 */ "xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition",
 /*  42 */ "xx_class_definition ::= xx_class_consts_definition xx_class_methods_definition",
 /*  43 */ "xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition xx_class_methods_definition",
 /*  44 */ "xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition xx_class_methods_definition",
 /*  45 */ "xx_interface_definition ::= xx_class_consts_definition",
 /*  46 */ "xx_interface_definition ::= xx_interface_methods_definition",
 /*  47 */ "xx_interface_definition ::= xx_class_consts_definition xx_interface_methods_definition",
 /*  48 */ "xx_class_properties_definition ::= xx_class_properties_definition xx_class_property_definition",
 /*  49 */ "xx_class_properties_definition ::= xx_class_property_definition",
 /*  50 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER DOTCOMMA",
 /*  51 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER DOTCOMMA",
 /*  52 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  53 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  54 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA",
 /*  55 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA",
 /*  56 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA",
 /*  57 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA",
 /*  58 */ "xx_class_property_shortcuts ::= BRACKET_OPEN BRACKET_CLOSE",
 /*  59 */ "xx_class_property_shortcuts ::= BRACKET_OPEN xx_class_property_shortcuts_list BRACKET_CLOSE",
 /*  60 */ "xx_class_property_shortcuts_list ::= xx_class_property_shortcuts_list COMMA xx_class_property_shortcut",
 /*  61 */ "xx_class_property_shortcuts_list ::= xx_class_property_shortcut",
 /*  62 */ "xx_class_property_shortcut ::= IDENTIFIER",
 /*  63 */ "xx_class_property_shortcut ::= COMMENT IDENTIFIER",
 /*  64 */ "xx_class_consts_definition ::= xx_class_consts_definition xx_class_const_definition",
 /*  65 */ "xx_class_consts_definition ::= xx_class_const_definition",
 /*  66 */ "xx_class_methods_definition ::= xx_class_methods_definition xx_class_method_definition",
 /*  67 */ "xx_class_methods_definition ::= xx_class_method_definition",
 /*  68 */ "xx_interface_methods_definition ::= xx_interface_methods_definition xx_interface_method_definition",
 /*  69 */ "xx_interface_methods_definition ::= xx_interface_method_definition",
 /*  70 */ "xx_class_const_definition ::= COMMENT CONST CONSTANT ASSIGN xx_literal_expr DOTCOMMA",
 /*  71 */ "xx_class_const_definition ::= CONST CONSTANT ASSIGN xx_literal_expr DOTCOMMA",
 /*  72 */ "xx_class_const_definition ::= COMMENT CONST IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  73 */ "xx_class_const_definition ::= CONST IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  74 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /*  75 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /*  76 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /*  77 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /*  78 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  79 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  80 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /*  81 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /*  82 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /*  83 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /*  84 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  85 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  86 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /*  87 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /*  88 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /*  89 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /*  90 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  91 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  92 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /*  93 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /*  94 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /*  95 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /*  96 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  97 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  98 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /*  99 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 100 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 101 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 102 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /* 103 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /* 104 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /* 105 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /* 106 */ "xx_visibility_list ::= xx_visibility_list xx_visibility",
 /* 107 */ "xx_visibility_list ::= xx_visibility",
 /* 108 */ "xx_visibility ::= PUBLIC",
 /* 109 */ "xx_visibility ::= PROTECTED",
 /* 110 */ "xx_visibility ::= PRIVATE",
 /* 111 */ "xx_visibility ::= STATIC",
 /* 112 */ "xx_visibility ::= SCOPED",
 /* 113 */ "xx_visibility ::= INLINE",
 /* 114 */ "xx_visibility ::= DEPRECATED",
 /* 115 */ "xx_visibility ::= ABSTRACT",
 /* 116 */ "xx_visibility ::= FINAL",
 /* 117 */ "xx_method_return_type ::= VOID",
 /* 118 */ "xx_method_return_type ::= xx_method_return_type_list",
 /* 119 */ "xx_method_return_type_list ::= xx_method_return_type_list BITWISE_OR xx_method_return_type_item",
 /* 120 */ "xx_method_return_type_list ::= xx_method_return_type_item",
 /* 121 */ "xx_method_return_type_item ::= xx_parameter_type",
 /* 122 */ "xx_method_return_type_item ::= NULL",
 /* 123 */ "xx_method_return_type_item ::= THIS",
 /* 124 */ "xx_method_return_type_item ::= xx_parameter_type NOT",
 /* 125 */ "xx_method_return_type_item ::= xx_parameter_cast",
 /* 126 */ "xx_method_return_type_item ::= xx_parameter_cast_collection",
 /* 127 */ "xx_parameter_list ::= xx_parameter_list COMMA xx_parameter",
 /* 128 */ "xx_parameter_list ::= xx_parameter",
 /* 129 */ "xx_parameter ::= IDENTIFIER",
 /* 130 */ "xx_parameter ::= CONST IDENTIFIER",
 /* 131 */ "xx_parameter ::= xx_parameter_type IDENTIFIER",
 /* 132 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER",
 /* 133 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER",
 /* 134 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER",
 /* 135 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER",
 /* 136 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER",
 /* 137 */ "xx_parameter ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 138 */ "xx_parameter ::= CONST IDENTIFIER ASSIGN xx_literal_expr",
 /* 139 */ "xx_parameter ::= xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 140 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 141 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 142 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 143 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 144 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 145 */ "xx_parameter_cast ::= LESS IDENTIFIER GREATER",
 /* 146 */ "xx_parameter_cast_collection ::= LESS IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE GREATER",
 /* 147 */ "xx_parameter_type ::= TYPE_INTEGER",
 /* 148 */ "xx_parameter_type ::= TYPE_UINTEGER",
 /* 149 */ "xx_parameter_type ::= TYPE_LONG",
 /* 150 */ "xx_parameter_type ::= TYPE_ULONG",
 /* 151 */ "xx_parameter_type ::= TYPE_CHAR",
 /* 152 */ "xx_parameter_type ::= TYPE_UCHAR",
 /* 153 */ "xx_parameter_type ::= TYPE_DOUBLE",
 /* 154 */ "xx_parameter_type ::= TYPE_BOOL",
 /* 155 */ "xx_parameter_type ::= TYPE_STRING",
 /* 156 */ "xx_parameter_type ::= TYPE_ARRAY",
 /* 157 */ "xx_parameter_type ::= TYPE_VAR",
 /* 158 */ "xx_parameter_type ::= TYPE_CALLABLE",
 /* 159 */ "xx_parameter_type ::= TYPE_RESOURCE",
 /* 160 */ "xx_parameter_type ::= TYPE_OBJECT",
 /* 161 */ "xx_statement_list ::= xx_statement_list xx_statement",
 /* 162 */ "xx_statement_list ::= xx_statement",
 /* 163 */ "xx_statement ::= xx_cblock",
 /* 164 */ "xx_statement ::= xx_let_statement",
 /* 165 */ "xx_statement ::= xx_if_statement",
 /* 166 */ "xx_statement ::= xx_loop_statement",
 /* 167 */ "xx_statement ::= xx_echo_statement",
 /* 168 */ "xx_statement ::= xx_return_statement",
 /* 169 */ "xx_statement ::= xx_require_statement",
 /* 170 */ "xx_statement ::= xx_fetch_statement",
 /* 171 */ "xx_statement ::= xx_fcall_statement",
 /* 172 */ "xx_statement ::= xx_mcall_statement",
 /* 173 */ "xx_statement ::= xx_scall_statement",
 /* 174 */ "xx_statement ::= xx_unset_statement",
 /* 175 */ "xx_statement ::= xx_throw_statement",
 /* 176 */ "xx_statement ::= xx_declare_statement",
 /* 177 */ "xx_statement ::= xx_break_statement",
 /* 178 */ "xx_statement ::= xx_continue_statement",
 /* 179 */ "xx_statement ::= xx_while_statement",
 /* 180 */ "xx_statement ::= xx_do_while_statement",
 /* 181 */ "xx_statement ::= xx_try_catch_statement",
 /* 182 */ "xx_statement ::= xx_switch_statement",
 /* 183 */ "xx_statement ::= xx_for_statement",
 /* 184 */ "xx_statement ::= xx_comment",
 /* 185 */ "xx_statement ::= xx_empty_statement",
 /* 186 */ "xx_empty_statement ::= DOTCOMMA",
 /* 187 */ "xx_break_statement ::= BREAK DOTCOMMA",
 /* 188 */ "xx_continue_statement ::= CONTINUE DOTCOMMA",
 /* 189 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 190 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements",
 /* 191 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 192 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 193 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 194 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements",
 /* 195 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 196 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 197 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 198 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 199 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 200 */ "xx_elseif_statements ::= xx_elseif_statements xx_elseif_statement",
 /* 201 */ "xx_elseif_statements ::= xx_elseif_statement",
 /* 202 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 203 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 204 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 205 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN xx_case_clauses BRACKET_CLOSE",
 /* 206 */ "xx_case_clauses ::= xx_case_clauses xx_case_clause",
 /* 207 */ "xx_case_clauses ::= xx_case_clause",
 /* 208 */ "xx_case_clause ::= CASE xx_literal_expr COLON",
 /* 209 */ "xx_case_clause ::= CASE xx_literal_expr COLON xx_statement_list",
 /* 210 */ "xx_case_clause ::= DEFAULT COLON xx_statement_list",
 /* 211 */ "xx_loop_statement ::= LOOP BRACKET_OPEN BRACKET_CLOSE",
 /* 212 */ "xx_loop_statement ::= LOOP BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 213 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 214 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 215 */ "xx_do_while_statement ::= DO BRACKET_OPEN BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 216 */ "xx_do_while_statement ::= DO BRACKET_OPEN xx_statement_list BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 217 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN BRACKET_CLOSE",
 /* 218 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 219 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_catch_statement_list",
 /* 220 */ "xx_catch_statement_list ::= xx_catch_statement_list xx_catch_statement",
 /* 221 */ "xx_catch_statement_list ::= xx_catch_statement",
 /* 222 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 223 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN BRACKET_CLOSE",
 /* 224 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN BRACKET_CLOSE",
 /* 225 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 226 */ "xx_catch_classes_list ::= xx_catch_classes_list BITWISE_OR xx_catch_class",
 /* 227 */ "xx_catch_classes_list ::= xx_catch_class",
 /* 228 */ "xx_catch_class ::= IDENTIFIER",
 /* 229 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 230 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 231 */ "xx_for_statement ::= FOR IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 232 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 233 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 234 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 235 */ "xx_let_statement ::= LET xx_let_assignments DOTCOMMA",
 /* 236 */ "xx_let_assignments ::= xx_let_assignments COMMA xx_let_assignment",
 /* 237 */ "xx_let_assignments ::= xx_let_assignment",
 /* 238 */ "xx_assignment_operator ::= ASSIGN",
 /* 239 */ "xx_assignment_operator ::= ADDASSIGN",
 /* 240 */ "xx_assignment_operator ::= SUBASSIGN",
 /* 241 */ "xx_assignment_operator ::= MULASSIGN",
 /* 242 */ "xx_assignment_operator ::= DIVASSIGN",
 /* 243 */ "xx_assignment_operator ::= CONCATASSIGN",
 /* 244 */ "xx_assignment_operator ::= MODASSIGN",
 /* 245 */ "xx_let_assignment ::= IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 246 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 247 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 248 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 249 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 250 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 251 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 252 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 253 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 254 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 255 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 256 */ "xx_let_assignment ::= IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 257 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 258 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 259 */ "xx_array_offset_list ::= xx_array_offset_list xx_array_offset",
 /* 260 */ "xx_array_offset_list ::= xx_array_offset",
 /* 261 */ "xx_array_offset ::= SBRACKET_OPEN xx_index_expr SBRACKET_CLOSE",
 /* 262 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER INCR",
 /* 263 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER DECR",
 /* 264 */ "xx_let_assignment ::= IDENTIFIER INCR",
 /* 265 */ "xx_let_assignment ::= IDENTIFIER DECR",
 /* 266 */ "xx_let_assignment ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 267 */ "xx_let_assignment ::= BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 268 */ "xx_index_expr ::= xx_common_expr",
 /* 269 */ "xx_echo_statement ::= ECHO xx_echo_expressions DOTCOMMA",
 /* 270 */ "xx_echo_expressions ::= xx_echo_expressions COMMA xx_echo_expression",
 /* 271 */ "xx_echo_expressions ::= xx_echo_expression",
 /* 272 */ "xx_echo_expression ::= xx_common_expr",
 /* 273 */ "xx_mcall_statement ::= xx_mcall_expr DOTCOMMA",
 /* 274 */ "xx_fcall_statement ::= xx_fcall_expr DOTCOMMA",
 /* 275 */ "xx_scall_statement ::= xx_scall_expr DOTCOMMA",
 /* 276 */ "xx_fetch_statement ::= xx_fetch_expr DOTCOMMA",
 /* 277 */ "xx_return_statement ::= RETURN xx_common_expr DOTCOMMA",
 /* 278 */ "xx_return_statement ::= RETURN DOTCOMMA",
 /* 279 */ "xx_require_statement ::= REQUIRE xx_common_expr DOTCOMMA",
 /* 280 */ "xx_unset_statement ::= UNSET xx_common_expr DOTCOMMA",
 /* 281 */ "xx_throw_statement ::= THROW xx_common_expr DOTCOMMA",
 /* 282 */ "xx_declare_statement ::= TYPE_INTEGER xx_declare_variable_list DOTCOMMA",
 /* 283 */ "xx_declare_statement ::= TYPE_UINTEGER xx_declare_variable_list DOTCOMMA",
 /* 284 */ "xx_declare_statement ::= TYPE_CHAR xx_declare_variable_list DOTCOMMA",
 /* 285 */ "xx_declare_statement ::= TYPE_UCHAR xx_declare_variable_list DOTCOMMA",
 /* 286 */ "xx_declare_statement ::= TYPE_LONG xx_declare_variable_list DOTCOMMA",
 /* 287 */ "xx_declare_statement ::= TYPE_ULONG xx_declare_variable_list DOTCOMMA",
 /* 288 */ "xx_declare_statement ::= TYPE_DOUBLE xx_declare_variable_list DOTCOMMA",
 /* 289 */ "xx_declare_statement ::= TYPE_STRING xx_declare_variable_list DOTCOMMA",
 /* 290 */ "xx_declare_statement ::= TYPE_BOOL xx_declare_variable_list DOTCOMMA",
 /* 291 */ "xx_declare_statement ::= TYPE_VAR xx_declare_variable_list DOTCOMMA",
 /* 292 */ "xx_declare_statement ::= TYPE_ARRAY xx_declare_variable_list DOTCOMMA",
 /* 293 */ "xx_declare_variable_list ::= xx_declare_variable_list COMMA xx_declare_variable",
 /* 294 */ "xx_declare_variable_list ::= xx_declare_variable",
 /* 295 */ "xx_declare_variable ::= IDENTIFIER",
 /* 296 */ "xx_declare_variable ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 297 */ "xx_assign_expr ::= xx_common_expr",
 /* 298 */ "xx_common_expr ::= NOT xx_common_expr",
 /* 299 */ "xx_common_expr ::= SUB xx_common_expr",
 /* 300 */ "xx_common_expr ::= ISSET xx_common_expr",
 /* 301 */ "xx_common_expr ::= REQUIRE xx_common_expr",
 /* 302 */ "xx_common_expr ::= CLONE xx_common_expr",
 /* 303 */ "xx_common_expr ::= EMPTY xx_common_expr",
 /* 304 */ "xx_common_expr ::= LIKELY xx_common_expr",
 /* 305 */ "xx_common_expr ::= UNLIKELY xx_common_expr",
 /* 306 */ "xx_common_expr ::= xx_common_expr EQUALS xx_common_expr",
 /* 307 */ "xx_common_expr ::= xx_common_expr NOTEQUALS xx_common_expr",
 /* 308 */ "xx_common_expr ::= xx_common_expr IDENTICAL xx_common_expr",
 /* 309 */ "xx_common_expr ::= xx_common_expr NOTIDENTICAL xx_common_expr",
 /* 310 */ "xx_common_expr ::= xx_common_expr LESS xx_common_expr",
 /* 311 */ "xx_common_expr ::= xx_common_expr GREATER xx_common_expr",
 /* 312 */ "xx_common_expr ::= xx_common_expr LESSEQUAL xx_common_expr",
 /* 313 */ "xx_common_expr ::= xx_common_expr GREATEREQUAL xx_common_expr",
 /* 314 */ "xx_common_expr ::= PARENTHESES_OPEN xx_common_expr PARENTHESES_CLOSE",
 /* 315 */ "xx_common_expr ::= PARENTHESES_OPEN xx_parameter_type PARENTHESES_CLOSE xx_common_expr",
 /* 316 */ "xx_common_expr ::= LESS IDENTIFIER GREATER xx_common_expr",
 /* 317 */ "xx_common_expr ::= xx_common_expr ARROW IDENTIFIER",
 /* 318 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 319 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE",
 /* 320 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER",
 /* 321 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 322 */ "xx_common_expr ::= xx_common_expr SBRACKET_OPEN xx_common_expr SBRACKET_CLOSE",
 /* 323 */ "xx_common_expr ::= xx_common_expr ADD xx_common_expr",
 /* 324 */ "xx_common_expr ::= xx_common_expr SUB xx_common_expr",
 /* 325 */ "xx_common_expr ::= xx_common_expr MUL xx_common_expr",
 /* 326 */ "xx_common_expr ::= xx_common_expr DIV xx_common_expr",
 /* 327 */ "xx_common_expr ::= xx_common_expr MOD xx_common_expr",
 /* 328 */ "xx_common_expr ::= xx_common_expr CONCAT xx_common_expr",
 /* 329 */ "xx_common_expr ::= xx_common_expr AND xx_common_expr",
 /* 330 */ "xx_common_expr ::= xx_common_expr OR xx_common_expr",
 /* 331 */ "xx_common_expr ::= xx_common_expr BITWISE_AND xx_common_expr",
 /* 332 */ "xx_common_expr ::= xx_common_expr BITWISE_OR xx_common_expr",
 /* 333 */ "xx_common_expr ::= xx_common_expr BITWISE_XOR xx_common_expr",
 /* 334 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTLEFT xx_common_expr",
 /* 335 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTRIGHT xx_common_expr",
 /* 336 */ "xx_common_expr ::= xx_common_expr INSTANCEOF xx_common_expr",
 /* 337 */ "xx_fetch_expr ::= FETCH IDENTIFIER COMMA xx_common_expr",
 /* 338 */ "xx_common_expr ::= xx_fetch_expr",
 /* 339 */ "xx_common_expr ::= TYPEOF xx_common_expr",
 /* 340 */ "xx_common_expr ::= IDENTIFIER",
 /* 341 */ "xx_common_expr ::= INTEGER",
 /* 342 */ "xx_common_expr ::= STRING",
 /* 343 */ "xx_common_expr ::= CHAR",
 /* 344 */ "xx_common_expr ::= DOUBLE",
 /* 345 */ "xx_common_expr ::= NULL",
 /* 346 */ "xx_common_expr ::= TRUE",
 /* 347 */ "xx_common_expr ::= FALSE",
 /* 348 */ "xx_common_expr ::= CONSTANT",
 /* 349 */ "xx_common_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 350 */ "xx_common_expr ::= SBRACKET_OPEN xx_array_list SBRACKET_CLOSE",
 /* 351 */ "xx_common_expr ::= NEW IDENTIFIER",
 /* 352 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 353 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 354 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 355 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 356 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 357 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 358 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 359 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 360 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 361 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 362 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 363 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 364 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 365 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 366 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 367 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 368 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 369 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 370 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 371 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 372 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 373 */ "xx_common_expr ::= xx_mcall_expr",
 /* 374 */ "xx_common_expr ::= xx_scall_expr",
 /* 375 */ "xx_common_expr ::= xx_fcall_expr",
 /* 376 */ "xx_common_expr ::= xx_common_expr QUESTION xx_common_expr COLON xx_common_expr",
 /* 377 */ "xx_call_parameters ::= xx_call_parameters COMMA xx_call_parameter",
 /* 378 */ "xx_call_parameters ::= xx_call_parameter",
 /* 379 */ "xx_call_parameter ::= xx_common_expr",
 /* 380 */ "xx_call_parameter ::= IDENTIFIER COLON xx_common_expr",
 /* 381 */ "xx_call_parameter ::= BITWISE_AND xx_common_expr",
 /* 382 */ "xx_call_parameter ::= IDENTIFIER COLON BITWISE_AND xx_common_expr",
 /* 383 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 384 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 385 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 386 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 387 */ "xx_array_list ::= xx_array_list COMMA xx_array_item",
 /* 388 */ "xx_array_list ::= xx_array_item",
 /* 389 */ "xx_array_item ::= xx_array_key COLON xx_array_value",
 /* 390 */ "xx_array_item ::= xx_array_value",
 /* 391 */ "xx_array_key ::= CONSTANT",
 /* 392 */ "xx_array_key ::= IDENTIFIER",
 /* 393 */ "xx_array_key ::= STRING",
 /* 394 */ "xx_array_key ::= INTEGER",
 /* 395 */ "xx_array_value ::= xx_common_expr",
 /* 396 */ "xx_literal_expr ::= INTEGER",
 /* 397 */ "xx_literal_expr ::= CHAR",
 /* 398 */ "xx_literal_expr ::= STRING",
 /* 399 */ "xx_literal_expr ::= DOUBLE",
 /* 400 */ "xx_literal_expr ::= NULL",
 /* 401 */ "xx_literal_expr ::= FALSE",
 /* 402 */ "xx_literal_expr ::= TRUE",
 /* 403 */ "xx_literal_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 404 */ "xx_literal_expr ::= CONSTANT",
 /* 405 */ "xx_literal_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 406 */ "xx_literal_expr ::= SBRACKET_OPEN xx_literal_array_list SBRACKET_CLOSE",
 /* 407 */ "xx_literal_array_list ::= xx_literal_array_list COMMA xx_literal_array_item",
 /* 408 */ "xx_literal_array_list ::= xx_literal_array_item",
 /* 409 */ "xx_literal_array_item ::= xx_literal_array_key COLON xx_literal_array_value",
 /* 410 */ "xx_literal_array_item ::= xx_literal_array_value",
 /* 411 */ "xx_literal_array_key ::= IDENTIFIER",
 /* 412 */ "xx_literal_array_key ::= STRING",
 /* 413 */ "xx_literal_array_key ::= INTEGER",
 /* 414 */ "xx_literal_array_value ::= xx_literal_expr",
 /* 415 */ "xx_eval_expr ::= xx_common_expr",
 /* 416 */ "xx_comment ::= COMMENT",
 /* 417 */ "xx_cblock ::= CBLOCK",
};
#endif /* NDEBUG */

/*
** This function returns the symbolic name associated with a token
** value.
*/
const char *xx_TokenName(int tokenType){
#ifndef NDEBUG
  if( tokenType>0 && tokenType<(sizeof(yyTokenName)/sizeof(yyTokenName[0])) ){
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
// 1198 "parser.lemon"
{
	/*if ((yypminor->yy0)) {
		if ((yypminor->yy0)->free_flag) {
			efree((yypminor->yy0)->token);
		}
		efree((yypminor->yy0));
	}*/
}
// 4156 "parser.c"
      break;
    case 122:
// 1211 "parser.lemon"
{ delete (yypminor->yy262); }
// 4161 "parser.c"
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
    if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
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
  { 125, 3 },
  { 125, 3 },
  { 131, 3 },
  { 131, 1 },
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
  { 133, 2 },
  { 133, 3 },
  { 134, 3 },
  { 134, 1 },
  { 136, 1 },
  { 132, 2 },
  { 132, 3 },
  { 135, 1 },
  { 135, 1 },
  { 135, 1 },
  { 135, 2 },
  { 135, 2 },
  { 135, 2 },
  { 135, 2 },
  { 135, 3 },
  { 135, 3 },
  { 137, 1 },
  { 137, 1 },
  { 137, 2 },
  { 138, 2 },
  { 138, 1 },
  { 142, 4 },
  { 142, 3 },
  { 142, 6 },
  { 142, 5 },
  { 142, 5 },
  { 142, 4 },
  { 142, 7 },
  { 142, 6 },
  { 145, 2 },
  { 145, 3 },
  { 146, 3 },
  { 146, 1 },
  { 147, 1 },
  { 147, 2 },
  { 139, 2 },
  { 139, 1 },
  { 140, 2 },
  { 140, 1 },
  { 141, 2 },
  { 141, 1 },
  { 148, 6 },
  { 148, 5 },
  { 148, 6 },
  { 148, 5 },
  { 149, 7 },
  { 149, 6 },
  { 149, 8 },
  { 149, 7 },
  { 149, 8 },
  { 149, 9 },
  { 149, 8 },
  { 149, 7 },
  { 149, 9 },
  { 149, 8 },
  { 149, 9 },
  { 149, 10 },
  { 149, 9 },
  { 149, 8 },
  { 149, 10 },
  { 149, 9 },
  { 149, 10 },
  { 149, 11 },
  { 149, 10 },
  { 149, 9 },
  { 149, 11 },
  { 149, 10 },
  { 149, 11 },
  { 149, 12 },
  { 150, 8 },
  { 150, 9 },
  { 150, 9 },
  { 150, 10 },
  { 150, 6 },
  { 150, 7 },
  { 150, 7 },
  { 150, 8 },
  { 143, 2 },
  { 143, 1 },
  { 154, 1 },
  { 154, 1 },
  { 154, 1 },
  { 154, 1 },
  { 154, 1 },
  { 154, 1 },
  { 154, 1 },
  { 154, 1 },
  { 154, 1 },
  { 153, 1 },
  { 153, 1 },
  { 155, 3 },
  { 155, 1 },
  { 156, 1 },
  { 156, 1 },
  { 156, 1 },
  { 156, 2 },
  { 156, 1 },
  { 156, 1 },
  { 151, 3 },
  { 151, 1 },
  { 160, 1 },
  { 160, 2 },
  { 160, 2 },
  { 160, 3 },
  { 160, 3 },
  { 160, 4 },
  { 160, 2 },
  { 160, 3 },
  { 160, 3 },
  { 160, 4 },
  { 160, 4 },
  { 160, 5 },
  { 160, 5 },
  { 160, 6 },
  { 160, 4 },
  { 160, 5 },
  { 158, 3 },
  { 159, 5 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 157, 1 },
  { 152, 2 },
  { 152, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 161, 1 },
  { 182, 1 },
  { 175, 2 },
  { 176, 2 },
  { 163, 4 },
  { 163, 5 },
  { 163, 7 },
  { 163, 8 },
  { 163, 5 },
  { 163, 6 },
  { 163, 9 },
  { 163, 10 },
  { 163, 8 },
  { 163, 9 },
  { 163, 8 },
  { 184, 2 },
  { 184, 1 },
  { 185, 4 },
  { 185, 5 },
  { 180, 4 },
  { 180, 5 },
  { 186, 2 },
  { 186, 1 },
  { 187, 3 },
  { 187, 4 },
  { 187, 3 },
  { 164, 3 },
  { 164, 4 },
  { 177, 4 },
  { 177, 5 },
  { 178, 6 },
  { 178, 7 },
  { 179, 3 },
  { 179, 4 },
  { 179, 5 },
  { 188, 2 },
  { 188, 1 },
  { 189, 5 },
  { 189, 4 },
  { 189, 6 },
  { 189, 7 },
  { 190, 3 },
  { 190, 1 },
  { 191, 1 },
  { 181, 7 },
  { 181, 6 },
  { 181, 8 },
  { 181, 9 },
  { 181, 8 },
  { 181, 10 },
  { 162, 3 },
  { 193, 3 },
  { 193, 1 },
  { 195, 1 },
  { 195, 1 },
  { 195, 1 },
  { 195, 1 },
  { 195, 1 },
  { 195, 1 },
  { 195, 1 },
  { 194, 3 },
  { 194, 5 },
  { 194, 7 },
  { 194, 7 },
  { 194, 7 },
  { 194, 6 },
  { 194, 8 },
  { 194, 5 },
  { 194, 7 },
  { 194, 6 },
  { 194, 8 },
  { 194, 5 },
  { 194, 4 },
  { 194, 6 },
  { 197, 2 },
  { 197, 1 },
  { 198, 3 },
  { 194, 4 },
  { 194, 4 },
  { 194, 2 },
  { 194, 2 },
  { 194, 5 },
  { 194, 5 },
  { 199, 1 },
  { 165, 3 },
  { 200, 3 },
  { 200, 1 },
  { 201, 1 },
  { 170, 2 },
  { 169, 2 },
  { 171, 2 },
  { 168, 2 },
  { 166, 3 },
  { 166, 2 },
  { 167, 3 },
  { 172, 3 },
  { 173, 3 },
  { 174, 3 },
  { 174, 3 },
  { 174, 3 },
  { 174, 3 },
  { 174, 3 },
  { 174, 3 },
  { 174, 3 },
  { 174, 3 },
  { 174, 3 },
  { 174, 3 },
  { 174, 3 },
  { 206, 3 },
  { 206, 1 },
  { 207, 1 },
  { 207, 3 },
  { 196, 1 },
  { 192, 2 },
  { 192, 2 },
  { 192, 2 },
  { 192, 2 },
  { 192, 2 },
  { 192, 2 },
  { 192, 2 },
  { 192, 2 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 4 },
  { 192, 4 },
  { 192, 3 },
  { 192, 5 },
  { 192, 5 },
  { 192, 3 },
  { 192, 3 },
  { 192, 4 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 192, 3 },
  { 205, 4 },
  { 192, 1 },
  { 192, 2 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 192, 2 },
  { 192, 3 },
  { 192, 2 },
  { 192, 4 },
  { 192, 5 },
  { 192, 4 },
  { 192, 6 },
  { 192, 7 },
  { 203, 4 },
  { 203, 3 },
  { 203, 6 },
  { 203, 5 },
  { 204, 6 },
  { 204, 5 },
  { 204, 8 },
  { 204, 7 },
  { 204, 10 },
  { 204, 9 },
  { 202, 6 },
  { 202, 5 },
  { 202, 8 },
  { 202, 7 },
  { 202, 8 },
  { 202, 7 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 192, 5 },
  { 209, 3 },
  { 209, 1 },
  { 210, 1 },
  { 210, 3 },
  { 210, 2 },
  { 210, 4 },
  { 192, 5 },
  { 192, 6 },
  { 192, 6 },
  { 192, 7 },
  { 208, 3 },
  { 208, 1 },
  { 211, 3 },
  { 211, 1 },
  { 212, 1 },
  { 212, 1 },
  { 212, 1 },
  { 212, 1 },
  { 213, 1 },
  { 144, 1 },
  { 144, 1 },
  { 144, 1 },
  { 144, 1 },
  { 144, 1 },
  { 144, 1 },
  { 144, 1 },
  { 144, 3 },
  { 144, 1 },
  { 144, 2 },
  { 144, 3 },
  { 214, 3 },
  { 214, 1 },
  { 215, 3 },
  { 215, 1 },
  { 216, 1 },
  { 216, 1 },
  { 216, 1 },
  { 217, 1 },
  { 183, 1 },
  { 129, 1 },
  { 130, 1 },
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
        && yyruleno<sizeof(yyRuleName)/sizeof(yyRuleName[0]) ){
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
// 1207 "parser.lemon"
{
	status->ret = yymsp[0].minor.yy262;
}
// 4796 "parser.c"
        break;
      case 1:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 163:
      case 164:
      case 165:
      case 166:
      case 167:
      case 168:
      case 169:
      case 170:
      case 171:
      case 172:
      case 173:
      case 174:
      case 175:
      case 176:
      case 177:
      case 178:
      case 179:
      case 180:
      case 181:
      case 182:
      case 183:
      case 184:
      case 185:
      case 268:
      case 297:
      case 338:
      case 373:
      case 374:
      case 375:
      case 395:
      case 414:
      case 415:
// 1213 "parser.lemon"
{
	yygotominor.yy262 = yymsp[0].minor.yy262;
}
// 4841 "parser.c"
        break;
      case 2:
      case 48:
      case 64:
      case 66:
      case 68:
      case 106:
      case 161:
      case 200:
      case 206:
      case 220:
      case 259:
// 1217 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_list(yymsp[-1].minor.yy262, yymsp[0].minor.yy262);
}
// 4858 "parser.c"
        break;
      case 3:
      case 13:
      case 32:
      case 49:
      case 61:
      case 65:
      case 67:
      case 69:
      case 107:
      case 120:
      case 128:
      case 162:
      case 201:
      case 207:
      case 221:
      case 227:
      case 237:
      case 260:
      case 271:
      case 294:
      case 378:
      case 388:
      case 408:
// 1221 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_list(NULL, yymsp[0].minor.yy262);
}
// 4887 "parser.c"
        break;
      case 10:
// 1249 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_namespace(yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(43,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 4896 "parser.c"
        break;
      case 11:
// 1253 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_use_aliases(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(46,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 4905 "parser.c"
        break;
      case 12:
      case 31:
      case 60:
      case 127:
      case 236:
      case 270:
      case 293:
      case 377:
      case 387:
      case 407:
// 1257 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_list(yymsp[-2].minor.yy262, yymsp[0].minor.yy262);
  yy_destructor(6,&yymsp[-1].minor);
}
// 4922 "parser.c"
        break;
      case 14:
// 1265 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_use_aliases_item(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 4929 "parser.c"
        break;
      case 15:
// 1269 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_use_aliases_item(yymsp[-2].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
  yy_destructor(47,&yymsp[-1].minor);
}
// 4937 "parser.c"
        break;
      case 16:
// 1273 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_interface(yymsp[-1].minor.yy0, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(48,&yymsp[-2].minor);
}
// 4945 "parser.c"
        break;
      case 17:
// 1277 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_interface(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(48,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 4954 "parser.c"
        break;
      case 18:
// 1281 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy262, 0, 0, NULL, NULL, status->scanner_state);
  yy_destructor(50,&yymsp[-2].minor);
}
// 4962 "parser.c"
        break;
      case 19:
// 1285 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 0, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 4971 "parser.c"
        break;
      case 20:
// 1289 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 0, 0, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 4980 "parser.c"
        break;
      case 21:
// 1293 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy262, 0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(50,&yymsp[-6].minor);
  yy_destructor(49,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 4990 "parser.c"
        break;
      case 22:
// 1297 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy262, 1, 0, NULL, NULL, status->scanner_state);
  yy_destructor(52,&yymsp[-3].minor);
  yy_destructor(50,&yymsp[-2].minor);
}
// 4999 "parser.c"
        break;
      case 23:
// 1301 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 1, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(52,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5009 "parser.c"
        break;
      case 24:
// 1305 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 1, 0, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(52,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5019 "parser.c"
        break;
      case 25:
// 1309 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy262, 1, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(52,&yymsp[-7].minor);
  yy_destructor(50,&yymsp[-6].minor);
  yy_destructor(49,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5030 "parser.c"
        break;
      case 26:
// 1313 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy262, 0, 1, NULL, NULL, status->scanner_state);
  yy_destructor(53,&yymsp[-3].minor);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5039 "parser.c"
        break;
      case 27:
// 1317 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 0, 1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(53,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5049 "parser.c"
        break;
      case 28:
// 1321 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 0, 1, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(53,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5059 "parser.c"
        break;
      case 29:
      case 58:
// 1325 "parser.lemon"
{
	yygotominor.yy262 = NULL;
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5069 "parser.c"
        break;
      case 30:
      case 59:
// 1329 "parser.lemon"
{
	yygotominor.yy262 = yymsp[-1].minor.yy262;
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5079 "parser.c"
        break;
      case 33:
      case 228:
      case 340:
      case 392:
      case 411:
// 1341 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state);
}
// 5090 "parser.c"
        break;
      case 34:
// 1345 "parser.lemon"
{
  yygotominor.yy262 = NULL;
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5099 "parser.c"
        break;
      case 35:
// 1349 "parser.lemon"
{
  yygotominor.yy262 = yymsp[-1].minor.yy262;
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5108 "parser.c"
        break;
      case 36:
// 1353 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
}
// 5115 "parser.c"
        break;
      case 37:
// 1357 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 5122 "parser.c"
        break;
      case 38:
// 1361 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(NULL, yymsp[0].minor.yy262, NULL, status->scanner_state);
}
// 5129 "parser.c"
        break;
      case 39:
// 1365 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[-1].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
}
// 5136 "parser.c"
        break;
      case 40:
// 1369 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[-1].minor.yy262, NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 5143 "parser.c"
        break;
      case 41:
// 1373 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[0].minor.yy262, NULL, yymsp[-1].minor.yy262, status->scanner_state);
}
// 5150 "parser.c"
        break;
      case 42:
// 1377 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(NULL, yymsp[0].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
}
// 5157 "parser.c"
        break;
      case 43:
// 1381 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[-2].minor.yy262, yymsp[0].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
}
// 5164 "parser.c"
        break;
      case 44:
// 1385 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[-1].minor.yy262, yymsp[0].minor.yy262, yymsp[-2].minor.yy262, status->scanner_state);
}
// 5171 "parser.c"
        break;
      case 45:
// 1389 "parser.lemon"
{
  yygotominor.yy262 = xx_ret_interface_definition(NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 5178 "parser.c"
        break;
      case 46:
// 1393 "parser.lemon"
{
  yygotominor.yy262 = xx_ret_interface_definition(yymsp[0].minor.yy262, NULL, status->scanner_state);
}
// 5185 "parser.c"
        break;
      case 47:
// 1397 "parser.lemon"
{
  yygotominor.yy262 = xx_ret_interface_definition(yymsp[0].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
}
// 5192 "parser.c"
        break;
      case 50:
// 1410 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-2].minor.yy262, yymsp[-1].minor.yy0, NULL, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5200 "parser.c"
        break;
      case 51:
// 1414 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-2].minor.yy262, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5208 "parser.c"
        break;
      case 52:
// 1418 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-4].minor.yy262, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, NULL, status->scanner_state);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5217 "parser.c"
        break;
      case 53:
// 1422 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-4].minor.yy262, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5226 "parser.c"
        break;
      case 54:
// 1426 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-3].minor.yy262, yymsp[-2].minor.yy0, NULL, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5234 "parser.c"
        break;
      case 55:
// 1430 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-3].minor.yy262, yymsp[-2].minor.yy0, NULL, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5242 "parser.c"
        break;
      case 56:
// 1434 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-5].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy262, yymsp[-6].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(57,&yymsp[-3].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5251 "parser.c"
        break;
      case 57:
// 1438 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-5].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy262, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(57,&yymsp[-3].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5260 "parser.c"
        break;
      case 62:
// 1458 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_property_shortcut(NULL, yymsp[0].minor.yy0, status->scanner_state);
}
// 5267 "parser.c"
        break;
      case 63:
// 1462 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_property_shortcut(yymsp[-1].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
}
// 5274 "parser.c"
        break;
      case 70:
      case 72:
// 1491 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5285 "parser.c"
        break;
      case 71:
      case 73:
// 1495 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5296 "parser.c"
        break;
      case 74:
// 1511 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-6].minor.yy262, yymsp[-4].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5308 "parser.c"
        break;
      case 75:
      case 102:
// 1516 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-5].minor.yy262, yymsp[-3].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5320 "parser.c"
        break;
      case 76:
// 1521 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, yymsp[-3].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5332 "parser.c"
        break;
      case 77:
      case 103:
// 1526 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-6].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5344 "parser.c"
        break;
      case 78:
// 1531 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5356 "parser.c"
        break;
      case 79:
// 1535 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy262, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5368 "parser.c"
        break;
      case 80:
// 1539 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-6].minor.yy262, yymsp[-4].minor.yy0, NULL, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5380 "parser.c"
        break;
      case 81:
      case 104:
// 1543 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-5].minor.yy262, yymsp[-3].minor.yy0, NULL, NULL, yymsp[-6].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5392 "parser.c"
        break;
      case 82:
// 1547 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, yymsp[-3].minor.yy262, NULL, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5404 "parser.c"
        break;
      case 83:
      case 105:
// 1551 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-6].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy262, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5416 "parser.c"
        break;
      case 84:
// 1555 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy262, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5428 "parser.c"
        break;
      case 85:
// 1559 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy262, yymsp[-1].minor.yy262, yymsp[-9].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5440 "parser.c"
        break;
      case 86:
// 1563 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, NULL, NULL, NULL, yymsp[-2].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5453 "parser.c"
        break;
      case 87:
      case 98:
// 1567 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, NULL, NULL, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5466 "parser.c"
        break;
      case 88:
// 1571 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-9].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy262, NULL, NULL, yymsp[-2].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5479 "parser.c"
        break;
      case 89:
      case 99:
// 1575 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy262, NULL, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5492 "parser.c"
        break;
      case 90:
// 1579 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-9].minor.yy262, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy262, NULL, yymsp[-3].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5505 "parser.c"
        break;
      case 91:
// 1583 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-10].minor.yy262, yymsp[-8].minor.yy0, yymsp[-6].minor.yy262, yymsp[-1].minor.yy262, NULL, yymsp[-3].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-9].minor);
  yy_destructor(61,&yymsp[-7].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5518 "parser.c"
        break;
      case 92:
// 1587 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, NULL, NULL, yymsp[-9].minor.yy0, yymsp[-2].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5531 "parser.c"
        break;
      case 93:
      case 100:
// 1591 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, NULL, NULL, yymsp[-8].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5544 "parser.c"
        break;
      case 94:
// 1595 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-9].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy262, NULL, yymsp[-10].minor.yy0, yymsp[-2].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5557 "parser.c"
        break;
      case 95:
      case 101:
// 1599 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy262, NULL, yymsp[-9].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5570 "parser.c"
        break;
      case 96:
// 1603 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-9].minor.yy262, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy262, yymsp[-10].minor.yy0, yymsp[-3].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5583 "parser.c"
        break;
      case 97:
// 1607 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-10].minor.yy262, yymsp[-8].minor.yy0, yymsp[-6].minor.yy262, yymsp[-1].minor.yy262, yymsp[-11].minor.yy0, yymsp[-3].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-9].minor);
  yy_destructor(61,&yymsp[-7].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5596 "parser.c"
        break;
      case 108:
// 1653 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("public");
  yy_destructor(1,&yymsp[0].minor);
}
// 5604 "parser.c"
        break;
      case 109:
// 1657 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("protected");
  yy_destructor(2,&yymsp[0].minor);
}
// 5612 "parser.c"
        break;
      case 110:
// 1661 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("private");
  yy_destructor(4,&yymsp[0].minor);
}
// 5620 "parser.c"
        break;
      case 111:
// 1665 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("static");
  yy_destructor(3,&yymsp[0].minor);
}
// 5628 "parser.c"
        break;
      case 112:
// 1669 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("scoped");
  yy_destructor(5,&yymsp[0].minor);
}
// 5636 "parser.c"
        break;
      case 113:
// 1673 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("inline");
  yy_destructor(62,&yymsp[0].minor);
}
// 5644 "parser.c"
        break;
      case 114:
// 1677 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("deprecated");
  yy_destructor(63,&yymsp[0].minor);
}
// 5652 "parser.c"
        break;
      case 115:
// 1681 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("abstract");
  yy_destructor(52,&yymsp[0].minor);
}
// 5660 "parser.c"
        break;
      case 116:
// 1685 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("final");
  yy_destructor(53,&yymsp[0].minor);
}
// 5668 "parser.c"
        break;
      case 117:
// 1690 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type(1, NULL, status->scanner_state);
  yy_destructor(64,&yymsp[0].minor);
}
// 5676 "parser.c"
        break;
      case 118:
// 1694 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type(0, yymsp[0].minor.yy262, status->scanner_state);
}
// 5683 "parser.c"
        break;
      case 119:
      case 226:
// 1698 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_list(yymsp[-2].minor.yy262, yymsp[0].minor.yy262);
  yy_destructor(14,&yymsp[-1].minor);
}
// 5692 "parser.c"
        break;
      case 121:
// 1706 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(yymsp[0].minor.yy262, NULL, 0, 0, status->scanner_state);
}
// 5699 "parser.c"
        break;
      case 122:
// 1710 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_NULL), NULL, 0, 0, status->scanner_state);
  yy_destructor(65,&yymsp[0].minor);
}
// 5707 "parser.c"
        break;
      case 123:
// 1714 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_THIS), NULL, 0, 0, status->scanner_state);
  yy_destructor(66,&yymsp[0].minor);
}
// 5715 "parser.c"
        break;
      case 124:
// 1718 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(yymsp[-1].minor.yy262, NULL, 1, 0, status->scanner_state);
  yy_destructor(39,&yymsp[0].minor);
}
// 5723 "parser.c"
        break;
      case 125:
// 1722 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy262, 0, 0, status->scanner_state);
}
// 5730 "parser.c"
        break;
      case 126:
// 1726 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy262, 0, 1, status->scanner_state);
}
// 5737 "parser.c"
        break;
      case 129:
// 1740 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 5744 "parser.c"
        break;
      case 130:
// 1744 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-1].minor);
}
// 5752 "parser.c"
        break;
      case 131:
// 1748 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, yymsp[-1].minor.yy262, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 5759 "parser.c"
        break;
      case 132:
// 1752 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, yymsp[-1].minor.yy262, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-2].minor);
}
// 5767 "parser.c"
        break;
      case 133:
// 1756 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, yymsp[-2].minor.yy262, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(39,&yymsp[-1].minor);
}
// 5775 "parser.c"
        break;
      case 134:
// 1760 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, yymsp[-2].minor.yy262, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(58,&yymsp[-3].minor);
  yy_destructor(39,&yymsp[-1].minor);
}
// 5784 "parser.c"
        break;
      case 135:
// 1764 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, NULL, yymsp[-1].minor.yy262, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 5791 "parser.c"
        break;
      case 136:
// 1768 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, NULL, yymsp[-1].minor.yy262, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-2].minor);
}
// 5799 "parser.c"
        break;
      case 137:
// 1772 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 5807 "parser.c"
        break;
      case 138:
// 1776 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 5816 "parser.c"
        break;
      case 139:
// 1780 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, yymsp[-3].minor.yy262, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 5824 "parser.c"
        break;
      case 140:
// 1784 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, yymsp[-3].minor.yy262, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 5833 "parser.c"
        break;
      case 141:
// 1788 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, yymsp[-4].minor.yy262, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 1, status->scanner_state);
  yy_destructor(39,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 5842 "parser.c"
        break;
      case 142:
// 1792 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, yymsp[-4].minor.yy262, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 1, status->scanner_state);
  yy_destructor(58,&yymsp[-5].minor);
  yy_destructor(39,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 5852 "parser.c"
        break;
      case 143:
// 1796 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, NULL, yymsp[-3].minor.yy262, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 5860 "parser.c"
        break;
      case 144:
// 1800 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, NULL, yymsp[-3].minor.yy262, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 5869 "parser.c"
        break;
      case 145:
// 1805 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(21,&yymsp[-2].minor);
  yy_destructor(22,&yymsp[0].minor);
}
// 5878 "parser.c"
        break;
      case 146:
// 1809 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state);
  yy_destructor(21,&yymsp[-4].minor);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[-1].minor);
  yy_destructor(22,&yymsp[0].minor);
}
// 5889 "parser.c"
        break;
      case 147:
// 1813 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_INTEGER);
  yy_destructor(68,&yymsp[0].minor);
}
// 5897 "parser.c"
        break;
      case 148:
// 1817 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_UINTEGER);
  yy_destructor(69,&yymsp[0].minor);
}
// 5905 "parser.c"
        break;
      case 149:
// 1821 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_LONG);
  yy_destructor(70,&yymsp[0].minor);
}
// 5913 "parser.c"
        break;
      case 150:
// 1825 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_ULONG);
  yy_destructor(71,&yymsp[0].minor);
}
// 5921 "parser.c"
        break;
      case 151:
// 1829 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_CHAR);
  yy_destructor(72,&yymsp[0].minor);
}
// 5929 "parser.c"
        break;
      case 152:
// 1833 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_UCHAR);
  yy_destructor(73,&yymsp[0].minor);
}
// 5937 "parser.c"
        break;
      case 153:
// 1837 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_DOUBLE);
  yy_destructor(74,&yymsp[0].minor);
}
// 5945 "parser.c"
        break;
      case 154:
// 1841 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_BOOL);
  yy_destructor(75,&yymsp[0].minor);
}
// 5953 "parser.c"
        break;
      case 155:
// 1845 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_STRING);
  yy_destructor(76,&yymsp[0].minor);
}
// 5961 "parser.c"
        break;
      case 156:
// 1849 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_ARRAY);
  yy_destructor(77,&yymsp[0].minor);
}
// 5969 "parser.c"
        break;
      case 157:
// 1853 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_VAR);
  yy_destructor(78,&yymsp[0].minor);
}
// 5977 "parser.c"
        break;
      case 158:
// 1857 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_CALLABLE);
  yy_destructor(79,&yymsp[0].minor);
}
// 5985 "parser.c"
        break;
      case 159:
// 1861 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_RESOURCE);
  yy_destructor(80,&yymsp[0].minor);
}
// 5993 "parser.c"
        break;
      case 160:
// 1865 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_OBJECT);
  yy_destructor(81,&yymsp[0].minor);
}
// 6001 "parser.c"
        break;
      case 186:
// 1971 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_empty_statement(status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 6009 "parser.c"
        break;
      case 187:
// 1975 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_break_statement(status->scanner_state);
  yy_destructor(82,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6018 "parser.c"
        break;
      case 188:
// 1979 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_continue_statement(status->scanner_state);
  yy_destructor(83,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6027 "parser.c"
        break;
      case 189:
// 1984 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-2].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6037 "parser.c"
        break;
      case 190:
// 1989 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-3].minor.yy262, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(84,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6047 "parser.c"
        break;
      case 191:
// 1994 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-5].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6060 "parser.c"
        break;
      case 192:
// 1999 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-6].minor.yy262, NULL, yymsp[-3].minor.yy262, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6073 "parser.c"
        break;
      case 193:
// 2004 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6083 "parser.c"
        break;
      case 194:
// 2009 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-4].minor.yy262, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-3].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6093 "parser.c"
        break;
      case 195:
// 2014 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-7].minor.yy262, yymsp[-5].minor.yy262, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(84,&yymsp[-8].minor);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6106 "parser.c"
        break;
      case 196:
// 2019 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-8].minor.yy262, yymsp[-6].minor.yy262, yymsp[-4].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(84,&yymsp[-9].minor);
  yy_destructor(54,&yymsp[-7].minor);
  yy_destructor(55,&yymsp[-5].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6119 "parser.c"
        break;
      case 197:
// 2024 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-6].minor.yy262, yymsp[-4].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6132 "parser.c"
        break;
      case 198:
// 2029 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-7].minor.yy262, yymsp[-5].minor.yy262, yymsp[-3].minor.yy262, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-8].minor);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6145 "parser.c"
        break;
      case 199:
// 2034 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-6].minor.yy262, NULL, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6158 "parser.c"
        break;
      case 202:
// 2047 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-2].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(86,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6168 "parser.c"
        break;
      case 203:
// 2052 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(86,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6178 "parser.c"
        break;
      case 204:
// 2056 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_switch_statement(yymsp[-2].minor.yy262, NULL, status->scanner_state);
  yy_destructor(87,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6188 "parser.c"
        break;
      case 205:
// 2060 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_switch_statement(yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(87,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6198 "parser.c"
        break;
      case 208:
// 2072 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_case_clause(yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(88,&yymsp[-2].minor);
  yy_destructor(89,&yymsp[0].minor);
}
// 6207 "parser.c"
        break;
      case 209:
// 2076 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_case_clause(yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(88,&yymsp[-3].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 6216 "parser.c"
        break;
      case 210:
// 2080 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_case_clause(NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(90,&yymsp[-2].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 6225 "parser.c"
        break;
      case 211:
// 2084 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_loop_statement(NULL, status->scanner_state);
  yy_destructor(91,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6235 "parser.c"
        break;
      case 212:
// 2088 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_loop_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(91,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6245 "parser.c"
        break;
      case 213:
// 2092 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_while_statement(yymsp[-2].minor.yy262, NULL, status->scanner_state);
  yy_destructor(92,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6255 "parser.c"
        break;
      case 214:
// 2096 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_while_statement(yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(92,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6265 "parser.c"
        break;
      case 215:
// 2100 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_do_while_statement(yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(93,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(92,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6277 "parser.c"
        break;
      case 216:
// 2104 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_do_while_statement(yymsp[-1].minor.yy262, yymsp[-4].minor.yy262, status->scanner_state);
  yy_destructor(93,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(92,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6289 "parser.c"
        break;
      case 217:
// 2108 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_try_catch_statement(NULL, NULL, status->scanner_state);
  yy_destructor(94,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6299 "parser.c"
        break;
      case 218:
// 2112 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_try_catch_statement(yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(94,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6309 "parser.c"
        break;
      case 219:
// 2116 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_try_catch_statement(yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(94,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-3].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6319 "parser.c"
        break;
      case 222:
// 2128 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_catch_statement(yymsp[-3].minor.yy262, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(95,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6329 "parser.c"
        break;
      case 223:
// 2132 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_catch_statement(yymsp[-2].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(95,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6339 "parser.c"
        break;
      case 224:
// 2136 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_catch_statement(yymsp[-4].minor.yy262, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(95,&yymsp[-5].minor);
  yy_destructor(6,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6350 "parser.c"
        break;
      case 225:
// 2140 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_catch_statement(yymsp[-5].minor.yy262, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state), yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(95,&yymsp[-6].minor);
  yy_destructor(6,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6361 "parser.c"
        break;
      case 229:
// 2156 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-3].minor.yy262, NULL, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(96,&yymsp[-6].minor);
  yy_destructor(97,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6372 "parser.c"
        break;
      case 230:
// 2160 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-2].minor.yy262, NULL, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(96,&yymsp[-5].minor);
  yy_destructor(97,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6383 "parser.c"
        break;
      case 231:
// 2164 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-3].minor.yy262, NULL, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(96,&yymsp[-7].minor);
  yy_destructor(97,&yymsp[-5].minor);
  yy_destructor(98,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6395 "parser.c"
        break;
      case 232:
// 2168 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-3].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(96,&yymsp[-8].minor);
  yy_destructor(6,&yymsp[-6].minor);
  yy_destructor(97,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6407 "parser.c"
        break;
      case 233:
// 2172 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-2].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(96,&yymsp[-7].minor);
  yy_destructor(6,&yymsp[-5].minor);
  yy_destructor(97,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6419 "parser.c"
        break;
      case 234:
// 2176 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-3].minor.yy262, yymsp[-8].minor.yy0, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(96,&yymsp[-9].minor);
  yy_destructor(6,&yymsp[-7].minor);
  yy_destructor(97,&yymsp[-5].minor);
  yy_destructor(98,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6432 "parser.c"
        break;
      case 235:
// 2180 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(99,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6441 "parser.c"
        break;
      case 238:
// 2193 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("assign");
  yy_destructor(57,&yymsp[0].minor);
}
// 6449 "parser.c"
        break;
      case 239:
// 2198 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("add-assign");
  yy_destructor(100,&yymsp[0].minor);
}
// 6457 "parser.c"
        break;
      case 240:
// 2203 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("sub-assign");
  yy_destructor(101,&yymsp[0].minor);
}
// 6465 "parser.c"
        break;
      case 241:
// 2207 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("mul-assign");
  yy_destructor(102,&yymsp[0].minor);
}
// 6473 "parser.c"
        break;
      case 242:
// 2211 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("div-assign");
  yy_destructor(103,&yymsp[0].minor);
}
// 6481 "parser.c"
        break;
      case 243:
// 2215 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("concat-assign");
  yy_destructor(104,&yymsp[0].minor);
}
// 6489 "parser.c"
        break;
      case 244:
// 2219 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("mod-assign");
  yy_destructor(105,&yymsp[0].minor);
}
// 6497 "parser.c"
        break;
      case 245:
// 2224 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("variable", yymsp[-1].minor.yy262, yymsp[-2].minor.yy0, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 6504 "parser.c"
        break;
      case 246:
// 2229 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property", yymsp[-1].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
}
// 6512 "parser.c"
        break;
      case 247:
// 2234 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("variable-dynamic-object-property", yymsp[-1].minor.yy262, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 6522 "parser.c"
        break;
      case 248:
// 2239 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("string-dynamic-object-property", yymsp[-1].minor.yy262, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 6532 "parser.c"
        break;
      case 249:
// 2244 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-append", yymsp[-1].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6542 "parser.c"
        break;
      case 250:
// 2249 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-array-index", yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-4].minor);
}
// 6550 "parser.c"
        break;
      case 251:
// 2253 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-array-index-append", yymsp[-1].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6560 "parser.c"
        break;
      case 252:
// 2258 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("static-property", yymsp[-1].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-3].minor);
}
// 6568 "parser.c"
        break;
      case 253:
// 2263 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("static-property-append", yymsp[-1].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-5].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6578 "parser.c"
        break;
      case 254:
// 2268 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("static-property-array-index", yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-4].minor);
}
// 6586 "parser.c"
        break;
      case 255:
// 2273 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("static-property-array-index-append", yymsp[-1].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-6].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6596 "parser.c"
        break;
      case 256:
// 2278 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("variable-append", yymsp[-1].minor.yy262, yymsp[-4].minor.yy0, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6605 "parser.c"
        break;
      case 257:
// 2283 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("array-index", yymsp[-1].minor.yy262, yymsp[-3].minor.yy0, NULL, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
}
// 6612 "parser.c"
        break;
      case 258:
// 2288 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("array-index-append", yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, NULL, yymsp[-4].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6621 "parser.c"
        break;
      case 261:
// 2300 "parser.lemon"
{
	yygotominor.yy262 = yymsp[-1].minor.yy262;
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 6630 "parser.c"
        break;
      case 262:
// 2305 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-incr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(108,&yymsp[0].minor);
}
// 6639 "parser.c"
        break;
      case 263:
// 2310 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-decr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(109,&yymsp[0].minor);
}
// 6648 "parser.c"
        break;
      case 264:
// 2315 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("incr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(108,&yymsp[0].minor);
}
// 6656 "parser.c"
        break;
      case 265:
// 2320 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("decr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(109,&yymsp[0].minor);
}
// 6664 "parser.c"
        break;
      case 266:
// 2325 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("dynamic-variable", yymsp[-1].minor.yy262, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 6673 "parser.c"
        break;
      case 267:
// 2330 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("dynamic-variable-string", yymsp[-1].minor.yy262, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 6682 "parser.c"
        break;
      case 269:
// 2338 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_echo_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(110,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6691 "parser.c"
        break;
      case 272:
// 2350 "parser.lemon"
{
	yygotominor.yy262 = yymsp[0].minor.yy262;;
}
// 6698 "parser.c"
        break;
      case 273:
// 2355 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 6706 "parser.c"
        break;
      case 274:
// 2360 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 6714 "parser.c"
        break;
      case 275:
// 2365 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 6722 "parser.c"
        break;
      case 276:
// 2370 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fetch_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 6730 "parser.c"
        break;
      case 277:
// 2375 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(111,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6739 "parser.c"
        break;
      case 278:
// 2380 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_statement(NULL, status->scanner_state);
  yy_destructor(111,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6748 "parser.c"
        break;
      case 279:
// 2385 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_require_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(7,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6757 "parser.c"
        break;
      case 280:
// 2390 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_unset_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(112,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6766 "parser.c"
        break;
      case 281:
// 2395 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_throw_exception(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(113,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6775 "parser.c"
        break;
      case 282:
// 2399 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_INTEGER, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(68,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6784 "parser.c"
        break;
      case 283:
// 2403 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_UINTEGER, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(69,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6793 "parser.c"
        break;
      case 284:
// 2407 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_CHAR, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(72,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6802 "parser.c"
        break;
      case 285:
// 2411 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_UCHAR, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(73,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6811 "parser.c"
        break;
      case 286:
// 2415 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_LONG, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(70,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6820 "parser.c"
        break;
      case 287:
// 2419 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_ULONG, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(71,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6829 "parser.c"
        break;
      case 288:
// 2423 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_DOUBLE, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(74,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6838 "parser.c"
        break;
      case 289:
// 2427 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_STRING, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(76,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6847 "parser.c"
        break;
      case 290:
// 2431 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_BOOL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(75,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6856 "parser.c"
        break;
      case 291:
// 2435 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_VAR, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(78,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6865 "parser.c"
        break;
      case 292:
// 2439 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_ARRAY, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(77,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6874 "parser.c"
        break;
      case 295:
// 2451 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_variable(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 6881 "parser.c"
        break;
      case 296:
// 2455 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_variable(yymsp[-2].minor.yy0, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6889 "parser.c"
        break;
      case 298:
// 2463 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("not", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(39,&yymsp[-1].minor);
}
// 6897 "parser.c"
        break;
      case 299:
// 2467 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("minus", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(28,&yymsp[-1].minor);
}
// 6905 "parser.c"
        break;
      case 300:
// 2471 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("isset", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(33,&yymsp[-1].minor);
}
// 6913 "parser.c"
        break;
      case 301:
// 2475 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("require", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(7,&yymsp[-1].minor);
}
// 6921 "parser.c"
        break;
      case 302:
// 2479 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("clone", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(37,&yymsp[-1].minor);
}
// 6929 "parser.c"
        break;
      case 303:
// 2483 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("empty", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(35,&yymsp[-1].minor);
}
// 6937 "parser.c"
        break;
      case 304:
// 2487 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("likely", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(9,&yymsp[-1].minor);
}
// 6945 "parser.c"
        break;
      case 305:
// 2491 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("unlikely", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(10,&yymsp[-1].minor);
}
// 6953 "parser.c"
        break;
      case 306:
// 2495 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("equals", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(19,&yymsp[-1].minor);
}
// 6961 "parser.c"
        break;
      case 307:
// 2499 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("not-equals", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(26,&yymsp[-1].minor);
}
// 6969 "parser.c"
        break;
      case 308:
// 2503 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("identical", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(20,&yymsp[-1].minor);
}
// 6977 "parser.c"
        break;
      case 309:
// 2507 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("not-identical", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(25,&yymsp[-1].minor);
}
// 6985 "parser.c"
        break;
      case 310:
// 2511 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("less", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(21,&yymsp[-1].minor);
}
// 6993 "parser.c"
        break;
      case 311:
// 2515 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("greater", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(22,&yymsp[-1].minor);
}
// 7001 "parser.c"
        break;
      case 312:
// 2519 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("less-equal", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(23,&yymsp[-1].minor);
}
// 7009 "parser.c"
        break;
      case 313:
// 2523 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("greater-equal", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(24,&yymsp[-1].minor);
}
// 7017 "parser.c"
        break;
      case 314:
// 2527 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("list", yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7026 "parser.c"
        break;
      case 315:
// 2531 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("cast", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
}
// 7035 "parser.c"
        break;
      case 316:
// 2535 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("type-hint", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(21,&yymsp[-3].minor);
  yy_destructor(22,&yymsp[-1].minor);
}
// 7044 "parser.c"
        break;
      case 317:
// 2539 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("property-access", yymsp[-2].minor.yy262, xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-1].minor);
}
// 7052 "parser.c"
        break;
      case 318:
// 2543 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("property-dynamic-access", yymsp[-4].minor.yy262, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7062 "parser.c"
        break;
      case 319:
// 2547 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("property-string-access", yymsp[-4].minor.yy262, xx_ret_literal(XX_T_STRING, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7072 "parser.c"
        break;
      case 320:
// 2551 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("static-property-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-1].minor);
}
// 7080 "parser.c"
        break;
      case 321:
      case 403:
// 2555 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("static-constant-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-1].minor);
}
// 7089 "parser.c"
        break;
      case 322:
// 2564 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("array-access", yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7098 "parser.c"
        break;
      case 323:
// 2569 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("add", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(27,&yymsp[-1].minor);
}
// 7106 "parser.c"
        break;
      case 324:
// 2574 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("sub", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(28,&yymsp[-1].minor);
}
// 7114 "parser.c"
        break;
      case 325:
// 2579 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("mul", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(30,&yymsp[-1].minor);
}
// 7122 "parser.c"
        break;
      case 326:
// 2584 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("div", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(31,&yymsp[-1].minor);
}
// 7130 "parser.c"
        break;
      case 327:
// 2589 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("mod", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(32,&yymsp[-1].minor);
}
// 7138 "parser.c"
        break;
      case 328:
// 2594 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("concat", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(29,&yymsp[-1].minor);
}
// 7146 "parser.c"
        break;
      case 329:
// 2599 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("and", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(13,&yymsp[-1].minor);
}
// 7154 "parser.c"
        break;
      case 330:
// 2604 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("or", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(12,&yymsp[-1].minor);
}
// 7162 "parser.c"
        break;
      case 331:
// 2609 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_and", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(15,&yymsp[-1].minor);
}
// 7170 "parser.c"
        break;
      case 332:
// 2614 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_or", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(14,&yymsp[-1].minor);
}
// 7178 "parser.c"
        break;
      case 333:
// 2619 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_xor", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(16,&yymsp[-1].minor);
}
// 7186 "parser.c"
        break;
      case 334:
// 2624 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_shiftleft", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(17,&yymsp[-1].minor);
}
// 7194 "parser.c"
        break;
      case 335:
// 2629 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_shiftright", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(18,&yymsp[-1].minor);
}
// 7202 "parser.c"
        break;
      case 336:
// 2634 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("instanceof", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(11,&yymsp[-1].minor);
}
// 7210 "parser.c"
        break;
      case 337:
// 2639 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("fetch", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(34,&yymsp[-3].minor);
  yy_destructor(6,&yymsp[-1].minor);
}
// 7219 "parser.c"
        break;
      case 339:
// 2649 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("typeof", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(36,&yymsp[-1].minor);
}
// 7227 "parser.c"
        break;
      case 341:
      case 394:
      case 396:
      case 413:
// 2659 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_INTEGER, yymsp[0].minor.yy0, status->scanner_state);
}
// 7237 "parser.c"
        break;
      case 342:
      case 393:
      case 398:
      case 412:
// 2664 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_STRING, yymsp[0].minor.yy0, status->scanner_state);
}
// 7247 "parser.c"
        break;
      case 343:
      case 397:
// 2669 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_CHAR, yymsp[0].minor.yy0, status->scanner_state);
}
// 7255 "parser.c"
        break;
      case 344:
      case 399:
// 2674 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_DOUBLE, yymsp[0].minor.yy0, status->scanner_state);
}
// 7263 "parser.c"
        break;
      case 345:
      case 400:
// 2679 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_NULL, NULL, status->scanner_state);
  yy_destructor(65,&yymsp[0].minor);
}
// 7272 "parser.c"
        break;
      case 346:
      case 402:
// 2684 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_TRUE, NULL, status->scanner_state);
  yy_destructor(117,&yymsp[0].minor);
}
// 7281 "parser.c"
        break;
      case 347:
      case 401:
// 2689 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_FALSE, NULL, status->scanner_state);
  yy_destructor(118,&yymsp[0].minor);
}
// 7290 "parser.c"
        break;
      case 348:
      case 391:
      case 404:
// 2694 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_CONSTANT, yymsp[0].minor.yy0, status->scanner_state);
}
// 7299 "parser.c"
        break;
      case 349:
      case 405:
// 2699 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("empty-array", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-1].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7309 "parser.c"
        break;
      case 350:
      case 406:
// 2704 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("array", yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7319 "parser.c"
        break;
      case 351:
// 2709 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(0, yymsp[0].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-1].minor);
}
// 7327 "parser.c"
        break;
      case 352:
// 2714 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7337 "parser.c"
        break;
      case 353:
// 2719 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(38,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7347 "parser.c"
        break;
      case 354:
// 2724 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7357 "parser.c"
        break;
      case 355:
// 2729 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7369 "parser.c"
        break;
      case 356:
// 2734 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(38,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7381 "parser.c"
        break;
      case 357:
// 2739 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall(1, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7390 "parser.c"
        break;
      case 358:
// 2744 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall(1, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7399 "parser.c"
        break;
      case 359:
// 2749 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall(2, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7410 "parser.c"
        break;
      case 360:
// 2754 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall(2, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7421 "parser.c"
        break;
      case 361:
// 2759 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(0, yymsp[-5].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7431 "parser.c"
        break;
      case 362:
// 2764 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(0, yymsp[-4].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7441 "parser.c"
        break;
      case 363:
// 2769 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(1, yymsp[-6].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(54,&yymsp[-7].minor);
  yy_destructor(55,&yymsp[-5].minor);
  yy_destructor(107,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7453 "parser.c"
        break;
      case 364:
// 2774 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(1, yymsp[-5].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(107,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7465 "parser.c"
        break;
      case 365:
// 2779 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(1, yymsp[-8].minor.yy0, 1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(54,&yymsp[-9].minor);
  yy_destructor(55,&yymsp[-7].minor);
  yy_destructor(107,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7479 "parser.c"
        break;
      case 366:
// 2784 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(1, yymsp[-7].minor.yy0, 1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-8].minor);
  yy_destructor(55,&yymsp[-6].minor);
  yy_destructor(107,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7493 "parser.c"
        break;
      case 367:
// 2789 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(1, yymsp[-5].minor.yy262, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7503 "parser.c"
        break;
      case 368:
// 2794 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(1, yymsp[-4].minor.yy262, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7513 "parser.c"
        break;
      case 369:
// 2799 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(2, yymsp[-7].minor.yy262, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7525 "parser.c"
        break;
      case 370:
// 2804 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(2, yymsp[-6].minor.yy262, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7537 "parser.c"
        break;
      case 371:
// 2809 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(3, yymsp[-7].minor.yy262, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7549 "parser.c"
        break;
      case 372:
// 2814 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(3, yymsp[-6].minor.yy262, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7561 "parser.c"
        break;
      case 376:
// 2834 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("ternary", yymsp[-4].minor.yy262, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(8,&yymsp[-3].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 7570 "parser.c"
        break;
      case 379:
// 2847 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy262, status->scanner_state, 0);
}
// 7577 "parser.c"
        break;
      case 380:
// 2852 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_call_parameter(yymsp[-2].minor.yy0, yymsp[0].minor.yy262, status->scanner_state, 0);
  yy_destructor(89,&yymsp[-1].minor);
}
// 7585 "parser.c"
        break;
      case 381:
// 2857 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy262, status->scanner_state, 1);
  yy_destructor(15,&yymsp[-1].minor);
}
// 7593 "parser.c"
        break;
      case 382:
// 2862 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_call_parameter(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, status->scanner_state, 0);
  yy_destructor(89,&yymsp[-2].minor);
  yy_destructor(15,&yymsp[-1].minor);
}
// 7602 "parser.c"
        break;
      case 383:
// 2867 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("closure", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7614 "parser.c"
        break;
      case 384:
// 2872 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("closure", NULL, yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7626 "parser.c"
        break;
      case 385:
// 2877 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("closure", yymsp[-3].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7638 "parser.c"
        break;
      case 386:
// 2882 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("closure", yymsp[-4].minor.yy262, yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7650 "parser.c"
        break;
      case 389:
      case 409:
// 2894 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_array_item(yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(89,&yymsp[-1].minor);
}
// 7659 "parser.c"
        break;
      case 390:
      case 410:
// 2898 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_array_item(NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 7667 "parser.c"
        break;
      case 416:
// 3003 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_comment(yymsp[0].minor.yy0, status->scanner_state);
}
// 7674 "parser.c"
        break;
      case 417:
// 3007 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_cblock(yymsp[0].minor.yy0, status->scanner_state);
}
// 7681 "parser.c"
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
// 1164 "parser.lemon"


	//fprintf(stderr, "error!\n");

	Json::Value* syntax_error = new Json::Value();

	(*syntax_error)["type"] = "error";

	/*if (status->scanner_state->start_length) {
		fprintf(stderr, "Syntax error, %s", status->scanner_state->start);
	} else {
		fprintf(stderr, "EOF");
	}*/

	//status->syntax_error_len = 48 + Z_STRLEN_P(status->scanner_state->active_file);
	//status->syntax_error = emalloc(sizeof(char) * status->syntax_error_len);

	if (status->scanner_state->start_length) {
		(*syntax_error)["message"] = "Syntax error";
	} else {
		(*syntax_error)["message"] = "Unexpected EOF";
	}

	(*syntax_error)["file"] = status->scanner_state->active_file;
	(*syntax_error)["line"] = status->scanner_state->active_line;
	(*syntax_error)["char"] = status->scanner_state->active_char;

	status->status = XX_PARSING_FAILED;

	status->ret = syntax_error;

	//status->scanner_state->active_file

// 7757 "parser.c"
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

char *strndup(const char *s, size_t len)
{
    if (s) {
        char *ns = (char *)malloc(len + 1);
        if (ns) {
            ns[len] = 0;
            return strncpy(ns, s, len);
        }
    }

    return NULL;
}

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
	//free(pointer);
}

/**
 * Creates a parser_token to be passed to the parser
 */
static void xx_parse_with_token(void* xx_parser, int opcode, int parsercode, xx_scanner_token *token, xx_parser_status *parser_status){

	xx_parser_token *pToken;

	pToken = (xx_parser_token *)malloc(sizeof(xx_parser_token));
	pToken->opcode = opcode;
	pToken->token = token->value;
	pToken->token_len = token->len;
	pToken->free_flag = 1;

	xx_(xx_parser, parsercode, pToken, parser_status);

	token->value = NULL;
	token->len = 0;
}

/**
 * Creates an error message when it's triggered by the scanner
 */
static void xx_scanner_error_msg(xx_parser_status *parser_status){

	/*char *error, *error_part;
	XX_scanner_state *state = parser_status->scanner_state;

	ALLOC_INIT_ZVAL(*error_msg);
	if (state->start) {
		error = emalloc(sizeof(char) * (128 + state->start_length +  Z_STRLEN_P(state->active_file)));
		if (state->start_length > 16) {
			error_part = estrndup(state->start, 16);
			sprintf(error, "Parsing error before '%s...' in %s on line %d", error_part, Z_STRVAL_P(state->active_file), state->active_line);
			efree(error_part);
		} else {
			sprintf(error, "Parsing error before '%s' in %s on line %d", state->start, Z_STRVAL_P(state->active_file), state->active_line);
		}
		ZVAL_STRING(*error_msg, error, 1);
	} else {
		error = emalloc(sizeof(char) * (64 + Z_STRLEN_P(state->active_file)));
		sprintf(error, "Parsing error near to EOF in %s", Z_STRVAL_P(state->active_file));
		ZVAL_STRING(*error_msg, error, 1);
	}
	efree(error);*/
}

/**
 * Parses a comment returning an intermediate array representation
 */
int xx_parse_program(char *program, unsigned int program_length, char *file_path) {

	char *error;
	xx_scanner_state *state;
	xx_scanner_token token;
	int scanner_status, status = SUCCESS, start_lines;
	xx_parser_status *parser_status = NULL;
	void* xx_parser;

	/**
	 * Check if the program has any length
	 */
	if (!program_length) {
		return FAILURE;
	}

	if (program_length < 2) {
		return SUCCESS;
	}

	/**
	 * Start the reentrant parser
	 */
	xx_parser = xx_Alloc(xx_wrapper_alloc);

	parser_status = (xx_parser_status *)malloc(sizeof(xx_parser_status));
	state = (xx_scanner_state *)malloc(sizeof(xx_scanner_state));

	parser_status->status = XX_PARSING_OK;
	parser_status->scanner_state = state;
	parser_status->ret = NULL;
	parser_status->token = &token;
	parser_status->syntax_error = NULL;
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
				fprintf(stderr, "Scanner: unknown opcode %d\n", token.opcode);
				/*if (!*error_msg) {
					error = emalloc(sizeof(char) * (48 + Z_STRLEN_P(state->active_file)));
					sprintf(error, "Scanner: unknown opcode %d on in %s line %d", token.opcode, Z_STRVAL_P(state->active_file), state->active_line);
					ALLOC_INIT_ZVAL(*error_msg);
					ZVAL_STRING(*error_msg, error, 1);
					efree(error);
				}*/
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
					char *x = (char *)malloc(sizeof(char) * 1024);
					if (state->start) {
						sprintf(x, "Scanner error: %d %s", scanner_status, state->start);
					} else {
						sprintf(x, "Scanner error: %d", scanner_status);
					}
					fprintf(stderr, "%s\n", x);
					free(x);
					status = FAILURE;
				}
				break;
			default:
				xx_(xx_parser, 0, NULL, parser_status);
		}
	}

	state->active_token = 0;
	state->start = NULL;

	if (parser_status->status != XX_PARSING_OK) {
		status = FAILURE;
		/*if (parser_status->syntax_error) {
			if (!*error_msg) {
				ALLOC_INIT_ZVAL(*error_msg);
				ZVAL_STRING(*error_msg, parser_status->syntax_error, 1);
			}
			efree(parser_status->syntax_error);
		}*/
		//std::cerr << "error!" std::endl;
	}

	xx_Free(xx_parser, xx_wrapper_free);

	if (status != FAILURE) {
		if (parser_status->status == XX_PARSING_OK) {
			//std::cout << parser_status->ret << std::endl;
			/*if (parser_status->ret) {
				ZVAL_ZVAL(*result, parser_status->ret, 0, 0);
				ZVAL_NULL(parser_status->ret);
				zval_ptr_dtor(&parser_status->ret);
			} else {
				array_init(*result);
			}*/
		}
	}

	if (parser_status->ret) {
		std::cout << *(parser_status->ret) << std::endl;
	}

	//efree(Z_STRVAL(processed_comment));*/

	free(parser_status);
	free(state);

	return status;
}
