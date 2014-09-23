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
#define YYNSTATE 922
#define YYNRULE 446
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
#define YY_ACTTAB_COUNT (11309)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   131,  922,  157,  155,  449,  566,  298,   80,   79,  185,
 /*    10 */   116,  587,  240,  625,  561,  153,  416,  634,  603,  564,
 /*    20 */   839,  167,   11,  641,  595,  564,  165,  558,  159,  133,
 /*    30 */   161,  406,  169,   76,   64,  598,  443,  411,  804,  654,
 /*    40 */   653,  651,  652,  650,  251,  893,  891,  552,  700,  730,
 /*    50 */   402,  669,  865,  550,   81,  641,  399,  531,  868,  698,
 /*    60 */   839,  236,  235,  232,  231,  234,  233,  230,  228,  229,
 /*    70 */   226,  227,  841,  543,  838,  528,  527,  112,  563,  477,
 /*    80 */   111,  287,   76,  584,  522,  110,  520,  517,  375,  516,
 /*    90 */   647,  646,  205,  671,  197,  101,  363,  203,  775,  871,
 /*   100 */   649,  648,   76,   77,  122,  130,  129,  872,  870,  869,
 /*   110 */   867,  866,  729,  131,  840,  157,  155,  152,  151,  147,
 /*   120 */   150,  149,  148,  553,  799,  315,   66,  561,  410,  782,
 /*   130 */   124,  153,  416,  485,  167,  858,  856,  857,  873,  165,
 /*   140 */   558,  159,  133,  161,  406,  169,  115,   64,  410,  619,
 /*   150 */   411,  804,  654,  653,  651,  652,  650,  253,    9,  496,
 /*   160 */   552,  209,  730,  369,   76,  865,  550,   81,  540,  399,
 /*   170 */   531,  868,  698,  541,  236,  235,  232,  231,  234,  233,
 /*   180 */   230,  228,  229,  226,  227,  245,  500,  863,  528,  527,
 /*   190 */   112,  628, 1342,  111,  295,  894,  641,  522,  110,  520,
 /*   200 */   517,  192,  516,  647,  646,  205,  630,  195,   75,  363,
 /*   210 */   410,  395,  871,  649,  648,  136,   77,  122,  130,  129,
 /*   220 */   872,  870,  869,  867,  866,  729,  131,   88,  157,  155,
 /*   230 */   150,  149,  148,  135,   19,  170,  189,  800,  602,  240,
 /*   240 */   561,  153,  416,   88,  634,  603,  107,  167,  204,  775,
 /*   250 */   641,  560,  165,  558,  159,  133,  161,  406,  169,  114,
 /*   260 */    64,  861,  613,  411,  804,  654,  653,  651,  652,  650,
 /*   270 */   253,    7,  496,  552,  801,  730,  366,  860,  865,  550,
 /*   280 */    81,  533,  399,  531,  868,  698,  534,  236,  235,  232,
 /*   290 */   231,  234,  233,  230,  228,  229,  226,  227,  182,  559,
 /*   300 */   526,  528,  527,  112,   72,  285,  111,   74,  276,  316,
 /*   310 */   522,  110,  520,  517,  682,  516,  647,  646,  205,  858,
 /*   320 */   856,  857,  873,  514,  446,  871,  649,  648,  509,   77,
 /*   330 */   122,  130,  129,  872,  870,  869,  867,  866,  729,  131,
 /*   340 */   183,  157,  155,  666,  468,  662,  661,  173,  557,  186,
 /*   350 */   381,  587,  240,  561,  597,  361, 1344,  640,  603,  842,
 /*   360 */   167,  399,  531,  641,  698,  165,  558,  159,  133,  161,
 /*   370 */   406,  169,  113,   64,  529,  607,  411,  804,  654,  653,
 /*   380 */   651,  652,  650, 1343,    5,  513,  552,  797,  730,  358,
 /*   390 */   508,  865,  550,   81,  481,  399,  531,  868,  698,  134,
 /*   400 */   236,  235,  232,  231,  234,  233,  230,  228,  229,  226,
 /*   410 */   227,  355,  502,  524,  528,  527,  112,  399,  531,  111,
 /*   420 */   698,  275,  316,  522,  110,  520,  517,  622,  516,  647,
 /*   430 */   646,  205,  858,  856,  857,  873,   10,  448,  871,  649,
 /*   440 */   648,  595,   77,  122,  130,  129,  872,  870,  869,  867,
 /*   450 */   866,  729,  131,  443,  157,  155,  479,  399,  531,  473,
 /*   460 */   854,  171,  191,  190,  251,  251,  561,  489,  272,  640,
 /*   470 */   634,  639,  639,  167, 1341,  641,  641,  488,  165,  558,
 /*   480 */   159,  133,  161,  406,  169,  108,   64,  202,  775,  411,
 /*   490 */   804,  654,  653,  651,  652,  650,  283,  457,  584,  552,
 /*   500 */   796,  730,  354,  123,  865,  550,   81,  791,  399,  531,
 /*   510 */   868,  698,  459,  236,  235,  232,  231,  234,  233,  230,
 /*   520 */   228,  229,  226,  227,  281,  184,  584,  528,  527,  112,
 /*   530 */    92,  187,  111,  602,  240,  394,  522,  110,  520,  517,
 /*   540 */   603,  516,  647,  646,  205,  641,  197,  120,  363,  296,
 /*   550 */   658,  871,  649,  648,   73,   77,  122,  130,  129,  872,
 /*   560 */   870,  869,  867,  866,  729,  131,  551,  157,  155,  756,
 /*   570 */   296,  470,  313,  469,  659,  468,  662,  661,  404,  561,
 /*   580 */   393,  754,  858,  856,  857,  873,  167,   88,  744,  511,
 /*   590 */   510,  165,  558,  159,  133,  161,  406,  169,  119,   64,
 /*   600 */   296,  656,  411,  804,  654,  653,  651,  652,  650,  740,
 /*   610 */   180,  616,  552,  519,  730,  610,  261,  865,  550,   81,
 /*   620 */     8,  695,  118,  868,    6,  644,  236,  235,  232,  231,
 /*   630 */   234,  233,  230,  228,  229,  226,  227,  839,  547,  739,
 /*   640 */   528,  527,  112,  296,  296,  111,  296,  296,  296,  522,
 /*   650 */   110,  520,  517,  604,  516,  647,  646,  205,  450,  196,
 /*   660 */   224,  363,    4,   65,  871,  649,  648, 1363,   77,  122,
 /*   670 */   130,  129,  872,  870,  869,  867,  866,  729,  131,  845,
 /*   680 */   157,  155,  738,  737,  321,  736,  735,  734,  401,  837,
 /*   690 */   543,  838,  561,  260,  858,  856,  857,  873,  296,  167,
 /*   700 */   117,  372,  855,  642,  165,  558,  159,  133,  161,  406,
 /*   710 */   169,  296,   64,  296,  453,  411,  804,  654,  653,  651,
 /*   720 */   652,  650,  297,  716, 1362,  552,  238,  730,  198,  455,
 /*   730 */   865,  550,   81,  525,   90,  225,  868,  733,   65,  236,
 /*   740 */   235,  232,  231,  234,  233,  230,  228,  229,  226,  227,
 /*   750 */   732,  474,  731,  528,  527,  112,   78,  600,  111,  493,
 /*   760 */    90,  259,  522,  110,  520,  517,  206,  516,  647,  646,
 /*   770 */   205,  437,  195,  222,  363,  539,   65,  871,  649,  648,
 /*   780 */   258,   77,  122,  130,  129,  872,  870,  869,  867,  866,
 /*   790 */   729,  131,  188,  157,  155,  245,  257,  321,  537,  293,
 /*   800 */   640,  603,   88,  832,  601,  561,  641,  858,  856,  857,
 /*   810 */   873,  535,  167,  206,  376,  855,  243,  165,  558,  159,
 /*   820 */   133,  161,  406,  169,  250,   64,  890,  832,  411,  804,
 /*   830 */   654,  653,  651,  652,  650,  641,  693,  256,  552,  793,
 /*   840 */   730,  350,  717,  865,  550,   81,  466,   65,  532,  868,
 /*   850 */   530,  421,  236,  235,  232,  231,  234,  233,  230,  228,
 /*   860 */   229,  226,  227,  590,  255,  521,  528,  527,  112,  254,
 /*   870 */   223,  111,  206,   65,  316,  522,  110,  520,  517,   26,
 /*   880 */   516,  647,  646,  205,  858,  856,  857,  873,  803,  472,
 /*   890 */   871,  649,  648,  802,   77,  122,  130,  129,  872,  870,
 /*   900 */   869,  867,  866,  729,  131,  591,  157,  155,  392,  741,
 /*   910 */   321,  434,   25,  221,  206,   24,   65,  242,  561,  207,
 /*   920 */   858,  856,  857,  873,  280,  167,  584,  377,  855,   88,
 /*   930 */   165,  558,  159,  133,  161,  406,  169,   23,   64,  391,
 /*   940 */   741,  411,  804,  654,  653,  651,  652,  650,  390,  741,
 /*   950 */    22,  552,  792,  730,  389,  741,  865,  550,   81,  388,
 /*   960 */   741,  105,  868,  689,  776,  236,  235,  232,  231,  234,
 /*   970 */   233,  230,  228,  229,  226,  227,  387,  741,  518,  528,
 /*   980 */   527,  112,  386,  741,  111,  385,  741,  316,  522,  110,
 /*   990 */   520,  517,   32,  516,  647,  646,  205,  858,  856,  857,
 /*  1000 */   873,   21,  463,  871,  649,  648,  109,   77,  122,  130,
 /*  1010 */   129,  872,  870,  869,  867,  866,  729,  131,   88,  157,
 /*  1020 */   155,  384,  741,  321,  383,  741,  244,   88,  382,  741,
 /*  1030 */   279,  561,  584,  858,  856,  857,  873,  641,  167,  795,
 /*  1040 */   405,  855,   88,  165,  558,  159,  133,  161,  406,  169,
 /*  1050 */    99,   64,  684,  776,  411,  804,  654,  653,  651,  652,
 /*  1060 */   650,  681,  291,  704,  552,  726,  730,  494,   90,  865,
 /*  1070 */   550,   81,  290,  704,   96,  868,  679,  776,  236,  235,
 /*  1080 */   232,  231,  234,  233,  230,  228,  229,  226,  227,  491,
 /*  1090 */    90,  497,  528,  527,  112,  278,  711,  111,  356,  593,
 /*  1100 */   316,  522,  110,  520,  517,   20,  516,  647,  646,  205,
 /*  1110 */   858,  856,  857,  873,  398,  448,  871,  649,  648,  779,
 /*  1120 */    77,  122,  130,  129,  872,  870,  869,  867,  866,  729,
 /*  1130 */   131,   88,  157,  155,  397,  757,  321,  400,  538,  241,
 /*  1140 */    88,  269,  268,  193,  561,  761,  858,  856,  857,  873,
 /*  1150 */   641,  167,  760,  403,  855,  183,  165,  558,  159,  133,
 /*  1160 */   161,  406,  169,  753,   64,  677,  263,  411,  804,  654,
 /*  1170 */   653,  651,  652,  650,  675,  262,  752,  552,  725,  730,
 /*  1180 */   751,  750,  865,  550,   81,  252,  501,  121,  868,  368,
 /*  1190 */   499,  236,  235,  232,  231,  234,  233,  230,  228,  229,
 /*  1200 */   226,  227,   16,  714,  495,  528,  527,  112,   91,   15,
 /*  1210 */   111,   29,  703,  316,  522,  110,  520,  517,   14,  516,
 /*  1220 */   647,  646,  205,  858,  856,  857,  873,  492,  446,  871,
 /*  1230 */   649,  648,   13,   77,  122,  130,  129,  872,  870,  869,
 /*  1240 */   867,  866,  729,  131,  183,  157,  155,   12,   71,  321,
 /*  1250 */   487,  486,   70,  183,  484,  483,   69,  561,  480,  858,
 /*  1260 */   856,  857,  873,   89,  167,   68,  380,  855,  183,  165,
 /*  1270 */   558,  159,  133,  161,  406,  169,  478,   64,  365,   67,
 /*  1280 */   411,  804,  654,  653,  651,  652,  650,  348,  476,  289,
 /*  1290 */   552,  723,  730,  294,  670,  865,  550,   81,  471,  181,
 /*  1300 */   665,  868,  346,  183,  236,  235,  232,  231,  234,  233,
 /*  1310 */   230,  228,  229,  226,  227,  668,  179,  370,  528,  527,
 /*  1320 */   112,  660,  657,  111,  465,  178,  183,  522,  110,  520,
 /*  1330 */   517,  462,  516,  647,  646,  205,  645,  430,  643,  364,
 /*  1340 */   249,  638,  871,  649,  648,  248,   77,  122,  130,  129,
 /*  1350 */   872,  870,  869,  867,  866,  729,  131,  637,  157,  155,
 /*  1360 */   425,  247,  321,  636,  246,  629,  635,  277,  288,  447,
 /*  1370 */   561,  445,  858,  856,  857,  873,  177,  167,  176,  379,
 /*  1380 */   855,  599,  165,  558,  159,  133,  161,  406,  169,  594,
 /*  1390 */    64,  592,  589,  411,  804,  654,  653,  651,  652,  650,
 /*  1400 */   588,  438,  274,  552,  721,  730,  282,  435,  865,  550,
 /*  1410 */    81,  273,  292,  432,  868,  175,  428,  236,  235,  232,
 /*  1420 */   231,  234,  233,  230,  228,  229,  226,  227,   28,    3,
 /*  1430 */   427,  528,  527,  112,  174,   27,  111,    2,  794,  892,
 /*  1440 */   522,  110,  520,  517,  565,  516,  647,  646,  205,  743,
 /*  1450 */   201,  503,  719,  718,  237,  871,  649,  648,  672,   77,
 /*  1460 */   122,  130,  129,  872,  870,  869,  867,  866,  729,  131,
 /*  1470 */    78,  157,  155,  585,   65,  321,  709,  798,  586,  577,
 /*  1480 */   673,  444,  632,  561,  631,  858,  856,  857,  873,  790,
 /*  1490 */   167,  583,  378,  855,  581,  165,  558,  159,  133,  161,
 /*  1500 */   406,  169,  596,   64,  441,  580,  411,  804,  654,  653,
 /*  1510 */   651,  652,  650,  579,  576,  442,  552,  708,  730,  633,
 /*  1520 */  1370,  865,  550,   81,  582,  578,  106,  868,  440,  104,
 /*  1530 */   236,  235,  232,  231,  234,  233,  230,  228,  229,  226,
 /*  1540 */   227,  853,  836,  835,  528,  527,  112, 1370,  834,  111,
 /*  1550 */   655, 1370,  833,  522,  110,  520,  517, 1370,  516,  647,
 /*  1560 */   646,  205, 1370,  200, 1370,  831,  103,  830,  871,  649,
 /*  1570 */   648, 1370,   77,  122,  130,  129,  872,  870,  869,  867,
 /*  1580 */   866,  729,  131,  102,  157,  155,  100,   98,  321,   97,
 /*  1590 */   829,  523,   95,   94,   93,  742,  561,  458,  858,  856,
 /*  1600 */   857,  873,  456,  167,  454,  374,  855,  452,  165,  558,
 /*  1610 */   159,  133,  161,  406,  169,  286,   64,  284, 1370,  411,
 /*  1620 */   804,  654,  653,  651,  652,  650, 1370, 1370, 1370,  552,
 /*  1630 */   706,  730, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,
 /*  1640 */   868, 1370, 1370,  236,  235,  232,  231,  234,  233,  230,
 /*  1650 */   228,  229,  226,  227, 1370, 1370, 1370,  528,  527,  112,
 /*  1660 */  1370, 1370,  111, 1370, 1370, 1370,  522,  110,  520,  517,
 /*  1670 */  1370,  516,  647,  646,  205, 1370,  199, 1370, 1370, 1370,
 /*  1680 */  1370,  871,  649,  648, 1370,   77,  122,  130,  129,  872,
 /*  1690 */   870,  869,  867,  866,  729,  131, 1370,  157,  155, 1370,
 /*  1700 */  1370,  321, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  561,
 /*  1710 */  1370,  858,  856,  857,  873, 1370,  167, 1370,  373,  855,
 /*  1720 */  1370,  165,  558,  159,  133,  161,  406,  169, 1370,   64,
 /*  1730 */  1370, 1370,  411,  804,  654,  653,  651,  652,  650, 1370,
 /*  1740 */  1370, 1370,  552,  702,  730, 1370, 1370,  865,  550,   81,
 /*  1750 */  1370, 1370, 1370,  868, 1370, 1370,  236,  235,  232,  231,
 /*  1760 */   234,  233,  230,  228,  229,  226,  227, 1370, 1370, 1370,
 /*  1770 */   528,  527,  112, 1370, 1370,  111, 1370, 1370,  420,  522,
 /*  1780 */   110,  520,  517, 1370,  516,  647,  646,  205,  858,  856,
 /*  1790 */   857,  873, 1370, 1370,  871,  649,  648, 1370,   77,  122,
 /*  1800 */   130,  129,  872,  870,  869,  867,  866,  729,  131, 1370,
 /*  1810 */   157,  155, 1370, 1370,  464, 1370,  469,  659,  468,  662,
 /*  1820 */   661,  461,  561,  469,  659,  468,  662,  661, 1370,  167,
 /*  1830 */  1370, 1370, 1370, 1370,  165,  558,  159,  133,  161,  406,
 /*  1840 */   169, 1370,   64, 1370, 1370,  411,  804,  460, 1370,  469,
 /*  1850 */   659,  468,  662,  661, 1370,  552,  697,  730, 1370, 1370,
 /*  1860 */   865,  550,   81, 1370, 1370, 1370,  868, 1370, 1370,  236,
 /*  1870 */   235,  232,  231,  234,  233,  230,  228,  229,  226,  227,
 /*  1880 */  1370, 1370, 1370,  528,  527,  112, 1370, 1370,  111, 1370,
 /*  1890 */  1370, 1370,  522,  110,  520,  517, 1370,  516, 1370,  362,
 /*  1900 */   205,  469,  659,  468,  662,  661, 1370,  871, 1370, 1370,
 /*  1910 */  1370,   77,  122,  130,  129,  872,  870,  869,  867,  866,
 /*  1920 */   729,  131, 1370,  157,  155, 1370, 1370,  360, 1370,  469,
 /*  1930 */   659,  468,  662,  661,  359,  561,  469,  659,  468,  662,
 /*  1940 */   661, 1370,  167, 1370, 1370, 1370, 1370,  165,  558,  159,
 /*  1950 */   133,  161,  406,  169, 1370,   64, 1370, 1370,  411,  804,
 /*  1960 */   357, 1370,  469,  659,  468,  662,  661, 1370,  552,  626,
 /*  1970 */   730, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,  868,
 /*  1980 */  1370, 1370,  236,  235,  232,  231,  234,  233,  230,  228,
 /*  1990 */   229,  226,  227, 1370, 1370, 1370,  528,  527,  112, 1370,
 /*  2000 */  1370,  111, 1370, 1370,  321,  522,  110,  520,  517, 1370,
 /*  2010 */   516, 1370, 1370,  205,  858,  856,  857,  873, 1370, 1370,
 /*  2020 */   871, 1370,  691, 1370,   77,  122,  130,  129,  872,  870,
 /*  2030 */   869,  867,  866,  729,  131, 1370,  157,  155, 1370,  315,
 /*  2040 */  1370, 1370, 1370,  781, 1370, 1370,  314, 1370,  561,  858,
 /*  2050 */   856,  857,  873,  515, 1370,  167,  858,  856,  857,  873,
 /*  2060 */   165,  558,  159,  133,  161,  406,  169,  313,   64, 1370,
 /*  2070 */  1370,  411,  804, 1370, 1370, 1370,  755,  858,  856,  857,
 /*  2080 */   873,  552,  623,  730, 1370, 1370,  865,  550,   81, 1370,
 /*  2090 */  1370, 1370,  868, 1370, 1370,  236,  235,  232,  231,  234,
 /*  2100 */   233,  230,  228,  229,  226,  227, 1370, 1370, 1370,  528,
 /*  2110 */   527,  112, 1370, 1370,  111, 1370, 1370,  333,  522,  110,
 /*  2120 */   520,  517, 1370,  516, 1370, 1370,  205,  858,  856,  857,
 /*  2130 */   873, 1370, 1370,  871, 1370, 1370, 1370,   77,  122,  130,
 /*  2140 */   129,  872,  870,  869,  867,  866,  729,  131, 1370,  157,
 /*  2150 */   155, 1370,  315, 1370, 1370, 1370,  780, 1370, 1370,  318,
 /*  2160 */  1370,  561,  858,  856,  857,  873, 1370, 1370,  167,  858,
 /*  2170 */   856,  857,  873,  165,  558,  159,  133,  161,  406,  169,
 /*  2180 */   687,   64, 1370, 1370,  411,  804, 1370, 1370, 1370, 1370,
 /*  2190 */  1370, 1370, 1370, 1370,  552,  620,  730, 1370, 1370,  865,
 /*  2200 */   550,   81, 1370, 1370, 1370,  868, 1370, 1370,  236,  235,
 /*  2210 */   232,  231,  234,  233,  230,  228,  229,  226,  227, 1370,
 /*  2220 */  1370, 1370,  528,  527,  112, 1370, 1370,  111, 1370, 1370,
 /*  2230 */   345,  522,  110,  520,  517, 1370,  516, 1370, 1370,  205,
 /*  2240 */   858,  856,  857,  873, 1370, 1370,  871, 1370, 1370, 1370,
 /*  2250 */    77,  122,  130,  129,  872,  870,  869,  867,  866,  729,
 /*  2260 */   131, 1370,  157,  155, 1370,  315, 1370, 1370, 1370,  778,
 /*  2270 */  1370, 1370,  334, 1370,  561,  858,  856,  857,  873, 1370,
 /*  2280 */  1370,  167,  858,  856,  857,  873,  165,  558,  159,  133,
 /*  2290 */   161,  406,  169,  419,   64, 1370, 1370,  411,  804, 1370,
 /*  2300 */  1370, 1370, 1370,  858,  856,  857,  873,  552,  617,  730,
 /*  2310 */  1370, 1370,  865,  550,   81, 1370, 1370, 1370,  868, 1370,
 /*  2320 */  1370,  236,  235,  232,  231,  234,  233,  230,  228,  229,
 /*  2330 */   226,  227, 1370, 1370, 1370,  528,  527,  112, 1370, 1370,
 /*  2340 */   111, 1370, 1370,  335,  522,  110,  520,  517, 1370,  516,
 /*  2350 */  1370, 1370,  205,  858,  856,  857,  873, 1370, 1370,  871,
 /*  2360 */  1370, 1370, 1370,   77,  122,  130,  129,  872,  870,  869,
 /*  2370 */   867,  866,  729,  131, 1370,  157,  155, 1370,  315, 1370,
 /*  2380 */  1370, 1370,  777, 1370, 1370, 1370, 1370,  561,  858,  856,
 /*  2390 */   857,  873, 1370, 1370,  167, 1370, 1370, 1370, 1370,  165,
 /*  2400 */   558,  159,  133,  161,  406,  169,  322,   64, 1370, 1370,
 /*  2410 */   411,  804, 1370, 1370, 1370, 1370,  858,  856,  857,  873,
 /*  2420 */   552,  614,  730, 1370, 1370,  865,  550,   81, 1370, 1370,
 /*  2430 */  1370,  868, 1370, 1370,  236,  235,  232,  231,  234,  233,
 /*  2440 */   230,  228,  229,  226,  227, 1370, 1370, 1370,  528,  527,
 /*  2450 */   112, 1370, 1370,  111, 1370, 1370,  336,  522,  110,  520,
 /*  2460 */   517, 1370,  516, 1370, 1370,  205,  858,  856,  857,  873,
 /*  2470 */  1370, 1370,  871, 1370, 1370, 1370,   77,  122,  130,  129,
 /*  2480 */   872,  870,  869,  867,  866,  729,  131, 1370,  157,  155,
 /*  2490 */  1370,  315, 1370, 1370, 1370,  772, 1370, 1370,  418, 1370,
 /*  2500 */   561,  858,  856,  857,  873, 1370, 1370,  167,  858,  856,
 /*  2510 */   857,  873,  165,  558,  159,  133,  161,  406,  169,  337,
 /*  2520 */    64, 1370, 1370,  411,  804, 1370, 1370, 1370, 1370,  858,
 /*  2530 */   856,  857,  873,  552,  611,  730, 1370, 1370,  865,  550,
 /*  2540 */    81, 1370, 1370, 1370,  868, 1370, 1370,  236,  235,  232,
 /*  2550 */   231,  234,  233,  230,  228,  229,  226,  227, 1370, 1370,
 /*  2560 */  1370,  528,  527,  112, 1370, 1370,  111, 1370, 1370,  417,
 /*  2570 */   522,  110,  520,  517, 1370,  516, 1370, 1370,  205,  858,
 /*  2580 */   856,  857,  873, 1370, 1370,  871, 1370, 1370, 1370,   77,
 /*  2590 */   122,  130,  129,  872,  870,  869,  867,  866,  729,  131,
 /*  2600 */  1370,  157,  155, 1370,  315, 1370, 1370, 1370,  771, 1370,
 /*  2610 */  1370,  338, 1370,  561,  858,  856,  857,  873, 1370, 1370,
 /*  2620 */   167,  858,  856,  857,  873,  165,  558,  159,  133,  161,
 /*  2630 */   406,  169,  324,   64, 1370, 1370,  411,  804, 1370, 1370,
 /*  2640 */  1370, 1370,  858,  856,  857,  873,  552,  608,  730, 1370,
 /*  2650 */  1370,  865,  550,   81, 1370, 1370, 1370,  868, 1370, 1370,
 /*  2660 */   236,  235,  232,  231,  234,  233,  230,  228,  229,  226,
 /*  2670 */   227, 1370, 1370, 1370,  528,  527,  112, 1370, 1370,  111,
 /*  2680 */  1370, 1370,  339,  522,  110,  520,  517, 1370,  516, 1370,
 /*  2690 */  1370,  205,  858,  856,  857,  873, 1370, 1370,  871, 1370,
 /*  2700 */  1370, 1370,   77,  122,  130,  129,  872,  870,  869,  867,
 /*  2710 */   866,  729,  131, 1370,  157,  155, 1370,  315, 1370, 1370,
 /*  2720 */  1370,  770, 1370, 1370,  323, 1370,  561,  858,  856,  857,
 /*  2730 */   873, 1370, 1370,  167,  858,  856,  857,  873,  165,  558,
 /*  2740 */   159,  133,  161,  406,  169,  341,   64, 1370, 1370,  411,
 /*  2750 */   804, 1370, 1370, 1370, 1370,  858,  856,  857,  873,  552,
 /*  2760 */   605,  730, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,
 /*  2770 */   868, 1370, 1370,  236,  235,  232,  231,  234,  233,  230,
 /*  2780 */   228,  229,  226,  227, 1370, 1370, 1370,  528,  527,  112,
 /*  2790 */  1370, 1370,  111, 1370, 1370,  310,  522,  110,  520,  517,
 /*  2800 */  1370,  516, 1370, 1370,  205,  858,  856,  857,  873, 1370,
 /*  2810 */  1370,  871, 1370, 1370, 1370,   77,  122,  130,  129,  872,
 /*  2820 */   870,  869,  867,  866,  729,  131, 1370,  157,  155, 1370,
 /*  2830 */   315, 1370, 1370, 1370,  769, 1370, 1370,  344, 1370,  561,
 /*  2840 */   858,  856,  857,  873, 1370, 1370,  167,  858,  856,  857,
 /*  2850 */   873,  165,  558,  159,  133,  161,  406,  169,  343,   64,
 /*  2860 */  1370, 1370,  411,  804, 1370, 1370, 1370, 1370,  858,  856,
 /*  2870 */   857,  873,  552,  575,  730, 1370, 1370,  865,  550,   81,
 /*  2880 */  1370, 1370, 1370,  868, 1370, 1370,  236,  235,  232,  231,
 /*  2890 */   234,  233,  230,  228,  229,  226,  227, 1370, 1370, 1370,
 /*  2900 */   528,  527,  112, 1370, 1370,  111, 1370, 1370,  415,  522,
 /*  2910 */   110,  520,  517, 1370,  516, 1370, 1370,  205,  858,  856,
 /*  2920 */   857,  873, 1370, 1370,  871, 1370, 1370, 1370,   77,  122,
 /*  2930 */   130,  129,  872,  870,  869,  867,  866,  729,  131, 1370,
 /*  2940 */   157,  155, 1370,  315, 1370, 1370, 1370,  768, 1370, 1370,
 /*  2950 */   414, 1370,  561,  858,  856,  857,  873, 1370, 1370,  167,
 /*  2960 */   858,  856,  857,  873,  165,  558,  159,  133,  161,  406,
 /*  2970 */   169,  413,   64, 1370, 1370,  411,  804, 1370, 1370, 1370,
 /*  2980 */  1370,  858,  856,  857,  873,  552,  573,  730, 1370, 1370,
 /*  2990 */   865,  550,   81, 1370, 1370, 1370,  868, 1370, 1370,  236,
 /*  3000 */   235,  232,  231,  234,  233,  230,  228,  229,  226,  227,
 /*  3010 */  1370, 1370, 1370,  528,  527,  112, 1370, 1370,  111, 1370,
 /*  3020 */  1370,  342,  522,  110,  520,  517, 1370,  516, 1370, 1370,
 /*  3030 */   205,  858,  856,  857,  873, 1370, 1370,  871, 1370, 1370,
 /*  3040 */  1370,   77,  122,  130,  129,  872,  870,  869,  867,  866,
 /*  3050 */   729,  131, 1370,  157,  155, 1370,  315, 1370, 1370, 1370,
 /*  3060 */   767, 1370, 1370,  327, 1370,  561,  858,  856,  857,  873,
 /*  3070 */  1370, 1370,  167,  858,  856,  857,  873,  165,  558,  159,
 /*  3080 */   133,  161,  406,  169,  326,   64, 1370, 1370,  411,  804,
 /*  3090 */  1370, 1370, 1370, 1370,  858,  856,  857,  873,  552,  572,
 /*  3100 */   730, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,  868,
 /*  3110 */  1370, 1370,  236,  235,  232,  231,  234,  233,  230,  228,
 /*  3120 */   229,  226,  227, 1370, 1370, 1370,  528,  527,  112, 1370,
 /*  3130 */  1370,  111, 1370, 1370,  332,  522,  110,  520,  517, 1370,
 /*  3140 */   516, 1370, 1370,  205,  858,  856,  857,  873, 1370, 1370,
 /*  3150 */   871, 1370, 1370, 1370,   77,  122,  130,  129,  872,  870,
 /*  3160 */   869,  867,  866,  729,  131, 1370,  157,  155, 1370,  315,
 /*  3170 */  1370, 1370, 1370,  766, 1370, 1370,  331, 1370,  561,  858,
 /*  3180 */   856,  857,  873, 1370, 1370,  167,  858,  856,  857,  873,
 /*  3190 */   165,  558,  159,  133,  161,  406,  169,  330,   64, 1370,
 /*  3200 */  1370,  411,  804, 1370, 1370, 1370, 1370,  858,  856,  857,
 /*  3210 */   873,  552,  570,  730, 1370, 1370,  865,  550,   81, 1370,
 /*  3220 */  1370, 1370,  868, 1370, 1370,  236,  235,  232,  231,  234,
 /*  3230 */   233,  230,  228,  229,  226,  227, 1370, 1370, 1370,  528,
 /*  3240 */   527,  112, 1370, 1370,  111, 1370, 1370,  329,  522,  110,
 /*  3250 */   520,  517, 1370,  516, 1370, 1370,  205,  858,  856,  857,
 /*  3260 */   873, 1370, 1370,  871, 1370, 1370, 1370,   77,  122,  130,
 /*  3270 */   129,  872,  870,  869,  867,  866,  729,  131, 1370,  157,
 /*  3280 */   155, 1370,  315, 1370, 1370, 1370,  765, 1370, 1370,  328,
 /*  3290 */  1370,  561,  858,  856,  857,  873, 1370, 1370,  167,  858,
 /*  3300 */   856,  857,  873,  165,  558,  159,  133,  161,  406,  169,
 /*  3310 */   325,   64, 1370, 1370,  411,  804, 1370, 1370, 1370, 1370,
 /*  3320 */   858,  856,  857,  873,  552,  699,  730, 1370, 1370,  865,
 /*  3330 */   550,   81, 1370, 1370, 1370,  868, 1370, 1370,  236,  235,
 /*  3340 */   232,  231,  234,  233,  230,  228,  229,  226,  227, 1370,
 /*  3350 */  1370, 1370,  528,  527,  112, 1370, 1370,  111, 1370, 1370,
 /*  3360 */   309,  522,  110,  520,  517, 1370,  516, 1370, 1370,  205,
 /*  3370 */   858,  856,  857,  873, 1370, 1370,  871, 1370, 1370, 1370,
 /*  3380 */    77,  122,  130,  129,  872,  870,  869,  867,  866,  729,
 /*  3390 */   131, 1370,  157,  155, 1370,  315, 1370, 1370, 1370,  764,
 /*  3400 */  1370, 1370,  320, 1370,  561,  858,  856,  857,  873, 1370,
 /*  3410 */  1370,  167,  858,  856,  857,  873,  165,  558,  159,  133,
 /*  3420 */   161,  406,  169,  412,   64, 1370, 1370,  411,  804, 1370,
 /*  3430 */  1370, 1370, 1370,  858,  856,  857,  873,  552,  208,  730,
 /*  3440 */  1370, 1370,  865,  550,   81, 1370, 1370, 1370,  868, 1370,
 /*  3450 */  1370,  236,  235,  232,  231,  234,  233,  230,  228,  229,
 /*  3460 */   226,  227, 1370, 1370, 1370,  528,  527,  112, 1370, 1370,
 /*  3470 */   111, 1370, 1370,  340,  522,  110,  520,  517, 1370,  516,
 /*  3480 */  1370, 1370,  205,  858,  856,  857,  873, 1370, 1370,  871,
 /*  3490 */  1370, 1370, 1370,   77,  122,  130,  129,  872,  870,  869,
 /*  3500 */   867,  866,  729,  131, 1370,  157,  155, 1370,  315, 1370,
 /*  3510 */  1370, 1370,  759, 1370, 1370,  409, 1370,  561,  858,  856,
 /*  3520 */   857,  873, 1370, 1370,  167,  858,  856,  857,  873,  165,
 /*  3530 */   558,  159,  133,  161,  406,  169,  408,   64, 1370, 1370,
 /*  3540 */   411,  804, 1370, 1370, 1370, 1370,  858,  856,  857,  873,
 /*  3550 */   552,  710,  730, 1370, 1370,  865,  550,   81, 1370, 1370,
 /*  3560 */  1370,  868, 1370, 1370,  236,  235,  232,  231,  234,  233,
 /*  3570 */   230,  228,  229,  226,  227, 1370, 1370, 1370,  528,  527,
 /*  3580 */   112, 1370, 1370,  111, 1370, 1370,  317,  522,  110,  520,
 /*  3590 */   517, 1370,  516, 1370, 1370,  205,  858,  856,  857,  873,
 /*  3600 */  1370, 1370,  871, 1370, 1370, 1370,   77,  122,  130,  129,
 /*  3610 */   872,  870,  869,  867,  866,  729,  131, 1370,  157,  155,
 /*  3620 */  1370,  315, 1370, 1370, 1370,  758, 1370, 1370,  305, 1370,
 /*  3630 */   561,  858,  856,  857,  873, 1370, 1370,  167,  858,  856,
 /*  3640 */   857,  873,  165,  558,  159,  133,  161,  406,  169,  304,
 /*  3650 */    64, 1370, 1370,  411,  804, 1370, 1370, 1370, 1370,  858,
 /*  3660 */   856,  857,  873,  552,  712,  730, 1370, 1370,  865,  550,
 /*  3670 */    81, 1370, 1370, 1370,  868, 1370, 1370,  236,  235,  232,
 /*  3680 */   231,  234,  233,  230,  228,  229,  226,  227, 1370, 1370,
 /*  3690 */  1370,  528,  527,  112, 1370, 1370,  111, 1370, 1370,  303,
 /*  3700 */   522,  110,  520,  517, 1370,  516, 1370, 1370,  205,  858,
 /*  3710 */   856,  857,  873, 1370, 1370,  871, 1370, 1370,  562,   77,
 /*  3720 */   122,  130,  129,  872,  870,  869,  867,  866,  729,  131,
 /*  3730 */  1370,  157,  155, 1370,  308, 1370, 1370, 1370, 1370, 1370,
 /*  3740 */  1370,  302, 1370,  561,  858,  856,  857,  873, 1370, 1370,
 /*  3750 */   167,  858,  856,  857,  873,  165,  558,  159,  133,  161,
 /*  3760 */   406,  169,  300,   64, 1370, 1370,  411,  804, 1370, 1370,
 /*  3770 */  1370, 1370,  858,  856,  857,  873,  552,  713,  730, 1370,
 /*  3780 */  1370,  865,  550,   81, 1370, 1370, 1370,  868, 1370, 1370,
 /*  3790 */   236,  235,  232,  231,  234,  233,  230,  228,  229,  226,
 /*  3800 */   227, 1370, 1370, 1370,  528,  527,  112, 1370, 1370,  111,
 /*  3810 */  1370, 1370,  311,  522,  110,  520,  517, 1370,  516, 1370,
 /*  3820 */  1370,  205,  858,  856,  857,  873, 1370, 1370,  871, 1370,
 /*  3830 */  1370, 1370,   77,  122,  130,  129,  872,  870,  869,  867,
 /*  3840 */   866,  729,  131, 1370,  157,  155, 1370,  299, 1370, 1370,
 /*  3850 */  1370, 1370, 1370, 1370,  319, 1370,  561,  858,  856,  857,
 /*  3860 */   873, 1370, 1370,  167,  858,  856,  857,  873,  165,  558,
 /*  3870 */   159,  133,  161,  406,  169,  307,   64, 1370, 1370,  411,
 /*  3880 */   804, 1370, 1370, 1370, 1370,  858,  856,  857,  873,  552,
 /*  3890 */   498,  730, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,
 /*  3900 */   868, 1370, 1370,  236,  235,  232,  231,  234,  233,  230,
 /*  3910 */   228,  229,  226,  227, 1370, 1370, 1370,  528,  527,  112,
 /*  3920 */  1370, 1370,  111, 1370, 1370,  306,  522,  110,  520,  517,
 /*  3930 */  1370,  516, 1370, 1370,  205,  858,  856,  857,  873, 1370,
 /*  3940 */  1370,  871, 1370, 1370, 1370,   77,  122,  130,  129,  872,
 /*  3950 */   870,  869,  867,  866,  729,  131, 1370,  157,  155, 1370,
 /*  3960 */   301, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  561,
 /*  3970 */   858,  856,  857,  873, 1370, 1370,  167, 1370, 1370, 1370,
 /*  3980 */  1370,  165,  558,  159,  133,  161,  406,  169, 1370,   64,
 /*  3990 */  1370, 1370,  411,  804, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4000 */  1370, 1370,  552,  715,  730, 1370, 1370,  865,  550,   81,
 /*  4010 */  1370, 1370, 1370,  868, 1370, 1370,  236,  235,  232,  231,
 /*  4020 */   234,  233,  230,  228,  229,  226,  227, 1370, 1370, 1370,
 /*  4030 */   528,  527,  112, 1370, 1370,  111, 1370, 1370, 1370,  522,
 /*  4040 */   110,  520,  517, 1370,  516, 1370, 1370,  205, 1370, 1370,
 /*  4050 */  1370, 1370, 1370, 1370,  871, 1370, 1370, 1370,   77,  122,
 /*  4060 */   130,  129,  872,  870,  869,  867,  866,  729,  131, 1370,
 /*  4070 */   157,  155, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4080 */  1370, 1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,  167,
 /*  4090 */  1370, 1370, 1370, 1370,  165,  558,  159,  133,  161,  406,
 /*  4100 */   169, 1370,   64, 1370, 1370,  411,  804, 1370, 1370, 1370,
 /*  4110 */  1370, 1370, 1370, 1370, 1370,  552,  722,  730, 1370, 1370,
 /*  4120 */   865,  550,   81, 1370, 1370, 1370,  868, 1370, 1370,  236,
 /*  4130 */   235,  232,  231,  234,  233,  230,  228,  229,  226,  227,
 /*  4140 */  1370, 1370, 1370,  528,  527,  112, 1370, 1370,  111, 1370,
 /*  4150 */  1370, 1370,  522,  110,  520,  517, 1370,  516, 1370, 1370,
 /*  4160 */   205, 1370, 1370, 1370, 1370, 1370, 1370,  871, 1370, 1370,
 /*  4170 */  1370,   77,  122,  130,  129,  872,  870,  869,  867,  866,
 /*  4180 */   729,  131, 1370,  157,  155, 1370, 1370, 1370, 1370, 1370,
 /*  4190 */  1370, 1370, 1370, 1370, 1370,  561, 1370, 1370, 1370, 1370,
 /*  4200 */  1370, 1370,  167, 1370, 1370, 1370, 1370,  165,  558,  159,
 /*  4210 */   133,  161,  406,  169, 1370,   64, 1370, 1370,  411,  804,
 /*  4220 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  552,  727,
 /*  4230 */   730, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,  868,
 /*  4240 */  1370, 1370,  236,  235,  232,  231,  234,  233,  230,  228,
 /*  4250 */   229,  226,  227, 1370, 1370, 1370,  528,  527,  112, 1370,
 /*  4260 */  1370,  111, 1370, 1370, 1370,  522,  110,  520,  517, 1370,
 /*  4270 */   516, 1370, 1370,  205, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4280 */   871, 1370, 1370, 1370,   77,  122,  130,  129,  872,  870,
 /*  4290 */   869,  867,  866,  729,  131, 1370,  157,  155, 1370, 1370,
 /*  4300 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  561, 1370,
 /*  4310 */  1370, 1370, 1370, 1370, 1370,  167, 1370, 1370, 1370, 1370,
 /*  4320 */   165,  558,  159,  133,  161,  406,  169, 1370,   64, 1370,
 /*  4330 */  1370,  411,  804, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4340 */  1370,  552,  724,  730, 1370, 1370,  865,  550,   81, 1370,
 /*  4350 */  1370, 1370,  868, 1370, 1370,  236,  235,  232,  231,  234,
 /*  4360 */   233,  230,  228,  229,  226,  227, 1370, 1370, 1370,  528,
 /*  4370 */   527,  112, 1370, 1370,  111, 1370, 1370, 1370,  522,  110,
 /*  4380 */   520,  517, 1370,  516, 1370, 1370,  205, 1370, 1370, 1370,
 /*  4390 */  1370, 1370, 1370,  871, 1370, 1370, 1370,   77,  122,  130,
 /*  4400 */   129,  872,  870,  869,  867,  866,  729,  131, 1370,  157,
 /*  4410 */   155, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4420 */  1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,  167, 1370,
 /*  4430 */  1370, 1370, 1370,  165,  558,  159,  133,  161,  406,  169,
 /*  4440 */  1370,   64, 1370, 1370,  411,  804, 1370, 1370, 1370, 1370,
 /*  4450 */  1370, 1370, 1370, 1370,  552,  720,  730, 1370, 1370,  865,
 /*  4460 */   550,   81, 1370, 1370, 1370,  868, 1370, 1370,  236,  235,
 /*  4470 */   232,  231,  234,  233,  230,  228,  229,  226,  227, 1370,
 /*  4480 */  1370, 1370,  528,  527,  112, 1370, 1370,  111, 1370, 1370,
 /*  4490 */  1370,  522,  110,  520,  517, 1370,  516, 1370, 1370,  205,
 /*  4500 */  1370, 1370, 1370, 1370, 1370, 1370,  871, 1370, 1370, 1370,
 /*  4510 */    77,  122,  130,  129,  872,  870,  869,  867,  866,  729,
 /*  4520 */   131, 1370,  157,  155, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4530 */  1370, 1370, 1370, 1370,  561, 1370, 1370, 1370, 1370, 1370,
 /*  4540 */  1370,  167, 1370, 1370, 1370, 1370,  165,  558,  159,  133,
 /*  4550 */   161,  406,  169, 1370,   64, 1370, 1370,  411,  804, 1370,
 /*  4560 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  552,  707,  730,
 /*  4570 */  1370, 1370,  865,  550,   81, 1370, 1370, 1370,  868, 1370,
 /*  4580 */  1370,  236,  235,  232,  231,  234,  233,  230,  228,  229,
 /*  4590 */   226,  227, 1370, 1370, 1370,  528,  527,  112, 1370, 1370,
 /*  4600 */   111, 1370, 1370, 1370,  522,  110,  520,  517, 1370,  516,
 /*  4610 */  1370, 1370,  205, 1370, 1370, 1370, 1370, 1370, 1370,  871,
 /*  4620 */  1370, 1370, 1370,   77,  122,  130,  129,  872,  870,  869,
 /*  4630 */   867,  866,  729,  131, 1370,  157,  155, 1370, 1370, 1370,
 /*  4640 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  561, 1370, 1370,
 /*  4650 */  1370, 1370, 1370, 1370,  167, 1370, 1370, 1370, 1370,  165,
 /*  4660 */   558,  159,  133,  161,  406,  169, 1370,   64, 1370, 1370,
 /*  4670 */   411,  804, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4680 */   552,  705,  730, 1370, 1370,  865,  550,   81, 1370, 1370,
 /*  4690 */  1370,  868, 1370, 1370,  236,  235,  232,  231,  234,  233,
 /*  4700 */   230,  228,  229,  226,  227, 1370, 1370, 1370,  528,  527,
 /*  4710 */   112, 1370, 1370,  111, 1370, 1370, 1370,  522,  110,  520,
 /*  4720 */   517, 1370,  516, 1370, 1370,  205, 1370, 1370, 1370, 1370,
 /*  4730 */  1370, 1370,  871, 1370, 1370, 1370,   77,  122,  130,  129,
 /*  4740 */   872,  870,  869,  867,  866,  729,  131, 1370,  157,  155,
 /*  4750 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4760 */   561, 1370, 1370, 1370, 1370, 1370, 1370,  167, 1370, 1370,
 /*  4770 */  1370, 1370,  165,  558,  159,  133,  161,  406,  169, 1370,
 /*  4780 */    64, 1370, 1370,  411,  804, 1370, 1370, 1370, 1370, 1370,
 /*  4790 */  1370, 1370, 1370,  552,  701,  730, 1370, 1370,  865,  550,
 /*  4800 */    81, 1370, 1370, 1370,  868, 1370, 1370,  236,  235,  232,
 /*  4810 */   231,  234,  233,  230,  228,  229,  226,  227, 1370, 1370,
 /*  4820 */  1370,  528,  527,  112, 1370, 1370,  111, 1370, 1370, 1370,
 /*  4830 */   522,  110,  520,  517, 1370,  516, 1370, 1370,  205, 1370,
 /*  4840 */  1370, 1370, 1370, 1370, 1370,  871, 1370, 1370, 1370,   77,
 /*  4850 */   122,  130,  129,  872,  870,  869,  867,  866,  729,  131,
 /*  4860 */  1370,  157,  155, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4870 */  1370, 1370, 1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  4880 */   167, 1370, 1370, 1370, 1370,  165,  558,  159,  133,  161,
 /*  4890 */   406,  169, 1370,   64, 1370, 1370,  411,  804, 1370, 1370,
 /*  4900 */  1370, 1370, 1370, 1370, 1370, 1370,  552,  696,  730, 1370,
 /*  4910 */  1370,  865,  550,   81, 1370, 1370, 1370,  868, 1370, 1370,
 /*  4920 */   236,  235,  232,  231,  234,  233,  230,  228,  229,  226,
 /*  4930 */   227, 1370, 1370, 1370,  528,  527,  112, 1370, 1370,  111,
 /*  4940 */  1370, 1370, 1370,  522,  110,  520,  517, 1370,  516, 1370,
 /*  4950 */  1370,  205, 1370, 1370, 1370, 1370, 1370, 1370,  871, 1370,
 /*  4960 */  1370, 1370,   77,  122,  130,  129,  872,  870,  869,  867,
 /*  4970 */   866,  729,  131, 1370,  157,  155, 1370, 1370, 1370, 1370,
 /*  4980 */  1370, 1370, 1370, 1370, 1370, 1370,  561, 1370, 1370, 1370,
 /*  4990 */  1370, 1370, 1370,  167, 1370, 1370, 1370, 1370,  165,  558,
 /*  5000 */   159,  133,  161,  406,  169, 1370,   64, 1370, 1370,  411,
 /*  5010 */   804, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  552,
 /*  5020 */   627,  730, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,
 /*  5030 */   868, 1370, 1370,  236,  235,  232,  231,  234,  233,  230,
 /*  5040 */   228,  229,  226,  227, 1370, 1370, 1370,  528,  527,  112,
 /*  5050 */  1370, 1370,  111, 1370, 1370, 1370,  522,  110,  520,  517,
 /*  5060 */  1370,  516, 1370, 1370,  205, 1370, 1370, 1370, 1370, 1370,
 /*  5070 */  1370,  871, 1370, 1370, 1370,   77,  122,  130,  129,  872,
 /*  5080 */   870,  869,  867,  866,  729,  131, 1370,  157,  155, 1370,
 /*  5090 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  561,
 /*  5100 */  1370, 1370, 1370, 1370, 1370, 1370,  167, 1370, 1370, 1370,
 /*  5110 */  1370,  165,  558,  159,  133,  161,  406,  169, 1370,   64,
 /*  5120 */  1370, 1370,  411,  804, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  5130 */  1370, 1370,  552,  624,  730, 1370, 1370,  865,  550,   81,
 /*  5140 */  1370, 1370, 1370,  868, 1370, 1370,  236,  235,  232,  231,
 /*  5150 */   234,  233,  230,  228,  229,  226,  227, 1370, 1370, 1370,
 /*  5160 */   528,  527,  112, 1370, 1370,  111, 1370, 1370, 1370,  522,
 /*  5170 */   110,  520,  517, 1370,  516, 1370, 1370,  205, 1370, 1370,
 /*  5180 */  1370, 1370, 1370, 1370,  871, 1370, 1370, 1370,   77,  122,
 /*  5190 */   130,  129,  872,  870,  869,  867,  866,  729,  131, 1370,
 /*  5200 */   157,  155, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  5210 */  1370, 1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,  167,
 /*  5220 */  1370, 1370, 1370, 1370,  165,  558,  159,  133,  161,  406,
 /*  5230 */   169, 1370,   64, 1370, 1370,  411,  804, 1370, 1370, 1370,
 /*  5240 */  1370, 1370, 1370, 1370, 1370,  552,  621,  730, 1370, 1370,
 /*  5250 */   865,  550,   81, 1370, 1370, 1370,  868, 1370, 1370,  236,
 /*  5260 */   235,  232,  231,  234,  233,  230,  228,  229,  226,  227,
 /*  5270 */  1370, 1370, 1370,  528,  527,  112, 1370, 1370,  111, 1370,
 /*  5280 */  1370, 1370,  522,  110,  520,  517, 1370,  516, 1370, 1370,
 /*  5290 */   205, 1370, 1370, 1370, 1370, 1370, 1370,  871, 1370, 1370,
 /*  5300 */  1370,   77,  122,  130,  129,  872,  870,  869,  867,  866,
 /*  5310 */   729,  131, 1370,  157,  155, 1370, 1370, 1370, 1370, 1370,
 /*  5320 */  1370, 1370, 1370, 1370, 1370,  561, 1370, 1370, 1370, 1370,
 /*  5330 */  1370, 1370,  167, 1370, 1370, 1370, 1370,  165,  558,  159,
 /*  5340 */   133,  161,  406,  169, 1370,   64, 1370, 1370,  411,  804,
 /*  5350 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  552,  618,
 /*  5360 */   730, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,  868,
 /*  5370 */  1370, 1370,  236,  235,  232,  231,  234,  233,  230,  228,
 /*  5380 */   229,  226,  227, 1370, 1370, 1370,  528,  527,  112, 1370,
 /*  5390 */  1370,  111, 1370, 1370, 1370,  522,  110,  520,  517, 1370,
 /*  5400 */   516, 1370, 1370,  205, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  5410 */   871, 1370, 1370, 1370,   77,  122,  130,  129,  872,  870,
 /*  5420 */   869,  867,  866,  729,  131, 1370,  157,  155, 1370, 1370,
 /*  5430 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  561, 1370,
 /*  5440 */  1370, 1370, 1370, 1370, 1370,  167, 1370, 1370, 1370, 1370,
 /*  5450 */   165,  558,  159,  133,  161,  406,  169, 1370,   64, 1370,
 /*  5460 */  1370,  411,  804, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  5470 */  1370,  552,  615,  730, 1370, 1370,  865,  550,   81, 1370,
 /*  5480 */  1370, 1370,  868, 1370, 1370,  236,  235,  232,  231,  234,
 /*  5490 */   233,  230,  228,  229,  226,  227, 1370, 1370, 1370,  528,
 /*  5500 */   527,  112, 1370, 1370,  111, 1370, 1370, 1370,  522,  110,
 /*  5510 */   520,  517, 1370,  516, 1370, 1370,  205, 1370, 1370, 1370,
 /*  5520 */  1370, 1370, 1370,  871, 1370, 1370, 1370,   77,  122,  130,
 /*  5530 */   129,  872,  870,  869,  867,  866,  729,  131, 1370,  157,
 /*  5540 */   155, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  5550 */  1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,  167, 1370,
 /*  5560 */  1370, 1370, 1370,  165,  558,  159,  133,  161,  406,  169,
 /*  5570 */  1370,   64, 1370, 1370,  411,  804, 1370, 1370, 1370, 1370,
 /*  5580 */  1370, 1370, 1370, 1370,  552,  612,  730, 1370, 1370,  865,
 /*  5590 */   550,   81, 1370, 1370, 1370,  868, 1370, 1370,  236,  235,
 /*  5600 */   232,  231,  234,  233,  230,  228,  229,  226,  227, 1370,
 /*  5610 */  1370, 1370,  528,  527,  112, 1370, 1370,  111, 1370, 1370,
 /*  5620 */  1370,  522,  110,  520,  517, 1370,  516, 1370, 1370,  205,
 /*  5630 */  1370, 1370, 1370, 1370, 1370, 1370,  871, 1370, 1370, 1370,
 /*  5640 */    77,  122,  130,  129,  872,  870,  869,  867,  866,  729,
 /*  5650 */   131, 1370,  157,  155, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  5660 */  1370, 1370, 1370, 1370,  561, 1370, 1370, 1370, 1370, 1370,
 /*  5670 */  1370,  167, 1370, 1370, 1370, 1370,  165,  558,  159,  133,
 /*  5680 */   161,  406,  169, 1370,   64, 1370, 1370,  411,  804, 1370,
 /*  5690 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  552,  609,  730,
 /*  5700 */  1370, 1370,  865,  550,   81, 1370, 1370, 1370,  868, 1370,
 /*  5710 */  1370,  236,  235,  232,  231,  234,  233,  230,  228,  229,
 /*  5720 */   226,  227, 1370, 1370, 1370,  528,  527,  112, 1370, 1370,
 /*  5730 */   111, 1370, 1370, 1370,  522,  110,  520,  517, 1370,  516,
 /*  5740 */  1370, 1370,  205, 1370, 1370, 1370, 1370, 1370, 1370,  871,
 /*  5750 */  1370, 1370, 1370,   77,  122,  130,  129,  872,  870,  869,
 /*  5760 */   867,  866,  729,  131, 1370,  157,  155, 1370, 1370, 1370,
 /*  5770 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  561, 1370, 1370,
 /*  5780 */  1370, 1370, 1370, 1370,  167, 1370, 1370, 1370, 1370,  165,
 /*  5790 */   558,  159,  133,  161,  406,  169, 1370,   64, 1370, 1370,
 /*  5800 */   411,  804, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  5810 */   552,  606,  730, 1370, 1370,  865,  550,   81, 1370, 1370,
 /*  5820 */  1370,  868, 1370, 1370,  236,  235,  232,  231,  234,  233,
 /*  5830 */   230,  228,  229,  226,  227, 1370, 1370, 1370,  528,  527,
 /*  5840 */   112, 1370, 1370,  111, 1370, 1370, 1370,  522,  110,  520,
 /*  5850 */   517, 1370,  516, 1370, 1370,  205, 1370, 1370, 1370, 1370,
 /*  5860 */  1370, 1370,  871, 1370, 1370, 1370,   77,  122,  130,  129,
 /*  5870 */   872,  870,  869,  867,  866,  729,  131, 1370,  157,  155,
 /*  5880 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  5890 */   561, 1370, 1370, 1370, 1370, 1370, 1370,  167, 1370, 1370,
 /*  5900 */  1370, 1370,  165,  558,  159,  133,  161,  406,  169, 1370,
 /*  5910 */    64, 1370, 1370,  411,  804, 1370, 1370, 1370, 1370, 1370,
 /*  5920 */  1370, 1370, 1370,  552,  574,  730, 1370, 1370,  865,  550,
 /*  5930 */    81, 1370, 1370, 1370,  868, 1370, 1370,  236,  235,  232,
 /*  5940 */   231,  234,  233,  230,  228,  229,  226,  227, 1370, 1370,
 /*  5950 */  1370,  528,  527,  112, 1370, 1370,  111, 1370, 1370, 1370,
 /*  5960 */   522,  110,  520,  517, 1370,  516, 1370, 1370,  205, 1370,
 /*  5970 */  1370, 1370, 1370, 1370, 1370,  871, 1370, 1370, 1370,   77,
 /*  5980 */   122,  130,  129,  872,  870,  869,  867,  866,  729,  131,
 /*  5990 */  1370,  157,  155, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  6000 */  1370, 1370, 1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  6010 */   167, 1370, 1370, 1370, 1370,  165,  558,  159,  133,  161,
 /*  6020 */   406,  169, 1370,   64, 1370, 1370,  411,  804, 1370, 1370,
 /*  6030 */  1370, 1370, 1370, 1370, 1370, 1370,  552,  571,  730, 1370,
 /*  6040 */  1370,  865,  550,   81, 1370, 1370, 1370,  868, 1370, 1370,
 /*  6050 */   236,  235,  232,  231,  234,  233,  230,  228,  229,  226,
 /*  6060 */   227, 1370, 1370, 1370,  528,  527,  112, 1370, 1370,  111,
 /*  6070 */  1370, 1370, 1370,  522,  110,  520,  517, 1370,  516, 1370,
 /*  6080 */  1370,  205, 1370, 1370, 1370, 1370, 1370, 1370,  871, 1370,
 /*  6090 */  1370, 1370,   77,  122,  130,  129,  872,  870,  869,  867,
 /*  6100 */   866,  729,  131, 1370,  157,  155, 1370, 1370, 1370, 1370,
 /*  6110 */  1370, 1370, 1370, 1370, 1370, 1370,  561, 1370, 1370, 1370,
 /*  6120 */  1370, 1370, 1370,  167, 1370, 1370, 1370, 1370,  165,  558,
 /*  6130 */   159,  133,  161,  406,  169, 1370,   64, 1370, 1370,  411,
 /*  6140 */   804, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  552,
 /*  6150 */  1370,  730, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,
 /*  6160 */   868, 1370, 1370,  236,  235,  232,  231,  234,  233,  230,
 /*  6170 */   228,  229,  226,  227, 1370, 1370, 1370,  528,  527,  112,
 /*  6180 */  1370, 1370,  111, 1370, 1370, 1370,  522,  110,  520,  517,
 /*  6190 */   125,  516, 1370, 1370,  205, 1370, 1370, 1370, 1370, 1370,
 /*  6200 */  1370,  871, 1370, 1370, 1370,   77,  122,  130,  129,  872,
 /*  6210 */   870,  869,  867,  866,  729, 1370, 1370,  558, 1370, 1370,
 /*  6220 */  1370, 1370, 1370, 1370, 1370, 1370,  567,  353, 1370,  239,
 /*  6230 */  1370,  475, 1370,  451, 1370,  439,  436,  552, 1370,  730,
 /*  6240 */  1370, 1370, 1370,  433, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  6250 */  1370,  220,  219,  218,  217,  216,  215,  214,  213,  212,
 /*  6260 */   211,  210,  877,  876,  875,  528,  527,  112, 1370, 1370,
 /*  6270 */   111, 1370, 1370, 1370,  522,  110,  520,  517,  163,  516,
 /*  6280 */   157,  155,  205, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  6290 */  1370, 1370,  561,   77,  122,  130,  129, 1370, 1370,  167,
 /*  6300 */  1370, 1370,  729, 1370,  165,  558,  159,  133,  161,  406,
 /*  6310 */   169, 1370,   64, 1370, 1370,  411, 1370, 1370, 1370, 1370,
 /*  6320 */  1370, 1370, 1370, 1370, 1370,  552, 1370, 1370, 1370, 1370,
 /*  6330 */   865,  550,   81, 1370, 1370, 1370,  868, 1370, 1370,  888,
 /*  6340 */   887,  886,  885,  884,  883,  882,  881,  880,  879,  878,
 /*  6350 */   877,  876,  875,  139,  145,  146,  143,  144,  142,  141,
 /*  6360 */   140,  168,  164,  160,  158,  156,  154,  162,  166,  152,
 /*  6370 */   151,  147,  150,  149,  148, 1370, 1370,  871, 1370, 1370,
 /*  6380 */  1370, 1370, 1370,  153,  416,  872,  870,  869,  867,  866,
 /*  6390 */  1370, 1370, 1370, 1370, 1370, 1370, 1369,  568,    1,  569,
 /*  6400 */   920,  919,  918,  917,  916,  915,  914,  913,  912,  911,
 /*  6410 */   910,  909,  908,  907,  906,  905,  904,  903,  902,  901,
 /*  6420 */   900,  899,  898,  897,  896,  895,  138, 1370, 1370,  139,
 /*  6430 */   145,  146,  143,  144,  142,  141,  140,  168,  164,  160,
 /*  6440 */   158,  156,  154,  162,  166,  152,  151,  147,  150,  149,
 /*  6450 */   148, 1370,  429, 1370, 1370, 1370, 1370, 1370, 1370,  153,
 /*  6460 */   416,  168,  164,  160,  158,  156,  154,  162,  166,  152,
 /*  6470 */   151,  147,  150,  149,  148, 1370,  318, 1370, 1370,  424,
 /*  6480 */   423,  422, 1370,  153,  416,  889,  858,  856,  857,  873,
 /*  6490 */  1370, 1370,  407, 1370, 1370,  685,  482,  686, 1370, 1370,
 /*  6500 */  1370, 1370,  921,  920,  919,  918,  917,  916,  915,  914,
 /*  6510 */   913,  912,  911,  910,  909,  908,  907,  906,  905,  904,
 /*  6520 */   903,  902,  901,  900,  899,  898,  897,  896,  895, 1370,
 /*  6530 */  1370, 1370, 1370, 1370,  806,  827,  826,  825,  824,  823,
 /*  6540 */   822,  821,  820,  819,  817,  816,  815,  814,  813,  812,
 /*  6550 */   811,  810,  809,  808,  807,  429, 1370, 1370,  318, 1370,
 /*  6560 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  858,  856,
 /*  6570 */   857,  873, 1370, 1370, 1370, 1370,   63,  688,  482,  686,
 /*  6580 */  1370, 1370,  424,  423,  422,  728,  818,  805, 1370, 1370,
 /*  6590 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  312, 1370, 1370,
 /*  6600 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  507,  506,  505,
 /*  6610 */   504,  806,  827,  826,  825,  824,  823,  822,  821,  820,
 /*  6620 */   819,  817,  816,  815,  814,  813,  812,  811,  810,  809,
 /*  6630 */   808,  807,  146,  143,  144,  142,  141,  140,  168,  164,
 /*  6640 */   160,  158,  156,  154,  162,  166,  152,  151,  147,  150,
 /*  6650 */   149,  148, 1370,   52, 1370,  789, 1370, 1370, 1370, 1370,
 /*  6660 */   153,  416,  728,  818,  805, 1370, 1370, 1370, 1370, 1370,
 /*  6670 */  1370, 1370, 1370, 1370,  312, 1370, 1370, 1370, 1370, 1370,
 /*  6680 */  1370, 1370, 1370, 1370,  507,  506,  505,  504, 1370, 1370,
 /*  6690 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  788,  787,
 /*  6700 */   786,  785,  784,  783, 1370, 1370, 1370,  806,  827,  826,
 /*  6710 */   825,  824,  823,  822,  821,  820,  819,  817,  816,  815,
 /*  6720 */   814,  813,  812,  811,  810,  809,  808,  807,  143,  144,
 /*  6730 */   142,  141,  140,  168,  164,  160,  158,  156,  154,  162,
 /*  6740 */   166,  152,  151,  147,  150,  149,  148, 1370, 1370,   50,
 /*  6750 */    83,  396, 1370, 1370,  172,  153,  416,  548,  728,  818,
 /*  6760 */   805,   86, 1370, 1370, 1370, 1370,  789, 1370, 1370, 1370,
 /*  6770 */   312, 1370,  844, 1370, 1370, 1370, 1370,  789,  848, 1370,
 /*  6780 */   507,  506,  505,  504,  806,  827,  826,  825,  824,  823,
 /*  6790 */   822,  821,  820,  819,  817,  816,  815,  814,  813,  812,
 /*  6800 */   811,  810,  809,  808,  807, 1370, 1370, 1370, 1370,  788,
 /*  6810 */   787,  786,  785,  784,  783,  536,  512,  763,  762,  850,
 /*  6820 */   788,  787,  786,  785,  784,  783,   62,  852,  851,  849,
 /*  6830 */   846,  847, 1370, 1370,  490,  728,  818,  805,  549, 1370,
 /*  6840 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  312, 1370, 1370,
 /*  6850 */  1370, 1370,  194, 1370, 1370, 1370,  536,  507,  506,  505,
 /*  6860 */   504, 1370,  888,  887,  886,  885,  884,  883,  882,  881,
 /*  6870 */   880,  879,  878,  877,  876,  875, 1370, 1370, 1370,  542,
 /*  6880 */   806,  827,  826,  825,  824,  823,  822,  821,  820,  819,
 /*  6890 */   817,  816,  815,  814,  813,  812,  811,  810,  809,  808,
 /*  6900 */   807, 1370, 1370,  888,  887,  886,  885,  884,  883,  882,
 /*  6910 */   881,  880,  879,  878,  877,  876,  875, 1370, 1370, 1370,
 /*  6920 */  1370,  172,   36, 1370,  544, 1370, 1370, 1370, 1370, 1370,
 /*  6930 */  1370,  728,  818,  805, 1370, 1370, 1370, 1370, 1370,  844,
 /*  6940 */  1370, 1370, 1370,  312, 1370,  848, 1370,  843, 1370, 1370,
 /*  6950 */  1370, 1370, 1370,  507,  506,  505,  504,  806,  827,  826,
 /*  6960 */   825,  824,  823,  822,  821,  820,  819,  817,  816,  815,
 /*  6970 */   814,  813,  812,  811,  810,  809,  808,  807, 1370, 1370,
 /*  6980 */  1370, 1370, 1370, 1370, 1370, 1370,  545, 1370,  536, 1370,
 /*  6990 */  1370, 1370, 1370, 1370,  546,  851,  849,  846,  847,   34,
 /*  7000 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  371,  728,  818,
 /*  7010 */   805,  549, 1370, 1370, 1370, 1370, 1370, 1370,   87, 1370,
 /*  7020 */   312, 1370, 1370, 1370, 1370,  194, 1370, 1370, 1370, 1370,
 /*  7030 */   507,  506,  505,  504,  789,  888,  887,  886,  885,  884,
 /*  7040 */   883,  882,  881,  880,  879,  878,  877,  876,  875, 1370,
 /*  7050 */  1370, 1370, 1370,  806,  827,  826,  825,  824,  823,  822,
 /*  7060 */   821,  820,  819,  817,  816,  815,  814,  813,  812,  811,
 /*  7070 */   810,  809,  808,  807, 1370, 1370, 1370,  788,  787,  786,
 /*  7080 */   785,  784,  783, 1370, 1370,  774,  773, 1370, 1370,  172,
 /*  7090 */  1370, 1370,  544, 1370, 1370,   61,   84, 1370, 1370, 1370,
 /*  7100 */  1370, 1370, 1370, 1370,  728,  818,  805,  844, 1370, 1370,
 /*  7110 */  1370, 1370,  789,  848, 1370, 1370,  312, 1370, 1370, 1370,
 /*  7120 */  1370, 1370, 1370, 1370, 1370, 1370,  507,  506,  505,  504,
 /*  7130 */   806,  827,  826,  825,  824,  823,  822,  821,  820,  819,
 /*  7140 */   817,  816,  815,  814,  813,  812,  811,  810,  809,  808,
 /*  7150 */   807, 1370, 1370, 1370,  545,  788,  787,  786,  785,  784,
 /*  7160 */   783,  536,  546,  851,  849,  846,  847, 1370, 1370, 1370,
 /*  7170 */  1370, 1370,   60, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7180 */   367,  728,  818,  805,  549, 1370, 1370, 1370, 1370, 1370,
 /*  7190 */  1370,   82, 1370,  312, 1370, 1370, 1370, 1370,  194, 1370,
 /*  7200 */  1370, 1370, 1370,  507,  506,  505,  504,  789,  888,  887,
 /*  7210 */   886,  885,  884,  883,  882,  881,  880,  879,  878,  877,
 /*  7220 */   876,  875, 1370, 1370, 1370, 1370,  806,  827,  826,  825,
 /*  7230 */   824,  823,  822,  821,  820,  819,  817,  816,  815,  814,
 /*  7240 */   813,  812,  811,  810,  809,  808,  807, 1370, 1370, 1370,
 /*  7250 */   788,  787,  786,  785,  784,  783, 1370, 1370, 1370, 1370,
 /*  7260 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,   59,   85,
 /*  7270 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  728,  818,  805,
 /*  7280 */  1370, 1370, 1370, 1370, 1370,  789, 1370, 1370, 1370,  312,
 /*  7290 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  507,
 /*  7300 */   506,  505,  504,  806,  827,  826,  825,  824,  823,  822,
 /*  7310 */   821,  820,  819,  817,  816,  815,  814,  813,  812,  811,
 /*  7320 */   810,  809,  808,  807, 1370, 1370, 1370, 1370,  788,  787,
 /*  7330 */   786,  785,  784,  783,  536, 1370, 1370, 1370, 1370, 1370,
 /*  7340 */  1370, 1370, 1370, 1370, 1370,   58, 1370, 1370, 1370, 1370,
 /*  7350 */  1370, 1370, 1370,  349,  728,  818,  805,  549, 1370, 1370,
 /*  7360 */  1370, 1370, 1370, 1370, 1370, 1370,  312, 1370, 1370, 1370,
 /*  7370 */  1370,  194, 1370, 1370, 1370, 1370,  507,  506,  505,  504,
 /*  7380 */  1370,  888,  887,  886,  885,  884,  883,  882,  881,  880,
 /*  7390 */   879,  878,  877,  876,  875, 1370, 1370, 1370, 1370,  806,
 /*  7400 */   827,  826,  825,  824,  823,  822,  821,  820,  819,  817,
 /*  7410 */   816,  815,  814,  813,  812,  811,  810,  809,  808,  807,
 /*  7420 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7430 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7440 */  1370,   57, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7450 */   728,  818,  805, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7460 */  1370, 1370,  312, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7470 */  1370, 1370,  507,  506,  505,  504,  806,  827,  826,  825,
 /*  7480 */   824,  823,  822,  821,  820,  819,  817,  816,  815,  814,
 /*  7490 */   813,  812,  811,  810,  809,  808,  807, 1370, 1370, 1370,
 /*  7500 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  536, 1370, 1370,
 /*  7510 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,   56, 1370,
 /*  7520 */  1370, 1370, 1370, 1370, 1370, 1370,  347,  728,  818,  805,
 /*  7530 */   549, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  312,
 /*  7540 */  1370, 1370, 1370, 1370,  194, 1370, 1370, 1370, 1370,  507,
 /*  7550 */   506,  505,  504, 1370,  888,  887,  886,  885,  884,  883,
 /*  7560 */   882,  881,  880,  879,  878,  877,  876,  875, 1370, 1370,
 /*  7570 */  1370, 1370,  806,  827,  826,  825,  824,  823,  822,  821,
 /*  7580 */   820,  819,  817,  816,  815,  814,  813,  812,  811,  810,
 /*  7590 */   809,  808,  807, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7600 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7610 */  1370, 1370, 1370, 1370,   55, 1370, 1370, 1370, 1370, 1370,
 /*  7620 */  1370, 1370, 1370,  728,  818,  805, 1370, 1370, 1370, 1370,
 /*  7630 */  1370, 1370, 1370, 1370, 1370,  312, 1370, 1370, 1370, 1370,
 /*  7640 */  1370, 1370, 1370, 1370, 1370,  507,  506,  505,  504,  806,
 /*  7650 */   827,  826,  825,  824,  823,  822,  821,  820,  819,  817,
 /*  7660 */   816,  815,  814,  813,  812,  811,  810,  809,  808,  807,
 /*  7670 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7680 */   536, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7690 */  1370,   54, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  431,
 /*  7700 */   728,  818,  805,  549, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7710 */  1370, 1370,  312, 1370, 1370, 1370, 1370,  194, 1370, 1370,
 /*  7720 */  1370, 1370,  507,  506,  505,  504, 1370,  888,  887,  886,
 /*  7730 */   885,  884,  883,  882,  881,  880,  879,  878,  877,  876,
 /*  7740 */   875, 1370, 1370, 1370, 1370,  806,  827,  826,  825,  824,
 /*  7750 */   823,  822,  821,  820,  819,  817,  816,  815,  814,  813,
 /*  7760 */   812,  811,  810,  809,  808,  807, 1370, 1370, 1370, 1370,
 /*  7770 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7780 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,   53, 1370, 1370,
 /*  7790 */  1370, 1370, 1370, 1370, 1370, 1370,  728,  818,  805, 1370,
 /*  7800 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  312, 1370,
 /*  7810 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  507,  506,
 /*  7820 */   505,  504,  806,  827,  826,  825,  824,  823,  822,  821,
 /*  7830 */   820,  819,  817,  816,  815,  814,  813,  812,  811,  810,
 /*  7840 */   809,  808,  807, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7850 */  1370, 1370, 1370,  536, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7860 */  1370, 1370, 1370, 1370,   51, 1370, 1370, 1370, 1370, 1370,
 /*  7870 */  1370, 1370,  426,  728,  818,  805,  549, 1370, 1370, 1370,
 /*  7880 */  1370, 1370, 1370, 1370, 1370,  312, 1370, 1370, 1370, 1370,
 /*  7890 */   194, 1370, 1370, 1370, 1370,  507,  506,  505,  504, 1370,
 /*  7900 */   888,  887,  886,  885,  884,  883,  882,  881,  880,  879,
 /*  7910 */   878,  877,  876,  875, 1370, 1370, 1370, 1370,  806,  827,
 /*  7920 */   826,  825,  824,  823,  822,  821,  820,  819,  817,  816,
 /*  7930 */   815,  814,  813,  812,  811,  810,  809,  808,  807, 1370,
 /*  7940 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7950 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7960 */    49, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  728,
 /*  7970 */   818,  805, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7980 */  1370,  312, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  7990 */  1370,  507,  506,  505,  504,  806,  827,  826,  825,  824,
 /*  8000 */   823,  822,  821,  820,  819,  817,  816,  815,  814,  813,
 /*  8010 */   812,  811,  810,  809,  808,  807, 1370, 1370, 1370, 1370,
 /*  8020 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  467,
 /*  8030 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,   48, 1370, 1370,
 /*  8040 */  1370, 1370, 1370, 1370, 1370, 1370,  728,  818,  805, 1370,
 /*  8050 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  312, 1370,
 /*  8060 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  507,  506,
 /*  8070 */   505,  504,  667,  664,  663, 1370,  888,  887,  886,  885,
 /*  8080 */   884,  883,  882,  881,  880,  879,  878,  877,  876,  875,
 /*  8090 */  1370,  806,  827,  826,  825,  824,  823,  822,  821,  820,
 /*  8100 */   819,  817,  816,  815,  814,  813,  812,  811,  810,  809,
 /*  8110 */   808,  807, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8120 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8130 */  1370, 1370, 1370,   47, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8140 */  1370, 1370,  728,  818,  805, 1370, 1370, 1370, 1370, 1370,
 /*  8150 */  1370, 1370, 1370, 1370,  312, 1370, 1370, 1370, 1370, 1370,
 /*  8160 */  1370, 1370, 1370, 1370,  507,  506,  505,  504,  806,  827,
 /*  8170 */   826,  825,  824,  823,  822,  821,  820,  819,  817,  816,
 /*  8180 */   815,  814,  813,  812,  811,  810,  809,  808,  807, 1370,
 /*  8190 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  536,
 /*  8200 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8210 */    46, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  728,
 /*  8220 */   818,  805,  549, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8230 */  1370,  312, 1370, 1370, 1370, 1370,  194, 1370, 1370, 1370,
 /*  8240 */  1370,  507,  506,  505,  504, 1370,  888,  887,  886,  885,
 /*  8250 */   884,  883,  882,  881,  880,  879,  878,  877,  876,  875,
 /*  8260 */  1370, 1370, 1370, 1370,  806,  827,  826,  825,  824,  823,
 /*  8270 */   822,  821,  820,  819,  817,  816,  815,  814,  813,  812,
 /*  8280 */   811,  810,  809,  808,  807, 1370, 1370, 1370, 1370, 1370,
 /*  8290 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8300 */  1370, 1370, 1370, 1370, 1370, 1370,   45, 1370, 1370, 1370,
 /*  8310 */  1370, 1370, 1370, 1370, 1370,  728,  818,  805, 1370, 1370,
 /*  8320 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  312, 1370, 1370,
 /*  8330 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  507,  506,  505,
 /*  8340 */   504,  806,  827,  826,  825,  824,  823,  822,  821,  820,
 /*  8350 */   819,  817,  816,  815,  814,  813,  812,  811,  810,  809,
 /*  8360 */   808,  807, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8370 */  1370, 1370, 1370, 1370,  467, 1370, 1370, 1370, 1370, 1370,
 /*  8380 */  1370, 1370, 1370,   44, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8390 */  1370, 1370,  728,  818,  805, 1370, 1370, 1370, 1370, 1370,
 /*  8400 */  1370, 1370, 1370, 1370,  312, 1370, 1370, 1370, 1370, 1370,
 /*  8410 */  1370, 1370, 1370, 1370,  507,  506,  505,  504,  664,  663,
 /*  8420 */  1370,  888,  887,  886,  885,  884,  883,  882,  881,  880,
 /*  8430 */   879,  878,  877,  876,  875, 1370, 1370,  806,  827,  826,
 /*  8440 */   825,  824,  823,  822,  821,  820,  819,  817,  816,  815,
 /*  8450 */   814,  813,  812,  811,  810,  809,  808,  807, 1370, 1370,
 /*  8460 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8470 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,   43,
 /*  8480 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  728,  818,
 /*  8490 */   805, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8500 */   312, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8510 */   507,  506,  505,  504,  806,  827,  826,  825,  824,  823,
 /*  8520 */   822,  821,  820,  819,  817,  816,  815,  814,  813,  812,
 /*  8530 */   811,  810,  809,  808,  807, 1370, 1370, 1370, 1370, 1370,
 /*  8540 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8550 */  1370, 1370, 1370, 1370, 1370, 1370,   42, 1370, 1370, 1370,
 /*  8560 */  1370, 1370, 1370, 1370, 1370,  728,  818,  805, 1370, 1370,
 /*  8570 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  312, 1370, 1370,
 /*  8580 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  507,  506,  505,
 /*  8590 */   504, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8600 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8610 */   806,  827,  826,  825,  824,  823,  822,  821,  820,  819,
 /*  8620 */   817,  816,  815,  814,  813,  812,  811,  810,  809,  808,
 /*  8630 */   807, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8640 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8650 */  1370, 1370,   41, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8660 */  1370,  728,  818,  805, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8670 */  1370, 1370, 1370,  312, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8680 */  1370, 1370, 1370,  507,  506,  505,  504,  806,  827,  826,
 /*  8690 */   825,  824,  823,  822,  821,  820,  819,  817,  816,  815,
 /*  8700 */   814,  813,  812,  811,  810,  809,  808,  807, 1370, 1370,
 /*  8710 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8720 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,   40,
 /*  8730 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  728,  818,
 /*  8740 */   805, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8750 */   312, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8760 */   507,  506,  505,  504, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8770 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8780 */  1370, 1370, 1370,  806,  827,  826,  825,  824,  823,  822,
 /*  8790 */   821,  820,  819,  817,  816,  815,  814,  813,  812,  811,
 /*  8800 */   810,  809,  808,  807, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8810 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8820 */  1370, 1370, 1370, 1370, 1370,   39, 1370, 1370, 1370, 1370,
 /*  8830 */  1370, 1370, 1370, 1370,  728,  818,  805, 1370, 1370, 1370,
 /*  8840 */  1370, 1370, 1370, 1370, 1370, 1370,  312, 1370, 1370, 1370,
 /*  8850 */  1370, 1370, 1370, 1370, 1370, 1370,  507,  506,  505,  504,
 /*  8860 */   806,  827,  826,  825,  824,  823,  822,  821,  820,  819,
 /*  8870 */   817,  816,  815,  814,  813,  812,  811,  810,  809,  808,
 /*  8880 */   807, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8890 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8900 */  1370, 1370,   38, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8910 */  1370,  728,  818,  805, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8920 */  1370, 1370, 1370,  312, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8930 */  1370, 1370, 1370,  507,  506,  505,  504, 1370, 1370, 1370,
 /*  8940 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8950 */  1370, 1370, 1370, 1370, 1370, 1370,  806,  827,  826,  825,
 /*  8960 */   824,  823,  822,  821,  820,  819,  817,  816,  815,  814,
 /*  8970 */   813,  812,  811,  810,  809,  808,  807, 1370, 1370, 1370,
 /*  8980 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  8990 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,   37, 1370,
 /*  9000 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  728,  818,  805,
 /*  9010 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  312,
 /*  9020 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  507,
 /*  9030 */   506,  505,  504,  806,  827,  826,  825,  824,  823,  822,
 /*  9040 */   821,  820,  819,  817,  816,  815,  814,  813,  812,  811,
 /*  9050 */   810,  809,  808,  807, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9060 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9070 */  1370, 1370, 1370, 1370, 1370,   35, 1370, 1370, 1370, 1370,
 /*  9080 */  1370, 1370, 1370, 1370,  728,  818,  805, 1370, 1370, 1370,
 /*  9090 */  1370, 1370, 1370, 1370, 1370, 1370,  312, 1370, 1370, 1370,
 /*  9100 */  1370, 1370, 1370, 1370, 1370, 1370,  507,  506,  505,  504,
 /*  9110 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9120 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  806,
 /*  9130 */   827,  826,  825,  824,  823,  822,  821,  820,  819,  817,
 /*  9140 */   816,  815,  814,  813,  812,  811,  810,  809,  808,  807,
 /*  9150 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9160 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9170 */  1370,   33, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9180 */   728,  818,  805, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9190 */  1370, 1370,  312, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9200 */  1370, 1370,  507,  506,  505,  504,  806,  827,  826,  825,
 /*  9210 */   824,  823,  822,  821,  820,  819,  817,  816,  815,  814,
 /*  9220 */   813,  812,  811,  810,  809,  808,  807, 1370,  138, 1370,
 /*  9230 */  1370,  139,  145,  146,  143,  144,  142,  141,  140,  168,
 /*  9240 */   164,  160,  158,  156,  154,  162,  166,  152,  151,  147,
 /*  9250 */   150,  149,  148, 1370, 1370, 1370, 1370,  828,  818,  805,
 /*  9260 */  1370,  153,  416, 1370, 1370, 1370, 1370, 1370, 1370,  312,
 /*  9270 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  507,
 /*  9280 */   506,  505,  504, 1370, 1370, 1370, 1370,  163, 1370,  157,
 /*  9290 */   155, 1370, 1370, 1370, 1370,  132, 1370, 1370, 1370, 1370,
 /*  9300 */  1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,  167,  137,
 /*  9310 */  1370, 1370, 1370,  165,  558,  159,  133,  161,  406,  169,
 /*  9320 */   674,   64, 1370,  163,  352,  157,  155, 1370, 1370, 1370,
 /*  9330 */  1370,  132, 1370, 1370,  552, 1370, 1370,  561, 1370,  865,
 /*  9340 */   550,   81, 1370, 1370,  167,  868, 1370, 1370, 1370,  165,
 /*  9350 */   558,  159,  133,  161,  406,  169,  680,   64, 1370, 1370,
 /*  9360 */   352, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9370 */   552, 1370, 1370, 1370, 1370,  865,  550,   81, 1370, 1370,
 /*  9380 */  1370,  868, 1370, 1370, 1370, 1370,  871, 1370, 1370, 1370,
 /*  9390 */  1370, 1370, 1370, 1370,  872,  870,  869,  867,  866,  145,
 /*  9400 */   146,  143,  144,  142,  141,  140,  168,  164,  160,  158,
 /*  9410 */   156,  154,  162,  166,  152,  151,  147,  150,  149,  148,
 /*  9420 */  1370, 1370,  871, 1370, 1370, 1370, 1370, 1370,  153,  416,
 /*  9430 */   872,  870,  869,  867,  866, 1370, 1370, 1370,  163, 1370,
 /*  9440 */   157,  155, 1370, 1370, 1370, 1370,  132, 1370, 1370, 1370,
 /*  9450 */  1370, 1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,  167,
 /*  9460 */  1370, 1370, 1370, 1370,  165,  558,  159,  133,  161,  406,
 /*  9470 */   169,  683,   64, 1370,  163,  352,  157,  155, 1370, 1370,
 /*  9480 */  1370, 1370,  132, 1370, 1370,  552, 1370, 1370,  561, 1370,
 /*  9490 */   865,  550,   81, 1370, 1370,  167,  868, 1370, 1370, 1370,
 /*  9500 */   165,  558,  159,  133,  161,  406,  169,  862,   64, 1370,
 /*  9510 */  1370,  352, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9520 */  1370,  552, 1370, 1370, 1370, 1370,  865,  550,   81, 1370,
 /*  9530 */  1370, 1370,  868, 1370, 1370, 1370, 1370,  871, 1370, 1370,
 /*  9540 */  1370, 1370, 1370, 1370, 1370,  872,  870,  869,  867,  866,
 /*  9550 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9560 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9570 */  1370, 1370, 1370,  871, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9580 */  1370,  872,  870,  869,  867,  866, 1370, 1370, 1370,  163,
 /*  9590 */  1370,  157,  155, 1370, 1370, 1370, 1370,  132, 1370, 1370,
 /*  9600 */  1370, 1370, 1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9610 */   167, 1370, 1370, 1370, 1370,  165,  558,  159,  133,  161,
 /*  9620 */   406,  169,  859,   64, 1370,  163,  352,  157,  155, 1370,
 /*  9630 */  1370, 1370, 1370,  132, 1370, 1370,  552, 1370, 1370,  561,
 /*  9640 */  1370,  865,  550,   81, 1370, 1370,  167,  868, 1370, 1370,
 /*  9650 */  1370,  165,  558,  159,  133,  161,  406,  169,  694,   64,
 /*  9660 */  1370, 1370,  352, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9670 */  1370, 1370,  552, 1370, 1370, 1370, 1370,  865,  550,   81,
 /*  9680 */  1370, 1370, 1370,  868, 1370, 1370, 1370, 1370,  871, 1370,
 /*  9690 */  1370, 1370, 1370, 1370, 1370, 1370,  872,  870,  869,  867,
 /*  9700 */   866, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9710 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9720 */  1370, 1370, 1370, 1370,  871, 1370, 1370, 1370, 1370, 1370,
 /*  9730 */  1370, 1370,  872,  870,  869,  867,  866, 1370, 1370, 1370,
 /*  9740 */   163, 1370,  157,  155, 1370, 1370, 1370, 1370,  132, 1370,
 /*  9750 */  1370, 1370, 1370, 1370,  561, 1370, 1370, 1370, 1370, 1370,
 /*  9760 */  1370,  167, 1370, 1370, 1370, 1370,  165,  558,  159,  133,
 /*  9770 */   161,  406,  169,  692,   64, 1370,  163,  352,  157,  155,
 /*  9780 */  1370, 1370, 1370, 1370,  132, 1370, 1370,  552, 1370, 1370,
 /*  9790 */   561, 1370,  865,  550,   81, 1370, 1370,  167,  868, 1370,
 /*  9800 */  1370, 1370,  165,  558,  159,  133,  161,  406,  169,  690,
 /*  9810 */    64, 1370, 1370,  352, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9820 */  1370, 1370, 1370,  552, 1370, 1370, 1370, 1370,  865,  550,
 /*  9830 */    81, 1370, 1370, 1370,  868, 1370, 1370, 1370, 1370,  871,
 /*  9840 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  872,  870,  869,
 /*  9850 */   867,  866, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9860 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /*  9870 */  1370, 1370, 1370, 1370, 1370,  871, 1370, 1370, 1370, 1370,
 /*  9880 */  1370, 1370, 1370,  872,  870,  869,  867,  866, 1370, 1370,
 /*  9890 */  1370,  163, 1370,  157,  155, 1370, 1370, 1370, 1370,  132,
 /*  9900 */  1370, 1370, 1370, 1370, 1370,  561, 1370, 1370, 1370, 1370,
 /*  9910 */  1370, 1370,  167, 1370, 1370, 1370, 1370,  165,  558,  159,
 /*  9920 */   133,  161,  406,  169,  678,   64, 1370,  163,  352,  157,
 /*  9930 */   155, 1370, 1370, 1370, 1370,  132, 1370, 1370,  552, 1370,
 /*  9940 */  1370,  561, 1370,  865,  550,   81, 1370, 1370,  167,  868,
 /*  9950 */  1370, 1370, 1370,  165,  558,  159,  133,  161,  406,  169,
 /*  9960 */   676,   64, 1370, 1370,  352, 1370, 1370, 1370, 1370, 1370,
 /*  9970 */  1370, 1370, 1370, 1370,  552, 1370, 1370, 1370, 1370,  865,
 /*  9980 */   550,   81, 1370, 1370, 1370,  868, 1370, 1370, 1370, 1370,
 /*  9990 */   871, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  872,  870,
 /* 10000 */   869,  867,  866, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10010 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10020 */  1370, 1370, 1370, 1370, 1370, 1370,  871, 1370, 1370, 1370,
 /* 10030 */  1370, 1370, 1370, 1370,  872,  870,  869,  867,  866, 1370,
 /* 10040 */  1370, 1370, 1370,  138, 1370, 1370,  139,  145,  146,  143,
 /* 10050 */   144,  142,  141,  140,  168,  164,  160,  158,  156,  154,
 /* 10060 */   162,  166,  152,  151,  147,  150,  149,  148, 1370, 1370,
 /* 10070 */  1370, 1370, 1370, 1370, 1370,  874,  153,  416, 1370, 1370,
 /* 10080 */  1370, 1370, 1370, 1370,  138, 1370, 1370,  139,  145,  146,
 /* 10090 */   143,  144,  142,  141,  140,  168,  164,  160,  158,  156,
 /* 10100 */   154,  162,  166,  152,  151,  147,  150,  149,  148, 1370,
 /* 10110 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  153,  416, 1370,
 /* 10120 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10130 */    18, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  138,
 /* 10140 */  1370, 1370,  139,  145,  146,  143,  144,  142,  141,  140,
 /* 10150 */   168,  164,  160,  158,  156,  154,  162,  166,  152,  151,
 /* 10160 */   147,  150,  149,  148, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10170 */  1370, 1370,  153,  416, 1370, 1370,  749, 1370, 1370, 1370,
 /* 10180 */  1370,  138, 1370, 1370,  139,  145,  146,  143,  144,  142,
 /* 10190 */   141,  140,  168,  164,  160,  158,  156,  154,  162,  166,
 /* 10200 */   152,  151,  147,  150,  149,  148, 1370, 1370, 1370, 1370,
 /* 10210 */  1370, 1370, 1370, 1370,  153,  416, 1370, 1370,  747, 1370,
 /* 10220 */  1370, 1370, 1370,  138, 1370, 1370,  139,  145,  146,  143,
 /* 10230 */   144,  142,  141,  140,  168,  164,  160,  158,  156,  154,
 /* 10240 */   162,  166,  152,  151,  147,  150,  149,  148, 1370, 1370,
 /* 10250 */  1370, 1370, 1370, 1370, 1370, 1370,  153,  416, 1370, 1370,
 /* 10260 */   746, 1370, 1370, 1370, 1370,  138, 1370, 1370,  139,  145,
 /* 10270 */   146,  143,  144,  142,  141,  140,  168,  164,  160,  158,
 /* 10280 */   156,  154,  162,  166,  152,  151,  147,  150,  149,  148,
 /* 10290 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  153,  416,
 /* 10300 */  1370, 1370,  745, 1370, 1370, 1370, 1370,  138, 1370, 1370,
 /* 10310 */   139,  145,  146,  143,  144,  142,  141,  140,  168,  164,
 /* 10320 */   160,  158,  156,  154,  162,  166,  152,  151,  147,  150,
 /* 10330 */   149,  148, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10340 */   153,  416, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10350 */  1370, 1370, 1370,   31, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10360 */  1370, 1370,  138, 1370, 1370,  139,  145,  146,  143,  144,
 /* 10370 */   142,  141,  140,  168,  164,  160,  158,  156,  154,  162,
 /* 10380 */   166,  152,  151,  147,  150,  149,  148, 1370, 1370, 1370,
 /* 10390 */  1370, 1370, 1370, 1370, 1370,  153,  416, 1370, 1370, 1370,
 /* 10400 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,   17, 1370,
 /* 10410 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  138, 1370, 1370,
 /* 10420 */   139,  145,  146,  143,  144,  142,  141,  140,  168,  164,
 /* 10430 */   160,  158,  156,  154,  162,  166,  152,  151,  147,  150,
 /* 10440 */   149,  148, 1370, 1370,  163, 1370,  157,  155, 1370, 1370,
 /* 10450 */   153,  416,  126, 1370, 1370, 1370, 1370, 1370,  561, 1370,
 /* 10460 */  1370, 1370, 1370,   30, 1370,  167, 1370, 1370, 1370, 1370,
 /* 10470 */   165,  558,  159,  133,  161,  406,  169, 1370,   64, 1370,
 /* 10480 */   163,  411,  157,  155, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10490 */  1370,  552, 1370, 1370,  561, 1370,  865,  550,   81, 1370,
 /* 10500 */  1370,  167,  868, 1370, 1370, 1370,  165,  558,  159,  133,
 /* 10510 */   161,  406,  169, 1370,   64, 1370, 1370,  411, 1370, 1370,
 /* 10520 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  552, 1370, 1370,
 /* 10530 */  1370, 1370,  865,  550,   81, 1370, 1370, 1370,  868, 1370,
 /* 10540 */  1370, 1370, 1370,  871, 1370, 1370,  163, 1370,  157,  155,
 /* 10550 */  1370,  872,  870,  869,  867,  866, 1370, 1370, 1370, 1370,
 /* 10560 */   561, 1370, 1370, 1370, 1370, 1370, 1370,  167, 1370, 1370,
 /* 10570 */  1370,  128,  165,  558,  159,  133,  161,  406,  169,  871,
 /* 10580 */    64, 1370, 1370,  411,  748, 1370, 1370,  872,  870,  869,
 /* 10590 */   867,  866, 1370,  552, 1370, 1370, 1370, 1370,  865,  550,
 /* 10600 */    81, 1370, 1370, 1370,  868, 1370, 1370, 1370, 1370, 1370,
 /* 10610 */  1370, 1370,  163, 1370,  157,  155, 1370, 1370, 1370, 1370,
 /* 10620 */  1370, 1370, 1370, 1370, 1370, 1370,  561, 1370, 1370, 1370,
 /* 10630 */  1370, 1370, 1370,  167, 1370, 1370, 1370, 1370,  165,  558,
 /* 10640 */   159,  133,  161,  406,  169,  871,   64, 1370, 1370,  411,
 /* 10650 */  1370, 1370, 1370,  872,  870,  869,  867,  866, 1370,  552,
 /* 10660 */  1370, 1370, 1370, 1370,  865,  550,   81, 1370, 1370, 1370,
 /* 10670 */   868, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  163, 1370,
 /* 10680 */   157,  155, 1370, 1370, 1370, 1370,  132, 1370, 1370, 1370,
 /* 10690 */  1370, 1370,  561, 1370, 1370, 1370, 1370, 1370, 1370,  167,
 /* 10700 */  1370, 1370, 1370,  127,  165,  558,  159,  133,  161,  406,
 /* 10710 */   169,  871,   64, 1370,  163,  352,  157,  155, 1370,  872,
 /* 10720 */   870,  869,  867,  866, 1370,  552, 1370, 1370,  561, 1370,
 /* 10730 */   865,  550,   81, 1370, 1370,  167,  868, 1370, 1370, 1370,
 /* 10740 */   165,  558,  159,  133,  161,  406,  169, 1370,   64, 1370,
 /* 10750 */  1370,  411, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10760 */  1370,  552, 1370, 1370, 1370, 1370,  865,  550,   81, 1370,
 /* 10770 */  1370, 1370,  868, 1370,  271, 1370, 1370,  871, 1370, 1370,
 /* 10780 */   163, 1370,  157,  155, 1370,  872,  870,  869,  867,  866,
 /* 10790 */  1370, 1370, 1370, 1370,  561, 1370, 1370, 1370, 1370, 1370,
 /* 10800 */  1370,  167, 1370, 1370, 1370, 1370,  165,  558,  159,  133,
 /* 10810 */   161,  406,  169,  871,   64, 1370,  163,  411,  157,  155,
 /* 10820 */  1370,  872,  870,  869,  867,  866, 1370,  552, 1370, 1370,
 /* 10830 */   561, 1370,  865,  550,   81, 1370, 1370,  167,  868, 1370,
 /* 10840 */   270, 1370,  165,  558,  159,  133,  161,  406,  169, 1370,
 /* 10850 */    64, 1370, 1370,  411, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 10860 */  1370, 1370, 1370,  552, 1370, 1370, 1370, 1370,  865,  550,
 /* 10870 */    81, 1370, 1370, 1370,  868, 1370,  267, 1370, 1370,  871,
 /* 10880 */  1370, 1370,  163, 1370,  157,  155, 1370,  872,  870,  869,
 /* 10890 */   867,  866, 1370, 1370, 1370, 1370,  561, 1370, 1370, 1370,
 /* 10900 */  1370, 1370, 1370,  167, 1370, 1370, 1370, 1370,  165,  558,
 /* 10910 */   159,  133,  161,  406,  169,  871,   64, 1370,  163,  411,
 /* 10920 */   157,  155, 1370,  872,  870,  869,  867,  866, 1370,  552,
 /* 10930 */  1370, 1370,  561, 1370,  865,  550,   81, 1370, 1370,  167,
 /* 10940 */   868, 1370,  266, 1370,  165,  558,  159,  133,  161,  406,
 /* 10950 */   169, 1370,   64, 1370, 1370,  411, 1370, 1370, 1370, 1370,
 /* 10960 */  1370, 1370, 1370, 1370, 1370,  552, 1370, 1370, 1370, 1370,
 /* 10970 */   865,  550,   81, 1370, 1370, 1370,  868, 1370,  265, 1370,
 /* 10980 */  1370,  871, 1370, 1370,  163, 1370,  157,  155, 1370,  872,
 /* 10990 */   870,  869,  867,  866, 1370, 1370, 1370, 1370,  561, 1370,
 /* 11000 */  1370, 1370, 1370, 1370, 1370,  167, 1370, 1370, 1370, 1370,
 /* 11010 */   165,  558,  159,  133,  161,  406,  169,  871,   64, 1370,
 /* 11020 */   163,  411,  157,  155, 1370,  872,  870,  869,  867,  866,
 /* 11030 */  1370,  552, 1370, 1370,  561, 1370,  865,  550,   81, 1370,
 /* 11040 */  1370,  167,  868, 1370,  264, 1370,  165,  558,  159,  133,
 /* 11050 */   161,  406,  169, 1370,   64, 1370, 1370,  351, 1370, 1370,
 /* 11060 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  552, 1370, 1370,
 /* 11070 */  1370, 1370,  554,  550,   81, 1370, 1370, 1370,  868, 1370,
 /* 11080 */   864, 1370, 1370,  871, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 11090 */  1370,  872,  870,  869,  867,  866, 1370, 1370, 1370, 1370,
 /* 11100 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 11110 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  555,
 /* 11120 */  1370, 1370, 1370, 1370, 1370, 1370, 1370,  556,  870,  869,
 /* 11130 */   867,  866, 1370, 1370, 1370, 1370,  138, 1370, 1370,  139,
 /* 11140 */   145,  146,  143,  144,  142,  141,  140,  168,  164,  160,
 /* 11150 */   158,  156,  154,  162,  166,  152,  151,  147,  150,  149,
 /* 11160 */   148,  163, 1370,  157,  155, 1370, 1370, 1370, 1370,  153,
 /* 11170 */   416, 1370, 1370, 1370, 1370,  561, 1370, 1370, 1370, 1370,
 /* 11180 */  1370, 1370,  167, 1370, 1370, 1370, 1370,  165,  558,  159,
 /* 11190 */   133,  161,  406,  169, 1370,   64, 1370,  163,  411,  157,
 /* 11200 */   155, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  552, 1370,
 /* 11210 */  1370,  561, 1370,  865,  550,   81, 1370, 1370,  167,  868,
 /* 11220 */  1370, 1370, 1370,  165,  558,  159,  133,  161,  406,  169,
 /* 11230 */  1370,   64, 1370, 1370,  351, 1370, 1370, 1370, 1370, 1370,
 /* 11240 */  1370, 1370, 1370, 1370,  552, 1370, 1370, 1370, 1370,  554,
 /* 11250 */   550,   81, 1370, 1370, 1370,  868, 1370, 1370, 1370, 1370,
 /* 11260 */   871, 1370, 1370, 1370, 1370, 1370, 1370, 1370,  872,  870,
 /* 11270 */   869,  867,  866, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 11280 */  1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370, 1370,
 /* 11290 */  1370, 1370, 1370, 1370, 1370, 1370,  555, 1370, 1370, 1370,
 /* 11300 */  1370, 1370, 1370, 1370,  556,  870,  869,  867,  866,
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
 /*   570 */     6,  173,  193,  175,  176,  177,  178,  179,   55,   21,
 /*   580 */   201,  202,  203,  204,  205,  206,   28,    6,   45,  108,
 /*   590 */   109,   33,   34,   35,   36,   37,   38,   39,   42,   41,
 /*   600 */     6,   45,   44,   45,    1,    2,    3,    4,    5,   45,
 /*   610 */    61,   45,   54,   55,   56,   45,   57,   59,   60,   61,
 /*   620 */    54,   40,   42,   65,   54,   45,   68,   69,   70,   71,
 /*   630 */    72,   73,   74,   75,   76,   77,   78,  164,  107,   45,
 /*   640 */    82,   83,   84,    6,    6,   87,    6,    6,    6,   91,
 /*   650 */    92,   93,   94,   45,   96,   52,   53,   99,   49,   56,
 /*   660 */    51,   58,   54,   54,  106,   62,   63,   89,  110,  111,
 /*   670 */   112,  113,  114,  115,  116,  117,  118,  119,    7,   59,
 /*   680 */     9,   10,   45,   45,  193,   45,   45,   45,  215,  216,
 /*   690 */   217,  218,   21,   57,  203,  204,  205,  206,    6,   28,
 /*   700 */    42,  210,  211,   45,   33,   34,   35,   36,   37,   38,
 /*   710 */    39,    6,   41,    6,   44,   44,   45,    1,    2,    3,
 /*   720 */     4,    5,  189,  190,   89,   54,   55,   56,   89,   59,
 /*   730 */    59,   60,   61,   85,   86,   51,   65,   45,   54,   68,
 /*   740 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   750 */    45,   49,   45,   82,   83,   84,   54,   45,   87,   85,
 /*   760 */    86,   57,   91,   92,   93,   94,   54,   96,   52,   53,
 /*   770 */    99,   49,   56,   51,   58,   44,   54,  106,   62,   63,
 /*   780 */    57,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*   790 */   119,    7,  160,    9,   10,  163,   57,  193,   44,    6,
 /*   800 */   168,  169,    6,   22,   45,   21,  174,  203,  204,  205,
 /*   810 */   206,   44,   28,   54,  210,  211,   57,   33,   34,   35,
 /*   820 */    36,   37,   38,   39,  163,   41,  126,   22,   44,   45,
 /*   830 */     1,    2,    3,    4,    5,  174,   40,   57,   54,   55,
 /*   840 */    56,  191,  192,   59,   60,   61,   41,   54,   44,   65,
 /*   850 */    44,  151,   68,   69,   70,   71,   72,   73,   74,   75,
 /*   860 */    76,   77,   78,   45,   57,  184,   82,   83,   84,   57,
 /*   870 */    51,   87,   54,   54,  193,   91,   92,   93,   94,   54,
 /*   880 */    96,   52,   53,   99,  203,  204,  205,  206,   45,   60,
 /*   890 */   106,   62,   63,   45,  110,  111,  112,  113,  114,  115,
 /*   900 */   116,  117,  118,  119,    7,   45,    9,   10,  207,  208,
 /*   910 */   193,   49,   54,   51,   54,   54,   54,   57,   21,   54,
 /*   920 */   203,  204,  205,  206,  154,   28,  156,  210,  211,    6,
 /*   930 */    33,   34,   35,   36,   37,   38,   39,   54,   41,  207,
 /*   940 */   208,   44,   45,    1,    2,    3,    4,    5,  207,  208,
 /*   950 */    54,   54,   55,   56,  207,  208,   59,   60,   61,  207,
 /*   960 */   208,  196,   65,   40,  199,   68,   69,   70,   71,   72,
 /*   970 */    73,   74,   75,   76,   77,   78,  207,  208,  184,   82,
 /*   980 */    83,   84,  207,  208,   87,  207,  208,  193,   91,   92,
 /*   990 */    93,   94,   89,   96,   52,   53,   99,  203,  204,  205,
 /*  1000 */   206,   54,   60,  106,   62,   63,   92,  110,  111,  112,
 /*  1010 */   113,  114,  115,  116,  117,  118,  119,    7,    6,    9,
 /*  1020 */    10,  207,  208,  193,  207,  208,  163,    6,  207,  208,
 /*  1030 */   154,   21,  156,  203,  204,  205,  206,  174,   28,   45,
 /*  1040 */   210,  211,    6,   33,   34,   35,   36,   37,   38,   39,
 /*  1050 */   196,   41,   40,  199,   44,   45,    1,    2,    3,    4,
 /*  1060 */     5,   40,  185,  186,   54,   55,   56,   85,   86,   59,
 /*  1070 */    60,   61,  185,  186,  196,   65,   40,  199,   68,   69,
 /*  1080 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   85,
 /*  1090 */    86,  184,   82,   83,   84,  187,  188,   87,  166,  167,
 /*  1100 */   193,   91,   92,   93,   94,   54,   96,   52,   53,   99,
 /*  1110 */   203,  204,  205,  206,   44,   60,  106,   62,   63,   67,
 /*  1120 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  1130 */     7,    6,    9,   10,  194,  195,  193,  177,  178,  163,
 /*  1140 */     6,   55,   55,   44,   21,   44,  203,  204,  205,  206,
 /*  1150 */   174,   28,   44,  210,  211,    6,   33,   34,   35,   36,
 /*  1160 */    37,   38,   39,   45,   41,   40,   55,   44,   45,    1,
 /*  1170 */     2,    3,    4,    5,   40,   55,   45,   54,   55,   56,
 /*  1180 */    45,   45,   59,   60,   61,   57,   44,   97,   65,   40,
 /*  1190 */    44,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  1200 */    77,   78,   54,   45,  184,   82,   83,   84,   92,   54,
 /*  1210 */    87,   89,   55,  193,   91,   92,   93,   94,   54,   96,
 /*  1220 */    52,   53,   99,  203,  204,  205,  206,   54,   60,  106,
 /*  1230 */    62,   63,   54,  110,  111,  112,  113,  114,  115,  116,
 /*  1240 */   117,  118,  119,    7,    6,    9,   10,   54,   61,  193,
 /*  1250 */    44,   55,   61,    6,   44,   55,   61,   21,   55,  203,
 /*  1260 */   204,  205,  206,   89,   28,   61,  210,  211,    6,   33,
 /*  1270 */    34,   35,   36,   37,   38,   39,   55,   41,   40,   61,
 /*  1280 */    44,   45,    1,    2,    3,    4,    5,   40,   44,   44,
 /*  1290 */    54,   55,   56,   44,   55,   59,   60,   61,   44,   14,
 /*  1300 */    39,   65,   40,    6,   68,   69,   70,   71,   72,   73,
 /*  1310 */    74,   75,   76,   77,   78,   45,   61,   44,   82,   83,
 /*  1320 */    84,   22,   45,   87,   67,   61,    6,   91,   92,   93,
 /*  1330 */    94,   44,   96,   52,   53,   99,   45,   40,   45,   58,
 /*  1340 */    57,   45,  106,   62,   63,   57,  110,  111,  112,  113,
 /*  1350 */   114,  115,  116,  117,  118,  119,    7,   45,    9,   10,
 /*  1360 */    40,   57,  193,   45,   57,   55,   45,   44,   44,   44,
 /*  1370 */    21,   44,  203,  204,  205,  206,   61,   28,   61,  210,
 /*  1380 */   211,   45,   33,   34,   35,   36,   37,   38,   39,   44,
 /*  1390 */    41,   45,   45,   44,   45,    1,    2,    3,    4,    5,
 /*  1400 */    45,   50,   44,   54,   55,   56,   44,   50,   59,   60,
 /*  1410 */    61,   44,   44,   44,   65,   61,   60,   68,   69,   70,
 /*  1420 */    71,   72,   73,   74,   75,   76,   77,   78,   54,   54,
 /*  1430 */    44,   82,   83,   84,   61,   54,   87,   54,  190,  126,
 /*  1440 */    91,   92,   93,   94,   44,   96,   52,   53,   99,  208,
 /*  1450 */    56,   44,  192,   44,   95,  106,   62,   63,  152,  110,
 /*  1460 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  1470 */    54,    9,   10,   44,   54,  193,  186,  188,  156,  153,
 /*  1480 */   152,  165,  153,   21,  153,  203,  204,  205,  206,  195,
 /*  1490 */    28,  153,  210,  211,  153,   33,   34,   35,   36,   37,
 /*  1500 */    38,   39,  167,   41,  165,  153,   44,   45,    1,    2,
 /*  1510 */     3,    4,    5,  153,  153,  165,   54,   55,   56,  153,
 /*  1520 */   219,   59,   60,   61,  153,  153,  196,   65,  165,  196,
 /*  1530 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  1540 */    78,  164,  164,  164,   82,   83,   84,  219,  164,   87,
 /*  1550 */   174,  219,  164,   91,   92,   93,   94,  219,   96,   52,
 /*  1560 */    53,   99,  219,   56,  219,  164,  196,  164,  106,   62,
 /*  1570 */    63,  219,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  1580 */   118,  119,    7,  196,    9,   10,  196,  196,  193,  196,
 /*  1590 */   164,  164,  196,  196,  196,  164,   21,  164,  203,  204,
 /*  1600 */   205,  206,  164,   28,  164,  210,  211,  164,   33,   34,
 /*  1610 */    35,   36,   37,   38,   39,  164,   41,  164,  219,   44,
 /*  1620 */    45,    1,    2,    3,    4,    5,  219,  219,  219,   54,
 /*  1630 */    55,   56,  219,  219,   59,   60,   61,  219,  219,  219,
 /*  1640 */    65,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  1650 */    75,   76,   77,   78,  219,  219,  219,   82,   83,   84,
 /*  1660 */   219,  219,   87,  219,  219,  219,   91,   92,   93,   94,
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
 /*  6150 */   219,   56,  219,  219,   59,   60,   61,  219,  219,  219,
 /*  6160 */    65,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  6170 */    75,   76,   77,   78,  219,  219,  219,   82,   83,   84,
 /*  6180 */   219,  219,   87,  219,  219,  219,   91,   92,   93,   94,
 /*  6190 */     7,   96,  219,  219,   99,  219,  219,  219,  219,  219,
 /*  6200 */   219,  106,  219,  219,  219,  110,  111,  112,  113,  114,
 /*  6210 */   115,  116,  117,  118,  119,  219,  219,   34,  219,  219,
 /*  6220 */   219,  219,  219,  219,  219,  219,   43,   44,  219,   46,
 /*  6230 */   219,   48,  219,   50,  219,   52,   53,   54,  219,   56,
 /*  6240 */   219,  219,  219,   60,  219,  219,  219,  219,  219,  219,
 /*  6250 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  6260 */    77,   78,   79,   80,   81,   82,   83,   84,  219,  219,
 /*  6270 */    87,  219,  219,  219,   91,   92,   93,   94,    7,   96,
 /*  6280 */     9,   10,   99,  219,  219,  219,  219,  219,  219,  219,
 /*  6290 */   219,  219,   21,  110,  111,  112,  113,  219,  219,   28,
 /*  6300 */   219,  219,  119,  219,   33,   34,   35,   36,   37,   38,
 /*  6310 */    39,  219,   41,  219,  219,   44,  219,  219,  219,  219,
 /*  6320 */   219,  219,  219,  219,  219,   54,  219,  219,  219,  219,
 /*  6330 */    59,   60,   61,  219,  219,  219,   65,  219,  219,   68,
 /*  6340 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  6350 */    79,   80,   81,   11,   12,   13,   14,   15,   16,   17,
 /*  6360 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*  6370 */    28,   29,   30,   31,   32,  219,  219,  106,  219,  219,
 /*  6380 */   219,  219,  219,   41,   42,  114,  115,  116,  117,  118,
 /*  6390 */   219,  219,  219,  219,  219,  219,  121,  122,  123,  124,
 /*  6400 */   125,  126,  127,  128,  129,  130,  131,  132,  133,  134,
 /*  6410 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  6420 */   145,  146,  147,  148,  149,  150,    8,  219,  219,   11,
 /*  6430 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /*  6440 */    22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
 /*  6450 */    32,  219,  177,  219,  219,  219,  219,  219,  219,   41,
 /*  6460 */    42,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*  6470 */    28,   29,   30,   31,   32,  219,  193,  219,  219,  204,
 /*  6480 */   205,  206,  219,   41,   42,   67,  203,  204,  205,  206,
 /*  6490 */   219,  219,  209,  219,  219,  212,  213,  214,  219,  219,
 /*  6500 */   219,  219,  124,  125,  126,  127,  128,  129,  130,  131,
 /*  6510 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  6520 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  6530 */   219,  219,  219,  219,  130,  131,  132,  133,  134,  135,
 /*  6540 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  6550 */   146,  147,  148,  149,  150,  177,  219,  219,  193,  219,
 /*  6560 */   219,  219,  219,  219,  219,  219,  219,  219,  203,  204,
 /*  6570 */   205,  206,  219,  219,  219,  219,  172,  212,  213,  214,
 /*  6580 */   219,  219,  204,  205,  206,  181,  182,  183,  219,  219,
 /*  6590 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  6600 */   219,  219,  219,  219,  219,  219,  219,  203,  204,  205,
 /*  6610 */   206,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  6620 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  6630 */   149,  150,   13,   14,   15,   16,   17,   18,   19,   20,
 /*  6640 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /*  6650 */    31,   32,  219,  172,  219,   57,  219,  219,  219,  219,
 /*  6660 */    41,   42,  181,  182,  183,  219,  219,  219,  219,  219,
 /*  6670 */   219,  219,  219,  219,  193,  219,  219,  219,  219,  219,
 /*  6680 */   219,  219,  219,  219,  203,  204,  205,  206,  219,  219,
 /*  6690 */   219,  219,  219,  219,  219,  219,  219,  219,  100,  101,
 /*  6700 */   102,  103,  104,  105,  219,  219,  219,  130,  131,  132,
 /*  6710 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  6720 */   143,  144,  145,  146,  147,  148,  149,  150,   14,   15,
 /*  6730 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*  6740 */    26,   27,   28,   29,   30,   31,   32,  219,  219,  172,
 /*  6750 */    41,   42,  219,  219,   41,   41,   42,   44,  181,  182,
 /*  6760 */   183,   41,  219,  219,  219,  219,   57,  219,  219,  219,
 /*  6770 */   193,  219,   59,  219,  219,  219,  219,   57,   65,  219,
 /*  6780 */   203,  204,  205,  206,  130,  131,  132,  133,  134,  135,
 /*  6790 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  6800 */   146,  147,  148,  149,  150,  219,  219,  219,  219,  100,
 /*  6810 */   101,  102,  103,  104,  105,   21,  107,  108,  109,  106,
 /*  6820 */   100,  101,  102,  103,  104,  105,  172,  114,  115,  116,
 /*  6830 */   117,  118,  219,  219,   40,  181,  182,  183,   44,  219,
 /*  6840 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  6850 */   219,  219,   58,  219,  219,  219,   21,  203,  204,  205,
 /*  6860 */   206,  219,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  6870 */    76,   77,   78,   79,   80,   81,  219,  219,  219,   44,
 /*  6880 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  6890 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  6900 */   150,  219,  219,   68,   69,   70,   71,   72,   73,   74,
 /*  6910 */    75,   76,   77,   78,   79,   80,   81,  219,  219,  219,
 /*  6920 */   219,   41,  172,  219,   44,  219,  219,  219,  219,  219,
 /*  6930 */   219,  181,  182,  183,  219,  219,  219,  219,  219,   59,
 /*  6940 */   219,  219,  219,  193,  219,   65,  219,   67,  219,  219,
 /*  6950 */   219,  219,  219,  203,  204,  205,  206,  130,  131,  132,
 /*  6960 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  6970 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /*  6980 */   219,  219,  219,  219,  219,  219,  106,  219,   21,  219,
 /*  6990 */   219,  219,  219,  219,  114,  115,  116,  117,  118,  172,
 /*  7000 */   219,  219,  219,  219,  219,  219,  219,   40,  181,  182,
 /*  7010 */   183,   44,  219,  219,  219,  219,  219,  219,   41,  219,
 /*  7020 */   193,  219,  219,  219,  219,   58,  219,  219,  219,  219,
 /*  7030 */   203,  204,  205,  206,   57,   68,   69,   70,   71,   72,
 /*  7040 */    73,   74,   75,   76,   77,   78,   79,   80,   81,  219,
 /*  7050 */   219,  219,  219,  130,  131,  132,  133,  134,  135,  136,
 /*  7060 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  7070 */   147,  148,  149,  150,  219,  219,  219,  100,  101,  102,
 /*  7080 */   103,  104,  105,  219,  219,  108,  109,  219,  219,   41,
 /*  7090 */   219,  219,   44,  219,  219,  172,   41,  219,  219,  219,
 /*  7100 */   219,  219,  219,  219,  181,  182,  183,   59,  219,  219,
 /*  7110 */   219,  219,   57,   65,  219,  219,  193,  219,  219,  219,
 /*  7120 */   219,  219,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  7130 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  7140 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  7150 */   150,  219,  219,  219,  106,  100,  101,  102,  103,  104,
 /*  7160 */   105,   21,  114,  115,  116,  117,  118,  219,  219,  219,
 /*  7170 */   219,  219,  172,  219,  219,  219,  219,  219,  219,  219,
 /*  7180 */    40,  181,  182,  183,   44,  219,  219,  219,  219,  219,
 /*  7190 */   219,   41,  219,  193,  219,  219,  219,  219,   58,  219,
 /*  7200 */   219,  219,  219,  203,  204,  205,  206,   57,   68,   69,
 /*  7210 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*  7220 */    80,   81,  219,  219,  219,  219,  130,  131,  132,  133,
 /*  7230 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  7240 */   144,  145,  146,  147,  148,  149,  150,  219,  219,  219,
 /*  7250 */   100,  101,  102,  103,  104,  105,  219,  219,  219,  219,
 /*  7260 */   219,  219,  219,  219,  219,  219,  219,  219,  172,   41,
 /*  7270 */   219,  219,  219,  219,  219,  219,  219,  181,  182,  183,
 /*  7280 */   219,  219,  219,  219,  219,   57,  219,  219,  219,  193,
 /*  7290 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  203,
 /*  7300 */   204,  205,  206,  130,  131,  132,  133,  134,  135,  136,
 /*  7310 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  7320 */   147,  148,  149,  150,  219,  219,  219,  219,  100,  101,
 /*  7330 */   102,  103,  104,  105,   21,  219,  219,  219,  219,  219,
 /*  7340 */   219,  219,  219,  219,  219,  172,  219,  219,  219,  219,
 /*  7350 */   219,  219,  219,   40,  181,  182,  183,   44,  219,  219,
 /*  7360 */   219,  219,  219,  219,  219,  219,  193,  219,  219,  219,
 /*  7370 */   219,   58,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  7380 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  7390 */    77,   78,   79,   80,   81,  219,  219,  219,  219,  130,
 /*  7400 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  7410 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  7420 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7430 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7440 */   219,  172,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7450 */   181,  182,  183,  219,  219,  219,  219,  219,  219,  219,
 /*  7460 */   219,  219,  193,  219,  219,  219,  219,  219,  219,  219,
 /*  7470 */   219,  219,  203,  204,  205,  206,  130,  131,  132,  133,
 /*  7480 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  7490 */   144,  145,  146,  147,  148,  149,  150,  219,  219,  219,
 /*  7500 */   219,  219,  219,  219,  219,  219,  219,   21,  219,  219,
 /*  7510 */   219,  219,  219,  219,  219,  219,  219,  219,  172,  219,
 /*  7520 */   219,  219,  219,  219,  219,  219,   40,  181,  182,  183,
 /*  7530 */    44,  219,  219,  219,  219,  219,  219,  219,  219,  193,
 /*  7540 */   219,  219,  219,  219,   58,  219,  219,  219,  219,  203,
 /*  7550 */   204,  205,  206,  219,   68,   69,   70,   71,   72,   73,
 /*  7560 */    74,   75,   76,   77,   78,   79,   80,   81,  219,  219,
 /*  7570 */   219,  219,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  7580 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  7590 */   148,  149,  150,  219,  219,  219,  219,  219,  219,  219,
 /*  7600 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7610 */   219,  219,  219,  219,  172,  219,  219,  219,  219,  219,
 /*  7620 */   219,  219,  219,  181,  182,  183,  219,  219,  219,  219,
 /*  7630 */   219,  219,  219,  219,  219,  193,  219,  219,  219,  219,
 /*  7640 */   219,  219,  219,  219,  219,  203,  204,  205,  206,  130,
 /*  7650 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  7660 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  7670 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7680 */    21,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7690 */   219,  172,  219,  219,  219,  219,  219,  219,  219,   40,
 /*  7700 */   181,  182,  183,   44,  219,  219,  219,  219,  219,  219,
 /*  7710 */   219,  219,  193,  219,  219,  219,  219,   58,  219,  219,
 /*  7720 */   219,  219,  203,  204,  205,  206,  219,   68,   69,   70,
 /*  7730 */    71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*  7740 */    81,  219,  219,  219,  219,  130,  131,  132,  133,  134,
 /*  7750 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  7760 */   145,  146,  147,  148,  149,  150,  219,  219,  219,  219,
 /*  7770 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7780 */   219,  219,  219,  219,  219,  219,  219,  172,  219,  219,
 /*  7790 */   219,  219,  219,  219,  219,  219,  181,  182,  183,  219,
 /*  7800 */   219,  219,  219,  219,  219,  219,  219,  219,  193,  219,
 /*  7810 */   219,  219,  219,  219,  219,  219,  219,  219,  203,  204,
 /*  7820 */   205,  206,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  7830 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  7840 */   148,  149,  150,  219,  219,  219,  219,  219,  219,  219,
 /*  7850 */   219,  219,  219,   21,  219,  219,  219,  219,  219,  219,
 /*  7860 */   219,  219,  219,  219,  172,  219,  219,  219,  219,  219,
 /*  7870 */   219,  219,   40,  181,  182,  183,   44,  219,  219,  219,
 /*  7880 */   219,  219,  219,  219,  219,  193,  219,  219,  219,  219,
 /*  7890 */    58,  219,  219,  219,  219,  203,  204,  205,  206,  219,
 /*  7900 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  7910 */    78,   79,   80,   81,  219,  219,  219,  219,  130,  131,
 /*  7920 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  7930 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  7940 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7950 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7960 */   172,  219,  219,  219,  219,  219,  219,  219,  219,  181,
 /*  7970 */   182,  183,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7980 */   219,  193,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  7990 */   219,  203,  204,  205,  206,  130,  131,  132,  133,  134,
 /*  8000 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  8010 */   145,  146,  147,  148,  149,  150,  219,  219,  219,  219,
 /*  8020 */   219,  219,  219,  219,  219,  219,  219,  219,  219,   21,
 /*  8030 */   219,  219,  219,  219,  219,  219,  219,  172,  219,  219,
 /*  8040 */   219,  219,  219,  219,  219,  219,  181,  182,  183,  219,
 /*  8050 */   219,  219,  219,  219,  219,  219,  219,  219,  193,  219,
 /*  8060 */   219,  219,  219,  219,  219,  219,  219,  219,  203,  204,
 /*  8070 */   205,  206,   64,   65,   66,  219,   68,   69,   70,   71,
 /*  8080 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*  8090 */   219,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  8100 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  8110 */   149,  150,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8120 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8130 */   219,  219,  219,  172,  219,  219,  219,  219,  219,  219,
 /*  8140 */   219,  219,  181,  182,  183,  219,  219,  219,  219,  219,
 /*  8150 */   219,  219,  219,  219,  193,  219,  219,  219,  219,  219,
 /*  8160 */   219,  219,  219,  219,  203,  204,  205,  206,  130,  131,
 /*  8170 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  8180 */   142,  143,  144,  145,  146,  147,  148,  149,  150,  219,
 /*  8190 */   219,  219,  219,  219,  219,  219,  219,  219,  219,   21,
 /*  8200 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8210 */   172,  219,  219,  219,  219,  219,  219,  219,  219,  181,
 /*  8220 */   182,  183,   44,  219,  219,  219,  219,  219,  219,  219,
 /*  8230 */   219,  193,  219,  219,  219,  219,   58,  219,  219,  219,
 /*  8240 */   219,  203,  204,  205,  206,  219,   68,   69,   70,   71,
 /*  8250 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*  8260 */   219,  219,  219,  219,  130,  131,  132,  133,  134,  135,
 /*  8270 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  8280 */   146,  147,  148,  149,  150,  219,  219,  219,  219,  219,
 /*  8290 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8300 */   219,  219,  219,  219,  219,  219,  172,  219,  219,  219,
 /*  8310 */   219,  219,  219,  219,  219,  181,  182,  183,  219,  219,
 /*  8320 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  8330 */   219,  219,  219,  219,  219,  219,  219,  203,  204,  205,
 /*  8340 */   206,  130,  131,  132,  133,  134,  135,  136,  137,  138,
 /*  8350 */   139,  140,  141,  142,  143,  144,  145,  146,  147,  148,
 /*  8360 */   149,  150,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8370 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /*  8380 */   219,  219,  219,  172,  219,  219,  219,  219,  219,  219,
 /*  8390 */   219,  219,  181,  182,  183,  219,  219,  219,  219,  219,
 /*  8400 */   219,  219,  219,  219,  193,  219,  219,  219,  219,  219,
 /*  8410 */   219,  219,  219,  219,  203,  204,  205,  206,   65,   66,
 /*  8420 */   219,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  8430 */    77,   78,   79,   80,   81,  219,  219,  130,  131,  132,
 /*  8440 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  8450 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /*  8460 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8470 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  172,
 /*  8480 */   219,  219,  219,  219,  219,  219,  219,  219,  181,  182,
 /*  8490 */   183,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8500 */   193,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8510 */   203,  204,  205,  206,  130,  131,  132,  133,  134,  135,
 /*  8520 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  8530 */   146,  147,  148,  149,  150,  219,  219,  219,  219,  219,
 /*  8540 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8550 */   219,  219,  219,  219,  219,  219,  172,  219,  219,  219,
 /*  8560 */   219,  219,  219,  219,  219,  181,  182,  183,  219,  219,
 /*  8570 */   219,  219,  219,  219,  219,  219,  219,  193,  219,  219,
 /*  8580 */   219,  219,  219,  219,  219,  219,  219,  203,  204,  205,
 /*  8590 */   206,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8600 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8610 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  8620 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  8630 */   150,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8640 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8650 */   219,  219,  172,  219,  219,  219,  219,  219,  219,  219,
 /*  8660 */   219,  181,  182,  183,  219,  219,  219,  219,  219,  219,
 /*  8670 */   219,  219,  219,  193,  219,  219,  219,  219,  219,  219,
 /*  8680 */   219,  219,  219,  203,  204,  205,  206,  130,  131,  132,
 /*  8690 */   133,  134,  135,  136,  137,  138,  139,  140,  141,  142,
 /*  8700 */   143,  144,  145,  146,  147,  148,  149,  150,  219,  219,
 /*  8710 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8720 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  172,
 /*  8730 */   219,  219,  219,  219,  219,  219,  219,  219,  181,  182,
 /*  8740 */   183,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8750 */   193,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8760 */   203,  204,  205,  206,  219,  219,  219,  219,  219,  219,
 /*  8770 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8780 */   219,  219,  219,  130,  131,  132,  133,  134,  135,  136,
 /*  8790 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  8800 */   147,  148,  149,  150,  219,  219,  219,  219,  219,  219,
 /*  8810 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8820 */   219,  219,  219,  219,  219,  172,  219,  219,  219,  219,
 /*  8830 */   219,  219,  219,  219,  181,  182,  183,  219,  219,  219,
 /*  8840 */   219,  219,  219,  219,  219,  219,  193,  219,  219,  219,
 /*  8850 */   219,  219,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  8860 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  8870 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  8880 */   150,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8890 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8900 */   219,  219,  172,  219,  219,  219,  219,  219,  219,  219,
 /*  8910 */   219,  181,  182,  183,  219,  219,  219,  219,  219,  219,
 /*  8920 */   219,  219,  219,  193,  219,  219,  219,  219,  219,  219,
 /*  8930 */   219,  219,  219,  203,  204,  205,  206,  219,  219,  219,
 /*  8940 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8950 */   219,  219,  219,  219,  219,  219,  130,  131,  132,  133,
 /*  8960 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  8970 */   144,  145,  146,  147,  148,  149,  150,  219,  219,  219,
 /*  8980 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  8990 */   219,  219,  219,  219,  219,  219,  219,  219,  172,  219,
 /*  9000 */   219,  219,  219,  219,  219,  219,  219,  181,  182,  183,
 /*  9010 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  193,
 /*  9020 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  203,
 /*  9030 */   204,  205,  206,  130,  131,  132,  133,  134,  135,  136,
 /*  9040 */   137,  138,  139,  140,  141,  142,  143,  144,  145,  146,
 /*  9050 */   147,  148,  149,  150,  219,  219,  219,  219,  219,  219,
 /*  9060 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9070 */   219,  219,  219,  219,  219,  172,  219,  219,  219,  219,
 /*  9080 */   219,  219,  219,  219,  181,  182,  183,  219,  219,  219,
 /*  9090 */   219,  219,  219,  219,  219,  219,  193,  219,  219,  219,
 /*  9100 */   219,  219,  219,  219,  219,  219,  203,  204,  205,  206,
 /*  9110 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9120 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  130,
 /*  9130 */   131,  132,  133,  134,  135,  136,  137,  138,  139,  140,
 /*  9140 */   141,  142,  143,  144,  145,  146,  147,  148,  149,  150,
 /*  9150 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9160 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9170 */   219,  172,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9180 */   181,  182,  183,  219,  219,  219,  219,  219,  219,  219,
 /*  9190 */   219,  219,  193,  219,  219,  219,  219,  219,  219,  219,
 /*  9200 */   219,  219,  203,  204,  205,  206,  130,  131,  132,  133,
 /*  9210 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  9220 */   144,  145,  146,  147,  148,  149,  150,  219,    8,  219,
 /*  9230 */   219,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*  9240 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*  9250 */    30,   31,   32,  219,  219,  219,  219,  181,  182,  183,
 /*  9260 */   219,   41,   42,  219,  219,  219,  219,  219,  219,  193,
 /*  9270 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  203,
 /*  9280 */   204,  205,  206,  219,  219,  219,  219,    7,  219,    9,
 /*  9290 */    10,  219,  219,  219,  219,   15,  219,  219,  219,  219,
 /*  9300 */   219,   21,  219,  219,  219,  219,  219,  219,   28,   89,
 /*  9310 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /*  9320 */    40,   41,  219,    7,   44,    9,   10,  219,  219,  219,
 /*  9330 */   219,   15,  219,  219,   54,  219,  219,   21,  219,   59,
 /*  9340 */    60,   61,  219,  219,   28,   65,  219,  219,  219,   33,
 /*  9350 */    34,   35,   36,   37,   38,   39,   40,   41,  219,  219,
 /*  9360 */    44,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9370 */    54,  219,  219,  219,  219,   59,   60,   61,  219,  219,
 /*  9380 */   219,   65,  219,  219,  219,  219,  106,  219,  219,  219,
 /*  9390 */   219,  219,  219,  219,  114,  115,  116,  117,  118,   12,
 /*  9400 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*  9410 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /*  9420 */   219,  219,  106,  219,  219,  219,  219,  219,   41,   42,
 /*  9430 */   114,  115,  116,  117,  118,  219,  219,  219,    7,  219,
 /*  9440 */     9,   10,  219,  219,  219,  219,   15,  219,  219,  219,
 /*  9450 */   219,  219,   21,  219,  219,  219,  219,  219,  219,   28,
 /*  9460 */   219,  219,  219,  219,   33,   34,   35,   36,   37,   38,
 /*  9470 */    39,   40,   41,  219,    7,   44,    9,   10,  219,  219,
 /*  9480 */   219,  219,   15,  219,  219,   54,  219,  219,   21,  219,
 /*  9490 */    59,   60,   61,  219,  219,   28,   65,  219,  219,  219,
 /*  9500 */    33,   34,   35,   36,   37,   38,   39,   40,   41,  219,
 /*  9510 */   219,   44,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9520 */   219,   54,  219,  219,  219,  219,   59,   60,   61,  219,
 /*  9530 */   219,  219,   65,  219,  219,  219,  219,  106,  219,  219,
 /*  9540 */   219,  219,  219,  219,  219,  114,  115,  116,  117,  118,
 /*  9550 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9560 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9570 */   219,  219,  219,  106,  219,  219,  219,  219,  219,  219,
 /*  9580 */   219,  114,  115,  116,  117,  118,  219,  219,  219,    7,
 /*  9590 */   219,    9,   10,  219,  219,  219,  219,   15,  219,  219,
 /*  9600 */   219,  219,  219,   21,  219,  219,  219,  219,  219,  219,
 /*  9610 */    28,  219,  219,  219,  219,   33,   34,   35,   36,   37,
 /*  9620 */    38,   39,   40,   41,  219,    7,   44,    9,   10,  219,
 /*  9630 */   219,  219,  219,   15,  219,  219,   54,  219,  219,   21,
 /*  9640 */   219,   59,   60,   61,  219,  219,   28,   65,  219,  219,
 /*  9650 */   219,   33,   34,   35,   36,   37,   38,   39,   40,   41,
 /*  9660 */   219,  219,   44,  219,  219,  219,  219,  219,  219,  219,
 /*  9670 */   219,  219,   54,  219,  219,  219,  219,   59,   60,   61,
 /*  9680 */   219,  219,  219,   65,  219,  219,  219,  219,  106,  219,
 /*  9690 */   219,  219,  219,  219,  219,  219,  114,  115,  116,  117,
 /*  9700 */   118,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9710 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9720 */   219,  219,  219,  219,  106,  219,  219,  219,  219,  219,
 /*  9730 */   219,  219,  114,  115,  116,  117,  118,  219,  219,  219,
 /*  9740 */     7,  219,    9,   10,  219,  219,  219,  219,   15,  219,
 /*  9750 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /*  9760 */   219,   28,  219,  219,  219,  219,   33,   34,   35,   36,
 /*  9770 */    37,   38,   39,   40,   41,  219,    7,   44,    9,   10,
 /*  9780 */   219,  219,  219,  219,   15,  219,  219,   54,  219,  219,
 /*  9790 */    21,  219,   59,   60,   61,  219,  219,   28,   65,  219,
 /*  9800 */   219,  219,   33,   34,   35,   36,   37,   38,   39,   40,
 /*  9810 */    41,  219,  219,   44,  219,  219,  219,  219,  219,  219,
 /*  9820 */   219,  219,  219,   54,  219,  219,  219,  219,   59,   60,
 /*  9830 */    61,  219,  219,  219,   65,  219,  219,  219,  219,  106,
 /*  9840 */   219,  219,  219,  219,  219,  219,  219,  114,  115,  116,
 /*  9850 */   117,  118,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9860 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /*  9870 */   219,  219,  219,  219,  219,  106,  219,  219,  219,  219,
 /*  9880 */   219,  219,  219,  114,  115,  116,  117,  118,  219,  219,
 /*  9890 */   219,    7,  219,    9,   10,  219,  219,  219,  219,   15,
 /*  9900 */   219,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /*  9910 */   219,  219,   28,  219,  219,  219,  219,   33,   34,   35,
 /*  9920 */    36,   37,   38,   39,   40,   41,  219,    7,   44,    9,
 /*  9930 */    10,  219,  219,  219,  219,   15,  219,  219,   54,  219,
 /*  9940 */   219,   21,  219,   59,   60,   61,  219,  219,   28,   65,
 /*  9950 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /*  9960 */    40,   41,  219,  219,   44,  219,  219,  219,  219,  219,
 /*  9970 */   219,  219,  219,  219,   54,  219,  219,  219,  219,   59,
 /*  9980 */    60,   61,  219,  219,  219,   65,  219,  219,  219,  219,
 /*  9990 */   106,  219,  219,  219,  219,  219,  219,  219,  114,  115,
 /* 10000 */   116,  117,  118,  219,  219,  219,  219,  219,  219,  219,
 /* 10010 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10020 */   219,  219,  219,  219,  219,  219,  106,  219,  219,  219,
 /* 10030 */   219,  219,  219,  219,  114,  115,  116,  117,  118,  219,
 /* 10040 */   219,  219,  219,    8,  219,  219,   11,   12,   13,   14,
 /* 10050 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /* 10060 */    25,   26,   27,   28,   29,   30,   31,   32,  219,  219,
 /* 10070 */   219,  219,  219,  219,  219,   40,   41,   42,  219,  219,
 /* 10080 */   219,  219,  219,  219,    8,  219,  219,   11,   12,   13,
 /* 10090 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /* 10100 */    24,   25,   26,   27,   28,   29,   30,   31,   32,  219,
 /* 10110 */   219,  219,  219,  219,  219,  219,  219,   41,   42,  219,
 /* 10120 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10130 */    54,  219,  219,  219,  219,  219,  219,  219,  219,    8,
 /* 10140 */   219,  219,   11,   12,   13,   14,   15,   16,   17,   18,
 /* 10150 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /* 10160 */    29,   30,   31,   32,  219,  219,  219,  219,  219,  219,
 /* 10170 */   219,  219,   41,   42,  219,  219,   45,  219,  219,  219,
 /* 10180 */   219,    8,  219,  219,   11,   12,   13,   14,   15,   16,
 /* 10190 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /* 10200 */    27,   28,   29,   30,   31,   32,  219,  219,  219,  219,
 /* 10210 */   219,  219,  219,  219,   41,   42,  219,  219,   45,  219,
 /* 10220 */   219,  219,  219,    8,  219,  219,   11,   12,   13,   14,
 /* 10230 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /* 10240 */    25,   26,   27,   28,   29,   30,   31,   32,  219,  219,
 /* 10250 */   219,  219,  219,  219,  219,  219,   41,   42,  219,  219,
 /* 10260 */    45,  219,  219,  219,  219,    8,  219,  219,   11,   12,
 /* 10270 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /* 10280 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /* 10290 */   219,  219,  219,  219,  219,  219,  219,  219,   41,   42,
 /* 10300 */   219,  219,   45,  219,  219,  219,  219,    8,  219,  219,
 /* 10310 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /* 10320 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /* 10330 */    31,   32,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10340 */    41,   42,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10350 */   219,  219,  219,   54,  219,  219,  219,  219,  219,  219,
 /* 10360 */   219,  219,    8,  219,  219,   11,   12,   13,   14,   15,
 /* 10370 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /* 10380 */    26,   27,   28,   29,   30,   31,   32,  219,  219,  219,
 /* 10390 */   219,  219,  219,  219,  219,   41,   42,  219,  219,  219,
 /* 10400 */   219,  219,  219,  219,  219,  219,  219,  219,   54,  219,
 /* 10410 */   219,  219,  219,  219,  219,  219,  219,    8,  219,  219,
 /* 10420 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /* 10430 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /* 10440 */    31,   32,  219,  219,    7,  219,    9,   10,  219,  219,
 /* 10450 */    41,   42,   15,  219,  219,  219,  219,  219,   21,  219,
 /* 10460 */   219,  219,  219,   54,  219,   28,  219,  219,  219,  219,
 /* 10470 */    33,   34,   35,   36,   37,   38,   39,  219,   41,  219,
 /* 10480 */     7,   44,    9,   10,  219,  219,  219,  219,  219,  219,
 /* 10490 */   219,   54,  219,  219,   21,  219,   59,   60,   61,  219,
 /* 10500 */   219,   28,   65,  219,  219,  219,   33,   34,   35,   36,
 /* 10510 */    37,   38,   39,  219,   41,  219,  219,   44,  219,  219,
 /* 10520 */   219,  219,  219,  219,  219,  219,  219,   54,  219,  219,
 /* 10530 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /* 10540 */   219,  219,  219,  106,  219,  219,    7,  219,    9,   10,
 /* 10550 */   219,  114,  115,  116,  117,  118,  219,  219,  219,  219,
 /* 10560 */    21,  219,  219,  219,  219,  219,  219,   28,  219,  219,
 /* 10570 */   219,   98,   33,   34,   35,   36,   37,   38,   39,  106,
 /* 10580 */    41,  219,  219,   44,   45,  219,  219,  114,  115,  116,
 /* 10590 */   117,  118,  219,   54,  219,  219,  219,  219,   59,   60,
 /* 10600 */    61,  219,  219,  219,   65,  219,  219,  219,  219,  219,
 /* 10610 */   219,  219,    7,  219,    9,   10,  219,  219,  219,  219,
 /* 10620 */   219,  219,  219,  219,  219,  219,   21,  219,  219,  219,
 /* 10630 */   219,  219,  219,   28,  219,  219,  219,  219,   33,   34,
 /* 10640 */    35,   36,   37,   38,   39,  106,   41,  219,  219,   44,
 /* 10650 */   219,  219,  219,  114,  115,  116,  117,  118,  219,   54,
 /* 10660 */   219,  219,  219,  219,   59,   60,   61,  219,  219,  219,
 /* 10670 */    65,  219,  219,  219,  219,  219,  219,  219,    7,  219,
 /* 10680 */     9,   10,  219,  219,  219,  219,   15,  219,  219,  219,
 /* 10690 */   219,  219,   21,  219,  219,  219,  219,  219,  219,   28,
 /* 10700 */   219,  219,  219,   98,   33,   34,   35,   36,   37,   38,
 /* 10710 */    39,  106,   41,  219,    7,   44,    9,   10,  219,  114,
 /* 10720 */   115,  116,  117,  118,  219,   54,  219,  219,   21,  219,
 /* 10730 */    59,   60,   61,  219,  219,   28,   65,  219,  219,  219,
 /* 10740 */    33,   34,   35,   36,   37,   38,   39,  219,   41,  219,
 /* 10750 */   219,   44,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 10760 */   219,   54,  219,  219,  219,  219,   59,   60,   61,  219,
 /* 10770 */   219,  219,   65,  219,   67,  219,  219,  106,  219,  219,
 /* 10780 */     7,  219,    9,   10,  219,  114,  115,  116,  117,  118,
 /* 10790 */   219,  219,  219,  219,   21,  219,  219,  219,  219,  219,
 /* 10800 */   219,   28,  219,  219,  219,  219,   33,   34,   35,   36,
 /* 10810 */    37,   38,   39,  106,   41,  219,    7,   44,    9,   10,
 /* 10820 */   219,  114,  115,  116,  117,  118,  219,   54,  219,  219,
 /* 10830 */    21,  219,   59,   60,   61,  219,  219,   28,   65,  219,
 /* 10840 */    67,  219,   33,   34,   35,   36,   37,   38,   39,  219,
 /* 10850 */    41,  219,  219,   44,  219,  219,  219,  219,  219,  219,
 /* 10860 */   219,  219,  219,   54,  219,  219,  219,  219,   59,   60,
 /* 10870 */    61,  219,  219,  219,   65,  219,   67,  219,  219,  106,
 /* 10880 */   219,  219,    7,  219,    9,   10,  219,  114,  115,  116,
 /* 10890 */   117,  118,  219,  219,  219,  219,   21,  219,  219,  219,
 /* 10900 */   219,  219,  219,   28,  219,  219,  219,  219,   33,   34,
 /* 10910 */    35,   36,   37,   38,   39,  106,   41,  219,    7,   44,
 /* 10920 */     9,   10,  219,  114,  115,  116,  117,  118,  219,   54,
 /* 10930 */   219,  219,   21,  219,   59,   60,   61,  219,  219,   28,
 /* 10940 */    65,  219,   67,  219,   33,   34,   35,   36,   37,   38,
 /* 10950 */    39,  219,   41,  219,  219,   44,  219,  219,  219,  219,
 /* 10960 */   219,  219,  219,  219,  219,   54,  219,  219,  219,  219,
 /* 10970 */    59,   60,   61,  219,  219,  219,   65,  219,   67,  219,
 /* 10980 */   219,  106,  219,  219,    7,  219,    9,   10,  219,  114,
 /* 10990 */   115,  116,  117,  118,  219,  219,  219,  219,   21,  219,
 /* 11000 */   219,  219,  219,  219,  219,   28,  219,  219,  219,  219,
 /* 11010 */    33,   34,   35,   36,   37,   38,   39,  106,   41,  219,
 /* 11020 */     7,   44,    9,   10,  219,  114,  115,  116,  117,  118,
 /* 11030 */   219,   54,  219,  219,   21,  219,   59,   60,   61,  219,
 /* 11040 */   219,   28,   65,  219,   67,  219,   33,   34,   35,   36,
 /* 11050 */    37,   38,   39,  219,   41,  219,  219,   44,  219,  219,
 /* 11060 */   219,  219,  219,  219,  219,  219,  219,   54,  219,  219,
 /* 11070 */   219,  219,   59,   60,   61,  219,  219,  219,   65,  219,
 /* 11080 */    67,  219,  219,  106,  219,  219,  219,  219,  219,  219,
 /* 11090 */   219,  114,  115,  116,  117,  118,  219,  219,  219,  219,
 /* 11100 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11110 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  106,
 /* 11120 */   219,  219,  219,  219,  219,  219,  219,  114,  115,  116,
 /* 11130 */   117,  118,  219,  219,  219,  219,    8,  219,  219,   11,
 /* 11140 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   21,
 /* 11150 */    22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
 /* 11160 */    32,    7,  219,    9,   10,  219,  219,  219,  219,   41,
 /* 11170 */    42,  219,  219,  219,  219,   21,  219,  219,  219,  219,
 /* 11180 */   219,  219,   28,  219,  219,  219,  219,   33,   34,   35,
 /* 11190 */    36,   37,   38,   39,  219,   41,  219,    7,   44,    9,
 /* 11200 */    10,  219,  219,  219,  219,  219,  219,  219,   54,  219,
 /* 11210 */   219,   21,  219,   59,   60,   61,  219,  219,   28,   65,
 /* 11220 */   219,  219,  219,   33,   34,   35,   36,   37,   38,   39,
 /* 11230 */   219,   41,  219,  219,   44,  219,  219,  219,  219,  219,
 /* 11240 */   219,  219,  219,  219,   54,  219,  219,  219,  219,   59,
 /* 11250 */    60,   61,  219,  219,  219,   65,  219,  219,  219,  219,
 /* 11260 */   106,  219,  219,  219,  219,  219,  219,  219,  114,  115,
 /* 11270 */   116,  117,  118,  219,  219,  219,  219,  219,  219,  219,
 /* 11280 */   219,  219,  219,  219,  219,  219,  219,  219,  219,  219,
 /* 11290 */   219,  219,  219,  219,  219,  219,  106,  219,  219,  219,
 /* 11300 */   219,  219,  219,  219,  114,  115,  116,  117,  118,
};
#define YY_SHIFT_USE_DFLT (-40)
#define YY_SHIFT_COUNT (568)
#define YY_SHIFT_MIN   (-39)
#define YY_SHIFT_MAX   (11190)
static const short yy_shift_ofst[] = {
 /*     0 */  6183, 6183, 5982, 5869, 5756, 5643, 5530, 5417, 5304, 5191,
 /*    10 */  5078, 4965, 4852, 4739, 4626, 4513, 4400, 4287, 4174, 4061,
 /*    20 */  3948, 3835, 3722, 3609, 3496, 3383, 3270, 6095, 6095, 6095,
 /*    30 */  6095, 6095, 6095, 3157, 3044, 2931, 2818, 2705, 2592, 2479,
 /*    40 */  2366, 2253, 2140, 2027, 1914, 1801, 1688, 1575, 1462, 1349,
 /*    50 */  1236, 1123, 1010,  897,  784,  671,  558,  445,  332,  219,
 /*    60 */   106,   -7, 6095, 6095, 11013,  151, 11190, 9920, 9884, 9769,
 /*    70 */  9733, 9618, 9582, 9467, 9431, 9316, 9280, 11154,   38,  716,
 /*    80 */   716, 6271, 10977, 10911, 10875, 10809, 10773, 10707, 10671, 11154,
 /*    90 */  11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154,
 /*   100 */  11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154,
 /*   110 */  11154, 11154, 11154, 8008, 8008, 8008, 8008, 8008, 8008, 8008,
 /*   120 */  8008, 10605, 10539, 10473, 10437, 11154, 11154, 11154, 11154, 11154,
 /*   130 */  11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154,
 /*   140 */  11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154,
 /*   150 */  11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154,
 /*   160 */  11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154, 11154,
 /*   170 */   603,  490, 6880, 1620, 7832, 7659, 7486, 7313, 7140, 6967,
 /*   180 */  6794, 8353, 7048, 8178, 6709, 1507, 1507, 1507, 1507, 1507,
 /*   190 */  1394, 1394, 6977, 7228, 6835, 1281, 1281, 1281, 6713, 1733,
 /*   200 */  1733, 1733, 7150, 7055, 6720,  481,  -20,  182, 1004,  982,
 /*   210 */  1407, 1407, 1407, 1407, 1407, 1407, 1407, 1407, 1407, 1407,
 /*   220 */  1407, 1429, 1429, 1429, 1429, 1429, 1407, 1407, 1407, 1407,
 /*   230 */  1407, 1407, 1407, 1407, 1407, 1407, 1407, 1409, 1359, 1400,
 /*   240 */   377,  264, 6713, 6713, 1168, 1055, 6713, 6713, 6713, 6713,
 /*   250 */   942,  829, 6713, 6713, 6713, 6713, 6713, 6713, 6713, 6713,
 /*   260 */  6713, 6713, 6598, 6598, 6598, 6598, 6598, 6598, 6598, 6598,
 /*   270 */  6598, 6598,  481,  862,  722,  860,  759,  609,   69,  793,
 /*   280 */   793,  793,  819,  793,  818,  397,  712,  793,  684,  702,
 /*   290 */   674,  648, 1420, 1429, 1416, 1409, 1407, 1359, 1400, 10173,
 /*   300 */  10409, 10354, 10299, 10257, 10215, 10173, 10131, 10076, 10035, 9220,
 /*   310 */  6418, 11128, 11128, 11128, 11128, 11128, 11128, 11128, 11128, 11128,
 /*   320 */  11128, 11128, 11128, 6342, 6342, 9387, 6619, 6714, 6442, 6442,
 /*   330 */  6442, 6442, 6442,   90,   90,   90,   90,   90,   90,   90,
 /*   340 */    90,   90,  200,  200,  200,  200,  330,  217,  104,  -32,
 /*   350 */   180,  103,   41,  -28, 1320, 1297,  299,  608, 1262,  570,
 /*   360 */   566, 1247,  382,  670,  453,  658, 1238,  580,  556, 1149,
 /*   370 */   805,  505, 1134, 1125, 1036,  350, 1021, 1012,  923,  796,
 /*   380 */   581,  423,  707,  705,  692,  642,  641,  640,  638,  637,
 /*   390 */   594,  564,  543,  524,  284,  279,  157,  462,  406,  242,
 /*   400 */   129,  292,  334,  237,  243,  221,   79,  120,  -26,  -26,
 /*   410 */   255,   21,  -26,  -26,  -26,  -26,   34,  -26,  -26,  -26,
 /*   420 */   -26,    0, 1136, 1135, 1131, 1383, 1381, 1373, 1386, 1356,
 /*   430 */  1375, 1374, 1354, 1369, 1368, 1367, 1357, 1362, 1358, 1351,
 /*   440 */  1355, 1347, 1346, 1345, 1336, 1317, 1327, 1315, 1325, 1310,
 /*   450 */  1324, 1323, 1321, 1307, 1318, 1304, 1312, 1288, 1296, 1283,
 /*   460 */  1293, 1291, 1264, 1287, 1277, 1299, 1257, 1273, 1261, 1285,
 /*   470 */  1270, 1255, 1254, 1239, 1249, 1245,  246, 1244, 1218, 1221,
 /*   480 */  1204, 1203, 1174, 1195, 1200, 1210, 1191, 1196, 1206, 1187,
 /*   490 */  1193, 1178, 1157, 1173, 1164, 1155, 1122, 1158, 1116, 1148,
 /*   500 */  1146, 1090, 1142, 1128, 1136, 1135, 1131, 1118, 1120, 1111,
 /*   510 */  1108, 1101, 1099, 1087, 1086, 1052, 1070, 1051,  994,  914,
 /*   520 */   947,  896,  883,  903,  865,  861,  858,  848,  843,  825,
 /*   530 */   812,  806,  807,  804,  780,  781,  767,  739,  754,  723,
 /*   540 */   731,  704,  636,  639,  531,  635,  578,  620,  531,  559,
 /*   550 */   549,  523,  522,  493,  385,  294,  267,  393,  304,  246,
 /*   560 */   211,  207,  175,  147,    2,  -22,  150,  -39,    1,
};
#define YY_REDUCE_USE_DFLT (-152)
#define YY_REDUCE_COUNT (298)
#define YY_REDUCE_MIN   (-151)
#define YY_REDUCE_MAX   (9076)
static const short yy_reduce_ofst[] = {
 /*     0 */  6275, 6378, 8999, 8903, 8826, 8730, 8653, 8557, 8480, 8384,
 /*    10 */  8307, 8211, 8134, 8038, 7961, 7865, 7788, 7692, 7615, 7519,
 /*    20 */  7442, 7346, 7269, 7173, 7096, 7000, 6923, 6827, 6750, 6654,
 /*    30 */  6577, 6481, 6404, 9076, 9076, 9076, 9076, 9076, 9076, 9076,
 /*    40 */  9076, 9076, 9076, 9076, 9076, 9076, 9076, 9076, 9076, 9076,
 /*    50 */  9076, 9076, 9076, 9076, 9076, 9076, 9076, 9076, 9076, 9076,
 /*    60 */  9076, 9076, 9076, 9076, 6283, -151, 6365, 1508, 1395, 1282,
 /*    70 */  1169, 1056,  943,  830,  717,  604,  491,  379,  302,  189,
 /*    80 */    76, 3541, 1853, 1853, 1853, 1853, 1853, 1853, 1811, 1966,
 /*    90 */  1020,  907, 1874, 3428, 3315, 3202, 3089, 2976, 2863, 2750,
 /*   100 */  2637, 2524, 2411, 2298, 2185, 2072, 1959, 1846,  -68,  794,
 /*   110 */   681,  229,  116, 1787, 1761, 1754, 1726, 1674, 1648, 1641,
 /*   120 */   398, 3767, 3732, 3682, 3661, 3654, 3619, 3569, 3548, 3506,
 /*   130 */  3456, 3435, 3393, 3343, 3322, 3280, 3230, 3209, 3167, 3117,
 /*   140 */  3096, 3054, 3004, 2983, 2941, 2891, 2870, 2828, 2778, 2757,
 /*   150 */  2715, 2665, 2644, 2602, 2552, 2531, 2489, 2439, 2418, 2376,
 /*   160 */  2326, 2305, 2263, 2213, 2150, 2100, 2079, 2037, 1924, 1585,
 /*   170 */   632,  301,  473,  371,  331,  240,  218,  184,  105,   -8,
 /*   180 */  -121,  167, -144,  280,  289,   22,   22,   22,   22,   22,
 /*   190 */  -119, -119,   50, -101,  960,  976,  863,  661, -104,  976,
 /*   200 */   863,  661,  878,  854,  765,  940,  932,  908,  887,  877,
 /*   210 */   817,  821,  778,  814,  775,  747,  741,  769,  752,  732,
 /*   220 */   701,  876,  770,  370,  342,  -73,  821,  817,  814,  778,
 /*   230 */   775,  769,  752,  747,  741,  732,  701,  650,  533,  700,
 /*   240 */  1376, 1376, 1453, 1451, 1376, 1376, 1443, 1440, 1438, 1433,
 /*   250 */  1376, 1376, 1431, 1427, 1426, 1403, 1401, 1388, 1384, 1379,
 /*   260 */  1378, 1377, 1398, 1397, 1396, 1393, 1391, 1390, 1387, 1370,
 /*   270 */  1333, 1330, 1294, 1372, 1371, 1363, 1350, 1366, 1289, 1361,
 /*   280 */  1360, 1352, 1341, 1338, 1339, 1335, 1316, 1331, 1329, 1328,
 /*   290 */  1290, 1290, 1326, 1322, 1306, 1260, 1241, 1248, 1313,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */  1368,  923, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*    10 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*    20 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*    30 */  1368, 1368, 1156, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*    40 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*    50 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*    60 */  1368, 1368, 1158, 1157, 1368, 1368, 1368, 1368, 1368, 1368,
 /*    70 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,  979,
 /*    80 */   978, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*    90 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   100 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   110 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   120 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   130 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   140 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   150 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   160 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   170 */   982,  987, 1368,  983, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   180 */  1368, 1368, 1368, 1368, 1368,  980,  984,  986,  985,  981,
 /*   190 */   988,  989, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   200 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1137, 1141,
 /*   210 */  1105, 1104, 1103, 1102, 1101, 1100, 1099, 1098, 1097, 1096,
 /*   220 */  1095, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   230 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1166, 1368,
 /*   240 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   250 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   260 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   270 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   280 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   290 */  1138, 1142, 1368, 1368, 1368, 1368, 1368, 1167, 1368, 1368,
 /*   300 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   310 */  1368, 1332, 1368, 1222, 1218, 1247, 1365, 1331, 1345, 1330,
 /*   320 */  1326, 1329, 1251, 1255, 1254, 1286, 1280, 1279, 1285, 1284,
 /*   330 */  1283, 1282, 1281, 1256, 1257, 1258, 1259, 1260, 1261, 1262,
 /*   340 */  1266, 1263, 1278, 1274, 1273, 1249, 1368, 1368, 1368, 1368,
 /*   350 */  1368, 1290, 1290,  956, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   360 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   370 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   380 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   390 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   400 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1289, 1287,
 /*   410 */  1368, 1290, 1265, 1277, 1276, 1275, 1368, 1253, 1252, 1250,
 /*   420 */  1248, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   430 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   440 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   450 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   460 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1063, 1060,
 /*   470 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1269, 1368,
 /*   480 */  1268, 1368, 1368, 1304, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   490 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   500 */  1368, 1368, 1368, 1245, 1288, 1324, 1325, 1323, 1368, 1368,
 /*   510 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   520 */  1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368, 1368,
 /*   530 */  1083, 1368, 1081, 1368, 1079, 1368, 1368, 1084, 1368, 1082,
 /*   540 */  1368, 1080, 1078, 1368, 1361, 1348, 1346, 1368, 1368, 1077,
 /*   550 */  1368, 1368, 1368, 1301, 1298, 1292, 1291, 1368, 1368, 1270,
 /*   560 */  1368, 1368, 1368, 1267, 1368,  956, 1368, 1368, 1368,  925,
 /*   570 */  1074, 1073, 1072, 1071, 1070, 1069,  970,  969,  968,  966,
 /*   580 */   967,  965,  964,  962,  974,  975,  973,  991,  997,  999,
 /*   590 */   995,  993,  996, 1003, 1005, 1004, 1002, 1001, 1000,  998,
 /*   600 */   994,  992,  990, 1009, 1037, 1039, 1036, 1025, 1027, 1024,
 /*   610 */  1035, 1038, 1034, 1023, 1026, 1022, 1031, 1033, 1030, 1019,
 /*   620 */  1021, 1018, 1029, 1032, 1028, 1017, 1020, 1016, 1008,  972,
 /*   630 */   971,  963,  961,  960, 1007, 1015, 1013, 1014, 1012, 1011,
 /*   640 */  1006, 1049, 1047, 1043, 1046, 1042, 1058, 1057, 1056, 1055,
 /*   650 */  1054, 1053, 1052, 1051, 1050, 1048, 1045, 1041, 1044, 1062,
 /*   660 */  1094, 1068, 1067, 1065, 1064, 1066, 1061, 1059, 1040, 1010,
 /*   670 */   977,  976,  959,  958, 1308, 1307, 1322, 1321, 1320, 1319,
 /*   680 */  1318, 1317, 1271, 1312, 1311, 1338, 1340, 1339, 1337, 1306,
 /*   690 */  1305, 1327, 1316, 1315, 1314, 1313, 1333, 1334, 1076, 1335,
 /*   700 */  1336, 1139, 1147, 1140, 1149, 1145, 1143, 1150, 1151, 1148,
 /*   710 */  1146, 1155, 1159, 1161, 1163, 1165, 1169, 1175, 1176, 1174,
 /*   720 */  1172, 1173, 1171, 1182, 1181, 1180, 1179, 1178, 1110, 1367,
 /*   730 */  1366, 1242, 1241, 1240, 1239, 1238, 1237, 1236, 1235, 1234,
 /*   740 */  1233, 1244, 1246, 1243, 1232, 1231, 1230, 1229, 1228, 1227,
 /*   750 */  1226, 1225, 1224, 1223, 1221, 1220, 1219, 1185, 1217, 1216,
 /*   760 */  1215, 1213, 1214, 1212, 1206, 1205, 1204, 1203, 1202, 1201,
 /*   770 */  1200, 1196, 1195, 1211, 1210, 1208, 1207, 1199, 1198, 1209,
 /*   780 */  1197, 1194, 1193, 1192, 1191, 1190, 1189, 1188, 1187, 1186,
 /*   790 */  1184, 1183, 1177, 1170, 1168, 1164, 1162, 1160, 1154, 1153,
 /*   800 */  1152, 1144, 1136, 1135, 1134, 1133, 1132, 1131, 1130, 1129,
 /*   810 */  1128, 1127, 1126, 1125, 1124, 1123, 1122, 1121, 1120, 1119,
 /*   820 */  1118, 1117, 1116, 1115, 1114, 1113, 1112, 1111, 1109, 1091,
 /*   830 */  1089, 1087, 1093, 1092, 1090, 1088, 1086, 1358, 1360, 1364,
 /*   840 */  1359, 1357, 1356, 1355, 1354, 1353, 1352, 1351, 1350, 1349,
 /*   850 */  1348, 1347, 1346, 1085, 1075, 1328, 1325, 1324, 1323, 1310,
 /*   860 */  1309, 1303, 1302, 1300, 1299, 1298, 1297, 1296, 1295, 1294,
 /*   870 */  1293, 1292, 1291, 1288, 1264, 1108, 1107, 1106, 1105, 1104,
 /*   880 */  1103, 1102, 1101, 1100, 1099, 1098, 1097, 1096, 1095, 1272,
 /*   890 */   955,  957,  954,  953,  952,  951,  950,  949,  948,  947,
 /*   900 */   946,  945,  944,  943,  942,  941,  940,  939,  938,  937,
 /*   910 */   936,  935,  934,  933,  932,  931,  930,  929,  928,  927,
 /*   920 */   926,  924,
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

// 4666 "parser.cpp"
}
      break;
    case 122: /* xx_language */
{
// 1390 "parser.lemon"
 delete (yypminor->yy396); 
// 4673 "parser.cpp"
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
// 5416 "parser.cpp"
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
      case 189: /* xx_statement ::= xx_cblock */ yytestcase(yyruleno==189);
      case 190: /* xx_statement ::= xx_let_statement */ yytestcase(yyruleno==190);
      case 191: /* xx_statement ::= xx_if_statement */ yytestcase(yyruleno==191);
      case 192: /* xx_statement ::= xx_loop_statement */ yytestcase(yyruleno==192);
      case 193: /* xx_statement ::= xx_echo_statement */ yytestcase(yyruleno==193);
      case 194: /* xx_statement ::= xx_return_statement */ yytestcase(yyruleno==194);
      case 195: /* xx_statement ::= xx_require_statement */ yytestcase(yyruleno==195);
      case 196: /* xx_statement ::= xx_fetch_statement */ yytestcase(yyruleno==196);
      case 197: /* xx_statement ::= xx_fcall_statement */ yytestcase(yyruleno==197);
      case 198: /* xx_statement ::= xx_mcall_statement */ yytestcase(yyruleno==198);
      case 199: /* xx_statement ::= xx_scall_statement */ yytestcase(yyruleno==199);
      case 200: /* xx_statement ::= xx_unset_statement */ yytestcase(yyruleno==200);
      case 201: /* xx_statement ::= xx_throw_statement */ yytestcase(yyruleno==201);
      case 202: /* xx_statement ::= xx_declare_statement */ yytestcase(yyruleno==202);
      case 203: /* xx_statement ::= xx_break_statement */ yytestcase(yyruleno==203);
      case 204: /* xx_statement ::= xx_continue_statement */ yytestcase(yyruleno==204);
      case 205: /* xx_statement ::= xx_while_statement */ yytestcase(yyruleno==205);
      case 206: /* xx_statement ::= xx_do_while_statement */ yytestcase(yyruleno==206);
      case 207: /* xx_statement ::= xx_try_catch_statement */ yytestcase(yyruleno==207);
      case 208: /* xx_statement ::= xx_switch_statement */ yytestcase(yyruleno==208);
      case 209: /* xx_statement ::= xx_for_statement */ yytestcase(yyruleno==209);
      case 210: /* xx_statement ::= xx_comment */ yytestcase(yyruleno==210);
      case 211: /* xx_statement ::= xx_empty_statement */ yytestcase(yyruleno==211);
      case 296: /* xx_index_expr ::= xx_common_expr */ yytestcase(yyruleno==296);
      case 300: /* xx_echo_expression ::= xx_common_expr */ yytestcase(yyruleno==300);
      case 325: /* xx_assign_expr ::= xx_common_expr */ yytestcase(yyruleno==325);
      case 366: /* xx_common_expr ::= xx_fetch_expr */ yytestcase(yyruleno==366);
      case 401: /* xx_common_expr ::= xx_mcall_expr */ yytestcase(yyruleno==401);
      case 402: /* xx_common_expr ::= xx_scall_expr */ yytestcase(yyruleno==402);
      case 403: /* xx_common_expr ::= xx_fcall_expr */ yytestcase(yyruleno==403);
      case 423: /* xx_array_value ::= xx_common_expr */ yytestcase(yyruleno==423);
      case 442: /* xx_literal_array_value ::= xx_literal_expr */ yytestcase(yyruleno==442);
      case 443: /* xx_eval_expr ::= xx_common_expr */ yytestcase(yyruleno==443);
// 1392 "parser.lemon"
{
	yygotominor.yy396 = yymsp[0].minor.yy396;
}
// 5482 "parser.cpp"
        break;
      case 2: /* xx_top_statement_list ::= xx_top_statement_list xx_top_statement */
      case 68: /* xx_class_properties_definition ::= xx_class_properties_definition xx_class_property_definition */ yytestcase(yyruleno==68);
      case 84: /* xx_class_consts_definition ::= xx_class_consts_definition xx_class_const_definition */ yytestcase(yyruleno==84);
      case 86: /* xx_class_methods_definition ::= xx_class_methods_definition xx_class_method_definition */ yytestcase(yyruleno==86);
      case 88: /* xx_interface_methods_definition ::= xx_interface_methods_definition xx_interface_method_definition */ yytestcase(yyruleno==88);
      case 126: /* xx_visibility_list ::= xx_visibility_list xx_visibility */ yytestcase(yyruleno==126);
      case 187: /* xx_statement_list ::= xx_statement_list xx_statement */ yytestcase(yyruleno==187);
      case 226: /* xx_elseif_statements ::= xx_elseif_statements xx_elseif_statement */ yytestcase(yyruleno==226);
      case 232: /* xx_case_clauses ::= xx_case_clauses xx_case_clause */ yytestcase(yyruleno==232);
      case 246: /* xx_catch_statement_list ::= xx_catch_statement_list xx_catch_statement */ yytestcase(yyruleno==246);
      case 285: /* xx_array_offset_list ::= xx_array_offset_list xx_array_offset */ yytestcase(yyruleno==285);
// 1396 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(yymsp[-1].minor.yy396, yymsp[0].minor.yy396);
}
// 5499 "parser.cpp"
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
      case 154: /* xx_parameter_list ::= xx_parameter */ yytestcase(yyruleno==154);
      case 188: /* xx_statement_list ::= xx_statement */ yytestcase(yyruleno==188);
      case 227: /* xx_elseif_statements ::= xx_elseif_statement */ yytestcase(yyruleno==227);
      case 233: /* xx_case_clauses ::= xx_case_clause */ yytestcase(yyruleno==233);
      case 247: /* xx_catch_statement_list ::= xx_catch_statement */ yytestcase(yyruleno==247);
      case 253: /* xx_catch_classes_list ::= xx_catch_class */ yytestcase(yyruleno==253);
      case 263: /* xx_let_assignments ::= xx_let_assignment */ yytestcase(yyruleno==263);
      case 286: /* xx_array_offset_list ::= xx_array_offset */ yytestcase(yyruleno==286);
      case 299: /* xx_echo_expressions ::= xx_echo_expression */ yytestcase(yyruleno==299);
      case 322: /* xx_declare_variable_list ::= xx_declare_variable */ yytestcase(yyruleno==322);
      case 406: /* xx_call_parameters ::= xx_call_parameter */ yytestcase(yyruleno==406);
      case 416: /* xx_array_list ::= xx_array_item */ yytestcase(yyruleno==416);
      case 436: /* xx_literal_array_list ::= xx_literal_array_item */ yytestcase(yyruleno==436);
// 1400 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(NULL, yymsp[0].minor.yy396);
}
// 5528 "parser.cpp"
        break;
      case 30: /* xx_namespace_def ::= NAMESPACE IDENTIFIER DOTCOMMA */
// 1508 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_namespace(yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,43,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5537 "parser.cpp"
        break;
      case 31: /* xx_namespace_def ::= USE xx_use_aliases_list DOTCOMMA */
// 1512 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_use_aliases(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,46,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5546 "parser.cpp"
        break;
      case 32: /* xx_use_aliases_list ::= xx_use_aliases_list COMMA xx_use_aliases */
      case 51: /* xx_implements_list ::= xx_implements_list COMMA xx_implements */ yytestcase(yyruleno==51);
      case 80: /* xx_class_property_shortcuts_list ::= xx_class_property_shortcuts_list COMMA xx_class_property_shortcut */ yytestcase(yyruleno==80);
      case 153: /* xx_parameter_list ::= xx_parameter_list COMMA xx_parameter */ yytestcase(yyruleno==153);
      case 262: /* xx_let_assignments ::= xx_let_assignments COMMA xx_let_assignment */ yytestcase(yyruleno==262);
      case 298: /* xx_echo_expressions ::= xx_echo_expressions COMMA xx_echo_expression */ yytestcase(yyruleno==298);
      case 321: /* xx_declare_variable_list ::= xx_declare_variable_list COMMA xx_declare_variable */ yytestcase(yyruleno==321);
      case 405: /* xx_call_parameters ::= xx_call_parameters COMMA xx_call_parameter */ yytestcase(yyruleno==405);
      case 415: /* xx_array_list ::= xx_array_list COMMA xx_array_item */ yytestcase(yyruleno==415);
      case 435: /* xx_literal_array_list ::= xx_literal_array_list COMMA xx_literal_array_item */ yytestcase(yyruleno==435);
// 1516 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(yymsp[-2].minor.yy396, yymsp[0].minor.yy396);
  yy_destructor(yypParser,6,&yymsp[-1].minor);
}
// 5563 "parser.cpp"
        break;
      case 34: /* xx_use_aliases ::= IDENTIFIER */
// 1524 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_use_aliases_item(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 5570 "parser.cpp"
        break;
      case 35: /* xx_use_aliases ::= IDENTIFIER AS IDENTIFIER */
// 1528 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_use_aliases_item(yymsp[-2].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,47,&yymsp[-1].minor);
}
// 5578 "parser.cpp"
        break;
      case 36: /* xx_interface_def ::= INTERFACE IDENTIFIER xx_interface_body */
// 1532 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_interface(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,48,&yymsp[-2].minor);
}
// 5586 "parser.cpp"
        break;
      case 37: /* xx_interface_def ::= INTERFACE IDENTIFIER EXTENDS IDENTIFIER xx_interface_body */
// 1536 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_interface(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,48,&yymsp[-4].minor);
  yy_destructor(yypParser,49,&yymsp[-2].minor);
}
// 5595 "parser.cpp"
        break;
      case 38: /* xx_class_def ::= CLASS IDENTIFIER xx_class_body */
// 1540 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, 0, 0, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
}
// 5603 "parser.cpp"
        break;
      case 39: /* xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body */
// 1544 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,49,&yymsp[-2].minor);
}
// 5612 "parser.cpp"
        break;
      case 40: /* xx_class_def ::= CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body */
// 1548 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 0, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
}
// 5621 "parser.cpp"
        break;
      case 41: /* xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body */
// 1552 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy396, 0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,50,&yymsp[-6].minor);
  yy_destructor(yypParser,49,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
}
// 5631 "parser.cpp"
        break;
      case 42: /* xx_class_def ::= ABSTRACT CLASS IDENTIFIER xx_class_body */
// 1556 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, 1, 0, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,52,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
}
// 5640 "parser.cpp"
        break;
      case 43: /* xx_class_def ::= ABSTRACT CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body */
// 1560 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 1, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,52,&yymsp[-5].minor);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,49,&yymsp[-2].minor);
}
// 5650 "parser.cpp"
        break;
      case 44: /* xx_class_def ::= ABSTRACT CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body */
// 1564 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 1, 0, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,52,&yymsp[-5].minor);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
}
// 5660 "parser.cpp"
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
// 5671 "parser.cpp"
        break;
      case 46: /* xx_class_def ::= FINAL CLASS IDENTIFIER xx_class_body */
// 1572 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy396, 0, 1, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,53,&yymsp[-3].minor);
  yy_destructor(yypParser,50,&yymsp[-2].minor);
}
// 5680 "parser.cpp"
        break;
      case 47: /* xx_class_def ::= FINAL CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body */
// 1576 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,53,&yymsp[-5].minor);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,49,&yymsp[-2].minor);
}
// 5690 "parser.cpp"
        break;
      case 48: /* xx_class_def ::= FINAL CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body */
// 1580 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, 0, 1, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,53,&yymsp[-5].minor);
  yy_destructor(yypParser,50,&yymsp[-4].minor);
  yy_destructor(yypParser,51,&yymsp[-2].minor);
}
// 5700 "parser.cpp"
        break;
      case 49: /* xx_class_body ::= BRACKET_OPEN BRACKET_CLOSE */
      case 78: /* xx_class_property_shortcuts ::= BRACKET_OPEN BRACKET_CLOSE */ yytestcase(yyruleno==78);
// 1584 "parser.lemon"
{
	yygotominor.yy396 = NULL;
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 5710 "parser.cpp"
        break;
      case 50: /* xx_class_body ::= BRACKET_OPEN xx_class_definition BRACKET_CLOSE */
      case 79: /* xx_class_property_shortcuts ::= BRACKET_OPEN xx_class_property_shortcuts_list BRACKET_CLOSE */ yytestcase(yyruleno==79);
// 1588 "parser.lemon"
{
	yygotominor.yy396 = yymsp[-1].minor.yy396;
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 5720 "parser.cpp"
        break;
      case 53: /* xx_implements ::= IDENTIFIER */
      case 254: /* xx_catch_class ::= IDENTIFIER */ yytestcase(yyruleno==254);
      case 368: /* xx_common_expr ::= IDENTIFIER */ yytestcase(yyruleno==368);
      case 420: /* xx_array_key ::= IDENTIFIER */ yytestcase(yyruleno==420);
      case 439: /* xx_literal_array_key ::= IDENTIFIER */ yytestcase(yyruleno==439);
// 1600 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state);
}
// 5731 "parser.cpp"
        break;
      case 54: /* xx_interface_body ::= BRACKET_OPEN BRACKET_CLOSE */
// 1604 "parser.lemon"
{
  yygotominor.yy396 = NULL;
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 5740 "parser.cpp"
        break;
      case 55: /* xx_interface_body ::= BRACKET_OPEN xx_interface_definition BRACKET_CLOSE */
// 1608 "parser.lemon"
{
  yygotominor.yy396 = yymsp[-1].minor.yy396;
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 5749 "parser.cpp"
        break;
      case 56: /* xx_class_definition ::= xx_class_properties_definition */
// 1612 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
}
// 5756 "parser.cpp"
        break;
      case 57: /* xx_class_definition ::= xx_class_consts_definition */
// 1616 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 5763 "parser.cpp"
        break;
      case 58: /* xx_class_definition ::= xx_class_methods_definition */
// 1620 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(NULL, yymsp[0].minor.yy396, NULL, status->scanner_state);
}
// 5770 "parser.cpp"
        break;
      case 59: /* xx_class_definition ::= xx_class_properties_definition xx_class_methods_definition */
// 1624 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-1].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
}
// 5777 "parser.cpp"
        break;
      case 60: /* xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition */
// 1628 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-1].minor.yy396, NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 5784 "parser.cpp"
        break;
      case 61: /* xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition */
// 1632 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[0].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
}
// 5791 "parser.cpp"
        break;
      case 62: /* xx_class_definition ::= xx_class_consts_definition xx_class_methods_definition */
// 1636 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(NULL, yymsp[0].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
}
// 5798 "parser.cpp"
        break;
      case 63: /* xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition xx_class_methods_definition */
// 1640 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
}
// 5805 "parser.cpp"
        break;
      case 64: /* xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition xx_class_methods_definition */
// 1644 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_definition(yymsp[-1].minor.yy396, yymsp[0].minor.yy396, yymsp[-2].minor.yy396, status->scanner_state);
}
// 5812 "parser.cpp"
        break;
      case 65: /* xx_interface_definition ::= xx_class_consts_definition */
// 1648 "parser.lemon"
{
  yygotominor.yy396 = xx_ret_interface_definition(NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 5819 "parser.cpp"
        break;
      case 66: /* xx_interface_definition ::= xx_interface_methods_definition */
// 1652 "parser.lemon"
{
  yygotominor.yy396 = xx_ret_interface_definition(yymsp[0].minor.yy396, NULL, status->scanner_state);
}
// 5826 "parser.cpp"
        break;
      case 67: /* xx_interface_definition ::= xx_class_consts_definition xx_interface_methods_definition */
// 1656 "parser.lemon"
{
  yygotominor.yy396 = xx_ret_interface_definition(yymsp[0].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
}
// 5833 "parser.cpp"
        break;
      case 70: /* xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER DOTCOMMA */
// 1669 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-2].minor.yy396, yymsp[-1].minor.yy0, NULL, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5841 "parser.cpp"
        break;
      case 71: /* xx_class_property_definition ::= xx_visibility_list IDENTIFIER DOTCOMMA */
// 1673 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-2].minor.yy396, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5849 "parser.cpp"
        break;
      case 72: /* xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA */
// 1677 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-4].minor.yy396, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5858 "parser.cpp"
        break;
      case 73: /* xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA */
// 1681 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-4].minor.yy396, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5867 "parser.cpp"
        break;
      case 74: /* xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA */
// 1685 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, NULL, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5875 "parser.cpp"
        break;
      case 75: /* xx_class_property_definition ::= xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA */
// 1689 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5883 "parser.cpp"
        break;
      case 76: /* xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA */
// 1693 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-5].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, yymsp[-6].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-3].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5892 "parser.cpp"
        break;
      case 77: /* xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA */
// 1697 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_class_property(yymsp[-5].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-3].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 5901 "parser.cpp"
        break;
      case 82: /* xx_class_property_shortcut ::= IDENTIFIER */
// 1717 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_property_shortcut(NULL, yymsp[0].minor.yy0, status->scanner_state);
}
// 5908 "parser.cpp"
        break;
      case 83: /* xx_class_property_shortcut ::= COMMENT IDENTIFIER */
// 1721 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_property_shortcut(yymsp[-1].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
}
// 5915 "parser.cpp"
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
// 5926 "parser.cpp"
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
// 5937 "parser.cpp"
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
// 5949 "parser.cpp"
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
// 5961 "parser.cpp"
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
// 5973 "parser.cpp"
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
// 5985 "parser.cpp"
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
// 5997 "parser.cpp"
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
// 6009 "parser.cpp"
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
// 6021 "parser.cpp"
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
// 6033 "parser.cpp"
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
// 6045 "parser.cpp"
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
// 6057 "parser.cpp"
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
// 6069 "parser.cpp"
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
// 6081 "parser.cpp"
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
// 6094 "parser.cpp"
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
// 6107 "parser.cpp"
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
// 6120 "parser.cpp"
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
// 6133 "parser.cpp"
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
// 6146 "parser.cpp"
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
// 6159 "parser.cpp"
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
// 6172 "parser.cpp"
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
// 6185 "parser.cpp"
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
// 6198 "parser.cpp"
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
// 6211 "parser.cpp"
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
// 6224 "parser.cpp"
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
// 6237 "parser.cpp"
        break;
      case 128: /* xx_visibility ::= PUBLIC */
// 1912 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("public");
  yy_destructor(yypParser,1,&yymsp[0].minor);
}
// 6245 "parser.cpp"
        break;
      case 129: /* xx_visibility ::= PROTECTED */
// 1916 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("protected");
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
// 6253 "parser.cpp"
        break;
      case 130: /* xx_visibility ::= PRIVATE */
// 1920 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("private");
  yy_destructor(yypParser,4,&yymsp[0].minor);
}
// 6261 "parser.cpp"
        break;
      case 131: /* xx_visibility ::= STATIC */
// 1924 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("static");
  yy_destructor(yypParser,3,&yymsp[0].minor);
}
// 6269 "parser.cpp"
        break;
      case 132: /* xx_visibility ::= SCOPED */
// 1928 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("scoped");
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
// 6277 "parser.cpp"
        break;
      case 133: /* xx_visibility ::= INLINE */
// 1932 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("inline");
  yy_destructor(yypParser,62,&yymsp[0].minor);
}
// 6285 "parser.cpp"
        break;
      case 134: /* xx_visibility ::= DEPRECATED */
// 1936 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("deprecated");
  yy_destructor(yypParser,63,&yymsp[0].minor);
}
// 6293 "parser.cpp"
        break;
      case 135: /* xx_visibility ::= ABSTRACT */
// 1940 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("abstract");
  yy_destructor(yypParser,52,&yymsp[0].minor);
}
// 6301 "parser.cpp"
        break;
      case 136: /* xx_visibility ::= FINAL */
// 1944 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("final");
  yy_destructor(yypParser,53,&yymsp[0].minor);
}
// 6309 "parser.cpp"
        break;
      case 137: /* xx_method_return_type ::= VOID */
// 1949 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type(1, NULL, status->scanner_state);
  yy_destructor(yypParser,64,&yymsp[0].minor);
}
// 6317 "parser.cpp"
        break;
      case 138: /* xx_method_return_type ::= xx_method_return_type_list */
// 1953 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type(0, yymsp[0].minor.yy396, status->scanner_state);
}
// 6324 "parser.cpp"
        break;
      case 139: /* xx_method_return_type_list ::= xx_method_return_type_list BITWISE_OR xx_method_return_type_item */
      case 252: /* xx_catch_classes_list ::= xx_catch_classes_list BITWISE_OR xx_catch_class */ yytestcase(yyruleno==252);
// 1957 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_list(yymsp[-2].minor.yy396, yymsp[0].minor.yy396);
  yy_destructor(yypParser,14,&yymsp[-1].minor);
}
// 6333 "parser.cpp"
        break;
      case 141: /* xx_method_return_type_item ::= xx_parameter_type */
// 1965 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(yymsp[0].minor.yy396, NULL, 0, 0, status->scanner_state);
}
// 6340 "parser.cpp"
        break;
      case 142: /* xx_method_return_type_item ::= NULL */
// 1969 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_NULL), NULL, 0, 0, status->scanner_state);
  yy_destructor(yypParser,65,&yymsp[0].minor);
}
// 6348 "parser.cpp"
        break;
      case 143: /* xx_method_return_type_item ::= THIS */
// 1973 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_THIS), NULL, 0, 0, status->scanner_state);
  yy_destructor(yypParser,66,&yymsp[0].minor);
}
// 6356 "parser.cpp"
        break;
      case 144: /* xx_method_return_type_item ::= xx_parameter_type NOT */
// 1977 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(yymsp[-1].minor.yy396, NULL, 1, 0, status->scanner_state);
  yy_destructor(yypParser,39,&yymsp[0].minor);
}
// 6364 "parser.cpp"
        break;
      case 145: /* xx_method_return_type_item ::= xx_parameter_cast */
// 1981 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy396, 0, 0, status->scanner_state);
}
// 6371 "parser.cpp"
        break;
      case 146: /* xx_method_return_type_item ::= xx_parameter_cast_collection */
// 1985 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy396, 0, 1, status->scanner_state);
}
// 6378 "parser.cpp"
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
// 6390 "parser.cpp"
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
// 6402 "parser.cpp"
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
// 6414 "parser.cpp"
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
// 6426 "parser.cpp"
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
// 6438 "parser.cpp"
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
// 6450 "parser.cpp"
        break;
      case 155: /* xx_parameter ::= IDENTIFIER */
// 2031 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6457 "parser.cpp"
        break;
      case 156: /* xx_parameter ::= CONST IDENTIFIER */
// 2035 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-1].minor);
}
// 6465 "parser.cpp"
        break;
      case 157: /* xx_parameter ::= xx_parameter_type IDENTIFIER */
// 2039 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-1].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6472 "parser.cpp"
        break;
      case 158: /* xx_parameter ::= CONST xx_parameter_type IDENTIFIER */
// 2043 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-1].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-2].minor);
}
// 6480 "parser.cpp"
        break;
      case 159: /* xx_parameter ::= xx_parameter_type NOT IDENTIFIER */
// 2047 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-2].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
}
// 6488 "parser.cpp"
        break;
      case 160: /* xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER */
// 2051 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-2].minor.yy396, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-3].minor);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
}
// 6497 "parser.cpp"
        break;
      case 161: /* xx_parameter ::= xx_parameter_cast IDENTIFIER */
// 2055 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, yymsp[-1].minor.yy396, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6504 "parser.cpp"
        break;
      case 162: /* xx_parameter ::= CONST xx_parameter_cast IDENTIFIER */
// 2059 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, yymsp[-1].minor.yy396, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-2].minor);
}
// 6512 "parser.cpp"
        break;
      case 163: /* xx_parameter ::= IDENTIFIER ASSIGN xx_literal_expr */
// 2063 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6520 "parser.cpp"
        break;
      case 164: /* xx_parameter ::= CONST IDENTIFIER ASSIGN xx_literal_expr */
// 2067 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-3].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6529 "parser.cpp"
        break;
      case 165: /* xx_parameter ::= xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr */
// 2071 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-3].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6537 "parser.cpp"
        break;
      case 166: /* xx_parameter ::= CONST xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr */
// 2075 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-3].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-4].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6546 "parser.cpp"
        break;
      case 167: /* xx_parameter ::= xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr */
// 2079 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, yymsp[-4].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 1, status->scanner_state);
  yy_destructor(yypParser,39,&yymsp[-3].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6555 "parser.cpp"
        break;
      case 168: /* xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr */
// 2083 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, yymsp[-4].minor.yy396, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 1, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-5].minor);
  yy_destructor(yypParser,39,&yymsp[-3].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6565 "parser.cpp"
        break;
      case 169: /* xx_parameter ::= xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr */
// 2087 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(0, NULL, yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6573 "parser.cpp"
        break;
      case 170: /* xx_parameter ::= CONST xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr */
// 2091 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_parameter(1, NULL, yymsp[-3].minor.yy396, yymsp[-2].minor.yy0, yymsp[0].minor.yy396, 0, status->scanner_state);
  yy_destructor(yypParser,58,&yymsp[-4].minor);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 6582 "parser.cpp"
        break;
      case 171: /* xx_parameter_cast ::= LESS IDENTIFIER GREATER */
// 2096 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,21,&yymsp[-2].minor);
  yy_destructor(yypParser,22,&yymsp[0].minor);
}
// 6591 "parser.cpp"
        break;
      case 172: /* xx_parameter_cast_collection ::= LESS IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE GREATER */
// 2100 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state);
  yy_destructor(yypParser,21,&yymsp[-4].minor);
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yy_destructor(yypParser,67,&yymsp[-1].minor);
  yy_destructor(yypParser,22,&yymsp[0].minor);
}
// 6602 "parser.cpp"
        break;
      case 173: /* xx_parameter_type ::= TYPE_INTEGER */
// 2104 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_INTEGER);
  yy_destructor(yypParser,68,&yymsp[0].minor);
}
// 6610 "parser.cpp"
        break;
      case 174: /* xx_parameter_type ::= TYPE_UINTEGER */
// 2108 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_UINTEGER);
  yy_destructor(yypParser,69,&yymsp[0].minor);
}
// 6618 "parser.cpp"
        break;
      case 175: /* xx_parameter_type ::= TYPE_LONG */
// 2112 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_LONG);
  yy_destructor(yypParser,70,&yymsp[0].minor);
}
// 6626 "parser.cpp"
        break;
      case 176: /* xx_parameter_type ::= TYPE_ULONG */
// 2116 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_ULONG);
  yy_destructor(yypParser,71,&yymsp[0].minor);
}
// 6634 "parser.cpp"
        break;
      case 177: /* xx_parameter_type ::= TYPE_CHAR */
// 2120 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_CHAR);
  yy_destructor(yypParser,72,&yymsp[0].minor);
}
// 6642 "parser.cpp"
        break;
      case 178: /* xx_parameter_type ::= TYPE_UCHAR */
// 2124 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_UCHAR);
  yy_destructor(yypParser,73,&yymsp[0].minor);
}
// 6650 "parser.cpp"
        break;
      case 179: /* xx_parameter_type ::= TYPE_DOUBLE */
// 2128 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_DOUBLE);
  yy_destructor(yypParser,74,&yymsp[0].minor);
}
// 6658 "parser.cpp"
        break;
      case 180: /* xx_parameter_type ::= TYPE_BOOL */
// 2132 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_BOOL);
  yy_destructor(yypParser,75,&yymsp[0].minor);
}
// 6666 "parser.cpp"
        break;
      case 181: /* xx_parameter_type ::= TYPE_STRING */
// 2136 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_STRING);
  yy_destructor(yypParser,76,&yymsp[0].minor);
}
// 6674 "parser.cpp"
        break;
      case 182: /* xx_parameter_type ::= TYPE_ARRAY */
// 2140 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_ARRAY);
  yy_destructor(yypParser,77,&yymsp[0].minor);
}
// 6682 "parser.cpp"
        break;
      case 183: /* xx_parameter_type ::= TYPE_VAR */
// 2144 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_VAR);
  yy_destructor(yypParser,78,&yymsp[0].minor);
}
// 6690 "parser.cpp"
        break;
      case 184: /* xx_parameter_type ::= TYPE_CALLABLE */
// 2148 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_CALLABLE);
  yy_destructor(yypParser,79,&yymsp[0].minor);
}
// 6698 "parser.cpp"
        break;
      case 185: /* xx_parameter_type ::= TYPE_RESOURCE */
// 2152 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_RESOURCE);
  yy_destructor(yypParser,80,&yymsp[0].minor);
}
// 6706 "parser.cpp"
        break;
      case 186: /* xx_parameter_type ::= TYPE_OBJECT */
// 2156 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_type(XX_TYPE_OBJECT);
  yy_destructor(yypParser,81,&yymsp[0].minor);
}
// 6714 "parser.cpp"
        break;
      case 212: /* xx_empty_statement ::= DOTCOMMA */
// 2260 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_empty_statement(status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6722 "parser.cpp"
        break;
      case 213: /* xx_break_statement ::= BREAK DOTCOMMA */
// 2264 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_break_statement(status->scanner_state);
  yy_destructor(yypParser,82,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6731 "parser.cpp"
        break;
      case 214: /* xx_continue_statement ::= CONTINUE DOTCOMMA */
// 2268 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_continue_statement(status->scanner_state);
  yy_destructor(yypParser,83,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6740 "parser.cpp"
        break;
      case 215: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE */
// 2273 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-2].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6750 "parser.cpp"
        break;
      case 216: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements */
// 2278 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-3].minor.yy396, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[-1].minor);
}
// 6760 "parser.cpp"
        break;
      case 217: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE */
// 2283 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-5].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,85,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6773 "parser.cpp"
        break;
      case 218: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE */
// 2288 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-6].minor.yy396, NULL, yymsp[-3].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-7].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,85,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6786 "parser.cpp"
        break;
      case 219: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2293 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6796 "parser.cpp"
        break;
      case 220: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements */
// 2298 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-4].minor.yy396, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-3].minor);
  yy_destructor(yypParser,55,&yymsp[-1].minor);
}
// 6806 "parser.cpp"
        break;
      case 221: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2303 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-7].minor.yy396, yymsp[-5].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-8].minor);
  yy_destructor(yypParser,54,&yymsp[-6].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,85,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6819 "parser.cpp"
        break;
      case 222: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2308 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-8].minor.yy396, yymsp[-6].minor.yy396, yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-9].minor);
  yy_destructor(yypParser,54,&yymsp[-7].minor);
  yy_destructor(yypParser,55,&yymsp[-5].minor);
  yy_destructor(yypParser,85,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6832 "parser.cpp"
        break;
      case 223: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE */
// 2313 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-6].minor.yy396, yymsp[-4].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-7].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,85,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6845 "parser.cpp"
        break;
      case 224: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE */
// 2318 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-7].minor.yy396, yymsp[-5].minor.yy396, yymsp[-3].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-8].minor);
  yy_destructor(yypParser,54,&yymsp[-6].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,85,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6858 "parser.cpp"
        break;
      case 225: /* xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2323 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-6].minor.yy396, NULL, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,84,&yymsp[-7].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,85,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6871 "parser.cpp"
        break;
      case 228: /* xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE */
// 2336 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-2].minor.yy396, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,86,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6881 "parser.cpp"
        break;
      case 229: /* xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2341 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_if_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,86,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6891 "parser.cpp"
        break;
      case 230: /* xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN BRACKET_CLOSE */
// 2345 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_switch_statement(yymsp[-2].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,87,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6901 "parser.cpp"
        break;
      case 231: /* xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN xx_case_clauses BRACKET_CLOSE */
// 2349 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_switch_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,87,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6911 "parser.cpp"
        break;
      case 234: /* xx_case_clause ::= CASE xx_literal_expr COLON */
// 2361 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_case_clause(yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,88,&yymsp[-2].minor);
  yy_destructor(yypParser,89,&yymsp[0].minor);
}
// 6920 "parser.cpp"
        break;
      case 235: /* xx_case_clause ::= CASE xx_literal_expr COLON xx_statement_list */
// 2365 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_case_clause(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,88,&yymsp[-3].minor);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 6929 "parser.cpp"
        break;
      case 236: /* xx_case_clause ::= DEFAULT COLON xx_statement_list */
// 2369 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_case_clause(NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,90,&yymsp[-2].minor);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 6938 "parser.cpp"
        break;
      case 237: /* xx_loop_statement ::= LOOP BRACKET_OPEN BRACKET_CLOSE */
// 2373 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_loop_statement(NULL, status->scanner_state);
  yy_destructor(yypParser,91,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6948 "parser.cpp"
        break;
      case 238: /* xx_loop_statement ::= LOOP BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2377 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_loop_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,91,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6958 "parser.cpp"
        break;
      case 239: /* xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN BRACKET_CLOSE */
// 2381 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_while_statement(yymsp[-2].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,92,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6968 "parser.cpp"
        break;
      case 240: /* xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2385 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_while_statement(yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,92,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 6978 "parser.cpp"
        break;
      case 241: /* xx_do_while_statement ::= DO BRACKET_OPEN BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA */
// 2389 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_do_while_statement(yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,93,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,92,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 6990 "parser.cpp"
        break;
      case 242: /* xx_do_while_statement ::= DO BRACKET_OPEN xx_statement_list BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA */
// 2393 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_do_while_statement(yymsp[-1].minor.yy396, yymsp[-4].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,93,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,92,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7002 "parser.cpp"
        break;
      case 243: /* xx_try_catch_statement ::= TRY BRACKET_OPEN BRACKET_CLOSE */
// 2397 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_try_catch_statement(NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,94,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7012 "parser.cpp"
        break;
      case 244: /* xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2401 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_try_catch_statement(yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,94,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7022 "parser.cpp"
        break;
      case 245: /* xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_catch_statement_list */
// 2405 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_try_catch_statement(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,94,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-3].minor);
  yy_destructor(yypParser,55,&yymsp[-1].minor);
}
// 7032 "parser.cpp"
        break;
      case 248: /* xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2417 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-3].minor.yy396, NULL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,95,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7042 "parser.cpp"
        break;
      case 249: /* xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN BRACKET_CLOSE */
// 2421 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-2].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,95,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7052 "parser.cpp"
        break;
      case 250: /* xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN BRACKET_CLOSE */
// 2425 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-4].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,95,&yymsp[-5].minor);
  yy_destructor(yypParser,6,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7063 "parser.cpp"
        break;
      case 251: /* xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2429 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_catch_statement(yymsp[-5].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state), yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,95,&yymsp[-6].minor);
  yy_destructor(yypParser,6,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7074 "parser.cpp"
        break;
      case 255: /* xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2445 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, NULL, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-6].minor);
  yy_destructor(yypParser,97,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7085 "parser.cpp"
        break;
      case 256: /* xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE */
// 2449 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-2].minor.yy396, NULL, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-5].minor);
  yy_destructor(yypParser,97,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7096 "parser.cpp"
        break;
      case 257: /* xx_for_statement ::= FOR IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2453 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, NULL, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-7].minor);
  yy_destructor(yypParser,97,&yymsp[-5].minor);
  yy_destructor(yypParser,98,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7108 "parser.cpp"
        break;
      case 258: /* xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2457 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-8].minor);
  yy_destructor(yypParser,6,&yymsp[-6].minor);
  yy_destructor(yypParser,97,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7120 "parser.cpp"
        break;
      case 259: /* xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE */
// 2461 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-2].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-7].minor);
  yy_destructor(yypParser,6,&yymsp[-5].minor);
  yy_destructor(yypParser,97,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7132 "parser.cpp"
        break;
      case 260: /* xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 2465 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_for_statement(yymsp[-3].minor.yy396, yymsp[-8].minor.yy0, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,96,&yymsp[-9].minor);
  yy_destructor(yypParser,6,&yymsp[-7].minor);
  yy_destructor(yypParser,97,&yymsp[-5].minor);
  yy_destructor(yypParser,98,&yymsp[-4].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7145 "parser.cpp"
        break;
      case 261: /* xx_let_statement ::= LET xx_let_assignments DOTCOMMA */
// 2469 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,99,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7154 "parser.cpp"
        break;
      case 264: /* xx_assignment_operator ::= ASSIGN */
// 2482 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("assign");
  yy_destructor(yypParser,57,&yymsp[0].minor);
}
// 7162 "parser.cpp"
        break;
      case 265: /* xx_assignment_operator ::= ADDASSIGN */
// 2487 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("add-assign");
  yy_destructor(yypParser,100,&yymsp[0].minor);
}
// 7170 "parser.cpp"
        break;
      case 266: /* xx_assignment_operator ::= SUBASSIGN */
// 2492 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("sub-assign");
  yy_destructor(yypParser,101,&yymsp[0].minor);
}
// 7178 "parser.cpp"
        break;
      case 267: /* xx_assignment_operator ::= MULASSIGN */
// 2496 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("mul-assign");
  yy_destructor(yypParser,102,&yymsp[0].minor);
}
// 7186 "parser.cpp"
        break;
      case 268: /* xx_assignment_operator ::= DIVASSIGN */
// 2500 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("div-assign");
  yy_destructor(yypParser,103,&yymsp[0].minor);
}
// 7194 "parser.cpp"
        break;
      case 269: /* xx_assignment_operator ::= CONCATASSIGN */
// 2504 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("concat-assign");
  yy_destructor(yypParser,104,&yymsp[0].minor);
}
// 7202 "parser.cpp"
        break;
      case 270: /* xx_assignment_operator ::= MODASSIGN */
// 2508 "parser.lemon"
{
	yygotominor.yy396 = new Json::Value("mod-assign");
  yy_destructor(yypParser,105,&yymsp[0].minor);
}
// 7210 "parser.cpp"
        break;
      case 271: /* xx_let_assignment ::= IDENTIFIER xx_assignment_operator xx_assign_expr */
// 2513 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("variable", yymsp[-1].minor.yy396, yymsp[-2].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 7217 "parser.cpp"
        break;
      case 272: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_assignment_operator xx_assign_expr */
// 2518 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property", yymsp[-1].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
}
// 7225 "parser.cpp"
        break;
      case 273: /* xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2523 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("variable-dynamic-object-property", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
}
// 7235 "parser.cpp"
        break;
      case 274: /* xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2528 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("string-dynamic-object-property", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
}
// 7245 "parser.cpp"
        break;
      case 275: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2533 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-append", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7255 "parser.cpp"
        break;
      case 276: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr */
// 2538 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-array-index", yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-4].minor);
}
// 7263 "parser.cpp"
        break;
      case 277: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2542 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-array-index-append", yymsp[-1].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-6].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7273 "parser.cpp"
        break;
      case 278: /* xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_assignment_operator xx_assign_expr */
// 2547 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property", yymsp[-1].minor.yy396, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-3].minor);
}
// 7281 "parser.cpp"
        break;
      case 279: /* xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2552 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property-append", yymsp[-1].minor.yy396, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-5].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7291 "parser.cpp"
        break;
      case 280: /* xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr */
// 2557 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property-array-index", yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-4].minor);
}
// 7299 "parser.cpp"
        break;
      case 281: /* xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2562 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("static-property-array-index-append", yymsp[-1].minor.yy396, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-6].minor);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7309 "parser.cpp"
        break;
      case 282: /* xx_let_assignment ::= IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2567 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("variable-append", yymsp[-1].minor.yy396, yymsp[-4].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7318 "parser.cpp"
        break;
      case 283: /* xx_let_assignment ::= IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr */
// 2572 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("array-index", yymsp[-1].minor.yy396, yymsp[-3].minor.yy0, NULL, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
}
// 7325 "parser.cpp"
        break;
      case 284: /* xx_let_assignment ::= IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2577 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("array-index-append", yymsp[-1].minor.yy396, yymsp[-5].minor.yy0, NULL, yymsp[-4].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-3].minor);
  yy_destructor(yypParser,67,&yymsp[-2].minor);
}
// 7334 "parser.cpp"
        break;
      case 287: /* xx_array_offset ::= SBRACKET_OPEN xx_index_expr SBRACKET_CLOSE */
// 2589 "parser.lemon"
{
	yygotominor.yy396 = yymsp[-1].minor.yy396;
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yy_destructor(yypParser,67,&yymsp[0].minor);
}
// 7343 "parser.cpp"
        break;
      case 288: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER INCR */
// 2594 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-incr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-2].minor);
  yy_destructor(yypParser,108,&yymsp[0].minor);
}
// 7352 "parser.cpp"
        break;
      case 289: /* xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER DECR */
// 2599 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("object-property-decr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-2].minor);
  yy_destructor(yypParser,109,&yymsp[0].minor);
}
// 7361 "parser.cpp"
        break;
      case 290: /* xx_let_assignment ::= IDENTIFIER INCR */
// 2604 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("incr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,108,&yymsp[0].minor);
}
// 7369 "parser.cpp"
        break;
      case 291: /* xx_let_assignment ::= INCR IDENTIFIER */
// 2609 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("incr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,108,&yymsp[-1].minor);
}
// 7377 "parser.cpp"
        break;
      case 292: /* xx_let_assignment ::= IDENTIFIER DECR */
// 2614 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("decr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,109,&yymsp[0].minor);
}
// 7385 "parser.cpp"
        break;
      case 293: /* xx_let_assignment ::= DECR IDENTIFIER */
// 2619 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("decr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,109,&yymsp[-1].minor);
}
// 7393 "parser.cpp"
        break;
      case 294: /* xx_let_assignment ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2624 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("dynamic-variable", yymsp[-1].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
}
// 7402 "parser.cpp"
        break;
      case 295: /* xx_let_assignment ::= BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr */
// 2629 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_let_assignment("dynamic-variable-string", yymsp[-1].minor.yy396, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
}
// 7411 "parser.cpp"
        break;
      case 297: /* xx_echo_statement ::= ECHO xx_echo_expressions DOTCOMMA */
// 2637 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_echo_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,110,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7420 "parser.cpp"
        break;
      case 301: /* xx_mcall_statement ::= xx_mcall_expr DOTCOMMA */
// 2654 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7428 "parser.cpp"
        break;
      case 302: /* xx_fcall_statement ::= xx_fcall_expr DOTCOMMA */
// 2659 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7436 "parser.cpp"
        break;
      case 303: /* xx_scall_statement ::= xx_scall_expr DOTCOMMA */
// 2664 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7444 "parser.cpp"
        break;
      case 304: /* xx_fetch_statement ::= xx_fetch_expr DOTCOMMA */
// 2669 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fetch_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7452 "parser.cpp"
        break;
      case 305: /* xx_return_statement ::= RETURN xx_common_expr DOTCOMMA */
// 2674 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,111,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7461 "parser.cpp"
        break;
      case 306: /* xx_return_statement ::= RETURN DOTCOMMA */
// 2679 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_return_statement(NULL, status->scanner_state);
  yy_destructor(yypParser,111,&yymsp[-1].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7470 "parser.cpp"
        break;
      case 307: /* xx_require_statement ::= REQUIRE xx_common_expr DOTCOMMA */
// 2684 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_require_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,7,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7479 "parser.cpp"
        break;
      case 308: /* xx_unset_statement ::= UNSET xx_common_expr DOTCOMMA */
// 2689 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_unset_statement(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,112,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7488 "parser.cpp"
        break;
      case 309: /* xx_throw_statement ::= THROW xx_common_expr DOTCOMMA */
// 2694 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_throw_exception(yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,113,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7497 "parser.cpp"
        break;
      case 310: /* xx_declare_statement ::= TYPE_INTEGER xx_declare_variable_list DOTCOMMA */
// 2698 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_INTEGER, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,68,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7506 "parser.cpp"
        break;
      case 311: /* xx_declare_statement ::= TYPE_UINTEGER xx_declare_variable_list DOTCOMMA */
// 2702 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_UINTEGER, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,69,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7515 "parser.cpp"
        break;
      case 312: /* xx_declare_statement ::= TYPE_CHAR xx_declare_variable_list DOTCOMMA */
// 2706 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_CHAR, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,72,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7524 "parser.cpp"
        break;
      case 313: /* xx_declare_statement ::= TYPE_UCHAR xx_declare_variable_list DOTCOMMA */
// 2710 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_UCHAR, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,73,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7533 "parser.cpp"
        break;
      case 314: /* xx_declare_statement ::= TYPE_LONG xx_declare_variable_list DOTCOMMA */
// 2714 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_LONG, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,70,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7542 "parser.cpp"
        break;
      case 315: /* xx_declare_statement ::= TYPE_ULONG xx_declare_variable_list DOTCOMMA */
// 2718 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_ULONG, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,71,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7551 "parser.cpp"
        break;
      case 316: /* xx_declare_statement ::= TYPE_DOUBLE xx_declare_variable_list DOTCOMMA */
// 2722 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_DOUBLE, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,74,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7560 "parser.cpp"
        break;
      case 317: /* xx_declare_statement ::= TYPE_STRING xx_declare_variable_list DOTCOMMA */
// 2726 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_STRING, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,76,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7569 "parser.cpp"
        break;
      case 318: /* xx_declare_statement ::= TYPE_BOOL xx_declare_variable_list DOTCOMMA */
// 2730 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_BOOL, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,75,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7578 "parser.cpp"
        break;
      case 319: /* xx_declare_statement ::= TYPE_VAR xx_declare_variable_list DOTCOMMA */
// 2734 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_VAR, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,78,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7587 "parser.cpp"
        break;
      case 320: /* xx_declare_statement ::= TYPE_ARRAY xx_declare_variable_list DOTCOMMA */
// 2738 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_statement(XX_T_TYPE_ARRAY, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,77,&yymsp[-2].minor);
  yy_destructor(yypParser,45,&yymsp[0].minor);
}
// 7596 "parser.cpp"
        break;
      case 323: /* xx_declare_variable ::= IDENTIFIER */
// 2750 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_variable(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 7603 "parser.cpp"
        break;
      case 324: /* xx_declare_variable ::= IDENTIFIER ASSIGN xx_literal_expr */
// 2754 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_declare_variable(yymsp[-2].minor.yy0, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,57,&yymsp[-1].minor);
}
// 7611 "parser.cpp"
        break;
      case 326: /* xx_common_expr ::= NOT xx_common_expr */
// 2762 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("not", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,39,&yymsp[-1].minor);
}
// 7619 "parser.cpp"
        break;
      case 327: /* xx_common_expr ::= SUB xx_common_expr */
// 2766 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("minus", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,28,&yymsp[-1].minor);
}
// 7627 "parser.cpp"
        break;
      case 328: /* xx_common_expr ::= ISSET xx_common_expr */
// 2770 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("isset", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,33,&yymsp[-1].minor);
}
// 7635 "parser.cpp"
        break;
      case 329: /* xx_common_expr ::= REQUIRE xx_common_expr */
// 2774 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("require", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,7,&yymsp[-1].minor);
}
// 7643 "parser.cpp"
        break;
      case 330: /* xx_common_expr ::= CLONE xx_common_expr */
// 2778 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("clone", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,37,&yymsp[-1].minor);
}
// 7651 "parser.cpp"
        break;
      case 331: /* xx_common_expr ::= EMPTY xx_common_expr */
// 2782 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("empty", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,35,&yymsp[-1].minor);
}
// 7659 "parser.cpp"
        break;
      case 332: /* xx_common_expr ::= LIKELY xx_common_expr */
// 2786 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("likely", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,9,&yymsp[-1].minor);
}
// 7667 "parser.cpp"
        break;
      case 333: /* xx_common_expr ::= UNLIKELY xx_common_expr */
// 2790 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("unlikely", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,10,&yymsp[-1].minor);
}
// 7675 "parser.cpp"
        break;
      case 334: /* xx_common_expr ::= xx_common_expr EQUALS xx_common_expr */
// 2794 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("equals", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,19,&yymsp[-1].minor);
}
// 7683 "parser.cpp"
        break;
      case 335: /* xx_common_expr ::= xx_common_expr NOTEQUALS xx_common_expr */
// 2798 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("not-equals", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,26,&yymsp[-1].minor);
}
// 7691 "parser.cpp"
        break;
      case 336: /* xx_common_expr ::= xx_common_expr IDENTICAL xx_common_expr */
// 2802 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("identical", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,20,&yymsp[-1].minor);
}
// 7699 "parser.cpp"
        break;
      case 337: /* xx_common_expr ::= xx_common_expr NOTIDENTICAL xx_common_expr */
// 2806 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("not-identical", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,25,&yymsp[-1].minor);
}
// 7707 "parser.cpp"
        break;
      case 338: /* xx_common_expr ::= xx_common_expr LESS xx_common_expr */
// 2810 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("less", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,21,&yymsp[-1].minor);
}
// 7715 "parser.cpp"
        break;
      case 339: /* xx_common_expr ::= xx_common_expr GREATER xx_common_expr */
// 2814 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("greater", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,22,&yymsp[-1].minor);
}
// 7723 "parser.cpp"
        break;
      case 340: /* xx_common_expr ::= xx_common_expr LESSEQUAL xx_common_expr */
// 2818 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("less-equal", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,23,&yymsp[-1].minor);
}
// 7731 "parser.cpp"
        break;
      case 341: /* xx_common_expr ::= xx_common_expr GREATEREQUAL xx_common_expr */
// 2822 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("greater-equal", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,24,&yymsp[-1].minor);
}
// 7739 "parser.cpp"
        break;
      case 342: /* xx_common_expr ::= PARENTHESES_OPEN xx_common_expr PARENTHESES_CLOSE */
// 2826 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("list", yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 7748 "parser.cpp"
        break;
      case 343: /* xx_common_expr ::= PARENTHESES_OPEN xx_parameter_type PARENTHESES_CLOSE xx_common_expr */
// 2830 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("cast", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,61,&yymsp[-3].minor);
  yy_destructor(yypParser,40,&yymsp[-1].minor);
}
// 7757 "parser.cpp"
        break;
      case 344: /* xx_common_expr ::= LESS IDENTIFIER GREATER xx_common_expr */
// 2834 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("type-hint", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,21,&yymsp[-3].minor);
  yy_destructor(yypParser,22,&yymsp[-1].minor);
}
// 7766 "parser.cpp"
        break;
      case 345: /* xx_common_expr ::= xx_common_expr ARROW IDENTIFIER */
// 2838 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("property-access", yymsp[-2].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-1].minor);
}
// 7774 "parser.cpp"
        break;
      case 346: /* xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE */
// 2842 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("property-dynamic-access", yymsp[-4].minor.yy396, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7784 "parser.cpp"
        break;
      case 347: /* xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE */
// 2846 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("property-string-access", yymsp[-4].minor.yy396, xx_ret_literal(XX_T_STRING, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 7794 "parser.cpp"
        break;
      case 348: /* xx_common_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER */
// 2850 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("static-property-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-1].minor);
}
// 7802 "parser.cpp"
        break;
      case 349: /* xx_common_expr ::= IDENTIFIER DOUBLECOLON CONSTANT */
      case 431: /* xx_literal_expr ::= IDENTIFIER DOUBLECOLON CONSTANT */ yytestcase(yyruleno==431);
// 2854 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("static-constant-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-1].minor);
}
// 7811 "parser.cpp"
        break;
      case 350: /* xx_common_expr ::= xx_common_expr SBRACKET_OPEN xx_common_expr SBRACKET_CLOSE */
// 2863 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("array-access", yymsp[-3].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yy_destructor(yypParser,67,&yymsp[0].minor);
}
// 7820 "parser.cpp"
        break;
      case 351: /* xx_common_expr ::= xx_common_expr ADD xx_common_expr */
// 2868 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("add", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,27,&yymsp[-1].minor);
}
// 7828 "parser.cpp"
        break;
      case 352: /* xx_common_expr ::= xx_common_expr SUB xx_common_expr */
// 2873 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("sub", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,28,&yymsp[-1].minor);
}
// 7836 "parser.cpp"
        break;
      case 353: /* xx_common_expr ::= xx_common_expr MUL xx_common_expr */
// 2878 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("mul", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,30,&yymsp[-1].minor);
}
// 7844 "parser.cpp"
        break;
      case 354: /* xx_common_expr ::= xx_common_expr DIV xx_common_expr */
// 2883 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("div", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,31,&yymsp[-1].minor);
}
// 7852 "parser.cpp"
        break;
      case 355: /* xx_common_expr ::= xx_common_expr MOD xx_common_expr */
// 2888 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("mod", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,32,&yymsp[-1].minor);
}
// 7860 "parser.cpp"
        break;
      case 356: /* xx_common_expr ::= xx_common_expr CONCAT xx_common_expr */
// 2893 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("concat", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,29,&yymsp[-1].minor);
}
// 7868 "parser.cpp"
        break;
      case 357: /* xx_common_expr ::= xx_common_expr AND xx_common_expr */
// 2898 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("and", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,13,&yymsp[-1].minor);
}
// 7876 "parser.cpp"
        break;
      case 358: /* xx_common_expr ::= xx_common_expr OR xx_common_expr */
// 2903 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("or", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,12,&yymsp[-1].minor);
}
// 7884 "parser.cpp"
        break;
      case 359: /* xx_common_expr ::= xx_common_expr BITWISE_AND xx_common_expr */
// 2908 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_and", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,15,&yymsp[-1].minor);
}
// 7892 "parser.cpp"
        break;
      case 360: /* xx_common_expr ::= xx_common_expr BITWISE_OR xx_common_expr */
// 2913 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_or", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,14,&yymsp[-1].minor);
}
// 7900 "parser.cpp"
        break;
      case 361: /* xx_common_expr ::= xx_common_expr BITWISE_XOR xx_common_expr */
// 2918 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_xor", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,16,&yymsp[-1].minor);
}
// 7908 "parser.cpp"
        break;
      case 362: /* xx_common_expr ::= xx_common_expr BITWISE_SHIFTLEFT xx_common_expr */
// 2923 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_shiftleft", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,17,&yymsp[-1].minor);
}
// 7916 "parser.cpp"
        break;
      case 363: /* xx_common_expr ::= xx_common_expr BITWISE_SHIFTRIGHT xx_common_expr */
// 2928 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("bitwise_shiftright", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,18,&yymsp[-1].minor);
}
// 7924 "parser.cpp"
        break;
      case 364: /* xx_common_expr ::= xx_common_expr INSTANCEOF xx_common_expr */
// 2933 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("instanceof", yymsp[-2].minor.yy396, yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,11,&yymsp[-1].minor);
}
// 7932 "parser.cpp"
        break;
      case 365: /* xx_fetch_expr ::= FETCH IDENTIFIER COMMA xx_common_expr */
// 2938 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("fetch", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,34,&yymsp[-3].minor);
  yy_destructor(yypParser,6,&yymsp[-1].minor);
}
// 7941 "parser.cpp"
        break;
      case 367: /* xx_common_expr ::= TYPEOF xx_common_expr */
// 2948 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("typeof", yymsp[0].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,36,&yymsp[-1].minor);
}
// 7949 "parser.cpp"
        break;
      case 369: /* xx_common_expr ::= INTEGER */
      case 422: /* xx_array_key ::= INTEGER */ yytestcase(yyruleno==422);
      case 424: /* xx_literal_expr ::= INTEGER */ yytestcase(yyruleno==424);
      case 441: /* xx_literal_array_key ::= INTEGER */ yytestcase(yyruleno==441);
// 2958 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_INTEGER, yymsp[0].minor.yy0, status->scanner_state);
}
// 7959 "parser.cpp"
        break;
      case 370: /* xx_common_expr ::= STRING */
      case 421: /* xx_array_key ::= STRING */ yytestcase(yyruleno==421);
      case 426: /* xx_literal_expr ::= STRING */ yytestcase(yyruleno==426);
      case 440: /* xx_literal_array_key ::= STRING */ yytestcase(yyruleno==440);
// 2963 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_STRING, yymsp[0].minor.yy0, status->scanner_state);
}
// 7969 "parser.cpp"
        break;
      case 371: /* xx_common_expr ::= CHAR */
      case 425: /* xx_literal_expr ::= CHAR */ yytestcase(yyruleno==425);
// 2968 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_CHAR, yymsp[0].minor.yy0, status->scanner_state);
}
// 7977 "parser.cpp"
        break;
      case 372: /* xx_common_expr ::= DOUBLE */
      case 427: /* xx_literal_expr ::= DOUBLE */ yytestcase(yyruleno==427);
// 2973 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_DOUBLE, yymsp[0].minor.yy0, status->scanner_state);
}
// 7985 "parser.cpp"
        break;
      case 373: /* xx_common_expr ::= NULL */
      case 428: /* xx_literal_expr ::= NULL */ yytestcase(yyruleno==428);
// 2978 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,65,&yymsp[0].minor);
}
// 7994 "parser.cpp"
        break;
      case 374: /* xx_common_expr ::= TRUE */
      case 430: /* xx_literal_expr ::= TRUE */ yytestcase(yyruleno==430);
// 2983 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_TRUE, NULL, status->scanner_state);
  yy_destructor(yypParser,117,&yymsp[0].minor);
}
// 8003 "parser.cpp"
        break;
      case 375: /* xx_common_expr ::= FALSE */
      case 429: /* xx_literal_expr ::= FALSE */ yytestcase(yyruleno==429);
// 2988 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_FALSE, NULL, status->scanner_state);
  yy_destructor(yypParser,118,&yymsp[0].minor);
}
// 8012 "parser.cpp"
        break;
      case 376: /* xx_common_expr ::= CONSTANT */
      case 419: /* xx_array_key ::= CONSTANT */ yytestcase(yyruleno==419);
      case 432: /* xx_literal_expr ::= CONSTANT */ yytestcase(yyruleno==432);
// 2993 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_literal(XX_T_CONSTANT, yymsp[0].minor.yy0, status->scanner_state);
}
// 8021 "parser.cpp"
        break;
      case 377: /* xx_common_expr ::= SBRACKET_OPEN SBRACKET_CLOSE */
      case 433: /* xx_literal_expr ::= SBRACKET_OPEN SBRACKET_CLOSE */ yytestcase(yyruleno==433);
// 2998 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("empty-array", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-1].minor);
  yy_destructor(yypParser,67,&yymsp[0].minor);
}
// 8031 "parser.cpp"
        break;
      case 378: /* xx_common_expr ::= SBRACKET_OPEN xx_array_list SBRACKET_CLOSE */
      case 434: /* xx_literal_expr ::= SBRACKET_OPEN xx_literal_array_list SBRACKET_CLOSE */ yytestcase(yyruleno==434);
// 3003 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("array", yymsp[-1].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,41,&yymsp[-2].minor);
  yy_destructor(yypParser,67,&yymsp[0].minor);
}
// 8041 "parser.cpp"
        break;
      case 379: /* xx_common_expr ::= NEW IDENTIFIER */
// 3008 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(0, yymsp[0].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-1].minor);
}
// 8049 "parser.cpp"
        break;
      case 380: /* xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3013 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8059 "parser.cpp"
        break;
      case 381: /* xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3018 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8069 "parser.cpp"
        break;
      case 382: /* xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE */
// 3023 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8079 "parser.cpp"
        break;
      case 383: /* xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3028 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8091 "parser.cpp"
        break;
      case 384: /* xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3033 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_new_instance(1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,38,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8103 "parser.cpp"
        break;
      case 385: /* xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3038 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(1, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8112 "parser.cpp"
        break;
      case 386: /* xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3043 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(1, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8121 "parser.cpp"
        break;
      case 387: /* xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3048 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(2, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8132 "parser.cpp"
        break;
      case 388: /* xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3053 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_fcall(2, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8143 "parser.cpp"
        break;
      case 389: /* xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3058 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(0, yymsp[-5].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8153 "parser.cpp"
        break;
      case 390: /* xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3063 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(0, yymsp[-4].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,107,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8163 "parser.cpp"
        break;
      case 391: /* xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3068 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-6].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-7].minor);
  yy_destructor(yypParser,55,&yymsp[-5].minor);
  yy_destructor(yypParser,107,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8175 "parser.cpp"
        break;
      case 392: /* xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3073 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_scall(1, yymsp[-5].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,54,&yymsp[-6].minor);
  yy_destructor(yypParser,55,&yymsp[-4].minor);
  yy_destructor(yypParser,107,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8187 "parser.cpp"
        break;
      case 393: /* xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3078 "parser.lemon"
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
// 8201 "parser.cpp"
        break;
      case 394: /* xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3083 "parser.lemon"
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
// 8215 "parser.cpp"
        break;
      case 395: /* xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3088 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(1, yymsp[-5].minor.yy396, yymsp[-3].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8225 "parser.cpp"
        break;
      case 396: /* xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3093 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(1, yymsp[-4].minor.yy396, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8235 "parser.cpp"
        break;
      case 397: /* xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3098 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(2, yymsp[-7].minor.yy396, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8247 "parser.cpp"
        break;
      case 398: /* xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3103 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(2, yymsp[-6].minor.yy396, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8259 "parser.cpp"
        break;
      case 399: /* xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE */
// 3108 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(3, yymsp[-7].minor.yy396, yymsp[-4].minor.yy0, yymsp[-1].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-6].minor);
  yy_destructor(yypParser,54,&yymsp[-5].minor);
  yy_destructor(yypParser,55,&yymsp[-3].minor);
  yy_destructor(yypParser,61,&yymsp[-2].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8271 "parser.cpp"
        break;
      case 400: /* xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE */
// 3113 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_mcall(3, yymsp[-6].minor.yy396, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(yypParser,42,&yymsp[-5].minor);
  yy_destructor(yypParser,54,&yymsp[-4].minor);
  yy_destructor(yypParser,55,&yymsp[-2].minor);
  yy_destructor(yypParser,61,&yymsp[-1].minor);
  yy_destructor(yypParser,40,&yymsp[0].minor);
}
// 8283 "parser.cpp"
        break;
      case 404: /* xx_common_expr ::= xx_common_expr QUESTION xx_common_expr COLON xx_common_expr */
// 3133 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("ternary", yymsp[-4].minor.yy396, yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,8,&yymsp[-3].minor);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 8292 "parser.cpp"
        break;
      case 407: /* xx_call_parameter ::= xx_common_expr */
// 3146 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy396, status->scanner_state, 0);
}
// 8299 "parser.cpp"
        break;
      case 408: /* xx_call_parameter ::= IDENTIFIER COLON xx_common_expr */
// 3151 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(yymsp[-2].minor.yy0, yymsp[0].minor.yy396, status->scanner_state, 0);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 8307 "parser.cpp"
        break;
      case 409: /* xx_call_parameter ::= BITWISE_AND xx_common_expr */
// 3156 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy396, status->scanner_state, 1);
  yy_destructor(yypParser,15,&yymsp[-1].minor);
}
// 8315 "parser.cpp"
        break;
      case 410: /* xx_call_parameter ::= IDENTIFIER COLON BITWISE_AND xx_common_expr */
// 3161 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_call_parameter(yymsp[-3].minor.yy0, yymsp[0].minor.yy396, status->scanner_state, 0);
  yy_destructor(yypParser,89,&yymsp[-2].minor);
  yy_destructor(yypParser,15,&yymsp[-1].minor);
}
// 8324 "parser.cpp"
        break;
      case 411: /* xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 3166 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-4].minor);
  yy_destructor(yypParser,61,&yymsp[-3].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8336 "parser.cpp"
        break;
      case 412: /* xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 3171 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", NULL, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-5].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8348 "parser.cpp"
        break;
      case 413: /* xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE */
// 3176 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", yymsp[-3].minor.yy396, NULL, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-5].minor);
  yy_destructor(yypParser,61,&yymsp[-4].minor);
  yy_destructor(yypParser,40,&yymsp[-2].minor);
  yy_destructor(yypParser,54,&yymsp[-1].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8360 "parser.cpp"
        break;
      case 414: /* xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE */
// 3181 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_expr("closure", yymsp[-4].minor.yy396, yymsp[-1].minor.yy396, NULL, status->scanner_state);
  yy_destructor(yypParser,60,&yymsp[-6].minor);
  yy_destructor(yypParser,61,&yymsp[-5].minor);
  yy_destructor(yypParser,40,&yymsp[-3].minor);
  yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,55,&yymsp[0].minor);
}
// 8372 "parser.cpp"
        break;
      case 417: /* xx_array_item ::= xx_array_key COLON xx_array_value */
      case 437: /* xx_literal_array_item ::= xx_literal_array_key COLON xx_literal_array_value */ yytestcase(yyruleno==437);
// 3193 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_array_item(yymsp[-2].minor.yy396, yymsp[0].minor.yy396, status->scanner_state);
  yy_destructor(yypParser,89,&yymsp[-1].minor);
}
// 8381 "parser.cpp"
        break;
      case 418: /* xx_array_item ::= xx_array_value */
      case 438: /* xx_literal_array_item ::= xx_literal_array_value */ yytestcase(yyruleno==438);
// 3197 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_array_item(NULL, yymsp[0].minor.yy396, status->scanner_state);
}
// 8389 "parser.cpp"
        break;
      case 444: /* xx_comment ::= COMMENT */
// 3302 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_comment(yymsp[0].minor.yy0, status->scanner_state);
}
// 8396 "parser.cpp"
        break;
      case 445: /* xx_cblock ::= CBLOCK */
// 3306 "parser.lemon"
{
	yygotominor.yy396 = xx_ret_cblock(yymsp[0].minor.yy0, status->scanner_state);
}
// 8403 "parser.cpp"
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
// 8532 "parser.cpp"
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
