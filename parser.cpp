/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
// 54 "parser.lemon"


#include <cstddef>
#include <string>
#include <iostream>
#include <sstream>

#include <assert.h>

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
	delete T;

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

// 1259 "parser.cpp"
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
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
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
  int yyinit;
  xx_TOKENTYPE yy0;
  Json::Value* yy396;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define xx_ARG_SDECL xx_parser_status *status;
#define xx_ARG_PDECL ,xx_parser_status *status
#define xx_ARG_FETCH xx_parser_status *status = yypParser->status
#define xx_ARG_STORE yypParser->status = status
#define YYNSTATE 950
#define YYNRULE 452
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
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
#define YY_ACTTAB_COUNT (12333)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   139,  950,  165,  163,  471,  588,  309,   88,   87,  195,
 /*    10 */   124,  615,  251,  653,  583,  161,  429,  662,  631,  586,
 /*    20 */   867,  175,   13,  669,  623,  586,  173,  580,  167,  141,
 /*    30 */   169,  419,  177,   84,   72,  626,  465,  424,  832,  682,
 /*    40 */   681,  679,  680,  678,  262,  921,  919,  574,  728,  758,
 /*    50 */   415,  697,  893,  572,   89,  669,  412,  553,  896,  726,
 /*    60 */   867,  246,  245,  242,  241,  244,  243,  240,  238,  239,
 /*    70 */   236,  237,  869,  565,  866,  550,  549,  120,  585,  499,
 /*    80 */   119,  298,   84,  612,  544,  118,  542,  539,  388,  538,
 /*    90 */   675,  674,  215,  699,  207,  109,  376,  213,  803,  899,
 /*   100 */   677,  676,   84,   85,  130,  138,  137,  900,  898,  897,
 /*   110 */   895,  894,  757,  139,  868,  165,  163,  160,  159,  155,
 /*   120 */   158,  157,  156,  575,  827,  326,   74,  583,  423,  810,
 /*   130 */   132,  161,  429,  507,  175,  886,  884,  885,  901,  173,
 /*   140 */   580,  167,  141,  169,  419,  177,  123,   72,  423,  647,
 /*   150 */   424,  832,  682,  681,  679,  680,  678,  264,   11,  518,
 /*   160 */   574,  219,  758,  382,   84,  893,  572,   89,  562,  412,
 /*   170 */   553,  896,  726,  563,  246,  245,  242,  241,  244,  243,
 /*   180 */   240,  238,  239,  236,  237,  256,  522,  891,  550,  549,
 /*   190 */   120,  656, 1376,  119,  306,  922,  669,  544,  118,  542,
 /*   200 */   539,  202,  538,  675,  674,  215,  658,  205,   83,  376,
 /*   210 */   423,  408,  899,  677,  676,  144,   85,  130,  138,  137,
 /*   220 */   900,  898,  897,  895,  894,  757,  139,   96,  165,  163,
 /*   230 */   158,  157,  156,  143,   21,  178,  199,  828,  630,  251,
 /*   240 */   583,  161,  429,   96,  662,  631,  115,  175,  214,  803,
 /*   250 */   669,  582,  173,  580,  167,  141,  169,  419,  177,  122,
 /*   260 */    72,  889,  641,  424,  832,  682,  681,  679,  680,  678,
 /*   270 */   264,    9,  518,  574,  829,  758,  379,  888,  893,  572,
 /*   280 */    89,  555,  412,  553,  896,  726,  556,  246,  245,  242,
 /*   290 */   241,  244,  243,  240,  238,  239,  236,  237,  192,  581,
 /*   300 */   548,  550,  549,  120,   80,  296,  119,   82,  287,  327,
 /*   310 */   544,  118,  542,  539,  710,  538,  675,  674,  215,  886,
 /*   320 */   884,  885,  901,  536,  468,  899,  677,  676,  531,   85,
 /*   330 */   130,  138,  137,  900,  898,  897,  895,  894,  757,  139,
 /*   340 */   193,  165,  163,  694,  490,  690,  689,  181,  579,  196,
 /*   350 */   394,  615,  251,  583,  625,  374, 1378,  668,  631,  870,
 /*   360 */   175,  412,  553,  669,  726,  173,  580,  167,  141,  169,
 /*   370 */   419,  177,  121,   72,  551,  635,  424,  832,  682,  681,
 /*   380 */   679,  680,  678, 1377,    7,  535,  574,  825,  758,  371,
 /*   390 */   530,  893,  572,   89,  503,  412,  553,  896,  726,  142,
 /*   400 */   246,  245,  242,  241,  244,  243,  240,  238,  239,  236,
 /*   410 */   237,  368,  524,  546,  550,  549,  120,  412,  553,  119,
 /*   420 */   726,  286,  327,  544,  118,  542,  539,  650,  538,  675,
 /*   430 */   674,  215,  886,  884,  885,  901,   12,  470,  899,  677,
 /*   440 */   676,  623,   85,  130,  138,  137,  900,  898,  897,  895,
 /*   450 */   894,  757,  139,  465,  165,  163,  501,  412,  553,  495,
 /*   460 */   882,  179,  201,  200,  262,  262,  583,  511,  283,  668,
 /*   470 */   662,  667,  667,  175, 1375,  669,  669,  510,  173,  580,
 /*   480 */   167,  141,  169,  419,  177,  116,   72,  212,  803,  424,
 /*   490 */   832,  682,  681,  679,  680,  678,  294,  479,  612,  574,
 /*   500 */   824,  758,  367,  131,  893,  572,   89,  819,  412,  553,
 /*   510 */   896,  726,  481,  246,  245,  242,  241,  244,  243,  240,
 /*   520 */   238,  239,  236,  237,  292,  194,  612,  550,  549,  120,
 /*   530 */   100,  197,  119,  630,  251,  407,  544,  118,  542,  539,
 /*   540 */   631,  538,  675,  674,  215,  669,  207,  128,  376,  307,
 /*   550 */   686,  899,  677,  676,   81,   85,  130,  138,  137,  900,
 /*   560 */   898,  897,  895,  894,  757,  139,  573,  165,  163,  784,
 /*   570 */   307,  492,  324,  491,  687,  490,  690,  689,  644,  583,
 /*   580 */   406,  782,  886,  884,  885,  901,  175,   10,  772,  533,
 /*   590 */   532,  173,  580,  167,  141,  169,  419,  177,  127,   72,
 /*   600 */   307,  684,  424,  832,  682,  681,  679,  680,  678,  768,
 /*   610 */   475,  496,  574,  541,  758,  366,   86,  893,  572,   89,
 /*   620 */   417,  412,  553,  896,  726,  477,  246,  245,  242,  241,
 /*   630 */   244,  243,  240,  238,  239,  236,  237,  867,  190,  767,
 /*   640 */   550,  549,  120,  307,  307,  119,  307,  307,  307,  544,
 /*   650 */   118,  542,  539,  638,  538,  675,  674,  215,  472,  206,
 /*   660 */   234,  376,    8,   73,  899,  677,  676,  272,   85,  130,
 /*   670 */   138,  137,  900,  898,  897,  895,  894,  757,  139,  860,
 /*   680 */   165,  163,  766,  765,  332,  764,  763,  762,  414,  865,
 /*   690 */   565,  866,  583,  569,  886,  884,  885,  901,  488,  175,
 /*   700 */   126,  385,  883,  672,  173,  580,  167,  141,  169,  419,
 /*   710 */   177,  307,   72,  307,  632,  424,  832,  682,  681,  679,
 /*   720 */   680,  678,  873,    6,  628,  574,  248,  758,  365,  618,
 /*   730 */   893,  572,   89,  216,  412,  553,  896,  726,  216,  246,
 /*   740 */   245,  242,  241,  244,  243,  240,  238,  239,  236,  237,
 /*   750 */   761, 1397,  760,  550,  549,  120,  125,  307,  119,  670,
 /*   760 */   547,   98,  544,  118,  542,  539, 1396,  538,  675,  674,
 /*   770 */   215,  459,  205,  232,  376,  208,   73,  899,  677,  676,
 /*   780 */   271,   85,  130,  138,  137,  900,  898,  897,  895,  894,
 /*   790 */   757,  139,  198,  165,  163,  256,  759,  332,  515,   98,
 /*   800 */   668,  631,   96,   96,  629,  583,  669,  886,  884,  885,
 /*   810 */   901,  270,  175,  216,  389,  883,  254,  173,  580,  167,
 /*   820 */   141,  169,  419,  177,  235,   72,  918,   73,  424,  832,
 /*   830 */   682,  681,  679,  680,  678,  561,  723,  721,  574,  821,
 /*   840 */   758,  308,  744,  893,  572,   89,  361,  745,  559,  896,
 /*   850 */   269,  434,  246,  245,  242,  241,  244,  243,  240,  238,
 /*   860 */   239,  236,  237,  268,  304,  543,  550,  549,  120,  233,
 /*   870 */   113,  119,   73,  804,  327,  544,  118,  542,  539,  557,
 /*   880 */   538,  675,  674,  215,  886,  884,  885,  901,  860,  494,
 /*   890 */   899,  677,  676,  267,   85,  130,  138,  137,  900,  898,
 /*   900 */   897,  895,  894,  757,  139,  619,  165,  163,  405,  769,
 /*   910 */   332,  456,   73,  231,  216,  554,   73,  253,  583,  552,
 /*   920 */   886,  884,  885,  901,  291,  175,  612,  390,  883,   96,
 /*   930 */   173,  580,  167,  141,  169,  419,  177,  266,   72,  404,
 /*   940 */   769,  424,  832,  682,  681,  679,  680,  678,  403,  769,
 /*   950 */   265,  574,  820,  758,  402,  769,  893,  572,   89,   28,
 /*   960 */   401,  769,  896,  717,  831,  246,  245,  242,  241,  244,
 /*   970 */   243,  240,  238,  239,  236,  237,  400,  769,  540,  550,
 /*   980 */   549,  120,  399,  769,  119,  398,  769,  327,  544,  118,
 /*   990 */   542,  539,  830,  538,  675,  674,  215,  886,  884,  885,
 /*  1000 */   901,   36,  485,  899,  677,  676,   27,   85,  130,  138,
 /*  1010 */   137,  900,  898,  897,  895,  894,  757,  139,   96,  165,
 /*  1020 */   163,  397,  769,  332,  396,  769,  261,   96,  395,  769,
 /*  1030 */   290,  583,  612,  886,  884,  885,  901,  669,  175,   26,
 /*  1040 */   418,  883,   96,  173,  580,  167,  141,  169,  419,  177,
 /*  1050 */   107,   72,  712,  804,  424,  832,  682,  681,  679,  680,
 /*  1060 */   678,  709,  302,  732,  574,  754,  758,  516,   98,  893,
 /*  1070 */   572,   89,  301,  732,  104,  896,  707,  804,  246,  245,
 /*  1080 */   242,  241,  244,  243,  240,  238,  239,  236,  237,  513,
 /*  1090 */    98,  519,  550,  549,  120,  289,  739,  119,  369,  621,
 /*  1100 */   327,  544,  118,  542,  539,  217,  538,  675,  674,  215,
 /*  1110 */   886,  884,  885,  901,   25,  470,  899,  677,  676,   24,
 /*  1120 */    85,  130,  138,  137,  900,  898,  897,  895,  894,  757,
 /*  1130 */   139,   96,  165,  163,  410,  785,  332,   23,  117,  255,
 /*  1140 */    96,  252,  823,   22,  583,  411,  886,  884,  885,  901,
 /*  1150 */   669,  175,  669,  416,  883,  193,  173,  580,  167,  141,
 /*  1160 */   169,  419,  177,  807,   72,  705,  280,  424,  832,  682,
 /*  1170 */   681,  679,  680,  678,  703,  413,  560,  574,  753,  758,
 /*  1180 */   203,  279,  893,  572,   89,  789,  788,  274,  896,  381,
 /*  1190 */   273,  246,  245,  242,  241,  244,  243,  240,  238,  239,
 /*  1200 */   236,  237,  781,  780,  517,  550,  549,  120,  779,  778,
 /*  1210 */   119,  263,  129,  327,  544,  118,  542,  539,  523,  538,
 /*  1220 */   675,  674,  215,  886,  884,  885,  901,  521,  468,  899,
 /*  1230 */   677,  676,   18,   85,  130,  138,  137,  900,  898,  897,
 /*  1240 */   895,  894,  757,  139,  193,  165,  163,   99,  742,  332,
 /*  1250 */    33,   17,   16,  193,  514,   79,  731,  583,   15,  886,
 /*  1260 */   884,  885,  901,   14,  175,  509,  393,  883,  193,  173,
 /*  1270 */   580,  167,  141,  169,  419,  177,   78,   72,  378,  508,
 /*  1280 */   424,  832,  682,  681,  679,  680,  678,  359,  506,  505,
 /*  1290 */   574,  751,  758,   77,   97,  893,  572,   89,  502,  696,
 /*  1300 */    76,  896,  357,  193,  246,  245,  242,  241,  244,  243,
 /*  1310 */   240,  238,  239,  236,  237,  500,   75,  498,  550,  549,
 /*  1320 */   120,  300,  305,  119,  698,  189,  193,  544,  118,  542,
 /*  1330 */   539,  493,  538,  675,  674,  215,  191,  452,  693,  377,
 /*  1340 */   383,  487,  899,  677,  676,  688,   85,  130,  138,  137,
 /*  1350 */   900,  898,  897,  895,  894,  757,  139,  193,  165,  163,
 /*  1360 */   447,  685,  332,  484,  188,  260,  193,  259,  258,  673,
 /*  1370 */   583,  671,  886,  884,  885,  901,  666,  175,  665,  392,
 /*  1380 */   883,  664,  173,  580,  167,  141,  169,  419,  177,  257,
 /*  1390 */    72,  443,  663,  424,  832,  682,  681,  679,  680,  678,
 /*  1400 */   438,  288,  299,  574,  749,  758,  469,  657,  893,  572,
 /*  1410 */    89,  187,  467,  622,  896,  186,  627,  246,  245,  242,
 /*  1420 */   241,  244,  243,  240,  238,  239,  236,  237,  620,  617,
 /*  1430 */   616,  550,  549,  120,  285,  293,  119,  460,  457,  185,
 /*  1440 */   544,  118,  542,  539,  284,  538,  675,  674,  215,  303,
 /*  1450 */   211,  454,   32,    5,  184,  899,  677,  676,  449,   85,
 /*  1460 */   130,  138,  137,  900,  898,  897,  895,  894,  757,  139,
 /*  1470 */   450,  165,  163,   31,    4,  332,  445,  183,   30,    3,
 /*  1480 */   440,  441,   29,  583,  182,  886,  884,  885,  901,    2,
 /*  1490 */   175,  920,  391,  883,  587,  173,  580,  167,  141,  169,
 /*  1500 */   419,  177,  822,   72,  247,  525,  424,  832,  682,  681,
 /*  1510 */   679,  680,  678,  771,  746,  700,  574,  736,  758,  747,
 /*  1520 */    86,  893,  572,   89,  613,  605,  614,  896,   73,  737,
 /*  1530 */   246,  245,  242,  241,  244,  243,  240,  238,  239,  236,
 /*  1540 */   237,  701,  466,  624,  550,  549,  120,  463,  818,  119,
 /*  1550 */   660,  464,  826,  544,  118,  542,  539,  659,  538,  675,
 /*  1560 */   674,  215,  114,  210,  611,  609,  608,  462,  899,  677,
 /*  1570 */   676,  607,   85,  130,  138,  137,  900,  898,  897,  895,
 /*  1580 */   894,  757,  139,  604,  165,  163,  661,  610,  332,  683,
 /*  1590 */   606,  881,  112,  864,  442, 1404,  583,  111,  886,  884,
 /*  1600 */   885,  901,  110,  175,  108,  387,  883, 1404,  173,  580,
 /*  1610 */   167,  141,  169,  419,  177,  863,   72,  862,  106,  424,
 /*  1620 */   832,  682,  681,  679,  680,  678, 1404,  105,  861,  574,
 /*  1630 */   734,  758,  859,  858,  893,  572,   89,  103,  102,  101,
 /*  1640 */   896,  857,  545,  246,  245,  242,  241,  244,  243,  240,
 /*  1650 */   238,  239,  236,  237,  770,  480,  478,  550,  549,  120,
 /*  1660 */   476,  474,  119,  297,  295, 1404,  544,  118,  542,  539,
 /*  1670 */  1404,  538,  675,  674,  215, 1404,  209, 1404, 1404, 1404,
 /*  1680 */  1404,  899,  677,  676, 1404,   85,  130,  138,  137,  900,
 /*  1690 */   898,  897,  895,  894,  757,  139, 1404,  165,  163, 1404,
 /*  1700 */  1404,  332, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  583,
 /*  1710 */  1404,  886,  884,  885,  901, 1404,  175, 1404,  386,  883,
 /*  1720 */  1404,  173,  580,  167,  141,  169,  419,  177, 1404,   72,
 /*  1730 */  1404, 1404,  424,  832,  682,  681,  679,  680,  678, 1404,
 /*  1740 */  1404, 1404,  574,  730,  758, 1404, 1404,  893,  572,   89,
 /*  1750 */  1404, 1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,
 /*  1760 */   244,  243,  240,  238,  239,  236,  237, 1404, 1404, 1404,
 /*  1770 */   550,  549,  120, 1404, 1404,  119, 1404, 1404,  433,  544,
 /*  1780 */   118,  542,  539, 1404,  538,  675,  674,  215,  886,  884,
 /*  1790 */   885,  901, 1404, 1404,  899,  677,  676, 1404,   85,  130,
 /*  1800 */   138,  137,  900,  898,  897,  895,  894,  757,  139, 1404,
 /*  1810 */   165,  163, 1404, 1404,  486, 1404,  491,  687,  490,  690,
 /*  1820 */   689,  483,  583,  491,  687,  490,  690,  689, 1404,  175,
 /*  1830 */  1404, 1404, 1404, 1404,  173,  580,  167,  141,  169,  419,
 /*  1840 */   177, 1404,   72, 1404, 1404,  424,  832,  482, 1404,  491,
 /*  1850 */   687,  490,  690,  689, 1404,  574,  725,  758, 1404, 1404,
 /*  1860 */   893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,
 /*  1870 */   245,  242,  241,  244,  243,  240,  238,  239,  236,  237,
 /*  1880 */  1404, 1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404,
 /*  1890 */  1404, 1404,  544,  118,  542,  539, 1404,  538, 1404,  375,
 /*  1900 */   215,  491,  687,  490,  690,  689, 1404,  899, 1404, 1404,
 /*  1910 */  1404,   85,  130,  138,  137,  900,  898,  897,  895,  894,
 /*  1920 */   757,  139, 1404,  165,  163, 1404, 1404,  373, 1404,  491,
 /*  1930 */   687,  490,  690,  689,  372,  583,  491,  687,  490,  690,
 /*  1940 */   689, 1404,  175, 1404, 1404, 1404, 1404,  173,  580,  167,
 /*  1950 */   141,  169,  419,  177, 1404,   72, 1404, 1404,  424,  832,
 /*  1960 */   370, 1404,  491,  687,  490,  690,  689, 1404,  574,  654,
 /*  1970 */   758, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896,
 /*  1980 */  1404, 1404,  246,  245,  242,  241,  244,  243,  240,  238,
 /*  1990 */   239,  236,  237, 1404, 1404, 1404,  550,  549,  120, 1404,
 /*  2000 */  1404,  119, 1404, 1404,  332,  544,  118,  542,  539, 1404,
 /*  2010 */   538, 1404, 1404,  215,  886,  884,  885,  901, 1404, 1404,
 /*  2020 */   899, 1404,  719, 1404,   85,  130,  138,  137,  900,  898,
 /*  2030 */   897,  895,  894,  757,  139, 1404,  165,  163, 1404,  326,
 /*  2040 */  1404, 1404, 1404,  809, 1404, 1404,  325, 1404,  583,  886,
 /*  2050 */   884,  885,  901,  537, 1404,  175,  886,  884,  885,  901,
 /*  2060 */   173,  580,  167,  141,  169,  419,  177,  324,   72, 1404,
 /*  2070 */  1404,  424,  832, 1404, 1404, 1404,  783,  886,  884,  885,
 /*  2080 */   901,  574,  651,  758, 1404, 1404,  893,  572,   89, 1404,
 /*  2090 */  1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,  244,
 /*  2100 */   243,  240,  238,  239,  236,  237, 1404, 1404, 1404,  550,
 /*  2110 */   549,  120, 1404, 1404,  119, 1404, 1404,  344,  544,  118,
 /*  2120 */   542,  539, 1404,  538, 1404, 1404,  215,  886,  884,  885,
 /*  2130 */   901, 1404, 1404,  899, 1404, 1404, 1404,   85,  130,  138,
 /*  2140 */   137,  900,  898,  897,  895,  894,  757,  139, 1404,  165,
 /*  2150 */   163, 1404,  326, 1404, 1404, 1404,  808, 1404, 1404,  329,
 /*  2160 */  1404,  583,  886,  884,  885,  901, 1404, 1404,  175,  886,
 /*  2170 */   884,  885,  901,  173,  580,  167,  141,  169,  419,  177,
 /*  2180 */   715,   72, 1404, 1404,  424,  832, 1404, 1404, 1404, 1404,
 /*  2190 */  1404, 1404, 1404, 1404,  574,  648,  758, 1404, 1404,  893,
 /*  2200 */   572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,  245,
 /*  2210 */   242,  241,  244,  243,  240,  238,  239,  236,  237, 1404,
 /*  2220 */  1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404, 1404,
 /*  2230 */   356,  544,  118,  542,  539, 1404,  538, 1404, 1404,  215,
 /*  2240 */   886,  884,  885,  901, 1404, 1404,  899, 1404, 1404, 1404,
 /*  2250 */    85,  130,  138,  137,  900,  898,  897,  895,  894,  757,
 /*  2260 */   139, 1404,  165,  163, 1404,  326, 1404, 1404, 1404,  806,
 /*  2270 */  1404, 1404,  345, 1404,  583,  886,  884,  885,  901, 1404,
 /*  2280 */  1404,  175,  886,  884,  885,  901,  173,  580,  167,  141,
 /*  2290 */   169,  419,  177,  432,   72, 1404, 1404,  424,  832, 1404,
 /*  2300 */  1404, 1404, 1404,  886,  884,  885,  901,  574,  645,  758,
 /*  2310 */  1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404,
 /*  2320 */  1404,  246,  245,  242,  241,  244,  243,  240,  238,  239,
 /*  2330 */   236,  237, 1404, 1404, 1404,  550,  549,  120, 1404, 1404,
 /*  2340 */   119, 1404, 1404,  346,  544,  118,  542,  539, 1404,  538,
 /*  2350 */  1404, 1404,  215,  886,  884,  885,  901, 1404, 1404,  899,
 /*  2360 */  1404, 1404, 1404,   85,  130,  138,  137,  900,  898,  897,
 /*  2370 */   895,  894,  757,  139, 1404,  165,  163, 1404,  326, 1404,
 /*  2380 */  1404, 1404,  805, 1404, 1404, 1404, 1404,  583,  886,  884,
 /*  2390 */   885,  901, 1404, 1404,  175, 1404, 1404, 1404, 1404,  173,
 /*  2400 */   580,  167,  141,  169,  419,  177,  333,   72, 1404, 1404,
 /*  2410 */   424,  832, 1404, 1404, 1404, 1404,  886,  884,  885,  901,
 /*  2420 */   574,  642,  758, 1404, 1404,  893,  572,   89, 1404, 1404,
 /*  2430 */  1404,  896, 1404, 1404,  246,  245,  242,  241,  244,  243,
 /*  2440 */   240,  238,  239,  236,  237, 1404, 1404, 1404,  550,  549,
 /*  2450 */   120, 1404, 1404,  119, 1404, 1404,  347,  544,  118,  542,
 /*  2460 */   539, 1404,  538, 1404, 1404,  215,  886,  884,  885,  901,
 /*  2470 */  1404, 1404,  899, 1404, 1404, 1404,   85,  130,  138,  137,
 /*  2480 */   900,  898,  897,  895,  894,  757,  139, 1404,  165,  163,
 /*  2490 */  1404,  326, 1404, 1404, 1404,  800, 1404, 1404,  431, 1404,
 /*  2500 */   583,  886,  884,  885,  901, 1404, 1404,  175,  886,  884,
 /*  2510 */   885,  901,  173,  580,  167,  141,  169,  419,  177,  348,
 /*  2520 */    72, 1404, 1404,  424,  832, 1404, 1404, 1404, 1404,  886,
 /*  2530 */   884,  885,  901,  574,  639,  758, 1404, 1404,  893,  572,
 /*  2540 */    89, 1404, 1404, 1404,  896, 1404, 1404,  246,  245,  242,
 /*  2550 */   241,  244,  243,  240,  238,  239,  236,  237, 1404, 1404,
 /*  2560 */  1404,  550,  549,  120, 1404, 1404,  119, 1404, 1404,  430,
 /*  2570 */   544,  118,  542,  539, 1404,  538, 1404, 1404,  215,  886,
 /*  2580 */   884,  885,  901, 1404, 1404,  899, 1404, 1404, 1404,   85,
 /*  2590 */   130,  138,  137,  900,  898,  897,  895,  894,  757,  139,
 /*  2600 */  1404,  165,  163, 1404,  326, 1404, 1404, 1404,  799, 1404,
 /*  2610 */  1404,  349, 1404,  583,  886,  884,  885,  901, 1404, 1404,
 /*  2620 */   175,  886,  884,  885,  901,  173,  580,  167,  141,  169,
 /*  2630 */   419,  177,  335,   72, 1404, 1404,  424,  832, 1404, 1404,
 /*  2640 */  1404, 1404,  886,  884,  885,  901,  574,  636,  758, 1404,
 /*  2650 */  1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,
 /*  2660 */   246,  245,  242,  241,  244,  243,  240,  238,  239,  236,
 /*  2670 */   237, 1404, 1404, 1404,  550,  549,  120, 1404, 1404,  119,
 /*  2680 */  1404, 1404,  350,  544,  118,  542,  539, 1404,  538, 1404,
 /*  2690 */  1404,  215,  886,  884,  885,  901, 1404, 1404,  899, 1404,
 /*  2700 */  1404, 1404,   85,  130,  138,  137,  900,  898,  897,  895,
 /*  2710 */   894,  757,  139, 1404,  165,  163, 1404,  326, 1404, 1404,
 /*  2720 */  1404,  798, 1404, 1404,  334, 1404,  583,  886,  884,  885,
 /*  2730 */   901, 1404, 1404,  175,  886,  884,  885,  901,  173,  580,
 /*  2740 */   167,  141,  169,  419,  177,  352,   72, 1404, 1404,  424,
 /*  2750 */   832, 1404, 1404, 1404, 1404,  886,  884,  885,  901,  574,
 /*  2760 */   633,  758, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,
 /*  2770 */   896, 1404, 1404,  246,  245,  242,  241,  244,  243,  240,
 /*  2780 */   238,  239,  236,  237, 1404, 1404, 1404,  550,  549,  120,
 /*  2790 */  1404, 1404,  119, 1404, 1404,  321,  544,  118,  542,  539,
 /*  2800 */  1404,  538, 1404, 1404,  215,  886,  884,  885,  901, 1404,
 /*  2810 */  1404,  899, 1404, 1404, 1404,   85,  130,  138,  137,  900,
 /*  2820 */   898,  897,  895,  894,  757,  139, 1404,  165,  163, 1404,
 /*  2830 */   326, 1404, 1404, 1404,  797, 1404, 1404,  355, 1404,  583,
 /*  2840 */   886,  884,  885,  901, 1404, 1404,  175,  886,  884,  885,
 /*  2850 */   901,  173,  580,  167,  141,  169,  419,  177,  354,   72,
 /*  2860 */  1404, 1404,  424,  832, 1404, 1404, 1404, 1404,  886,  884,
 /*  2870 */   885,  901,  574,  603,  758, 1404, 1404,  893,  572,   89,
 /*  2880 */  1404, 1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,
 /*  2890 */   244,  243,  240,  238,  239,  236,  237, 1404, 1404, 1404,
 /*  2900 */   550,  549,  120, 1404, 1404,  119, 1404, 1404,  428,  544,
 /*  2910 */   118,  542,  539, 1404,  538, 1404, 1404,  215,  886,  884,
 /*  2920 */   885,  901, 1404, 1404,  899, 1404, 1404, 1404,   85,  130,
 /*  2930 */   138,  137,  900,  898,  897,  895,  894,  757,  139, 1404,
 /*  2940 */   165,  163, 1404,  326, 1404, 1404, 1404,  796, 1404, 1404,
 /*  2950 */   427, 1404,  583,  886,  884,  885,  901, 1404, 1404,  175,
 /*  2960 */   886,  884,  885,  901,  173,  580,  167,  141,  169,  419,
 /*  2970 */   177,  426,   72, 1404, 1404,  424,  832, 1404, 1404, 1404,
 /*  2980 */  1404,  886,  884,  885,  901,  574,  601,  758, 1404, 1404,
 /*  2990 */   893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,
 /*  3000 */   245,  242,  241,  244,  243,  240,  238,  239,  236,  237,
 /*  3010 */  1404, 1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404,
 /*  3020 */  1404,  353,  544,  118,  542,  539, 1404,  538, 1404, 1404,
 /*  3030 */   215,  886,  884,  885,  901, 1404, 1404,  899, 1404, 1404,
 /*  3040 */  1404,   85,  130,  138,  137,  900,  898,  897,  895,  894,
 /*  3050 */   757,  139, 1404,  165,  163, 1404,  326, 1404, 1404, 1404,
 /*  3060 */   795, 1404, 1404,  338, 1404,  583,  886,  884,  885,  901,
 /*  3070 */  1404, 1404,  175,  886,  884,  885,  901,  173,  580,  167,
 /*  3080 */   141,  169,  419,  177,  337,   72, 1404, 1404,  424,  832,
 /*  3090 */  1404, 1404, 1404, 1404,  886,  884,  885,  901,  574,  600,
 /*  3100 */   758, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896,
 /*  3110 */  1404, 1404,  246,  245,  242,  241,  244,  243,  240,  238,
 /*  3120 */   239,  236,  237, 1404, 1404, 1404,  550,  549,  120, 1404,
 /*  3130 */  1404,  119, 1404, 1404,  343,  544,  118,  542,  539, 1404,
 /*  3140 */   538, 1404, 1404,  215,  886,  884,  885,  901, 1404, 1404,
 /*  3150 */   899, 1404, 1404, 1404,   85,  130,  138,  137,  900,  898,
 /*  3160 */   897,  895,  894,  757,  139, 1404,  165,  163, 1404,  326,
 /*  3170 */  1404, 1404, 1404,  794, 1404, 1404,  342, 1404,  583,  886,
 /*  3180 */   884,  885,  901, 1404, 1404,  175,  886,  884,  885,  901,
 /*  3190 */   173,  580,  167,  141,  169,  419,  177,  341,   72, 1404,
 /*  3200 */  1404,  424,  832, 1404, 1404, 1404, 1404,  886,  884,  885,
 /*  3210 */   901,  574,  598,  758, 1404, 1404,  893,  572,   89, 1404,
 /*  3220 */  1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,  244,
 /*  3230 */   243,  240,  238,  239,  236,  237, 1404, 1404, 1404,  550,
 /*  3240 */   549,  120, 1404, 1404,  119, 1404, 1404,  340,  544,  118,
 /*  3250 */   542,  539, 1404,  538, 1404, 1404,  215,  886,  884,  885,
 /*  3260 */   901, 1404, 1404,  899, 1404, 1404, 1404,   85,  130,  138,
 /*  3270 */   137,  900,  898,  897,  895,  894,  757,  139, 1404,  165,
 /*  3280 */   163, 1404,  326, 1404, 1404, 1404,  793, 1404, 1404,  339,
 /*  3290 */  1404,  583,  886,  884,  885,  901, 1404, 1404,  175,  886,
 /*  3300 */   884,  885,  901,  173,  580,  167,  141,  169,  419,  177,
 /*  3310 */   336,   72, 1404, 1404,  424,  832, 1404, 1404, 1404, 1404,
 /*  3320 */   886,  884,  885,  901,  574,  597,  758, 1404, 1404,  893,
 /*  3330 */   572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,  245,
 /*  3340 */   242,  241,  244,  243,  240,  238,  239,  236,  237, 1404,
 /*  3350 */  1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404, 1404,
 /*  3360 */   320,  544,  118,  542,  539, 1404,  538, 1404, 1404,  215,
 /*  3370 */   886,  884,  885,  901, 1404, 1404,  899, 1404, 1404, 1404,
 /*  3380 */    85,  130,  138,  137,  900,  898,  897,  895,  894,  757,
 /*  3390 */   139, 1404,  165,  163, 1404,  326, 1404, 1404, 1404,  792,
 /*  3400 */  1404, 1404,  331, 1404,  583,  886,  884,  885,  901, 1404,
 /*  3410 */  1404,  175,  886,  884,  885,  901,  173,  580,  167,  141,
 /*  3420 */   169,  419,  177,  425,   72, 1404, 1404,  424,  832, 1404,
 /*  3430 */  1404, 1404, 1404,  886,  884,  885,  901,  574,  595,  758,
 /*  3440 */  1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404,
 /*  3450 */  1404,  246,  245,  242,  241,  244,  243,  240,  238,  239,
 /*  3460 */   236,  237, 1404, 1404, 1404,  550,  549,  120, 1404, 1404,
 /*  3470 */   119, 1404, 1404,  351,  544,  118,  542,  539, 1404,  538,
 /*  3480 */  1404, 1404,  215,  886,  884,  885,  901, 1404, 1404,  899,
 /*  3490 */  1404, 1404, 1404,   85,  130,  138,  137,  900,  898,  897,
 /*  3500 */   895,  894,  757,  139, 1404,  165,  163, 1404,  326, 1404,
 /*  3510 */  1404, 1404,  787, 1404, 1404,  422, 1404,  583,  886,  884,
 /*  3520 */   885,  901, 1404, 1404,  175,  886,  884,  885,  901,  173,
 /*  3530 */   580,  167,  141,  169,  419,  177,  421,   72, 1404, 1404,
 /*  3540 */   424,  832, 1404, 1404, 1404, 1404,  886,  884,  885,  901,
 /*  3550 */   574,  594,  758, 1404, 1404,  893,  572,   89, 1404, 1404,
 /*  3560 */  1404,  896, 1404, 1404,  246,  245,  242,  241,  244,  243,
 /*  3570 */   240,  238,  239,  236,  237, 1404, 1404, 1404,  550,  549,
 /*  3580 */   120, 1404, 1404,  119, 1404, 1404,  328,  544,  118,  542,
 /*  3590 */   539, 1404,  538, 1404, 1404,  215,  886,  884,  885,  901,
 /*  3600 */  1404, 1404,  899, 1404, 1404, 1404,   85,  130,  138,  137,
 /*  3610 */   900,  898,  897,  895,  894,  757,  139, 1404,  165,  163,
 /*  3620 */  1404,  326, 1404, 1404, 1404,  786, 1404, 1404,  316, 1404,
 /*  3630 */   583,  886,  884,  885,  901, 1404, 1404,  175,  886,  884,
 /*  3640 */   885,  901,  173,  580,  167,  141,  169,  419,  177,  315,
 /*  3650 */    72, 1404, 1404,  424,  832, 1404, 1404, 1404, 1404,  886,
 /*  3660 */   884,  885,  901,  574,  592,  758, 1404, 1404,  893,  572,
 /*  3670 */    89, 1404, 1404, 1404,  896, 1404, 1404,  246,  245,  242,
 /*  3680 */   241,  244,  243,  240,  238,  239,  236,  237, 1404, 1404,
 /*  3690 */  1404,  550,  549,  120, 1404, 1404,  119, 1404, 1404,  314,
 /*  3700 */   544,  118,  542,  539, 1404,  538, 1404, 1404,  215,  886,
 /*  3710 */   884,  885,  901, 1404, 1404,  899, 1404, 1404,  584,   85,
 /*  3720 */   130,  138,  137,  900,  898,  897,  895,  894,  757,  139,
 /*  3730 */  1404,  165,  163, 1404,  319, 1404, 1404, 1404, 1404, 1404,
 /*  3740 */  1404,  313, 1404,  583,  886,  884,  885,  901, 1404, 1404,
 /*  3750 */   175,  886,  884,  885,  901,  173,  580,  167,  141,  169,
 /*  3760 */   419,  177,  311,   72, 1404, 1404,  424,  832, 1404, 1404,
 /*  3770 */  1404, 1404,  886,  884,  885,  901,  574,  727,  758, 1404,
 /*  3780 */  1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,
 /*  3790 */   246,  245,  242,  241,  244,  243,  240,  238,  239,  236,
 /*  3800 */   237, 1404, 1404, 1404,  550,  549,  120, 1404, 1404,  119,
 /*  3810 */  1404, 1404,  322,  544,  118,  542,  539, 1404,  538, 1404,
 /*  3820 */  1404,  215,  886,  884,  885,  901, 1404, 1404,  899, 1404,
 /*  3830 */  1404, 1404,   85,  130,  138,  137,  900,  898,  897,  895,
 /*  3840 */   894,  757,  139, 1404,  165,  163, 1404,  310, 1404, 1404,
 /*  3850 */  1404, 1404, 1404, 1404,  330, 1404,  583,  886,  884,  885,
 /*  3860 */   901, 1404, 1404,  175,  886,  884,  885,  901,  173,  580,
 /*  3870 */   167,  141,  169,  419,  177,  318,   72, 1404, 1404,  424,
 /*  3880 */   832, 1404, 1404, 1404, 1404,  886,  884,  885,  901,  574,
 /*  3890 */   218,  758, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,
 /*  3900 */   896, 1404, 1404,  246,  245,  242,  241,  244,  243,  240,
 /*  3910 */   238,  239,  236,  237, 1404, 1404, 1404,  550,  549,  120,
 /*  3920 */  1404, 1404,  119, 1404, 1404,  317,  544,  118,  542,  539,
 /*  3930 */  1404,  538, 1404, 1404,  215,  886,  884,  885,  901, 1404,
 /*  3940 */  1404,  899, 1404, 1404, 1404,   85,  130,  138,  137,  900,
 /*  3950 */   898,  897,  895,  894,  757,  139, 1404,  165,  163, 1404,
 /*  3960 */   312, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  583,
 /*  3970 */   886,  884,  885,  901, 1404, 1404,  175, 1404, 1404, 1404,
 /*  3980 */  1404,  173,  580,  167,  141,  169,  419,  177, 1404,   72,
 /*  3990 */  1404, 1404,  424,  832, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4000 */  1404, 1404,  574,  738,  758, 1404, 1404,  893,  572,   89,
 /*  4010 */  1404, 1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,
 /*  4020 */   244,  243,  240,  238,  239,  236,  237, 1404, 1404, 1404,
 /*  4030 */   550,  549,  120, 1404, 1404,  119, 1404, 1404, 1404,  544,
 /*  4040 */   118,  542,  539, 1404,  538, 1404, 1404,  215, 1404, 1404,
 /*  4050 */  1404, 1404, 1404, 1404,  899, 1404, 1404, 1404,   85,  130,
 /*  4060 */   138,  137,  900,  898,  897,  895,  894,  757,  139, 1404,
 /*  4070 */   165,  163, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4080 */  1404, 1404,  583, 1404, 1404, 1404, 1404, 1404, 1404,  175,
 /*  4090 */  1404, 1404, 1404, 1404,  173,  580,  167,  141,  169,  419,
 /*  4100 */   177, 1404,   72, 1404, 1404,  424,  832, 1404, 1404, 1404,
 /*  4110 */  1404, 1404, 1404, 1404, 1404,  574,  740,  758, 1404, 1404,
 /*  4120 */   893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,
 /*  4130 */   245,  242,  241,  244,  243,  240,  238,  239,  236,  237,
 /*  4140 */  1404, 1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404,
 /*  4150 */  1404, 1404,  544,  118,  542,  539, 1404,  538, 1404, 1404,
 /*  4160 */   215, 1404, 1404, 1404, 1404, 1404, 1404,  899, 1404, 1404,
 /*  4170 */  1404,   85,  130,  138,  137,  900,  898,  897,  895,  894,
 /*  4180 */   757,  139, 1404,  165,  163, 1404, 1404, 1404, 1404, 1404,
 /*  4190 */  1404, 1404, 1404, 1404, 1404,  583, 1404, 1404, 1404, 1404,
 /*  4200 */  1404, 1404,  175, 1404, 1404, 1404, 1404,  173,  580,  167,
 /*  4210 */   141,  169,  419,  177, 1404,   72, 1404, 1404,  424,  832,
 /*  4220 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  574,  741,
 /*  4230 */   758, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896,
 /*  4240 */  1404, 1404,  246,  245,  242,  241,  244,  243,  240,  238,
 /*  4250 */   239,  236,  237, 1404, 1404, 1404,  550,  549,  120, 1404,
 /*  4260 */  1404,  119, 1404, 1404, 1404,  544,  118,  542,  539, 1404,
 /*  4270 */   538, 1404, 1404,  215, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4280 */   899, 1404, 1404, 1404,   85,  130,  138,  137,  900,  898,
 /*  4290 */   897,  895,  894,  757,  139, 1404,  165,  163, 1404, 1404,
 /*  4300 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  583, 1404,
 /*  4310 */  1404, 1404, 1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,
 /*  4320 */   173,  580,  167,  141,  169,  419,  177, 1404,   72, 1404,
 /*  4330 */  1404,  424,  832, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4340 */  1404,  574,  520,  758, 1404, 1404,  893,  572,   89, 1404,
 /*  4350 */  1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,  244,
 /*  4360 */   243,  240,  238,  239,  236,  237, 1404, 1404, 1404,  550,
 /*  4370 */   549,  120, 1404, 1404,  119, 1404, 1404, 1404,  544,  118,
 /*  4380 */   542,  539, 1404,  538, 1404, 1404,  215, 1404, 1404, 1404,
 /*  4390 */  1404, 1404, 1404,  899, 1404, 1404, 1404,   85,  130,  138,
 /*  4400 */   137,  900,  898,  897,  895,  894,  757,  139, 1404,  165,
 /*  4410 */   163, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4420 */  1404,  583, 1404, 1404, 1404, 1404, 1404, 1404,  175, 1404,
 /*  4430 */  1404, 1404, 1404,  173,  580,  167,  141,  169,  419,  177,
 /*  4440 */  1404,   72, 1404, 1404,  424,  832, 1404, 1404, 1404, 1404,
 /*  4450 */  1404, 1404, 1404, 1404,  574,  743,  758, 1404, 1404,  893,
 /*  4460 */   572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,  245,
 /*  4470 */   242,  241,  244,  243,  240,  238,  239,  236,  237, 1404,
 /*  4480 */  1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404, 1404,
 /*  4490 */  1404,  544,  118,  542,  539, 1404,  538, 1404, 1404,  215,
 /*  4500 */  1404, 1404, 1404, 1404, 1404, 1404,  899, 1404, 1404, 1404,
 /*  4510 */    85,  130,  138,  137,  900,  898,  897,  895,  894,  757,
 /*  4520 */   139, 1404,  165,  163, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4530 */  1404, 1404, 1404, 1404,  583, 1404, 1404, 1404, 1404, 1404,
 /*  4540 */  1404,  175, 1404, 1404, 1404, 1404,  173,  580,  167,  141,
 /*  4550 */   169,  419,  177, 1404,   72, 1404, 1404,  424,  832, 1404,
 /*  4560 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  574,  750,  758,
 /*  4570 */  1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404,
 /*  4580 */  1404,  246,  245,  242,  241,  244,  243,  240,  238,  239,
 /*  4590 */   236,  237, 1404, 1404, 1404,  550,  549,  120, 1404, 1404,
 /*  4600 */   119, 1404, 1404, 1404,  544,  118,  542,  539, 1404,  538,
 /*  4610 */  1404, 1404,  215, 1404, 1404, 1404, 1404, 1404, 1404,  899,
 /*  4620 */  1404, 1404, 1404,   85,  130,  138,  137,  900,  898,  897,
 /*  4630 */   895,  894,  757,  139, 1404,  165,  163, 1404, 1404, 1404,
 /*  4640 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  583, 1404, 1404,
 /*  4650 */  1404, 1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,  173,
 /*  4660 */   580,  167,  141,  169,  419,  177, 1404,   72, 1404, 1404,
 /*  4670 */   424,  832, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4680 */   574,  755,  758, 1404, 1404,  893,  572,   89, 1404, 1404,
 /*  4690 */  1404,  896, 1404, 1404,  246,  245,  242,  241,  244,  243,
 /*  4700 */   240,  238,  239,  236,  237, 1404, 1404, 1404,  550,  549,
 /*  4710 */   120, 1404, 1404,  119, 1404, 1404, 1404,  544,  118,  542,
 /*  4720 */   539, 1404,  538, 1404, 1404,  215, 1404, 1404, 1404, 1404,
 /*  4730 */  1404, 1404,  899, 1404, 1404, 1404,   85,  130,  138,  137,
 /*  4740 */   900,  898,  897,  895,  894,  757,  139, 1404,  165,  163,
 /*  4750 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4760 */   583, 1404, 1404, 1404, 1404, 1404, 1404,  175, 1404, 1404,
 /*  4770 */  1404, 1404,  173,  580,  167,  141,  169,  419,  177, 1404,
 /*  4780 */    72, 1404, 1404,  424,  832, 1404, 1404, 1404, 1404, 1404,
 /*  4790 */  1404, 1404, 1404,  574,  752,  758, 1404, 1404,  893,  572,
 /*  4800 */    89, 1404, 1404, 1404,  896, 1404, 1404,  246,  245,  242,
 /*  4810 */   241,  244,  243,  240,  238,  239,  236,  237, 1404, 1404,
 /*  4820 */  1404,  550,  549,  120, 1404, 1404,  119, 1404, 1404, 1404,
 /*  4830 */   544,  118,  542,  539, 1404,  538, 1404, 1404,  215, 1404,
 /*  4840 */  1404, 1404, 1404, 1404, 1404,  899, 1404, 1404, 1404,   85,
 /*  4850 */   130,  138,  137,  900,  898,  897,  895,  894,  757,  139,
 /*  4860 */  1404,  165,  163, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4870 */  1404, 1404, 1404,  583, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  4880 */   175, 1404, 1404, 1404, 1404,  173,  580,  167,  141,  169,
 /*  4890 */   419,  177, 1404,   72, 1404, 1404,  424,  832, 1404, 1404,
 /*  4900 */  1404, 1404, 1404, 1404, 1404, 1404,  574,  748,  758, 1404,
 /*  4910 */  1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,
 /*  4920 */   246,  245,  242,  241,  244,  243,  240,  238,  239,  236,
 /*  4930 */   237, 1404, 1404, 1404,  550,  549,  120, 1404, 1404,  119,
 /*  4940 */  1404, 1404, 1404,  544,  118,  542,  539, 1404,  538, 1404,
 /*  4950 */  1404,  215, 1404, 1404, 1404, 1404, 1404, 1404,  899, 1404,
 /*  4960 */  1404, 1404,   85,  130,  138,  137,  900,  898,  897,  895,
 /*  4970 */   894,  757,  139, 1404,  165,  163, 1404, 1404, 1404, 1404,
 /*  4980 */  1404, 1404, 1404, 1404, 1404, 1404,  583, 1404, 1404, 1404,
 /*  4990 */  1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,  173,  580,
 /*  5000 */   167,  141,  169,  419,  177, 1404,   72, 1404, 1404,  424,
 /*  5010 */   832, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  574,
 /*  5020 */   735,  758, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,
 /*  5030 */   896, 1404, 1404,  246,  245,  242,  241,  244,  243,  240,
 /*  5040 */   238,  239,  236,  237, 1404, 1404, 1404,  550,  549,  120,
 /*  5050 */  1404, 1404,  119, 1404, 1404, 1404,  544,  118,  542,  539,
 /*  5060 */  1404,  538, 1404, 1404,  215, 1404, 1404, 1404, 1404, 1404,
 /*  5070 */  1404,  899, 1404, 1404, 1404,   85,  130,  138,  137,  900,
 /*  5080 */   898,  897,  895,  894,  757,  139, 1404,  165,  163, 1404,
 /*  5090 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  583,
 /*  5100 */  1404, 1404, 1404, 1404, 1404, 1404,  175, 1404, 1404, 1404,
 /*  5110 */  1404,  173,  580,  167,  141,  169,  419,  177, 1404,   72,
 /*  5120 */  1404, 1404,  424,  832, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  5130 */  1404, 1404,  574,  733,  758, 1404, 1404,  893,  572,   89,
 /*  5140 */  1404, 1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,
 /*  5150 */   244,  243,  240,  238,  239,  236,  237, 1404, 1404, 1404,
 /*  5160 */   550,  549,  120, 1404, 1404,  119, 1404, 1404, 1404,  544,
 /*  5170 */   118,  542,  539, 1404,  538, 1404, 1404,  215, 1404, 1404,
 /*  5180 */  1404, 1404, 1404, 1404,  899, 1404, 1404, 1404,   85,  130,
 /*  5190 */   138,  137,  900,  898,  897,  895,  894,  757,  139, 1404,
 /*  5200 */   165,  163, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  5210 */  1404, 1404,  583, 1404, 1404, 1404, 1404, 1404, 1404,  175,
 /*  5220 */  1404, 1404, 1404, 1404,  173,  580,  167,  141,  169,  419,
 /*  5230 */   177, 1404,   72, 1404, 1404,  424,  832, 1404, 1404, 1404,
 /*  5240 */  1404, 1404, 1404, 1404, 1404,  574,  729,  758, 1404, 1404,
 /*  5250 */   893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,
 /*  5260 */   245,  242,  241,  244,  243,  240,  238,  239,  236,  237,
 /*  5270 */  1404, 1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404,
 /*  5280 */  1404, 1404,  544,  118,  542,  539, 1404,  538, 1404, 1404,
 /*  5290 */   215, 1404, 1404, 1404, 1404, 1404, 1404,  899, 1404, 1404,
 /*  5300 */  1404,   85,  130,  138,  137,  900,  898,  897,  895,  894,
 /*  5310 */   757,  139, 1404,  165,  163, 1404, 1404, 1404, 1404, 1404,
 /*  5320 */  1404, 1404, 1404, 1404, 1404,  583, 1404, 1404, 1404, 1404,
 /*  5330 */  1404, 1404,  175, 1404, 1404, 1404, 1404,  173,  580,  167,
 /*  5340 */   141,  169,  419,  177, 1404,   72, 1404, 1404,  424,  832,
 /*  5350 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  574,  724,
 /*  5360 */   758, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896,
 /*  5370 */  1404, 1404,  246,  245,  242,  241,  244,  243,  240,  238,
 /*  5380 */   239,  236,  237, 1404, 1404, 1404,  550,  549,  120, 1404,
 /*  5390 */  1404,  119, 1404, 1404, 1404,  544,  118,  542,  539, 1404,
 /*  5400 */   538, 1404, 1404,  215, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  5410 */   899, 1404, 1404, 1404,   85,  130,  138,  137,  900,  898,
 /*  5420 */   897,  895,  894,  757,  139, 1404,  165,  163, 1404, 1404,
 /*  5430 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  583, 1404,
 /*  5440 */  1404, 1404, 1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,
 /*  5450 */   173,  580,  167,  141,  169,  419,  177, 1404,   72, 1404,
 /*  5460 */  1404,  424,  832, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  5470 */  1404,  574,  655,  758, 1404, 1404,  893,  572,   89, 1404,
 /*  5480 */  1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,  244,
 /*  5490 */   243,  240,  238,  239,  236,  237, 1404, 1404, 1404,  550,
 /*  5500 */   549,  120, 1404, 1404,  119, 1404, 1404, 1404,  544,  118,
 /*  5510 */   542,  539, 1404,  538, 1404, 1404,  215, 1404, 1404, 1404,
 /*  5520 */  1404, 1404, 1404,  899, 1404, 1404, 1404,   85,  130,  138,
 /*  5530 */   137,  900,  898,  897,  895,  894,  757,  139, 1404,  165,
 /*  5540 */   163, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  5550 */  1404,  583, 1404, 1404, 1404, 1404, 1404, 1404,  175, 1404,
 /*  5560 */  1404, 1404, 1404,  173,  580,  167,  141,  169,  419,  177,
 /*  5570 */  1404,   72, 1404, 1404,  424,  832, 1404, 1404, 1404, 1404,
 /*  5580 */  1404, 1404, 1404, 1404,  574,  652,  758, 1404, 1404,  893,
 /*  5590 */   572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,  245,
 /*  5600 */   242,  241,  244,  243,  240,  238,  239,  236,  237, 1404,
 /*  5610 */  1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404, 1404,
 /*  5620 */  1404,  544,  118,  542,  539, 1404,  538, 1404, 1404,  215,
 /*  5630 */  1404, 1404, 1404, 1404, 1404, 1404,  899, 1404, 1404, 1404,
 /*  5640 */    85,  130,  138,  137,  900,  898,  897,  895,  894,  757,
 /*  5650 */   139, 1404,  165,  163, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  5660 */  1404, 1404, 1404, 1404,  583, 1404, 1404, 1404, 1404, 1404,
 /*  5670 */  1404,  175, 1404, 1404, 1404, 1404,  173,  580,  167,  141,
 /*  5680 */   169,  419,  177, 1404,   72, 1404, 1404,  424,  832, 1404,
 /*  5690 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  574,  649,  758,
 /*  5700 */  1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404,
 /*  5710 */  1404,  246,  245,  242,  241,  244,  243,  240,  238,  239,
 /*  5720 */   236,  237, 1404, 1404, 1404,  550,  549,  120, 1404, 1404,
 /*  5730 */   119, 1404, 1404, 1404,  544,  118,  542,  539, 1404,  538,
 /*  5740 */  1404, 1404,  215, 1404, 1404, 1404, 1404, 1404, 1404,  899,
 /*  5750 */  1404, 1404, 1404,   85,  130,  138,  137,  900,  898,  897,
 /*  5760 */   895,  894,  757,  139, 1404,  165,  163, 1404, 1404, 1404,
 /*  5770 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  583, 1404, 1404,
 /*  5780 */  1404, 1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,  173,
 /*  5790 */   580,  167,  141,  169,  419,  177, 1404,   72, 1404, 1404,
 /*  5800 */   424,  832, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  5810 */   574,  646,  758, 1404, 1404,  893,  572,   89, 1404, 1404,
 /*  5820 */  1404,  896, 1404, 1404,  246,  245,  242,  241,  244,  243,
 /*  5830 */   240,  238,  239,  236,  237, 1404, 1404, 1404,  550,  549,
 /*  5840 */   120, 1404, 1404,  119, 1404, 1404, 1404,  544,  118,  542,
 /*  5850 */   539, 1404,  538, 1404, 1404,  215, 1404, 1404, 1404, 1404,
 /*  5860 */  1404, 1404,  899, 1404, 1404, 1404,   85,  130,  138,  137,
 /*  5870 */   900,  898,  897,  895,  894,  757,  139, 1404,  165,  163,
 /*  5880 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  5890 */   583, 1404, 1404, 1404, 1404, 1404, 1404,  175, 1404, 1404,
 /*  5900 */  1404, 1404,  173,  580,  167,  141,  169,  419,  177, 1404,
 /*  5910 */    72, 1404, 1404,  424,  832, 1404, 1404, 1404, 1404, 1404,
 /*  5920 */  1404, 1404, 1404,  574,  643,  758, 1404, 1404,  893,  572,
 /*  5930 */    89, 1404, 1404, 1404,  896, 1404, 1404,  246,  245,  242,
 /*  5940 */   241,  244,  243,  240,  238,  239,  236,  237, 1404, 1404,
 /*  5950 */  1404,  550,  549,  120, 1404, 1404,  119, 1404, 1404, 1404,
 /*  5960 */   544,  118,  542,  539, 1404,  538, 1404, 1404,  215, 1404,
 /*  5970 */  1404, 1404, 1404, 1404, 1404,  899, 1404, 1404, 1404,   85,
 /*  5980 */   130,  138,  137,  900,  898,  897,  895,  894,  757,  139,
 /*  5990 */  1404,  165,  163, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  6000 */  1404, 1404, 1404,  583, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  6010 */   175, 1404, 1404, 1404, 1404,  173,  580,  167,  141,  169,
 /*  6020 */   419,  177, 1404,   72, 1404, 1404,  424,  832, 1404, 1404,
 /*  6030 */  1404, 1404, 1404, 1404, 1404, 1404,  574,  640,  758, 1404,
 /*  6040 */  1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,
 /*  6050 */   246,  245,  242,  241,  244,  243,  240,  238,  239,  236,
 /*  6060 */   237, 1404, 1404, 1404,  550,  549,  120, 1404, 1404,  119,
 /*  6070 */  1404, 1404, 1404,  544,  118,  542,  539, 1404,  538, 1404,
 /*  6080 */  1404,  215, 1404, 1404, 1404, 1404, 1404, 1404,  899, 1404,
 /*  6090 */  1404, 1404,   85,  130,  138,  137,  900,  898,  897,  895,
 /*  6100 */   894,  757,  139, 1404,  165,  163, 1404, 1404, 1404, 1404,
 /*  6110 */  1404, 1404, 1404, 1404, 1404, 1404,  583, 1404, 1404, 1404,
 /*  6120 */  1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,  173,  580,
 /*  6130 */   167,  141,  169,  419,  177, 1404,   72, 1404, 1404,  424,
 /*  6140 */   832, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  574,
 /*  6150 */   637,  758, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,
 /*  6160 */   896, 1404, 1404,  246,  245,  242,  241,  244,  243,  240,
 /*  6170 */   238,  239,  236,  237, 1404, 1404, 1404,  550,  549,  120,
 /*  6180 */  1404, 1404,  119, 1404, 1404, 1404,  544,  118,  542,  539,
 /*  6190 */  1404,  538, 1404, 1404,  215, 1404, 1404, 1404, 1404, 1404,
 /*  6200 */  1404,  899, 1404, 1404, 1404,   85,  130,  138,  137,  900,
 /*  6210 */   898,  897,  895,  894,  757,  139, 1404,  165,  163, 1404,
 /*  6220 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  583,
 /*  6230 */  1404, 1404, 1404, 1404, 1404, 1404,  175, 1404, 1404, 1404,
 /*  6240 */  1404,  173,  580,  167,  141,  169,  419,  177, 1404,   72,
 /*  6250 */  1404, 1404,  424,  832, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  6260 */  1404, 1404,  574,  634,  758, 1404, 1404,  893,  572,   89,
 /*  6270 */  1404, 1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,
 /*  6280 */   244,  243,  240,  238,  239,  236,  237, 1404, 1404, 1404,
 /*  6290 */   550,  549,  120, 1404, 1404,  119, 1404, 1404, 1404,  544,
 /*  6300 */   118,  542,  539, 1404,  538, 1404, 1404,  215, 1404, 1404,
 /*  6310 */  1404, 1404, 1404, 1404,  899, 1404, 1404, 1404,   85,  130,
 /*  6320 */   138,  137,  900,  898,  897,  895,  894,  757,  139, 1404,
 /*  6330 */   165,  163, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  6340 */  1404, 1404,  583, 1404, 1404, 1404, 1404, 1404, 1404,  175,
 /*  6350 */  1404, 1404, 1404, 1404,  173,  580,  167,  141,  169,  419,
 /*  6360 */   177, 1404,   72, 1404, 1404,  424,  832, 1404, 1404, 1404,
 /*  6370 */  1404, 1404, 1404, 1404, 1404,  574,  602,  758, 1404, 1404,
 /*  6380 */   893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,
 /*  6390 */   245,  242,  241,  244,  243,  240,  238,  239,  236,  237,
 /*  6400 */  1404, 1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404,
 /*  6410 */  1404, 1404,  544,  118,  542,  539, 1404,  538, 1404, 1404,
 /*  6420 */   215, 1404, 1404, 1404, 1404, 1404, 1404,  899, 1404, 1404,
 /*  6430 */  1404,   85,  130,  138,  137,  900,  898,  897,  895,  894,
 /*  6440 */   757,  139, 1404,  165,  163, 1404, 1404, 1404, 1404, 1404,
 /*  6450 */  1404, 1404, 1404, 1404, 1404,  583, 1404, 1404, 1404, 1404,
 /*  6460 */  1404, 1404,  175, 1404, 1404, 1404, 1404,  173,  580,  167,
 /*  6470 */   141,  169,  419,  177, 1404,   72, 1404, 1404,  424,  832,
 /*  6480 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  574,  599,
 /*  6490 */   758, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896,
 /*  6500 */  1404, 1404,  246,  245,  242,  241,  244,  243,  240,  238,
 /*  6510 */   239,  236,  237, 1404, 1404, 1404,  550,  549,  120, 1404,
 /*  6520 */  1404,  119, 1404, 1404, 1404,  544,  118,  542,  539, 1404,
 /*  6530 */   538, 1404, 1404,  215, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  6540 */   899, 1404, 1404, 1404,   85,  130,  138,  137,  900,  898,
 /*  6550 */   897,  895,  894,  757,  139, 1404,  165,  163, 1404, 1404,
 /*  6560 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  583, 1404,
 /*  6570 */  1404, 1404, 1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,
 /*  6580 */   173,  580,  167,  141,  169,  419,  177, 1404,   72, 1404,
 /*  6590 */  1404,  424,  832, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  6600 */  1404,  574,  596,  758, 1404, 1404,  893,  572,   89, 1404,
 /*  6610 */  1404, 1404,  896, 1404, 1404,  246,  245,  242,  241,  244,
 /*  6620 */   243,  240,  238,  239,  236,  237, 1404, 1404, 1404,  550,
 /*  6630 */   549,  120, 1404, 1404,  119, 1404, 1404, 1404,  544,  118,
 /*  6640 */   542,  539, 1404,  538, 1404, 1404,  215, 1404, 1404, 1404,
 /*  6650 */  1404, 1404, 1404,  899, 1404, 1404, 1404,   85,  130,  138,
 /*  6660 */   137,  900,  898,  897,  895,  894,  757,  139, 1404,  165,
 /*  6670 */   163, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  6680 */  1404,  583, 1404, 1404, 1404, 1404, 1404, 1404,  175, 1404,
 /*  6690 */  1404, 1404, 1404,  173,  580,  167,  141,  169,  419,  177,
 /*  6700 */  1404,   72, 1404, 1404,  424,  832, 1404, 1404, 1404, 1404,
 /*  6710 */  1404, 1404, 1404, 1404,  574,  593,  758, 1404, 1404,  893,
 /*  6720 */   572,   89, 1404, 1404, 1404,  896, 1404, 1404,  246,  245,
 /*  6730 */   242,  241,  244,  243,  240,  238,  239,  236,  237, 1404,
 /*  6740 */  1404, 1404,  550,  549,  120, 1404, 1404,  119, 1404, 1404,
 /*  6750 */  1404,  544,  118,  542,  539, 1404,  538, 1404, 1404,  215,
 /*  6760 */  1404, 1404, 1404, 1404, 1404, 1404,  899, 1404, 1404, 1404,
 /*  6770 */    85,  130,  138,  137,  900,  898,  897,  895,  894,  757,
 /*  6780 */   139, 1404,  165,  163, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  6790 */  1404, 1404, 1404, 1404,  583, 1404, 1404, 1404, 1404, 1404,
 /*  6800 */  1404,  175, 1404, 1404, 1404, 1404,  173,  580,  167,  141,
 /*  6810 */   169,  419,  177, 1404,   72, 1404, 1404,  424,  832, 1404,
 /*  6820 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  574, 1404,  758,
 /*  6830 */  1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404,
 /*  6840 */  1404,  246,  245,  242,  241,  244,  243,  240,  238,  239,
 /*  6850 */   236,  237, 1404, 1404, 1404,  550,  549,  120, 1404, 1404,
 /*  6860 */   119, 1404, 1404, 1404,  544,  118,  542,  539,  133,  538,
 /*  6870 */  1404, 1404,  215, 1404, 1404, 1404, 1404, 1404, 1404,  899,
 /*  6880 */  1404, 1404, 1404,   85,  130,  138,  137,  900,  898,  897,
 /*  6890 */   895,  894,  757, 1404, 1404,  580, 1404, 1404, 1404, 1404,
 /*  6900 */  1404, 1404, 1404, 1404,  589,  364, 1404,  249, 1404,  497,
 /*  6910 */  1404,  473, 1404,  461,  458,  574, 1404,  250, 1404, 1404,
 /*  6920 */  1404,  455, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  230,
 /*  6930 */   229,  228,  227,  226,  225,  224,  223,  222,  221,  220,
 /*  6940 */   905,  904,  903,  550,  549,  120, 1404, 1404,  119, 1404,
 /*  6950 */  1404, 1404,  544,  118,  542,  539,  171,  538,  165,  163,
 /*  6960 */   215, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  6970 */   583,   85,  130,  138,  137, 1404, 1404,  175, 1404, 1404,
 /*  6980 */   757, 1404,  173,  580,  167,  141,  169,  419,  177, 1404,
 /*  6990 */    72, 1404, 1404,  424, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  7000 */  1404, 1404, 1404,  574, 1404, 1404, 1404, 1404,  893,  572,
 /*  7010 */    89, 1404, 1404, 1404,  896, 1404, 1404,  916,  915,  914,
 /*  7020 */   913,  912,  911,  910,  909,  908,  907,  906,  905,  904,
 /*  7030 */   903,  147,  153,  154,  151,  152,  150,  149,  148,  176,
 /*  7040 */   172,  168,  166,  164,  162,  170,  174,  160,  159,  155,
 /*  7050 */   158,  157,  156, 1404, 1404,  899, 1404, 1404, 1404, 1404,
 /*  7060 */  1404,  161,  429,  900,  898,  897,  895,  894, 1404, 1404,
 /*  7070 */  1404, 1404, 1404, 1404, 1403,  590,    1,  591,  948,  947,
 /*  7080 */   946,  945,  944,  943,  942,  941,  940,  939,  938,  937,
 /*  7090 */   936,  935,  934,  933,  932,  931,  930,  929,  928,  927,
 /*  7100 */   926,  925,  924,  923,  146, 1404, 1404,  147,  153,  154,
 /*  7110 */   151,  152,  150,  149,  148,  176,  172,  168,  166,  164,
 /*  7120 */   162,  170,  174,  160,  159,  155,  158,  157,  156, 1404,
 /*  7130 */   451, 1404, 1404, 1404, 1404, 1404, 1404,  161,  429,  176,
 /*  7140 */   172,  168,  166,  164,  162,  170,  174,  160,  159,  155,
 /*  7150 */   158,  157,  156, 1404,  329, 1404, 1404,  437,  436,  435,
 /*  7160 */  1404,  161,  429,  917,  886,  884,  885,  901, 1404, 1404,
 /*  7170 */   420, 1404, 1404,  713,  504,  714, 1404, 1404, 1404, 1404,
 /*  7180 */   949,  948,  947,  946,  945,  944,  943,  942,  941,  940,
 /*  7190 */   939,  938,  937,  936,  935,  934,  933,  932,  931,  930,
 /*  7200 */   929,  928,  927,  926,  925,  924,  923, 1404, 1404, 1404,
 /*  7210 */  1404, 1404,  834,  855,  854,  853,  852,  851,  850,  849,
 /*  7220 */   848,  847,  845,  844,  843,  842,  841,  840,  839,  838,
 /*  7230 */   837,  836,  835,  451, 1404, 1404,  329, 1404, 1404, 1404,
 /*  7240 */  1404, 1404, 1404, 1404, 1404, 1404,  886,  884,  885,  901,
 /*  7250 */  1404, 1404, 1404, 1404,   71,  716,  504,  714, 1404, 1404,
 /*  7260 */   437,  436,  435,  756,  846,  833, 1404, 1404, 1404, 1404,
 /*  7270 */  1404, 1404, 1404, 1404, 1404,  323, 1404, 1404, 1404, 1404,
 /*  7280 */  1404, 1404, 1404, 1404, 1404,  529,  528,  527,  526,  834,
 /*  7290 */   855,  854,  853,  852,  851,  850,  849,  848,  847,  845,
 /*  7300 */   844,  843,  842,  841,  840,  839,  838,  837,  836,  835,
 /*  7310 */   154,  151,  152,  150,  149,  148,  176,  172,  168,  166,
 /*  7320 */   164,  162,  170,  174,  160,  159,  155,  158,  157,  156,
 /*  7330 */  1404,   60, 1404, 1404, 1404, 1404, 1404, 1404,  161,  429,
 /*  7340 */   756,  846,  833, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  7350 */  1404, 1404,  323, 1404, 1404, 1404, 1404, 1404,  446, 1404,
 /*  7360 */  1404, 1404,  529,  528,  527,  526,  916,  915,  914,  913,
 /*  7370 */   912,  911,  910,  909,  908,  907,  906,  905,  904,  903,
 /*  7380 */  1404, 1404, 1404, 1404, 1404,  834,  855,  854,  853,  852,
 /*  7390 */   851,  850,  849,  848,  847,  845,  844,  843,  842,  841,
 /*  7400 */   840,  839,  838,  837,  836,  835,  151,  152,  150,  149,
 /*  7410 */   148,  176,  172,  168,  166,  164,  162,  170,  174,  160,
 /*  7420 */   159,  155,  158,  157,  156, 1404, 1404,   58,   91,  409,
 /*  7430 */  1404, 1404,  180,  161,  429,  570,  756,  846,  833,   94,
 /*  7440 */  1404, 1404, 1404, 1404,  817, 1404, 1404, 1404,  323, 1404,
 /*  7450 */   872, 1404, 1404, 1404, 1404,  817,  876, 1404,  529,  528,
 /*  7460 */   527,  526,  834,  855,  854,  853,  852,  851,  850,  849,
 /*  7470 */   848,  847,  845,  844,  843,  842,  841,  840,  839,  838,
 /*  7480 */   837,  836,  835, 1404, 1404, 1404, 1404,  816,  815,  814,
 /*  7490 */   813,  812,  811,  558,  534,  791,  790,  878,  816,  815,
 /*  7500 */   814,  813,  812,  811,   70,  880,  879,  877,  874,  875,
 /*  7510 */  1404, 1404,  512,  756,  846,  833,  571, 1404, 1404, 1404,
 /*  7520 */  1404, 1404, 1404, 1404, 1404,  323, 1404, 1404, 1404, 1404,
 /*  7530 */   204, 1404, 1404, 1404,  558,  529,  528,  527,  526, 1404,
 /*  7540 */   916,  915,  914,  913,  912,  911,  910,  909,  908,  907,
 /*  7550 */   906,  905,  904,  903, 1404, 1404, 1404,  564,  834,  855,
 /*  7560 */   854,  853,  852,  851,  850,  849,  848,  847,  845,  844,
 /*  7570 */   843,  842,  841,  840,  839,  838,  837,  836,  835, 1404,
 /*  7580 */  1404,  916,  915,  914,  913,  912,  911,  910,  909,  908,
 /*  7590 */   907,  906,  905,  904,  903, 1404, 1404, 1404, 1404,  180,
 /*  7600 */    44, 1404,  566, 1404, 1404, 1404, 1404, 1404, 1404,  756,
 /*  7610 */   846,  833, 1404, 1404, 1404, 1404, 1404,  872, 1404, 1404,
 /*  7620 */  1404,  323, 1404,  876, 1404,  871, 1404, 1404, 1404, 1404,
 /*  7630 */  1404,  529,  528,  527,  526,  834,  855,  854,  853,  852,
 /*  7640 */   851,  850,  849,  848,  847,  845,  844,  843,  842,  841,
 /*  7650 */   840,  839,  838,  837,  836,  835, 1404, 1404, 1404, 1404,
 /*  7660 */  1404, 1404, 1404, 1404,  567, 1404,  558, 1404, 1404, 1404,
 /*  7670 */  1404, 1404,  568,  879,  877,  874,  875,   42, 1404, 1404,
 /*  7680 */  1404, 1404, 1404, 1404, 1404,  384,  756,  846,  833,  571,
 /*  7690 */  1404, 1404, 1404, 1404, 1404, 1404,   95, 1404,  323, 1404,
 /*  7700 */  1404, 1404, 1404,  204, 1404, 1404, 1404, 1404,  529,  528,
 /*  7710 */   527,  526,  817,  916,  915,  914,  913,  912,  911,  910,
 /*  7720 */   909,  908,  907,  906,  905,  904,  903, 1404, 1404, 1404,
 /*  7730 */  1404,  834,  855,  854,  853,  852,  851,  850,  849,  848,
 /*  7740 */   847,  845,  844,  843,  842,  841,  840,  839,  838,  837,
 /*  7750 */   836,  835, 1404, 1404,  817,  816,  815,  814,  813,  812,
 /*  7760 */   811, 1404, 1404,  802,  801, 1404, 1404,  180, 1404, 1404,
 /*  7770 */   566, 1404, 1404,   40,   92, 1404, 1404, 1404, 1404, 1404,
 /*  7780 */  1404, 1404,  756,  846,  833,  872, 1404, 1404, 1404, 1404,
 /*  7790 */   817,  876, 1404, 1404,  323, 1404, 1404,  816,  815,  814,
 /*  7800 */   813,  812,  811, 1404,  529,  528,  527,  526,  834,  855,
 /*  7810 */   854,  853,  852,  851,  850,  849,  848,  847,  845,  844,
 /*  7820 */   843,  842,  841,  840,  839,  838,  837,  836,  835, 1404,
 /*  7830 */  1404, 1404,  567,  816,  815,  814,  813,  812,  811,  558,
 /*  7840 */   568,  879,  877,  874,  875, 1404, 1404, 1404, 1404, 1404,
 /*  7850 */    38, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  380,  756,
 /*  7860 */   846,  833,  571, 1404, 1404, 1404, 1404, 1404, 1404,   90,
 /*  7870 */  1404,  323, 1404, 1404, 1404, 1404,  204, 1404, 1404, 1404,
 /*  7880 */  1404,  529,  528,  527,  526,  817,  916,  915,  914,  913,
 /*  7890 */   912,  911,  910,  909,  908,  907,  906,  905,  904,  903,
 /*  7900 */  1404, 1404, 1404, 1404,  834,  855,  854,  853,  852,  851,
 /*  7910 */   850,  849,  848,  847,  845,  844,  843,  842,  841,  840,
 /*  7920 */   839,  838,  837,  836,  835, 1404, 1404, 1404,  816,  815,
 /*  7930 */   814,  813,  812,  811, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  7940 */  1404, 1404, 1404, 1404, 1404, 1404,   69,   93, 1404, 1404,
 /*  7950 */  1404, 1404, 1404, 1404, 1404,  756,  846,  833, 1404, 1404,
 /*  7960 */  1404, 1404, 1404,  817, 1404, 1404, 1404,  323, 1404, 1404,
 /*  7970 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  529,  528,  527,
 /*  7980 */   526,  834,  855,  854,  853,  852,  851,  850,  849,  848,
 /*  7990 */   847,  845,  844,  843,  842,  841,  840,  839,  838,  837,
 /*  8000 */   836,  835, 1404, 1404, 1404, 1404,  816,  815,  814,  813,
 /*  8010 */   812,  811,  558, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8020 */  1404, 1404, 1404,   68, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8030 */  1404,  360,  756,  846,  833,  571, 1404, 1404, 1404, 1404,
 /*  8040 */  1404, 1404, 1404, 1404,  323, 1404, 1404, 1404, 1404,  204,
 /*  8050 */  1404, 1404, 1404, 1404,  529,  528,  527,  526, 1404,  916,
 /*  8060 */   915,  914,  913,  912,  911,  910,  909,  908,  907,  906,
 /*  8070 */   905,  904,  903, 1404, 1404, 1404, 1404,  834,  855,  854,
 /*  8080 */   853,  852,  851,  850,  849,  848,  847,  845,  844,  843,
 /*  8090 */   842,  841,  840,  839,  838,  837,  836,  835, 1404, 1404,
 /*  8100 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8110 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,   67,
 /*  8120 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  756,  846,
 /*  8130 */   833, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8140 */   323, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8150 */   529,  528,  527,  526,  834,  855,  854,  853,  852,  851,
 /*  8160 */   850,  849,  848,  847,  845,  844,  843,  842,  841,  840,
 /*  8170 */   839,  838,  837,  836,  835, 1404, 1404, 1404, 1404, 1404,
 /*  8180 */  1404, 1404, 1404, 1404, 1404,  558, 1404, 1404, 1404, 1404,
 /*  8190 */  1404, 1404, 1404, 1404, 1404, 1404,   66, 1404, 1404, 1404,
 /*  8200 */  1404, 1404, 1404, 1404,  358,  756,  846,  833,  571, 1404,
 /*  8210 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  323, 1404, 1404,
 /*  8220 */  1404, 1404,  204, 1404, 1404, 1404, 1404,  529,  528,  527,
 /*  8230 */   526, 1404,  916,  915,  914,  913,  912,  911,  910,  909,
 /*  8240 */   908,  907,  906,  905,  904,  903, 1404, 1404, 1404, 1404,
 /*  8250 */   834,  855,  854,  853,  852,  851,  850,  849,  848,  847,
 /*  8260 */   845,  844,  843,  842,  841,  840,  839,  838,  837,  836,
 /*  8270 */   835, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8280 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8290 */  1404, 1404,   65, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8300 */  1404,  756,  846,  833, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8310 */  1404, 1404, 1404,  323, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8320 */  1404, 1404, 1404,  529,  528,  527,  526,  834,  855,  854,
 /*  8330 */   853,  852,  851,  850,  849,  848,  847,  845,  844,  843,
 /*  8340 */   842,  841,  840,  839,  838,  837,  836,  835, 1404, 1404,
 /*  8350 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  558, 1404,
 /*  8360 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,   64,
 /*  8370 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  453,  756,  846,
 /*  8380 */   833,  571, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8390 */   323, 1404, 1404, 1404, 1404,  204, 1404, 1404, 1404, 1404,
 /*  8400 */   529,  528,  527,  526, 1404,  916,  915,  914,  913,  912,
 /*  8410 */   911,  910,  909,  908,  907,  906,  905,  904,  903, 1404,
 /*  8420 */  1404, 1404, 1404,  834,  855,  854,  853,  852,  851,  850,
 /*  8430 */   849,  848,  847,  845,  844,  843,  842,  841,  840,  839,
 /*  8440 */   838,  837,  836,  835, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8450 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8460 */  1404, 1404, 1404, 1404, 1404,   63, 1404, 1404, 1404, 1404,
 /*  8470 */  1404, 1404, 1404, 1404,  756,  846,  833, 1404, 1404, 1404,
 /*  8480 */  1404, 1404, 1404, 1404, 1404, 1404,  323, 1404, 1404, 1404,
 /*  8490 */  1404, 1404, 1404, 1404, 1404, 1404,  529,  528,  527,  526,
 /*  8500 */   834,  855,  854,  853,  852,  851,  850,  849,  848,  847,
 /*  8510 */   845,  844,  843,  842,  841,  840,  839,  838,  837,  836,
 /*  8520 */   835, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8530 */  1404,  558, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8540 */  1404, 1404,   62, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8550 */   448,  756,  846,  833,  571, 1404, 1404, 1404, 1404, 1404,
 /*  8560 */  1404, 1404, 1404,  323, 1404, 1404, 1404, 1404,  204, 1404,
 /*  8570 */  1404, 1404, 1404,  529,  528,  527,  526, 1404,  916,  915,
 /*  8580 */   914,  913,  912,  911,  910,  909,  908,  907,  906,  905,
 /*  8590 */   904,  903, 1404, 1404, 1404, 1404,  834,  855,  854,  853,
 /*  8600 */   852,  851,  850,  849,  848,  847,  845,  844,  843,  842,
 /*  8610 */   841,  840,  839,  838,  837,  836,  835, 1404, 1404, 1404,
 /*  8620 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8630 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,   61, 1404,
 /*  8640 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  756,  846,  833,
 /*  8650 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  323,
 /*  8660 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  529,
 /*  8670 */   528,  527,  526,  834,  855,  854,  853,  852,  851,  850,
 /*  8680 */   849,  848,  847,  845,  844,  843,  842,  841,  840,  839,
 /*  8690 */   838,  837,  836,  835, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8700 */  1404, 1404, 1404, 1404,  558, 1404, 1404, 1404, 1404, 1404,
 /*  8710 */  1404, 1404, 1404, 1404, 1404,   59, 1404, 1404, 1404, 1404,
 /*  8720 */  1404, 1404, 1404,  444,  756,  846,  833,  571, 1404, 1404,
 /*  8730 */  1404, 1404, 1404, 1404, 1404, 1404,  323, 1404, 1404, 1404,
 /*  8740 */  1404,  204, 1404, 1404, 1404, 1404,  529,  528,  527,  526,
 /*  8750 */  1404,  916,  915,  914,  913,  912,  911,  910,  909,  908,
 /*  8760 */   907,  906,  905,  904,  903, 1404, 1404, 1404, 1404,  834,
 /*  8770 */   855,  854,  853,  852,  851,  850,  849,  848,  847,  845,
 /*  8780 */   844,  843,  842,  841,  840,  839,  838,  837,  836,  835,
 /*  8790 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8800 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8810 */  1404,   57, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8820 */   756,  846,  833, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8830 */  1404, 1404,  323, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8840 */  1404, 1404,  529,  528,  527,  526,  834,  855,  854,  853,
 /*  8850 */   852,  851,  850,  849,  848,  847,  845,  844,  843,  842,
 /*  8860 */   841,  840,  839,  838,  837,  836,  835, 1404, 1404, 1404,
 /*  8870 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  558, 1404, 1404,
 /*  8880 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,   56, 1404,
 /*  8890 */  1404, 1404, 1404, 1404, 1404, 1404,  439,  756,  846,  833,
 /*  8900 */   571, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  323,
 /*  8910 */  1404, 1404, 1404, 1404,  204, 1404, 1404, 1404, 1404,  529,
 /*  8920 */   528,  527,  526, 1404,  916,  915,  914,  913,  912,  911,
 /*  8930 */   910,  909,  908,  907,  906,  905,  904,  903, 1404, 1404,
 /*  8940 */  1404, 1404,  834,  855,  854,  853,  852,  851,  850,  849,
 /*  8950 */   848,  847,  845,  844,  843,  842,  841,  840,  839,  838,
 /*  8960 */   837,  836,  835, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8970 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  8980 */  1404, 1404, 1404, 1404,   55, 1404, 1404, 1404, 1404, 1404,
 /*  8990 */  1404, 1404, 1404,  756,  846,  833, 1404, 1404, 1404, 1404,
 /*  9000 */  1404, 1404, 1404, 1404, 1404,  323, 1404, 1404, 1404, 1404,
 /*  9010 */  1404, 1404, 1404, 1404, 1404,  529,  528,  527,  526,  834,
 /*  9020 */   855,  854,  853,  852,  851,  850,  849,  848,  847,  845,
 /*  9030 */   844,  843,  842,  841,  840,  839,  838,  837,  836,  835,
 /*  9040 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9050 */  1404, 1404, 1404,  489, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9060 */  1404,   54, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9070 */   756,  846,  833, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9080 */  1404, 1404,  323, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9090 */  1404, 1404,  529,  528,  527,  526,  695,  692,  691, 1404,
 /*  9100 */   916,  915,  914,  913,  912,  911,  910,  909,  908,  907,
 /*  9110 */   906,  905,  904,  903, 1404,  834,  855,  854,  853,  852,
 /*  9120 */   851,  850,  849,  848,  847,  845,  844,  843,  842,  841,
 /*  9130 */   840,  839,  838,  837,  836,  835, 1404, 1404, 1404, 1404,
 /*  9140 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9150 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,   53, 1404, 1404,
 /*  9160 */  1404, 1404, 1404, 1404, 1404, 1404,  756,  846,  833, 1404,
 /*  9170 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  323, 1404,
 /*  9180 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  529,  528,
 /*  9190 */   527,  526,  834,  855,  854,  853,  852,  851,  850,  849,
 /*  9200 */   848,  847,  845,  844,  843,  842,  841,  840,  839,  838,
 /*  9210 */   837,  836,  835, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9220 */  1404, 1404, 1404,  558, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9230 */  1404, 1404, 1404, 1404,   52, 1404, 1404, 1404, 1404, 1404,
 /*  9240 */  1404, 1404, 1404,  756,  846,  833,  571, 1404, 1404, 1404,
 /*  9250 */  1404, 1404, 1404, 1404, 1404,  323, 1404, 1404, 1404, 1404,
 /*  9260 */   204, 1404, 1404, 1404, 1404,  529,  528,  527,  526, 1404,
 /*  9270 */   916,  915,  914,  913,  912,  911,  910,  909,  908,  907,
 /*  9280 */   906,  905,  904,  903, 1404, 1404, 1404, 1404,  834,  855,
 /*  9290 */   854,  853,  852,  851,  850,  849,  848,  847,  845,  844,
 /*  9300 */   843,  842,  841,  840,  839,  838,  837,  836,  835, 1404,
 /*  9310 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9320 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9330 */    51, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  756,
 /*  9340 */   846,  833, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9350 */  1404,  323, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9360 */  1404,  529,  528,  527,  526,  834,  855,  854,  853,  852,
 /*  9370 */   851,  850,  849,  848,  847,  845,  844,  843,  842,  841,
 /*  9380 */   840,  839,  838,  837,  836,  835, 1404, 1404, 1404, 1404,
 /*  9390 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  489, 1404,
 /*  9400 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,   50, 1404, 1404,
 /*  9410 */  1404, 1404, 1404, 1404, 1404, 1404,  756,  846,  833, 1404,
 /*  9420 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  323, 1404,
 /*  9430 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  529,  528,
 /*  9440 */   527,  526,  692,  691, 1404,  916,  915,  914,  913,  912,
 /*  9450 */   911,  910,  909,  908,  907,  906,  905,  904,  903, 1404,
 /*  9460 */  1404,  834,  855,  854,  853,  852,  851,  850,  849,  848,
 /*  9470 */   847,  845,  844,  843,  842,  841,  840,  839,  838,  837,
 /*  9480 */   836,  835, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9490 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9500 */  1404, 1404, 1404,   49, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9510 */  1404, 1404,  756,  846,  833, 1404, 1404, 1404, 1404, 1404,
 /*  9520 */  1404, 1404, 1404, 1404,  323, 1404, 1404, 1404, 1404, 1404,
 /*  9530 */  1404, 1404, 1404, 1404,  529,  528,  527,  526,  834,  855,
 /*  9540 */   854,  853,  852,  851,  850,  849,  848,  847,  845,  844,
 /*  9550 */   843,  842,  841,  840,  839,  838,  837,  836,  835, 1404,
 /*  9560 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9570 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9580 */    48, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  756,
 /*  9590 */   846,  833, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9600 */  1404,  323, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9610 */  1404,  529,  528,  527,  526, 1404, 1404, 1404, 1404, 1404,
 /*  9620 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9630 */  1404, 1404, 1404, 1404,  834,  855,  854,  853,  852,  851,
 /*  9640 */   850,  849,  848,  847,  845,  844,  843,  842,  841,  840,
 /*  9650 */   839,  838,  837,  836,  835, 1404, 1404, 1404, 1404, 1404,
 /*  9660 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9670 */  1404, 1404, 1404, 1404, 1404, 1404,   47, 1404, 1404, 1404,
 /*  9680 */  1404, 1404, 1404, 1404, 1404,  756,  846,  833, 1404, 1404,
 /*  9690 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  323, 1404, 1404,
 /*  9700 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  529,  528,  527,
 /*  9710 */   526,  834,  855,  854,  853,  852,  851,  850,  849,  848,
 /*  9720 */   847,  845,  844,  843,  842,  841,  840,  839,  838,  837,
 /*  9730 */   836,  835, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9740 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9750 */  1404, 1404, 1404,   46, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9760 */  1404, 1404,  756,  846,  833, 1404, 1404, 1404, 1404, 1404,
 /*  9770 */  1404, 1404, 1404, 1404,  323, 1404, 1404, 1404, 1404, 1404,
 /*  9780 */  1404, 1404, 1404, 1404,  529,  528,  527,  526, 1404, 1404,
 /*  9790 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9800 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  834,  855,  854,
 /*  9810 */   853,  852,  851,  850,  849,  848,  847,  845,  844,  843,
 /*  9820 */   842,  841,  840,  839,  838,  837,  836,  835, 1404, 1404,
 /*  9830 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9840 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,   45,
 /*  9850 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  756,  846,
 /*  9860 */   833, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9870 */   323, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9880 */   529,  528,  527,  526,  834,  855,  854,  853,  852,  851,
 /*  9890 */   850,  849,  848,  847,  845,  844,  843,  842,  841,  840,
 /*  9900 */   839,  838,  837,  836,  835, 1404, 1404, 1404, 1404, 1404,
 /*  9910 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9920 */  1404, 1404, 1404, 1404, 1404, 1404,   43, 1404, 1404, 1404,
 /*  9930 */  1404, 1404, 1404, 1404, 1404,  756,  846,  833, 1404, 1404,
 /*  9940 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  323, 1404, 1404,
 /*  9950 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  529,  528,  527,
 /*  9960 */   526, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9970 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /*  9980 */   834,  855,  854,  853,  852,  851,  850,  849,  848,  847,
 /*  9990 */   845,  844,  843,  842,  841,  840,  839,  838,  837,  836,
 /* 10000 */   835, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10010 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10020 */  1404, 1404,   41, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10030 */  1404,  756,  846,  833, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10040 */  1404, 1404, 1404,  323, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10050 */  1404, 1404, 1404,  529,  528,  527,  526,  834,  855,  854,
 /* 10060 */   853,  852,  851,  850,  849,  848,  847,  845,  844,  843,
 /* 10070 */   842,  841,  840,  839,  838,  837,  836,  835, 1404, 1404,
 /* 10080 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10090 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,   39,
 /* 10100 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  756,  846,
 /* 10110 */   833, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10120 */   323, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10130 */   529,  528,  527,  526, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10140 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10150 */  1404, 1404, 1404,  834,  855,  854,  853,  852,  851,  850,
 /* 10160 */   849,  848,  847,  845,  844,  843,  842,  841,  840,  839,
 /* 10170 */   838,  837,  836,  835, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10180 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10190 */  1404, 1404, 1404, 1404, 1404,   37, 1404, 1404, 1404, 1404,
 /* 10200 */  1404, 1404, 1404, 1404,  756,  846,  833, 1404, 1404, 1404,
 /* 10210 */  1404, 1404, 1404, 1404, 1404, 1404,  323, 1404, 1404, 1404,
 /* 10220 */  1404, 1404, 1404, 1404, 1404, 1404,  529,  528,  527,  526,
 /* 10230 */   834,  855,  854,  853,  852,  851,  850,  849,  848,  847,
 /* 10240 */   845,  844,  843,  842,  841,  840,  839,  838,  837,  836,
 /* 10250 */   835, 1404,  146, 1404, 1404,  147,  153,  154,  151,  152,
 /* 10260 */   150,  149,  148,  176,  172,  168,  166,  164,  162,  170,
 /* 10270 */   174,  160,  159,  155,  158,  157,  156, 1404, 1404, 1404,
 /* 10280 */  1404,  856,  846,  833, 1404,  161,  429, 1404, 1404, 1404,
 /* 10290 */  1404, 1404, 1404,  323, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10300 */  1404, 1404, 1404,  529,  528,  527,  526, 1404, 1404, 1404,
 /* 10310 */  1404,  171, 1404,  165,  163, 1404, 1404, 1404, 1404,  140,
 /* 10320 */  1404, 1404, 1404, 1404, 1404,  583, 1404, 1404, 1404, 1404,
 /* 10330 */  1404, 1404,  175,  145, 1404, 1404, 1404,  173,  580,  167,
 /* 10340 */   141,  169,  419,  177,  702,   72, 1404,  171,  363,  165,
 /* 10350 */   163, 1404, 1404, 1404, 1404,  140, 1404, 1404,  574, 1404,
 /* 10360 */  1404,  583, 1404,  893,  572,   89, 1404, 1404,  175,  896,
 /* 10370 */  1404, 1404, 1404,  173,  580,  167,  141,  169,  419,  177,
 /* 10380 */   708,   72, 1404, 1404,  363, 1404, 1404, 1404, 1404, 1404,
 /* 10390 */  1404, 1404, 1404, 1404,  574, 1404, 1404, 1404, 1404,  893,
 /* 10400 */   572,   89, 1404, 1404, 1404,  896, 1404, 1404, 1404, 1404,
 /* 10410 */   899, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  900,  898,
 /* 10420 */   897,  895,  894,  153,  154,  151,  152,  150,  149,  148,
 /* 10430 */   176,  172,  168,  166,  164,  162,  170,  174,  160,  159,
 /* 10440 */   155,  158,  157,  156, 1404, 1404,  899, 1404, 1404, 1404,
 /* 10450 */  1404, 1404,  161,  429,  900,  898,  897,  895,  894, 1404,
 /* 10460 */  1404, 1404,  171, 1404,  165,  163, 1404, 1404, 1404, 1404,
 /* 10470 */   140, 1404, 1404, 1404, 1404, 1404,  583, 1404, 1404, 1404,
 /* 10480 */  1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,  173,  580,
 /* 10490 */   167,  141,  169,  419,  177,  711,   72, 1404,  171,  363,
 /* 10500 */   165,  163, 1404, 1404, 1404, 1404,  140, 1404, 1404,  574,
 /* 10510 */  1404, 1404,  583, 1404,  893,  572,   89, 1404, 1404,  175,
 /* 10520 */   896, 1404, 1404, 1404,  173,  580,  167,  141,  169,  419,
 /* 10530 */   177,  890,   72, 1404, 1404,  363, 1404, 1404, 1404, 1404,
 /* 10540 */  1404, 1404, 1404, 1404, 1404,  574, 1404, 1404, 1404, 1404,
 /* 10550 */   893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404, 1404,
 /* 10560 */  1404,  899, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  900,
 /* 10570 */   898,  897,  895,  894, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10580 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10590 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  899, 1404, 1404,
 /* 10600 */  1404, 1404, 1404, 1404, 1404,  900,  898,  897,  895,  894,
 /* 10610 */  1404, 1404, 1404,  171, 1404,  165,  163, 1404, 1404, 1404,
 /* 10620 */  1404,  140, 1404, 1404, 1404, 1404, 1404,  583, 1404, 1404,
 /* 10630 */  1404, 1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,  173,
 /* 10640 */   580,  167,  141,  169,  419,  177,  887,   72, 1404,  171,
 /* 10650 */   363,  165,  163, 1404, 1404, 1404, 1404,  140, 1404, 1404,
 /* 10660 */   574, 1404, 1404,  583, 1404,  893,  572,   89, 1404, 1404,
 /* 10670 */   175,  896, 1404, 1404, 1404,  173,  580,  167,  141,  169,
 /* 10680 */   419,  177,  722,   72, 1404, 1404,  363, 1404, 1404, 1404,
 /* 10690 */  1404, 1404, 1404, 1404, 1404, 1404,  574, 1404, 1404, 1404,
 /* 10700 */  1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404, 1404,
 /* 10710 */  1404, 1404,  899, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10720 */   900,  898,  897,  895,  894, 1404, 1404, 1404, 1404, 1404,
 /* 10730 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10740 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  899, 1404,
 /* 10750 */  1404, 1404, 1404, 1404, 1404, 1404,  900,  898,  897,  895,
 /* 10760 */   894, 1404, 1404, 1404,  171, 1404,  165,  163, 1404, 1404,
 /* 10770 */  1404, 1404,  140, 1404, 1404, 1404, 1404, 1404,  583, 1404,
 /* 10780 */  1404, 1404, 1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,
 /* 10790 */   173,  580,  167,  141,  169,  419,  177,  720,   72, 1404,
 /* 10800 */   171,  363,  165,  163, 1404, 1404, 1404, 1404,  140, 1404,
 /* 10810 */  1404,  574, 1404, 1404,  583, 1404,  893,  572,   89, 1404,
 /* 10820 */  1404,  175,  896, 1404, 1404, 1404,  173,  580,  167,  141,
 /* 10830 */   169,  419,  177,  718,   72, 1404, 1404,  363, 1404, 1404,
 /* 10840 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  574, 1404, 1404,
 /* 10850 */  1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404,
 /* 10860 */  1404, 1404, 1404,  899, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10870 */  1404,  900,  898,  897,  895,  894, 1404, 1404, 1404, 1404,
 /* 10880 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 10890 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  899,
 /* 10900 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  900,  898,  897,
 /* 10910 */   895,  894, 1404, 1404, 1404,  171, 1404,  165,  163, 1404,
 /* 10920 */  1404, 1404, 1404,  140, 1404, 1404, 1404, 1404, 1404,  583,
 /* 10930 */  1404, 1404, 1404, 1404, 1404, 1404,  175, 1404, 1404, 1404,
 /* 10940 */  1404,  173,  580,  167,  141,  169,  419,  177,  706,   72,
 /* 10950 */  1404,  171,  363,  165,  163, 1404, 1404, 1404, 1404,  140,
 /* 10960 */  1404, 1404,  574, 1404, 1404,  583, 1404,  893,  572,   89,
 /* 10970 */  1404, 1404,  175,  896, 1404, 1404, 1404,  173,  580,  167,
 /* 10980 */   141,  169,  419,  177,  704,   72, 1404, 1404,  363, 1404,
 /* 10990 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  574, 1404,
 /* 11000 */  1404, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896,
 /* 11010 */  1404, 1404, 1404, 1404,  899, 1404, 1404, 1404, 1404, 1404,
 /* 11020 */  1404, 1404,  900,  898,  897,  895,  894, 1404, 1404, 1404,
 /* 11030 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11040 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11050 */   899, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  900,  898,
 /* 11060 */   897,  895,  894, 1404, 1404, 1404, 1404,  146, 1404, 1404,
 /* 11070 */   147,  153,  154,  151,  152,  150,  149,  148,  176,  172,
 /* 11080 */   168,  166,  164,  162,  170,  174,  160,  159,  155,  158,
 /* 11090 */   157,  156, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  902,
 /* 11100 */   161,  429, 1404, 1404, 1404, 1404, 1404, 1404,  146, 1404,
 /* 11110 */  1404,  147,  153,  154,  151,  152,  150,  149,  148,  176,
 /* 11120 */   172,  168,  166,  164,  162,  170,  174,  160,  159,  155,
 /* 11130 */   158,  157,  156, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11140 */  1404,  161,  429, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11150 */  1404, 1404, 1404, 1404,   20, 1404, 1404, 1404, 1404, 1404,
 /* 11160 */  1404, 1404, 1404,  146, 1404, 1404,  147,  153,  154,  151,
 /* 11170 */   152,  150,  149,  148,  176,  172,  168,  166,  164,  162,
 /* 11180 */   170,  174,  160,  159,  155,  158,  157,  156, 1404, 1404,
 /* 11190 */  1404, 1404, 1404, 1404, 1404, 1404,  161,  429, 1404, 1404,
 /* 11200 */   777, 1404, 1404, 1404, 1404,  146, 1404, 1404,  147,  153,
 /* 11210 */   154,  151,  152,  150,  149,  148,  176,  172,  168,  166,
 /* 11220 */   164,  162,  170,  174,  160,  159,  155,  158,  157,  156,
 /* 11230 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  161,  429,
 /* 11240 */  1404, 1404,  775, 1404, 1404, 1404, 1404,  146, 1404, 1404,
 /* 11250 */   147,  153,  154,  151,  152,  150,  149,  148,  176,  172,
 /* 11260 */   168,  166,  164,  162,  170,  174,  160,  159,  155,  158,
 /* 11270 */   157,  156, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11280 */   161,  429, 1404, 1404,  774, 1404, 1404, 1404, 1404,  146,
 /* 11290 */  1404, 1404,  147,  153,  154,  151,  152,  150,  149,  148,
 /* 11300 */   176,  172,  168,  166,  164,  162,  170,  174,  160,  159,
 /* 11310 */   155,  158,  157,  156, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11320 */  1404, 1404,  161,  429, 1404, 1404,  773, 1404, 1404, 1404,
 /* 11330 */  1404,  146, 1404, 1404,  147,  153,  154,  151,  152,  150,
 /* 11340 */   149,  148,  176,  172,  168,  166,  164,  162,  170,  174,
 /* 11350 */   160,  159,  155,  158,  157,  156, 1404, 1404, 1404, 1404,
 /* 11360 */  1404, 1404, 1404, 1404,  161,  429, 1404, 1404, 1404, 1404,
 /* 11370 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,   35, 1404, 1404,
 /* 11380 */  1404, 1404, 1404, 1404, 1404, 1404,  146, 1404, 1404,  147,
 /* 11390 */   153,  154,  151,  152,  150,  149,  148,  176,  172,  168,
 /* 11400 */   166,  164,  162,  170,  174,  160,  159,  155,  158,  157,
 /* 11410 */   156, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  161,
 /* 11420 */   429, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11430 */  1404, 1404,   19, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11440 */  1404,  146, 1404, 1404,  147,  153,  154,  151,  152,  150,
 /* 11450 */   149,  148,  176,  172,  168,  166,  164,  162,  170,  174,
 /* 11460 */   160,  159,  155,  158,  157,  156, 1404, 1404,  171, 1404,
 /* 11470 */   165,  163, 1404, 1404,  161,  429,  134, 1404, 1404, 1404,
 /* 11480 */  1404, 1404,  583, 1404, 1404, 1404, 1404,   34, 1404,  175,
 /* 11490 */  1404, 1404, 1404, 1404,  173,  580,  167,  141,  169,  419,
 /* 11500 */   177, 1404,   72, 1404,  171,  424,  165,  163, 1404, 1404,
 /* 11510 */  1404, 1404, 1404, 1404, 1404,  574, 1404, 1404,  583, 1404,
 /* 11520 */   893,  572,   89, 1404, 1404,  175,  896, 1404, 1404, 1404,
 /* 11530 */   173,  580,  167,  141,  169,  419,  177, 1404,   72, 1404,
 /* 11540 */  1404,  424, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11550 */  1404,  574, 1404, 1404, 1404, 1404,  893,  572,   89, 1404,
 /* 11560 */  1404, 1404,  896, 1404, 1404, 1404, 1404,  899, 1404, 1404,
 /* 11570 */   171, 1404,  165,  163, 1404,  900,  898,  897,  895,  894,
 /* 11580 */  1404, 1404, 1404, 1404,  583, 1404, 1404, 1404, 1404, 1404,
 /* 11590 */  1404,  175, 1404, 1404, 1404,  136,  173,  580,  167,  141,
 /* 11600 */   169,  419,  177,  899,   72, 1404, 1404,  424,  776, 1404,
 /* 11610 */  1404,  900,  898,  897,  895,  894, 1404,  574, 1404, 1404,
 /* 11620 */  1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404,
 /* 11630 */  1404, 1404, 1404, 1404, 1404, 1404,  171, 1404,  165,  163,
 /* 11640 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 11650 */   583, 1404, 1404, 1404, 1404, 1404, 1404,  175, 1404, 1404,
 /* 11660 */  1404, 1404,  173,  580,  167,  141,  169,  419,  177,  899,
 /* 11670 */    72, 1404, 1404,  424, 1404, 1404, 1404,  900,  898,  897,
 /* 11680 */   895,  894, 1404,  574, 1404, 1404, 1404, 1404,  893,  572,
 /* 11690 */    89, 1404, 1404, 1404,  896, 1404, 1404, 1404, 1404, 1404,
 /* 11700 */  1404, 1404,  171, 1404,  165,  163, 1404, 1404, 1404, 1404,
 /* 11710 */   140, 1404, 1404, 1404, 1404, 1404,  583, 1404, 1404, 1404,
 /* 11720 */  1404, 1404, 1404,  175, 1404, 1404, 1404,  135,  173,  580,
 /* 11730 */   167,  141,  169,  419,  177,  899,   72, 1404,  171,  363,
 /* 11740 */   165,  163, 1404,  900,  898,  897,  895,  894, 1404,  574,
 /* 11750 */  1404, 1404,  583, 1404,  893,  572,   89, 1404, 1404,  175,
 /* 11760 */   896, 1404, 1404, 1404,  173,  580,  167,  141,  169,  419,
 /* 11770 */   177, 1404,   72, 1404, 1404,  424, 1404, 1404, 1404, 1404,
 /* 11780 */  1404, 1404, 1404, 1404, 1404,  574, 1404, 1404, 1404, 1404,
 /* 11790 */   893,  572,   89, 1404, 1404, 1404,  896, 1404,  282, 1404,
 /* 11800 */  1404,  899, 1404, 1404,  171, 1404,  165,  163, 1404,  900,
 /* 11810 */   898,  897,  895,  894, 1404, 1404, 1404, 1404,  583, 1404,
 /* 11820 */  1404, 1404, 1404, 1404, 1404,  175, 1404, 1404, 1404, 1404,
 /* 11830 */   173,  580,  167,  141,  169,  419,  177,  899,   72, 1404,
 /* 11840 */   171,  424,  165,  163, 1404,  900,  898,  897,  895,  894,
 /* 11850 */  1404,  574, 1404, 1404,  583, 1404,  893,  572,   89, 1404,
 /* 11860 */  1404,  175,  896, 1404,  281, 1404,  173,  580,  167,  141,
 /* 11870 */   169,  419,  177, 1404,   72, 1404, 1404,  424, 1404, 1404,
 /* 11880 */  1404, 1404, 1404, 1404, 1404, 1404, 1404,  574, 1404, 1404,
 /* 11890 */  1404, 1404,  893,  572,   89, 1404, 1404, 1404,  896, 1404,
 /* 11900 */   278, 1404, 1404,  899, 1404, 1404,  171, 1404,  165,  163,
 /* 11910 */  1404,  900,  898,  897,  895,  894, 1404, 1404, 1404, 1404,
 /* 11920 */   583, 1404, 1404, 1404, 1404, 1404, 1404,  175, 1404, 1404,
 /* 11930 */  1404, 1404,  173,  580,  167,  141,  169,  419,  177,  899,
 /* 11940 */    72, 1404,  171,  424,  165,  163, 1404,  900,  898,  897,
 /* 11950 */   895,  894, 1404,  574, 1404, 1404,  583, 1404,  893,  572,
 /* 11960 */    89, 1404, 1404,  175,  896, 1404,  277, 1404,  173,  580,
 /* 11970 */   167,  141,  169,  419,  177, 1404,   72, 1404, 1404,  424,
 /* 11980 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  574,
 /* 11990 */  1404, 1404, 1404, 1404,  893,  572,   89, 1404, 1404, 1404,
 /* 12000 */   896, 1404,  276, 1404, 1404,  899, 1404, 1404,  171, 1404,
 /* 12010 */   165,  163, 1404,  900,  898,  897,  895,  894, 1404, 1404,
 /* 12020 */  1404, 1404,  583, 1404, 1404, 1404, 1404, 1404, 1404,  175,
 /* 12030 */  1404, 1404, 1404, 1404,  173,  580,  167,  141,  169,  419,
 /* 12040 */   177,  899,   72, 1404,  171,  424,  165,  163, 1404,  900,
 /* 12050 */   898,  897,  895,  894, 1404,  574, 1404, 1404,  583, 1404,
 /* 12060 */   893,  572,   89, 1404, 1404,  175,  896, 1404,  275, 1404,
 /* 12070 */   173,  580,  167,  141,  169,  419,  177, 1404,   72, 1404,
 /* 12080 */  1404,  362, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 12090 */  1404,  574, 1404, 1404, 1404, 1404,  576,  572,   89, 1404,
 /* 12100 */  1404, 1404,  896, 1404,  892, 1404, 1404,  899, 1404, 1404,
 /* 12110 */  1404, 1404, 1404, 1404, 1404,  900,  898,  897,  895,  894,
 /* 12120 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 12130 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 12140 */  1404, 1404, 1404,  577, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 12150 */  1404,  578,  898,  897,  895,  894, 1404, 1404, 1404, 1404,
 /* 12160 */   146, 1404, 1404,  147,  153,  154,  151,  152,  150,  149,
 /* 12170 */   148,  176,  172,  168,  166,  164,  162,  170,  174,  160,
 /* 12180 */   159,  155,  158,  157,  156,  171, 1404,  165,  163, 1404,
 /* 12190 */  1404, 1404, 1404,  161,  429, 1404, 1404, 1404, 1404,  583,
 /* 12200 */  1404, 1404, 1404, 1404, 1404, 1404,  175, 1404, 1404, 1404,
 /* 12210 */  1404,  173,  580,  167,  141,  169,  419,  177, 1404,   72,
 /* 12220 */  1404,  171,  424,  165,  163, 1404, 1404, 1404, 1404, 1404,
 /* 12230 */  1404, 1404,  574, 1404, 1404,  583, 1404,  893,  572,   89,
 /* 12240 */  1404, 1404,  175,  896, 1404, 1404, 1404,  173,  580,  167,
 /* 12250 */   141,  169,  419,  177, 1404,   72, 1404, 1404,  362, 1404,
 /* 12260 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  574, 1404,
 /* 12270 */  1404, 1404, 1404,  576,  572,   89, 1404, 1404, 1404,  896,
 /* 12280 */  1404, 1404, 1404, 1404,  899, 1404, 1404, 1404, 1404, 1404,
 /* 12290 */  1404, 1404,  900,  898,  897,  895,  894, 1404, 1404, 1404,
 /* 12300 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 12310 */  1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404, 1404,
 /* 12320 */   577, 1404, 1404, 1404, 1404, 1404, 1404, 1404,  578,  898,
 /* 12330 */   897,  895,  894,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     7,    0,    9,   10,  155,   44,    6,  158,  159,  160,
 /*    10 */    42,  162,  163,   45,   21,   41,   42,  168,  169,   47,
 /*    20 */   164,   28,   54,  174,   44,   47,   33,   34,   35,   36,
 /*    30 */    37,   38,   39,   61,   41,   55,   56,   44,   45,    1,
 /*    40 */     2,    3,    4,    5,  163,   45,   44,   54,   55,   56,
 /*    50 */   171,  170,   59,   60,   61,  174,  177,  178,   65,  180,
 /*    60 */   164,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*    70 */    77,   78,  216,  217,  218,   82,   83,   84,   44,  107,
 /*    80 */    87,  154,   61,  156,   91,   92,   93,   94,   54,   96,
 /*    90 */    52,   53,   99,   55,   56,  196,   58,  198,  199,  106,
 /*   100 */    62,   63,   61,  110,  111,  112,  113,  114,  115,  116,
 /*   110 */   117,  118,  119,    7,  218,    9,   10,   27,   28,   29,
 /*   120 */    30,   31,   32,   44,   55,  193,    6,   21,  107,  197,
 /*   130 */    89,   41,   42,   54,   28,  203,  204,  205,  206,   33,
 /*   140 */    34,   35,   36,   37,   38,   39,   42,   41,  107,   45,
 /*   150 */    44,   45,    1,    2,    3,    4,    5,   88,   54,   90,
 /*   160 */    54,   55,   56,  171,   61,   59,   60,   61,   39,  177,
 /*   170 */   178,   65,  180,   44,   68,   69,   70,   71,   72,   73,
 /*   180 */    74,   75,   76,   77,   78,  163,    6,   67,   82,   83,
 /*   190 */    84,  169,   89,   87,   14,   45,  174,   91,   92,   93,
 /*   200 */    94,   44,   96,   52,   53,   99,   55,   56,   61,   58,
 /*   210 */   107,   54,  106,   62,   63,   40,  110,  111,  112,  113,
 /*   220 */   114,  115,  116,  117,  118,  119,    7,    6,    9,   10,
 /*   230 */    30,   31,   32,   22,   54,  159,  160,   55,  162,  163,
 /*   240 */    21,   41,   42,    6,  168,  169,  196,   28,  198,  199,
 /*   250 */   174,   44,   33,   34,   35,   36,   37,   38,   39,   42,
 /*   260 */    41,   40,   45,   44,   45,    1,    2,    3,    4,    5,
 /*   270 */    88,   54,   90,   54,   55,   56,  171,   40,   59,   60,
 /*   280 */    61,   39,  177,  178,   65,  180,   44,   68,   69,   70,
 /*   290 */    71,   72,   73,   74,   75,   76,   77,   78,    6,   44,
 /*   300 */   184,   82,   83,   84,   61,    6,   87,   61,   44,  193,
 /*   310 */    91,   92,   93,   94,   59,   96,   52,   53,   99,  203,
 /*   320 */   204,  205,  206,   44,   60,  106,   62,   63,   44,  110,
 /*   330 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*   340 */     6,    9,   10,  176,  177,  178,  179,  158,   44,  160,
 /*   350 */   107,  162,  163,   21,   55,  171,   89,  168,  169,   67,
 /*   360 */    28,  177,  178,  174,  180,   33,   34,   35,   36,   37,
 /*   370 */    38,   39,   42,   41,   40,   45,   44,   45,    1,    2,
 /*   380 */     3,    4,    5,   89,   54,  106,   54,   55,   56,  171,
 /*   390 */   106,   59,   60,   61,   44,  177,  178,   65,  180,    6,
 /*   400 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*   410 */    78,  171,    6,  184,   82,   83,   84,  177,  178,   87,
 /*   420 */   180,   44,  193,   91,   92,   93,   94,   45,   96,   52,
 /*   430 */    53,   99,  203,  204,  205,  206,   54,   60,  106,   62,
 /*   440 */    63,   44,  110,  111,  112,  113,  114,  115,  116,  117,
 /*   450 */   118,  119,    7,   56,    9,   10,  106,  177,  178,  157,
 /*   460 */   180,  159,  161,  161,  163,  163,   21,   44,    6,  168,
 /*   470 */   168,  170,  170,   28,   89,  174,  174,   54,   33,   34,
 /*   480 */    35,   36,   37,   38,   39,  196,   41,  198,  199,   44,
 /*   490 */    45,    1,    2,    3,    4,    5,  154,   44,  156,   54,
 /*   500 */    55,   56,  171,   97,   59,   60,   61,   45,  177,  178,
 /*   510 */    65,  180,   59,   68,   69,   70,   71,   72,   73,   74,
 /*   520 */    75,   76,   77,   78,  154,   44,  156,   82,   83,   84,
 /*   530 */     6,  160,   87,  162,  163,   54,   91,   92,   93,   94,
 /*   540 */   169,   96,   52,   53,   99,  174,   56,   42,   58,    6,
 /*   550 */    45,  106,   62,   63,   61,  110,  111,  112,  113,  114,
 /*   560 */   115,  116,  117,  118,  119,    7,   44,    9,   10,   45,
 /*   570 */     6,  173,  193,  175,  176,  177,  178,  179,   45,   21,
 /*   580 */   201,  202,  203,  204,  205,  206,   28,   54,   45,  108,
 /*   590 */   109,   33,   34,   35,   36,   37,   38,   39,   42,   41,
 /*   600 */     6,   45,   44,   45,    1,    2,    3,    4,    5,   45,
 /*   610 */    44,   49,   54,   55,   56,  171,   54,   59,   60,   61,
 /*   620 */    55,  177,  178,   65,  180,   59,   68,   69,   70,   71,
 /*   630 */    72,   73,   74,   75,   76,   77,   78,  164,   61,   45,
 /*   640 */    82,   83,   84,    6,    6,   87,    6,    6,    6,   91,
 /*   650 */    92,   93,   94,   45,   96,   52,   53,   99,   49,   56,
 /*   660 */    51,   58,   54,   54,  106,   62,   63,   57,  110,  111,
 /*   670 */   112,  113,  114,  115,  116,  117,  118,  119,    7,   22,
 /*   680 */     9,   10,   45,   45,  193,   45,   45,   45,  215,  216,
 /*   690 */   217,  218,   21,  107,  203,  204,  205,  206,   41,   28,
 /*   700 */    42,  210,  211,   45,   33,   34,   35,   36,   37,   38,
 /*   710 */    39,    6,   41,    6,   45,   44,   45,    1,    2,    3,
 /*   720 */     4,    5,   59,   54,   45,   54,   55,   56,  171,   45,
 /*   730 */    59,   60,   61,   54,  177,  178,   65,  180,   54,   68,
 /*   740 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   750 */    45,   89,   45,   82,   83,   84,   42,    6,   87,   45,
 /*   760 */    85,   86,   91,   92,   93,   94,   89,   96,   52,   53,
 /*   770 */    99,   49,   56,   51,   58,   89,   54,  106,   62,   63,
 /*   780 */    57,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*   790 */   119,    7,  160,    9,   10,  163,   45,  193,   85,   86,
 /*   800 */   168,  169,    6,    6,   45,   21,  174,  203,  204,  205,
 /*   810 */   206,   57,   28,   54,  210,  211,   57,   33,   34,   35,
 /*   820 */    36,   37,   38,   39,   51,   41,  126,   54,   44,   45,
 /*   830 */     1,    2,    3,    4,    5,   44,   40,   40,   54,   55,
 /*   840 */    56,  189,  190,   59,   60,   61,  191,  192,   44,   65,
 /*   850 */    57,  151,   68,   69,   70,   71,   72,   73,   74,   75,
 /*   860 */    76,   77,   78,   57,    6,  184,   82,   83,   84,   51,
 /*   870 */   196,   87,   54,  199,  193,   91,   92,   93,   94,   44,
 /*   880 */    96,   52,   53,   99,  203,  204,  205,  206,   22,   60,
 /*   890 */   106,   62,   63,   57,  110,  111,  112,  113,  114,  115,
 /*   900 */   116,  117,  118,  119,    7,   45,    9,   10,  207,  208,
 /*   910 */   193,   49,   54,   51,   54,   44,   54,   57,   21,   44,
 /*   920 */   203,  204,  205,  206,  154,   28,  156,  210,  211,    6,
 /*   930 */    33,   34,   35,   36,   37,   38,   39,   57,   41,  207,
 /*   940 */   208,   44,   45,    1,    2,    3,    4,    5,  207,  208,
 /*   950 */    57,   54,   55,   56,  207,  208,   59,   60,   61,   54,
 /*   960 */   207,  208,   65,   40,   45,   68,   69,   70,   71,   72,
 /*   970 */    73,   74,   75,   76,   77,   78,  207,  208,  184,   82,
 /*   980 */    83,   84,  207,  208,   87,  207,  208,  193,   91,   92,
 /*   990 */    93,   94,   45,   96,   52,   53,   99,  203,  204,  205,
 /*  1000 */   206,   89,   60,  106,   62,   63,   54,  110,  111,  112,
 /*  1010 */   113,  114,  115,  116,  117,  118,  119,    7,    6,    9,
 /*  1020 */    10,  207,  208,  193,  207,  208,  163,    6,  207,  208,
 /*  1030 */   154,   21,  156,  203,  204,  205,  206,  174,   28,   54,
 /*  1040 */   210,  211,    6,   33,   34,   35,   36,   37,   38,   39,
 /*  1050 */   196,   41,   40,  199,   44,   45,    1,    2,    3,    4,
 /*  1060 */     5,   40,  185,  186,   54,   55,   56,   85,   86,   59,
 /*  1070 */    60,   61,  185,  186,  196,   65,   40,  199,   68,   69,
 /*  1080 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   85,
 /*  1090 */    86,  184,   82,   83,   84,  187,  188,   87,  166,  167,
 /*  1100 */   193,   91,   92,   93,   94,   54,   96,   52,   53,   99,
 /*  1110 */   203,  204,  205,  206,   54,   60,  106,   62,   63,   54,
 /*  1120 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  1130 */     7,    6,    9,   10,  194,  195,  193,   54,   92,  163,
 /*  1140 */     6,  163,   45,   54,   21,   44,  203,  204,  205,  206,
 /*  1150 */   174,   28,  174,  210,  211,    6,   33,   34,   35,   36,
 /*  1160 */    37,   38,   39,   67,   41,   40,   55,   44,   45,    1,
 /*  1170 */     2,    3,    4,    5,   40,  177,  178,   54,   55,   56,
 /*  1180 */    44,   55,   59,   60,   61,   44,   44,   55,   65,   40,
 /*  1190 */    55,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  1200 */    77,   78,   45,   45,  184,   82,   83,   84,   45,   45,
 /*  1210 */    87,   57,   97,  193,   91,   92,   93,   94,   44,   96,
 /*  1220 */    52,   53,   99,  203,  204,  205,  206,   44,   60,  106,
 /*  1230 */    62,   63,   54,  110,  111,  112,  113,  114,  115,  116,
 /*  1240 */   117,  118,  119,    7,    6,    9,   10,   92,   45,  193,
 /*  1250 */    89,   54,   54,    6,   54,   61,   55,   21,   54,  203,
 /*  1260 */   204,  205,  206,   54,   28,   44,  210,  211,    6,   33,
 /*  1270 */    34,   35,   36,   37,   38,   39,   61,   41,   40,   55,
 /*  1280 */    44,   45,    1,    2,    3,    4,    5,   40,   44,   55,
 /*  1290 */    54,   55,   56,   61,   89,   59,   60,   61,   55,   45,
 /*  1300 */    61,   65,   40,    6,   68,   69,   70,   71,   72,   73,
 /*  1310 */    74,   75,   76,   77,   78,   55,   61,   44,   82,   83,
 /*  1320 */    84,   44,   44,   87,   55,   61,    6,   91,   92,   93,
 /*  1330 */    94,   44,   96,   52,   53,   99,   14,   40,   39,   58,
 /*  1340 */    44,   67,  106,   62,   63,   22,  110,  111,  112,  113,
 /*  1350 */   114,  115,  116,  117,  118,  119,    7,    6,    9,   10,
 /*  1360 */    40,   45,  193,   44,   61,   57,    6,   57,   57,   45,
 /*  1370 */    21,   45,  203,  204,  205,  206,   45,   28,   45,  210,
 /*  1380 */   211,   45,   33,   34,   35,   36,   37,   38,   39,   57,
 /*  1390 */    41,   40,   45,   44,   45,    1,    2,    3,    4,    5,
 /*  1400 */    40,   44,   44,   54,   55,   56,   44,   55,   59,   60,
 /*  1410 */    61,   61,   44,   44,   65,   61,   45,   68,   69,   70,
 /*  1420 */    71,   72,   73,   74,   75,   76,   77,   78,   45,   45,
 /*  1430 */    45,   82,   83,   84,   44,   44,   87,   50,   50,   61,
 /*  1440 */    91,   92,   93,   94,   44,   96,   52,   53,   99,   44,
 /*  1450 */    56,   44,   54,   54,   61,  106,   62,   63,   44,  110,
 /*  1460 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  1470 */    60,    9,   10,   54,   54,  193,   44,   61,   54,   54,
 /*  1480 */    44,   60,   54,   21,   61,  203,  204,  205,  206,   54,
 /*  1490 */    28,  126,  210,  211,   44,   33,   34,   35,   36,   37,
 /*  1500 */    38,   39,  190,   41,   95,   44,   44,   45,    1,    2,
 /*  1510 */     3,    4,    5,  208,   44,  152,   54,   55,   56,  192,
 /*  1520 */    54,   59,   60,   61,   44,  153,  156,   65,   54,  186,
 /*  1530 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  1540 */    78,  152,  165,  167,   82,   83,   84,  165,  195,   87,
 /*  1550 */   153,  165,  188,   91,   92,   93,   94,  153,   96,   52,
 /*  1560 */    53,   99,  196,   56,  153,  153,  153,  165,  106,   62,
 /*  1570 */    63,  153,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  1580 */   118,  119,    7,  153,    9,   10,  153,  153,  193,  174,
 /*  1590 */   153,  164,  196,  164,  177,  219,   21,  196,  203,  204,
 /*  1600 */   205,  206,  196,   28,  196,  210,  211,  219,   33,   34,
 /*  1610 */    35,   36,   37,   38,   39,  164,   41,  164,  196,   44,
 /*  1620 */    45,    1,    2,    3,    4,    5,  219,  196,  164,   54,
 /*  1630 */    55,   56,  164,  164,   59,   60,   61,  196,  196,  196,
 /*  1640 */    65,  164,  164,   68,   69,   70,   71,   72,   73,   74,
 /*  1650 */    75,   76,   77,   78,  164,  164,  164,   82,   83,   84,
 /*  1660 */   164,  164,   87,  164,  164,  219,   91,   92,   93,   94,
 /*  1670 */   219,   96,   52,   53,   99,  219,   56,  219,  219,  219,
 /*  1680 */   219,  106,   62,   63,  219,  110,  111,  112,  113,  114,
 /*  1690 */   115,  116,  117,  118,  119,    7,  219,    9,   10,  219,
 /*  1700 */   219,  193,  219,  219,  219,  219,  219,  219,  219,   21,
 /*  1710 */   219,  203,  204,  205,  206,  219,   28,  219,  210,  211,
 /*  1720 */   219,   33,   34,   35,   36,   37,   38,   39,  219,   41,
 /*  1730 */   219,  219,   44,   45,    1,    2,    3,    4,    5,  219,
 /*  1740 */   219,  219,   54,   55,   56,  219,  219,   59,   60,   61,
 /*  1750 */   219,  219,  219,   65,  219,  219,   68,   69,   70,   71,
 /*  1760 */    72,   73,   74,   75,   76,   77,   78,  219,  219,  219,
 /*  1770 */    82,   83,   84,  219,  219,   87,  219,  219,  193,   91,
 /*  1780 */    92,   93,   94,  219,   96,   52,   53,   99,  203,  204,
 /*  1790 */   205,  206,  219,  219,  106,   62,   63,  219,  110,  111,
 /*  1800 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  219,
 /*  1810 */     9,   10,  219,  219,  173,  219,  175,  176,  177,  178,
 /*  1820 */   179,  173,   21,  175,  176,  177,  178,  179,  219,   28,
 /*  1830 */   219,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /*  1840 */    39,  219,   41,  219,  219,   44,   45,  173,  219,  175,
 /*  1850 */   176,  177,  178,  179,  219,   54,   55,   56,  219,  219,
 /*  1860 */    59,   60,   61,  219,  219,  219,   65,  219,  219,   68,
 /*  1870 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  1880 */   219,  219,  219,   82,   83,   84,  219,  219,   87,  219,
 /*  1890 */   219,  219,   91,   92,   93,   94,  219,   96,  219,  173,
 /*  1900 */    99,  175,  176,  177,  178,  179,  219,  106,  219,  219,
 /*  1910 */   219,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  1920 */   119,    7,  219,    9,   10,  219,  219,  173,  219,  175,
 /*  1930 */   176,  177,  178,  179,  173,   21,  175,  176,  177,  178,
 /*  1940 */   179,  219,   28,  219,  219,  219,  219,   33,   34,   35,
 /*  1950 */    36,   37,   38,   39,  219,   41,  219,  219,   44,   45,
 /*  1960 */   173,  219,  175,  176,  177,  178,  179,  219,   54,   55,
 /*  1970 */    56,  219,  219,   59,   60,   61,  219,  219,  219,   65,
 /*  1980 */   219,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  1990 */    76,   77,   78,  219,  219,  219,   82,   83,   84,  219,
 /*  2000 */   219,   87,  219,  219,  193,   91,   92,   93,   94,  219,
 /*  2010 */    96,  219,  219,   99,  203,  204,  205,  206,  219,  219,
 /*  2020 */   106,  219,  211,  219,  110,  111,  112,  113,  114,  115,
 /*  2030 */   116,  117,  118,  119,    7,  219,    9,   10,  219,  193,
 /*  2040 */   219,  219,  219,  197,  219,  219,  193,  219,   21,  203,
 /*  2050 */   204,  205,  206,  200,  219,   28,  203,  204,  205,  206,
 /*  2060 */    33,   34,   35,   36,   37,   38,   39,  193,   41,  219,
 /*  2070 */   219,   44,   45,  219,  219,  219,  202,  203,  204,  205,
 /*  2080 */   206,   54,   55,   56,  219,  219,   59,   60,   61,  219,
 /*  2090 */   219,  219,   65,  219,  219,   68,   69,   70,   71,   72,
 /*  2100 */    73,   74,   75,   76,   77,   78,  219,  219,  219,   82,
 /*  2110 */    83,   84,  219,  219,   87,  219,  219,  193,   91,   92,
 /*  2120 */    93,   94,  219,   96,  219,  219,   99,  203,  204,  205,
 /*  2130 */   206,  219,  219,  106,  219,  219,  219,  110,  111,  112,
 /*  2140 */   113,  114,  115,  116,  117,  118,  119,    7,  219,    9,
 /*  2150 */    10,  219,  193,  219,  219,  219,  197,  219,  219,  193,
 /*  2160 */   219,   21,  203,  204,  205,  206,  219,  219,   28,  203,
 /*  2170 */   204,  205,  206,   33,   34,   35,   36,   37,   38,   39,
 /*  2180 */   214,   41,  219,  219,   44,   45,  219,  219,  219,  219,
 /*  2190 */   219,  219,  219,  219,   54,   55,   56,  219,  219,   59,
 /*  2200 */    60,   61,  219,  219,  219,   65,  219,  219,   68,   69,
 /*  2210 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  219,
 /*  2220 */   219,  219,   82,   83,   84,  219,  219,   87,  219,  219,
 /*  2230 */   193,   91,   92,   93,   94,  219,   96,  219,  219,   99,
 /*  2240 */   203,  204,  205,  206,  219,  219,  106,  219,  219,  219,
 /*  2250 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  2260 */     7,  219,    9,   10,  219,  193,  219,  219,  219,  197,
 /*  2270 */   219,  219,  193,  219,   21,  203,  204,  205,  206,  219,
 /*  2280 */   219,   28,  203,  204,  205,  206,   33,   34,   35,   36,
 /*  2290 */    37,   38,   39,  193,   41,  219,  219,   44,   45,  219,
 /*  2300 */   219,  219,  219,  203,  204,  205,  206,   54,   55,   56,
 /*  2310 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /*  2320 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  2330 */    77,   78,  219,  219,  219,   82,   83,   84,  219,  219,
 /*  2340 */    87,  219,  219,  193,   91,   92,   93,   94,  219,   96,
 /*  2350 */   219,  219,   99,  203,  204,  205,  206,  219,  219,  106,
 /*  2360 */   219,  219,  219,  110,  111,  112,  113,  114,  115,  116,
 /*  2370 */   117,  118,  119,    7,  219,    9,   10,  219,  193,  219,
 /*  2380 */   219,  219,  197,  219,  219,  219,  219,   21,  203,  204,
 /*  2390 */   205,  206,  219,  219,   28,  219,  219,  219,  219,   33,
 /*  2400 */    34,   35,   36,   37,   38,   39,  193,   41,  219,  219,
 /*  2410 */    44,   45,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  2420 */    54,   55,   56,  219,  219,   59,   60,   61,  219,  219,
 /*  2430 */   219,   65,  219,  219,   68,   69,   70,   71,   72,   73,
 /*  2440 */    74,   75,   76,   77,   78,  219,  219,  219,   82,   83,
 /*  2450 */    84,  219,  219,   87,  219,  219,  193,   91,   92,   93,
 /*  2460 */    94,  219,   96,  219,  219,   99,  203,  204,  205,  206,
 /*  2470 */   219,  219,  106,  219,  219,  219,  110,  111,  112,  113,
 /*  2480 */   114,  115,  116,  117,  118,  119,    7,  219,    9,   10,
 /*  2490 */   219,  193,  219,  219,  219,  197,  219,  219,  193,  219,
 /*  2500 */    21,  203,  204,  205,  206,  219,  219,   28,  203,  204,
 /*  2510 */   205,  206,   33,   34,   35,   36,   37,   38,   39,  193,
 /*  2520 */    41,  219,  219,   44,   45,  219,  219,  219,  219,  203,
 /*  2530 */   204,  205,  206,   54,   55,   56,  219,  219,   59,   60,
 /*  2540 */    61,  219,  219,  219,   65,  219,  219,   68,   69,   70,
 /*  2550 */    71,   72,   73,   74,   75,   76,   77,   78,  219,  219,
 /*  2560 */   219,   82,   83,   84,  219,  219,   87,  219,  219,  193,
 /*  2570 */    91,   92,   93,   94,  219,   96,  219,  219,   99,  203,
 /*  2580 */   204,  205,  206,  219,  219,  106,  219,  219,  219,  110,
 /*  2590 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  2600 */   219,    9,   10,  219,  193,  219,  219,  219,  197,  219,
 /*  2610 */   219,  193,  219,   21,  203,  204,  205,  206,  219,  219,
 /*  2620 */    28,  203,  204,  205,  206,   33,   34,   35,   36,   37,
 /*  2630 */    38,   39,  193,   41,  219,  219,   44,   45,  219,  219,
 /*  2640 */   219,  219,  203,  204,  205,  206,   54,   55,   56,  219,
 /*  2650 */   219,   59,   60,   61,  219,  219,  219,   65,  219,  219,
 /*  2660 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  2670 */    78,  219,  219,  219,   82,   83,   84,  219,  219,   87,
 /*  2680 */   219,  219,  193,   91,   92,   93,   94,  219,   96,  219,
 /*  2690 */   219,   99,  203,  204,  205,  206,  219,  219,  106,  219,
 /*  2700 */   219,  219,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  2710 */   118,  119,    7,  219,    9,   10,  219,  193,  219,  219,
 /*  2720 */   219,  197,  219,  219,  193,  219,   21,  203,  204,  205,
 /*  2730 */   206,  219,  219,   28,  203,  204,  205,  206,   33,   34,
 /*  2740 */    35,   36,   37,   38,   39,  193,   41,  219,  219,   44,
 /*  2750 */    45,  219,  219,  219,  219,  203,  204,  205,  206,   54,
 /*  2760 */    55,   56,  219,  219,   59,   60,   61,  219,  219,  219,
 /*  2770 */    65,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  2780 */    75,   76,   77,   78,  219,  219,  219,   82,   83,   84,
 /*  2790 */   219,  219,   87,  219,  219,  193,   91,   92,   93,   94,
 /*  2800 */   219,   96,  219,  219,   99,  203,  204,  205,  206,  219,
 /*  2810 */   219,  106,  219,  219,  219,  110,  111,  112,  113,  114,
 /*  2820 */   115,  116,  117,  118,  119,    7,  219,    9,   10,  219,
 /*  2830 */   193,  219,  219,  219,  197,  219,  219,  193,  219,   21,
 /*  2840 */   203,  204,  205,  206,  219,  219,   28,  203,  204,  205,
 /*  2850 */   206,   33,   34,   35,   36,   37,   38,   39,  193,   41,
 /*  2860 */   219,  219,   44,   45,  219,  219,  219,  219,  203,  204,
 /*  2870 */   205,  206,   54,   55,   56,  219,  219,   59,   60,   61,
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
 /*  3710 */   204,  205,  206,  219,  219,  106,  219,  219,  177,  110,
 /*  3720 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  3730 */   219,    9,   10,  219,  193,  219,  219,  219,  219,  219,
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
 /*  3850 */   219,  219,  219,  219,  193,  219,   21,  203,  204,  205,
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
 /*  3960 */   193,  219,  219,  219,  219,  219,  219,  219,  219,   21,
 /*  3970 */   203,  204,  205,  206,  219,  219,   28,  219,  219,  219,
 /*  3980 */   219,   33,   34,   35,   36,   37,   38,   39,  219,   41,
 /*  3990 */   219,  219,   44,   45,  219,  219,  219,  219,  219,  219,
 /*  4000 */   219,  219,   54,   55,   56,  219,  219,   59,   60,   61,
 /*  4010 */   219,  219,  219,   65,  219,  219,   68,   69,   70,   71,
 /*  4020 */    72,   73,   74,   75,   76,   77,   78,  219,  219,  219,
 /*  4030 */    82,   83,   84,  219,  219,   87,  219,  219,  219,   91,
 /*  4040 */    92,   93,   94,  219,   96,  219,  219,   99,  219,  219,
 /*  4050 */   219,  219,  219,  219,  106,  219,  219,  219,  110,  111,
 /*  4060 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  219,
 /*  4070 */     9,   10,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  4080 */   219,  219,   21,  219,  219,  219,  219,  219,  219,   28,
 /*  4090 */   219,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /*  4100 */    39,  219,   41,  219,  219,   44,   45,  219,  219,  219,
 /*  4110 */   219,  219,  219,  219,  219,   54,   55,   56,  219,  219,
 /*  4120 */    59,   60,   61,  219,  219,  219,   65,  219,  219,   68,
 /*  4130 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  4140 */   219,  219,  219,   82,   83,   84,  219,  219,   87,  219,
 /*  4150 */   219,  219,   91,   92,   93,   94,  219,   96,  219,  219,
 /*  4160 */    99,  219,  219,  219,  219,  219,  219,  106,  219,  219,
 /*  4170 */   219,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  4180 */   119,    7,  219,    9,   10,  219,  219,  219,  219,  219,
 /*  4190 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /*  4200 */   219,  219,   28,  219,  219,  219,  219,   33,   34,   35,
 /*  4210 */    36,   37,   38,   39,  219,   41,  219,  219,   44,   45,
 /*  4220 */   219,  219,  219,  219,  219,  219,  219,  219,   54,   55,
 /*  4230 */    56,  219,  219,   59,   60,   61,  219,  219,  219,   65,
 /*  4240 */   219,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  4250 */    76,   77,   78,  219,  219,  219,   82,   83,   84,  219,
 /*  4260 */   219,   87,  219,  219,  219,   91,   92,   93,   94,  219,
 /*  4270 */    96,  219,  219,   99,  219,  219,  219,  219,  219,  219,
 /*  4280 */   106,  219,  219,  219,  110,  111,  112,  113,  114,  115,
 /*  4290 */   116,  117,  118,  119,    7,  219,    9,   10,  219,  219,
 /*  4300 */   219,  219,  219,  219,  219,  219,  219,  219,   21,  219,
 /*  4310 */   219,  219,  219,  219,  219,   28,  219,  219,  219,  219,
 /*  4320 */    33,   34,   35,   36,   37,   38,   39,  219,   41,  219,
 /*  4330 */   219,   44,   45,  219,  219,  219,  219,  219,  219,  219,
 /*  4340 */   219,   54,   55,   56,  219,  219,   59,   60,   61,  219,
 /*  4350 */   219,  219,   65,  219,  219,   68,   69,   70,   71,   72,
 /*  4360 */    73,   74,   75,   76,   77,   78,  219,  219,  219,   82,
 /*  4370 */    83,   84,  219,  219,   87,  219,  219,  219,   91,   92,
 /*  4380 */    93,   94,  219,   96,  219,  219,   99,  219,  219,  219,
 /*  4390 */   219,  219,  219,  106,  219,  219,  219,  110,  111,  112,
 /*  4400 */   113,  114,  115,  116,  117,  118,  119,    7,  219,    9,
 /*  4410 */    10,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  4420 */   219,   21,  219,  219,  219,  219,  219,  219,   28,  219,
 /*  4430 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /*  4440 */   219,   41,  219,  219,   44,   45,  219,  219,  219,  219,
 /*  4450 */   219,  219,  219,  219,   54,   55,   56,  219,  219,   59,
 /*  4460 */    60,   61,  219,  219,  219,   65,  219,  219,   68,   69,
 /*  4470 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  219,
 /*  4480 */   219,  219,   82,   83,   84,  219,  219,   87,  219,  219,
 /*  4490 */   219,   91,   92,   93,   94,  219,   96,  219,  219,   99,
 /*  4500 */   219,  219,  219,  219,  219,  219,  106,  219,  219,  219,
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
 /*  6150 */    55,   56,  219,  219,   59,   60,   61,  219,  219,  219,
 /*  6160 */    65,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  6170 */    75,   76,   77,   78,  219,  219,  219,   82,   83,   84,
 /*  6180 */   219,  219,   87,  219,  219,  219,   91,   92,   93,   94,
 /*  6190 */   219,   96,  219,  219,   99,  219,  219,  219,  219,  219,
 /*  6200 */   219,  106,  219,  219,  219,  110,  111,  112,  113,  114,
 /*  6210 */   115,  116,  117,  118,  119,    7,  219,    9,   10,  219,
 /*  6220 */   219,  219,  219,  219,  219,  219,  219,  219,  219,   21,
 /*  6230 */   219,  219,  219,  219,  219,  219,   28,  219,  219,  219,
 /*  6240 */   219,   33,   34,   35,   36,   37,   38,   39,  219,   41,
 /*  6250 */   219,  219,   44,   45,  219,  219,  219,  219,  219,  219,
 /*  6260 */   219,  219,   54,   55,   56,  219,  219,   59,   60,   61,
 /*  6270 */   219,  219,  219,   65,  219,  219,   68,   69,   70,   71,
 /*  6280 */    72,   73,   74,   75,   76,   77,   78,  219,  219,  219,
 /*  6290 */    82,   83,   84,  219,  219,   87,  219,  219,  219,   91,
 /*  6300 */    92,   93,   94,  219,   96,  219,  219,   99,  219,  219,
 /*  6310 */   219,  219,  219,  219,  106,  219,  219,  219,  110,  111,
 /*  6320 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  219,
 /*  6330 */     9,   10,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  6340 */   219,  219,   21,  219,  219,  219,  219,  219,  219,   28,
 /*  6350 */   219,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /*  6360 */    39,  219,   41,  219,  219,   44,   45,  219,  219,  219,
 /*  6370 */   219,  219,  219,  219,  219,   54,   55,   56,  219,  219,
 /*  6380 */    59,   60,   61,  219,  219,  219,   65,  219,  219,   68,
 /*  6390 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  6400 */   219,  219,  219,   82,   83,   84,  219,  219,   87,  219,
 /*  6410 */   219,  219,   91,   92,   93,   94,  219,   96,  219,  219,
 /*  6420 */    99,  219,  219,  219,  219,  219,  219,  106,  219,  219,
 /*  6430 */   219,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  6440 */   119,    7,  219,    9,   10,  219,  219,  219,  219,  219,
 /*  6450 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /*  6460 */   219,  219,   28,  219,  219,  219,  219,   33,   34,   35,
 /*  6470 */    36,   37,   38,   39,  219,   41,  219,  219,   44,   45,
 /*  6480 */   219,  219,  219,  219,  219,  219,  219,  219,   54,   55,
 /*  6490 */    56,  219,  219,   59,   60,   61,  219,  219,  219,   65,
 /*  6500 */   219,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  6510 */    76,   77,   78,  219,  219,  219,   82,   83,   84,  219,
 /*  6520 */   219,   87,  219,  219,  219,   91,   92,   93,   94,  219,
 /*  6530 */    96,  219,  219,   99,  219,  219,  219,  219,  219,  219,
 /*  6540 */   106,  219,  219,  219,  110,  111,  112,  113,  114,  115,
 /*  6550 */   116,  117,  118,  119,    7,  219,    9,   10,  219,  219,
 /*  6560 */   219,  219,  219,  219,  219,  219,  219,  219,   21,  219,
 /*  6570 */   219,  219,  219,  219,  219,   28,  219,  219,  219,  219,
 /*  6580 */    33,   34,   35,   36,   37,   38,   39,  219,   41,  219,
 /*  6590 */   219,   44,   45,  219,  219,  219,  219,  219,  219,  219,
 /*  6600 */   219,   54,   55,   56,  219,  219,   59,   60,   61,  219,
 /*  6610 */   219,  219,   65,  219,  219,   68,   69,   70,   71,   72,
 /*  6620 */    73,   74,   75,   76,   77,   78,  219,  219,  219,   82,
 /*  6630 */    83,   84,  219,  219,   87,  219,  219,  219,   91,   92,
 /*  6640 */    93,   94,  219,   96,  219,  219,   99,  219,  219,  219,
 /*  6650 */   219,  219,  219,  106,  219,  219,  219,  110,  111,  112,
 /*  6660 */   113,  114,  115,  116,  117,  118,  119,    7,  219,    9,
 /*  6670 */    10,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  6680 */   219,   21,  219,  219,  219,  219,  219,  219,   28,  219,
 /*  6690 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /*  6700 */   219,   41,  219,  219,   44,   45,  219,  219,  219,  219,
 /*  6710 */   219,  219,  219,  219,   54,   55,   56,  219,  219,   59,
 /*  6720 */    60,   61,  219,  219,  219,   65,  219,  219,   68,   69,
 /*  6730 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  219,
 /*  6740 */   219,  219,   82,   83,   84,  219,  219,   87,  219,  219,
 /*  6750 */   219,   91,   92,   93,   94,  219,   96,  219,  219,   99,
 /*  6760 */   219,  219,  219,  219,  219,  219,  106,  219,  219,  219,
 /*  6770 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  6780 */     7,  219,    9,   10,  219,  219,  219,  219,  219,  219,
 /*  6790 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /*  6800 */   219,   28,  219,  219,  219,  219,   33,   34,   35,   36,
 /*  6810 */    37,   38,   39,  219,   41,  219,  219,   44,   45,  219,
 /*  6820 */   219,  219,  219,  219,  219,  219,  219,   54,  219,   56,
 /*  6830 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /*  6840 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  6850 */    77,   78,  219,  219,  219,   82,   83,   84,  219,  219,
 /*  6860 */    87,  219,  219,  219,   91,   92,   93,   94,    7,   96,
 /*  6870 */   219,  219,   99,  219,  219,  219,  219,  219,  219,  106,
 /*  6880 */   219,  219,  219,  110,  111,  112,  113,  114,  115,  116,
 /*  6890 */   117,  118,  119,  219,  219,   34,  219,  219,  219,  219,
 /*  6900 */   219,  219,  219,  219,   43,   44,  219,   46,  219,   48,
 /*  6910 */   219,   50,  219,   52,   53,   54,  219,   56,  219,  219,
 /*  6920 */   219,   60,  219,  219,  219,  219,  219,  219,  219,   68,
 /*  6930 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  6940 */    79,   80,   81,   82,   83,   84,  219,  219,   87,  219,
 /*  6950 */   219,  219,   91,   92,   93,   94,    7,   96,    9,   10,
 /*  6960 */    99,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  6970 */    21,  110,  111,  112,  113,  219,  219,   28,  219,  219,
 /*  6980 */   119,  219,   33,   34,   35,   36,   37,   38,   39,  219,
 /*  6990 */    41,  219,  219,   44,  219,  219,  219,  219,  219,  219,
 /*  7000 */   219,  219,  219,   54,  219,  219,  219,  219,   59,   60,
 /*  7010 */    61,  219,  219,  219,   65,  219,  219,   68,   69,   70,
 /*  7020 */    71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*  7030 */    81,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*  7040 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*  7050 */    30,   31,   32,  219,  219,  106,  219,  219,  219,  219,
 /*  7060 */   219,   41,   42,  114,  115,  116,  117,  118,  219,  219,
 /*  7070 */   219,  219,  219,  219,  121,  122,  123,  124,  125,  126,
 /*  7080 */   127,  128,  129,  130,  131,  132,  133,  134,  135,  136,
 /*  7090 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  7100 */   147,  148,  149,  150,    8,  219,  219,   11,   12,   13,
 /*  7110 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*  7120 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  219,
 /*  7130 */   177,  219,  219,  219,  219,  219,  219,   41,   42,   19,
 /*  7140 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*  7150 */    30,   31,   32,  219,  193,  219,  219,  204,  205,  206,
 /*  7160 */   219,   41,   42,   67,  203,  204,  205,  206,  219,  219,
 /*  7170 */   209,  219,  219,  212,  213,  214,  219,  219,  219,  219,
 /*  7180 */   124,  125,  126,  127,  128,  129,  130,  131,  132,  133,
 /*  7190 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  7200 */   144,  145,  146,  147,  148,  149,  150,  219,  219,  219,
 /*  7210 */   219,  219,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  7220 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  7230 */   148,  149,  150,  177,  219,  219,  193,  219,  219,  219,
 /*  7240 */   219,  219,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  7250 */   219,  219,  219,  219,  172,  212,  213,  214,  219,  219,
 /*  7260 */   204,  205,  206,  181,  182,  183,  219,  219,  219,  219,
 /*  7270 */   219,  219,  219,  219,  219,  193,  219,  219,  219,  219,
 /*  7280 */   219,  219,  219,  219,  219,  203,  204,  205,  206,  130,
 /*  7290 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  7300 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  7310 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*  7320 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /*  7330 */   219,  172,  219,  219,  219,  219,  219,  219,   41,   42,
 /*  7340 */   181,  182,  183,  219,  219,  219,  219,  219,  219,  219,
 /*  7350 */   219,  219,  193,  219,  219,  219,  219,  219,   60,  219,
 /*  7360 */   219,  219,  203,  204,  205,  206,   68,   69,   70,   71,
 /*  7370 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*  7380 */   219,  219,  219,  219,  219,  130,  131,  132,  133,  134,
 /*  7390 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  7400 */   145,  146,  147,  148,  149,  150,   14,   15,   16,   17,
 /*  7410 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*  7420 */    28,   29,   30,   31,   32,  219,  219,  172,   41,   42,
 /*  7430 */   219,  219,   41,   41,   42,   44,  181,  182,  183,   41,
 /*  7440 */   219,  219,  219,  219,   57,  219,  219,  219,  193,  219,
 /*  7450 */    59,  219,  219,  219,  219,   57,   65,  219,  203,  204,
 /*  7460 */   205,  206,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  7470 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  7480 */   148,  149,  150,  219,  219,  219,  219,  100,  101,  102,
 /*  7490 */   103,  104,  105,   21,  107,  108,  109,  106,  100,  101,
 /*  7500 */   102,  103,  104,  105,  172,  114,  115,  116,  117,  118,
 /*  7510 */   219,  219,   40,  181,  182,  183,   44,  219,  219,  219,
 /*  7520 */   219,  219,  219,  219,  219,  193,  219,  219,  219,  219,
 /*  7530 */    58,  219,  219,  219,   21,  203,  204,  205,  206,  219,
 /*  7540 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  7550 */    78,   79,   80,   81,  219,  219,  219,   44,  130,  131,
 /*  7560 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  7570 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  7580 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  7590 */    77,   78,   79,   80,   81,  219,  219,  219,  219,   41,
 /*  7600 */   172,  219,   44,  219,  219,  219,  219,  219,  219,  181,
 /*  7610 */   182,  183,  219,  219,  219,  219,  219,   59,  219,  219,
 /*  7620 */   219,  193,  219,   65,  219,   67,  219,  219,  219,  219,
 /*  7630 */   219,  203,  204,  205,  206,  130,  131,  132,  133,  134,
 /*  7640 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  7650 */   145,  146,  147,  148,  149,  150,  219,  219,  219,  219,
 /*  7660 */   219,  219,  219,  219,  106,  219,   21,  219,  219,  219,
 /*  7670 */   219,  219,  114,  115,  116,  117,  118,  172,  219,  219,
 /*  7680 */   219,  219,  219,  219,  219,   40,  181,  182,  183,   44,
 /*  7690 */   219,  219,  219,  219,  219,  219,   41,  219,  193,  219,
 /*  7700 */   219,  219,  219,   58,  219,  219,  219,  219,  203,  204,
 /*  7710 */   205,  206,   57,   68,   69,   70,   71,   72,   73,   74,
 /*  7720 */    75,   76,   77,   78,   79,   80,   81,  219,  219,  219,
 /*  7730 */   219,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  7740 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  7750 */   149,  150,  219,  219,   57,  100,  101,  102,  103,  104,
 /*  7760 */   105,  219,  219,  108,  109,  219,  219,   41,  219,  219,
 /*  7770 */    44,  219,  219,  172,   41,  219,  219,  219,  219,  219,
 /*  7780 */   219,  219,  181,  182,  183,   59,  219,  219,  219,  219,
 /*  7790 */    57,   65,  219,  219,  193,  219,  219,  100,  101,  102,
 /*  7800 */   103,  104,  105,  219,  203,  204,  205,  206,  130,  131,
 /*  7810 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  7820 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  7830 */   219,  219,  106,  100,  101,  102,  103,  104,  105,   21,
 /*  7840 */   114,  115,  116,  117,  118,  219,  219,  219,  219,  219,
 /*  7850 */   172,  219,  219,  219,  219,  219,  219,  219,   40,  181,
 /*  7860 */   182,  183,   44,  219,  219,  219,  219,  219,  219,   41,
 /*  7870 */   219,  193,  219,  219,  219,  219,   58,  219,  219,  219,
 /*  7880 */   219,  203,  204,  205,  206,   57,   68,   69,   70,   71,
 /*  7890 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*  7900 */   219,  219,  219,  219,  130,  131,  132,  133,  134,  135,
 /*  7910 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  7920 */   146,  147,  148,  149,  150,  219,  219,  219,  100,  101,
 /*  7930 */   102,  103,  104,  105,  219,  219,  219,  219,  219,  219,
 /*  7940 */   219,  219,  219,  219,  219,  219,  172,   41,  219,  219,
 /*  7950 */   219,  219,  219,  219,  219,  181,  182,  183,  219,  219,
 /*  7960 */   219,  219,  219,   57,  219,  219,  219,  193,  219,  219,
 /*  7970 */   219,  219,  219,  219,  219,  219,  219,  203,  204,  205,
 /*  7980 */   206,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  7990 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  8000 */   149,  150,  219,  219,  219,  219,  100,  101,  102,  103,
 /*  8010 */   104,  105,   21,  219,  219,  219,  219,  219,  219,  219,
 /*  8020 */   219,  219,  219,  172,  219,  219,  219,  219,  219,  219,
 /*  8030 */   219,   40,  181,  182,  183,   44,  219,  219,  219,  219,
 /*  8040 */   219,  219,  219,  219,  193,  219,  219,  219,  219,   58,
 /*  8050 */   219,  219,  219,  219,  203,  204,  205,  206,  219,   68,
 /*  8060 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  8070 */    79,   80,   81,  219,  219,  219,  219,  130,  131,  132,
 /*  8080 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  8090 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /*  8100 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8110 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  172,
 /*  8120 */   219,  219,  219,  219,  219,  219,  219,  219,  181,  182,
 /*  8130 */   183,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8140 */   193,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8150 */   203,  204,  205,  206,  130,  131,  132,  133,  134,  135,
 /*  8160 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  8170 */   146,  147,  148,  149,  150,  219,  219,  219,  219,  219,
 /*  8180 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /*  8190 */   219,  219,  219,  219,  219,  219,  172,  219,  219,  219,
 /*  8200 */   219,  219,  219,  219,   40,  181,  182,  183,   44,  219,
 /*  8210 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  8220 */   219,  219,   58,  219,  219,  219,  219,  203,  204,  205,
 /*  8230 */   206,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  8240 */    76,   77,   78,   79,   80,   81,  219,  219,  219,  219,
 /*  8250 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  8260 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  8270 */   150,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8280 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8290 */   219,  219,  172,  219,  219,  219,  219,  219,  219,  219,
 /*  8300 */   219,  181,  182,  183,  219,  219,  219,  219,  219,  219,
 /*  8310 */   219,  219,  219,  193,  219,  219,  219,  219,  219,  219,
 /*  8320 */   219,  219,  219,  203,  204,  205,  206,  130,  131,  132,
 /*  8330 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  8340 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /*  8350 */   219,  219,  219,  219,  219,  219,  219,  219,   21,  219,
 /*  8360 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  172,
 /*  8370 */   219,  219,  219,  219,  219,  219,  219,   40,  181,  182,
 /*  8380 */   183,   44,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8390 */   193,  219,  219,  219,  219,   58,  219,  219,  219,  219,
 /*  8400 */   203,  204,  205,  206,  219,   68,   69,   70,   71,   72,
 /*  8410 */    73,   74,   75,   76,   77,   78,   79,   80,   81,  219,
 /*  8420 */   219,  219,  219,  130,  131,  132,  133,  134,  135,  136,
 /*  8430 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  8440 */   147,  148,  149,  150,  219,  219,  219,  219,  219,  219,
 /*  8450 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8460 */   219,  219,  219,  219,  219,  172,  219,  219,  219,  219,
 /*  8470 */   219,  219,  219,  219,  181,  182,  183,  219,  219,  219,
 /*  8480 */   219,  219,  219,  219,  219,  219,  193,  219,  219,  219,
 /*  8490 */   219,  219,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  8500 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  8510 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  8520 */   150,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8530 */   219,   21,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8540 */   219,  219,  172,  219,  219,  219,  219,  219,  219,  219,
 /*  8550 */    40,  181,  182,  183,   44,  219,  219,  219,  219,  219,
 /*  8560 */   219,  219,  219,  193,  219,  219,  219,  219,   58,  219,
 /*  8570 */   219,  219,  219,  203,  204,  205,  206,  219,   68,   69,
 /*  8580 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*  8590 */    80,   81,  219,  219,  219,  219,  130,  131,  132,  133,
 /*  8600 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  8610 */   144,  145,  146,  147,  148,  149,  150,  219,  219,  219,
 /*  8620 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8630 */   219,  219,  219,  219,  219,  219,  219,  219,  172,  219,
 /*  8640 */   219,  219,  219,  219,  219,  219,  219,  181,  182,  183,
 /*  8650 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  193,
 /*  8660 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  203,
 /*  8670 */   204,  205,  206,  130,  131,  132,  133,  134,  135,  136,
 /*  8680 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  8690 */   147,  148,  149,  150,  219,  219,  219,  219,  219,  219,
 /*  8700 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /*  8710 */   219,  219,  219,  219,  219,  172,  219,  219,  219,  219,
 /*  8720 */   219,  219,  219,   40,  181,  182,  183,   44,  219,  219,
 /*  8730 */   219,  219,  219,  219,  219,  219,  193,  219,  219,  219,
 /*  8740 */   219,   58,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  8750 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  8760 */    77,   78,   79,   80,   81,  219,  219,  219,  219,  130,
 /*  8770 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  8780 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  8790 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8800 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8810 */   219,  172,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8820 */   181,  182,  183,  219,  219,  219,  219,  219,  219,  219,
 /*  8830 */   219,  219,  193,  219,  219,  219,  219,  219,  219,  219,
 /*  8840 */   219,  219,  203,  204,  205,  206,  130,  131,  132,  133,
 /*  8850 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  8860 */   144,  145,  146,  147,  148,  149,  150,  219,  219,  219,
 /*  8870 */   219,  219,  219,  219,  219,  219,  219,   21,  219,  219,
 /*  8880 */   219,  219,  219,  219,  219,  219,  219,  219,  172,  219,
 /*  8890 */   219,  219,  219,  219,  219,  219,   40,  181,  182,  183,
 /*  8900 */    44,  219,  219,  219,  219,  219,  219,  219,  219,  193,
 /*  8910 */   219,  219,  219,  219,   58,  219,  219,  219,  219,  203,
 /*  8920 */   204,  205,  206,  219,   68,   69,   70,   71,   72,   73,
 /*  8930 */    74,   75,   76,   77,   78,   79,   80,   81,  219,  219,
 /*  8940 */   219,  219,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  8950 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  8960 */   148,  149,  150,  219,  219,  219,  219,  219,  219,  219,
 /*  8970 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8980 */   219,  219,  219,  219,  172,  219,  219,  219,  219,  219,
 /*  8990 */   219,  219,  219,  181,  182,  183,  219,  219,  219,  219,
 /*  9000 */   219,  219,  219,  219,  219,  193,  219,  219,  219,  219,
 /*  9010 */   219,  219,  219,  219,  219,  203,  204,  205,  206,  130,
 /*  9020 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  9030 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  9040 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9050 */   219,  219,  219,   21,  219,  219,  219,  219,  219,  219,
 /*  9060 */   219,  172,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9070 */   181,  182,  183,  219,  219,  219,  219,  219,  219,  219,
 /*  9080 */   219,  219,  193,  219,  219,  219,  219,  219,  219,  219,
 /*  9090 */   219,  219,  203,  204,  205,  206,   64,   65,   66,  219,
 /*  9100 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  9110 */    78,   79,   80,   81,  219,  130,  131,  132,  133,  134,
 /*  9120 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  9130 */   145,  146,  147,  148,  149,  150,  219,  219,  219,  219,
 /*  9140 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9150 */   219,  219,  219,  219,  219,  219,  219,  172,  219,  219,
 /*  9160 */   219,  219,  219,  219,  219,  219,  181,  182,  183,  219,
 /*  9170 */   219,  219,  219,  219,  219,  219,  219,  219,  193,  219,
 /*  9180 */   219,  219,  219,  219,  219,  219,  219,  219,  203,  204,
 /*  9190 */   205,  206,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  9200 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  9210 */   148,  149,  150,  219,  219,  219,  219,  219,  219,  219,
 /*  9220 */   219,  219,  219,   21,  219,  219,  219,  219,  219,  219,
 /*  9230 */   219,  219,  219,  219,  172,  219,  219,  219,  219,  219,
 /*  9240 */   219,  219,  219,  181,  182,  183,   44,  219,  219,  219,
 /*  9250 */   219,  219,  219,  219,  219,  193,  219,  219,  219,  219,
 /*  9260 */    58,  219,  219,  219,  219,  203,  204,  205,  206,  219,
 /*  9270 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  9280 */    78,   79,   80,   81,  219,  219,  219,  219,  130,  131,
 /*  9290 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  9300 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  9310 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9320 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9330 */   172,  219,  219,  219,  219,  219,  219,  219,  219,  181,
 /*  9340 */   182,  183,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9350 */   219,  193,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9360 */   219,  203,  204,  205,  206,  130,  131,  132,  133,  134,
 /*  9370 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  9380 */   145,  146,  147,  148,  149,  150,  219,  219,  219,  219,
 /*  9390 */   219,  219,  219,  219,  219,  219,  219,  219,   21,  219,
 /*  9400 */   219,  219,  219,  219,  219,  219,  219,  172,  219,  219,
 /*  9410 */   219,  219,  219,  219,  219,  219,  181,  182,  183,  219,
 /*  9420 */   219,  219,  219,  219,  219,  219,  219,  219,  193,  219,
 /*  9430 */   219,  219,  219,  219,  219,  219,  219,  219,  203,  204,
 /*  9440 */   205,  206,   65,   66,  219,   68,   69,   70,   71,   72,
 /*  9450 */    73,   74,   75,   76,   77,   78,   79,   80,   81,  219,
 /*  9460 */   219,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  9470 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  9480 */   149,  150,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9490 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9500 */   219,  219,  219,  172,  219,  219,  219,  219,  219,  219,
 /*  9510 */   219,  219,  181,  182,  183,  219,  219,  219,  219,  219,
 /*  9520 */   219,  219,  219,  219,  193,  219,  219,  219,  219,  219,
 /*  9530 */   219,  219,  219,  219,  203,  204,  205,  206,  130,  131,
 /*  9540 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  9550 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  9560 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9570 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9580 */   172,  219,  219,  219,  219,  219,  219,  219,  219,  181,
 /*  9590 */   182,  183,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9600 */   219,  193,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9610 */   219,  203,  204,  205,  206,  219,  219,  219,  219,  219,
 /*  9620 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9630 */   219,  219,  219,  219,  130,  131,  132,  133,  134,  135,
 /*  9640 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  9650 */   146,  147,  148,  149,  150,  219,  219,  219,  219,  219,
 /*  9660 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9670 */   219,  219,  219,  219,  219,  219,  172,  219,  219,  219,
 /*  9680 */   219,  219,  219,  219,  219,  181,  182,  183,  219,  219,
 /*  9690 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  9700 */   219,  219,  219,  219,  219,  219,  219,  203,  204,  205,
 /*  9710 */   206,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  9720 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  9730 */   149,  150,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9740 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9750 */   219,  219,  219,  172,  219,  219,  219,  219,  219,  219,
 /*  9760 */   219,  219,  181,  182,  183,  219,  219,  219,  219,  219,
 /*  9770 */   219,  219,  219,  219,  193,  219,  219,  219,  219,  219,
 /*  9780 */   219,  219,  219,  219,  203,  204,  205,  206,  219,  219,
 /*  9790 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9800 */   219,  219,  219,  219,  219,  219,  219,  130,  131,  132,
 /*  9810 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  9820 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /*  9830 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9840 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  172,
 /*  9850 */   219,  219,  219,  219,  219,  219,  219,  219,  181,  182,
 /*  9860 */   183,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9870 */   193,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9880 */   203,  204,  205,  206,  130,  131,  132,  133,  134,  135,
 /*  9890 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  9900 */   146,  147,  148,  149,  150,  219,  219,  219,  219,  219,
 /*  9910 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9920 */   219,  219,  219,  219,  219,  219,  172,  219,  219,  219,
 /*  9930 */   219,  219,  219,  219,  219,  181,  182,  183,  219,  219,
 /*  9940 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  9950 */   219,  219,  219,  219,  219,  219,  219,  203,  204,  205,
 /*  9960 */   206,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9970 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9980 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  9990 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /* 10000 */   150,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10010 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10020 */   219,  219,  172,  219,  219,  219,  219,  219,  219,  219,
 /* 10030 */   219,  181,  182,  183,  219,  219,  219,  219,  219,  219,
 /* 10040 */   219,  219,  219,  193,  219,  219,  219,  219,  219,  219,
 /* 10050 */   219,  219,  219,  203,  204,  205,  206,  130,  131,  132,
 /* 10060 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /* 10070 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /* 10080 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10090 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  172,
 /* 10100 */   219,  219,  219,  219,  219,  219,  219,  219,  181,  182,
 /* 10110 */   183,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10120 */   193,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10130 */   203,  204,  205,  206,  219,  219,  219,  219,  219,  219,
 /* 10140 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10150 */   219,  219,  219,  130,  131,  132,  133,  134,  135,  136,
 /* 10160 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /* 10170 */   147,  148,  149,  150,  219,  219,  219,  219,  219,  219,
 /* 10180 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10190 */   219,  219,  219,  219,  219,  172,  219,  219,  219,  219,
 /* 10200 */   219,  219,  219,  219,  181,  182,  183,  219,  219,  219,
 /* 10210 */   219,  219,  219,  219,  219,  219,  193,  219,  219,  219,
 /* 10220 */   219,  219,  219,  219,  219,  219,  203,  204,  205,  206,
 /* 10230 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /* 10240 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /* 10250 */   150,  219,    8,  219,  219,   11,   12,   13,   14,   15,
 /* 10260 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /* 10270 */    26,   27,   28,   29,   30,   31,   32,  219,  219,  219,
 /* 10280 */   219,  181,  182,  183,  219,   41,   42,  219,  219,  219,
 /* 10290 */   219,  219,  219,  193,  219,  219,  219,  219,  219,  219,
 /* 10300 */   219,  219,  219,  203,  204,  205,  206,  219,  219,  219,
 /* 10310 */   219,    7,  219,    9,   10,  219,  219,  219,  219,   15,
 /* 10320 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /* 10330 */   219,  219,   28,   89,  219,  219,  219,   33,   34,   35,
 /* 10340 */    36,   37,   38,   39,   40,   41,  219,    7,   44,    9,
 /* 10350 */    10,  219,  219,  219,  219,   15,  219,  219,   54,  219,
 /* 10360 */   219,   21,  219,   59,   60,   61,  219,  219,   28,   65,
 /* 10370 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /* 10380 */    40,   41,  219,  219,   44,  219,  219,  219,  219,  219,
 /* 10390 */   219,  219,  219,  219,   54,  219,  219,  219,  219,   59,
 /* 10400 */    60,   61,  219,  219,  219,   65,  219,  219,  219,  219,
 /* 10410 */   106,  219,  219,  219,  219,  219,  219,  219,  114,  115,
 /* 10420 */   116,  117,  118,   12,   13,   14,   15,   16,   17,   18,
 /* 10430 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /* 10440 */    29,   30,   31,   32,  219,  219,  106,  219,  219,  219,
 /* 10450 */   219,  219,   41,   42,  114,  115,  116,  117,  118,  219,
 /* 10460 */   219,  219,    7,  219,    9,   10,  219,  219,  219,  219,
 /* 10470 */    15,  219,  219,  219,  219,  219,   21,  219,  219,  219,
 /* 10480 */   219,  219,  219,   28,  219,  219,  219,  219,   33,   34,
 /* 10490 */    35,   36,   37,   38,   39,   40,   41,  219,    7,   44,
 /* 10500 */     9,   10,  219,  219,  219,  219,   15,  219,  219,   54,
 /* 10510 */   219,  219,   21,  219,   59,   60,   61,  219,  219,   28,
 /* 10520 */    65,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /* 10530 */    39,   40,   41,  219,  219,   44,  219,  219,  219,  219,
 /* 10540 */   219,  219,  219,  219,  219,   54,  219,  219,  219,  219,
 /* 10550 */    59,   60,   61,  219,  219,  219,   65,  219,  219,  219,
 /* 10560 */   219,  106,  219,  219,  219,  219,  219,  219,  219,  114,
 /* 10570 */   115,  116,  117,  118,  219,  219,  219,  219,  219,  219,
 /* 10580 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10590 */   219,  219,  219,  219,  219,  219,  219,  106,  219,  219,
 /* 10600 */   219,  219,  219,  219,  219,  114,  115,  116,  117,  118,
 /* 10610 */   219,  219,  219,    7,  219,    9,   10,  219,  219,  219,
 /* 10620 */   219,   15,  219,  219,  219,  219,  219,   21,  219,  219,
 /* 10630 */   219,  219,  219,  219,   28,  219,  219,  219,  219,   33,
 /* 10640 */    34,   35,   36,   37,   38,   39,   40,   41,  219,    7,
 /* 10650 */    44,    9,   10,  219,  219,  219,  219,   15,  219,  219,
 /* 10660 */    54,  219,  219,   21,  219,   59,   60,   61,  219,  219,
 /* 10670 */    28,   65,  219,  219,  219,   33,   34,   35,   36,   37,
 /* 10680 */    38,   39,   40,   41,  219,  219,   44,  219,  219,  219,
 /* 10690 */   219,  219,  219,  219,  219,  219,   54,  219,  219,  219,
 /* 10700 */   219,   59,   60,   61,  219,  219,  219,   65,  219,  219,
 /* 10710 */   219,  219,  106,  219,  219,  219,  219,  219,  219,  219,
 /* 10720 */   114,  115,  116,  117,  118,  219,  219,  219,  219,  219,
 /* 10730 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10740 */   219,  219,  219,  219,  219,  219,  219,  219,  106,  219,
 /* 10750 */   219,  219,  219,  219,  219,  219,  114,  115,  116,  117,
 /* 10760 */   118,  219,  219,  219,    7,  219,    9,   10,  219,  219,
 /* 10770 */   219,  219,   15,  219,  219,  219,  219,  219,   21,  219,
 /* 10780 */   219,  219,  219,  219,  219,   28,  219,  219,  219,  219,
 /* 10790 */    33,   34,   35,   36,   37,   38,   39,   40,   41,  219,
 /* 10800 */     7,   44,    9,   10,  219,  219,  219,  219,   15,  219,
 /* 10810 */   219,   54,  219,  219,   21,  219,   59,   60,   61,  219,
 /* 10820 */   219,   28,   65,  219,  219,  219,   33,   34,   35,   36,
 /* 10830 */    37,   38,   39,   40,   41,  219,  219,   44,  219,  219,
 /* 10840 */   219,  219,  219,  219,  219,  219,  219,   54,  219,  219,
 /* 10850 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /* 10860 */   219,  219,  219,  106,  219,  219,  219,  219,  219,  219,
 /* 10870 */   219,  114,  115,  116,  117,  118,  219,  219,  219,  219,
 /* 10880 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10890 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  106,
 /* 10900 */   219,  219,  219,  219,  219,  219,  219,  114,  115,  116,
 /* 10910 */   117,  118,  219,  219,  219,    7,  219,    9,   10,  219,
 /* 10920 */   219,  219,  219,   15,  219,  219,  219,  219,  219,   21,
 /* 10930 */   219,  219,  219,  219,  219,  219,   28,  219,  219,  219,
 /* 10940 */   219,   33,   34,   35,   36,   37,   38,   39,   40,   41,
 /* 10950 */   219,    7,   44,    9,   10,  219,  219,  219,  219,   15,
 /* 10960 */   219,  219,   54,  219,  219,   21,  219,   59,   60,   61,
 /* 10970 */   219,  219,   28,   65,  219,  219,  219,   33,   34,   35,
 /* 10980 */    36,   37,   38,   39,   40,   41,  219,  219,   44,  219,
 /* 10990 */   219,  219,  219,  219,  219,  219,  219,  219,   54,  219,
 /* 11000 */   219,  219,  219,   59,   60,   61,  219,  219,  219,   65,
 /* 11010 */   219,  219,  219,  219,  106,  219,  219,  219,  219,  219,
 /* 11020 */   219,  219,  114,  115,  116,  117,  118,  219,  219,  219,
 /* 11030 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11040 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11050 */   106,  219,  219,  219,  219,  219,  219,  219,  114,  115,
 /* 11060 */   116,  117,  118,  219,  219,  219,  219,    8,  219,  219,
 /* 11070 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /* 11080 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /* 11090 */    31,   32,  219,  219,  219,  219,  219,  219,  219,   40,
 /* 11100 */    41,   42,  219,  219,  219,  219,  219,  219,    8,  219,
 /* 11110 */   219,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /* 11120 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /* 11130 */    30,   31,   32,  219,  219,  219,  219,  219,  219,  219,
 /* 11140 */   219,   41,   42,  219,  219,  219,  219,  219,  219,  219,
 /* 11150 */   219,  219,  219,  219,   54,  219,  219,  219,  219,  219,
 /* 11160 */   219,  219,  219,    8,  219,  219,   11,   12,   13,   14,
 /* 11170 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /* 11180 */    25,   26,   27,   28,   29,   30,   31,   32,  219,  219,
 /* 11190 */   219,  219,  219,  219,  219,  219,   41,   42,  219,  219,
 /* 11200 */    45,  219,  219,  219,  219,    8,  219,  219,   11,   12,
 /* 11210 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /* 11220 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /* 11230 */   219,  219,  219,  219,  219,  219,  219,  219,   41,   42,
 /* 11240 */   219,  219,   45,  219,  219,  219,  219,    8,  219,  219,
 /* 11250 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /* 11260 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /* 11270 */    31,   32,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11280 */    41,   42,  219,  219,   45,  219,  219,  219,  219,    8,
 /* 11290 */   219,  219,   11,   12,   13,   14,   15,   16,   17,   18,
 /* 11300 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /* 11310 */    29,   30,   31,   32,  219,  219,  219,  219,  219,  219,
 /* 11320 */   219,  219,   41,   42,  219,  219,   45,  219,  219,  219,
 /* 11330 */   219,    8,  219,  219,   11,   12,   13,   14,   15,   16,
 /* 11340 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /* 11350 */    27,   28,   29,   30,   31,   32,  219,  219,  219,  219,
 /* 11360 */   219,  219,  219,  219,   41,   42,  219,  219,  219,  219,
 /* 11370 */   219,  219,  219,  219,  219,  219,  219,   54,  219,  219,
 /* 11380 */   219,  219,  219,  219,  219,  219,    8,  219,  219,   11,
 /* 11390 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /* 11400 */    22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
 /* 11410 */    32,  219,  219,  219,  219,  219,  219,  219,  219,   41,
 /* 11420 */    42,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11430 */   219,  219,   54,  219,  219,  219,  219,  219,  219,  219,
 /* 11440 */   219,    8,  219,  219,   11,   12,   13,   14,   15,   16,
 /* 11450 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /* 11460 */    27,   28,   29,   30,   31,   32,  219,  219,    7,  219,
 /* 11470 */     9,   10,  219,  219,   41,   42,   15,  219,  219,  219,
 /* 11480 */   219,  219,   21,  219,  219,  219,  219,   54,  219,   28,
 /* 11490 */   219,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /* 11500 */    39,  219,   41,  219,    7,   44,    9,   10,  219,  219,
 /* 11510 */   219,  219,  219,  219,  219,   54,  219,  219,   21,  219,
 /* 11520 */    59,   60,   61,  219,  219,   28,   65,  219,  219,  219,
 /* 11530 */    33,   34,   35,   36,   37,   38,   39,  219,   41,  219,
 /* 11540 */   219,   44,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11550 */   219,   54,  219,  219,  219,  219,   59,   60,   61,  219,
 /* 11560 */   219,  219,   65,  219,  219,  219,  219,  106,  219,  219,
 /* 11570 */     7,  219,    9,   10,  219,  114,  115,  116,  117,  118,
 /* 11580 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /* 11590 */   219,   28,  219,  219,  219,   98,   33,   34,   35,   36,
 /* 11600 */    37,   38,   39,  106,   41,  219,  219,   44,   45,  219,
 /* 11610 */   219,  114,  115,  116,  117,  118,  219,   54,  219,  219,
 /* 11620 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /* 11630 */   219,  219,  219,  219,  219,  219,    7,  219,    9,   10,
 /* 11640 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11650 */    21,  219,  219,  219,  219,  219,  219,   28,  219,  219,
 /* 11660 */   219,  219,   33,   34,   35,   36,   37,   38,   39,  106,
 /* 11670 */    41,  219,  219,   44,  219,  219,  219,  114,  115,  116,
 /* 11680 */   117,  118,  219,   54,  219,  219,  219,  219,   59,   60,
 /* 11690 */    61,  219,  219,  219,   65,  219,  219,  219,  219,  219,
 /* 11700 */   219,  219,    7,  219,    9,   10,  219,  219,  219,  219,
 /* 11710 */    15,  219,  219,  219,  219,  219,   21,  219,  219,  219,
 /* 11720 */   219,  219,  219,   28,  219,  219,  219,   98,   33,   34,
 /* 11730 */    35,   36,   37,   38,   39,  106,   41,  219,    7,   44,
 /* 11740 */     9,   10,  219,  114,  115,  116,  117,  118,  219,   54,
 /* 11750 */   219,  219,   21,  219,   59,   60,   61,  219,  219,   28,
 /* 11760 */    65,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /* 11770 */    39,  219,   41,  219,  219,   44,  219,  219,  219,  219,
 /* 11780 */   219,  219,  219,  219,  219,   54,  219,  219,  219,  219,
 /* 11790 */    59,   60,   61,  219,  219,  219,   65,  219,   67,  219,
 /* 11800 */   219,  106,  219,  219,    7,  219,    9,   10,  219,  114,
 /* 11810 */   115,  116,  117,  118,  219,  219,  219,  219,   21,  219,
 /* 11820 */   219,  219,  219,  219,  219,   28,  219,  219,  219,  219,
 /* 11830 */    33,   34,   35,   36,   37,   38,   39,  106,   41,  219,
 /* 11840 */     7,   44,    9,   10,  219,  114,  115,  116,  117,  118,
 /* 11850 */   219,   54,  219,  219,   21,  219,   59,   60,   61,  219,
 /* 11860 */   219,   28,   65,  219,   67,  219,   33,   34,   35,   36,
 /* 11870 */    37,   38,   39,  219,   41,  219,  219,   44,  219,  219,
 /* 11880 */   219,  219,  219,  219,  219,  219,  219,   54,  219,  219,
 /* 11890 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /* 11900 */    67,  219,  219,  106,  219,  219,    7,  219,    9,   10,
 /* 11910 */   219,  114,  115,  116,  117,  118,  219,  219,  219,  219,
 /* 11920 */    21,  219,  219,  219,  219,  219,  219,   28,  219,  219,
 /* 11930 */   219,  219,   33,   34,   35,   36,   37,   38,   39,  106,
 /* 11940 */    41,  219,    7,   44,    9,   10,  219,  114,  115,  116,
 /* 11950 */   117,  118,  219,   54,  219,  219,   21,  219,   59,   60,
 /* 11960 */    61,  219,  219,   28,   65,  219,   67,  219,   33,   34,
 /* 11970 */    35,   36,   37,   38,   39,  219,   41,  219,  219,   44,
 /* 11980 */   219,  219,  219,  219,  219,  219,  219,  219,  219,   54,
 /* 11990 */   219,  219,  219,  219,   59,   60,   61,  219,  219,  219,
 /* 12000 */    65,  219,   67,  219,  219,  106,  219,  219,    7,  219,
 /* 12010 */     9,   10,  219,  114,  115,  116,  117,  118,  219,  219,
 /* 12020 */   219,  219,   21,  219,  219,  219,  219,  219,  219,   28,
 /* 12030 */   219,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /* 12040 */    39,  106,   41,  219,    7,   44,    9,   10,  219,  114,
 /* 12050 */   115,  116,  117,  118,  219,   54,  219,  219,   21,  219,
 /* 12060 */    59,   60,   61,  219,  219,   28,   65,  219,   67,  219,
 /* 12070 */    33,   34,   35,   36,   37,   38,   39,  219,   41,  219,
 /* 12080 */   219,   44,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 12090 */   219,   54,  219,  219,  219,  219,   59,   60,   61,  219,
 /* 12100 */   219,  219,   65,  219,   67,  219,  219,  106,  219,  219,
 /* 12110 */   219,  219,  219,  219,  219,  114,  115,  116,  117,  118,
 /* 12120 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 12130 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 12140 */   219,  219,  219,  106,  219,  219,  219,  219,  219,  219,
 /* 12150 */   219,  114,  115,  116,  117,  118,  219,  219,  219,  219,
 /* 12160 */     8,  219,  219,   11,   12,   13,   14,   15,   16,   17,
 /* 12170 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /* 12180 */    28,   29,   30,   31,   32,    7,  219,    9,   10,  219,
 /* 12190 */   219,  219,  219,   41,   42,  219,  219,  219,  219,   21,
 /* 12200 */   219,  219,  219,  219,  219,  219,   28,  219,  219,  219,
 /* 12210 */   219,   33,   34,   35,   36,   37,   38,   39,  219,   41,
 /* 12220 */   219,    7,   44,    9,   10,  219,  219,  219,  219,  219,
 /* 12230 */   219,  219,   54,  219,  219,   21,  219,   59,   60,   61,
 /* 12240 */   219,  219,   28,   65,  219,  219,  219,   33,   34,   35,
 /* 12250 */    36,   37,   38,   39,  219,   41,  219,  219,   44,  219,
 /* 12260 */   219,  219,  219,  219,  219,  219,  219,  219,   54,  219,
 /* 12270 */   219,  219,  219,   59,   60,   61,  219,  219,  219,   65,
 /* 12280 */   219,  219,  219,  219,  106,  219,  219,  219,  219,  219,
 /* 12290 */   219,  219,  114,  115,  116,  117,  118,  219,  219,  219,
 /* 12300 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 12310 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 12320 */   106,  219,  219,  219,  219,  219,  219,  219,  114,  115,
 /* 12330 */   116,  117,  118,
};
#define YY_SHIFT_USE_DFLT (-40)
#define YY_SHIFT_COUNT (590)
#define YY_SHIFT_MIN   (-39)
#define YY_SHIFT_MAX   (12214)
static const short yy_shift_ofst[] = {
 /*     0 */  6861, 6861, 6660, 6547, 6434, 6321, 6208, 6095, 5982, 5869,
 /*    10 */  5756, 5643, 5530, 5417, 5304, 5191, 5078, 4965, 4852, 4739,
 /*    20 */  4626, 4513, 4400, 4287, 4174, 4061, 3948, 3835, 3722, 6773,
 /*    30 */  6773, 6773, 6773, 6773, 6773, 6773, 6773, 3609, 3496, 3383,
 /*    40 */  3270, 3157, 3044, 2931, 2818, 2705, 2592, 2479, 2366, 2253,
 /*    50 */  2140, 2027, 1914, 1801, 1688, 1575, 1462, 1349, 1236, 1123,
 /*    60 */  1010,  897,  784,  671,  558,  445,  332,  219,  106,   -7,
 /*    70 */  6773, 6773, 12037,  151, 12214, 10944, 10908, 10793, 10757, 10642,
 /*    80 */  10606, 10491, 10455, 10340, 10304, 12178,   38,  716,  716, 6949,
 /*    90 */  12001, 11935, 11899, 11833, 11797, 11731, 11695, 12178, 12178, 12178,
 /*   100 */  12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178,
 /*   110 */  12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178,
 /*   120 */  12178, 9032, 9032, 9032, 9032, 9032, 9032, 9032, 9032, 11629,
 /*   130 */  11563, 11497, 11461, 12178, 12178, 12178, 12178, 12178, 12178, 12178,
 /*   140 */  12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178,
 /*   150 */  12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178,
 /*   160 */  12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178,
 /*   170 */  12178, 12178, 12178, 12178, 12178, 12178, 12178, 12178,  603,  490,
 /*   180 */  7558, 1620, 8856, 8683, 8510, 8337, 8164, 7991, 7818, 7645,
 /*   190 */  7472, 9377, 7726, 9202, 7387, 1507, 1507, 1507, 1507, 1507,
 /*   200 */  1394, 1394, 7655, 7906, 7513, 1281, 1281, 1281, 7391, 1733,
 /*   210 */  1733, 1733, 7828, 7733, 7398,  481,  -20,  182, 1004,  982,
 /*   220 */  1461, 1461, 1461, 1461, 1461, 1461, 1461, 1461, 1461, 1461,
 /*   230 */  1461, 1480, 1480, 1480, 1480, 1480, 1461, 1461, 1461, 1461,
 /*   240 */  1461, 1461, 1461, 1461, 1461, 1461, 1461, 1470, 1409, 1450,
 /*   250 */  7298,  377,  264, 7391, 7391, 1168, 1055, 7391, 7391, 7391,
 /*   260 */  7391,  942,  829, 7391, 7391, 7391, 7391, 7391, 7391, 7391,
 /*   270 */  7391, 7391, 7391, 7697, 7697, 7697, 7697, 7697, 7697, 7697,
 /*   280 */  7697, 7697, 7697,  481,  862,  722,  860,  759,  609,   69,
 /*   290 */   858,  858,  858,  818,  858,  684,  397,  679,  858,  773,
 /*   300 */   562,  713,  675, 1474, 1480, 1466, 1470, 1461, 1409, 1450,
 /*   310 */  11197, 11433, 11378, 11323, 11281, 11239, 11197, 11155, 11100, 11059,
 /*   320 */  10244, 7096, 12152, 12152, 12152, 12152, 12152, 12152, 12152, 12152,
 /*   330 */  12152, 12152, 12152, 12152, 7020, 7020, 10411, 7297, 7392, 7120,
 /*   340 */  7120, 7120, 7120, 7120,   90,   90,   90,   90,   90,   90,
 /*   350 */    90,   90,   90,  200,  200,  200,  200,  330,  217,  104,
 /*   360 */   -32,  180,  103,   41,  -28, 1360, 1351, 1320, 1297,  299,
 /*   370 */   669, 1262,  608,  533, 1247,  382,  566,  453,  714, 1238,
 /*   380 */   658,  556, 1149,  657,  505, 1134, 1125, 1036,  350, 1021,
 /*   390 */  1012,  923,  797,  796,  423,  751,  707,  705,  642,  641,
 /*   400 */   640,  638,  637,  594,  564,  543,  524,  284,  279,  157,
 /*   410 */   462,  406,  242,  129,  292,  334,  237,  243,  221,   79,
 /*   420 */   120,  -26,  -26,  255,   21,  -26,  -26,  -26,  -26,   34,
 /*   430 */   -26,  -26,  -26,  -26,    0, 1164, 1163, 1158, 1435, 1428,
 /*   440 */  1423, 1436, 1421, 1425, 1424, 1416, 1432, 1420, 1419, 1393,
 /*   450 */  1414, 1410, 1399, 1398, 1378, 1407, 1405, 1400, 1388, 1391,
 /*   460 */  1390, 1387, 1385, 1384, 1383, 1369, 1371, 1354, 1368, 1350,
 /*   470 */  1362, 1352, 1358, 1357, 1347, 1332, 1336, 1311, 1333, 1310,
 /*   480 */  1331, 1308, 1326, 1324, 1303, 1319, 1316, 1323, 1274, 1296,
 /*   490 */  1299, 1322, 1254, 1264, 1287, 1269, 1278, 1277,  246, 1273,
 /*   500 */  1255, 1260, 1239, 1243, 1205, 1232, 1234, 1244, 1215, 1224,
 /*   510 */  1221, 1194, 1209, 1204, 1201, 1200, 1198, 1197, 1161, 1203,
 /*   520 */  1155, 1178, 1183, 1115, 1174, 1154, 1164, 1163, 1158, 1157,
 /*   530 */  1135, 1132, 1142, 1141, 1136, 1126, 1111, 1096, 1101, 1089,
 /*   540 */  1097, 1046, 1083, 1065, 1060,  912, 1051,  985,  952,  947,
 /*   550 */   919,  905,  893,  875,  880,  871,  836,  866,  835,  806,
 /*   560 */   804,  793,  791,  754,  723,  686,  586,  677,  662,  663,
 /*   570 */   586,  610,  577,  565,  522,  493,  385,  294,  267,  393,
 /*   580 */   304,  246,  211,  207,  175,  147,    2,  -22,  150,  -39,
 /*   590 */     1,
};
#define YY_REDUCE_USE_DFLT (-152)
#define YY_REDUCE_COUNT (309)
#define YY_REDUCE_MIN   (-151)
#define YY_REDUCE_MAX   (10100)
static const short yy_reduce_ofst[] = {
 /*     0 */  6953, 7056, 10023, 9927, 9850, 9754, 9677, 9581, 9504, 9408,
 /*    10 */  9331, 9235, 9158, 9062, 8985, 8889, 8812, 8716, 8639, 8543,
 /*    20 */  8466, 8370, 8293, 8197, 8120, 8024, 7947, 7851, 7774, 7678,
 /*    30 */  7601, 7505, 7428, 7332, 7255, 7159, 7082, 10100, 10100, 10100,
 /*    40 */  10100, 10100, 10100, 10100, 10100, 10100, 10100, 10100, 10100, 10100,
 /*    50 */  10100, 10100, 10100, 10100, 10100, 10100, 10100, 10100, 10100, 10100,
 /*    60 */  10100, 10100, 10100, 10100, 10100, 10100, 10100, 10100, 10100, 10100,
 /*    70 */  10100, 10100, 6961, -151, 7043, 1508, 1395, 1282, 1169, 1056,
 /*    80 */   943,  830,  717,  604,  491,  379,  302,  189,   76, 3541,
 /*    90 */  1853, 1853, 1853, 1853, 1853, 1853, 1811, 1966, 1020,  907,
 /*   100 */  1874, 3428, 3315, 3202, 3089, 2976, 2863, 2750, 2637, 2524,
 /*   110 */  2411, 2298, 2185, 2072, 1959, 1846,  -68,  794,  681,  229,
 /*   120 */   116, 1787, 1761, 1754, 1726, 1674, 1648, 1641,  398, 3767,
 /*   130 */  3732, 3682, 3661, 3654, 3619, 3569, 3548, 3506, 3456, 3435,
 /*   140 */  3393, 3343, 3322, 3280, 3230, 3209, 3167, 3117, 3096, 3054,
 /*   150 */  3004, 2983, 2941, 2891, 2870, 2828, 2778, 2757, 2715, 2665,
 /*   160 */  2644, 2602, 2552, 2531, 2489, 2439, 2418, 2376, 2326, 2305,
 /*   170 */  2263, 2213, 2150, 2100, 2079, 2037, 1924, 1585,  632,  301,
 /*   180 */   473,  371,  557,  444,  331,  240,  218,  184,  105,   -8,
 /*   190 */  -121,  167, -144,  280,  289,   22,   22,   22,   22,   22,
 /*   200 */  -119, -119,   50, -101,  998,  978,  976,  863, -104,  978,
 /*   210 */   976,  863,  878,  854,  674,  940,  932,  908,  887,  877,
 /*   220 */   817,  821,  778,  814,  775,  747,  741,  769,  753,  732,
 /*   230 */   701,  876,  770,  370,  342,  -73,  821,  817,  814,  778,
 /*   240 */   775,  769,  753,  747,  741,  732,  701,  655,  652,  700,
 /*   250 */  1417, 1415, 1415, 1500, 1499, 1415, 1415, 1497, 1496, 1492,
 /*   260 */  1491, 1415, 1415, 1490, 1478, 1477, 1469, 1468, 1464, 1453,
 /*   270 */  1451, 1429, 1427, 1443, 1442, 1441, 1431, 1422, 1408, 1406,
 /*   280 */  1401, 1396, 1366, 1353, 1437, 1434, 1402, 1386, 1433, 1364,
 /*   290 */  1430, 1418, 1413, 1412, 1411, 1382, 1376, 1377, 1404, 1397,
 /*   300 */  1389, 1343, 1343, 1372, 1370, 1363, 1327, 1305, 1312, 1365,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */  1402,  951, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*    10 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*    20 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*    30 */  1402, 1402, 1402, 1402, 1402, 1402, 1190, 1402, 1402, 1402,
 /*    40 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*    50 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*    60 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*    70 */  1192, 1191, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*    80 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1007, 1006, 1402,
 /*    90 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   100 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   110 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   120 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   130 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   140 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   150 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   160 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   170 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1010, 1015,
 /*   180 */  1402, 1011, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   190 */  1402, 1402, 1402, 1402, 1402, 1008, 1012, 1014, 1013, 1009,
 /*   200 */  1016, 1017, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   210 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1171, 1175,
 /*   220 */  1139, 1138, 1137, 1136, 1135, 1134, 1133, 1132, 1131, 1130,
 /*   230 */  1129, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   240 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1200, 1402,
 /*   250 */  1400, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   260 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   270 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   280 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   290 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   300 */  1402, 1172, 1176, 1402, 1402, 1402, 1402, 1402, 1201, 1402,
 /*   310 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   320 */  1402, 1402, 1366, 1402, 1256, 1252, 1281, 1399, 1365, 1379,
 /*   330 */  1364, 1360, 1363, 1285, 1289, 1288, 1320, 1314, 1313, 1319,
 /*   340 */  1318, 1317, 1316, 1315, 1290, 1291, 1292, 1293, 1294, 1295,
 /*   350 */  1296, 1300, 1297, 1312, 1308, 1307, 1283, 1402, 1402, 1402,
 /*   360 */  1402, 1402, 1324, 1324,  984, 1402, 1402, 1402, 1402, 1402,
 /*   370 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   380 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   390 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   400 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   410 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   420 */  1402, 1323, 1321, 1402, 1324, 1299, 1311, 1310, 1309, 1402,
 /*   430 */  1287, 1286, 1284, 1282, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   440 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   450 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   460 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   470 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   480 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   490 */  1091, 1088, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   500 */  1303, 1402, 1302, 1402, 1402, 1338, 1402, 1402, 1402, 1402,
 /*   510 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   520 */  1402, 1402, 1402, 1402, 1402, 1279, 1322, 1358, 1359, 1357,
 /*   530 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   540 */  1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402, 1402,
 /*   550 */  1402, 1402, 1117, 1402, 1115, 1402, 1113, 1402, 1402, 1118,
 /*   560 */  1402, 1116, 1402, 1114, 1112, 1402, 1395, 1382, 1380, 1402,
 /*   570 */  1402, 1111, 1402, 1402, 1402, 1335, 1332, 1326, 1325, 1402,
 /*   580 */  1402, 1304, 1402, 1402, 1402, 1301, 1402,  984, 1402, 1402,
 /*   590 */  1402,  953, 1108, 1107, 1106, 1105, 1104, 1103, 1102, 1101,
 /*   600 */  1100, 1099, 1098, 1097,  998,  997,  996,  994,  995,  993,
 /*   610 */   992,  990, 1002, 1003, 1001, 1019, 1025, 1027, 1023, 1021,
 /*   620 */  1024, 1031, 1033, 1032, 1030, 1029, 1028, 1026, 1022, 1020,
 /*   630 */  1018, 1037, 1065, 1067, 1064, 1053, 1055, 1052, 1063, 1066,
 /*   640 */  1062, 1051, 1054, 1050, 1059, 1061, 1058, 1047, 1049, 1046,
 /*   650 */  1057, 1060, 1056, 1045, 1048, 1044, 1036, 1000,  999,  991,
 /*   660 */   989,  988, 1035, 1043, 1041, 1042, 1040, 1039, 1034, 1077,
 /*   670 */  1075, 1071, 1074, 1070, 1086, 1085, 1084, 1083, 1082, 1081,
 /*   680 */  1080, 1079, 1078, 1076, 1073, 1069, 1072, 1090, 1128, 1096,
 /*   690 */  1095, 1093, 1092, 1094, 1089, 1087, 1068, 1038, 1005, 1004,
 /*   700 */   987,  986, 1342, 1341, 1356, 1355, 1354, 1353, 1352, 1351,
 /*   710 */  1305, 1346, 1345, 1372, 1374, 1373, 1371, 1340, 1339, 1361,
 /*   720 */  1350, 1349, 1348, 1347, 1367, 1368, 1110, 1369, 1370, 1173,
 /*   730 */  1181, 1174, 1183, 1179, 1177, 1184, 1185, 1182, 1180, 1189,
 /*   740 */  1193, 1195, 1197, 1199, 1203, 1209, 1210, 1208, 1206, 1207,
 /*   750 */  1205, 1216, 1215, 1214, 1213, 1212, 1144, 1401, 1400, 1276,
 /*   760 */  1275, 1274, 1273, 1272, 1271, 1270, 1269, 1268, 1267, 1278,
 /*   770 */  1280, 1277, 1266, 1265, 1264, 1263, 1262, 1261, 1260, 1259,
 /*   780 */  1258, 1257, 1255, 1254, 1253, 1219, 1251, 1250, 1249, 1247,
 /*   790 */  1248, 1246, 1240, 1239, 1238, 1237, 1236, 1235, 1234, 1230,
 /*   800 */  1229, 1245, 1244, 1242, 1241, 1233, 1232, 1243, 1231, 1228,
 /*   810 */  1227, 1226, 1225, 1224, 1223, 1222, 1221, 1220, 1218, 1217,
 /*   820 */  1211, 1204, 1202, 1198, 1196, 1194, 1188, 1187, 1186, 1178,
 /*   830 */  1170, 1169, 1168, 1167, 1166, 1165, 1164, 1163, 1162, 1161,
 /*   840 */  1160, 1159, 1158, 1157, 1156, 1155, 1154, 1153, 1152, 1151,
 /*   850 */  1150, 1149, 1148, 1147, 1146, 1145, 1143, 1125, 1123, 1121,
 /*   860 */  1127, 1126, 1124, 1122, 1120, 1392, 1394, 1398, 1393, 1391,
 /*   870 */  1390, 1389, 1388, 1387, 1386, 1385, 1384, 1383, 1382, 1381,
 /*   880 */  1380, 1119, 1109, 1362, 1359, 1358, 1357, 1344, 1343, 1337,
 /*   890 */  1336, 1334, 1333, 1332, 1331, 1330, 1329, 1328, 1327, 1326,
 /*   900 */  1325, 1322, 1298, 1142, 1141, 1140, 1139, 1138, 1137, 1136,
 /*   910 */  1135, 1134, 1133, 1132, 1131, 1130, 1129, 1306,  983,  985,
 /*   920 */   982,  981,  980,  979,  978,  977,  976,  975,  974,  973,
 /*   930 */   972,  971,  970,  969,  968,  967,  966,  965,  964,  963,
 /*   940 */   962,  961,  960,  959,  958,  957,  956,  955,  954,  952,
};

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
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
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  xx_ARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
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
static const char *const yyTokenName[] = { 
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
static const char *const yyRuleName[] = {
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
 /* 153 */ "xx_function_def ::= COMMENT FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 154 */ "xx_function_def ::= COMMENT FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 155 */ "xx_function_def ::= COMMENT FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 156 */ "xx_function_def ::= COMMENT xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 157 */ "xx_function_def ::= COMMENT xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 158 */ "xx_function_def ::= COMMENT xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 159 */ "xx_parameter_list ::= xx_parameter_list COMMA xx_parameter",
 /* 160 */ "xx_parameter_list ::= xx_parameter",
 /* 161 */ "xx_parameter ::= IDENTIFIER",
 /* 162 */ "xx_parameter ::= CONST IDENTIFIER",
 /* 163 */ "xx_parameter ::= xx_parameter_type IDENTIFIER",
 /* 164 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER",
 /* 165 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER",
 /* 166 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER",
 /* 167 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER",
 /* 168 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER",
 /* 169 */ "xx_parameter ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 170 */ "xx_parameter ::= CONST IDENTIFIER ASSIGN xx_literal_expr",
 /* 171 */ "xx_parameter ::= xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 172 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 173 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 174 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 175 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 176 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 177 */ "xx_parameter_cast ::= LESS IDENTIFIER GREATER",
 /* 178 */ "xx_parameter_cast_collection ::= LESS IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE GREATER",
 /* 179 */ "xx_parameter_type ::= TYPE_INTEGER",
 /* 180 */ "xx_parameter_type ::= TYPE_UINTEGER",
 /* 181 */ "xx_parameter_type ::= TYPE_LONG",
 /* 182 */ "xx_parameter_type ::= TYPE_ULONG",
 /* 183 */ "xx_parameter_type ::= TYPE_CHAR",
 /* 184 */ "xx_parameter_type ::= TYPE_UCHAR",
 /* 185 */ "xx_parameter_type ::= TYPE_DOUBLE",
 /* 186 */ "xx_parameter_type ::= TYPE_BOOL",
 /* 187 */ "xx_parameter_type ::= TYPE_STRING",
 /* 188 */ "xx_parameter_type ::= TYPE_ARRAY",
 /* 189 */ "xx_parameter_type ::= TYPE_VAR",
 /* 190 */ "xx_parameter_type ::= TYPE_CALLABLE",
 /* 191 */ "xx_parameter_type ::= TYPE_RESOURCE",
 /* 192 */ "xx_parameter_type ::= TYPE_OBJECT",
 /* 193 */ "xx_statement_list ::= xx_statement_list xx_statement",
 /* 194 */ "xx_statement_list ::= xx_statement",
 /* 195 */ "xx_statement ::= xx_cblock",
 /* 196 */ "xx_statement ::= xx_let_statement",
 /* 197 */ "xx_statement ::= xx_if_statement",
 /* 198 */ "xx_statement ::= xx_loop_statement",
 /* 199 */ "xx_statement ::= xx_echo_statement",
 /* 200 */ "xx_statement ::= xx_return_statement",
 /* 201 */ "xx_statement ::= xx_require_statement",
 /* 202 */ "xx_statement ::= xx_fetch_statement",
 /* 203 */ "xx_statement ::= xx_fcall_statement",
 /* 204 */ "xx_statement ::= xx_mcall_statement",
 /* 205 */ "xx_statement ::= xx_scall_statement",
 /* 206 */ "xx_statement ::= xx_unset_statement",
 /* 207 */ "xx_statement ::= xx_throw_statement",
 /* 208 */ "xx_statement ::= xx_declare_statement",
 /* 209 */ "xx_statement ::= xx_break_statement",
 /* 210 */ "xx_statement ::= xx_continue_statement",
 /* 211 */ "xx_statement ::= xx_while_statement",
 /* 212 */ "xx_statement ::= xx_do_while_statement",
 /* 213 */ "xx_statement ::= xx_try_catch_statement",
 /* 214 */ "xx_statement ::= xx_switch_statement",
 /* 215 */ "xx_statement ::= xx_for_statement",
 /* 216 */ "xx_statement ::= xx_comment",
 /* 217 */ "xx_statement ::= xx_empty_statement",
 /* 218 */ "xx_empty_statement ::= DOTCOMMA",
 /* 219 */ "xx_break_statement ::= BREAK DOTCOMMA",
 /* 220 */ "xx_continue_statement ::= CONTINUE DOTCOMMA",
 /* 221 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 222 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements",
 /* 223 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 224 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 225 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 226 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements",
 /* 227 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 228 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 229 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 230 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 231 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 232 */ "xx_elseif_statements ::= xx_elseif_statements xx_elseif_statement",
 /* 233 */ "xx_elseif_statements ::= xx_elseif_statement",
 /* 234 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 235 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 236 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 237 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN xx_case_clauses BRACKET_CLOSE",
 /* 238 */ "xx_case_clauses ::= xx_case_clauses xx_case_clause",
 /* 239 */ "xx_case_clauses ::= xx_case_clause",
 /* 240 */ "xx_case_clause ::= CASE xx_literal_expr COLON",
 /* 241 */ "xx_case_clause ::= CASE xx_literal_expr COLON xx_statement_list",
 /* 242 */ "xx_case_clause ::= DEFAULT COLON xx_statement_list",
 /* 243 */ "xx_loop_statement ::= LOOP BRACKET_OPEN BRACKET_CLOSE",
 /* 244 */ "xx_loop_statement ::= LOOP BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 245 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 246 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 247 */ "xx_do_while_statement ::= DO BRACKET_OPEN BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 248 */ "xx_do_while_statement ::= DO BRACKET_OPEN xx_statement_list BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 249 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN BRACKET_CLOSE",
 /* 250 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 251 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_catch_statement_list",
 /* 252 */ "xx_catch_statement_list ::= xx_catch_statement_list xx_catch_statement",
 /* 253 */ "xx_catch_statement_list ::= xx_catch_statement",
 /* 254 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 255 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN BRACKET_CLOSE",
 /* 256 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN BRACKET_CLOSE",
 /* 257 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 258 */ "xx_catch_classes_list ::= xx_catch_classes_list BITWISE_OR xx_catch_class",
 /* 259 */ "xx_catch_classes_list ::= xx_catch_class",
 /* 260 */ "xx_catch_class ::= IDENTIFIER",
 /* 261 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 262 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 263 */ "xx_for_statement ::= FOR IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 264 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 265 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 266 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 267 */ "xx_let_statement ::= LET xx_let_assignments DOTCOMMA",
 /* 268 */ "xx_let_assignments ::= xx_let_assignments COMMA xx_let_assignment",
 /* 269 */ "xx_let_assignments ::= xx_let_assignment",
 /* 270 */ "xx_assignment_operator ::= ASSIGN",
 /* 271 */ "xx_assignment_operator ::= ADDASSIGN",
 /* 272 */ "xx_assignment_operator ::= SUBASSIGN",
 /* 273 */ "xx_assignment_operator ::= MULASSIGN",
 /* 274 */ "xx_assignment_operator ::= DIVASSIGN",
 /* 275 */ "xx_assignment_operator ::= CONCATASSIGN",
 /* 276 */ "xx_assignment_operator ::= MODASSIGN",
 /* 277 */ "xx_let_assignment ::= IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 278 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 279 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 280 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 281 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 282 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 283 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 284 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 285 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 286 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 287 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 288 */ "xx_let_assignment ::= IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 289 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 290 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 291 */ "xx_array_offset_list ::= xx_array_offset_list xx_array_offset",
 /* 292 */ "xx_array_offset_list ::= xx_array_offset",
 /* 293 */ "xx_array_offset ::= SBRACKET_OPEN xx_index_expr SBRACKET_CLOSE",
 /* 294 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER INCR",
 /* 295 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER DECR",
 /* 296 */ "xx_let_assignment ::= IDENTIFIER INCR",
 /* 297 */ "xx_let_assignment ::= INCR IDENTIFIER",
 /* 298 */ "xx_let_assignment ::= IDENTIFIER DECR",
 /* 299 */ "xx_let_assignment ::= DECR IDENTIFIER",
 /* 300 */ "xx_let_assignment ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 301 */ "xx_let_assignment ::= BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 302 */ "xx_index_expr ::= xx_common_expr",
 /* 303 */ "xx_echo_statement ::= ECHO xx_echo_expressions DOTCOMMA",
 /* 304 */ "xx_echo_expressions ::= xx_echo_expressions COMMA xx_echo_expression",
 /* 305 */ "xx_echo_expressions ::= xx_echo_expression",
 /* 306 */ "xx_echo_expression ::= xx_common_expr",
 /* 307 */ "xx_mcall_statement ::= xx_mcall_expr DOTCOMMA",
 /* 308 */ "xx_fcall_statement ::= xx_fcall_expr DOTCOMMA",
 /* 309 */ "xx_scall_statement ::= xx_scall_expr DOTCOMMA",
 /* 310 */ "xx_fetch_statement ::= xx_fetch_expr DOTCOMMA",
 /* 311 */ "xx_return_statement ::= RETURN xx_common_expr DOTCOMMA",
 /* 312 */ "xx_return_statement ::= RETURN DOTCOMMA",
 /* 313 */ "xx_require_statement ::= REQUIRE xx_common_expr DOTCOMMA",
 /* 314 */ "xx_unset_statement ::= UNSET xx_common_expr DOTCOMMA",
 /* 315 */ "xx_throw_statement ::= THROW xx_common_expr DOTCOMMA",
 /* 316 */ "xx_declare_statement ::= TYPE_INTEGER xx_declare_variable_list DOTCOMMA",
 /* 317 */ "xx_declare_statement ::= TYPE_UINTEGER xx_declare_variable_list DOTCOMMA",
 /* 318 */ "xx_declare_statement ::= TYPE_CHAR xx_declare_variable_list DOTCOMMA",
 /* 319 */ "xx_declare_statement ::= TYPE_UCHAR xx_declare_variable_list DOTCOMMA",
 /* 320 */ "xx_declare_statement ::= TYPE_LONG xx_declare_variable_list DOTCOMMA",
 /* 321 */ "xx_declare_statement ::= TYPE_ULONG xx_declare_variable_list DOTCOMMA",
 /* 322 */ "xx_declare_statement ::= TYPE_DOUBLE xx_declare_variable_list DOTCOMMA",
 /* 323 */ "xx_declare_statement ::= TYPE_STRING xx_declare_variable_list DOTCOMMA",
 /* 324 */ "xx_declare_statement ::= TYPE_BOOL xx_declare_variable_list DOTCOMMA",
 /* 325 */ "xx_declare_statement ::= TYPE_VAR xx_declare_variable_list DOTCOMMA",
 /* 326 */ "xx_declare_statement ::= TYPE_ARRAY xx_declare_variable_list DOTCOMMA",
 /* 327 */ "xx_declare_variable_list ::= xx_declare_variable_list COMMA xx_declare_variable",
 /* 328 */ "xx_declare_variable_list ::= xx_declare_variable",
 /* 329 */ "xx_declare_variable ::= IDENTIFIER",
 /* 330 */ "xx_declare_variable ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 331 */ "xx_assign_expr ::= xx_common_expr",
 /* 332 */ "xx_common_expr ::= NOT xx_common_expr",
 /* 333 */ "xx_common_expr ::= SUB xx_common_expr",
 /* 334 */ "xx_common_expr ::= ISSET xx_common_expr",
 /* 335 */ "xx_common_expr ::= REQUIRE xx_common_expr",
 /* 336 */ "xx_common_expr ::= CLONE xx_common_expr",
 /* 337 */ "xx_common_expr ::= EMPTY xx_common_expr",
 /* 338 */ "xx_common_expr ::= LIKELY xx_common_expr",
 /* 339 */ "xx_common_expr ::= UNLIKELY xx_common_expr",
 /* 340 */ "xx_common_expr ::= xx_common_expr EQUALS xx_common_expr",
 /* 341 */ "xx_common_expr ::= xx_common_expr NOTEQUALS xx_common_expr",
 /* 342 */ "xx_common_expr ::= xx_common_expr IDENTICAL xx_common_expr",
 /* 343 */ "xx_common_expr ::= xx_common_expr NOTIDENTICAL xx_common_expr",
 /* 344 */ "xx_common_expr ::= xx_common_expr LESS xx_common_expr",
 /* 345 */ "xx_common_expr ::= xx_common_expr GREATER xx_common_expr",
 /* 346 */ "xx_common_expr ::= xx_common_expr LESSEQUAL xx_common_expr",
 /* 347 */ "xx_common_expr ::= xx_common_expr GREATEREQUAL xx_common_expr",
 /* 348 */ "xx_common_expr ::= PARENTHESES_OPEN xx_common_expr PARENTHESES_CLOSE",
 /* 349 */ "xx_common_expr ::= PARENTHESES_OPEN xx_parameter_type PARENTHESES_CLOSE xx_common_expr",
 /* 350 */ "xx_common_expr ::= LESS IDENTIFIER GREATER xx_common_expr",
 /* 351 */ "xx_common_expr ::= xx_common_expr ARROW IDENTIFIER",
 /* 352 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 353 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE",
 /* 354 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER",
 /* 355 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 356 */ "xx_common_expr ::= xx_common_expr SBRACKET_OPEN xx_common_expr SBRACKET_CLOSE",
 /* 357 */ "xx_common_expr ::= xx_common_expr ADD xx_common_expr",
 /* 358 */ "xx_common_expr ::= xx_common_expr SUB xx_common_expr",
 /* 359 */ "xx_common_expr ::= xx_common_expr MUL xx_common_expr",
 /* 360 */ "xx_common_expr ::= xx_common_expr DIV xx_common_expr",
 /* 361 */ "xx_common_expr ::= xx_common_expr MOD xx_common_expr",
 /* 362 */ "xx_common_expr ::= xx_common_expr CONCAT xx_common_expr",
 /* 363 */ "xx_common_expr ::= xx_common_expr AND xx_common_expr",
 /* 364 */ "xx_common_expr ::= xx_common_expr OR xx_common_expr",
 /* 365 */ "xx_common_expr ::= xx_common_expr BITWISE_AND xx_common_expr",
 /* 366 */ "xx_common_expr ::= xx_common_expr BITWISE_OR xx_common_expr",
 /* 367 */ "xx_common_expr ::= xx_common_expr BITWISE_XOR xx_common_expr",
 /* 368 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTLEFT xx_common_expr",
 /* 369 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTRIGHT xx_common_expr",
 /* 370 */ "xx_common_expr ::= xx_common_expr INSTANCEOF xx_common_expr",
 /* 371 */ "xx_fetch_expr ::= FETCH IDENTIFIER COMMA xx_common_expr",
 /* 372 */ "xx_common_expr ::= xx_fetch_expr",
 /* 373 */ "xx_common_expr ::= TYPEOF xx_common_expr",
 /* 374 */ "xx_common_expr ::= IDENTIFIER",
 /* 375 */ "xx_common_expr ::= INTEGER",
 /* 376 */ "xx_common_expr ::= STRING",
 /* 377 */ "xx_common_expr ::= CHAR",
 /* 378 */ "xx_common_expr ::= DOUBLE",
 /* 379 */ "xx_common_expr ::= NULL",
 /* 380 */ "xx_common_expr ::= TRUE",
 /* 381 */ "xx_common_expr ::= FALSE",
 /* 382 */ "xx_common_expr ::= CONSTANT",
 /* 383 */ "xx_common_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 384 */ "xx_common_expr ::= SBRACKET_OPEN xx_array_list SBRACKET_CLOSE",
 /* 385 */ "xx_common_expr ::= NEW IDENTIFIER",
 /* 386 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 387 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 388 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 389 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 390 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 391 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 392 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 393 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 394 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 395 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 396 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 397 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 398 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 399 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 400 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 401 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 402 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 403 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 404 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 405 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 406 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 407 */ "xx_common_expr ::= xx_mcall_expr",
 /* 408 */ "xx_common_expr ::= xx_scall_expr",
 /* 409 */ "xx_common_expr ::= xx_fcall_expr",
 /* 410 */ "xx_common_expr ::= xx_common_expr QUESTION xx_common_expr COLON xx_common_expr",
 /* 411 */ "xx_call_parameters ::= xx_call_parameters COMMA xx_call_parameter",
 /* 412 */ "xx_call_parameters ::= xx_call_parameter",
 /* 413 */ "xx_call_parameter ::= xx_common_expr",
 /* 414 */ "xx_call_parameter ::= IDENTIFIER COLON xx_common_expr",
 /* 415 */ "xx_call_parameter ::= BITWISE_AND xx_common_expr",
 /* 416 */ "xx_call_parameter ::= IDENTIFIER COLON BITWISE_AND xx_common_expr",
 /* 417 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 418 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 419 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 420 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 421 */ "xx_array_list ::= xx_array_list COMMA xx_array_item",
 /* 422 */ "xx_array_list ::= xx_array_item",
 /* 423 */ "xx_array_item ::= xx_array_key COLON xx_array_value",
 /* 424 */ "xx_array_item ::= xx_array_value",
 /* 425 */ "xx_array_key ::= CONSTANT",
 /* 426 */ "xx_array_key ::= IDENTIFIER",
 /* 427 */ "xx_array_key ::= STRING",
 /* 428 */ "xx_array_key ::= INTEGER",
 /* 429 */ "xx_array_value ::= xx_common_expr",
 /* 430 */ "xx_literal_expr ::= INTEGER",
 /* 431 */ "xx_literal_expr ::= CHAR",
 /* 432 */ "xx_literal_expr ::= STRING",
 /* 433 */ "xx_literal_expr ::= DOUBLE",
 /* 434 */ "xx_literal_expr ::= NULL",
 /* 435 */ "xx_literal_expr ::= FALSE",
 /* 436 */ "xx_literal_expr ::= TRUE",
 /* 437 */ "xx_literal_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 438 */ "xx_literal_expr ::= CONSTANT",
 /* 439 */ "xx_literal_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 440 */ "xx_literal_expr ::= SBRACKET_OPEN xx_literal_array_list SBRACKET_CLOSE",
 /* 441 */ "xx_literal_array_list ::= xx_literal_array_list COMMA xx_literal_array_item",
 /* 442 */ "xx_literal_array_list ::= xx_literal_array_item",
 /* 443 */ "xx_literal_array_item ::= xx_literal_array_key COLON xx_literal_array_value",
 /* 444 */ "xx_literal_array_item ::= xx_literal_array_value",
 /* 445 */ "xx_literal_array_key ::= IDENTIFIER",
 /* 446 */ "xx_literal_array_key ::= STRING",
 /* 447 */ "xx_literal_array_key ::= INTEGER",
 /* 448 */ "xx_literal_array_value ::= xx_literal_expr",
 /* 449 */ "xx_eval_expr ::= xx_common_expr",
 /* 450 */ "xx_comment ::= COMMENT",
 /* 451 */ "xx_cblock ::= CBLOCK",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

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
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  xx_ARG_FETCH;
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
      /* TERMINAL Destructor */
    case 1: /* PUBLIC */
    case 2: /* PROTECTED */
    case 3: /* STATIC */
    case 4: /* PRIVATE */
    case 5: /* SCOPED */
    case 6: /* COMMA */
    case 7: /* REQUIRE */
    case 8: /* QUESTION */
    case 9: /* LIKELY */
    case 10: /* UNLIKELY */
    case 11: /* INSTANCEOF */
    case 12: /* OR */
    case 13: /* AND */
    case 14: /* BITWISE_OR */
    case 15: /* BITWISE_AND */
    case 16: /* BITWISE_XOR */
    case 17: /* BITWISE_SHIFTLEFT */
    case 18: /* BITWISE_SHIFTRIGHT */
    case 19: /* EQUALS */
    case 20: /* IDENTICAL */
    case 21: /* LESS */
    case 22: /* GREATER */
    case 23: /* LESSEQUAL */
    case 24: /* GREATEREQUAL */
    case 25: /* NOTIDENTICAL */
    case 26: /* NOTEQUALS */
    case 27: /* ADD */
    case 28: /* SUB */
    case 29: /* CONCAT */
    case 30: /* MUL */
    case 31: /* DIV */
    case 32: /* MOD */
    case 33: /* ISSET */
    case 34: /* FETCH */
    case 35: /* EMPTY */
    case 36: /* TYPEOF */
    case 37: /* CLONE */
    case 38: /* NEW */
    case 39: /* NOT */
    case 40: /* PARENTHESES_CLOSE */
    case 41: /* SBRACKET_OPEN */
    case 42: /* ARROW */
    case 43: /* NAMESPACE */
    case 44: /* IDENTIFIER */
    case 45: /* DOTCOMMA */
    case 46: /* USE */
    case 47: /* AS */
    case 48: /* INTERFACE */
    case 49: /* EXTENDS */
    case 50: /* CLASS */
    case 51: /* IMPLEMENTS */
    case 52: /* ABSTRACT */
    case 53: /* FINAL */
    case 54: /* BRACKET_OPEN */
    case 55: /* BRACKET_CLOSE */
    case 56: /* COMMENT */
    case 57: /* ASSIGN */
    case 58: /* CONST */
    case 59: /* CONSTANT */
    case 60: /* FUNCTION */
    case 61: /* PARENTHESES_OPEN */
    case 62: /* INLINE */
    case 63: /* DEPRECATED */
    case 64: /* VOID */
    case 65: /* NULL */
    case 66: /* THIS */
    case 67: /* SBRACKET_CLOSE */
    case 68: /* TYPE_INTEGER */
    case 69: /* TYPE_UINTEGER */
    case 70: /* TYPE_LONG */
    case 71: /* TYPE_ULONG */
    case 72: /* TYPE_CHAR */
    case 73: /* TYPE_UCHAR */
    case 74: /* TYPE_DOUBLE */
    case 75: /* TYPE_BOOL */
    case 76: /* TYPE_STRING */
    case 77: /* TYPE_ARRAY */
    case 78: /* TYPE_VAR */
    case 79: /* TYPE_CALLABLE */
    case 80: /* TYPE_RESOURCE */
    case 81: /* TYPE_OBJECT */
    case 82: /* BREAK */
    case 83: /* CONTINUE */
    case 84: /* IF */
    case 85: /* ELSE */
    case 86: /* ELSEIF */
    case 87: /* SWITCH */
    case 88: /* CASE */
    case 89: /* COLON */
    case 90: /* DEFAULT */
    case 91: /* LOOP */
    case 92: /* WHILE */
    case 93: /* DO */
    case 94: /* TRY */
    case 95: /* CATCH */
    case 96: /* FOR */
    case 97: /* IN */
    case 98: /* REVERSE */
    case 99: /* LET */
    case 100: /* ADDASSIGN */
    case 101: /* SUBASSIGN */
    case 102: /* MULASSIGN */
    case 103: /* DIVASSIGN */
    case 104: /* CONCATASSIGN */
    case 105: /* MODASSIGN */
    case 106: /* STRING */
    case 107: /* DOUBLECOLON */
    case 108: /* INCR */
    case 109: /* DECR */
    case 110: /* ECHO */
    case 111: /* RETURN */
    case 112: /* UNSET */
    case 113: /* THROW */
    case 114: /* INTEGER */
    case 115: /* CHAR */
    case 116: /* DOUBLE */
    case 117: /* TRUE */
    case 118: /* FALSE */
    case 119: /* CBLOCK */
{
// 1374 "parser.lemon"

	if (status->status != XX_PARSING_FAILED) {
		//TODO:
	}
	if ((yypminor->yy0)) {
		if ((yypminor->yy0)->free_flag) {
			
		}
		delete (yypminor->yy0);
	}

// 4884 "parser.cpp"
}
      break;
    case 122: /* xx_language */
{
// 1390 "parser.lemon"
 delete (yypminor->yy396); 
// 4891 "parser.cpp"
}
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
  yy_destructor(pParser, yymajor, &yytos->minor);
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
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int xx_StackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

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
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_COUNT
   || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
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
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
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
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
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
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
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
static const struct {
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
  { 129, 8 },
  { 129, 8 },
  { 129, 9 },
  { 129, 9 },
  { 129, 9 },
  { 129, 10 },
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

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  // <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  // <lineno> <thisfile>
  **     break;
  */
      case 0: /* program ::= xx_language */
// 1386 "parser.lemon"
{
	status->ret = yymsp[0].minor.yy396;
}
// 5640 "parser.cpp"
        break;
      case 1: /* xx_language ::= xx_top_statement_list */
      case 4: /* xx_top_statement ::= xx_namespace_def */ yytestcase(yyruleno==4);
      case 5: /* xx_top_statement ::= xx_use_aliases */ yytestcase(yyruleno==5);
      case 6: /* xx_top_statement ::= xx_class_def */ yytestcase(yyruleno==6);
      case 7: /* xx_top_statement ::= xx_interface_def */ yytestcase(yyruleno==7);
      case 8: /* xx_top_statement ::= xx_function_def */ yytestcase(yyruleno==8);
      case 9: /* xx_top_statement ::= xx_comment */ yytestcase(yyruleno==9);
      case 10: /* xx_top_statement ::= xx_cblock */ yytestcase(yyruleno==10);
      case 11: /* xx_top_statement ::= xx_let_statement */ yytestcase(yyruleno==11);
      case 12: /* xx_top_statement ::= xx_if_statement */ yytestcase(yyruleno==12);
      case 13: /* xx_top_statement ::= xx_loop_statement */ yytestcase(yyruleno==13);
      case 14: /* xx_top_statement ::= xx_echo_statement */ yytestcase(yyruleno==14);
      case 15: /* xx_top_statement ::= xx_return_statement */ yytestcase(yyruleno==15);
      case 16: /* xx_top_statement ::= xx_require_statement */ yytestcase(yyruleno==16);
      case 17: /* xx_top_statement ::= xx_fetch_statement */ yytestcase(yyruleno==17);
      case 18: /* xx_top_statement ::= xx_fcall_statement */ yytestcase(yyruleno==18);
      case 19: /* xx_top_statement ::= xx_scall_statement */ yytestcase(yyruleno==19);
      case 20: /* xx_top_statement ::= xx_unset_statement */ yytestcase(yyruleno==20);
      case 21: /* xx_top_statement ::= xx_throw_statement */ yytestcase(yyruleno==21);
      case 22: /* xx_top_statement ::= xx_declare_statement */ yytestcase(yyruleno==22);
      case 23: /* xx_top_statement ::= xx_break_statement */ yytestcase(yyruleno==23);
      case 24: /* xx_top_statement ::= xx_continue_statement */ yytestcase(yyruleno==24);
      case 25: /* xx_top_statement ::= xx_while_statement */ yytestcase(yyruleno==25);
      case 26: /* xx_top_statement ::= xx_do_while_statement */ yytestcase(yyruleno==26);
      case 27: /* xx_top_statement ::= xx_try_catch_statement */ yytestcase(yyruleno==27);
      case 28: /* xx_top_statement ::= xx_switch_statement */ yytestcase(yyruleno==28);
      case 29: /* xx_top_statement ::= xx_for_statement */ yytestcase(yyruleno==29);
      case 195: /* xx_statement ::= xx_cblock */ yytestcase(yyruleno==195);
      case 196: /* xx_statement ::= xx_let_statement */ yytestcase(yyruleno==196);
      case 197: /* xx_statement ::= xx_if_statement */ yytestcase(yyruleno==197);
      case 198: /* xx_statement ::= xx_loop_statement */ yytestcase(yyruleno==198);
      case 199: /* xx_statement ::= xx_echo_statement */ yytestcase(yyruleno==199);
      case 200: /* xx_statement ::= xx_return_statement */ yytestcase(yyruleno==200);
      case 201: /* xx_statement ::= xx_require_statement */ yytestcase(yyruleno==201);
      case 202: /* xx_statement ::= xx_fetch_statement */ yytestcase(yyruleno==202);
      case 203: /* xx_statement ::= xx_fcall_statement */ yytestcase(yyruleno==203);
      case 204: /* xx_statement ::= xx_mcall_statement */ yytestcase(yyruleno==204);
      case 205: /* xx_statement ::= xx_scall_statement */ yytestcase(yyruleno==205);
      case 206: /* xx_statement ::= xx_unset_statement */ yytestcase(yyruleno==206);
      case 207: /* xx_statement ::= xx_throw_statement */ yytestcase(yyruleno==207);
      case 208: /* xx_statement ::= xx_declare_statement */ yytestcase(yyruleno==208);
      case 209: /* xx_statement ::= xx_break_statement */ yytestcase(yyruleno==209);
      case 210: /* xx_statement ::= xx_continue_statement */ yytestcase(yyruleno==210);
      case 211: /* xx_statement ::= xx_while_statement */ yytestcase(yyruleno==211);
      case 212: /* xx_statement ::= xx_do_while_statement */ yytestcase(yyruleno==212);
      case 213: /* xx_statement ::= xx_try_catch_statement */ yytestcase(yyruleno==213);
      case 214: /* xx_statement ::= xx_switch_statement */ yytestcase(yyruleno==214);
      case 215: /* xx_statement ::= xx_for_statement */ yytestcase(yyruleno==215);
      case 216: /* xx_statement ::= xx_comment */ yytestcase(yyruleno==216);
      case 217: /* xx_statement ::= xx_empty_statement */ yytestcase(yyruleno==217);
      case 302: /* xx_index_expr ::= xx_common_expr */ yytestcase(yyruleno==302);
      case 306: /* xx_echo_expression ::= xx_common_expr */ yytestcase(yyruleno==306);
      case 331: /* xx_assign_expr ::= xx_common_expr */ yytestcase(yyruleno==331);
      case 372: /* xx_common_expr ::= xx_fetch_expr */ yytestcase(yyruleno==372);
      case 407: /* xx_common_expr ::= xx_mcall_expr */ yytestcase(yyruleno==407);
      case 408: /* xx_common_expr ::= xx_scall_expr */ yytestcase(yyruleno==408);
      case 409: /* xx_common_expr ::= xx_fcall_expr */ yytestcase(yyruleno==409);
      case 429: /* xx_array_value ::= xx_common_expr */ yytestcase(yyruleno==429);
      case 448: /* xx_literal_array_value ::= xx_literal_expr */ yytestcase(yyruleno==448);
      case 449: /* xx_eval_expr ::= xx_common_expr */ yytestcase(yyruleno==449);
// 1392 "parser.lemon"
{
	yygotominor.yy396 = yymsp[0].minor.yy396;
}
// 5706 "parser.cpp"
        break;
      case 2: /* xx_top_statement_list ::= xx_top_statement_list xx_top_statement */
      case 68: /* xx_class_properties_definition ::= xx_class_properties_definition xx_class_property_definition */ yytestcase(yyruleno==68);
      case 84: /* xx_class_consts_definition ::= xx_class_consts_definition xx_class_const_definition */ yytestcase(yyruleno==84);
      case 86: /* xx_class_methods_definition ::= xx_class_methods_definition xx_class_method_definition */ yytestcase(yyruleno==86);
      case 88: /* xx_interface_methods_definition ::= xx_interface_methods_definition xx_interface_method_definition */ yytestcase(yyruleno==88);
      case 126: /* xx_visibility_list ::= xx_visibility_list xx_visibility */ yytestcase(yyruleno==126);
      case 193: /* xx_statement_list ::= xx_statement_list xx_statement */ yytestcase(yyruleno==193);
      case 232: /* xx_elseif_statements ::= xx_elseif_statements xx_elseif_statement */ yytestcase(yyruleno==232);
      case 238: /* xx_case_clauses ::= xx_case_clauses xx_case_clause */ yytestcase(yyruleno==238);
      case 252: /* xx_catch_statement_list ::= xx_catch_statement_list xx_catch_statement */ yytestcase(yyruleno==252);
      case 291: /* xx_array_offset_list ::= xx_array_offset_list xx_array_offset */ yytestcase(yyruleno==291);
// 1396 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(yymsp[-1].minor.yy396, yymsp[0].minor.yy396);
}
// 5723 "parser.cpp"
        break;
      case 3: /* xx_top_statement_list ::= xx_top_statement */
      case 33: /* xx_use_aliases_list ::= xx_use_aliases */ yytestcase(yyruleno==33);
      case 52: /* xx_implements_list ::= xx_implements */ yytestcase(yyruleno==52);
      case 69: /* xx_class_properties_definition ::= xx_class_property_definition */ yytestcase(yyruleno==69);
      case 81: /* xx_class_property_shortcuts_list ::= xx_class_property_shortcut */ yytestcase(yyruleno==81);
      case 85: /* xx_class_consts_definition ::= xx_class_const_definition */ yytestcase(yyruleno==85);
      case 87: /* xx_class_methods_definition ::= xx_class_method_definition */ yytestcase(yyruleno==87);
      case 89: /* xx_interface_methods_definition ::= xx_interface_method_definition */ yytestcase(yyruleno==89);
      case 127: /* xx_visibility_list ::= xx_visibility */ yytestcase(yyruleno==127);
      case 140: /* xx_method_return_type_list ::= xx_method_return_type_item */ yytestcase(yyruleno==140);
      case 160: /* xx_parameter_list ::= xx_parameter */ yytestcase(yyruleno==160);
      case 194: /* xx_statement_list ::= xx_statement */ yytestcase(yyruleno==194);
      case 233: /* xx_elseif_statements ::= xx_elseif_statement */ yytestcase(yyruleno==233);
      case 239: /* xx_case_clauses ::= xx_case_clause */ yytestcase(yyruleno==239);
      case 253: /* xx_catch_statement_list ::= xx_catch_statement */ yytestcase(yyruleno==253);
      case 259: /* xx_catch_classes_list ::= xx_catch_class */ yytestcase(yyruleno==259);
      case 269: /* xx_let_assignments ::= xx_let_assignment */ yytestcase(yyruleno==269);
      case 292: /* xx_array_offset_list ::= xx_array_offset */ yytestcase(yyruleno==292);
      case 305: /* xx_echo_expressions ::= xx_echo_expression */ yytestcase(yyruleno==305);
      case 328: /* xx_declare_variable_list ::= xx_declare_variable */ yytestcase(yyruleno==328);
      case 412: /* xx_call_parameters ::= xx_call_parameter */ yytestcase(yyruleno==412);
      case 422: /* xx_array_list ::= xx_array_item */ yytestcase(yyruleno==422);
      case 442: /* xx_literal_array_list ::= xx_literal_array_item */ yytestcase(yyruleno==442);
// 1400 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(NULL, yymsp[0].minor.yy396);
}
// 5752 "parser.cpp"
        break;
      case 30: /* xx_namespace_def ::= NAMESPACE IDENTIFIER DOTCOMMA */
// 1508 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_namespace(yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,43,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5761 "parser.cpp"
        break;
      case 31: /* xx_namespace_def ::= USE xx_use_aliases_list DOTCOMMA */
// 1512 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_use_aliases(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,46,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5770 "parser.cpp"
        break;
      case 32: /* xx_use_aliases_list ::= xx_use_aliases_list COMMA xx_use_aliases */
      case 51: /* xx_implements_list ::= xx_implements_list COMMA xx_implements */ yytestcase(yyruleno==51);
      case 80: /* xx_class_property_shortcuts_list ::= xx_class_property_shortcuts_list COMMA xx_class_property_shortcut */ yytestcase(yyruleno==80);
      case 159: /* xx_parameter_list ::= xx_parameter_list COMMA xx_parameter */ yytestcase(yyruleno==159);
      case 268: /* xx_let_assignments ::= xx_let_assignments COMMA xx_let_assignment */ yytestcase(yyruleno==268);
      case 304: /* xx_echo_expressions ::= xx_echo_expressions COMMA xx_echo_expression */ yytestcase(yyruleno==304);
      case 327: /* xx_declare_variable_list ::= xx_declare_variable_list COMMA xx_declare_variable */ yytestcase(yyruleno==327);
      case 411: /* xx_call_parameters ::= xx_call_parameters COMMA xx_call_parameter */ yytestcase(yyruleno==411);
      case 421: /* xx_array_list ::= xx_array_list COMMA xx_array_item */ yytestcase(yyruleno==421);
      case 441: /* xx_literal_array_list ::= xx_literal_array_list COMMA xx_literal_array_item */ yytestcase(yyruleno==441);
// 1516 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(yymsp[-2].minor.yy396, yymsp[0].minor.yy396);
  yy_destructor(yypParser,6,&yymsp[-1].minor);
}
// 5787 "parser.cpp"
        break;
      case 34: /* xx_use_aliases ::= IDENTIFIER */
// 1524 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_use_aliases_item(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 5794 "parser.cpp"
        break;
      case 35: /* xx_use_aliases ::= IDENTIFIER AS IDENTIFIER */
// 1528 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_use_aliases_item(yymsp[-2].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,47,&yymsp[-1].minor);
}
// 5802 "parser.cpp"
        break;
      case 36: /* xx_interface_def ::= INTERFACE IDENTIFIER xx_interface_body */
// 1532 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_interface(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
}
// 5810 "parser.cpp"
        break;
      case 37: /* xx_interface_def ::= INTERFACE IDENTIFIER EXTENDS IDENTIFIER xx_interface_body */
// 1536 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_interface(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,48,&yymsp[-4].minor);
  yy_destructor(yypParser,49,&yymsp[-2].minor);
}
// 5819 "parser.cpp"
        break;
      case 38: /* xx_class_def ::= CLASS IDENTIFIER xx_class_body */
// 1540 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, 0, 0, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
}
// 5827 "parser.cpp"
        break;
      case 39: /* xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body */
// 1544 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,49,&yymsp[-2].minor);
}
// 5836 "parser.cpp"
        break;
      case 40: /* xx_class_def ::= CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body */
// 1548 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 0, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
}
// 5845 "parser.cpp"
        break;
      case 41: /* xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body */
// 1552 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy396, 0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,50,&yymsp[-6].minor);
  yy_destructor(yypParser,49,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
}
// 5855 "parser.cpp"
        break;
      case 42: /* xx_class_def ::= ABSTRACT CLASS IDENTIFIER xx_class_body */
// 1556 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, 1, 0, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,52,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
}
// 5864 "parser.cpp"
        break;
      case 43: /* xx_class_def ::= ABSTRACT CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body */
// 1560 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 1, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,52,&yymsp[-5].minor);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,49,&yymsp[-2].minor);
}
// 5874 "parser.cpp"
        break;
      case 44: /* xx_class_def ::= ABSTRACT CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body */
// 1564 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 1, 0, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,52,&yymsp[-5].minor);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
}
// 5884 "parser.cpp"
        break;
      case 45: /* xx_class_def ::= ABSTRACT CLASS IDENTIFIER EXTENDS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body */
// 1568 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy396, 1, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,52,&yymsp[-7].minor);
  yy_destructor(yypParser,50,&yymsp[-6].minor);
  yy_destructor(yypParser,49,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
}
// 5895 "parser.cpp"
        break;
      case 46: /* xx_class_def ::= FINAL CLASS IDENTIFIER xx_class_body */
// 1572 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, 0, 1, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,53,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
}
// 5904 "parser.cpp"
        break;
      case 47: /* xx_class_def ::= FINAL CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body */
// 1576 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,53,&yymsp[-5].minor);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,49,&yymsp[-2].minor);
}
// 5914 "parser.cpp"
        break;
      case 48: /* xx_class_def ::= FINAL CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body */
// 1580 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 1, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,53,&yymsp[-5].minor);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
}
// 5924 "parser.cpp"
        break;
      case 49: /* xx_class_body ::= BRACKET_OPEN BRACKET_CLOSE */
      case 78: /* xx_class_property_shortcuts ::= BRACKET_OPEN BRACKET_CLOSE */ yytestcase(yyruleno==78);
// 1584 "parser.lemon"
{
	yygotominor.yy396 = NULL;
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 5934 "parser.cpp"
        break;
      case 50: /* xx_class_body ::= BRACKET_OPEN xx_class_definition BRACKET_CLOSE */
      case 79: /* xx_class_property_shortcuts ::= BRACKET_OPEN xx_class_property_shortcuts_list BRACKET_CLOSE */ yytestcase(yyruleno==79);
// 1588 "parser.lemon"
{
	yygotominor.yy396 = yymsp[-1].minor.yy396;
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 5944 "parser.cpp"
        break;
      case 53: /* xx_implements ::= IDENTIFIER */
      case 260: /* xx_catch_class ::= IDENTIFIER */ yytestcase(yyruleno==260);
      case 374: /* xx_common_expr ::= IDENTIFIER */ yytestcase(yyruleno==374);
      case 426: /* xx_array_key ::= IDENTIFIER */ yytestcase(yyruleno==426);
      case 445: /* xx_literal_array_key ::= IDENTIFIER */ yytestcase(yyruleno==445);
// 1600 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state);
}
// 5955 "parser.cpp"
        break;
      case 54: /* xx_interface_body ::= BRACKET_OPEN BRACKET_CLOSE */
// 1604 "parser.lemon"
{
  yygotominor.yy396 = NULL;
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 5964 "parser.cpp"
        break;
      case 55: /* xx_interface_body ::= BRACKET_OPEN xx_interface_definition BRACKET_CLOSE */
// 1608 "parser.lemon"
{
  yygotominor.yy396 = yymsp[-1].minor.yy396;
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 5973 "parser.cpp"
        break;
      case 56: /* xx_class_definition ::= xx_class_properties_definition */
// 1612 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
}
// 5980 "parser.cpp"
        break;
      case 57: /* xx_class_definition ::= xx_class_consts_definition */
// 1616 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 5987 "parser.cpp"
        break;
      case 58: /* xx_class_definition ::= xx_class_methods_definition */
// 1620 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(NULL, yymsp[0].minor.yy396, NULL, status->scanner_state);
}
// 5994 "parser.cpp"
        break;
      case 59: /* xx_class_definition ::= xx_class_properties_definition xx_class_methods_definition */
// 1624 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-1].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
}
// 6001 "parser.cpp"
        break;
      case 60: /* xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition */
// 1628 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-1].minor.yy396, NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 6008 "parser.cpp"
        break;
      case 61: /* xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition */
// 1632 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[0].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
}
// 6015 "parser.cpp"
        break;
      case 62: /* xx_class_definition ::= xx_class_consts_definition xx_class_methods_definition */
// 1636 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(NULL, yymsp[0].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
}
// 6022 "parser.cpp"
        break;
      case 63: /* xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition xx_class_methods_definition */
// 1640 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
}
// 6029 "parser.cpp"
        break;
      case 64: /* xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition xx_class_methods_definition */
// 1644 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-1].minor.yy396, yymsp[0].minor.yy396, yymsp[-2].minor.yy396, status->scanner_state);
}
// 6036 "parser.cpp"
        break;
      case 65: /* xx_interface_definition ::= xx_class_consts_definition */
// 1648 "parser.lemon"
{
  yygotominor.yy396 = xx_ret_interface_definition(NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 6043 "parser.cpp"
        break;
      case 66: /* xx_interface_definition ::= xx_interface_methods_definition */
// 1652 "parser.lemon"
{
  yygotominor.yy396 = xx_ret_interface_definition(yymsp[0].minor.yy396, NULL, status->scanner_state);
}
// 6050 "parser.cpp"
        break;
      case 67: /* xx_interface_definition ::= xx_class_consts_definition xx_interface_methods_definition */
// 1656 "parser.lemon"
{
  yygotominor.yy396 = xx_ret_interface_definition(yymsp[0].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
}
// 6057 "parser.cpp"
        break;
      case 70: /* xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER DOTCOMMA */
// 1669 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-2].minor.yy396, yymsp[-1].minor.yy0, NULL, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6065 "parser.cpp"
        break;
      case 71: /* xx_class_property_definition ::= xx_visibility_list IDENTIFIER DOTCOMMA */
// 1673 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-2].minor.yy396, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6073 "parser.cpp"
        break;
      case 72: /* xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA */
// 1677 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-4].minor.yy396, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6082 "parser.cpp"
        break;
      case 73: /* xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA */
// 1681 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-4].minor.yy396, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6091 "parser.cpp"
        break;
      case 74: /* xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA */
// 1685 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, NULL, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6099 "parser.cpp"
        break;
      case 75: /* xx_class_property_definition ::= xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA */
// 1689 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6107 "parser.cpp"
        break;
      case 76: /* xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA */
// 1693 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-5].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, yymsp[-6].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-3].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6116 "parser.cpp"
        break;
      case 77: /* xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA */
// 1697 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-5].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-3].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6125 "parser.cpp"
        break;
      case 82: /* xx_class_property_shortcut ::= IDENTIFIER */
// 1717 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_property_shortcut(NULL, yymsp[0].minor.yy0, status->scanner_state);
}
// 6132 "parser.cpp"
        break;
      case 83: /* xx_class_property_shortcut ::= COMMENT IDENTIFIER */
// 1721 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_property_shortcut(yymsp[-1].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
}
// 6139 "parser.cpp"
        break;
      case 90: /* xx_class_const_definition ::= COMMENT CONST CONSTANT ASSIGN xx_literal_expr DOTCOMMA */
      case 92: /* xx_class_const_definition ::= COMMENT CONST IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA */ yytestcase(yyruleno==92);
// 1750 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-4].minor);
  yy_destructor(yypParser,57,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6150 "parser.cpp"
        break;
      case 91: /* xx_class_const_definition ::= CONST CONSTANT ASSIGN xx_literal_expr DOTCOMMA */
      case 93: /* xx_class_const_definition ::= CONST IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA */ yytestcase(yyruleno==93);
// 1754 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-4].minor);
  yy_destructor(yypParser,57,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6161 "parser.cpp"
        break;
      case 94: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 1770 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-6].minor.yy396, yymsp[-4].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-5].minor);
  yy_destructor(yypParser,61,&yymsp[-3].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6173 "parser.cpp"
        break;
      case 95: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA */
      case 122: /* xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA */ yytestcase(yyruleno==122);
// 1775 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-5].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6185 "parser.cpp"
        break;
      case 96: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 1780 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6197 "parser.cpp"
        break;
      case 97: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA */
      case 123: /* xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA */ yytestcase(yyruleno==123);
// 1785 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-6].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-5].minor);
  yy_destructor(yypParser,61,&yymsp[-3].minor);
  yy_destructor(yypParser,40,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6209 "parser.cpp"
        break;
      case 98: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 1790 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6221 "parser.cpp"
        break;
      case 99: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 1794 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6233 "parser.cpp"
        break;
      case 100: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 1798 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-6].minor.yy396, yymsp[-4].minor.yy0, NULL, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-5].minor);
  yy_destructor(yypParser,61,&yymsp[-3].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6245 "parser.cpp"
        break;
      case 101: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA */
      case 124: /* xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA */ yytestcase(yyruleno==124);
// 1802 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-5].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, yymsp[-6].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6257 "parser.cpp"
        break;
      case 102: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 1806 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6269 "parser.cpp"
        break;
      case 103: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA */
      case 125: /* xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA */ yytestcase(yyruleno==125);
// 1810 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-6].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-5].minor);
  yy_destructor(yypParser,61,&yymsp[-3].minor);
  yy_destructor(yypParser,40,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6281 "parser.cpp"
        break;
      case 104: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 1814 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6293 "parser.cpp"
        break;
      case 105: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 1818 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, yymsp[-9].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6305 "parser.cpp"
        break;
      case 106: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE */
// 1822 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, NULL, NULL, NULL, yymsp[-2].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-4].minor);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6318 "parser.cpp"
        break;
      case 107: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA */
      case 118: /* xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA */ yytestcase(yyruleno==118);
// 1826 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,42,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6331 "parser.cpp"
        break;
      case 108: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE */
// 1830 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-9].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy396, NULL, NULL, yymsp[-2].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-8].minor);
  yy_destructor(yypParser,61,&yymsp[-6].minor);
  yy_destructor(yypParser,40,&yymsp[-4].minor);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6344 "parser.cpp"
        break;
      case 109: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA */
      case 119: /* xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA */ yytestcase(yyruleno==119);
// 1834 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,42,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6357 "parser.cpp"
        break;
      case 110: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 1838 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-9].minor.yy396, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy396, NULL, yymsp[-3].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-8].minor);
  yy_destructor(yypParser,61,&yymsp[-6].minor);
  yy_destructor(yypParser,40,&yymsp[-5].minor);
  yy_destructor(yypParser,42,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6370 "parser.cpp"
        break;
      case 111: /* xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 1842 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-10].minor.yy396, yymsp[-8].minor.yy0, yymsp[-6].minor.yy396, yymsp[-1].minor.yy396, NULL, yymsp[-3].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-9].minor);
  yy_destructor(yypParser,61,&yymsp[-7].minor);
  yy_destructor(yypParser,40,&yymsp[-5].minor);
  yy_destructor(yypParser,42,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6383 "parser.cpp"
        break;
      case 112: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE */
// 1846 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, NULL, NULL, yymsp[-9].minor.yy0, yymsp[-2].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-4].minor);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6396 "parser.cpp"
        break;
      case 113: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA */
      case 120: /* xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA */ yytestcase(yyruleno==120);
// 1850 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, NULL, yymsp[-8].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,42,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6409 "parser.cpp"
        break;
      case 114: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE */
// 1854 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-9].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy396, NULL, yymsp[-10].minor.yy0, yymsp[-2].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-8].minor);
  yy_destructor(yypParser,61,&yymsp[-6].minor);
  yy_destructor(yypParser,40,&yymsp[-4].minor);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6422 "parser.cpp"
        break;
      case 115: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA */
      case 121: /* xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA */ yytestcase(yyruleno==121);
// 1858 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, NULL, yymsp[-9].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,42,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6435 "parser.cpp"
        break;
      case 116: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 1862 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-9].minor.yy396, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy396, yymsp[-10].minor.yy0, yymsp[-3].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-8].minor);
  yy_destructor(yypParser,61,&yymsp[-6].minor);
  yy_destructor(yypParser,40,&yymsp[-5].minor);
  yy_destructor(yypParser,42,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6448 "parser.cpp"
        break;
      case 117: /* xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 1866 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_method(yymsp[-10].minor.yy396, yymsp[-8].minor.yy0, yymsp[-6].minor.yy396, yymsp[-1].minor.yy396, yymsp[-11].minor.yy0, yymsp[-3].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-9].minor);
  yy_destructor(yypParser,61,&yymsp[-7].minor);
  yy_destructor(yypParser,40,&yymsp[-5].minor);
  yy_destructor(yypParser,42,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6461 "parser.cpp"
        break;
      case 128: /* xx_visibility ::= PUBLIC */
// 1912 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("public");
  yy_destructor(yypParser,1,&yymsp[0].minor);
}
// 6469 "parser.cpp"
        break;
      case 129: /* xx_visibility ::= PROTECTED */
// 1916 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("protected");
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
// 6477 "parser.cpp"
        break;
      case 130: /* xx_visibility ::= PRIVATE */
// 1920 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("private");
  yy_destructor(yypParser,4,&yymsp[0].minor);
}
// 6485 "parser.cpp"
        break;
      case 131: /* xx_visibility ::= STATIC */
// 1924 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("static");
  yy_destructor(yypParser,3,&yymsp[0].minor);
}
// 6493 "parser.cpp"
        break;
      case 132: /* xx_visibility ::= SCOPED */
// 1928 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("scoped");
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
// 6501 "parser.cpp"
        break;
      case 133: /* xx_visibility ::= INLINE */
// 1932 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("inline");
  yy_destructor(yypParser,62,&yymsp[0].minor);
}
// 6509 "parser.cpp"
        break;
      case 134: /* xx_visibility ::= DEPRECATED */
// 1936 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("deprecated");
  yy_destructor(yypParser,63,&yymsp[0].minor);
}
// 6517 "parser.cpp"
        break;
      case 135: /* xx_visibility ::= ABSTRACT */
// 1940 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("abstract");
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
// 6525 "parser.cpp"
        break;
      case 136: /* xx_visibility ::= FINAL */
// 1944 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("final");
  yy_destructor(yypParser,53,&yymsp[0].minor);
}
// 6533 "parser.cpp"
        break;
      case 137: /* xx_method_return_type ::= VOID */
// 1949 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type(1, NULL, status->scanner_state);
  yy_destructor(yypParser,64,&yymsp[0].minor);
}
// 6541 "parser.cpp"
        break;
      case 138: /* xx_method_return_type ::= xx_method_return_type_list */
// 1953 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type(0, yymsp[0].minor.yy396, status->scanner_state);
}
// 6548 "parser.cpp"
        break;
      case 139: /* xx_method_return_type_list ::= xx_method_return_type_list BITWISE_OR xx_method_return_type_item */
      case 258: /* xx_catch_classes_list ::= xx_catch_classes_list BITWISE_OR xx_catch_class */ yytestcase(yyruleno==258);
// 1957 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(yymsp[-2].minor.yy396, yymsp[0].minor.yy396);
  yy_destructor(yypParser,14,&yymsp[-1].minor);
}
// 6557 "parser.cpp"
        break;
      case 141: /* xx_method_return_type_item ::= xx_parameter_type */
// 1965 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(yymsp[0].minor.yy396, NULL, 0, 0, status->scanner_state);
}
// 6564 "parser.cpp"
        break;
      case 142: /* xx_method_return_type_item ::= NULL */
// 1969 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_NULL), NULL, 0, 0, status->scanner_state);
  yy_destructor(yypParser,65,&yymsp[0].minor);
}
// 6572 "parser.cpp"
        break;
      case 143: /* xx_method_return_type_item ::= THIS */
// 1973 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_THIS), NULL, 0, 0, status->scanner_state);
  yy_destructor(yypParser,66,&yymsp[0].minor);
}
// 6580 "parser.cpp"
        break;
      case 144: /* xx_method_return_type_item ::= xx_parameter_type NOT */
// 1977 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(yymsp[-1].minor.yy396, NULL, 1, 0, status->scanner_state);
  yy_destructor(yypParser,39,&yymsp[0].minor);
}
// 6588 "parser.cpp"
        break;
      case 145: /* xx_method_return_type_item ::= xx_parameter_cast */
// 1981 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy396, 0, 0, status->scanner_state);
}
// 6595 "parser.cpp"
        break;
      case 146: /* xx_method_return_type_item ::= xx_parameter_cast_collection */
// 1985 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy396, 0, 1, status->scanner_state);
}
// 6602 "parser.cpp"
        break;
      case 147: /* xx_function_def ::= FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 1992 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(NULL, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6614 "parser.cpp"
        break;
      case 148: /* xx_function_def ::= FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 1997 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(NULL, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6626 "parser.cpp"
        break;
      case 149: /* xx_function_def ::= FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2002 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(NULL, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6638 "parser.cpp"
        break;
      case 150: /* xx_function_def ::= xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2007 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6650 "parser.cpp"
        break;
      case 151: /* xx_function_def ::= xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 2012 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6662 "parser.cpp"
        break;
      case 152: /* xx_function_def ::= xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2017 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6674 "parser.cpp"
        break;
      case 153: /* xx_function_def ::= COMMENT FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2021 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(NULL, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, yymsp[-7].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6686 "parser.cpp"
        break;
      case 154: /* xx_function_def ::= COMMENT FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 2025 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(NULL, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, yymsp[-7].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6698 "parser.cpp"
        break;
      case 155: /* xx_function_def ::= COMMENT FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2029 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(NULL, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, yymsp[-8].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6710 "parser.cpp"
        break;
      case 156: /* xx_function_def ::= COMMENT xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2033 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy396, yymsp[-8].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6722 "parser.cpp"
        break;
      case 157: /* xx_function_def ::= COMMENT xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 2037 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(yymsp[-7].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy396, NULL, yymsp[-8].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6734 "parser.cpp"
        break;
      case 158: /* xx_function_def ::= COMMENT xx_parameter_type FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2041 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_function_definition(yymsp[-8].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, yymsp[-9].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-7].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6746 "parser.cpp"
        break;
      case 161: /* xx_parameter ::= IDENTIFIER */
// 2055 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6753 "parser.cpp"
        break;
      case 162: /* xx_parameter ::= CONST IDENTIFIER */
// 2059 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-1].minor);
}
// 6761 "parser.cpp"
        break;
      case 163: /* xx_parameter ::= xx_parameter_type IDENTIFIER */
// 2063 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-1].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6768 "parser.cpp"
        break;
      case 164: /* xx_parameter ::= CONST xx_parameter_type IDENTIFIER */
// 2067 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-1].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-2].minor);
}
// 6776 "parser.cpp"
        break;
      case 165: /* xx_parameter ::= xx_parameter_type NOT IDENTIFIER */
// 2071 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-2].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
}
// 6784 "parser.cpp"
        break;
      case 166: /* xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER */
// 2075 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-2].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-3].minor);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
}
// 6793 "parser.cpp"
        break;
      case 167: /* xx_parameter ::= xx_parameter_cast IDENTIFIER */
// 2079 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, yymsp[-1].minor.yy396, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6800 "parser.cpp"
        break;
      case 168: /* xx_parameter ::= CONST xx_parameter_cast IDENTIFIER */
// 2083 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, yymsp[-1].minor.yy396, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-2].minor);
}
// 6808 "parser.cpp"
        break;
      case 169: /* xx_parameter ::= IDENTIFIER ASSIGN xx_literal_expr */
// 2087 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6816 "parser.cpp"
        break;
      case 170: /* xx_parameter ::= CONST IDENTIFIER ASSIGN xx_literal_expr */
// 2091 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-3].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6825 "parser.cpp"
        break;
      case 171: /* xx_parameter ::= xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr */
// 2095 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-3].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6833 "parser.cpp"
        break;
      case 172: /* xx_parameter ::= CONST xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr */
// 2099 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-3].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-4].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6842 "parser.cpp"
        break;
      case 173: /* xx_parameter ::= xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr */
// 2103 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-4].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 1, status->scanner_state);
  yy_destructor(yypParser,39,&yymsp[-3].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6851 "parser.cpp"
        break;
      case 174: /* xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr */
// 2107 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-4].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 1, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-5].minor);
  yy_destructor(yypParser,39,&yymsp[-3].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6861 "parser.cpp"
        break;
      case 175: /* xx_parameter ::= xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr */
// 2111 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6869 "parser.cpp"
        break;
      case 176: /* xx_parameter ::= CONST xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr */
// 2115 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-4].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6878 "parser.cpp"
        break;
      case 177: /* xx_parameter_cast ::= LESS IDENTIFIER GREATER */
// 2120 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,21,&yymsp[-2].minor);
  yy_destructor(yypParser,22,&yymsp[0].minor);
}
// 6887 "parser.cpp"
        break;
      case 178: /* xx_parameter_cast_collection ::= LESS IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE GREATER */
// 2124 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,21,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yy_destructor(yypParser,67,&yymsp[-1].minor);
  yy_destructor(yypParser,22,&yymsp[0].minor);
}
// 6898 "parser.cpp"
        break;
      case 179: /* xx_parameter_type ::= TYPE_INTEGER */
// 2128 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_INTEGER);
  yy_destructor(yypParser,68,&yymsp[0].minor);
}
// 6906 "parser.cpp"
        break;
      case 180: /* xx_parameter_type ::= TYPE_UINTEGER */
// 2132 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_UINTEGER);
  yy_destructor(yypParser,69,&yymsp[0].minor);
}
// 6914 "parser.cpp"
        break;
      case 181: /* xx_parameter_type ::= TYPE_LONG */
// 2136 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_LONG);
  yy_destructor(yypParser,70,&yymsp[0].minor);
}
// 6922 "parser.cpp"
        break;
      case 182: /* xx_parameter_type ::= TYPE_ULONG */
// 2140 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_ULONG);
  yy_destructor(yypParser,71,&yymsp[0].minor);
}
// 6930 "parser.cpp"
        break;
      case 183: /* xx_parameter_type ::= TYPE_CHAR */
// 2144 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_CHAR);
  yy_destructor(yypParser,72,&yymsp[0].minor);
}
// 6938 "parser.cpp"
        break;
      case 184: /* xx_parameter_type ::= TYPE_UCHAR */
// 2148 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_UCHAR);
  yy_destructor(yypParser,73,&yymsp[0].minor);
}
// 6946 "parser.cpp"
        break;
      case 185: /* xx_parameter_type ::= TYPE_DOUBLE */
// 2152 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_DOUBLE);
  yy_destructor(yypParser,74,&yymsp[0].minor);
}
// 6954 "parser.cpp"
        break;
      case 186: /* xx_parameter_type ::= TYPE_BOOL */
// 2156 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_BOOL);
  yy_destructor(yypParser,75,&yymsp[0].minor);
}
// 6962 "parser.cpp"
        break;
      case 187: /* xx_parameter_type ::= TYPE_STRING */
// 2160 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_STRING);
  yy_destructor(yypParser,76,&yymsp[0].minor);
}
// 6970 "parser.cpp"
        break;
      case 188: /* xx_parameter_type ::= TYPE_ARRAY */
// 2164 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_ARRAY);
  yy_destructor(yypParser,77,&yymsp[0].minor);
}
// 6978 "parser.cpp"
        break;
      case 189: /* xx_parameter_type ::= TYPE_VAR */
// 2168 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_VAR);
  yy_destructor(yypParser,78,&yymsp[0].minor);
}
// 6986 "parser.cpp"
        break;
      case 190: /* xx_parameter_type ::= TYPE_CALLABLE */
// 2172 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_CALLABLE);
  yy_destructor(yypParser,79,&yymsp[0].minor);
}
// 6994 "parser.cpp"
        break;
      case 191: /* xx_parameter_type ::= TYPE_RESOURCE */
// 2176 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_RESOURCE);
  yy_destructor(yypParser,80,&yymsp[0].minor);
}
// 7002 "parser.cpp"
        break;
      case 192: /* xx_parameter_type ::= TYPE_OBJECT */
// 2180 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_OBJECT);
  yy_destructor(yypParser,81,&yymsp[0].minor);
}
// 7010 "parser.cpp"
        break;
      case 218: /* xx_empty_statement ::= DOTCOMMA */
// 2284 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_empty_statement(status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7018 "parser.cpp"
        break;
      case 219: /* xx_break_statement ::= BREAK DOTCOMMA */
// 2288 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_break_statement(status->scanner_state);
  yy_destructor(yypParser,82,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7027 "parser.cpp"
        break;
      case 220: /* xx_continue_statement ::= CONTINUE DOTCOMMA */
// 2292 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_continue_statement(status->scanner_state);
  yy_destructor(yypParser,83,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7036 "parser.cpp"
        break;
      case 221: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE */
// 2297 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-2].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7046 "parser.cpp"
        break;
      case 222: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements */
// 2302 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-3].minor.yy396, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[-1].minor);
}
// 7056 "parser.cpp"
        break;
      case 223: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE */
// 2307 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-5].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,85,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7069 "parser.cpp"
        break;
      case 224: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE */
// 2312 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-6].minor.yy396, NULL, yymsp[-3].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-7].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,85,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7082 "parser.cpp"
        break;
      case 225: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2317 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7092 "parser.cpp"
        break;
      case 226: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements */
// 2322 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-4].minor.yy396, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-3].minor);
  yy_destructor(yypParser,55,&yymsp[-1].minor);
}
// 7102 "parser.cpp"
        break;
      case 227: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2327 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-7].minor.yy396, yymsp[-5].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-8].minor);
  yy_destructor(yypParser,54,&yymsp[-6].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,85,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7115 "parser.cpp"
        break;
      case 228: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2332 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-8].minor.yy396, yymsp[-6].minor.yy396, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-9].minor);
  yy_destructor(yypParser,54,&yymsp[-7].minor);
  yy_destructor(yypParser,55,&yymsp[-5].minor);
  yy_destructor(yypParser,85,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7128 "parser.cpp"
        break;
      case 229: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE */
// 2337 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-6].minor.yy396, yymsp[-4].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-7].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,85,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7141 "parser.cpp"
        break;
      case 230: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE */
// 2342 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-7].minor.yy396, yymsp[-5].minor.yy396, yymsp[-3].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-8].minor);
  yy_destructor(yypParser,54,&yymsp[-6].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,85,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7154 "parser.cpp"
        break;
      case 231: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2347 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-6].minor.yy396, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-7].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,85,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7167 "parser.cpp"
        break;
      case 234: /* xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE */
// 2360 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-2].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,86,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7177 "parser.cpp"
        break;
      case 235: /* xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2365 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,86,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7187 "parser.cpp"
        break;
      case 236: /* xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN BRACKET_CLOSE */
// 2369 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_switch_statement(yymsp[-2].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,87,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7197 "parser.cpp"
        break;
      case 237: /* xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN xx_case_clauses BRACKET_CLOSE */
// 2373 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_switch_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,87,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7207 "parser.cpp"
        break;
      case 240: /* xx_case_clause ::= CASE xx_literal_expr COLON */
// 2385 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_case_clause(yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,88,&yymsp[-2].minor);
  yy_destructor(yypParser,89,&yymsp[0].minor);
}
// 7216 "parser.cpp"
        break;
      case 241: /* xx_case_clause ::= CASE xx_literal_expr COLON xx_statement_list */
// 2389 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_case_clause(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,88,&yymsp[-3].minor);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 7225 "parser.cpp"
        break;
      case 242: /* xx_case_clause ::= DEFAULT COLON xx_statement_list */
// 2393 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_case_clause(NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,90,&yymsp[-2].minor);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 7234 "parser.cpp"
        break;
      case 243: /* xx_loop_statement ::= LOOP BRACKET_OPEN BRACKET_CLOSE */
// 2397 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_loop_statement(NULL, status->scanner_state);
  yy_destructor(yypParser,91,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7244 "parser.cpp"
        break;
      case 244: /* xx_loop_statement ::= LOOP BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2401 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_loop_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,91,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7254 "parser.cpp"
        break;
      case 245: /* xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN BRACKET_CLOSE */
// 2405 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_while_statement(yymsp[-2].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,92,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7264 "parser.cpp"
        break;
      case 246: /* xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2409 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_while_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,92,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7274 "parser.cpp"
        break;
      case 247: /* xx_do_while_statement ::= DO BRACKET_OPEN BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA */
// 2413 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_do_while_statement(yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,93,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,92,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7286 "parser.cpp"
        break;
      case 248: /* xx_do_while_statement ::= DO BRACKET_OPEN xx_statement_list BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA */
// 2417 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_do_while_statement(yymsp[-1].minor.yy396, yymsp[-4].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,93,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,92,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7298 "parser.cpp"
        break;
      case 249: /* xx_try_catch_statement ::= TRY BRACKET_OPEN BRACKET_CLOSE */
// 2421 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_try_catch_statement(NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,94,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7308 "parser.cpp"
        break;
      case 250: /* xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2425 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_try_catch_statement(yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,94,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7318 "parser.cpp"
        break;
      case 251: /* xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_catch_statement_list */
// 2429 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_try_catch_statement(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,94,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-3].minor);
  yy_destructor(yypParser,55,&yymsp[-1].minor);
}
// 7328 "parser.cpp"
        break;
      case 254: /* xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2441 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-3].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,95,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7338 "parser.cpp"
        break;
      case 255: /* xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN BRACKET_CLOSE */
// 2445 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-2].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,95,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7348 "parser.cpp"
        break;
      case 256: /* xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN BRACKET_CLOSE */
// 2449 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-4].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,95,&yymsp[-5].minor);
  yy_destructor(yypParser,6,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7359 "parser.cpp"
        break;
      case 257: /* xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2453 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-5].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state), yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,95,&yymsp[-6].minor);
  yy_destructor(yypParser,6,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7370 "parser.cpp"
        break;
      case 261: /* xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2469 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, NULL, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-6].minor);
  yy_destructor(yypParser,97,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7381 "parser.cpp"
        break;
      case 262: /* xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE */
// 2473 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-2].minor.yy396, NULL, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-5].minor);
  yy_destructor(yypParser,97,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7392 "parser.cpp"
        break;
      case 263: /* xx_for_statement ::= FOR IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2477 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, NULL, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-7].minor);
  yy_destructor(yypParser,97,&yymsp[-5].minor);
  yy_destructor(yypParser,98,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7404 "parser.cpp"
        break;
      case 264: /* xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2481 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-8].minor);
  yy_destructor(yypParser,6,&yymsp[-6].minor);
  yy_destructor(yypParser,97,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7416 "parser.cpp"
        break;
      case 265: /* xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE */
// 2485 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-2].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-7].minor);
  yy_destructor(yypParser,6,&yymsp[-5].minor);
  yy_destructor(yypParser,97,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7428 "parser.cpp"
        break;
      case 266: /* xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2489 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, yymsp[-8].minor.yy0, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-9].minor);
  yy_destructor(yypParser,6,&yymsp[-7].minor);
  yy_destructor(yypParser,97,&yymsp[-5].minor);
  yy_destructor(yypParser,98,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7441 "parser.cpp"
        break;
      case 267: /* xx_let_statement ::= LET xx_let_assignments DOTCOMMA */
// 2493 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,99,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7450 "parser.cpp"
        break;
      case 270: /* xx_assignment_operator ::= ASSIGN */
// 2506 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("assign");
  yy_destructor(yypParser,57,&yymsp[0].minor);
}
// 7458 "parser.cpp"
        break;
      case 271: /* xx_assignment_operator ::= ADDASSIGN */
// 2511 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("add-assign");
  yy_destructor(yypParser,100,&yymsp[0].minor);
}
// 7466 "parser.cpp"
        break;
      case 272: /* xx_assignment_operator ::= SUBASSIGN */
// 2516 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("sub-assign");
  yy_destructor(yypParser,101,&yymsp[0].minor);
}
// 7474 "parser.cpp"
        break;
      case 273: /* xx_assignment_operator ::= MULASSIGN */
// 2520 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("mul-assign");
  yy_destructor(yypParser,102,&yymsp[0].minor);
}
// 7482 "parser.cpp"
        break;
      case 274: /* xx_assignment_operator ::= DIVASSIGN */
// 2524 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("div-assign");
  yy_destructor(yypParser,103,&yymsp[0].minor);
}
// 7490 "parser.cpp"
        break;
      case 275: /* xx_assignment_operator ::= CONCATASSIGN */
// 2528 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("concat-assign");
  yy_destructor(yypParser,104,&yymsp[0].minor);
}
// 7498 "parser.cpp"
        break;
      case 276: /* xx_assignment_operator ::= MODASSIGN */
// 2532 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("mod-assign");
  yy_destructor(yypParser,105,&yymsp[0].minor);
}
// 7506 "parser.cpp"
        break;
      case 277: /* xx_let_assignment ::= IDENTIFIER xx_assignment_operator xx_assign_expr */
// 2537 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("variable", yymsp[-1].minor.yy396, yymsp[-2].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 7513 "parser.cpp"
        break;
      case 278: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_assignment_operator xx_assign_expr */
// 2542 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property", yymsp[-1].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
}
// 7521 "parser.cpp"
        break;
      case 279: /* xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2547 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("variable-dynamic-object-property", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
}
// 7531 "parser.cpp"
        break;
      case 280: /* xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2552 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("string-dynamic-object-property", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
}
// 7541 "parser.cpp"
        break;
      case 281: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2557 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-append", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7551 "parser.cpp"
        break;
      case 282: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr */
// 2562 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-array-index", yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-4].minor);
}
// 7559 "parser.cpp"
        break;
      case 283: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2566 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-array-index-append", yymsp[-1].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-6].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7569 "parser.cpp"
        break;
      case 284: /* xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_assignment_operator xx_assign_expr */
// 2571 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property", yymsp[-1].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-3].minor);
}
// 7577 "parser.cpp"
        break;
      case 285: /* xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2576 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property-append", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-5].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7587 "parser.cpp"
        break;
      case 286: /* xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr */
// 2581 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property-array-index", yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-4].minor);
}
// 7595 "parser.cpp"
        break;
      case 287: /* xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2586 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property-array-index-append", yymsp[-1].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-6].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7605 "parser.cpp"
        break;
      case 288: /* xx_let_assignment ::= IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2591 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("variable-append", yymsp[-1].minor.yy396, yymsp[-4].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7614 "parser.cpp"
        break;
      case 289: /* xx_let_assignment ::= IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr */
// 2596 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("array-index", yymsp[-1].minor.yy396, yymsp[-3].minor.yy0, NULL, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
}
// 7621 "parser.cpp"
        break;
      case 290: /* xx_let_assignment ::= IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2601 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("array-index-append", yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-4].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7630 "parser.cpp"
        break;
      case 293: /* xx_array_offset ::= SBRACKET_OPEN xx_index_expr SBRACKET_CLOSE */
// 2613 "parser.lemon"
{
	yygotominor.yy396 = yymsp[-1].minor.yy396;
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yy_destructor(yypParser,67,&yymsp[0].minor);
}
// 7639 "parser.cpp"
        break;
      case 294: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER INCR */
// 2618 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-incr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-2].minor);
  yy_destructor(yypParser,108,&yymsp[0].minor);
}
// 7648 "parser.cpp"
        break;
      case 295: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER DECR */
// 2623 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-decr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-2].minor);
  yy_destructor(yypParser,109,&yymsp[0].minor);
}
// 7657 "parser.cpp"
        break;
      case 296: /* xx_let_assignment ::= IDENTIFIER INCR */
// 2628 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("incr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,108,&yymsp[0].minor);
}
// 7665 "parser.cpp"
        break;
      case 297: /* xx_let_assignment ::= INCR IDENTIFIER */
// 2633 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("incr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,108,&yymsp[-1].minor);
}
// 7673 "parser.cpp"
        break;
      case 298: /* xx_let_assignment ::= IDENTIFIER DECR */
// 2638 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("decr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,109,&yymsp[0].minor);
}
// 7681 "parser.cpp"
        break;
      case 299: /* xx_let_assignment ::= DECR IDENTIFIER */
// 2643 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("decr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,109,&yymsp[-1].minor);
}
// 7689 "parser.cpp"
        break;
      case 300: /* xx_let_assignment ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2648 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("dynamic-variable", yymsp[-1].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
}
// 7698 "parser.cpp"
        break;
      case 301: /* xx_let_assignment ::= BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2653 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("dynamic-variable-string", yymsp[-1].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
}
// 7707 "parser.cpp"
        break;
      case 303: /* xx_echo_statement ::= ECHO xx_echo_expressions DOTCOMMA */
// 2661 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_echo_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,110,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7716 "parser.cpp"
        break;
      case 307: /* xx_mcall_statement ::= xx_mcall_expr DOTCOMMA */
// 2678 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7724 "parser.cpp"
        break;
      case 308: /* xx_fcall_statement ::= xx_fcall_expr DOTCOMMA */
// 2683 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7732 "parser.cpp"
        break;
      case 309: /* xx_scall_statement ::= xx_scall_expr DOTCOMMA */
// 2688 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7740 "parser.cpp"
        break;
      case 310: /* xx_fetch_statement ::= xx_fetch_expr DOTCOMMA */
// 2693 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fetch_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7748 "parser.cpp"
        break;
      case 311: /* xx_return_statement ::= RETURN xx_common_expr DOTCOMMA */
// 2698 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,111,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7757 "parser.cpp"
        break;
      case 312: /* xx_return_statement ::= RETURN DOTCOMMA */
// 2703 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_statement(NULL, status->scanner_state);
  yy_destructor(yypParser,111,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7766 "parser.cpp"
        break;
      case 313: /* xx_require_statement ::= REQUIRE xx_common_expr DOTCOMMA */
// 2708 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_require_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,7,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7775 "parser.cpp"
        break;
      case 314: /* xx_unset_statement ::= UNSET xx_common_expr DOTCOMMA */
// 2713 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_unset_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,112,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7784 "parser.cpp"
        break;
      case 315: /* xx_throw_statement ::= THROW xx_common_expr DOTCOMMA */
// 2718 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_throw_exception(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,113,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7793 "parser.cpp"
        break;
      case 316: /* xx_declare_statement ::= TYPE_INTEGER xx_declare_variable_list DOTCOMMA */
// 2722 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_INTEGER, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,68,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7802 "parser.cpp"
        break;
      case 317: /* xx_declare_statement ::= TYPE_UINTEGER xx_declare_variable_list DOTCOMMA */
// 2726 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_UINTEGER, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,69,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7811 "parser.cpp"
        break;
      case 318: /* xx_declare_statement ::= TYPE_CHAR xx_declare_variable_list DOTCOMMA */
// 2730 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_CHAR, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,72,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7820 "parser.cpp"
        break;
      case 319: /* xx_declare_statement ::= TYPE_UCHAR xx_declare_variable_list DOTCOMMA */
// 2734 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_UCHAR, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,73,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7829 "parser.cpp"
        break;
      case 320: /* xx_declare_statement ::= TYPE_LONG xx_declare_variable_list DOTCOMMA */
// 2738 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_LONG, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,70,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7838 "parser.cpp"
        break;
      case 321: /* xx_declare_statement ::= TYPE_ULONG xx_declare_variable_list DOTCOMMA */
// 2742 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_ULONG, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,71,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7847 "parser.cpp"
        break;
      case 322: /* xx_declare_statement ::= TYPE_DOUBLE xx_declare_variable_list DOTCOMMA */
// 2746 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_DOUBLE, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,74,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7856 "parser.cpp"
        break;
      case 323: /* xx_declare_statement ::= TYPE_STRING xx_declare_variable_list DOTCOMMA */
// 2750 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_STRING, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,76,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7865 "parser.cpp"
        break;
      case 324: /* xx_declare_statement ::= TYPE_BOOL xx_declare_variable_list DOTCOMMA */
// 2754 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_BOOL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,75,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7874 "parser.cpp"
        break;
      case 325: /* xx_declare_statement ::= TYPE_VAR xx_declare_variable_list DOTCOMMA */
// 2758 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_VAR, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,78,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7883 "parser.cpp"
        break;
      case 326: /* xx_declare_statement ::= TYPE_ARRAY xx_declare_variable_list DOTCOMMA */
// 2762 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_ARRAY, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,77,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7892 "parser.cpp"
        break;
      case 329: /* xx_declare_variable ::= IDENTIFIER */
// 2774 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_variable(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 7899 "parser.cpp"
        break;
      case 330: /* xx_declare_variable ::= IDENTIFIER ASSIGN xx_literal_expr */
// 2778 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_variable(yymsp[-2].minor.yy0, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 7907 "parser.cpp"
        break;
      case 332: /* xx_common_expr ::= NOT xx_common_expr */
// 2786 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("not", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
}
// 7915 "parser.cpp"
        break;
      case 333: /* xx_common_expr ::= SUB xx_common_expr */
// 2790 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("minus", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,28,&yymsp[-1].minor);
}
// 7923 "parser.cpp"
        break;
      case 334: /* xx_common_expr ::= ISSET xx_common_expr */
// 2794 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("isset", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,33,&yymsp[-1].minor);
}
// 7931 "parser.cpp"
        break;
      case 335: /* xx_common_expr ::= REQUIRE xx_common_expr */
// 2798 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("require", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,7,&yymsp[-1].minor);
}
// 7939 "parser.cpp"
        break;
      case 336: /* xx_common_expr ::= CLONE xx_common_expr */
// 2802 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("clone", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,37,&yymsp[-1].minor);
}
// 7947 "parser.cpp"
        break;
      case 337: /* xx_common_expr ::= EMPTY xx_common_expr */
// 2806 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("empty", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,35,&yymsp[-1].minor);
}
// 7955 "parser.cpp"
        break;
      case 338: /* xx_common_expr ::= LIKELY xx_common_expr */
// 2810 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("likely", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,9,&yymsp[-1].minor);
}
// 7963 "parser.cpp"
        break;
      case 339: /* xx_common_expr ::= UNLIKELY xx_common_expr */
// 2814 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("unlikely", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,10,&yymsp[-1].minor);
}
// 7971 "parser.cpp"
        break;
      case 340: /* xx_common_expr ::= xx_common_expr EQUALS xx_common_expr */
// 2818 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("equals", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,19,&yymsp[-1].minor);
}
// 7979 "parser.cpp"
        break;
      case 341: /* xx_common_expr ::= xx_common_expr NOTEQUALS xx_common_expr */
// 2822 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("not-equals", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,26,&yymsp[-1].minor);
}
// 7987 "parser.cpp"
        break;
      case 342: /* xx_common_expr ::= xx_common_expr IDENTICAL xx_common_expr */
// 2826 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("identical", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,20,&yymsp[-1].minor);
}
// 7995 "parser.cpp"
        break;
      case 343: /* xx_common_expr ::= xx_common_expr NOTIDENTICAL xx_common_expr */
// 2830 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("not-identical", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,25,&yymsp[-1].minor);
}
// 8003 "parser.cpp"
        break;
      case 344: /* xx_common_expr ::= xx_common_expr LESS xx_common_expr */
// 2834 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("less", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,21,&yymsp[-1].minor);
}
// 8011 "parser.cpp"
        break;
      case 345: /* xx_common_expr ::= xx_common_expr GREATER xx_common_expr */
// 2838 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("greater", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,22,&yymsp[-1].minor);
}
// 8019 "parser.cpp"
        break;
      case 346: /* xx_common_expr ::= xx_common_expr LESSEQUAL xx_common_expr */
// 2842 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("less-equal", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,23,&yymsp[-1].minor);
}
// 8027 "parser.cpp"
        break;
      case 347: /* xx_common_expr ::= xx_common_expr GREATEREQUAL xx_common_expr */
// 2846 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("greater-equal", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,24,&yymsp[-1].minor);
}
// 8035 "parser.cpp"
        break;
      case 348: /* xx_common_expr ::= PARENTHESES_OPEN xx_common_expr PARENTHESES_CLOSE */
// 2850 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("list", yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8044 "parser.cpp"
        break;
      case 349: /* xx_common_expr ::= PARENTHESES_OPEN xx_parameter_type PARENTHESES_CLOSE xx_common_expr */
// 2854 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("cast", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,61,&yymsp[-3].minor);
  yy_destructor(yypParser,40,&yymsp[-1].minor);
}
// 8053 "parser.cpp"
        break;
      case 350: /* xx_common_expr ::= LESS IDENTIFIER GREATER xx_common_expr */
// 2858 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("type-hint", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,21,&yymsp[-3].minor);
  yy_destructor(yypParser,22,&yymsp[-1].minor);
}
// 8062 "parser.cpp"
        break;
      case 351: /* xx_common_expr ::= xx_common_expr ARROW IDENTIFIER */
// 2862 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("property-access", yymsp[-2].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-1].minor);
}
// 8070 "parser.cpp"
        break;
      case 352: /* xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE */
// 2866 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("property-dynamic-access", yymsp[-4].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8080 "parser.cpp"
        break;
      case 353: /* xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE */
// 2870 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("property-string-access", yymsp[-4].minor.yy396, xx_ret_literal(XX_T_STRING, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8090 "parser.cpp"
        break;
      case 354: /* xx_common_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER */
// 2874 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("static-property-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-1].minor);
}
// 8098 "parser.cpp"
        break;
      case 355: /* xx_common_expr ::= IDENTIFIER DOUBLECOLON CONSTANT */
      case 437: /* xx_literal_expr ::= IDENTIFIER DOUBLECOLON CONSTANT */ yytestcase(yyruleno==437);
// 2878 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("static-constant-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-1].minor);
}
// 8107 "parser.cpp"
        break;
      case 356: /* xx_common_expr ::= xx_common_expr SBRACKET_OPEN xx_common_expr SBRACKET_CLOSE */
// 2887 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("array-access", yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yy_destructor(yypParser,67,&yymsp[0].minor);
}
// 8116 "parser.cpp"
        break;
      case 357: /* xx_common_expr ::= xx_common_expr ADD xx_common_expr */
// 2892 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("add", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,27,&yymsp[-1].minor);
}
// 8124 "parser.cpp"
        break;
      case 358: /* xx_common_expr ::= xx_common_expr SUB xx_common_expr */
// 2897 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("sub", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,28,&yymsp[-1].minor);
}
// 8132 "parser.cpp"
        break;
      case 359: /* xx_common_expr ::= xx_common_expr MUL xx_common_expr */
// 2902 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("mul", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,30,&yymsp[-1].minor);
}
// 8140 "parser.cpp"
        break;
      case 360: /* xx_common_expr ::= xx_common_expr DIV xx_common_expr */
// 2907 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("div", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,31,&yymsp[-1].minor);
}
// 8148 "parser.cpp"
        break;
      case 361: /* xx_common_expr ::= xx_common_expr MOD xx_common_expr */
// 2912 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("mod", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,32,&yymsp[-1].minor);
}
// 8156 "parser.cpp"
        break;
      case 362: /* xx_common_expr ::= xx_common_expr CONCAT xx_common_expr */
// 2917 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("concat", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,29,&yymsp[-1].minor);
}
// 8164 "parser.cpp"
        break;
      case 363: /* xx_common_expr ::= xx_common_expr AND xx_common_expr */
// 2922 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("and", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,13,&yymsp[-1].minor);
}
// 8172 "parser.cpp"
        break;
      case 364: /* xx_common_expr ::= xx_common_expr OR xx_common_expr */
// 2927 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("or", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,12,&yymsp[-1].minor);
}
// 8180 "parser.cpp"
        break;
      case 365: /* xx_common_expr ::= xx_common_expr BITWISE_AND xx_common_expr */
// 2932 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_and", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,15,&yymsp[-1].minor);
}
// 8188 "parser.cpp"
        break;
      case 366: /* xx_common_expr ::= xx_common_expr BITWISE_OR xx_common_expr */
// 2937 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_or", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,14,&yymsp[-1].minor);
}
// 8196 "parser.cpp"
        break;
      case 367: /* xx_common_expr ::= xx_common_expr BITWISE_XOR xx_common_expr */
// 2942 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_xor", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,16,&yymsp[-1].minor);
}
// 8204 "parser.cpp"
        break;
      case 368: /* xx_common_expr ::= xx_common_expr BITWISE_SHIFTLEFT xx_common_expr */
// 2947 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_shiftleft", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,17,&yymsp[-1].minor);
}
// 8212 "parser.cpp"
        break;
      case 369: /* xx_common_expr ::= xx_common_expr BITWISE_SHIFTRIGHT xx_common_expr */
// 2952 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_shiftright", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,18,&yymsp[-1].minor);
}
// 8220 "parser.cpp"
        break;
      case 370: /* xx_common_expr ::= xx_common_expr INSTANCEOF xx_common_expr */
// 2957 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("instanceof", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
// 8228 "parser.cpp"
        break;
      case 371: /* xx_fetch_expr ::= FETCH IDENTIFIER COMMA xx_common_expr */
// 2962 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("fetch", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,34,&yymsp[-3].minor);
  yy_destructor(yypParser,6,&yymsp[-1].minor);
}
// 8237 "parser.cpp"
        break;
      case 373: /* xx_common_expr ::= TYPEOF xx_common_expr */
// 2972 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("typeof", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,36,&yymsp[-1].minor);
}
// 8245 "parser.cpp"
        break;
      case 375: /* xx_common_expr ::= INTEGER */
      case 428: /* xx_array_key ::= INTEGER */ yytestcase(yyruleno==428);
      case 430: /* xx_literal_expr ::= INTEGER */ yytestcase(yyruleno==430);
      case 447: /* xx_literal_array_key ::= INTEGER */ yytestcase(yyruleno==447);
// 2982 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_INTEGER, yymsp[0].minor.yy0, status->scanner_state);
}
// 8255 "parser.cpp"
        break;
      case 376: /* xx_common_expr ::= STRING */
      case 427: /* xx_array_key ::= STRING */ yytestcase(yyruleno==427);
      case 432: /* xx_literal_expr ::= STRING */ yytestcase(yyruleno==432);
      case 446: /* xx_literal_array_key ::= STRING */ yytestcase(yyruleno==446);
// 2987 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_STRING, yymsp[0].minor.yy0, status->scanner_state);
}
// 8265 "parser.cpp"
        break;
      case 377: /* xx_common_expr ::= CHAR */
      case 431: /* xx_literal_expr ::= CHAR */ yytestcase(yyruleno==431);
// 2992 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_CHAR, yymsp[0].minor.yy0, status->scanner_state);
}
// 8273 "parser.cpp"
        break;
      case 378: /* xx_common_expr ::= DOUBLE */
      case 433: /* xx_literal_expr ::= DOUBLE */ yytestcase(yyruleno==433);
// 2997 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_DOUBLE, yymsp[0].minor.yy0, status->scanner_state);
}
// 8281 "parser.cpp"
        break;
      case 379: /* xx_common_expr ::= NULL */
      case 434: /* xx_literal_expr ::= NULL */ yytestcase(yyruleno==434);
// 3002 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,65,&yymsp[0].minor);
}
// 8290 "parser.cpp"
        break;
      case 380: /* xx_common_expr ::= TRUE */
      case 436: /* xx_literal_expr ::= TRUE */ yytestcase(yyruleno==436);
// 3007 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_TRUE, NULL, status->scanner_state);
  yy_destructor(yypParser,117,&yymsp[0].minor);
}
// 8299 "parser.cpp"
        break;
      case 381: /* xx_common_expr ::= FALSE */
      case 435: /* xx_literal_expr ::= FALSE */ yytestcase(yyruleno==435);
// 3012 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_FALSE, NULL, status->scanner_state);
  yy_destructor(yypParser,118,&yymsp[0].minor);
}
// 8308 "parser.cpp"
        break;
      case 382: /* xx_common_expr ::= CONSTANT */
      case 425: /* xx_array_key ::= CONSTANT */ yytestcase(yyruleno==425);
      case 438: /* xx_literal_expr ::= CONSTANT */ yytestcase(yyruleno==438);
// 3017 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_CONSTANT, yymsp[0].minor.yy0, status->scanner_state);
}
// 8317 "parser.cpp"
        break;
      case 383: /* xx_common_expr ::= SBRACKET_OPEN SBRACKET_CLOSE */
      case 439: /* xx_literal_expr ::= SBRACKET_OPEN SBRACKET_CLOSE */ yytestcase(yyruleno==439);
// 3022 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("empty-array", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-1].minor);
  yy_destructor(yypParser,67,&yymsp[0].minor);
}
// 8327 "parser.cpp"
        break;
      case 384: /* xx_common_expr ::= SBRACKET_OPEN xx_array_list SBRACKET_CLOSE */
      case 440: /* xx_literal_expr ::= SBRACKET_OPEN xx_literal_array_list SBRACKET_CLOSE */ yytestcase(yyruleno==440);
// 3027 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("array", yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yy_destructor(yypParser,67,&yymsp[0].minor);
}
// 8337 "parser.cpp"
        break;
      case 385: /* xx_common_expr ::= NEW IDENTIFIER */
// 3032 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(0, yymsp[0].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-1].minor);
}
// 8345 "parser.cpp"
        break;
      case 386: /* xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3037 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8355 "parser.cpp"
        break;
      case 387: /* xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3042 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8365 "parser.cpp"
        break;
      case 388: /* xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE */
// 3047 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8375 "parser.cpp"
        break;
      case 389: /* xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3052 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8387 "parser.cpp"
        break;
      case 390: /* xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3057 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8399 "parser.cpp"
        break;
      case 391: /* xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3062 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(1, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8408 "parser.cpp"
        break;
      case 392: /* xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3067 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(1, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8417 "parser.cpp"
        break;
      case 393: /* xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3072 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(2, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8428 "parser.cpp"
        break;
      case 394: /* xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3077 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(2, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8439 "parser.cpp"
        break;
      case 395: /* xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3082 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(0, yymsp[-5].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8449 "parser.cpp"
        break;
      case 396: /* xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3087 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(0, yymsp[-4].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8459 "parser.cpp"
        break;
      case 397: /* xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3092 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-6].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-7].minor);
  yy_destructor(yypParser,55,&yymsp[-5].minor);
  yy_destructor(yypParser,107,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8471 "parser.cpp"
        break;
      case 398: /* xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3097 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-5].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-6].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,107,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8483 "parser.cpp"
        break;
      case 399: /* xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3102 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-8].minor.yy0, 1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-9].minor);
  yy_destructor(yypParser,55,&yymsp[-7].minor);
  yy_destructor(yypParser,107,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8497 "parser.cpp"
        break;
      case 400: /* xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3107 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-7].minor.yy0, 1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-8].minor);
  yy_destructor(yypParser,55,&yymsp[-6].minor);
  yy_destructor(yypParser,107,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8511 "parser.cpp"
        break;
      case 401: /* xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3112 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(1, yymsp[-5].minor.yy396, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8521 "parser.cpp"
        break;
      case 402: /* xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3117 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(1, yymsp[-4].minor.yy396, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8531 "parser.cpp"
        break;
      case 403: /* xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3122 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(2, yymsp[-7].minor.yy396, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8543 "parser.cpp"
        break;
      case 404: /* xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3127 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(2, yymsp[-6].minor.yy396, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8555 "parser.cpp"
        break;
      case 405: /* xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3132 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(3, yymsp[-7].minor.yy396, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8567 "parser.cpp"
        break;
      case 406: /* xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3137 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(3, yymsp[-6].minor.yy396, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8579 "parser.cpp"
        break;
      case 410: /* xx_common_expr ::= xx_common_expr QUESTION xx_common_expr COLON xx_common_expr */
// 3157 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("ternary", yymsp[-4].minor.yy396, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,8,&yymsp[-3].minor);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 8588 "parser.cpp"
        break;
      case 413: /* xx_call_parameter ::= xx_common_expr */
// 3170 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy396, status->scanner_state, 0);
}
// 8595 "parser.cpp"
        break;
      case 414: /* xx_call_parameter ::= IDENTIFIER COLON xx_common_expr */
// 3175 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(yymsp[-2].minor.yy0, yymsp[0].minor.yy396, status->scanner_state, 0);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 8603 "parser.cpp"
        break;
      case 415: /* xx_call_parameter ::= BITWISE_AND xx_common_expr */
// 3180 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy396, status->scanner_state, 1);
  yy_destructor(yypParser,15,&yymsp[-1].minor);
}
// 8611 "parser.cpp"
        break;
      case 416: /* xx_call_parameter ::= IDENTIFIER COLON BITWISE_AND xx_common_expr */
// 3185 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, status->scanner_state, 0);
  yy_destructor(yypParser,89,&yymsp[-2].minor);
  yy_destructor(yypParser,15,&yymsp[-1].minor);
}
// 8620 "parser.cpp"
        break;
      case 417: /* xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 3190 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-3].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8632 "parser.cpp"
        break;
      case 418: /* xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 3195 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", NULL, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-5].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8644 "parser.cpp"
        break;
      case 419: /* xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 3200 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", yymsp[-3].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-5].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8656 "parser.cpp"
        break;
      case 420: /* xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 3205 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8668 "parser.cpp"
        break;
      case 423: /* xx_array_item ::= xx_array_key COLON xx_array_value */
      case 443: /* xx_literal_array_item ::= xx_literal_array_key COLON xx_literal_array_value */ yytestcase(yyruleno==443);
// 3217 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_array_item(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 8677 "parser.cpp"
        break;
      case 424: /* xx_array_item ::= xx_array_value */
      case 444: /* xx_literal_array_item ::= xx_literal_array_value */ yytestcase(yyruleno==444);
// 3221 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_array_item(NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 8685 "parser.cpp"
        break;
      case 450: /* xx_comment ::= COMMENT */
// 3326 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_comment(yymsp[0].minor.yy0, status->scanner_state);
}
// 8692 "parser.cpp"
        break;
      case 451: /* xx_cblock ::= CBLOCK */
// 3330 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_cblock(yymsp[0].minor.yy0, status->scanner_state);
}
// 8699 "parser.cpp"
        break;
      default:
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
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
#endif /* YYNOERRORRECOVERY */

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
// 1306 "parser.lemon"

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
// 8828 "parser.cpp"
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
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
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
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
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
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
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
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
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
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
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
