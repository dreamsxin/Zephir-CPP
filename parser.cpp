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


// 1223 "parser.cpp"
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
#define YYNSTATE 883
#define YYNRULE 439
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
 /*     0 */   444,   40,   61,   64,  736,   42,   35,  738,  840,  846,
 /*    10 */   169,  845,  830,  185,  127,   72,   68,  724,  806,  236,
 /*    20 */   244,   46,  564,  701,  824,   33,   49,  138,   58,  143,
 /*    30 */    55,  163,   43,  136,  153,  819,  825,  131,  275,  676,
 /*    40 */   677,  679,  678,  680,  168,   34,  883,  170,  563,  493,
 /*    50 */   669,  132,  152,  183,  107,  235,  236,  244,  149,  564,
 /*    60 */   214,  453,  462,  471,  474,  465,  468,  477,  483,  480,
 /*    70 */   489,  486,  695,   69,  664,  276,  278,  280,  236,  244,
 /*    80 */   290,  564,   31,  606,  301,  305,  310,  318,   38,  329,
 /*    90 */   683,  684,  336,  638,  704,  293,  714,   30,  162,  146,
 /*   100 */   681,  682,   42,  425,  440,  447,  450,  145,  147,  148,
 /*   110 */   150,  151,  494,  444,  213,   61,   64,   75,   77,   85,
 /*   120 */    79,   81,   83,  236,  244,  351,  187,  127,  297,  350,
 /*   130 */   534,   72,   68,  214,   46,  177,  179,  178,  142,   49,
 /*   140 */   138,   58,  143,   55,  163,   43,  751,  153,  629,  750,
 /*   150 */   131,  275,  676,  677,  679,  678,  680,  741,  746,  161,
 /*   160 */   170,  562,  493,  740,   42,  152,  183,  107,  701,   42,
 /*   170 */   349,  149,  402,  371,  453,  462,  471,  474,  465,  468,
 /*   180 */   477,  483,  480,  489,  486,  210,  211,  215,  276,  278,
 /*   190 */   280,  164, 1296,  290,  221,  229,   36,  301,  305,  310,
 /*   200 */   318,  584,  329,  683,  684,  336,  947,  704,  364,  714,
 /*   210 */   132,  370,  146,  681,  682,  132,  425,  440,  447,  450,
 /*   220 */   145,  147,  148,  150,  151,  494,  444,  686,   61,   64,
 /*   230 */   354,   71,  363,  371,   79,   81,   83,  295,  701,  173,
 /*   240 */   127,  177,  179,  178,  142,   72,   68,   46,  626,  180,
 /*   250 */   546,  540,   49,  138,   58,  143,   55,  163,   43,  385,
 /*   260 */   153,  391,  371,  131,  275,  676,  677,  679,  678,  680,
 /*   270 */   297,  634,  534,  170,  552,  493,  637,  225,  152,  183,
 /*   280 */   107,  728,  222,  851,  149,  570,  734,  453,  462,  471,
 /*   290 */   474,  465,  468,  477,  483,  480,  489,  486,  214,  341,
 /*   300 */   516,  276,  278,  280,  353, 1315,  290,   37,  522,  415,
 /*   310 */   301,  305,  310,  318,  374,  329,  683,  684,  336,  735,
 /*   320 */   809,   38,  714,  199,  169,  146,  681,  682,  133,  425,
 /*   330 */   440,  447,  450,  145,  147,  148,  150,  151,  494,  444,
 /*   340 */   186,   61,   64,  602,   71,  285,  551,   70,  326,  207,
 /*   350 */   216,  211,  215,  127,  177,  179,  178,  142,  175,  375,
 /*   360 */    46,  603,  180,  411,  413,   49,  138,   58,  143,   55,
 /*   370 */   163,   43,  765,  153,  248,  764,  131,  275,  676,  677,
 /*   380 */   679,  678,  680,  644,  760,  129,  170,  284,  493,  571,
 /*   390 */   643,  152,  183,  107,  701,  286,  540,  149,  209,  576,
 /*   400 */   453,  462,  471,  474,  465,  468,  477,  483,  480,  489,
 /*   410 */   486,  758,  294,  537,  276,  278,  280,  236,  244,  290,
 /*   420 */   564,  379,  240,  301,  305,  310,  318,  237,  329,  683,
 /*   430 */   684,  336,  938,  809,  502,  714,  322,  526,  146,  681,
 /*   440 */   682,  824,  425,  440,  447,  450,  145,  147,  148,  150,
 /*   450 */   151,  494,  444,  825,   61,   64,  858,   71,  864,  208,
 /*   460 */   783,  734,  842,  782,  808,  830,  127,  177,  179,  178,
 /*   470 */   142,  806,  778,   46,  599,  180,  701,   39,   49,  138,
 /*   480 */    58,  143,   55,  163,   43,  797,  153,  339,  796,  131,
 /*   490 */   275,  676,  677,  679,  678,  680,  732,  792,  850,  170,
 /*   500 */   538,  493,  325,  525,  152,  183,  107,  871,  392,  874,
 /*   510 */   149,  370,  734,  453,  462,  471,  474,  465,  468,  477,
 /*   520 */   483,  480,  489,  486,  790,  331,  338,  276,  278,  280,
 /*   530 */   236,  244,  290,  564,  337,  424,  301,  305,  310,  318,
 /*   540 */   125,  329,  683,  684,  336,  942,  807,  403,  714,  416,
 /*   550 */   370,  146,  681,  682,  128,  425,  440,  447,  450,  145,
 /*   560 */   147,  148,  150,  151,  494,  444,  649,   61,   64,  668,
 /*   570 */    71,  428,  134,  655,  656,  660,  661,  454,  461,  127,
 /*   580 */   177,  179,  178,  142,  139,  456,   46,  167,  180,  812,
 /*   590 */   140,   49,  138,   58,  143,   55,  163,   43,  818,  153,
 /*   600 */   456,  813,  131,  275,  676,  677,  679,  678,  680, 1298,
 /*   610 */   427,  420,  170,  289,  493,  463,  461,  152,  183,  107,
 /*   620 */   466,  461,  671,  149,  455,  674,  453,  462,  471,  474,
 /*   630 */   465,  468,  477,  483,  480,  489,  486,  469,  461,  464,
 /*   640 */   276,  278,  280,  456,  757,  290,  456,  472,  461,  301,
 /*   650 */   305,  310,  318,  753,  329,  683,  684,  336,  939,  809,
 /*   660 */   607,  714, 1297,  456,  146,  681,  682, 1295,  425,  440,
 /*   670 */   447,  450,  145,  147,  148,  150,  151,  494,  444,  832,
 /*   680 */    61,   64,  467,   71,  456,  470,  475,  461,  818,  478,
 /*   690 */   461,  833,  127,  177,  179,  178,  142,  165,  456,   46,
 /*   700 */   174,  180,  473,  171,   49,  138,   58,  143,   55,  163,
 /*   710 */    43,  456,  153,  481,  461,  131,  275,  676,  677,  679,
 /*   720 */   678,  680,  613,  476,  771,  170,  533,  493,  484,  461,
 /*   730 */   152,  183,  107,  767,  456,  456,  149,  479,  172,  453,
 /*   740 */   462,  471,  474,  465,  468,  477,  483,  480,  489,  486,
 /*   750 */   482,  710,  342,  276,  278,  280,  487,  461,  290,  490,
 /*   760 */   461,  184,  301,  305,  310,  318,  706,  329,  683,  684,
 /*   770 */   336,  949,  685,  485,  488,  456,  847,  146,  681,  682,
 /*   780 */   189,  425,  440,  447,  450,  145,  147,  148,  150,  151,
 /*   790 */   494,  444,  190,   61,   64,  343,  344,  345,  346,  347,
 /*   800 */   348,  169,  557,  540,  650,  127,  653,  667,  656,  660,
 /*   810 */   661,  199,   46,  169,  491,  553,  551,   49,  138,   58,
 /*   820 */   143,   55,  163,   43,  734,  153,  554,  540,  131,  275,
 /*   830 */   676,  677,  679,  678,  680,  574,  200,  789,  170,  304,
 /*   840 */   493,  803, 1317,  152,  183,  107,  785,  581,  691,  149,
 /*   850 */   799,  694,  453,  462,  471,  474,  465,  468,  477,  483,
 /*   860 */   480,  489,  486,  822,  719, 1316,  276,  278,  280,  697,
 /*   870 */   731,  290,  700,  734,  169,  301,  305,  310,  318,  715,
 /*   880 */   329,  683,  684,  336,  948,  685,  820,  827,  212,  219,
 /*   890 */   146,  681,  682,  220,  425,  440,  447,  450,  145,  147,
 /*   900 */   148,  150,  151,  494,  444,  169,   61,   64,  590,  223,
 /*   910 */   861,  430,  821,  734,  169,  169,  226,  773,  127,  426,
 /*   920 */   431,  177,  179,  178,  142,   46,  169,  169,  701,  186,
 /*   930 */    49,  138,   58,  143,   55,  163,   43,  224,  153,  600,
 /*   940 */   227,  131,  275,  676,  677,  679,  678,  680,  604,  611,
 /*   950 */   815,  170,  532,  493,  228,  230,  152,  183,  107,  818,
 /*   960 */   617,  627,  149,  670,  231,  453,  462,  471,  474,  465,
 /*   970 */   468,  477,  483,  480,  489,  486,  835,  232,  234,  276,
 /*   980 */   278,  280,  235,  238,  290,  818,  239,  186,  301,  305,
 /*   990 */   310,  318,  241,  329,  683,  684,  336,  941,  772,  852,
 /*  1000 */   243,  850,  186,  146,  681,  682,  242,  425,  440,  447,
 /*  1010 */   450,  145,  147,  148,  150,  151,  494,  444,  186,   61,
 /*  1020 */    64,  696,   71,  862,  245,  850,  810,  865,  875,  850,
 /*  1030 */   850,  127,  177,  179,  178,  142,  759,  701,   46,  573,
 /*  1040 */   180,  246,  810,   49,  138,   58,  143,   55,  163,   43,
 /*  1050 */   247,  153,  791,  701,  131,  275,  676,  677,  679,  678,
 /*  1060 */   680,  249,  277,  279,  170,  309,  493,  282,  287,  152,
 /*  1070 */   183,  107,  296,  539,  292,  149,  298,  299,  453,  462,
 /*  1080 */   471,  474,  465,  468,  477,  483,  480,  489,  486,  302,
 /*  1090 */   307,  311,  276,  278,  280,  314,  316,  290,  319,  324,
 /*  1100 */   340,  301,  305,  310,  318,  323,  329,  683,  684,  336,
 /*  1110 */   945,  772,  524,  330,  358,  361,  146,  681,  682,  368,
 /*  1120 */   425,  440,  447,  450,  145,  147,  148,  150,  151,  494,
 /*  1130 */   444,  376,   61,   64,  377,   71,  380,  384,  412,  381,
 /*  1140 */   389,  396,  400,  414,  127,  177,  179,  178,  142,  407,
 /*  1150 */   417,   46,  580,  180,  418,  421,   49,  138,   58,  143,
 /*  1160 */    55,  163,   43,  422,  153,  433,  435,  131,  275,  676,
 /*  1170 */   677,  679,  678,  680,  437,  439,  458,  170,  528,  493,
 /*  1180 */   457,  459,  152,  183,  107,  460,  503,  504,  149,  517,
 /*  1190 */   518,  453,  462,  471,  474,  465,  468,  477,  483,  480,
 /*  1200 */   489,  486,  523,  529,  281,  276,  278,  280,  531,  542,
 /*  1210 */   290,  535,  811,  317,  301,  305,  310,  318,  547,  329,
 /*  1220 */   683,  684,  336,  177,  179,  178,  142,  555,  774,  146,
 /*  1230 */   681,  682,  556,  425,  440,  447,  450,  145,  147,  148,
 /*  1240 */   150,  151,  494,  444,  558,   61,   64,  566,   71,  572,
 /*  1250 */   577,  578,  579,  585,  586,  587,  593,  127,  177,  179,
 /*  1260 */   178,  142,  608,  609,   46,  589,  180,  614,  630,   49,
 /*  1270 */   138,   58,  143,   55,  163,   43,  615,  153,  632,  633,
 /*  1280 */   131,  275,  676,  677,  679,  678,  680,  635,  636,  637,
 /*  1290 */   170,  313,  493,  640,  646,  152,  183,  107,  675,  651,
 /*  1300 */   647,  149,  654,  657,  453,  462,  471,  474,  465,  468,
 /*  1310 */   477,  483,  480,  489,  486,  663,  665,  291,  276,  278,
 /*  1320 */   280,  666,  673,  290,  688,  831,  317,  301,  305,  310,
 /*  1330 */   318,  689,  329,  683,  684,  336,  177,  179,  178,  142,
 /*  1340 */   693,  742,  146,  681,  682,  699,  425,  440,  447,  450,
 /*  1350 */   145,  147,  148,  150,  151,  494,  444,  709,   61,   64,
 /*  1360 */   707,   71,  711,  708,  712,  713,  716,  717,  718,  722,
 /*  1370 */   127,  177,  179,  178,  142,  720,  721,   46,  610,  180,
 /*  1380 */   726,  727,   49,  138,   58,  143,   55,  163,   43,  729,
 /*  1390 */   153,  849,  730,  131,  275,  676,  677,  679,  678,  680,
 /*  1400 */   733,  737,  743,  170,  527,  493,  775,  814,  152,  183,
 /*  1410 */   107,  744,  776,  817,  149,  828,  816,  453,  462,  471,
 /*  1420 */   474,  465,  468,  477,  483,  480,  489,  486,  960,  961,
 /*  1430 */   826,  276,  278,  280,  823,  829,  290,  838,  834,  848,
 /*  1440 */   301,  305,  310,  318,  837,  329,  683,  684,  336,  943,
 /*  1450 */   843,  836,  839,  853,  855,  146,  681,  682,  856,  425,
 /*  1460 */   440,  447,  450,  145,  147,  148,  150,  151,  494,  444,
 /*  1470 */   859,   61,   64,  857,   71,  868,  734,  860,  863,  866,
 /*  1480 */   869,  870,  872,  127,  177,  179,  178,  142,  659,  873,
 /*  1490 */    46,  616,  180,  876,  659,   49,  138,   58,  143,   55,
 /*  1500 */   163,   43,  659,  153,  659,  659,  131,  275,  676,  677,
 /*  1510 */   679,  678,  680,  659,  659,  659,  170,  321,  493,  659,
 /*  1520 */   659,  152,  183,  107,  659,  659,  659,  149,  659,  659,
 /*  1530 */   453,  462,  471,  474,  465,  468,  477,  483,  480,  489,
 /*  1540 */   486,  659,  659,  659,  276,  278,  280,  659,  659,  290,
 /*  1550 */   659,  659,  659,  301,  305,  310,  318,  659,  329,  683,
 /*  1560 */   684,  336,  946,  772,  659,  659,  659,  659,  146,  681,
 /*  1570 */   682,  659,  425,  440,  447,  450,  145,  147,  148,  150,
 /*  1580 */   151,  494,  444,  659,   61,   64,  659,  659,  639,  659,
 /*  1590 */   641,  659,  723,  642,  644,  644,  127,  659,  659,  724,
 /*  1600 */   702,  703,  703,   46,  659,  701,  701,  659,   49,  138,
 /*  1610 */    58,  143,   55,  163,   43,  659,  153,  659,  659,  131,
 /*  1620 */   275,  676,  677,  679,  678,  680,  659,  659,  659,  170,
 /*  1630 */   515,  493,  659,  659,  152,  183,  107,  659,  659,  659,
 /*  1640 */   149,  659,  659,  453,  462,  471,  474,  465,  468,  477,
 /*  1650 */   483,  480,  489,  486,  659,  659,  659,  276,  278,  280,
 /*  1660 */   659,  659,  290,  659,  659,  659,  301,  305,  310,  318,
 /*  1670 */   659,  329,  683,  684,  336,  944,  772,  659,  659,  659,
 /*  1680 */   659,  146,  681,  682,  659,  425,  440,  447,  450,  145,
 /*  1690 */   147,  148,  150,  151,  494,  444,  659,   61,   64,  659,
 /*  1700 */   659,  659,  659,  804,  739,  659,  808,  830,  659,  127,
 /*  1710 */   659,  659,  724,  806,  659,  659,   46,  659,  701,  659,
 /*  1720 */   659,   49,  138,   58,  143,   55,  163,   43,  659,  153,
 /*  1730 */   659,  659,  131,  275,  676,  677,  679,  678,  680,  659,
 /*  1740 */   659,  659,  170,  328,  493,  659,  659,  152,  183,  107,
 /*  1750 */   659,  659,  659,  149,  659,  659,  453,  462,  471,  474,
 /*  1760 */   465,  468,  477,  483,  480,  489,  486,  659,  659,  659,
 /*  1770 */   276,  278,  280,  659,  659,  290,  659,  659,  659,  301,
 /*  1780 */   305,  310,  318,  659,  329,  683,  684,  336,  940,  772,
 /*  1790 */   659,  659,  659,  659,  146,  681,  682,  659,  425,  440,
 /*  1800 */   447,  450,  145,  147,  148,  150,  151,  494,  444,  659,
 /*  1810 */    61,   64,  659,  659,  659,  841,  659,  844,  659,  845,
 /*  1820 */   830,  659,  127,  659,  659,  702,  806,  659,  659,   46,
 /*  1830 */   659,  701,  659,  659,   49,  138,   58,  143,   55,  163,
 /*  1840 */    43,  659,  153,  659,  659,  131,  275,  676,  677,  679,
 /*  1850 */   678,  680,  659,  659,  659,  170,  496,  493,  659,  659,
 /*  1860 */   152,  183,  107,  659,  659,  659,  149,  659,  659,  453,
 /*  1870 */   462,  471,  474,  465,  468,  477,  483,  480,  489,  486,
 /*  1880 */   659,  659,  306,  276,  278,  280,  659,  659,  290,  659,
 /*  1890 */   659,  317,  301,  305,  310,  318,  659,  329,  683,  684,
 /*  1900 */   336,  177,  179,  178,  142,  659,  645,  146,  681,  682,
 /*  1910 */   124,  425,  440,  447,  450,  145,  147,  148,  150,  151,
 /*  1920 */   494,  444,  659,   61,   64,  659,  122,  659,  659,  659,
 /*  1930 */   659,  659,  659,  659,  659,  127,  177,  179,  178,  142,
 /*  1940 */   659,  659,   46,  659,  659,  659,  659,   49,  138,   58,
 /*  1950 */   143,   55,  163,   43,  659,  153,  659,  659,  131,  275,
 /*  1960 */   676,  677,  679,  678,  680,  659,  659,  659,  170,  335,
 /*  1970 */   493,  659,  659,  152,  183,  107,  659,  659,  659,  149,
 /*  1980 */   659,  659,  453,  462,  471,  474,  465,  468,  477,  483,
 /*  1990 */   480,  489,  486,  659,  659,  315,  276,  278,  280,  659,
 /*  2000 */   659,  290,  659,  659,  317,  301,  305,  310,  318,  659,
 /*  2010 */   329,  683,  684,  336,  177,  179,  178,  142,  659,  687,
 /*  2020 */   146,  681,  682,  659,  425,  440,  447,  450,  145,  147,
 /*  2030 */   148,  150,  151,  494,  444,  659,   61,   64,  659,   71,
 /*  2040 */   659,  659,  659,  659,  659,  659,  659,  659,  127,  177,
 /*  2050 */   179,  178,  142,  659,  659,   46,  659,  583,  659,  659,
 /*  2060 */    49,  138,   58,  143,   55,  163,   43,  659,  153,  659,
 /*  2070 */   659,  131,  275,  676,  677,  679,  678,  680,  659,  659,
 /*  2080 */   659,  170,  501,  493,  659,  659,  152,  183,  107,  659,
 /*  2090 */   659,  659,  149,  659,  659,  453,  462,  471,  474,  465,
 /*  2100 */   468,  477,  483,  480,  489,  486,  659,  659,  659,  276,
 /*  2110 */   278,  280,  659,  659,  290,  659,  659,  659,  301,  305,
 /*  2120 */   310,  318,  659,  329,  683,  684,  336,  659,  659,  659,
 /*  2130 */   705,  659,  659,  146,  681,  682,  659,  425,  440,  447,
 /*  2140 */   450,  145,  147,  148,  150,  151,  494,  444,  659,   61,
 /*  2150 */    64,  659,  351,  659,  659,  659,  355,  659,  659,  659,
 /*  2160 */   659,  127,  177,  179,  178,  142,  659,  659,   46,  659,
 /*  2170 */   659,  659,  659,   49,  138,   58,  143,   55,  163,   43,
 /*  2180 */   659,  153,  659,  659,  131,  275,  676,  677,  679,  678,
 /*  2190 */   680,  659,  659,  659,  170,  509,  493,  659,  659,  152,
 /*  2200 */   183,  107,  659,  659,  659,  149,  659,  659,  453,  462,
 /*  2210 */   471,  474,  465,  468,  477,  483,  480,  489,  486,  659,
 /*  2220 */   659,  530,  276,  278,  280,  659,  659,  290,  659,  659,
 /*  2230 */   317,  301,  305,  310,  318,  659,  329,  683,  684,  336,
 /*  2240 */   177,  179,  178,  142,  659,  742,  146,  681,  682,  659,
 /*  2250 */   425,  440,  447,  450,  145,  147,  148,  150,  151,  494,
 /*  2260 */   444,  659,   61,   64,  659,  362,  659,  659,  659,  659,
 /*  2270 */   659,  659,  360,  659,  127,  177,  179,  178,  142,  659,
 /*  2280 */   659,   46,  659,  659,  659,  659,   49,  138,   58,  143,
 /*  2290 */    55,  163,   43,  659,  153,  659,  659,  131,  275,  676,
 /*  2300 */   677,  679,  678,  680,  659,  659,  659,  170,  508,  493,
 /*  2310 */   659,  659,  152,  183,  107,  659,  659,  659,  149,  659,
 /*  2320 */   659,  453,  462,  471,  474,  465,  468,  477,  483,  480,
 /*  2330 */   489,  486,  659,  659,  541,  276,  278,  280,  659,  659,
 /*  2340 */   290,  659,  659,  317,  301,  305,  310,  318,  659,  329,
 /*  2350 */   683,  684,  336,  177,  179,  178,  142,  659,  774,  146,
 /*  2360 */   681,  682,  659,  425,  440,  447,  450,  145,  147,  148,
 /*  2370 */   150,  151,  494,  444,  659,   61,   64,  659,  351,  659,
 /*  2380 */   659,  659,  359,  659,  659,  659,  659,  127,  177,  179,
 /*  2390 */   178,  142,  659,  659,   46,  659,  659,  659,  659,   49,
 /*  2400 */   138,   58,  143,   55,  163,   43,  659,  153,  659,  659,
 /*  2410 */   131,  275,  676,  677,  679,  678,  680,  659,  659,  659,
 /*  2420 */   170,  514,  493,  659,  659,  152,  183,  107,  659,  659,
 /*  2430 */   659,  149,  659,  659,  453,  462,  471,  474,  465,  468,
 /*  2440 */   477,  483,  480,  489,  486,  659,  659,  659,  276,  278,
 /*  2450 */   280,  659,  659,  290,  659,  659,   44,  301,  305,  310,
 /*  2460 */   318,  659,  329,  683,  684,  336,  177,  179,  178,  142,
 /*  2470 */   659,  659,  146,  681,  682,  659,  425,  440,  447,  450,
 /*  2480 */   145,  147,  148,  150,  151,  494,  444,  659,   61,   64,
 /*  2490 */   659,  351,  659,  659,  659,  365,  659,  659,  659,  659,
 /*  2500 */   127,  177,  179,  178,  142,  659,  659,   46,  659,  659,
 /*  2510 */   659,  659,   49,  138,   58,  143,   55,  163,   43,  430,
 /*  2520 */   153,  659,  659,  131,  275,  659,  659,  659,  429,  177,
 /*  2530 */   179,  178,  142,  170,  521,  493,  659,  659,  152,  183,
 /*  2540 */   107,  659,  659,  659,  149,  659,  659,  453,  462,  471,
 /*  2550 */   474,  465,  468,  477,  483,  480,  489,  486,  659,  659,
 /*  2560 */   659,  276,  278,  280,  659,  659,  290,  659,  659,  659,
 /*  2570 */   301,  305,  310,  318,  659,  329,  659,  672,  336,  653,
 /*  2580 */   667,  656,  660,  661,  659,  146,  659,  659,  659,  425,
 /*  2590 */   440,  447,  450,  145,  147,  148,  150,  151,  494,  444,
 /*  2600 */   659,   61,   64,  659,  351,  659,  659,  659,  369,  659,
 /*  2610 */   659,  659,  659,  127,  177,  179,  178,  142,  659,  659,
 /*  2620 */    46,  659,  659,  659,  659,   49,  138,   58,  143,   55,
 /*  2630 */   163,   43,  659,  153,  659,  659,  131,  275,  692,  659,
 /*  2640 */   653,  667,  656,  660,  661,  659,  170,  520,  493,  659,
 /*  2650 */   659,  152,  183,  107,  659,  659,  659,  149,  659,  659,
 /*  2660 */   453,  462,  471,  474,  465,  468,  477,  483,  480,  489,
 /*  2670 */   486,  659,  659,  659,  276,  278,  280,  659,  659,  290,
 /*  2680 */   659,  659,  659,  301,  305,  310,  318,  659,  329,  659,
 /*  2690 */   698,  336,  653,  667,  656,  660,  661,  659,  146,  659,
 /*  2700 */   659,  659,  425,  440,  447,  450,  145,  147,  148,  150,
 /*  2710 */   151,  494,  444,  659,   61,   64,  659,  351,  659,  659,
 /*  2720 */   659,  378,  659,  659,  154,  659,  127,  177,  179,  178,
 /*  2730 */   142,  659,  659,   46,  177,  179,  178,  142,   49,  138,
 /*  2740 */    58,  143,   55,  163,   43,  594,  153,  659,  659,  131,
 /*  2750 */   275,  752,  659,  653,  667,  656,  660,  661,  659,  170,
 /*  2760 */   545,  493,  659,  659,  152,  183,  107,  659,  659,  659,
 /*  2770 */   149,  659,  659,  453,  462,  471,  474,  465,  468,  477,
 /*  2780 */   483,  480,  489,  486,  659,  659,  659,  276,  278,  280,
 /*  2790 */   659,  659,  290,  659,  659,  659,  301,  305,  310,  318,
 /*  2800 */   659,  329,  659,  766,  336,  653,  667,  656,  660,  661,
 /*  2810 */   659,  146,  659,  659,  659,  425,  440,  447,  450,  145,
 /*  2820 */   147,  148,  150,  151,  494,  444,  659,   61,   64,  659,
 /*  2830 */   351,  659,  659,  659,  382,  659,  659,  625,  659,  127,
 /*  2840 */   177,  179,  178,  142,  659,  659,   46,  177,  179,  178,
 /*  2850 */   142,   49,  138,   58,  143,   55,  163,   43,  659,  153,
 /*  2860 */   659,  659,  131,  275,  784,  659,  653,  667,  656,  660,
 /*  2870 */   661,  659,  170,  544,  493,  659,  659,  152,  183,  107,
 /*  2880 */   659,  659,  659,  149,  659,  659,  453,  462,  471,  474,
 /*  2890 */   465,  468,  477,  483,  480,  489,  486,  659,  659,  659,
 /*  2900 */   276,  278,  280,  659,  659,  290,  659,  659,  659,  301,
 /*  2910 */   305,  310,  318,  659,  329,  659,  798,  336,  653,  667,
 /*  2920 */   656,  660,  661,  659,  146,  659,  659,  659,  425,  440,
 /*  2930 */   447,  450,  145,  147,  148,  150,  151,  494,  444,  659,
 /*  2940 */    61,   64,  659,  351,  659,  659,  659,  386,  659,  659,
 /*  2950 */    47,  659,  127,  177,  179,  178,  142,  659,  659,   46,
 /*  2960 */   177,  179,  178,  142,   49,  138,   58,  143,   55,  163,
 /*  2970 */    43,  624,  153,  659,  659,  131,  275,  659,  659,  659,
 /*  2980 */   659,  177,  179,  178,  142,  170,  550,  493,  659,  659,
 /*  2990 */   152,  183,  107,  659,  659,  659,  149,  659,  659,  453,
 /*  3000 */   462,  471,  474,  465,  468,  477,  483,  480,  489,  486,
 /*  3010 */   659,  659,  659,  276,  278,  280,  659,  659,  290,  659,
 /*  3020 */   659,   50,  301,  305,  310,  318,  659,  329,  659,  659,
 /*  3030 */   336,  177,  179,  178,  142,  659,  659,  146,  659,  659,
 /*  3040 */   659,  425,  440,  447,  450,  145,  147,  148,  150,  151,
 /*  3050 */   494,  444,  659,   61,   64,  659,  351,  659,  659,  659,
 /*  3060 */   390,  659,  659,  623,  659,  127,  177,  179,  178,  142,
 /*  3070 */   659,  659,   46,  177,  179,  178,  142,   49,  138,   58,
 /*  3080 */   143,   55,  163,   43,   53,  153,  659,  659,  131,  275,
 /*  3090 */   659,  659,  659,  659,  177,  179,  178,  142,  170,  549,
 /*  3100 */   493,  659,  659,  152,  183,  107,  659,  659,  659,  149,
 /*  3110 */   659,  659,  453,  462,  471,  474,  465,  468,  477,  483,
 /*  3120 */   480,  489,  486,  659,  659,  659,  276,  278,  280,  659,
 /*  3130 */   659,  290,  659,  659,  622,  301,  305,  310,  318,  659,
 /*  3140 */   329,  659,  659,  336,  177,  179,  178,  142,  659,  659,
 /*  3150 */   146,  659,  659,  659,  425,  440,  447,  450,  145,  147,
 /*  3160 */   148,  150,  151,  494,  444,  659,   61,   64,  659,  351,
 /*  3170 */   659,  659,  659,  393,  659,  659,   56,  659,  127,  177,
 /*  3180 */   179,  178,  142,  659,  659,   46,  177,  179,  178,  142,
 /*  3190 */    49,  138,   58,  143,   55,  163,   43,  621,  153,  659,
 /*  3200 */   659,  131,  275,  659,  659,  659,  659,  177,  179,  178,
 /*  3210 */   142,  170,  561,  493,  659,  659,  152,  183,  107,  659,
 /*  3220 */   659,  659,  149,  659,  659,  453,  462,  471,  474,  465,
 /*  3230 */   468,  477,  483,  480,  489,  486,  659,  659,  659,  276,
 /*  3240 */   278,  280,  659,  659,  290,  659,  659,   59,  301,  305,
 /*  3250 */   310,  318,  659,  329,  659,  659,  336,  177,  179,  178,
 /*  3260 */   142,  659,  659,  146,  659,  659,  659,  425,  440,  447,
 /*  3270 */   450,  145,  147,  148,  150,  151,  494,  444,  659,   61,
 /*  3280 */    64,  659,  351,  659,  659,  659,  397,  659,  659,  620,
 /*  3290 */   659,  127,  177,  179,  178,  142,  659,  659,   46,  177,
 /*  3300 */   179,  178,  142,   49,  138,   58,  143,   55,  163,   43,
 /*  3310 */    62,  153,  659,  659,  131,  275,  659,  659,  659,  659,
 /*  3320 */   177,  179,  178,  142,  170,  560,  493,  659,  659,  152,
 /*  3330 */   183,  107,  659,  659,  659,  149,  659,  659,  453,  462,
 /*  3340 */   471,  474,  465,  468,  477,  483,  480,  489,  486,  659,
 /*  3350 */   659,  659,  276,  278,  280,  659,  659,  290,  659,  659,
 /*  3360 */   619,  301,  305,  310,  318,  659,  329,  659,  659,  336,
 /*  3370 */   177,  179,  178,  142,  659,  659,  146,  659,  659,  659,
 /*  3380 */   425,  440,  447,  450,  145,  147,  148,  150,  151,  494,
 /*  3390 */   444,  659,   61,   64,  659,  351,  659,  659,  659,  401,
 /*  3400 */   659,  659,   65,  659,  127,  177,  179,  178,  142,  659,
 /*  3410 */   659,   46,  177,  179,  178,  142,   49,  138,   58,  143,
 /*  3420 */    55,  163,   43,   67,  153,  659,  659,  131,  275,  659,
 /*  3430 */   659,  659,  659,  177,  179,  178,  142,  170,  569,  493,
 /*  3440 */   659,  659,  152,  183,  107,  659,  659,  659,  149,  659,
 /*  3450 */   659,  453,  462,  471,  474,  465,  468,  477,  483,  480,
 /*  3460 */   489,  486,  659,  659,  659,  276,  278,  280,  659,  659,
 /*  3470 */   290,  659,  659,   73,  301,  305,  310,  318,  659,  329,
 /*  3480 */   659,  659,  336,  177,  179,  178,  142,  659,  659,  146,
 /*  3490 */   659,  659,  659,  425,  440,  447,  450,  145,  147,  148,
 /*  3500 */   150,  151,  494,  444,  659,   61,   64,  659,  351,  659,
 /*  3510 */   659,  659,  404,  659,  659,   76,  659,  127,  177,  179,
 /*  3520 */   178,  142,  659,  659,   46,  177,  179,  178,  142,   49,
 /*  3530 */   138,   58,  143,   55,  163,   43,   78,  153,  659,  659,
 /*  3540 */   131,  275,  659,  659,  659,  659,  177,  179,  178,  142,
 /*  3550 */   170,  568,  493,  659,  659,  152,  183,  107,  659,  659,
 /*  3560 */   659,  149,  659,  659,  453,  462,  471,  474,  465,  468,
 /*  3570 */   477,  483,  480,  489,  486,  659,  659,  659,  276,  278,
 /*  3580 */   280,  659,  659,  290,  659,  659,   80,  301,  305,  310,
 /*  3590 */   318,  659,  329,  659,  659,  336,  177,  179,  178,  142,
 /*  3600 */   659,  659,  146,  659,  659,  659,  425,  440,  447,  450,
 /*  3610 */   145,  147,  148,  150,  151,  494,  444,  659,   61,   64,
 /*  3620 */   659,  351,  659,  659,  659,  408,  659,  659,   82,  659,
 /*  3630 */   127,  177,  179,  178,  142,  659,  659,   46,  177,  179,
 /*  3640 */   178,  142,   49,  138,   58,  143,   55,  163,   43,   84,
 /*  3650 */   153,  659,  659,  131,  275,  659,  659,  659,  659,  177,
 /*  3660 */   179,  178,  142,  170,  747,  493,  659,  659,  152,  183,
 /*  3670 */   107,  659,  659,  659,  149,  659,  659,  453,  462,  471,
 /*  3680 */   474,  465,  468,  477,  483,  480,  489,  486,  659,  659,
 /*  3690 */   659,  276,  278,  280,  659,  659,  290,  659,  659,   86,
 /*  3700 */   301,  305,  310,  318,  659,  329,  659,  659,  336,  177,
 /*  3710 */   179,  178,  142,  659,  659,  146,  659,  659,  659,  425,
 /*  3720 */   440,  447,  450,  145,  147,  148,  150,  151,  494,  444,
 /*  3730 */   659,   61,   64,  659,  351,  659,  659,  659,  419,  659,
 /*  3740 */   659,   88,  659,  127,  177,  179,  178,  142,  659,  659,
 /*  3750 */    46,  177,  179,  178,  142,   49,  138,   58,  143,   55,
 /*  3760 */   163,   43,   90,  153,  659,  659,  131,  275,  659,  659,
 /*  3770 */   659,  659,  177,  179,  178,  142,  170,  749,  493,  659,
 /*  3780 */   659,  152,  183,  107,  659,  659,  659,  149,  659,  659,
 /*  3790 */   453,  462,  471,  474,  465,  468,  477,  483,  480,  489,
 /*  3800 */   486,  659,  659,  659,  276,  278,  280,  659,  659,  290,
 /*  3810 */   659,  659,   92,  301,  305,  310,  318,  659,  329,  659,
 /*  3820 */   659,  336,  177,  179,  178,  142,  659,  659,  146,  659,
 /*  3830 */   659,  659,  425,  440,  447,  450,  145,  147,  148,  150,
 /*  3840 */   151,  494,  444,  659,   61,   64,  659,  351,  659,  659,
 /*  3850 */   659,  423,  659,  659,   94,  659,  127,  177,  179,  178,
 /*  3860 */   142,  659,  659,   46,  177,  179,  178,  142,   49,  138,
 /*  3870 */    58,  143,   55,  163,   43,   96,  153,  659,  659,  131,
 /*  3880 */   275,  659,  659,  659,  659,  177,  179,  178,  142,  170,
 /*  3890 */   754,  493,  659,  659,  152,  183,  107,  659,  659,  659,
 /*  3900 */   149,  659,  659,  453,  462,  471,  474,  465,  468,  477,
 /*  3910 */   483,  480,  489,  486,  659,  659,  659,  276,  278,  280,
 /*  3920 */   659,  659,  290,  659,  659,   98,  301,  305,  310,  318,
 /*  3930 */   659,  329,  659,  659,  336,  177,  179,  178,  142,  659,
 /*  3940 */   659,  146,  659,  659,  659,  425,  440,  447,  450,  145,
 /*  3950 */   147,  148,  150,  151,  494,  444,  659,   61,   64,  659,
 /*  3960 */   100,  659,  659,  659,  659,  659,  659,  102,  659,  127,
 /*  3970 */   177,  179,  178,  142,  659,  659,   46,  177,  179,  178,
 /*  3980 */   142,   49,  138,   58,  143,   55,  163,   43,  104,  153,
 /*  3990 */   659,  659,  131,  275,  659,  659,  659,  659,  177,  179,
 /*  4000 */   178,  142,  170,  756,  493,  659,  659,  152,  183,  107,
 /*  4010 */   659,  659,  659,  149,  659,  659,  453,  462,  471,  474,
 /*  4020 */   465,  468,  477,  483,  480,  489,  486,  659,  659,  659,
 /*  4030 */   276,  278,  280,  659,  659,  290,  659,  659,  106,  301,
 /*  4040 */   305,  310,  318,  659,  329,  659,  659,  336,  177,  179,
 /*  4050 */   178,  142,  659,  659,  146,  659,  659,  659,  425,  440,
 /*  4060 */   447,  450,  145,  147,  148,  150,  151,  494,  444,  659,
 /*  4070 */    61,   64,  659,  126,  659,  659,  659,  659,  659,  659,
 /*  4080 */   130,  659,  127,  177,  179,  178,  142,  659,  659,   46,
 /*  4090 */   177,  179,  178,  142,   49,  138,   58,  143,   55,  163,
 /*  4100 */    43,  137,  153,  659,  659,  131,  275,  659,  659,  659,
 /*  4110 */   659,  177,  179,  178,  142,  170,  761,  493,  659,  659,
 /*  4120 */   152,  183,  107,  659,  659,  659,  149,  659,  659,  453,
 /*  4130 */   462,  471,  474,  465,  468,  477,  483,  480,  489,  486,
 /*  4140 */   659,  659,  659,  276,  278,  280,  659,  659,  290,  659,
 /*  4150 */   659,  141,  301,  305,  310,  318,  659,  329,  659,  659,
 /*  4160 */   336,  177,  179,  178,  142,  659,  659,  146,  659,  659,
 /*  4170 */   659,  425,  440,  447,  450,  145,  147,  148,  150,  151,
 /*  4180 */   494,  444,  659,   61,   64,  659,  144,  659,  659,  659,
 /*  4190 */   659,  659,  659,  182,  659,  127,  177,  179,  178,  142,
 /*  4200 */   659,  659,   46,  177,  179,  178,  142,   49,  138,   58,
 /*  4210 */   143,   55,  163,   43,  332,  153,  659,  659,  131,  275,
 /*  4220 */   659,  659,  659,  659,  177,  179,  178,  142,  170,  763,
 /*  4230 */   493,  659,  659,  152,  183,  107,  659,  659,  659,  149,
 /*  4240 */   659,  659,  453,  462,  471,  474,  465,  468,  477,  483,
 /*  4250 */   480,  489,  486,  659,  659,  659,  276,  278,  280,  659,
 /*  4260 */   659,  290,  659,  659,  441,  301,  305,  310,  318,  659,
 /*  4270 */   329,  659,  659,  336,  177,  179,  178,  142,  659,  659,
 /*  4280 */   146,  659,  659,  659,  425,  440,  447,  450,  145,  147,
 /*  4290 */   148,  150,  151,  494,  444,  659,   61,   64,  659,  445,
 /*  4300 */   659,  659,  659,  659,  659,  659,  448,  659,  127,  177,
 /*  4310 */   179,  178,  142,  659,  659,   46,  177,  179,  178,  142,
 /*  4320 */    49,  138,   58,  143,   55,  163,   43,  451,  153,  659,
 /*  4330 */   659,  131,  275,  659,  659,  659,  659,  177,  179,  178,
 /*  4340 */   142,  170,  768,  493,  659,  659,  152,  183,  107,  659,
 /*  4350 */   659,  659,  149,  659,  659,  453,  462,  471,  474,  465,
 /*  4360 */   468,  477,  483,  480,  489,  486,  659,  659,  659,  276,
 /*  4370 */   278,  280,  659,  659,  290,  659,  659,  498,  301,  305,
 /*  4380 */   310,  318,  659,  329,  659,  659,  336,  177,  179,  178,
 /*  4390 */   142,  659,  659,  146,  659,  659,  659,  425,  440,  447,
 /*  4400 */   450,  145,  147,  148,  150,  151,  494,  444,  659,   61,
 /*  4410 */    64,  659,  505,  659,  659,  659,  659,  659,  659,  511,
 /*  4420 */   659,  127,  177,  179,  178,  142,  659,  659,   46,  177,
 /*  4430 */   179,  178,  142,   49,  138,   58,  143,   55,  163,   43,
 /*  4440 */   598,  153,  659,  659,  131,  275,  659,  659,  659,  659,
 /*  4450 */   177,  179,  178,  142,  170,  770,  493,  659,  659,  152,
 /*  4460 */   183,  107,  659,  659,  659,  149,  659,  659,  453,  462,
 /*  4470 */   471,  474,  465,  468,  477,  483,  480,  489,  486,  659,
 /*  4480 */   659,  659,  276,  278,  280,  659,  659,  290,  659,  659,
 /*  4490 */   881,  301,  305,  310,  318,  659,  329,  659,  659,  336,
 /*  4500 */   177,  179,  178,  142,  659,  659,  146,  659,  659,  659,
 /*  4510 */   425,  440,  447,  450,  145,  147,  148,  150,  151,  494,
 /*  4520 */   444,  805,   61,   64,  741,  659,  659,  659,  659,  702,
 /*  4530 */   806,  659,  659,  659,  127,  701,  659,  659,  659,  659,
 /*  4540 */   659,   46,  659,  659,  659,  659,   49,  138,   58,  143,
 /*  4550 */    55,  163,   43,  659,  153,  659,  659,  131,  275,  659,
 /*  4560 */   659,  659,  659,  659,  659,  659,  659,  170,  779,  493,
 /*  4570 */   659,  659,  152,  183,  107,  659,  659,  659,  149,  659,
 /*  4580 */   659,  453,  462,  471,  474,  465,  468,  477,  483,  480,
 /*  4590 */   489,  486,  659,  659,  659,  276,  278,  280,  659,  659,
 /*  4600 */   290,  659,  659,  659,  301,  305,  310,  318,  659,  329,
 /*  4610 */   659,  659,  336,  659,  659,  659,  659,  659,  659,  146,
 /*  4620 */   659,  659,  659,  425,  440,  447,  450,  145,  147,  148,
 /*  4630 */   150,  151,  494,  444,  659,   61,   64,  659,  659,  659,
 /*  4640 */   659,  659,  659,  659,  659,  659,  659,  127,  659,  659,
 /*  4650 */   659,  659,  659,  659,   46,  659,  659,  659,  659,   49,
 /*  4660 */   138,   58,  143,   55,  163,   43,  659,  153,  659,  659,
 /*  4670 */   131,  275,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  4680 */   170,  781,  493,  659,  659,  152,  183,  107,  659,  659,
 /*  4690 */   659,  149,  659,  659,  453,  462,  471,  474,  465,  468,
 /*  4700 */   477,  483,  480,  489,  486,  659,  659,  659,  276,  278,
 /*  4710 */   280,  659,  659,  290,  659,  659,  659,  301,  305,  310,
 /*  4720 */   318,  659,  329,  659,  659,  336,  659,  659,  659,  659,
 /*  4730 */   659,  659,  146,  659,  659,  659,  425,  440,  447,  450,
 /*  4740 */   145,  147,  148,  150,  151,  494,  444,  659,   61,   64,
 /*  4750 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  4760 */   127,  659,  659,  659,  659,  659,  659,   46,  659,  659,
 /*  4770 */   659,  659,   49,  138,   58,  143,   55,  163,   43,  659,
 /*  4780 */   153,  659,  659,  131,  275,  659,  659,  659,  659,  659,
 /*  4790 */   659,  659,  659,  170,  786,  493,  659,  659,  152,  183,
 /*  4800 */   107,  659,  659,  659,  149,  659,  659,  453,  462,  471,
 /*  4810 */   474,  465,  468,  477,  483,  480,  489,  486,  659,  659,
 /*  4820 */   659,  276,  278,  280,  659,  659,  290,  659,  659,  659,
 /*  4830 */   301,  305,  310,  318,  659,  329,  659,  659,  336,  659,
 /*  4840 */   659,  659,  659,  659,  659,  146,  659,  659,  659,  425,
 /*  4850 */   440,  447,  450,  145,  147,  148,  150,  151,  494,  444,
 /*  4860 */   659,   61,   64,  659,  659,  659,  659,  659,  659,  659,
 /*  4870 */   659,  659,  659,  127,  659,  659,  659,  659,  659,  659,
 /*  4880 */    46,  659,  659,  659,  659,   49,  138,   58,  143,   55,
 /*  4890 */   163,   43,  659,  153,  659,  659,  131,  275,  659,  659,
 /*  4900 */   659,  659,  659,  659,  659,  659,  170,  788,  493,  659,
 /*  4910 */   659,  152,  183,  107,  659,  659,  659,  149,  659,  659,
 /*  4920 */   453,  462,  471,  474,  465,  468,  477,  483,  480,  489,
 /*  4930 */   486,  659,  659,  659,  276,  278,  280,  659,  659,  290,
 /*  4940 */   659,  659,  659,  301,  305,  310,  318,  659,  329,  659,
 /*  4950 */   659,  336,  659,  659,  659,  659,  659,  659,  146,  659,
 /*  4960 */   659,  659,  425,  440,  447,  450,  145,  147,  148,  150,
 /*  4970 */   151,  494,  444,  659,   61,   64,  659,  659,  659,  659,
 /*  4980 */   659,  659,  659,  659,  659,  659,  127,  659,  659,  659,
 /*  4990 */   659,  659,  659,   46,  659,  659,  659,  659,   49,  138,
 /*  5000 */    58,  143,   55,  163,   43,  659,  153,  659,  659,  131,
 /*  5010 */   275,  659,  659,  659,  659,  659,  659,  659,  659,  170,
 /*  5020 */   793,  493,  659,  659,  152,  183,  107,  659,  659,  659,
 /*  5030 */   149,  659,  659,  453,  462,  471,  474,  465,  468,  477,
 /*  5040 */   483,  480,  489,  486,  659,  659,  659,  276,  278,  280,
 /*  5050 */   659,  659,  290,  659,  659,  659,  301,  305,  310,  318,
 /*  5060 */   659,  329,  659,  659,  336,  659,  659,  659,  659,  659,
 /*  5070 */   659,  146,  659,  659,  659,  425,  440,  447,  450,  145,
 /*  5080 */   147,  148,  150,  151,  494,  444,  659,   61,   64,  659,
 /*  5090 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  127,
 /*  5100 */   659,  659,  659,  659,  659,  659,   46,  659,  659,  659,
 /*  5110 */   659,   49,  138,   58,  143,   55,  163,   43,  659,  153,
 /*  5120 */   659,  659,  131,  275,  659,  659,  659,  659,  659,  659,
 /*  5130 */   659,  659,  170,  795,  493,  659,  659,  152,  183,  107,
 /*  5140 */   659,  659,  659,  149,  659,  659,  453,  462,  471,  474,
 /*  5150 */   465,  468,  477,  483,  480,  489,  486,  659,  659,  659,
 /*  5160 */   276,  278,  280,  659,  659,  290,  659,  659,  659,  301,
 /*  5170 */   305,  310,  318,  659,  329,  659,  659,  336,  659,  659,
 /*  5180 */   659,  659,  659,  659,  146,  659,  659,  659,  425,  440,
 /*  5190 */   447,  450,  145,  147,  148,  150,  151,  494,  444,  659,
 /*  5200 */    61,   64,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  5210 */   659,  659,  127,  659,  659,  659,  659,  659,  659,   46,
 /*  5220 */   659,  659,  659,  659,   49,  138,   58,  143,   55,  163,
 /*  5230 */    43,  659,  153,  659,  659,  131,  275,  659,  659,  659,
 /*  5240 */   659,  659,  659,  659,  659,  170,  800,  493,  659,  659,
 /*  5250 */   152,  183,  107,  659,  659,  659,  149,  659,  659,  453,
 /*  5260 */   462,  471,  474,  465,  468,  477,  483,  480,  489,  486,
 /*  5270 */   659,  659,  659,  276,  278,  280,  659,  659,  290,  659,
 /*  5280 */   659,  659,  301,  305,  310,  318,  659,  329,  659,  659,
 /*  5290 */   336,  659,  659,  659,  659,  659,  659,  146,  659,  659,
 /*  5300 */   659,  425,  440,  447,  450,  145,  147,  148,  150,  151,
 /*  5310 */   494,  444,  659,   61,   64,  659,  659,  659,  659,  659,
 /*  5320 */   659,  659,  659,  659,  659,  127,  659,  659,  659,  659,
 /*  5330 */   659,  659,   46,  659,  659,  659,  659,   49,  138,   58,
 /*  5340 */   143,   55,  163,   43,  659,  153,  659,  659,  131,  275,
 /*  5350 */   659,  659,  659,  659,  659,  659,  659,  659,  170,  802,
 /*  5360 */   493,  659,  659,  152,  183,  107,  659,  659,  659,  149,
 /*  5370 */   659,  659,  453,  462,  471,  474,  465,  468,  477,  483,
 /*  5380 */   480,  489,  486,  659,  659,  659,  276,  278,  280,  659,
 /*  5390 */   659,  290,  659,  659,  659,  301,  305,  310,  318,  659,
 /*  5400 */   329,  659,  659,  336,  659,  659,  659,  659,  659,  659,
 /*  5410 */   146,  659,  659,  659,  425,  440,  447,  450,  145,  147,
 /*  5420 */   148,  150,  151,  494,  444,  659,   61,   64,  659,  659,
 /*  5430 */   659,  659,  659,  659,  659,  659,  659,  659,  127,  659,
 /*  5440 */   659,  659,  659,  659,  659,   46,  659,  659,  659,  659,
 /*  5450 */    49,  138,   58,  143,   55,  163,   43,  659,  153,  659,
 /*  5460 */   659,  131,  275,  659,  659,  659,  659,  659,  659,  659,
 /*  5470 */   659,  170,  659,  493,  659,  659,  152,  183,  107,  659,
 /*  5480 */   659,  659,  149,  659,  659,  453,  462,  471,  474,  465,
 /*  5490 */   468,  477,  483,  480,  489,  486,  659,  659,  659,  276,
 /*  5500 */   278,  280,  659,  659,  290,  659,  659,  659,  301,  305,
 /*  5510 */   310,  318,  659,  329,  659,  659,  336,  659,  659,  659,
 /*  5520 */   659,  659,  659,  146,  659,  659,  659,  425,  440,  447,
 /*  5530 */   450,  145,  147,  148,  150,  151,  494,   52,  659,   61,
 /*  5540 */    64,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  5550 */   659,  127,  659,  659,  659,  659,  659,  659,   46,  659,
 /*  5560 */   659,  659,  659,   49,  138,   58,  143,   55,  163,   43,
 /*  5570 */   659,  153,  659,  659,  131,  659,  659,  659,  659,  659,
 /*  5580 */   659,  659,  659,  659,  170,  884,  659,  659,  659,  152,
 /*  5590 */   183,  107,  880,  659,  659,  149,  659,  659,  108,  109,
 /*  5600 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  5610 */   120,  121,  659,  659,  659,  659,  659,  659,  659,  138,
 /*  5620 */   659,  659,  659,  659,  659,  659,  659,  659,   29,   41,
 /*  5630 */   659,   32,  659,  631,  659,  725,  146,  854,  867,  170,
 /*  5640 */   659,  493,  659,  659,  145,  147,  148,  150,  151,  659,
 /*  5650 */   659,  659,  659,  453,  462,  471,  474,  465,  468,  477,
 /*  5660 */   483,  480,  489,  486,  880,  659,  659,  276,  278,  280,
 /*  5670 */   659,  659,  290,  659,  659,  659,  301,  305,  310,  318,
 /*  5680 */   659,  329,  659,  659,  336,  659,  659,  659,  659,  659,
 /*  5690 */   659,  138,  659,  659,  659,  425,  440,  447,  450,  366,
 /*  5700 */    29,   41,  659,   32,  494,  631,  659,  725,  659,  854,
 /*  5710 */   867,  170,  659,  493,  659,  342,  659,  659,  659,  659,
 /*  5720 */   659,  659,  659,  659,  659,  453,  462,  471,  474,  465,
 /*  5730 */   468,  477,  483,  480,  489,  486,  659,  659,  659,  276,
 /*  5740 */   278,  280,  659,  659,  290,  659,  659,  659,  301,  305,
 /*  5750 */   310,  318,  659,  329,  659,  659,  336,  659,  343,  344,
 /*  5760 */   345,  346,  347,  348,  659,  659,  659,  425,  440,  447,
 /*  5770 */   450,  659,  659,  659,  659,  659,  494, 1323,    1,    2,
 /*  5780 */   882,    4,    5,    6,    7,    8,    9,   10,   11,   12,
 /*  5790 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*  5800 */    23,   24,   25,   26,   27,   28,  659,  659,  659,  659,
 /*  5810 */   659,  273,  252,  253,  254,  255,  256,  257,  258,  259,
 /*  5820 */   260,  262,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  5830 */   271,  272,   45,   51,   57,   60,   63,   66,   54,   48,
 /*  5840 */    75,   77,   85,   79,   81,   83,  659,  659,  659,  659,
 /*  5850 */   659,  659,  659,  250,   72,   68,  154,  659,  659,  877,
 /*  5860 */   878,  879,  495,  261,  274,  659,  177,  179,  178,  142,
 /*  5870 */   154,  659,  160,  659,  492,  596,  592,  595,  659,  659,
 /*  5880 */   177,  179,  178,  142,  432,  434,  436,  438,  659,  591,
 /*  5890 */   592,  595,  659,  659,  659,  659,  659,  273,  252,  253,
 /*  5900 */   254,  255,  256,  257,  258,  259,  260,  262,  263,  264,
 /*  5910 */   265,  266,  267,  268,  269,  270,  271,  272,   87,   93,
 /*  5920 */    91,   95,   97,   99,   45,   51,   57,   60,   63,   66,
 /*  5930 */    54,   48,   75,   77,   85,   79,   81,   83,  659,  283,
 /*  5940 */   659,  659,  659,  659,  659,  659,   72,   68,  495,  261,
 /*  5950 */   274,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  5960 */   492,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  5970 */   432,  434,  436,  438,  659,  659,  659,  659,  659,  659,
 /*  5980 */   659,  659,  659,  273,  252,  253,  254,  255,  256,  257,
 /*  5990 */   258,  259,  260,  262,  263,  264,  265,  266,  267,  268,
 /*  6000 */   269,  270,  271,  272,   93,   91,   95,   97,   99,   45,
 /*  6010 */    51,   57,   60,   63,   66,   54,   48,   75,   77,   85,
 /*  6020 */    79,   81,   83,  659,  659,  288,  659,  659,  659,  659,
 /*  6030 */   659,   72,   68,  659,  495,  261,  274,  659,  659,  659,
 /*  6040 */   659,  659,  659,  233,  659,  356,  492,  659,  659,  659,
 /*  6050 */   659,  659,  659,  659,  659,  659,  432,  434,  436,  438,
 /*  6060 */   659,  342,  659,  659,  659,  659,  218,  659,  659,  273,
 /*  6070 */   252,  253,  254,  255,  256,  257,  258,  259,  260,  262,
 /*  6080 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  6090 */   108,  109,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  6100 */   118,  119,  120,  121,  343,  344,  345,  346,  347,  348,
 /*  6110 */   659,  300,  372,  373,  659,  659,  659,  398,  352,  659,
 /*  6120 */   495,  261,  274,  659,  659,  659,  659,  659,  202,  659,
 /*  6130 */   659,  205,  492,  342,  659,  659,  659,  659,  659,  659,
 /*  6140 */   659,  659,  432,  434,  436,  438,  201,  659,  659,  659,
 /*  6150 */   659,  659,  195,  659,  206,  273,  252,  253,  254,  255,
 /*  6160 */   256,  257,  258,  259,  260,  262,  263,  264,  265,  266,
 /*  6170 */   267,  268,  269,  270,  271,  272,  343,  344,  345,  346,
 /*  6180 */   347,  348,  659,  383,  409,  410,  659,  659,  659,  659,
 /*  6190 */   659,  659,  659,  204,  659,  659,  659,  303,  659,  659,
 /*  6200 */   659,  203,  192,  194,  197,  196,  495,  261,  274,  387,
 /*  6210 */   659,  659,  659,  659,  202,  659,  659,  198,  492,  659,
 /*  6220 */   659,  659,  659,  659,  659,  342,  659,  659,  432,  434,
 /*  6230 */   436,  438,  201,  659,  659,  659,  659,  659,  195,  659,
 /*  6240 */   659,  273,  252,  253,  254,  255,  256,  257,  258,  259,
 /*  6250 */   260,  262,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  6260 */   271,  272,  659,  659,  659,  659,  659,  659,  343,  344,
 /*  6270 */   345,  346,  347,  348,  659,  659,  659,  659,  659,  193,
 /*  6280 */   659,  659,  659,  308,  659,  659,  659,  191,  192,  194,
 /*  6290 */   197,  196,  495,  261,  274,  394,  659,  659,  659,  659,
 /*  6300 */   202,  659,  659,  205,  492,  659,  659,  659,  659,  659,
 /*  6310 */   659,  342,  659,  659,  432,  434,  436,  438,  201,  659,
 /*  6320 */   659,  659,  659,  659,  195,  659,  659,  273,  252,  253,
 /*  6330 */   254,  255,  256,  257,  258,  259,  260,  262,  263,  264,
 /*  6340 */   265,  266,  267,  268,  269,  270,  271,  272,  659,  659,
 /*  6350 */   659,  659,  659,  659,  343,  344,  345,  346,  347,  348,
 /*  6360 */   659,  659,  659,  659,  659,  204,  659,  659,  659,  312,
 /*  6370 */   659,  659,  659,  203,  192,  194,  197,  196,  495,  261,
 /*  6380 */   274,  405,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6390 */   492,  659,  659,  659,  659,  659,  659,  342,  659,  659,
 /*  6400 */   432,  434,  436,  438,  659,  659,  659,  659,  659,  659,
 /*  6410 */   659,  659,  659,  273,  252,  253,  254,  255,  256,  257,
 /*  6420 */   258,  259,  260,  262,  263,  264,  265,  266,  267,  268,
 /*  6430 */   269,  270,  271,  272,  659,  659,  659,  659,  659,  659,
 /*  6440 */   343,  344,  345,  346,  347,  348,  659,  659,  659,  659,
 /*  6450 */   659,  659,  659,  659,  659,  320,  659,  659,  659,  659,
 /*  6460 */   659,  659,  659,  659,  495,  261,  274,  659,  659,  659,
 /*  6470 */   659,  659,  659,  659,  659,  659,  492,  659,  659,  659,
 /*  6480 */   659,  659,  659,  659,  659,  659,  432,  434,  436,  438,
 /*  6490 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  273,
 /*  6500 */   252,  253,  254,  255,  256,  257,  258,  259,  260,  262,
 /*  6510 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  6520 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6530 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6540 */   659,  327,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6550 */   495,  261,  274,  659,  659,  659,  659,  659,  659,  659,
 /*  6560 */   659,  659,  492,  659,  659,  659,  659,  659,  659,  659,
 /*  6570 */   659,  659,  432,  434,  436,  438,  659,  659,  659,  659,
 /*  6580 */   659,  659,  659,  659,  659,  273,  252,  253,  254,  255,
 /*  6590 */   256,  257,  258,  259,  260,  262,  263,  264,  265,  266,
 /*  6600 */   267,  268,  269,  270,  271,  272,  659,  659,  659,  659,
 /*  6610 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6620 */   659,  659,  659,  659,  659,  659,  659,  334,  659,  659,
 /*  6630 */   659,  659,  659,  659,  659,  659,  495,  261,  274,  659,
 /*  6640 */   659,  659,  659,  659,  659,  659,  659,  659,  492,  659,
 /*  6650 */   659,  659,  659,  659,  659,  659,  659,  659,  432,  434,
 /*  6660 */   436,  438,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6670 */   659,  273,  252,  253,  254,  255,  256,  257,  258,  259,
 /*  6680 */   260,  262,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  6690 */   271,  272,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6700 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6710 */   659,  659,  659,  500,  659,  659,  659,  659,  659,  659,
 /*  6720 */   659,  659,  495,  261,  274,  659,  659,  659,  659,  659,
 /*  6730 */   659,  659,  659,  659,  492,  659,  659,  659,  659,  659,
 /*  6740 */   659,  659,  659,  659,  432,  434,  436,  438,  659,  659,
 /*  6750 */   659,  659,  659,  659,  659,  659,  659,  273,  252,  253,
 /*  6760 */   254,  255,  256,  257,  258,  259,  260,  262,  263,  264,
 /*  6770 */   265,  266,  267,  268,  269,  270,  271,  272,  659,  659,
 /*  6780 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6790 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  507,
 /*  6800 */   659,  659,  659,  659,  659,  659,  659,  659,  495,  261,
 /*  6810 */   274,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6820 */   492,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6830 */   432,  434,  436,  438,  659,  659,  659,  659,  659,  659,
 /*  6840 */   659,  659,  659,  273,  252,  253,  254,  255,  256,  257,
 /*  6850 */   258,  259,  260,  262,  263,  264,  265,  266,  267,  268,
 /*  6860 */   269,  270,  271,  272,  659,  659,  659,  659,  659,  659,
 /*  6870 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6880 */   659,  659,  659,  659,  659,  513,  659,  659,  659,  659,
 /*  6890 */   659,  659,  659,  659,  495,  261,  274,  659,  659,  659,
 /*  6900 */   659,  659,  659,  659,  659,  659,  492,  659,  659,  659,
 /*  6910 */   659,  659,  659,  659,  659,  659,  432,  434,  436,  438,
 /*  6920 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  273,
 /*  6930 */   252,  253,  254,  255,  256,  257,  258,  259,  260,  262,
 /*  6940 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  6950 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6960 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6970 */   659,  519,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  6980 */   495,  261,  274,  659,  659,  659,  659,  659,  659,  659,
 /*  6990 */   659,  659,  492,  659,  659,  659,  659,  659,  659,  659,
 /*  7000 */   659,  659,  432,  434,  436,  438,  659,  659,  659,  659,
 /*  7010 */   659,  659,  659,  659,  659,  273,  252,  253,  254,  255,
 /*  7020 */   256,  257,  258,  259,  260,  262,  263,  264,  265,  266,
 /*  7030 */   267,  268,  269,  270,  271,  272,  659,  659,  659,  659,
 /*  7040 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7050 */   659,  659,  659,  659,  659,  659,  659,  536,  659,  659,
 /*  7060 */   659,  659,  659,  659,  659,  659,  495,  261,  274,  659,
 /*  7070 */   659,  659,  659,  659,  659,  659,  659,  659,  492,  659,
 /*  7080 */   659,  659,  659,  659,  659,  659,  659,  659,  432,  434,
 /*  7090 */   436,  438,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7100 */   659,  273,  252,  253,  254,  255,  256,  257,  258,  259,
 /*  7110 */   260,  262,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  7120 */   271,  272,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7130 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7140 */   659,  659,  659,  543,  659,  659,  659,  659,  659,  659,
 /*  7150 */   659,  659,  495,  261,  274,  659,  659,  659,  659,  659,
 /*  7160 */   659,  659,  659,  659,  492,  659,  659,  659,  659,  659,
 /*  7170 */   659,  659,  659,  659,  432,  434,  436,  438,  659,  659,
 /*  7180 */   659,  659,  659,  659,  659,  659,  659,  273,  252,  253,
 /*  7190 */   254,  255,  256,  257,  258,  259,  260,  262,  263,  264,
 /*  7200 */   265,  266,  267,  268,  269,  270,  271,  272,  659,  659,
 /*  7210 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7220 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  548,
 /*  7230 */   659,  659,  659,  659,  659,  659,  659,  659,  495,  261,
 /*  7240 */   274,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7250 */   492,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7260 */   432,  434,  436,  438,  659,  659,  659,  659,  659,  659,
 /*  7270 */   659,  659,  659,  273,  252,  253,  254,  255,  256,  257,
 /*  7280 */   258,  259,  260,  262,  263,  264,  265,  266,  267,  268,
 /*  7290 */   269,  270,  271,  272,  659,  659,  659,  659,  659,  659,
 /*  7300 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7310 */   659,  659,  659,  659,  659,  559,  659,  659,  659,  659,
 /*  7320 */   659,  659,  659,  659,  495,  261,  274,  659,  659,  659,
 /*  7330 */   659,  659,  659,  659,  659,  659,  492,  659,  659,  659,
 /*  7340 */   659,  659,  659,  659,  659,  659,  432,  434,  436,  438,
 /*  7350 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  273,
 /*  7360 */   252,  253,  254,  255,  256,  257,  258,  259,  260,  262,
 /*  7370 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  7380 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7390 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7400 */   659,  567,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7410 */   495,  261,  274,  659,  659,  659,  659,  659,  659,  659,
 /*  7420 */   659,  659,  492,  659,  659,  659,  659,  659,  659,  659,
 /*  7430 */   659,  659,  432,  434,  436,  438,  659,  659,  659,  659,
 /*  7440 */   659,  659,  659,  659,  659,  273,  252,  253,  254,  255,
 /*  7450 */   256,  257,  258,  259,  260,  262,  263,  264,  265,  266,
 /*  7460 */   267,  268,  269,  270,  271,  272,  659,  659,  659,  659,
 /*  7470 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7480 */   659,  659,  659,  659,  659,  659,  659,  748,  659,  659,
 /*  7490 */   659,  659,  659,  659,  659,  659,  495,  261,  274,  659,
 /*  7500 */   659,  659,  659,  659,  659,  659,  659,  659,  492,  659,
 /*  7510 */   659,  659,  659,  659,  659,  659,  659,  659,  432,  434,
 /*  7520 */   436,  438,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7530 */   659,  273,  252,  253,  254,  255,  256,  257,  258,  259,
 /*  7540 */   260,  262,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  7550 */   271,  272,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7560 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7570 */   659,  659,  659,  755,  659,  659,  659,  659,  659,  659,
 /*  7580 */   659,  659,  495,  261,  274,  659,  659,  659,  659,  659,
 /*  7590 */   659,  659,  659,  659,  492,  659,  659,  659,  659,  659,
 /*  7600 */   659,  659,  659,  659,  432,  434,  436,  438,  659,  659,
 /*  7610 */   659,  659,  659,  659,  659,  659,  659,  273,  252,  253,
 /*  7620 */   254,  255,  256,  257,  258,  259,  260,  262,  263,  264,
 /*  7630 */   265,  266,  267,  268,  269,  270,  271,  272,  659,  659,
 /*  7640 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7650 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  762,
 /*  7660 */   659,  659,  659,  659,  659,  659,  659,  659,  495,  261,
 /*  7670 */   274,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7680 */   492,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7690 */   432,  434,  436,  438,  659,  659,  659,  659,  659,  659,
 /*  7700 */   659,  659,  659,  273,  252,  253,  254,  255,  256,  257,
 /*  7710 */   258,  259,  260,  262,  263,  264,  265,  266,  267,  268,
 /*  7720 */   269,  270,  271,  272,  659,  659,  659,  659,  659,  659,
 /*  7730 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7740 */   659,  659,  659,  659,  659,  769,  659,  659,  659,  659,
 /*  7750 */   659,  659,  659,  659,  495,  261,  274,  659,  659,  659,
 /*  7760 */   659,  659,  659,  659,  659,  659,  492,  659,  659,  659,
 /*  7770 */   659,  659,  659,  659,  659,  659,  432,  434,  436,  438,
 /*  7780 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  273,
 /*  7790 */   252,  253,  254,  255,  256,  257,  258,  259,  260,  262,
 /*  7800 */   263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
 /*  7810 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7820 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7830 */   659,  780,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7840 */   495,  261,  274,  659,  659,  659,  659,  659,  659,  659,
 /*  7850 */   659,  659,  492,  659,  659,  659,  659,  659,  659,  659,
 /*  7860 */   659,  659,  432,  434,  436,  438,  659,  659,  659,  659,
 /*  7870 */   659,  659,  659,  659,  659,  273,  252,  253,  254,  255,
 /*  7880 */   256,  257,  258,  259,  260,  262,  263,  264,  265,  266,
 /*  7890 */   267,  268,  269,  270,  271,  272,  659,  659,  659,  659,
 /*  7900 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7910 */   659,  659,  659,  659,  659,  659,  659,  787,  659,  659,
 /*  7920 */   659,  659,  659,  659,  659,  659,  495,  261,  274,  659,
 /*  7930 */   659,  659,  659,  659,  659,  659,  659,  659,  492,  659,
 /*  7940 */   659,  659,  659,  659,  659,  659,  659,  659,  432,  434,
 /*  7950 */   436,  438,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7960 */   659,  273,  252,  253,  254,  255,  256,  257,  258,  259,
 /*  7970 */   260,  262,  263,  264,  265,  266,  267,  268,  269,  270,
 /*  7980 */   271,  272,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  7990 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8000 */   659,  659,  659,  794,  659,  659,  659,  659,  659,  659,
 /*  8010 */   659,  659,  495,  261,  274,  659,  659,  659,  659,  659,
 /*  8020 */   659,  659,  659,  659,  492,  659,  659,  659,  659,  659,
 /*  8030 */   659,  659,  659,  659,  432,  434,  436,  438,  659,  659,
 /*  8040 */   659,  659,  659,  659,  659,  659,  659,  273,  252,  253,
 /*  8050 */   254,  255,  256,  257,  258,  259,  260,  262,  263,  264,
 /*  8060 */   265,  266,  267,  268,  269,  270,  271,  272,  659,  659,
 /*  8070 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8080 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  801,
 /*  8090 */   659,  659,  659,  659,  659,  659,  659,  659,  495,  261,
 /*  8100 */   274,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8110 */   492,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8120 */   432,  434,  436,  438,  659,  659,  659,  659,    3,    4,
 /*  8130 */     5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
 /*  8140 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*  8150 */    25,   26,   27,   28,  659,  659,  659,  659,  659,  659,
 /*  8160 */   659,  659,  659,  659,  273,  252,  253,  254,  255,  256,
 /*  8170 */   257,  258,  259,  260,  262,  263,  264,  265,  266,  267,
 /*  8180 */   268,  269,  270,  271,  272,  659,  659,  659,  659,  659,
 /*  8190 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8200 */   659,  659,  659,  659,  659,  659,  659,  877,  878,  879,
 /*  8210 */   659,  659,  659,  659,  659,  251,  261,  274,  659,  659,
 /*  8220 */   659,  659,  659,  659,  659,  659,  659,  492,  659,  659,
 /*  8230 */   659,  659,  659,  659,  659,  659,  659,  432,  434,  436,
 /*  8240 */   438,  659,  659,  659,  659,   52,  659,   61,   64,  659,
 /*  8250 */   659,  659,  659,  181,  659,  659,  659,  659,  659,  127,
 /*  8260 */   659,  659,  659,  659,  659,  659,   46,  659,  659,  659,
 /*  8270 */   659,   49,  138,   58,  143,   55,  163,   43,  628,  153,
 /*  8280 */   659,   52,  135,   61,   64,  659,  659,  659,  659,  181,
 /*  8290 */   659,  659,  170,  659,  659,  127,  659,  152,  183,  107,
 /*  8300 */   659,  659,   46,  149,  659,  659,  659,   49,  138,   58,
 /*  8310 */   143,   55,  163,   43,  605,  153,  659,  659,  135,  659,
 /*  8320 */   659,  659,  659,  659,  659,  659,  659,  659,  170,  659,
 /*  8330 */   659,  659,  659,  152,  183,  107,  659,  659,  659,  149,
 /*  8340 */   659,  659,  659,  659,  146,  659,  659,  659,  659,  659,
 /*  8350 */   659,  659,  145,  147,  148,  150,  151,   89,   87,   93,
 /*  8360 */    91,   95,   97,   99,   45,   51,   57,   60,   63,   66,
 /*  8370 */    54,   48,   75,   77,   85,   79,   81,   83,  659,  659,
 /*  8380 */   146,  659,  659,  659,  659,  659,   72,   68,  145,  147,
 /*  8390 */   148,  150,  151,  659,  659,  659,   52,  659,   61,   64,
 /*  8400 */   659,  659,  659,  659,  181,  659,  659,  659,  659,  659,
 /*  8410 */   127,  659,  659,  659,  659,  659,  659,   46,  659,  659,
 /*  8420 */   659,  659,   49,  138,   58,  143,   55,  163,   43,  601,
 /*  8430 */   153,  659,   52,  135,   61,   64,  659,  659,  659,  659,
 /*  8440 */   181,  659,  659,  170,  659,  659,  127,  659,  152,  183,
 /*  8450 */   107,  659,  659,   46,  149,  659,  659,  659,   49,  138,
 /*  8460 */    58,  143,   55,  163,   43,  166,  153,  659,  662,  135,
 /*  8470 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  170,
 /*  8480 */   659,  659,  659,  659,  152,  183,  107,  659,  659,  659,
 /*  8490 */   149,  659,  659,  659,  659,  146,  659,  659,  659,  659,
 /*  8500 */   659,  659,  659,  145,  147,  148,  150,  151,  659,  659,
 /*  8510 */   659,  652,  658,  659,  659,  108,  109,  110,  111,  112,
 /*  8520 */   113,  114,  115,  116,  117,  118,  119,  120,  121,  659,
 /*  8530 */   659,  146,  659,  659,  659,  659,  659,  659,  659,  145,
 /*  8540 */   147,  148,  150,  151,  659,  659,  659,   52,  659,   61,
 /*  8550 */    64,  659,  659,  659,  659,  181,  659,  659,  659,  659,
 /*  8560 */   659,  127,  659,  659,  659,  659,  659,  659,   46,  659,
 /*  8570 */   659,  659,  659,   49,  138,   58,  143,   55,  163,   43,
 /*  8580 */   176,  153,  659,   52,  135,   61,   64,  659,  659,  659,
 /*  8590 */   659,  181,  659,  659,  170,  659,  659,  127,  659,  152,
 /*  8600 */   183,  107,  659,  659,   46,  149,  659,  659,  659,   49,
 /*  8610 */   138,   58,  143,   55,  163,   43,  575,  153,  659,  662,
 /*  8620 */   135,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8630 */   170,  659,  659,  659,  659,  152,  183,  107,  659,  659,
 /*  8640 */   659,  149,  659,  659,  659,  659,  146,  659,  659,  659,
 /*  8650 */   659,  659,  659,  659,  145,  147,  148,  150,  151,  659,
 /*  8660 */   659,  659,  659,  658,  659,  659,  108,  109,  110,  111,
 /*  8670 */   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,
 /*  8680 */   659,  659,  146,  659,  659,  659,  659,  659,  659,  659,
 /*  8690 */   145,  147,  148,  150,  151,  659,  659,  659,   52,  659,
 /*  8700 */    61,   64,  659,  659,  659,  659,  181,  659,  659,  659,
 /*  8710 */   659,  659,  127,  659,  659,  659,  659,  659,  659,   46,
 /*  8720 */   659,  659,  659,  659,   49,  138,   58,  143,   55,  163,
 /*  8730 */    43,  582,  153,  659,   52,  135,   61,   64,  659,  659,
 /*  8740 */   659,  659,  181,  659,  659,  170,  659,  659,  127,  659,
 /*  8750 */   152,  183,  107,  659,  659,   46,  149,  659,  659,  659,
 /*  8760 */    49,  138,   58,  143,   55,  163,   43,  588,  153,  659,
 /*  8770 */   659,  135,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8780 */   659,  170,  659,  659,  659,  659,  152,  183,  107,  659,
 /*  8790 */   659,  659,  149,  659,  659,  659,  659,  146,  659,  659,
 /*  8800 */   659,  659,  659,  659,  659,  145,  147,  148,  150,  151,
 /*  8810 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8820 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8830 */   659,  659,  659,  146,  659,  659,  659,  659,  659,  659,
 /*  8840 */   659,  145,  147,  148,  150,  151,  659,  659,  659,   52,
 /*  8850 */   659,   61,   64,  659,  659,  659,  659,  181,  659,  659,
 /*  8860 */   659,  659,  659,  127,  659,  659,  659,  659,  659,  659,
 /*  8870 */    46,  659,  659,  659,  659,   49,  138,   58,  143,   55,
 /*  8880 */   163,   43,  612,  153,  659,   52,  135,   61,   64,  659,
 /*  8890 */   659,  659,  659,  181,  659,  659,  170,  659,  659,  127,
 /*  8900 */   659,  152,  183,  107,  659,  659,   46,  149,  659,  659,
 /*  8910 */   659,   49,  138,   58,  143,   55,  163,   43,  618,  153,
 /*  8920 */   659,  659,  135,  659,  659,  659,  659,  659,  659,  659,
 /*  8930 */   659,  659,  170,  659,  659,  659,  659,  152,  183,  107,
 /*  8940 */   659,  659,  659,  149,  659,  659,  659,  659,  146,  659,
 /*  8950 */   659,  659,  659,  659,  659,  659,  145,  147,  148,  150,
 /*  8960 */   151,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8970 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  8980 */   659,  659,  659,  659,  146,  659,  659,  659,  659,  659,
 /*  8990 */   659,  659,  145,  147,  148,  150,  151,  659,  659,  659,
 /*  9000 */   659,  103,  659,  659,  101,   89,   87,   93,   91,   95,
 /*  9010 */    97,   99,   45,   51,   57,   60,   63,   66,   54,   48,
 /*  9020 */    75,   77,   85,   79,   81,   83,  659,  659,  659,  659,
 /*  9030 */   659,  659,  659,  103,   72,   68,  101,   89,   87,   93,
 /*  9040 */    91,   95,   97,   99,   45,   51,   57,   60,   63,   66,
 /*  9050 */    54,   48,   75,   77,   85,   79,   81,   83,  659,  659,
 /*  9060 */    74,  659,  659,  659,  659,  103,   72,   68,  101,   89,
 /*  9070 */    87,   93,   91,   95,   97,   99,   45,   51,   57,   60,
 /*  9080 */    63,   66,   54,   48,   75,   77,   85,   79,   81,   83,
 /*  9090 */   659,  659,   52,  659,   61,   64,  659,  123,   72,   68,
 /*  9100 */   597,  659,  659,  659,  659,  659,  127,  659,  659,  659,
 /*  9110 */   659,  659,  659,   46,  105,  659,  659,  659,   49,  138,
 /*  9120 */    58,  143,   55,  163,   43,  659,  153,  659,   52,  131,
 /*  9130 */    61,   64,  659,  659,  659,  659,  659,  659,  659,  170,
 /*  9140 */   659,  659,  127,  659,  152,  183,  107,  659,  659,   46,
 /*  9150 */   149,  659,  659,  659,   49,  138,   58,  143,   55,  163,
 /*  9160 */    43,  233,  153,  659,  659,  155,  659,  659,  659,  659,
 /*  9170 */   659,  659,  659,  659,  659,  170,  659,  659,  659,  659,
 /*  9180 */   158,  183,  107,  659,  188,  659,  149,  659,  159,  659,
 /*  9190 */   659,  146,  659,  659,  659,  659,  659,  659,  217,  145,
 /*  9200 */   147,  148,  150,  151,  659,  659,  659,  659,  108,  109,
 /*  9210 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  9220 */   120,  121,  659,  659,  659,  659,  659,  157,  659,  659,
 /*  9230 */   659,  659,  659,  659,  659,  156,  147,  148,  150,  151,
 /*  9240 */   659,  659,  659,   52,  659,   61,   64,  659,  659,  659,
 /*  9250 */   659,  181,  659,  659,  659,  659,  659,  127,  659,  659,
 /*  9260 */   659,  659,  659,  659,   46,  659,  659,  659,  659,   49,
 /*  9270 */   138,   58,  143,   55,  163,   43,  659,  153,  659,   52,
 /*  9280 */   135,   61,   64,  659,  659,  659,  659,  659,  659,  659,
 /*  9290 */   170,  659,  659,  127,  659,  152,  183,  107,  659,  659,
 /*  9300 */    46,  149,  659,  659,  659,   49,  138,   58,  143,   55,
 /*  9310 */   163,   43,  659,  153,  659,  659,  131,  659,  659,  659,
 /*  9320 */   659,  659,  659,  659,  659,  659,  170,  659,  659,  659,
 /*  9330 */   659,  152,  183,  107,  659,  659,  659,  149,  659,  659,
 /*  9340 */   659,  659,  146,  659,  659,  659,  659,  659,  659,  659,
 /*  9350 */   145,  147,  148,  150,  151,  659,  659,  659,  659,  659,
 /*  9360 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  9370 */   497,  659,  659,  659,  659,  659,  659,  659,  146,  659,
 /*  9380 */   659,  659,  659,  659,  659,  659,  145,  147,  148,  150,
 /*  9390 */   151,  659,  659,  659,  659,  103,  659,  659,  101,   89,
 /*  9400 */    87,   93,   91,   95,   97,   99,   45,   51,   57,   60,
 /*  9410 */    63,   66,   54,   48,   75,   77,   85,   79,   81,   83,
 /*  9420 */   659,   52,  659,   61,   64,  659,  659,  659,   72,   68,
 /*  9430 */   659,  659,  659,  659,  659,  127,  659,  659,  659,  659,
 /*  9440 */   659,  333,   46,  659,  659,  659,  659,   49,  138,   58,
 /*  9450 */   143,   55,  163,   43,  659,  153,  659,  659,  131,  659,
 /*  9460 */   659,  659,  659,  659,  659,  659,  659,  659,  170,  659,
 /*  9470 */   659,  659,  659,  152,  183,  107,  659,  659,  659,  149,
 /*  9480 */   659,  357,  103,  659,  659,  101,   89,   87,   93,   91,
 /*  9490 */    95,   97,   99,   45,   51,   57,   60,   63,   66,   54,
 /*  9500 */    48,   75,   77,   85,   79,   81,   83,  659,  659,  659,
 /*  9510 */   659,  659,  659,  659,  659,   72,   68,  659,  659,  659,
 /*  9520 */   146,  659,  659,   52,  659,   61,   64,  659,  145,  147,
 /*  9530 */   148,  150,  151,  659,  659,  659,  659,  127,  659,  659,
 /*  9540 */   659, 1172,  659,  659,   46,  659,  659,  659,  659,   49,
 /*  9550 */   138,   58,  143,   55,  163,   43,  659,  153,  659,   52,
 /*  9560 */   131,   61,   64,  659,  659,  659,  659,  659,  659,  659,
 /*  9570 */   170,  659,  659,  127,  659,  152,  183,  107,  659,  659,
 /*  9580 */    46,  149,  659,  367,  659,   49,  138,   58,  143,   55,
 /*  9590 */   163,   43,  659,  153,  659,  659,  131,  659,  659,  659,
 /*  9600 */   659,  659,  659,  659,  659,  659,  170,  659,  659,  659,
 /*  9610 */   659,  152,  183,  107,  659,  659,  659,  149,  659,  388,
 /*  9620 */   659,  659,  146,  659,  659,   52,  659,   61,   64,  659,
 /*  9630 */   145,  147,  148,  150,  151,  659,  659,  659,  659,  127,
 /*  9640 */   659,  659,  659,  659,  659,  659,   46,  659,  659,  659,
 /*  9650 */   659,   49,  138,   58,  143,   55,  163,   43,  146,  153,
 /*  9660 */   659,   52,  131,   61,   64,  659,  145,  147,  148,  150,
 /*  9670 */   151,  659,  170,  659,  659,  127,  659,  152,  183,  107,
 /*  9680 */   659,  659,   46,  149,  659,  395,  659,   49,  138,   58,
 /*  9690 */   143,   55,  163,   43,  659,  153,  659,  659,  131,  659,
 /*  9700 */   659,  659,  659,  659,  659,  659,  659,  659,  170,  659,
 /*  9710 */   659,  659,  659,  152,  183,  107,  659,  659,  659,  149,
 /*  9720 */   659,  399,  659,  659,  146,  659,  659,   52,  659,   61,
 /*  9730 */    64,  659,  145,  147,  148,  150,  151,  659,  659,  659,
 /*  9740 */   659,  127,  659,  659,  659,  659,  659,  659,   46,  659,
 /*  9750 */   659,  659,  659,   49,  138,   58,  143,   55,  163,   43,
 /*  9760 */   146,  153,  659,   52,  131,   61,   64,  659,  145,  147,
 /*  9770 */   148,  150,  151,  659,  170,  659,  659,  127,  659,  152,
 /*  9780 */   183,  107,  659,  659,   46,  149,  659,  406,  659,   49,
 /*  9790 */   138,   58,  143,   55,  163,   43,  659,  153,  659,  659,
 /*  9800 */   131,  443,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  9810 */   170,  659,  659,  659,  659,  152,  183,  107,  659,  659,
 /*  9820 */   659,  149,  659,  659,  659,  659,  146,  659,  659,  659,
 /*  9830 */   659,  659,  659,  659,  145,  147,  148,  150,  151,  659,
 /*  9840 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  9850 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /*  9860 */   659,  659,  146,  659,  659,  659,  659,  659,  659,  659,
 /*  9870 */   145,  147,  148,  150,  151,  659,  659,  659,  659,  103,
 /*  9880 */   659,  659,  101,   89,   87,   93,   91,   95,   97,   99,
 /*  9890 */    45,   51,   57,   60,   63,   66,   54,   48,   75,   77,
 /*  9900 */    85,   79,   81,   83,  659,  659,  659,  659,  659,  659,
 /*  9910 */   659,  659,   72,   68,  659,  659,  442,  659,  659,  659,
 /*  9920 */   659,  103,  659,  659,  101,   89,   87,   93,   91,   95,
 /*  9930 */    97,   99,   45,   51,   57,   60,   63,   66,   54,   48,
 /*  9940 */    75,   77,   85,   79,   81,   83,  659,  659,  659,  659,
 /*  9950 */   659,  659,  659,  659,   72,   68,  659,  659,  446,  659,
 /*  9960 */   659,  659,  659,  103,  659,  659,  101,   89,   87,   93,
 /*  9970 */    91,   95,   97,   99,   45,   51,   57,   60,   63,   66,
 /*  9980 */    54,   48,   75,   77,   85,   79,   81,   83,  659,  659,
 /*  9990 */   659,  659,  659,  659,  659,  659,   72,   68,  659,  659,
 /* 10000 */   449,  659,  659,  659,  659,  103,  659,  659,  101,   89,
 /* 10010 */    87,   93,   91,   95,   97,   99,   45,   51,   57,   60,
 /* 10020 */    63,   66,   54,   48,   75,   77,   85,   79,   81,   83,
 /* 10030 */   659,  659,  659,  659,  659,  659,  659,  659,   72,   68,
 /* 10040 */   659,  659,  452,  659,  659,  659,  659,  103,  659,  659,
 /* 10050 */   101,   89,   87,   93,   91,   95,   97,   99,   45,   51,
 /* 10060 */    57,   60,   63,   66,   54,   48,   75,   77,   85,   79,
 /* 10070 */    81,   83,  659,   52,  659,   61,   64,  659,  659,  659,
 /* 10080 */    72,   68,  659,  659,  659,  659,  659,  127,  659,  659,
 /* 10090 */   659,  659,  659,  499,   46,  659,  659,  659,  659,   49,
 /* 10100 */   138,   58,  143,   55,  163,   43,  659,  153,  659,  659,
 /* 10110 */   131,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /* 10120 */   170,  659,  659,  659,  659,  152,  183,  107,  659,  103,
 /* 10130 */   659,  149,  101,   89,   87,   93,   91,   95,   97,   99,
 /* 10140 */    45,   51,   57,   60,   63,   66,   54,   48,   75,   77,
 /* 10150 */    85,   79,   81,   83,  659,  659,  659,  659,  659,  659,
 /* 10160 */   659,  659,   72,   68,  510,  659,  659,  659,  659,  659,
 /* 10170 */   659,  659,  146,  659,  659,  506,  659,  659,  659,  659,
 /* 10180 */   145,  147,  148,  150,  151,  659,  659,  659,  659,  103,
 /* 10190 */   659,  659,  101,   89,   87,   93,   91,   95,   97,   99,
 /* 10200 */    45,   51,   57,   60,   63,   66,   54,   48,   75,   77,
 /* 10210 */    85,   79,   81,   83,  659,   52,  659,   61,   64,  659,
 /* 10220 */   659,  659,   72,   68,  659,  659,  659,  659,  659,  127,
 /* 10230 */   659,  659,  659,  659,  659,  512,   46,  659,  659,  659,
 /* 10240 */   659,   49,  138,   58,  143,   55,  163,   43,  659,  153,
 /* 10250 */   659,  659,  131,  659,  659,  659,  659,  659,  659,  659,
 /* 10260 */   659,  659,  170,  659,  659,  659,  659,  152,  183,  107,
 /* 10270 */   659,  103,  659,  149,  101,   89,   87,   93,   91,   95,
 /* 10280 */    97,   99,   45,   51,   57,   60,   63,   66,   54,   48,
 /* 10290 */    75,   77,   85,   79,   81,   83,  659,  659,  659,  659,
 /* 10300 */   659,  659,  659,  659,   72,   68,  659,  659,  659,  659,
 /* 10310 */   659,  659,  659,   52,  146,   61,   64,  659,  659,  659,
 /* 10320 */   659,  659,  145,  147,  148,  150,  151,  127,  659,  659,
 /* 10330 */   659,  659,  659,  659,   46,  659,  659,  659,  659,   49,
 /* 10340 */   138,   58,  143,   55,  163,   43,  659,  153,  659,  659,
 /* 10350 */   155,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /* 10360 */   170,  659,  659,  659,  659,  158,  183,  107,  659,  659,
 /* 10370 */   659,  149,  101,   89,   87,   93,   91,   95,   97,   99,
 /* 10380 */    45,   51,   57,   60,   63,   66,   54,   48,   75,   77,
 /* 10390 */    85,   79,   81,   83,  233,  659,  659,  659,  659,  659,
 /* 10400 */   659,  659,   72,   68,  659,  659,  659,  659,  659,  659,
 /* 10410 */   659,  659,  157,  565,  659,  659,  659,  188,  659,  659,
 /* 10420 */   156,  147,  148,  150,  151,  659,  659,  659,  659,  659,
 /* 10430 */   659,  217,  659,  659,  659,  659,  659,  659,  659,  233,
 /* 10440 */   659,  108,  109,  110,  111,  112,  113,  114,  115,  116,
 /* 10450 */   117,  118,  119,  120,  121,  659,  659,  659,  648,  659,
 /* 10460 */   659,  659,  188,  659,  659,  659,  659,  659,  659,  659,
 /* 10470 */   659,  659,  659,  659,  659,  659,  217,  659,  659,  659,
 /* 10480 */   659,  659,  659,  659,  233,  659,  108,  109,  110,  111,
 /* 10490 */   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,
 /* 10500 */   659,  659,  659,  690,  659,  659,  659,  188,  659,  659,
 /* 10510 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /* 10520 */   659,  217,  659,  659,  659,  659,  659,  659,  659,  233,
 /* 10530 */   659,  108,  109,  110,  111,  112,  113,  114,  115,  116,
 /* 10540 */   117,  118,  119,  120,  121,  659,  659,  659,  745,  659,
 /* 10550 */   659,  659,  188,  659,  659,  659,  659,  659,  659,  659,
 /* 10560 */   659,  659,  659,  659,  659,  659,  217,  659,  659,  659,
 /* 10570 */   659,  659,  659,  659,  233,  659,  108,  109,  110,  111,
 /* 10580 */   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,
 /* 10590 */   659,  659,  659,  777,  659,  659,  659,  188,  659,  659,
 /* 10600 */   659,  659,  659,  659,  659,  659,  659,  659,  659,  659,
 /* 10610 */   659,  217,  659,  659,  659,  659,  659,  659,  659,  659,
 /* 10620 */   659,  108,  109,  110,  111,  112,  113,  114,  115,  116,
 /* 10630 */   117,  118,  119,  120,  121,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */     7,  126,    9,   10,  154,   61,    6,  157,  158,  159,
 /*    10 */     6,  161,  162,  170,   21,   41,   42,  167,  168,  176,
 /*    20 */   177,   28,  179,  173,   44,  150,   33,   34,   35,   36,
 /*    30 */    37,   38,   39,   89,   41,   55,   56,   44,   45,    1,
 /*    40 */     2,    3,    4,    5,   40,   45,    0,   54,   55,   56,
 /*    50 */   170,  107,   59,   60,   61,   22,  176,  177,   65,  179,
 /*    60 */   163,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*    70 */    77,   78,  170,   44,   41,   82,   83,   84,  176,  177,
 /*    80 */    87,  179,   45,   54,   91,   92,   93,   94,   47,   96,
 /*    90 */    52,   53,   99,   55,   56,   55,   58,   44,    6,  106,
 /*   100 */    62,   63,   61,  110,  111,  112,  113,  114,  115,  116,
 /*   110 */   117,  118,  119,    7,  217,    9,   10,   27,   28,   29,
 /*   120 */    30,   31,   32,  176,  177,  192,  179,   21,   88,  196,
 /*   130 */    90,   41,   42,  163,   28,  202,  203,  204,  205,   33,
 /*   140 */    34,   35,   36,   37,   38,   39,   42,   41,  107,   45,
 /*   150 */    44,   45,    1,    2,    3,    4,    5,  162,   54,   67,
 /*   160 */    54,   55,   56,  168,   61,   59,   60,   61,  173,   61,
 /*   170 */   195,   65,  197,  198,   68,   69,   70,   71,   72,   73,
 /*   180 */    74,   75,   76,   77,   78,  215,  216,  217,   82,   83,
 /*   190 */    84,   44,   89,   87,  176,  177,  126,   91,   92,   93,
 /*   200 */    94,   54,   96,   52,   53,   99,   55,   56,  195,   58,
 /*   210 */   107,  198,  106,   62,   63,  107,  110,  111,  112,  113,
 /*   220 */   114,  115,  116,  117,  118,  119,    7,  162,    9,   10,
 /*   230 */   195,  192,  197,  198,   30,   31,   32,   55,  173,   61,
 /*   240 */    21,  202,  203,  204,  205,   41,   42,   28,  209,  210,
 /*   250 */    85,   86,   33,   34,   35,   36,   37,   38,   39,  195,
 /*   260 */    41,  197,  198,   44,   45,    1,    2,    3,    4,    5,
 /*   270 */    88,   49,   90,   54,   55,   56,   54,   39,   59,   60,
 /*   280 */    61,   49,   44,   51,   65,  107,   54,   68,   69,   70,
 /*   290 */    71,   72,   73,   74,   75,   76,   77,   78,  163,   44,
 /*   300 */     6,   82,   83,   84,   44,   89,   87,   44,   14,   54,
 /*   310 */    91,   92,   93,   94,   54,   96,   52,   53,   99,   55,
 /*   320 */    56,   47,   58,  107,    6,  106,   62,   63,   44,  110,
 /*   330 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*   340 */     6,    9,   10,   59,  192,  184,  185,   61,   54,  214,
 /*   350 */   215,  216,  217,   21,  202,  203,  204,  205,   40,   44,
 /*   360 */    28,  209,  210,  108,  109,   33,   34,   35,   36,   37,
 /*   370 */    38,   39,   42,   41,   40,   45,   44,   45,    1,    2,
 /*   380 */     3,    4,    5,  162,   54,   22,   54,   55,   56,   44,
 /*   390 */   169,   59,   60,   61,  173,   85,   86,   65,    6,   54,
 /*   400 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*   410 */    78,  170,  186,  187,   82,   83,   84,  176,  177,   87,
 /*   420 */   179,  106,   39,   91,   92,   93,   94,   44,   96,   52,
 /*   430 */    53,   99,   55,   56,    6,   58,  188,  189,  106,   62,
 /*   440 */    63,   44,  110,  111,  112,  113,  114,  115,  116,  117,
 /*   450 */   118,  119,    7,   56,    9,   10,   49,  192,   51,   67,
 /*   460 */    42,   54,  159,   45,  161,  162,   21,  202,  203,  204,
 /*   470 */   205,  168,   54,   28,  209,  210,  173,   44,   33,   34,
 /*   480 */    35,   36,   37,   38,   39,   42,   41,    6,   45,   44,
 /*   490 */    45,    1,    2,    3,    4,    5,  153,   54,  155,   54,
 /*   500 */    55,   56,  190,  191,   59,   60,   61,   49,  195,   51,
 /*   510 */    65,  198,   54,   68,   69,   70,   71,   72,   73,   74,
 /*   520 */    75,   76,   77,   78,  170,   97,   45,   82,   83,   84,
 /*   530 */   176,  177,   87,  179,  193,  194,   91,   92,   93,   94,
 /*   540 */    40,   96,   52,   53,   99,   55,   56,  195,   58,   44,
 /*   550 */   198,  106,   62,   63,   44,  110,  111,  112,  113,  114,
 /*   560 */   115,  116,  117,  118,  119,    7,   42,    9,   10,   45,
 /*   570 */   192,    6,   61,  175,  176,  177,  178,  206,  207,   21,
 /*   580 */   202,  203,  204,  205,   44,    6,   28,  209,  210,   45,
 /*   590 */     6,   33,   34,   35,   36,   37,   38,   39,   54,   41,
 /*   600 */     6,   57,   44,   45,    1,    2,    3,    4,    5,   89,
 /*   610 */    45,  106,   54,   55,   56,  206,  207,   59,   60,   61,
 /*   620 */   206,  207,   42,   65,   45,   45,   68,   69,   70,   71,
 /*   630 */    72,   73,   74,   75,   76,   77,   78,  206,  207,   45,
 /*   640 */    82,   83,   84,    6,   45,   87,    6,  206,  207,   91,
 /*   650 */    92,   93,   94,   54,   96,   52,   53,   99,   55,   56,
 /*   660 */    44,   58,   89,    6,  106,   62,   63,   89,  110,  111,
 /*   670 */   112,  113,  114,  115,  116,  117,  118,  119,    7,   45,
 /*   680 */     9,   10,   45,  192,    6,   45,  206,  207,   54,  206,
 /*   690 */   207,   57,   21,  202,  203,  204,  205,   61,    6,   28,
 /*   700 */   209,  210,   45,   44,   33,   34,   35,   36,   37,   38,
 /*   710 */    39,    6,   41,  206,  207,   44,   45,    1,    2,    3,
 /*   720 */     4,    5,  106,   45,   45,   54,   55,   56,  206,  207,
 /*   730 */    59,   60,   61,   54,    6,    6,   65,   45,   55,   68,
 /*   740 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   750 */    45,   44,   57,   82,   83,   84,  206,  207,   87,  206,
 /*   760 */   207,   61,   91,   92,   93,   94,   59,   96,   52,   53,
 /*   770 */    99,   55,   56,   45,   45,    6,    6,  106,   62,   63,
 /*   780 */    57,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*   790 */   119,    7,  163,    9,   10,  100,  101,  102,  103,  104,
 /*   800 */   105,    6,   85,   86,  172,   21,  174,  175,  176,  177,
 /*   810 */   178,  107,   28,    6,   45,  184,  185,   33,   34,   35,
 /*   820 */    36,   37,   38,   39,   54,   41,   85,   86,   44,   45,
 /*   830 */     1,    2,    3,    4,    5,   40,   59,   45,   54,   55,
 /*   840 */    56,   45,   89,   59,   60,   61,   54,   40,   42,   65,
 /*   850 */    54,   45,   68,   69,   70,   71,   72,   73,   74,   75,
 /*   860 */    76,   77,   78,    6,   44,   89,   82,   83,   84,   42,
 /*   870 */    51,   87,   45,   54,    6,   91,   92,   93,   94,   59,
 /*   880 */    96,   52,   53,   99,   55,   56,  165,  166,   89,   57,
 /*   890 */   106,   62,   63,  163,  110,  111,  112,  113,  114,  115,
 /*   900 */   116,  117,  118,  119,    7,    6,    9,   10,   40,   57,
 /*   910 */    51,  192,   55,   54,    6,    6,   44,  162,   21,  200,
 /*   920 */   201,  202,  203,  204,  205,   28,    6,    6,  173,    6,
 /*   930 */    33,   34,   35,   36,   37,   38,   39,  163,   41,   40,
 /*   940 */    57,   44,   45,    1,    2,    3,    4,    5,   40,   40,
 /*   950 */    45,   54,   55,   56,  163,   44,   59,   60,   61,   54,
 /*   960 */    40,   40,   65,   40,   57,   68,   69,   70,   71,   72,
 /*   970 */    73,   74,   75,   76,   77,   78,   45,  163,   44,   82,
 /*   980 */    83,   84,   22,   57,   87,   54,  163,    6,   91,   92,
 /*   990 */    93,   94,   44,   96,   52,   53,   99,   55,   56,  153,
 /*  1000 */   163,  155,    6,  106,   62,   63,   57,  110,  111,  112,
 /*  1010 */   113,  114,  115,  116,  117,  118,  119,    7,    6,    9,
 /*  1020 */    10,   40,  192,  153,   44,  155,  162,  153,  153,  155,
 /*  1030 */   155,   21,  202,  203,  204,  205,   40,  173,   28,  209,
 /*  1040 */   210,   57,  162,   33,   34,   35,   36,   37,   38,   39,
 /*  1050 */   163,   41,   40,  173,   44,   45,    1,    2,    3,    4,
 /*  1060 */     5,   54,   45,   45,   54,   55,   56,   54,   54,   59,
 /*  1070 */    60,   61,  187,  185,   54,   65,  163,   89,   68,   69,
 /*  1080 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   54,
 /*  1090 */    54,   54,   82,   83,   84,   92,   45,   87,   54,   95,
 /*  1100 */   194,   91,   92,   93,   94,  189,   96,   52,   53,   99,
 /*  1110 */    55,   56,   44,   44,  195,   67,  106,   62,   63,  195,
 /*  1120 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  1130 */     7,   55,    9,   10,  195,  192,   55,   44,   44,  195,
 /*  1140 */   195,  195,  195,   44,   21,  202,  203,  204,  205,  195,
 /*  1150 */    55,   28,  209,  210,  195,   55,   33,   34,   35,   36,
 /*  1160 */    37,   38,   39,  195,   41,   45,   45,   44,   45,    1,
 /*  1170 */     2,    3,    4,    5,   45,   45,   44,   54,   55,   56,
 /*  1180 */   207,   57,   59,   60,   61,  163,   44,   97,   65,   44,
 /*  1190 */    54,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /*  1200 */    77,   78,  191,   92,  183,   82,   83,   84,   45,   54,
 /*  1210 */    87,   89,   44,  192,   91,   92,   93,   94,   54,   96,
 /*  1220 */    52,   53,   99,  202,  203,  204,  205,   54,   60,  106,
 /*  1230 */    62,   63,   55,  110,  111,  112,  113,  114,  115,  116,
 /*  1240 */   117,  118,  119,    7,   54,    9,   10,   54,  192,   61,
 /*  1250 */    44,   55,   61,   44,   55,   61,   89,   21,  202,  203,
 /*  1260 */   204,  205,   55,   61,   28,  209,  210,   55,   44,   33,
 /*  1270 */    34,   35,   36,   37,   38,   39,   61,   41,   44,  151,
 /*  1280 */    44,   45,    1,    2,    3,    4,    5,   44,  151,   54,
 /*  1290 */    54,   55,   56,   55,   44,   59,   60,   61,  173,   45,
 /*  1300 */    61,   65,   14,   39,   68,   69,   70,   71,   72,   73,
 /*  1310 */    74,   75,   76,   77,   78,   44,   67,  183,   82,   83,
 /*  1320 */    84,   22,   45,   87,   44,   44,  192,   91,   92,   93,
 /*  1330 */    94,   61,   96,   52,   53,   99,  202,  203,  204,  205,
 /*  1340 */    45,   60,  106,   62,   63,   45,  110,  111,  112,  113,
 /*  1350 */   114,  115,  116,  117,  118,  119,    7,   45,    9,   10,
 /*  1360 */    57,  192,   57,  163,  163,   45,   57,  163,   45,   45,
 /*  1370 */    21,  202,  203,  204,  205,   57,  163,   28,  209,  210,
 /*  1380 */    44,  152,   33,   34,   35,   36,   37,   38,   39,   44,
 /*  1390 */    41,   44,  152,   44,   45,    1,    2,    3,    4,    5,
 /*  1400 */   152,   55,   44,   54,   55,   56,   44,  163,   59,   60,
 /*  1410 */    61,   61,   61,   45,   65,  164,  164,   68,   69,   70,
 /*  1420 */    71,   72,   73,   74,   75,   76,   77,   78,   45,   45,
 /*  1430 */    44,   82,   83,   84,  166,   45,   87,  164,  163,  155,
 /*  1440 */    91,   92,   93,   94,   45,   96,   52,   53,   99,   55,
 /*  1450 */    56,  164,   45,  152,   50,  106,   62,   63,   44,  110,
 /*  1460 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  1470 */    44,    9,   10,  152,  192,   50,   54,  152,  152,  152,
 /*  1480 */    44,  152,   44,   21,  202,  203,  204,  205,  218,  152,
 /*  1490 */    28,  209,  210,  152,  218,   33,   34,   35,   36,   37,
 /*  1500 */    38,   39,  218,   41,  218,  218,   44,   45,    1,    2,
 /*  1510 */     3,    4,    5,  218,  218,  218,   54,   55,   56,  218,
 /*  1520 */   218,   59,   60,   61,  218,  218,  218,   65,  218,  218,
 /*  1530 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  1540 */    78,  218,  218,  218,   82,   83,   84,  218,  218,   87,
 /*  1550 */   218,  218,  218,   91,   92,   93,   94,  218,   96,   52,
 /*  1560 */    53,   99,   55,   56,  218,  218,  218,  218,  106,   62,
 /*  1570 */    63,  218,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  1580 */   118,  119,    7,  218,    9,   10,  218,  218,  156,  218,
 /*  1590 */   158,  218,  160,  160,  162,  162,   21,  218,  218,  167,
 /*  1600 */   167,  169,  169,   28,  218,  173,  173,  218,   33,   34,
 /*  1610 */    35,   36,   37,   38,   39,  218,   41,  218,  218,   44,
 /*  1620 */    45,    1,    2,    3,    4,    5,  218,  218,  218,   54,
 /*  1630 */    55,   56,  218,  218,   59,   60,   61,  218,  218,  218,
 /*  1640 */    65,  218,  218,   68,   69,   70,   71,   72,   73,   74,
 /*  1650 */    75,   76,   77,   78,  218,  218,  218,   82,   83,   84,
 /*  1660 */   218,  218,   87,  218,  218,  218,   91,   92,   93,   94,
 /*  1670 */   218,   96,   52,   53,   99,   55,   56,  218,  218,  218,
 /*  1680 */   218,  106,   62,   63,  218,  110,  111,  112,  113,  114,
 /*  1690 */   115,  116,  117,  118,  119,    7,  218,    9,   10,  218,
 /*  1700 */   218,  218,  218,  158,  159,  218,  161,  162,  218,   21,
 /*  1710 */   218,  218,  167,  168,  218,  218,   28,  218,  173,  218,
 /*  1720 */   218,   33,   34,   35,   36,   37,   38,   39,  218,   41,
 /*  1730 */   218,  218,   44,   45,    1,    2,    3,    4,    5,  218,
 /*  1740 */   218,  218,   54,   55,   56,  218,  218,   59,   60,   61,
 /*  1750 */   218,  218,  218,   65,  218,  218,   68,   69,   70,   71,
 /*  1760 */    72,   73,   74,   75,   76,   77,   78,  218,  218,  218,
 /*  1770 */    82,   83,   84,  218,  218,   87,  218,  218,  218,   91,
 /*  1780 */    92,   93,   94,  218,   96,   52,   53,   99,   55,   56,
 /*  1790 */   218,  218,  218,  218,  106,   62,   63,  218,  110,  111,
 /*  1800 */   112,  113,  114,  115,  116,  117,  118,  119,    7,  218,
 /*  1810 */     9,   10,  218,  218,  218,  157,  218,  159,  218,  161,
 /*  1820 */   162,  218,   21,  218,  218,  167,  168,  218,  218,   28,
 /*  1830 */   218,  173,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  1840 */    39,  218,   41,  218,  218,   44,   45,    1,    2,    3,
 /*  1850 */     4,    5,  218,  218,  218,   54,   55,   56,  218,  218,
 /*  1860 */    59,   60,   61,  218,  218,  218,   65,  218,  218,   68,
 /*  1870 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*  1880 */   218,  218,  183,   82,   83,   84,  218,  218,   87,  218,
 /*  1890 */   218,  192,   91,   92,   93,   94,  218,   96,   52,   53,
 /*  1900 */    99,  202,  203,  204,  205,  218,   60,  106,   62,   63,
 /*  1910 */   176,  110,  111,  112,  113,  114,  115,  116,  117,  118,
 /*  1920 */   119,    7,  218,    9,   10,  218,  192,  218,  218,  218,
 /*  1930 */   218,  218,  218,  218,  218,   21,  202,  203,  204,  205,
 /*  1940 */   218,  218,   28,  218,  218,  218,  218,   33,   34,   35,
 /*  1950 */    36,   37,   38,   39,  218,   41,  218,  218,   44,   45,
 /*  1960 */     1,    2,    3,    4,    5,  218,  218,  218,   54,   55,
 /*  1970 */    56,  218,  218,   59,   60,   61,  218,  218,  218,   65,
 /*  1980 */   218,  218,   68,   69,   70,   71,   72,   73,   74,   75,
 /*  1990 */    76,   77,   78,  218,  218,  183,   82,   83,   84,  218,
 /*  2000 */   218,   87,  218,  218,  192,   91,   92,   93,   94,  218,
 /*  2010 */    96,   52,   53,   99,  202,  203,  204,  205,  218,   60,
 /*  2020 */   106,   62,   63,  218,  110,  111,  112,  113,  114,  115,
 /*  2030 */   116,  117,  118,  119,    7,  218,    9,   10,  218,  192,
 /*  2040 */   218,  218,  218,  218,  218,  218,  218,  218,   21,  202,
 /*  2050 */   203,  204,  205,  218,  218,   28,  218,  210,  218,  218,
 /*  2060 */    33,   34,   35,   36,   37,   38,   39,  218,   41,  218,
 /*  2070 */   218,   44,   45,    1,    2,    3,    4,    5,  218,  218,
 /*  2080 */   218,   54,   55,   56,  218,  218,   59,   60,   61,  218,
 /*  2090 */   218,  218,   65,  218,  218,   68,   69,   70,   71,   72,
 /*  2100 */    73,   74,   75,   76,   77,   78,  218,  218,  218,   82,
 /*  2110 */    83,   84,  218,  218,   87,  218,  218,  218,   91,   92,
 /*  2120 */    93,   94,  218,   96,   52,   53,   99,  218,  218,  218,
 /*  2130 */    58,  218,  218,  106,   62,   63,  218,  110,  111,  112,
 /*  2140 */   113,  114,  115,  116,  117,  118,  119,    7,  218,    9,
 /*  2150 */    10,  218,  192,  218,  218,  218,  196,  218,  218,  218,
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
 /*  2270 */   218,  218,  199,  218,   21,  202,  203,  204,  205,  218,
 /*  2280 */   218,   28,  218,  218,  218,  218,   33,   34,   35,   36,
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
 /*  2440 */    74,   75,   76,   77,   78,  218,  218,  218,   82,   83,
 /*  2450 */    84,  218,  218,   87,  218,  218,  192,   91,   92,   93,
 /*  2460 */    94,  218,   96,   52,   53,   99,  202,  203,  204,  205,
 /*  2470 */   218,  218,  106,   62,   63,  218,  110,  111,  112,  113,
 /*  2480 */   114,  115,  116,  117,  118,  119,    7,  218,    9,   10,
 /*  2490 */   218,  192,  218,  218,  218,  196,  218,  218,  218,  218,
 /*  2500 */    21,  202,  203,  204,  205,  218,  218,   28,  218,  218,
 /*  2510 */   218,  218,   33,   34,   35,   36,   37,   38,   39,  192,
 /*  2520 */    41,  218,  218,   44,   45,  218,  218,  218,  201,  202,
 /*  2530 */   203,  204,  205,   54,   55,   56,  218,  218,   59,   60,
 /*  2540 */    61,  218,  218,  218,   65,  218,  218,   68,   69,   70,
 /*  2550 */    71,   72,   73,   74,   75,   76,   77,   78,  218,  218,
 /*  2560 */   218,   82,   83,   84,  218,  218,   87,  218,  218,  218,
 /*  2570 */    91,   92,   93,   94,  218,   96,  218,  172,   99,  174,
 /*  2580 */   175,  176,  177,  178,  218,  106,  218,  218,  218,  110,
 /*  2590 */   111,  112,  113,  114,  115,  116,  117,  118,  119,    7,
 /*  2600 */   218,    9,   10,  218,  192,  218,  218,  218,  196,  218,
 /*  2610 */   218,  218,  218,   21,  202,  203,  204,  205,  218,  218,
 /*  2620 */    28,  218,  218,  218,  218,   33,   34,   35,   36,   37,
 /*  2630 */    38,   39,  218,   41,  218,  218,   44,   45,  172,  218,
 /*  2640 */   174,  175,  176,  177,  178,  218,   54,   55,   56,  218,
 /*  2650 */   218,   59,   60,   61,  218,  218,  218,   65,  218,  218,
 /*  2660 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  2670 */    78,  218,  218,  218,   82,   83,   84,  218,  218,   87,
 /*  2680 */   218,  218,  218,   91,   92,   93,   94,  218,   96,  218,
 /*  2690 */   172,   99,  174,  175,  176,  177,  178,  218,  106,  218,
 /*  2700 */   218,  218,  110,  111,  112,  113,  114,  115,  116,  117,
 /*  2710 */   118,  119,    7,  218,    9,   10,  218,  192,  218,  218,
 /*  2720 */   218,  196,  218,  218,  192,  218,   21,  202,  203,  204,
 /*  2730 */   205,  218,  218,   28,  202,  203,  204,  205,   33,   34,
 /*  2740 */    35,   36,   37,   38,   39,  213,   41,  218,  218,   44,
 /*  2750 */    45,  172,  218,  174,  175,  176,  177,  178,  218,   54,
 /*  2760 */    55,   56,  218,  218,   59,   60,   61,  218,  218,  218,
 /*  2770 */    65,  218,  218,   68,   69,   70,   71,   72,   73,   74,
 /*  2780 */    75,   76,   77,   78,  218,  218,  218,   82,   83,   84,
 /*  2790 */   218,  218,   87,  218,  218,  218,   91,   92,   93,   94,
 /*  2800 */   218,   96,  218,  172,   99,  174,  175,  176,  177,  178,
 /*  2810 */   218,  106,  218,  218,  218,  110,  111,  112,  113,  114,
 /*  2820 */   115,  116,  117,  118,  119,    7,  218,    9,   10,  218,
 /*  2830 */   192,  218,  218,  218,  196,  218,  218,  192,  218,   21,
 /*  2840 */   202,  203,  204,  205,  218,  218,   28,  202,  203,  204,
 /*  2850 */   205,   33,   34,   35,   36,   37,   38,   39,  218,   41,
 /*  2860 */   218,  218,   44,   45,  172,  218,  174,  175,  176,  177,
 /*  2870 */   178,  218,   54,   55,   56,  218,  218,   59,   60,   61,
 /*  2880 */   218,  218,  218,   65,  218,  218,   68,   69,   70,   71,
 /*  2890 */    72,   73,   74,   75,   76,   77,   78,  218,  218,  218,
 /*  2900 */    82,   83,   84,  218,  218,   87,  218,  218,  218,   91,
 /*  2910 */    92,   93,   94,  218,   96,  218,  172,   99,  174,  175,
 /*  2920 */   176,  177,  178,  218,  106,  218,  218,  218,  110,  111,
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
 /*  3960 */   192,  218,  218,  218,  218,  218,  218,  192,  218,   21,
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
 /*  4070 */     9,   10,  218,  192,  218,  218,  218,  218,  218,  218,
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
 /*  4370 */    83,   84,  218,  218,   87,  218,  218,  192,   91,   92,
 /*  4380 */    93,   94,  218,   96,  218,  218,   99,  202,  203,  204,
 /*  4390 */   205,  218,  218,  106,  218,  218,  218,  110,  111,  112,
 /*  4400 */   113,  114,  115,  116,  117,  118,  119,    7,  218,    9,
 /*  4410 */    10,  218,  192,  218,  218,  218,  218,  218,  218,  192,
 /*  4420 */   218,   21,  202,  203,  204,  205,  218,  218,   28,  202,
 /*  4430 */   203,  204,  205,   33,   34,   35,   36,   37,   38,   39,
 /*  4440 */   192,   41,  218,  218,   44,   45,  218,  218,  218,  218,
 /*  4450 */   202,  203,  204,  205,   54,   55,   56,  218,  218,   59,
 /*  4460 */    60,   61,  218,  218,  218,   65,  218,  218,   68,   69,
 /*  4470 */    70,   71,   72,   73,   74,   75,   76,   77,   78,  218,
 /*  4480 */   218,  218,   82,   83,   84,  218,  218,   87,  218,  218,
 /*  4490 */   192,   91,   92,   93,   94,  218,   96,  218,  218,   99,
 /*  4500 */   202,  203,  204,  205,  218,  218,  106,  218,  218,  218,
 /*  4510 */   110,  111,  112,  113,  114,  115,  116,  117,  118,  119,
 /*  4520 */     7,  159,    9,   10,  162,  218,  218,  218,  218,  167,
 /*  4530 */   168,  218,  218,  218,   21,  173,  218,  218,  218,  218,
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
 /*  5580 */   218,  218,  218,  218,   54,    0,  218,  218,  218,   59,
 /*  5590 */    60,   61,    7,  218,  218,   65,  218,  218,   68,   69,
 /*  5600 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*  5610 */    80,   81,  218,  218,  218,  218,  218,  218,  218,   34,
 /*  5620 */   218,  218,  218,  218,  218,  218,  218,  218,   43,   44,
 /*  5630 */   218,   46,  218,   48,  218,   50,  106,   52,   53,   54,
 /*  5640 */   218,   56,  218,  218,  114,  115,  116,  117,  118,  218,
 /*  5650 */   218,  218,  218,   68,   69,   70,   71,   72,   73,   74,
 /*  5660 */    75,   76,   77,   78,    7,  218,  218,   82,   83,   84,
 /*  5670 */   218,  218,   87,  218,  218,  218,   91,   92,   93,   94,
 /*  5680 */   218,   96,  218,  218,   99,  218,  218,  218,  218,  218,
 /*  5690 */   218,   34,  218,  218,  218,  110,  111,  112,  113,   41,
 /*  5700 */    43,   44,  218,   46,  119,   48,  218,   50,  218,   52,
 /*  5710 */    53,   54,  218,   56,  218,   57,  218,  218,  218,  218,
 /*  5720 */   218,  218,  218,  218,  218,   68,   69,   70,   71,   72,
 /*  5730 */    73,   74,   75,   76,   77,   78,  218,  218,  218,   82,
 /*  5740 */    83,   84,  218,  218,   87,  218,  218,  218,   91,   92,
 /*  5750 */    93,   94,  218,   96,  218,  218,   99,  218,  100,  101,
 /*  5760 */   102,  103,  104,  105,  218,  218,  218,  110,  111,  112,
 /*  5770 */   113,  218,  218,  218,  218,  218,  119,  121,  122,  123,
 /*  5780 */   124,  125,  126,  127,  128,  129,  130,  131,  132,  133,
 /*  5790 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  5800 */   144,  145,  146,  147,  148,  149,  218,  218,  218,  218,
 /*  5810 */   218,  129,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  5820 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  5830 */   148,  149,   19,   20,   21,   22,   23,   24,   25,   26,
 /*  5840 */    27,   28,   29,   30,   31,   32,  218,  218,  218,  218,
 /*  5850 */   218,  218,  218,  171,   41,   42,  192,  218,  218,  203,
 /*  5860 */   204,  205,  180,  181,  182,  218,  202,  203,  204,  205,
 /*  5870 */   192,  218,  208,  218,  192,  211,  212,  213,  218,  218,
 /*  5880 */   202,  203,  204,  205,  202,  203,  204,  205,  218,  211,
 /*  5890 */   212,  213,  218,  218,  218,  218,  218,  129,  130,  131,
 /*  5900 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  5910 */   142,  143,  144,  145,  146,  147,  148,  149,   13,   14,
 /*  5920 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*  5930 */    25,   26,   27,   28,   29,   30,   31,   32,  218,  171,
 /*  5940 */   218,  218,  218,  218,  218,  218,   41,   42,  180,  181,
 /*  5950 */   182,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  5960 */   192,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  5970 */   202,  203,  204,  205,  218,  218,  218,  218,  218,  218,
 /*  5980 */   218,  218,  218,  129,  130,  131,  132,  133,  134,  135,
 /*  5990 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  6000 */   146,  147,  148,  149,   14,   15,   16,   17,   18,   19,
 /*  6010 */    20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
 /*  6020 */    30,   31,   32,  218,  218,  171,  218,  218,  218,  218,
 /*  6030 */   218,   41,   42,  218,  180,  181,  182,  218,  218,  218,
 /*  6040 */   218,  218,  218,   21,  218,   41,  192,  218,  218,  218,
 /*  6050 */   218,  218,  218,  218,  218,  218,  202,  203,  204,  205,
 /*  6060 */   218,   57,  218,  218,  218,  218,   44,  218,  218,  129,
 /*  6070 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  6080 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  6090 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*  6100 */    78,   79,   80,   81,  100,  101,  102,  103,  104,  105,
 /*  6110 */   218,  171,  108,  109,  218,  218,  218,   41,   42,  218,
 /*  6120 */   180,  181,  182,  218,  218,  218,  218,  218,   41,  218,
 /*  6130 */   218,   44,  192,   57,  218,  218,  218,  218,  218,  218,
 /*  6140 */   218,  218,  202,  203,  204,  205,   59,  218,  218,  218,
 /*  6150 */   218,  218,   65,  218,   67,  129,  130,  131,  132,  133,
 /*  6160 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  6170 */   144,  145,  146,  147,  148,  149,  100,  101,  102,  103,
 /*  6180 */   104,  105,  218,  107,  108,  109,  218,  218,  218,  218,
 /*  6190 */   218,  218,  218,  106,  218,  218,  218,  171,  218,  218,
 /*  6200 */   218,  114,  115,  116,  117,  118,  180,  181,  182,   41,
 /*  6210 */   218,  218,  218,  218,   41,  218,  218,   44,  192,  218,
 /*  6220 */   218,  218,  218,  218,  218,   57,  218,  218,  202,  203,
 /*  6230 */   204,  205,   59,  218,  218,  218,  218,  218,   65,  218,
 /*  6240 */   218,  129,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  6250 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  6260 */   148,  149,  218,  218,  218,  218,  218,  218,  100,  101,
 /*  6270 */   102,  103,  104,  105,  218,  218,  218,  218,  218,  106,
 /*  6280 */   218,  218,  218,  171,  218,  218,  218,  114,  115,  116,
 /*  6290 */   117,  118,  180,  181,  182,   41,  218,  218,  218,  218,
 /*  6300 */    41,  218,  218,   44,  192,  218,  218,  218,  218,  218,
 /*  6310 */   218,   57,  218,  218,  202,  203,  204,  205,   59,  218,
 /*  6320 */   218,  218,  218,  218,   65,  218,  218,  129,  130,  131,
 /*  6330 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  6340 */   142,  143,  144,  145,  146,  147,  148,  149,  218,  218,
 /*  6350 */   218,  218,  218,  218,  100,  101,  102,  103,  104,  105,
 /*  6360 */   218,  218,  218,  218,  218,  106,  218,  218,  218,  171,
 /*  6370 */   218,  218,  218,  114,  115,  116,  117,  118,  180,  181,
 /*  6380 */   182,   41,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6390 */   192,  218,  218,  218,  218,  218,  218,   57,  218,  218,
 /*  6400 */   202,  203,  204,  205,  218,  218,  218,  218,  218,  218,
 /*  6410 */   218,  218,  218,  129,  130,  131,  132,  133,  134,  135,
 /*  6420 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  6430 */   146,  147,  148,  149,  218,  218,  218,  218,  218,  218,
 /*  6440 */   100,  101,  102,  103,  104,  105,  218,  218,  218,  218,
 /*  6450 */   218,  218,  218,  218,  218,  171,  218,  218,  218,  218,
 /*  6460 */   218,  218,  218,  218,  180,  181,  182,  218,  218,  218,
 /*  6470 */   218,  218,  218,  218,  218,  218,  192,  218,  218,  218,
 /*  6480 */   218,  218,  218,  218,  218,  218,  202,  203,  204,  205,
 /*  6490 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  129,
 /*  6500 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  6510 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  6520 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6530 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6540 */   218,  171,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6550 */   180,  181,  182,  218,  218,  218,  218,  218,  218,  218,
 /*  6560 */   218,  218,  192,  218,  218,  218,  218,  218,  218,  218,
 /*  6570 */   218,  218,  202,  203,  204,  205,  218,  218,  218,  218,
 /*  6580 */   218,  218,  218,  218,  218,  129,  130,  131,  132,  133,
 /*  6590 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  6600 */   144,  145,  146,  147,  148,  149,  218,  218,  218,  218,
 /*  6610 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6620 */   218,  218,  218,  218,  218,  218,  218,  171,  218,  218,
 /*  6630 */   218,  218,  218,  218,  218,  218,  180,  181,  182,  218,
 /*  6640 */   218,  218,  218,  218,  218,  218,  218,  218,  192,  218,
 /*  6650 */   218,  218,  218,  218,  218,  218,  218,  218,  202,  203,
 /*  6660 */   204,  205,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6670 */   218,  129,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  6680 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  6690 */   148,  149,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6700 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6710 */   218,  218,  218,  171,  218,  218,  218,  218,  218,  218,
 /*  6720 */   218,  218,  180,  181,  182,  218,  218,  218,  218,  218,
 /*  6730 */   218,  218,  218,  218,  192,  218,  218,  218,  218,  218,
 /*  6740 */   218,  218,  218,  218,  202,  203,  204,  205,  218,  218,
 /*  6750 */   218,  218,  218,  218,  218,  218,  218,  129,  130,  131,
 /*  6760 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  6770 */   142,  143,  144,  145,  146,  147,  148,  149,  218,  218,
 /*  6780 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6790 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  171,
 /*  6800 */   218,  218,  218,  218,  218,  218,  218,  218,  180,  181,
 /*  6810 */   182,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6820 */   192,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6830 */   202,  203,  204,  205,  218,  218,  218,  218,  218,  218,
 /*  6840 */   218,  218,  218,  129,  130,  131,  132,  133,  134,  135,
 /*  6850 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  6860 */   146,  147,  148,  149,  218,  218,  218,  218,  218,  218,
 /*  6870 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6880 */   218,  218,  218,  218,  218,  171,  218,  218,  218,  218,
 /*  6890 */   218,  218,  218,  218,  180,  181,  182,  218,  218,  218,
 /*  6900 */   218,  218,  218,  218,  218,  218,  192,  218,  218,  218,
 /*  6910 */   218,  218,  218,  218,  218,  218,  202,  203,  204,  205,
 /*  6920 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  129,
 /*  6930 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  6940 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  6950 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6960 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6970 */   218,  171,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  6980 */   180,  181,  182,  218,  218,  218,  218,  218,  218,  218,
 /*  6990 */   218,  218,  192,  218,  218,  218,  218,  218,  218,  218,
 /*  7000 */   218,  218,  202,  203,  204,  205,  218,  218,  218,  218,
 /*  7010 */   218,  218,  218,  218,  218,  129,  130,  131,  132,  133,
 /*  7020 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  7030 */   144,  145,  146,  147,  148,  149,  218,  218,  218,  218,
 /*  7040 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7050 */   218,  218,  218,  218,  218,  218,  218,  171,  218,  218,
 /*  7060 */   218,  218,  218,  218,  218,  218,  180,  181,  182,  218,
 /*  7070 */   218,  218,  218,  218,  218,  218,  218,  218,  192,  218,
 /*  7080 */   218,  218,  218,  218,  218,  218,  218,  218,  202,  203,
 /*  7090 */   204,  205,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7100 */   218,  129,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  7110 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  7120 */   148,  149,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7130 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7140 */   218,  218,  218,  171,  218,  218,  218,  218,  218,  218,
 /*  7150 */   218,  218,  180,  181,  182,  218,  218,  218,  218,  218,
 /*  7160 */   218,  218,  218,  218,  192,  218,  218,  218,  218,  218,
 /*  7170 */   218,  218,  218,  218,  202,  203,  204,  205,  218,  218,
 /*  7180 */   218,  218,  218,  218,  218,  218,  218,  129,  130,  131,
 /*  7190 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  7200 */   142,  143,  144,  145,  146,  147,  148,  149,  218,  218,
 /*  7210 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7220 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  171,
 /*  7230 */   218,  218,  218,  218,  218,  218,  218,  218,  180,  181,
 /*  7240 */   182,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7250 */   192,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7260 */   202,  203,  204,  205,  218,  218,  218,  218,  218,  218,
 /*  7270 */   218,  218,  218,  129,  130,  131,  132,  133,  134,  135,
 /*  7280 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  7290 */   146,  147,  148,  149,  218,  218,  218,  218,  218,  218,
 /*  7300 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7310 */   218,  218,  218,  218,  218,  171,  218,  218,  218,  218,
 /*  7320 */   218,  218,  218,  218,  180,  181,  182,  218,  218,  218,
 /*  7330 */   218,  218,  218,  218,  218,  218,  192,  218,  218,  218,
 /*  7340 */   218,  218,  218,  218,  218,  218,  202,  203,  204,  205,
 /*  7350 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  129,
 /*  7360 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  7370 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  7380 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7390 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7400 */   218,  171,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7410 */   180,  181,  182,  218,  218,  218,  218,  218,  218,  218,
 /*  7420 */   218,  218,  192,  218,  218,  218,  218,  218,  218,  218,
 /*  7430 */   218,  218,  202,  203,  204,  205,  218,  218,  218,  218,
 /*  7440 */   218,  218,  218,  218,  218,  129,  130,  131,  132,  133,
 /*  7450 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  7460 */   144,  145,  146,  147,  148,  149,  218,  218,  218,  218,
 /*  7470 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7480 */   218,  218,  218,  218,  218,  218,  218,  171,  218,  218,
 /*  7490 */   218,  218,  218,  218,  218,  218,  180,  181,  182,  218,
 /*  7500 */   218,  218,  218,  218,  218,  218,  218,  218,  192,  218,
 /*  7510 */   218,  218,  218,  218,  218,  218,  218,  218,  202,  203,
 /*  7520 */   204,  205,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7530 */   218,  129,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  7540 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  7550 */   148,  149,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7560 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7570 */   218,  218,  218,  171,  218,  218,  218,  218,  218,  218,
 /*  7580 */   218,  218,  180,  181,  182,  218,  218,  218,  218,  218,
 /*  7590 */   218,  218,  218,  218,  192,  218,  218,  218,  218,  218,
 /*  7600 */   218,  218,  218,  218,  202,  203,  204,  205,  218,  218,
 /*  7610 */   218,  218,  218,  218,  218,  218,  218,  129,  130,  131,
 /*  7620 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  7630 */   142,  143,  144,  145,  146,  147,  148,  149,  218,  218,
 /*  7640 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7650 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  171,
 /*  7660 */   218,  218,  218,  218,  218,  218,  218,  218,  180,  181,
 /*  7670 */   182,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7680 */   192,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7690 */   202,  203,  204,  205,  218,  218,  218,  218,  218,  218,
 /*  7700 */   218,  218,  218,  129,  130,  131,  132,  133,  134,  135,
 /*  7710 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  7720 */   146,  147,  148,  149,  218,  218,  218,  218,  218,  218,
 /*  7730 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7740 */   218,  218,  218,  218,  218,  171,  218,  218,  218,  218,
 /*  7750 */   218,  218,  218,  218,  180,  181,  182,  218,  218,  218,
 /*  7760 */   218,  218,  218,  218,  218,  218,  192,  218,  218,  218,
 /*  7770 */   218,  218,  218,  218,  218,  218,  202,  203,  204,  205,
 /*  7780 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  129,
 /*  7790 */   130,  131,  132,  133,  134,  135,  136,  137,  138,  139,
 /*  7800 */   140,  141,  142,  143,  144,  145,  146,  147,  148,  149,
 /*  7810 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7820 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7830 */   218,  171,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7840 */   180,  181,  182,  218,  218,  218,  218,  218,  218,  218,
 /*  7850 */   218,  218,  192,  218,  218,  218,  218,  218,  218,  218,
 /*  7860 */   218,  218,  202,  203,  204,  205,  218,  218,  218,  218,
 /*  7870 */   218,  218,  218,  218,  218,  129,  130,  131,  132,  133,
 /*  7880 */   134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
 /*  7890 */   144,  145,  146,  147,  148,  149,  218,  218,  218,  218,
 /*  7900 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7910 */   218,  218,  218,  218,  218,  218,  218,  171,  218,  218,
 /*  7920 */   218,  218,  218,  218,  218,  218,  180,  181,  182,  218,
 /*  7930 */   218,  218,  218,  218,  218,  218,  218,  218,  192,  218,
 /*  7940 */   218,  218,  218,  218,  218,  218,  218,  218,  202,  203,
 /*  7950 */   204,  205,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7960 */   218,  129,  130,  131,  132,  133,  134,  135,  136,  137,
 /*  7970 */   138,  139,  140,  141,  142,  143,  144,  145,  146,  147,
 /*  7980 */   148,  149,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  7990 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8000 */   218,  218,  218,  171,  218,  218,  218,  218,  218,  218,
 /*  8010 */   218,  218,  180,  181,  182,  218,  218,  218,  218,  218,
 /*  8020 */   218,  218,  218,  218,  192,  218,  218,  218,  218,  218,
 /*  8030 */   218,  218,  218,  218,  202,  203,  204,  205,  218,  218,
 /*  8040 */   218,  218,  218,  218,  218,  218,  218,  129,  130,  131,
 /*  8050 */   132,  133,  134,  135,  136,  137,  138,  139,  140,  141,
 /*  8060 */   142,  143,  144,  145,  146,  147,  148,  149,  218,  218,
 /*  8070 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8080 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  171,
 /*  8090 */   218,  218,  218,  218,  218,  218,  218,  218,  180,  181,
 /*  8100 */   182,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8110 */   192,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8120 */   202,  203,  204,  205,  218,  218,  218,  218,  124,  125,
 /*  8130 */   126,  127,  128,  129,  130,  131,  132,  133,  134,  135,
 /*  8140 */   136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
 /*  8150 */   146,  147,  148,  149,  218,  218,  218,  218,  218,  218,
 /*  8160 */   218,  218,  218,  218,  129,  130,  131,  132,  133,  134,
 /*  8170 */   135,  136,  137,  138,  139,  140,  141,  142,  143,  144,
 /*  8180 */   145,  146,  147,  148,  149,  218,  218,  218,  218,  218,
 /*  8190 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8200 */   218,  218,  218,  218,  218,  218,  218,  203,  204,  205,
 /*  8210 */   218,  218,  218,  218,  218,  180,  181,  182,  218,  218,
 /*  8220 */   218,  218,  218,  218,  218,  218,  218,  192,  218,  218,
 /*  8230 */   218,  218,  218,  218,  218,  218,  218,  202,  203,  204,
 /*  8240 */   205,  218,  218,  218,  218,    7,  218,    9,   10,  218,
 /*  8250 */   218,  218,  218,   15,  218,  218,  218,  218,  218,   21,
 /*  8260 */   218,  218,  218,  218,  218,  218,   28,  218,  218,  218,
 /*  8270 */   218,   33,   34,   35,   36,   37,   38,   39,   40,   41,
 /*  8280 */   218,    7,   44,    9,   10,  218,  218,  218,  218,   15,
 /*  8290 */   218,  218,   54,  218,  218,   21,  218,   59,   60,   61,
 /*  8300 */   218,  218,   28,   65,  218,  218,  218,   33,   34,   35,
 /*  8310 */    36,   37,   38,   39,   40,   41,  218,  218,   44,  218,
 /*  8320 */   218,  218,  218,  218,  218,  218,  218,  218,   54,  218,
 /*  8330 */   218,  218,  218,   59,   60,   61,  218,  218,  218,   65,
 /*  8340 */   218,  218,  218,  218,  106,  218,  218,  218,  218,  218,
 /*  8350 */   218,  218,  114,  115,  116,  117,  118,   12,   13,   14,
 /*  8360 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*  8370 */    25,   26,   27,   28,   29,   30,   31,   32,  218,  218,
 /*  8380 */   106,  218,  218,  218,  218,  218,   41,   42,  114,  115,
 /*  8390 */   116,  117,  118,  218,  218,  218,    7,  218,    9,   10,
 /*  8400 */   218,  218,  218,  218,   15,  218,  218,  218,  218,  218,
 /*  8410 */    21,  218,  218,  218,  218,  218,  218,   28,  218,  218,
 /*  8420 */   218,  218,   33,   34,   35,   36,   37,   38,   39,   40,
 /*  8430 */    41,  218,    7,   44,    9,   10,  218,  218,  218,  218,
 /*  8440 */    15,  218,  218,   54,  218,  218,   21,  218,   59,   60,
 /*  8450 */    61,  218,  218,   28,   65,  218,  218,  218,   33,   34,
 /*  8460 */    35,   36,   37,   38,   39,   40,   41,  218,   21,   44,
 /*  8470 */   218,  218,  218,  218,  218,  218,  218,  218,  218,   54,
 /*  8480 */   218,  218,  218,  218,   59,   60,   61,  218,  218,  218,
 /*  8490 */    65,  218,  218,  218,  218,  106,  218,  218,  218,  218,
 /*  8500 */   218,  218,  218,  114,  115,  116,  117,  118,  218,  218,
 /*  8510 */   218,   64,   65,   66,  218,   68,   69,   70,   71,   72,
 /*  8520 */    73,   74,   75,   76,   77,   78,   79,   80,   81,  218,
 /*  8530 */   218,  106,  218,  218,  218,  218,  218,  218,  218,  114,
 /*  8540 */   115,  116,  117,  118,  218,  218,  218,    7,  218,    9,
 /*  8550 */    10,  218,  218,  218,  218,   15,  218,  218,  218,  218,
 /*  8560 */   218,   21,  218,  218,  218,  218,  218,  218,   28,  218,
 /*  8570 */   218,  218,  218,   33,   34,   35,   36,   37,   38,   39,
 /*  8580 */    40,   41,  218,    7,   44,    9,   10,  218,  218,  218,
 /*  8590 */   218,   15,  218,  218,   54,  218,  218,   21,  218,   59,
 /*  8600 */    60,   61,  218,  218,   28,   65,  218,  218,  218,   33,
 /*  8610 */    34,   35,   36,   37,   38,   39,   40,   41,  218,   21,
 /*  8620 */    44,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8630 */    54,  218,  218,  218,  218,   59,   60,   61,  218,  218,
 /*  8640 */   218,   65,  218,  218,  218,  218,  106,  218,  218,  218,
 /*  8650 */   218,  218,  218,  218,  114,  115,  116,  117,  118,  218,
 /*  8660 */   218,  218,  218,   65,   66,  218,   68,   69,   70,   71,
 /*  8670 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*  8680 */   218,  218,  106,  218,  218,  218,  218,  218,  218,  218,
 /*  8690 */   114,  115,  116,  117,  118,  218,  218,  218,    7,  218,
 /*  8700 */     9,   10,  218,  218,  218,  218,   15,  218,  218,  218,
 /*  8710 */   218,  218,   21,  218,  218,  218,  218,  218,  218,   28,
 /*  8720 */   218,  218,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  8730 */    39,   40,   41,  218,    7,   44,    9,   10,  218,  218,
 /*  8740 */   218,  218,   15,  218,  218,   54,  218,  218,   21,  218,
 /*  8750 */    59,   60,   61,  218,  218,   28,   65,  218,  218,  218,
 /*  8760 */    33,   34,   35,   36,   37,   38,   39,   40,   41,  218,
 /*  8770 */   218,   44,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8780 */   218,   54,  218,  218,  218,  218,   59,   60,   61,  218,
 /*  8790 */   218,  218,   65,  218,  218,  218,  218,  106,  218,  218,
 /*  8800 */   218,  218,  218,  218,  218,  114,  115,  116,  117,  118,
 /*  8810 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8820 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8830 */   218,  218,  218,  106,  218,  218,  218,  218,  218,  218,
 /*  8840 */   218,  114,  115,  116,  117,  118,  218,  218,  218,    7,
 /*  8850 */   218,    9,   10,  218,  218,  218,  218,   15,  218,  218,
 /*  8860 */   218,  218,  218,   21,  218,  218,  218,  218,  218,  218,
 /*  8870 */    28,  218,  218,  218,  218,   33,   34,   35,   36,   37,
 /*  8880 */    38,   39,   40,   41,  218,    7,   44,    9,   10,  218,
 /*  8890 */   218,  218,  218,   15,  218,  218,   54,  218,  218,   21,
 /*  8900 */   218,   59,   60,   61,  218,  218,   28,   65,  218,  218,
 /*  8910 */   218,   33,   34,   35,   36,   37,   38,   39,   40,   41,
 /*  8920 */   218,  218,   44,  218,  218,  218,  218,  218,  218,  218,
 /*  8930 */   218,  218,   54,  218,  218,  218,  218,   59,   60,   61,
 /*  8940 */   218,  218,  218,   65,  218,  218,  218,  218,  106,  218,
 /*  8950 */   218,  218,  218,  218,  218,  218,  114,  115,  116,  117,
 /*  8960 */   118,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8970 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  8980 */   218,  218,  218,  218,  106,  218,  218,  218,  218,  218,
 /*  8990 */   218,  218,  114,  115,  116,  117,  118,  218,  218,  218,
 /*  9000 */   218,    8,  218,  218,   11,   12,   13,   14,   15,   16,
 /*  9010 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /*  9020 */    27,   28,   29,   30,   31,   32,  218,  218,  218,  218,
 /*  9030 */   218,  218,  218,    8,   41,   42,   11,   12,   13,   14,
 /*  9040 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*  9050 */    25,   26,   27,   28,   29,   30,   31,   32,  218,  218,
 /*  9060 */    67,  218,  218,  218,  218,    8,   41,   42,   11,   12,
 /*  9070 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*  9080 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /*  9090 */   218,  218,    7,  218,    9,   10,  218,   40,   41,   42,
 /*  9100 */    15,  218,  218,  218,  218,  218,   21,  218,  218,  218,
 /*  9110 */   218,  218,  218,   28,   89,  218,  218,  218,   33,   34,
 /*  9120 */    35,   36,   37,   38,   39,  218,   41,  218,    7,   44,
 /*  9130 */     9,   10,  218,  218,  218,  218,  218,  218,  218,   54,
 /*  9140 */   218,  218,   21,  218,   59,   60,   61,  218,  218,   28,
 /*  9150 */    65,  218,  218,  218,   33,   34,   35,   36,   37,   38,
 /*  9160 */    39,   21,   41,  218,  218,   44,  218,  218,  218,  218,
 /*  9170 */   218,  218,  218,  218,  218,   54,  218,  218,  218,  218,
 /*  9180 */    59,   60,   61,  218,   44,  218,   65,  218,   67,  218,
 /*  9190 */   218,  106,  218,  218,  218,  218,  218,  218,   58,  114,
 /*  9200 */   115,  116,  117,  118,  218,  218,  218,  218,   68,   69,
 /*  9210 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*  9220 */    80,   81,  218,  218,  218,  218,  218,  106,  218,  218,
 /*  9230 */   218,  218,  218,  218,  218,  114,  115,  116,  117,  118,
 /*  9240 */   218,  218,  218,    7,  218,    9,   10,  218,  218,  218,
 /*  9250 */   218,   15,  218,  218,  218,  218,  218,   21,  218,  218,
 /*  9260 */   218,  218,  218,  218,   28,  218,  218,  218,  218,   33,
 /*  9270 */    34,   35,   36,   37,   38,   39,  218,   41,  218,    7,
 /*  9280 */    44,    9,   10,  218,  218,  218,  218,  218,  218,  218,
 /*  9290 */    54,  218,  218,   21,  218,   59,   60,   61,  218,  218,
 /*  9300 */    28,   65,  218,  218,  218,   33,   34,   35,   36,   37,
 /*  9310 */    38,   39,  218,   41,  218,  218,   44,  218,  218,  218,
 /*  9320 */   218,  218,  218,  218,  218,  218,   54,  218,  218,  218,
 /*  9330 */   218,   59,   60,   61,  218,  218,  218,   65,  218,  218,
 /*  9340 */   218,  218,  106,  218,  218,  218,  218,  218,  218,  218,
 /*  9350 */   114,  115,  116,  117,  118,  218,  218,  218,  218,  218,
 /*  9360 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  9370 */    98,  218,  218,  218,  218,  218,  218,  218,  106,  218,
 /*  9380 */   218,  218,  218,  218,  218,  218,  114,  115,  116,  117,
 /*  9390 */   118,  218,  218,  218,  218,    8,  218,  218,   11,   12,
 /*  9400 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*  9410 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /*  9420 */   218,    7,  218,    9,   10,  218,  218,  218,   41,   42,
 /*  9430 */   218,  218,  218,  218,  218,   21,  218,  218,  218,  218,
 /*  9440 */   218,   54,   28,  218,  218,  218,  218,   33,   34,   35,
 /*  9450 */    36,   37,   38,   39,  218,   41,  218,  218,   44,  218,
 /*  9460 */   218,  218,  218,  218,  218,  218,  218,  218,   54,  218,
 /*  9470 */   218,  218,  218,   59,   60,   61,  218,  218,  218,   65,
 /*  9480 */   218,   67,    8,  218,  218,   11,   12,   13,   14,   15,
 /*  9490 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*  9500 */    26,   27,   28,   29,   30,   31,   32,  218,  218,  218,
 /*  9510 */   218,  218,  218,  218,  218,   41,   42,  218,  218,  218,
 /*  9520 */   106,  218,  218,    7,  218,    9,   10,  218,  114,  115,
 /*  9530 */   116,  117,  118,  218,  218,  218,  218,   21,  218,  218,
 /*  9540 */   218,   67,  218,  218,   28,  218,  218,  218,  218,   33,
 /*  9550 */    34,   35,   36,   37,   38,   39,  218,   41,  218,    7,
 /*  9560 */    44,    9,   10,  218,  218,  218,  218,  218,  218,  218,
 /*  9570 */    54,  218,  218,   21,  218,   59,   60,   61,  218,  218,
 /*  9580 */    28,   65,  218,   67,  218,   33,   34,   35,   36,   37,
 /*  9590 */    38,   39,  218,   41,  218,  218,   44,  218,  218,  218,
 /*  9600 */   218,  218,  218,  218,  218,  218,   54,  218,  218,  218,
 /*  9610 */   218,   59,   60,   61,  218,  218,  218,   65,  218,   67,
 /*  9620 */   218,  218,  106,  218,  218,    7,  218,    9,   10,  218,
 /*  9630 */   114,  115,  116,  117,  118,  218,  218,  218,  218,   21,
 /*  9640 */   218,  218,  218,  218,  218,  218,   28,  218,  218,  218,
 /*  9650 */   218,   33,   34,   35,   36,   37,   38,   39,  106,   41,
 /*  9660 */   218,    7,   44,    9,   10,  218,  114,  115,  116,  117,
 /*  9670 */   118,  218,   54,  218,  218,   21,  218,   59,   60,   61,
 /*  9680 */   218,  218,   28,   65,  218,   67,  218,   33,   34,   35,
 /*  9690 */    36,   37,   38,   39,  218,   41,  218,  218,   44,  218,
 /*  9700 */   218,  218,  218,  218,  218,  218,  218,  218,   54,  218,
 /*  9710 */   218,  218,  218,   59,   60,   61,  218,  218,  218,   65,
 /*  9720 */   218,   67,  218,  218,  106,  218,  218,    7,  218,    9,
 /*  9730 */    10,  218,  114,  115,  116,  117,  118,  218,  218,  218,
 /*  9740 */   218,   21,  218,  218,  218,  218,  218,  218,   28,  218,
 /*  9750 */   218,  218,  218,   33,   34,   35,   36,   37,   38,   39,
 /*  9760 */   106,   41,  218,    7,   44,    9,   10,  218,  114,  115,
 /*  9770 */   116,  117,  118,  218,   54,  218,  218,   21,  218,   59,
 /*  9780 */    60,   61,  218,  218,   28,   65,  218,   67,  218,   33,
 /*  9790 */    34,   35,   36,   37,   38,   39,  218,   41,  218,  218,
 /*  9800 */    44,   45,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  9810 */    54,  218,  218,  218,  218,   59,   60,   61,  218,  218,
 /*  9820 */   218,   65,  218,  218,  218,  218,  106,  218,  218,  218,
 /*  9830 */   218,  218,  218,  218,  114,  115,  116,  117,  118,  218,
 /*  9840 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  9850 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /*  9860 */   218,  218,  106,  218,  218,  218,  218,  218,  218,  218,
 /*  9870 */   114,  115,  116,  117,  118,  218,  218,  218,  218,    8,
 /*  9880 */   218,  218,   11,   12,   13,   14,   15,   16,   17,   18,
 /*  9890 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /*  9900 */    29,   30,   31,   32,  218,  218,  218,  218,  218,  218,
 /*  9910 */   218,  218,   41,   42,  218,  218,   45,  218,  218,  218,
 /*  9920 */   218,    8,  218,  218,   11,   12,   13,   14,   15,   16,
 /*  9930 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /*  9940 */    27,   28,   29,   30,   31,   32,  218,  218,  218,  218,
 /*  9950 */   218,  218,  218,  218,   41,   42,  218,  218,   45,  218,
 /*  9960 */   218,  218,  218,    8,  218,  218,   11,   12,   13,   14,
 /*  9970 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*  9980 */    25,   26,   27,   28,   29,   30,   31,   32,  218,  218,
 /*  9990 */   218,  218,  218,  218,  218,  218,   41,   42,  218,  218,
 /* 10000 */    45,  218,  218,  218,  218,    8,  218,  218,   11,   12,
 /* 10010 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /* 10020 */    23,   24,   25,   26,   27,   28,   29,   30,   31,   32,
 /* 10030 */   218,  218,  218,  218,  218,  218,  218,  218,   41,   42,
 /* 10040 */   218,  218,   45,  218,  218,  218,  218,    8,  218,  218,
 /* 10050 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /* 10060 */    21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
 /* 10070 */    31,   32,  218,    7,  218,    9,   10,  218,  218,  218,
 /* 10080 */    41,   42,  218,  218,  218,  218,  218,   21,  218,  218,
 /* 10090 */   218,  218,  218,   54,   28,  218,  218,  218,  218,   33,
 /* 10100 */    34,   35,   36,   37,   38,   39,  218,   41,  218,  218,
 /* 10110 */    44,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /* 10120 */    54,  218,  218,  218,  218,   59,   60,   61,  218,    8,
 /* 10130 */   218,   65,   11,   12,   13,   14,   15,   16,   17,   18,
 /* 10140 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /* 10150 */    29,   30,   31,   32,  218,  218,  218,  218,  218,  218,
 /* 10160 */   218,  218,   41,   42,   98,  218,  218,  218,  218,  218,
 /* 10170 */   218,  218,  106,  218,  218,   54,  218,  218,  218,  218,
 /* 10180 */   114,  115,  116,  117,  118,  218,  218,  218,  218,    8,
 /* 10190 */   218,  218,   11,   12,   13,   14,   15,   16,   17,   18,
 /* 10200 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /* 10210 */    29,   30,   31,   32,  218,    7,  218,    9,   10,  218,
 /* 10220 */   218,  218,   41,   42,  218,  218,  218,  218,  218,   21,
 /* 10230 */   218,  218,  218,  218,  218,   54,   28,  218,  218,  218,
 /* 10240 */   218,   33,   34,   35,   36,   37,   38,   39,  218,   41,
 /* 10250 */   218,  218,   44,  218,  218,  218,  218,  218,  218,  218,
 /* 10260 */   218,  218,   54,  218,  218,  218,  218,   59,   60,   61,
 /* 10270 */   218,    8,  218,   65,   11,   12,   13,   14,   15,   16,
 /* 10280 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /* 10290 */    27,   28,   29,   30,   31,   32,  218,  218,  218,  218,
 /* 10300 */   218,  218,  218,  218,   41,   42,  218,  218,  218,  218,
 /* 10310 */   218,  218,  218,    7,  106,    9,   10,  218,  218,  218,
 /* 10320 */   218,  218,  114,  115,  116,  117,  118,   21,  218,  218,
 /* 10330 */   218,  218,  218,  218,   28,  218,  218,  218,  218,   33,
 /* 10340 */    34,   35,   36,   37,   38,   39,  218,   41,  218,  218,
 /* 10350 */    44,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /* 10360 */    54,  218,  218,  218,  218,   59,   60,   61,  218,  218,
 /* 10370 */   218,   65,   11,   12,   13,   14,   15,   16,   17,   18,
 /* 10380 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /* 10390 */    29,   30,   31,   32,   21,  218,  218,  218,  218,  218,
 /* 10400 */   218,  218,   41,   42,  218,  218,  218,  218,  218,  218,
 /* 10410 */   218,  218,  106,   40,  218,  218,  218,   44,  218,  218,
 /* 10420 */   114,  115,  116,  117,  118,  218,  218,  218,  218,  218,
 /* 10430 */   218,   58,  218,  218,  218,  218,  218,  218,  218,   21,
 /* 10440 */   218,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /* 10450 */    77,   78,   79,   80,   81,  218,  218,  218,   40,  218,
 /* 10460 */   218,  218,   44,  218,  218,  218,  218,  218,  218,  218,
 /* 10470 */   218,  218,  218,  218,  218,  218,   58,  218,  218,  218,
 /* 10480 */   218,  218,  218,  218,   21,  218,   68,   69,   70,   71,
 /* 10490 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /* 10500 */   218,  218,  218,   40,  218,  218,  218,   44,  218,  218,
 /* 10510 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /* 10520 */   218,   58,  218,  218,  218,  218,  218,  218,  218,   21,
 /* 10530 */   218,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /* 10540 */    77,   78,   79,   80,   81,  218,  218,  218,   40,  218,
 /* 10550 */   218,  218,   44,  218,  218,  218,  218,  218,  218,  218,
 /* 10560 */   218,  218,  218,  218,  218,  218,   58,  218,  218,  218,
 /* 10570 */   218,  218,  218,  218,   21,  218,   68,   69,   70,   71,
 /* 10580 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /* 10590 */   218,  218,  218,   40,  218,  218,  218,   44,  218,  218,
 /* 10600 */   218,  218,  218,  218,  218,  218,  218,  218,  218,  218,
 /* 10610 */   218,   58,  218,  218,  218,  218,  218,  218,  218,  218,
 /* 10620 */   218,   68,   69,   70,   71,   72,   73,   74,   75,   76,
 /* 10630 */    77,   78,   79,   80,   81,
};
#define YY_SHIFT_USE_DFLT (-57)
static short yy_shift_ofst[] = {
 /*     0 */  5657,   46, 5585,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*    10 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*    20 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,   53,
 /*    30 */    37,  -57,  263,    0,  -57,  263,  -57,  274,  433,  -57,
 /*    40 */   -57,   41, 8238, 10208,  -26, 10208, 10208,  204, 10208, 10208,
 /*    50 */   -26, 10208, 10208, 10263, 10208, 10208,  -26, 10208, 10208,  -26,
 /*    60 */  10208, 10208, 10361, 10208, 10208, 10361, 10208,   90,   29,  286,
 /*    70 */  8274, 10263, 10208, 8993,  -57, 10208,  204, 10208,  204, 10208,
 /*    80 */   -26, 10208,  -26, 10208,  -26, 10208,  204, 10208, 5990, 10208,
 /*    90 */  5905, 10208, 5813, 10208, 5813, 10208, 5813, 10208, 5813, 10208,
 /*   100 */  5813, 10208, 8345, 10208, 9025, 10208, 10263, 5530,  -57,  -57,
 /*   110 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*   120 */   -57,  -57, 9057,  -57,  500, 10208,  -26,  510,  363, 10208,
 /*   130 */    90,  108,  284,  511, 8389,  -56, 9085, 10263,  540,  584,
 /*   140 */  10208,  -26,  -57, 10208,  -26,  -57,  -57,  -57,  -57,  -57,
 /*   150 */   -57,  -57,  -57, 9121, 10263,  103,  520,  573,  578,  -57,
 /*   160 */    92,  -57, 10306,  147,  636, 8425,  -57,    4,  -57, 9236,
 /*   170 */   659,  683,  178, 8540,  318,  -57,  -57,  -57,  -57,  -57,
 /*   180 */   -57, 10208, 10263,  700, 10373,  334, 9140,  -57,  723, 6173,
 /*   190 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  704,  777,
 /*   200 */   -57,  -57, 6087,  753,  776,  216,  -57,  392,  -57, 6259,
 /*   210 */   -57,  799, 6173,  -57,  -57,  -57,  -57, 6022,  832, 6173,
 /*   220 */   -57,  238,  852, 6173,  -57,  872,  883, 6173,  -57,  911,
 /*   230 */   907, 6173,  -57,  934,  960,  -57,  383,  926, 6173,  -57,
 /*   240 */   948,  949, 6173,  -57,  980,  984, 6173,  -57, 1007,   -7,
 /*   250 */   106,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*   260 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*   270 */   -57,  -57,  -57,  -57,  -57,  -57, 1017,  -57, 1018,  -57,
 /*   280 */  10208, 1013,  219,  332,  165,  310, 1014,  445,  558,  -57,
 /*   290 */  10208, 1020,   40,  -57,  182,  -57,  -57, 6173,  988, 5417,
 /*   300 */  5417, 1035,  671,  784,  -57, 10208, 1036,  897, 1010,  -57,
 /*   310 */  1037, 1123, 1236, 1003, 10208, 1051,  -57, 10263, 1044, 1349,
 /*   320 */  1462, 1004, 1004,  -57, 1068,  294, 1575, 1688,  -57, 1069,
 /*   330 */   428, 9272, 9387, 1801, 1914,  -57,  255,  481,  -57,  255,
 /*   340 */   -57, 6076,  -57,  -57,  -57,  -57,  -57,  -57,  -57, 10208,
 /*   350 */   -57, 10263,  260, 6004, 10208,  -57, 9414,  695, 10208,  -57,
 /*   360 */  1048,  -57, 9474, 5658, 10208,  -57, 9516,  695, 10208,  -57,
 /*   370 */   -57,  -57,  -57,  -57,  315, 1076,  695, 10208,  -57, 1081,
 /*   380 */   695, 10208,  -57, 1093, 6168, 10208,  -57, 9552,  695, 10208,
 /*   390 */   -57, 6254, 10208,  -57, 9618,  695, 10208,  -57, 9654,  695,
 /*   400 */  10208,  -57, 6340, 10208,  -57, 9720,  695, 10208,  -57,  -57,
 /*   410 */   -57, 1094,  -57, 1099,  -57,  505, 1095,  695, 10208,  -57,
 /*   420 */  1100,  695, 10208,  -57,  -57, 10208,  565,  -57, 10208,  -57,
 /*   430 */  10263,  -57, 1120,  -57, 1121,  -57, 1129,  -57, 1130,  -57,
 /*   440 */  9756, 9871,  -57,  -57, 10208, 9913,  -57, 10208, 9955,  -57,
 /*   450 */  10208, 9997,  -57, 1132,  579,  -57, 1132,  -57, 1124, 6173,
 /*   460 */   -57,  -57, 1132,  594,  -57, 1132,  637,  -57, 1132,  640,
 /*   470 */   -57, 1132,  657,  -57, 1132,  678,  -57, 1132,  692,  -57,
 /*   480 */  1132,  705,  -57, 1132,  728,  -57, 1132,  729,  -57, 1132,
 /*   490 */   769,  -57, 10263,  -57,  -57,  -57,  -57, 10208, 10039, 5417,
 /*   500 */  2027,  -57, 1142, 1090, 10066, 10121, 2140, 2253,  -57,  -57,
 /*   510 */  10208, 10181, 5417, 2366,  -57,  -57, 1145, 1136, 2479, 2592,
 /*   520 */   -57,  -57, 1068,  -57,  -57,  -57,  -57,  -57, 1111, 10208,
 /*   530 */  1163,  -57,  -57,  -57, 1122, 5417, 5417,  -57,  -57,  -57,
 /*   540 */  10208, 1155, 2705, 2818,  -57,  -57, 1164, 2931, 3044,  -57,
 /*   550 */   -57,  -57,  717,  741, 1173, 1177,  -57, 1190, 3157, 3270,
 /*   560 */   -57,  -57,  -57,  -57,  -57, 1193, 3383, 3496,  -57,  -57,
 /*   570 */   345, 1188, 8576,  795,  -57,  -57, 1206, 1196, 1191, 8691,
 /*   580 */   807,  -57,  -57,  -57, 1209, 1199, 1194, 8727,  -57,  868,
 /*   590 */   -57,  -57, 1167, 10208,  -57,  -57,  -57, 10208, 10263,  899,
 /*   600 */   -57,  -57,  -57,  908,  -57,  -57,  616, 1207, 1202, 8842,
 /*   610 */   909,  -57,  -57, 1212, 1215, 8878,  920,  -57,  -57,   90,
 /*   620 */    90,   90,   90,   90,   90,   90,  921,  -57,  -57, 1224,
 /*   630 */   511, 1234,  222,  -57, 1243, 1235,  -57,   38,  -57, 1238,
 /*   640 */   -57,  151,  716,  -57, 1846, 1250, 1239, 10418,  524, 8447,
 /*   650 */  1254,  -57,  -57, 1288, 8598,  -57, 1264,  -57,  -57,  -57,
 /*   660 */   -57,  -57, 1271,   33, 1249, 1299,  -57,  -57,  -57,  923,
 /*   670 */   580, 8447, 1277,  -57,  -57,  -57,  -57,  -57,  -57,  -57,
 /*   680 */   -57,  -57,  -57,  -57,  -57, 2411, 1959, 1280, 1270, 10463,
 /*   690 */   806, 8447, 1295,  -57,  -57,  981,  827, 8447, 1300,  -57,
 /*   700 */   -57,  -57,  -57,  -57, 2072,  707, 1303, 6173, 1312,  -57,
 /*   710 */  1305, 6173, 1320,  -57,  820, 1309, 6173, 1323,  -57, 1318,
 /*   720 */  6173, 1324,  -57,  829,  -57, 1336,  232,  -57, 1345,  819,
 /*   730 */   -57, 1347,  770,  -57,  264,  -57, 1346,  -57,  377,  942,
 /*   740 */   -57, 2185, 1358, 1350, 10508,  104, 3609,  -57, 3722,  -57,
 /*   750 */   -57, 8447,  599, 3835,  -57, 3948,  -57,  -57,  996,  330,
 /*   760 */  4061,  -57, 4174,  -57,  -57, 8447,  679, 4287,  -57, 4400,
 /*   770 */   -57,  -57, 2411, 2298, 1362, 1351, 10553,  418, 4513,  -57,
 /*   780 */  4626,  -57,  -57, 8447,  792, 4739,  -57, 4852,  -57,  -57,
 /*   790 */  1012,  443, 4965,  -57, 5078,  -57,  -57, 8447,  796, 5191,
 /*   800 */   -57, 5304,  -57,  -57,  490, 1055,  -57, 2072,  -57, 2072,
 /*   810 */  1168,  544,  -57, 6173,  905,  -57, 1368,  -57,  -20, 1383,
 /*   820 */   857, 1384,  397,  -57,  -57, 1386,  -57,  -57, 1390,  -57,
 /*   830 */  1281,  634,  -57, 6173,  931,  -57, 1399,  -57, 1407,  -57,
 /*   840 */   603, 1394, 1507, 2411, 1620,  -57, 1733, 1347,  -57,  -57,
 /*   850 */   -57, 1347,  770,  -57, 1404, 1414,  407,  -57, 1426,  859,
 /*   860 */   -57, 1347,  770,  -57, 1347,  770,  -57, 1425, 1436,  458,
 /*   870 */   -57, 1438, 1422,  -57, 1347,  770,  -57, 1121, 1129, 1130,
 /*   880 */  10208, 9913,  -57,
};
#define YY_REDUCE_USE_DFLT (-158)
static short yy_reduce_ofst[] = {
 /*     0 */  5656, -158, 8004, -158, -158, -158, -158, -158, -158, -158,
 /*    10 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*    20 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*    30 */  -158, -158, -125, -158, -158,   70, -158, -158, -158, -158,
 /*    40 */  -158, -158,   39, 2264, -158, 2645, 2758, -158, 2779, 2829,
 /*    50 */  -158, 2871, 2892, -158, 2942, 2984, -158, 3005, 3055, -158,
 /*    60 */  3097, 3118, -158, 3168, 3210, -158, 3231, -158, -158, -158,
 /*    70 */   152, -158, 3281, -158, -158, 3323, -158, 3344, -158, 3394,
 /*    80 */  -158, 3436, -158, 3457, -158, 3507, -158, 3549, -158, 3570,
 /*    90 */  -158, 3620, -158, 3662, -158, 3683, -158, 3733, -158, 3768,
 /*   100 */  -158, 3775, -158, 3796, -158, 3846, -158, 1734, -158, -158,
 /*   110 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*   120 */  -158, -158, -158, -158, -158, 3881, -158, -158, -158, 3888,
 /*   130 */  -158, -158, -158, -158,  265, -158, 3909, -158, -158, -158,
 /*   140 */  3959, -158, -158, 3994, -158, -158, -158, -158, -158, -158,
 /*   150 */  -158, -158, -158, 5664, -158, -158, -158, -158, -158, -158,
 /*   160 */  -158, -158, 5678, -158, -158,  378, -158, -158, -158, 1847,
 /*   170 */  -158, -158, -158,  491, -158, -158, -158, -158, -158, -158,
 /*   180 */  -158, 4001, -158, -158, -157, -158,  -53, -158, -158,  629,
 /*   190 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*   200 */  -158, -158,  135, -158, -158, -158, -158, -158, -158,  -30,
 /*   210 */  -158, -158, -103, -158, -158, -158, -158,   18, -158,  730,
 /*   220 */  -158, -158, -158,  774, -158, -158, -158,  791, -158, -158,
 /*   230 */  -158,  814, -158, -158, -158, -158, -158, -158,  823, -158,
 /*   240 */  -158, -158,  837, -158, -158, -158,  887, -158, -158, 5682,
 /*   250 */  8035, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*   260 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*   270 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*   280 */  1021, -158, 5768, 8035,  161,  888, -158, 5854, 8035, -158,
 /*   290 */  1134, -158,  226, -158,  885, -158, -158,  913, -158, 5940,
 /*   300 */  8035, -158, 6026, 8035, -158, 1699, -158, 6112, 8035, -158,
 /*   310 */  -158, 6198, 8035, -158, 1812, -158, -158, -158, -158, 6284,
 /*   320 */  8035,  248,  916, -158,  312, -158, 6370, 8035, -158, -158,
 /*   330 */  -158, 4022, -158, 6456, 8035, -158,  341, -158, -158,  906,
 /*   340 */  -158,  -25, -158, -158, -158, -158, -158, -158, -158,  -67,
 /*   350 */  -158, -158, -158,   35, 1960, -158, 2073,  919, 2186, -158,
 /*   360 */  -158, -158, -158,   13, 2299, -158, 2073,  924, 2412, -158,
 /*   370 */  -158, -158, -158, -158, -158, -158,  939, 2525, -158, -158,
 /*   380 */   944, 2638, -158, -158,   64, 2751, -158, 2073,  945, 2864,
 /*   390 */  -158,  313, 2977, -158, 2073,  946, 3090, -158, 2073,  947,
 /*   400 */  3203, -158,  352, 3316, -158, 2073,  954, 3429, -158, -158,
 /*   410 */  -158, -158, -158, -158, -158, -158, -158,  959, 3542, -158,
 /*   420 */  -158,  968, 3655, -158, -158,  719, -158, -158, 2327, -158,
 /*   430 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*   440 */  4072, -158, -158, -158, 4107, -158, -158, 4114, -158, -158,
 /*   450 */  4135, -158, -158,  371, -158, -158,  973, -158, -158, 1022,
 /*   460 */  -158, -158,  409, -158, -158,  414, -158, -158,  431, -158,
 /*   470 */  -158,  441, -158, -158,  480, -158, -158,  483, -158, -158,
 /*   480 */   507, -158, -158,  522, -158, -158,  550, -158, -158,  553,
 /*   490 */  -158, -158, -158, -158, -158, -158, -158, 4185, -158, 6542,
 /*   500 */  8035, -158, -158, -158, 4220, -158, 6628, 8035, -158, -158,
 /*   510 */  4227, -158, 6714, 8035, -158, -158, -158, -158, 6800, 8035,
 /*   520 */  -158, -158, 1011, -158, -158, -158, -158, -158, -158, 2038,
 /*   530 */  -158, -158, -158, -158, -158, 6886, 8035, -158, -158, -158,
 /*   540 */  2151, -158, 6972, 8035, -158, -158, -158, 7058, 8035, -158,
 /*   550 */  -158, -158,  631,  888, -158, -158, -158, -158, 7144, 8035,
 /*   560 */  -158, -158, -158, -158, -158, -158, 7230, 8035, -158, -158,
 /*   570 */  -158, -158,  830, -158, -158, -158, -158, -158, -158,  943,
 /*   580 */  -158, -158, -158, -158, -158, -158, -158, 1056, -158, -158,
 /*   590 */  -158, -158, -158, 2532, -158, -158, -158, 4248, -158, -158,
 /*   600 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, 1169,
 /*   610 */  -158, -158, -158, -158, -158, 1282, -158, -158, -158, -158,
 /*   620 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*   630 */  -158, -158, 1128, -158, -158, 1137, -158, 1432, -158, -158,
 /*   640 */  -158, 1433,  221, -158, 1125, -158, -158, -120, -158,  632,
 /*   650 */  -158, -158, -158, -158,  398, -158, -158, -158, -158, -158,
 /*   660 */  -158, -158, -158, -158, -158, -158, -158, -158, -158, -158,
 /*   670 */  -158, 2405, -158, -158, -158, -158, -158, -158, -158, -158,
 /*   680 */  -158, -158, -158, -158, -158,   65, 1125, -158, -158,  -98,
 /*   690 */  -158, 2466, -158, -158, -158, -158, -158, 2518, -158, -158,
 /*   700 */  -158, -158, -158, -158,   65, -158, -158, 1200, -158, -158,
 /*   710 */  -158, 1201, -158, -158, -158, -158, 1204, -158, -158, -158,
 /*   720 */  1213, -158, -158,  221, -158, -158, 1229, -158, -158, 1240,
 /*   730 */  -158,  343, 1248, -158, -150, -158, -158, -158, 1545,   -5,
 /*   740 */  -158, 1125, -158, -158,  241, -158, 7316, -158, 8035, -158,
 /*   750 */  -158, 2579, -158, 7402, -158, 8035, -158, -158, -158, -158,
 /*   760 */  7488, -158, 8035, -158, -158, 2631, -158, 7574, -158, 8035,
 /*   770 */  -158, -158,  755, 1125, -158, -158,  354, -158, 7660, -158,
 /*   780 */  8035, -158, -158, 2692, -158, 7746, -158, 8035, -158, -158,
 /*   790 */  -158, -158, 7832, -158, 8035, -158, -158, 2744, -158, 7918,
 /*   800 */  -158, 8035, -158, -158, 4362,   -5, -158,  755, -158,  880,
 /*   810 */  1125, 1251, -158, 1244, 1252, -158, -158, -158,  721, -158,
 /*   820 */  -158, -158, 1268, -158, -158, -158, -158, -158, -158, -158,
 /*   830 */  1125, 1273, -158, 1275, 1287, -158, -158, -158, -158, -158,
 /*   840 */  1658,  303,   -5,  864,   -5, -158,   -5, 1284, -158, -158,
 /*   850 */  -158,  846, 1301, -158, -158, -158, 1321, -158, -158, 1325,
 /*   860 */  -158,  870, 1326, -158,  874, 1327, -158, -158, -158, 1329,
 /*   870 */  -158, -158, 1337, -158,  875, 1341, -158, -158, -158, -158,
 /*   880 */  4298, -158, -158,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */  1322, 1322, 1322,  885,  887,  888,  889,  890,  891,  892,
 /*    10 */   893,  894,  895,  896,  897,  898,  899,  900,  901,  902,
 /*    20 */   903,  904,  905,  906,  907,  908,  909,  910,  911, 1322,
 /*    30 */  1322,  912, 1322, 1322,  913, 1322,  914,  916, 1322,  917,
 /*    40 */   915,  916, 1322, 1322, 1202, 1322, 1322, 1203, 1322, 1322,
 /*    50 */  1204, 1322, 1322, 1205, 1322, 1322, 1206, 1322, 1322, 1207,
 /*    60 */  1322, 1322, 1208, 1322, 1322, 1209, 1322, 1217, 1322, 1221,
 /*    70 */  1322, 1283, 1322, 1322, 1226, 1322, 1227, 1322, 1228, 1322,
 /*    80 */  1229, 1322, 1230, 1322, 1231, 1322, 1232, 1322, 1233, 1322,
 /*    90 */  1234, 1322, 1235, 1322, 1236, 1322, 1237, 1322, 1238, 1322,
 /*   100 */  1239, 1322, 1240, 1322, 1322, 1322, 1280, 1322, 1049, 1050,
 /*   110 */  1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060,
 /*   120 */  1061, 1062, 1322, 1218, 1322, 1322, 1219, 1322, 1322, 1322,
 /*   130 */  1220, 1244, 1322, 1224, 1322, 1244, 1322, 1284, 1322, 1322,
 /*   140 */  1322, 1241, 1242, 1322, 1243, 1245, 1246, 1247, 1248, 1249,
 /*   150 */  1250, 1251, 1252, 1322, 1299, 1244, 1245, 1246, 1252, 1253,
 /*   160 */  1322, 1254, 1322, 1322, 1255, 1322, 1256, 1322, 1257, 1322,
 /*   170 */  1322, 1322, 1322, 1322, 1322, 1263, 1264, 1277, 1278, 1279,
 /*   180 */  1282, 1322, 1285, 1322, 1322, 1322, 1322, 1029, 1031, 1322,
 /*   190 */  1039, 1300, 1301, 1302, 1303, 1304, 1305, 1306, 1322, 1322,
 /*   200 */  1307, 1308, 1322, 1300, 1302, 1322, 1309, 1322, 1310, 1322,
 /*   210 */  1311, 1322, 1322, 1313, 1318, 1314, 1312, 1322, 1032, 1322,
 /*   220 */  1040, 1322, 1034, 1322, 1042, 1322, 1036, 1322, 1044, 1322,
 /*   230 */  1038, 1322, 1046, 1322, 1322, 1047, 1322, 1033, 1322, 1041,
 /*   240 */  1322, 1035, 1322, 1043, 1322, 1037, 1322, 1045, 1322, 1322,
 /*   250 */  1322, 1063, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072,
 /*   260 */  1073, 1074, 1075, 1076, 1077, 1078, 1079, 1080, 1081, 1082,
 /*   270 */  1083, 1084, 1085, 1086, 1087, 1088, 1322, 1089, 1322, 1090,
 /*   280 */  1322, 1322, 1322, 1322, 1095, 1096, 1322, 1322, 1322, 1098,
 /*   290 */  1322, 1322, 1322, 1106, 1322, 1107, 1108, 1322, 1322, 1110,
 /*   300 */  1111, 1322, 1322, 1322, 1114, 1322, 1322, 1322, 1322, 1116,
 /*   310 */  1322, 1322, 1322, 1322, 1322, 1322, 1118, 1319, 1322, 1322,
 /*   320 */  1322, 1120, 1121, 1122, 1322, 1322, 1322, 1322, 1124, 1322,
 /*   330 */  1322, 1322, 1322, 1322, 1322, 1131, 1322, 1322, 1137, 1322,
 /*   340 */  1138, 1322, 1140, 1141, 1142, 1143, 1144, 1145, 1146, 1322,
 /*   350 */  1147, 1201, 1322, 1322, 1322, 1148, 1322, 1322, 1322, 1151,
 /*   360 */  1322, 1163, 1322, 1322, 1322, 1152, 1322, 1322, 1322, 1153,
 /*   370 */  1161, 1162, 1164, 1165, 1322, 1322, 1322, 1322, 1149, 1322,
 /*   380 */  1322, 1322, 1150, 1322, 1322, 1322, 1154, 1322, 1322, 1322,
 /*   390 */  1155, 1322, 1322, 1156, 1322, 1322, 1322, 1157, 1322, 1322,
 /*   400 */  1322, 1158, 1322, 1322, 1159, 1322, 1322, 1322, 1160, 1166,
 /*   410 */  1168, 1322, 1167, 1322, 1169, 1322, 1322, 1322, 1322, 1170,
 /*   420 */  1322, 1322, 1322, 1171, 1139, 1322, 1322, 1173, 1322, 1174,
 /*   430 */  1176, 1175, 1277, 1177, 1279, 1178, 1278, 1179, 1242, 1180,
 /*   440 */  1322, 1322, 1181, 1182, 1322, 1322, 1183, 1322, 1322, 1184,
 /*   450 */  1322, 1322, 1185, 1322, 1322, 1186, 1322, 1197, 1199, 1322,
 /*   460 */  1200, 1198, 1322, 1322, 1187, 1322, 1322, 1188, 1322, 1322,
 /*   470 */  1189, 1322, 1322, 1190, 1322, 1322, 1191, 1322, 1322, 1192,
 /*   480 */  1322, 1322, 1193, 1322, 1322, 1194, 1322, 1322, 1195, 1322,
 /*   490 */  1322, 1196, 1322, 1320, 1321, 1064, 1132, 1322, 1322, 1322,
 /*   500 */  1322, 1133, 1322, 1322, 1322, 1322, 1322, 1322, 1134, 1135,
 /*   510 */  1322, 1322, 1322, 1322, 1136, 1125, 1322, 1322, 1322, 1322,
 /*   520 */  1127, 1126, 1322, 1128, 1130, 1129, 1123, 1119, 1322, 1322,
 /*   530 */  1322, 1117, 1115, 1113, 1322, 1322, 1112, 1109, 1100, 1102,
 /*   540 */  1322, 1322, 1322, 1322, 1105, 1104, 1322, 1322, 1322, 1097,
 /*   550 */  1099, 1103, 1091, 1092, 1322, 1322, 1094, 1322, 1322, 1322,
 /*   560 */  1101, 1093, 1290, 1289, 1030, 1322, 1322, 1322, 1288, 1287,
 /*   570 */  1322, 1322, 1322, 1322, 1267, 1268, 1322, 1322, 1322, 1322,
 /*   580 */  1322, 1269, 1270, 1281, 1322, 1322, 1258, 1322, 1259, 1322,
 /*   590 */  1260, 1291, 1322, 1322, 1293, 1294, 1292, 1322, 1286, 1322,
 /*   600 */  1265, 1266, 1225, 1322, 1271, 1272, 1322, 1322, 1222, 1322,
 /*   610 */  1322, 1273, 1274, 1322, 1223, 1322, 1322, 1275, 1276, 1216,
 /*   620 */  1215, 1214, 1213, 1212, 1211, 1210, 1322, 1261, 1262, 1322,
 /*   630 */  1322, 1322, 1322,  918, 1322, 1322,  919, 1322,  936, 1322,
 /*   640 */   937, 1322, 1322,  970, 1322, 1322, 1322, 1322, 1322, 1322,
 /*   650 */  1322, 1000, 1019, 1020, 1322, 1021, 1023, 1026, 1024, 1025,
 /*   660 */  1027, 1028, 1322, 1322, 1322, 1322, 1048, 1022, 1004, 1322,
 /*   670 */  1322, 1322, 1322, 1001, 1005, 1008, 1010, 1011, 1012, 1013,
 /*   680 */  1014, 1015, 1016, 1017, 1018, 1322, 1322, 1322, 1322, 1322,
 /*   690 */  1322, 1322, 1322, 1002, 1006, 1322, 1322, 1322, 1322, 1003,
 /*   700 */  1007, 1009,  966,  971, 1322, 1322, 1322, 1322, 1322,  972,
 /*   710 */  1322, 1322, 1322,  974, 1322, 1322, 1322, 1322,  973, 1322,
 /*   720 */  1322, 1322,  975, 1322,  967, 1322, 1322,  920, 1322, 1322,
 /*   730 */   921, 1322, 1322,  923, 1322,  931, 1322,  932, 1322, 1322,
 /*   740 */   968, 1322, 1322, 1322, 1322, 1322, 1322,  976, 1322,  980,
 /*   750 */   977, 1322, 1322, 1322,  988, 1322,  992,  989, 1322, 1322,
 /*   760 */  1322,  978, 1322,  981,  979, 1322, 1322, 1322,  990, 1322,
 /*   770 */   993,  991, 1322, 1322, 1322, 1322, 1322, 1322, 1322,  982,
 /*   780 */  1322,  986,  983, 1322, 1322, 1322,  994, 1322,  998,  995,
 /*   790 */  1322, 1322, 1322,  984, 1322,  987,  985, 1322, 1322, 1322,
 /*   800 */   996, 1322,  999,  997, 1322, 1322,  969, 1322,  950, 1322,
 /*   810 */  1322, 1322,  952, 1322, 1322,  954, 1322,  958, 1322, 1322,
 /*   820 */  1322, 1322, 1322,  962,  964, 1322,  965,  963, 1322,  956,
 /*   830 */  1322, 1322,  953, 1322, 1322,  955, 1322,  959, 1322,  957,
 /*   840 */  1322, 1322, 1322, 1322, 1322,  951, 1322, 1322,  933,  935,
 /*   850 */   934, 1322, 1322,  922, 1322, 1322, 1322,  924, 1322, 1322,
 /*   860 */   925, 1322, 1322,  927, 1322, 1322,  926, 1322, 1322, 1322,
 /*   870 */   928, 1322, 1322,  929, 1322, 1322,  930, 1322, 1322, 1322,
 /*   880 */  1322, 1322,  886,
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
  "xx_interface_def",  "xx_comment",    "xx_cblock",     "xx_let_statement",
  "xx_if_statement",  "xx_loop_statement",  "xx_echo_statement",  "xx_return_statement",
  "xx_require_statement",  "xx_fetch_statement",  "xx_fcall_statement",  "xx_scall_statement",
  "xx_unset_statement",  "xx_throw_statement",  "xx_declare_statement",  "xx_break_statement",
  "xx_continue_statement",  "xx_while_statement",  "xx_do_while_statement",  "xx_try_catch_statement",
  "xx_switch_statement",  "xx_for_statement",  "xx_use_aliases_list",  "xx_interface_body",
  "xx_class_body",  "xx_implements_list",  "xx_class_definition",  "xx_implements",
  "xx_interface_definition",  "xx_class_properties_definition",  "xx_class_consts_definition",  "xx_class_methods_definition",
  "xx_interface_methods_definition",  "xx_class_property_definition",  "xx_visibility_list",  "xx_literal_expr",
  "xx_class_property_shortcuts",  "xx_class_property_shortcuts_list",  "xx_class_property_shortcut",  "xx_class_const_definition",
  "xx_class_method_definition",  "xx_interface_method_definition",  "xx_parameter_list",  "xx_statement_list",
  "xx_method_return_type",  "xx_visibility",  "xx_method_return_type_list",  "xx_method_return_type_item",
  "xx_parameter_type",  "xx_parameter_cast",  "xx_parameter_cast_collection",  "xx_parameter",
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
 /*   8 */ "xx_top_statement ::= xx_comment",
 /*   9 */ "xx_top_statement ::= xx_cblock",
 /*  10 */ "xx_top_statement ::= xx_let_statement",
 /*  11 */ "xx_top_statement ::= xx_if_statement",
 /*  12 */ "xx_top_statement ::= xx_loop_statement",
 /*  13 */ "xx_top_statement ::= xx_echo_statement",
 /*  14 */ "xx_top_statement ::= xx_return_statement",
 /*  15 */ "xx_top_statement ::= xx_require_statement",
 /*  16 */ "xx_top_statement ::= xx_fetch_statement",
 /*  17 */ "xx_top_statement ::= xx_fcall_statement",
 /*  18 */ "xx_top_statement ::= xx_scall_statement",
 /*  19 */ "xx_top_statement ::= xx_unset_statement",
 /*  20 */ "xx_top_statement ::= xx_throw_statement",
 /*  21 */ "xx_top_statement ::= xx_declare_statement",
 /*  22 */ "xx_top_statement ::= xx_break_statement",
 /*  23 */ "xx_top_statement ::= xx_continue_statement",
 /*  24 */ "xx_top_statement ::= xx_while_statement",
 /*  25 */ "xx_top_statement ::= xx_do_while_statement",
 /*  26 */ "xx_top_statement ::= xx_try_catch_statement",
 /*  27 */ "xx_top_statement ::= xx_switch_statement",
 /*  28 */ "xx_top_statement ::= xx_for_statement",
 /*  29 */ "xx_namespace_def ::= NAMESPACE IDENTIFIER DOTCOMMA",
 /*  30 */ "xx_namespace_def ::= USE xx_use_aliases_list DOTCOMMA",
 /*  31 */ "xx_use_aliases_list ::= xx_use_aliases_list COMMA xx_use_aliases",
 /*  32 */ "xx_use_aliases_list ::= xx_use_aliases",
 /*  33 */ "xx_use_aliases ::= IDENTIFIER",
 /*  34 */ "xx_use_aliases ::= IDENTIFIER AS IDENTIFIER",
 /*  35 */ "xx_interface_def ::= INTERFACE IDENTIFIER xx_interface_body",
 /*  36 */ "xx_interface_def ::= INTERFACE IDENTIFIER EXTENDS IDENTIFIER xx_interface_body",
 /*  37 */ "xx_class_def ::= CLASS IDENTIFIER xx_class_body",
 /*  38 */ "xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body",
 /*  39 */ "xx_class_def ::= CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  40 */ "xx_class_def ::= CLASS IDENTIFIER EXTENDS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  41 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER xx_class_body",
 /*  42 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body",
 /*  43 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  44 */ "xx_class_def ::= ABSTRACT CLASS IDENTIFIER EXTENDS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  45 */ "xx_class_def ::= FINAL CLASS IDENTIFIER xx_class_body",
 /*  46 */ "xx_class_def ::= FINAL CLASS IDENTIFIER EXTENDS IDENTIFIER xx_class_body",
 /*  47 */ "xx_class_def ::= FINAL CLASS IDENTIFIER IMPLEMENTS xx_implements_list xx_class_body",
 /*  48 */ "xx_class_body ::= BRACKET_OPEN BRACKET_CLOSE",
 /*  49 */ "xx_class_body ::= BRACKET_OPEN xx_class_definition BRACKET_CLOSE",
 /*  50 */ "xx_implements_list ::= xx_implements_list COMMA xx_implements",
 /*  51 */ "xx_implements_list ::= xx_implements",
 /*  52 */ "xx_implements ::= IDENTIFIER",
 /*  53 */ "xx_interface_body ::= BRACKET_OPEN BRACKET_CLOSE",
 /*  54 */ "xx_interface_body ::= BRACKET_OPEN xx_interface_definition BRACKET_CLOSE",
 /*  55 */ "xx_class_definition ::= xx_class_properties_definition",
 /*  56 */ "xx_class_definition ::= xx_class_consts_definition",
 /*  57 */ "xx_class_definition ::= xx_class_methods_definition",
 /*  58 */ "xx_class_definition ::= xx_class_properties_definition xx_class_methods_definition",
 /*  59 */ "xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition",
 /*  60 */ "xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition",
 /*  61 */ "xx_class_definition ::= xx_class_consts_definition xx_class_methods_definition",
 /*  62 */ "xx_class_definition ::= xx_class_properties_definition xx_class_consts_definition xx_class_methods_definition",
 /*  63 */ "xx_class_definition ::= xx_class_consts_definition xx_class_properties_definition xx_class_methods_definition",
 /*  64 */ "xx_interface_definition ::= xx_class_consts_definition",
 /*  65 */ "xx_interface_definition ::= xx_interface_methods_definition",
 /*  66 */ "xx_interface_definition ::= xx_class_consts_definition xx_interface_methods_definition",
 /*  67 */ "xx_class_properties_definition ::= xx_class_properties_definition xx_class_property_definition",
 /*  68 */ "xx_class_properties_definition ::= xx_class_property_definition",
 /*  69 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER DOTCOMMA",
 /*  70 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER DOTCOMMA",
 /*  71 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  72 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  73 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA",
 /*  74 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER xx_class_property_shortcuts DOTCOMMA",
 /*  75 */ "xx_class_property_definition ::= COMMENT xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA",
 /*  76 */ "xx_class_property_definition ::= xx_visibility_list IDENTIFIER ASSIGN xx_literal_expr xx_class_property_shortcuts DOTCOMMA",
 /*  77 */ "xx_class_property_shortcuts ::= BRACKET_OPEN BRACKET_CLOSE",
 /*  78 */ "xx_class_property_shortcuts ::= BRACKET_OPEN xx_class_property_shortcuts_list BRACKET_CLOSE",
 /*  79 */ "xx_class_property_shortcuts_list ::= xx_class_property_shortcuts_list COMMA xx_class_property_shortcut",
 /*  80 */ "xx_class_property_shortcuts_list ::= xx_class_property_shortcut",
 /*  81 */ "xx_class_property_shortcut ::= IDENTIFIER",
 /*  82 */ "xx_class_property_shortcut ::= COMMENT IDENTIFIER",
 /*  83 */ "xx_class_consts_definition ::= xx_class_consts_definition xx_class_const_definition",
 /*  84 */ "xx_class_consts_definition ::= xx_class_const_definition",
 /*  85 */ "xx_class_methods_definition ::= xx_class_methods_definition xx_class_method_definition",
 /*  86 */ "xx_class_methods_definition ::= xx_class_method_definition",
 /*  87 */ "xx_interface_methods_definition ::= xx_interface_methods_definition xx_interface_method_definition",
 /*  88 */ "xx_interface_methods_definition ::= xx_interface_method_definition",
 /*  89 */ "xx_class_const_definition ::= COMMENT CONST CONSTANT ASSIGN xx_literal_expr DOTCOMMA",
 /*  90 */ "xx_class_const_definition ::= CONST CONSTANT ASSIGN xx_literal_expr DOTCOMMA",
 /*  91 */ "xx_class_const_definition ::= COMMENT CONST IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  92 */ "xx_class_const_definition ::= CONST IDENTIFIER ASSIGN xx_literal_expr DOTCOMMA",
 /*  93 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /*  94 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /*  95 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /*  96 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /*  97 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  98 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /*  99 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 100 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /* 101 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 102 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /* 103 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 104 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 105 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /* 106 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 107 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /* 108 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 109 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 110 */ "xx_class_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 111 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /* 112 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 113 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN BRACKET_CLOSE",
 /* 114 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 115 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 116 */ "xx_class_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 117 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 118 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 119 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 120 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE ARROW xx_method_return_type DOTCOMMA",
 /* 121 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /* 122 */ "xx_interface_method_definition ::= xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /* 123 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE DOTCOMMA",
 /* 124 */ "xx_interface_method_definition ::= COMMENT xx_visibility_list FUNCTION IDENTIFIER PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE DOTCOMMA",
 /* 125 */ "xx_visibility_list ::= xx_visibility_list xx_visibility",
 /* 126 */ "xx_visibility_list ::= xx_visibility",
 /* 127 */ "xx_visibility ::= PUBLIC",
 /* 128 */ "xx_visibility ::= PROTECTED",
 /* 129 */ "xx_visibility ::= PRIVATE",
 /* 130 */ "xx_visibility ::= STATIC",
 /* 131 */ "xx_visibility ::= SCOPED",
 /* 132 */ "xx_visibility ::= INLINE",
 /* 133 */ "xx_visibility ::= DEPRECATED",
 /* 134 */ "xx_visibility ::= ABSTRACT",
 /* 135 */ "xx_visibility ::= FINAL",
 /* 136 */ "xx_method_return_type ::= VOID",
 /* 137 */ "xx_method_return_type ::= xx_method_return_type_list",
 /* 138 */ "xx_method_return_type_list ::= xx_method_return_type_list BITWISE_OR xx_method_return_type_item",
 /* 139 */ "xx_method_return_type_list ::= xx_method_return_type_item",
 /* 140 */ "xx_method_return_type_item ::= xx_parameter_type",
 /* 141 */ "xx_method_return_type_item ::= NULL",
 /* 142 */ "xx_method_return_type_item ::= THIS",
 /* 143 */ "xx_method_return_type_item ::= xx_parameter_type NOT",
 /* 144 */ "xx_method_return_type_item ::= xx_parameter_cast",
 /* 145 */ "xx_method_return_type_item ::= xx_parameter_cast_collection",
 /* 146 */ "xx_parameter_list ::= xx_parameter_list COMMA xx_parameter",
 /* 147 */ "xx_parameter_list ::= xx_parameter",
 /* 148 */ "xx_parameter ::= IDENTIFIER",
 /* 149 */ "xx_parameter ::= CONST IDENTIFIER",
 /* 150 */ "xx_parameter ::= xx_parameter_type IDENTIFIER",
 /* 151 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER",
 /* 152 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER",
 /* 153 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER",
 /* 154 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER",
 /* 155 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER",
 /* 156 */ "xx_parameter ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 157 */ "xx_parameter ::= CONST IDENTIFIER ASSIGN xx_literal_expr",
 /* 158 */ "xx_parameter ::= xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 159 */ "xx_parameter ::= CONST xx_parameter_type IDENTIFIER ASSIGN xx_literal_expr",
 /* 160 */ "xx_parameter ::= xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 161 */ "xx_parameter ::= CONST xx_parameter_type NOT IDENTIFIER ASSIGN xx_literal_expr",
 /* 162 */ "xx_parameter ::= xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 163 */ "xx_parameter ::= CONST xx_parameter_cast IDENTIFIER ASSIGN xx_literal_expr",
 /* 164 */ "xx_parameter_cast ::= LESS IDENTIFIER GREATER",
 /* 165 */ "xx_parameter_cast_collection ::= LESS IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE GREATER",
 /* 166 */ "xx_parameter_type ::= TYPE_INTEGER",
 /* 167 */ "xx_parameter_type ::= TYPE_UINTEGER",
 /* 168 */ "xx_parameter_type ::= TYPE_LONG",
 /* 169 */ "xx_parameter_type ::= TYPE_ULONG",
 /* 170 */ "xx_parameter_type ::= TYPE_CHAR",
 /* 171 */ "xx_parameter_type ::= TYPE_UCHAR",
 /* 172 */ "xx_parameter_type ::= TYPE_DOUBLE",
 /* 173 */ "xx_parameter_type ::= TYPE_BOOL",
 /* 174 */ "xx_parameter_type ::= TYPE_STRING",
 /* 175 */ "xx_parameter_type ::= TYPE_ARRAY",
 /* 176 */ "xx_parameter_type ::= TYPE_VAR",
 /* 177 */ "xx_parameter_type ::= TYPE_CALLABLE",
 /* 178 */ "xx_parameter_type ::= TYPE_RESOURCE",
 /* 179 */ "xx_parameter_type ::= TYPE_OBJECT",
 /* 180 */ "xx_statement_list ::= xx_statement_list xx_statement",
 /* 181 */ "xx_statement_list ::= xx_statement",
 /* 182 */ "xx_statement ::= xx_cblock",
 /* 183 */ "xx_statement ::= xx_let_statement",
 /* 184 */ "xx_statement ::= xx_if_statement",
 /* 185 */ "xx_statement ::= xx_loop_statement",
 /* 186 */ "xx_statement ::= xx_echo_statement",
 /* 187 */ "xx_statement ::= xx_return_statement",
 /* 188 */ "xx_statement ::= xx_require_statement",
 /* 189 */ "xx_statement ::= xx_fetch_statement",
 /* 190 */ "xx_statement ::= xx_fcall_statement",
 /* 191 */ "xx_statement ::= xx_mcall_statement",
 /* 192 */ "xx_statement ::= xx_scall_statement",
 /* 193 */ "xx_statement ::= xx_unset_statement",
 /* 194 */ "xx_statement ::= xx_throw_statement",
 /* 195 */ "xx_statement ::= xx_declare_statement",
 /* 196 */ "xx_statement ::= xx_break_statement",
 /* 197 */ "xx_statement ::= xx_continue_statement",
 /* 198 */ "xx_statement ::= xx_while_statement",
 /* 199 */ "xx_statement ::= xx_do_while_statement",
 /* 200 */ "xx_statement ::= xx_try_catch_statement",
 /* 201 */ "xx_statement ::= xx_switch_statement",
 /* 202 */ "xx_statement ::= xx_for_statement",
 /* 203 */ "xx_statement ::= xx_comment",
 /* 204 */ "xx_statement ::= xx_empty_statement",
 /* 205 */ "xx_empty_statement ::= DOTCOMMA",
 /* 206 */ "xx_break_statement ::= BREAK DOTCOMMA",
 /* 207 */ "xx_continue_statement ::= CONTINUE DOTCOMMA",
 /* 208 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 209 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements",
 /* 210 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 211 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 212 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 213 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements",
 /* 214 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 215 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 216 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 217 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_elseif_statements ELSE BRACKET_OPEN BRACKET_CLOSE",
 /* 218 */ "xx_if_statement ::= IF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE ELSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 219 */ "xx_elseif_statements ::= xx_elseif_statements xx_elseif_statement",
 /* 220 */ "xx_elseif_statements ::= xx_elseif_statement",
 /* 221 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 222 */ "xx_elseif_statement ::= ELSEIF xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 223 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 224 */ "xx_switch_statement ::= SWITCH xx_eval_expr BRACKET_OPEN xx_case_clauses BRACKET_CLOSE",
 /* 225 */ "xx_case_clauses ::= xx_case_clauses xx_case_clause",
 /* 226 */ "xx_case_clauses ::= xx_case_clause",
 /* 227 */ "xx_case_clause ::= CASE xx_literal_expr COLON",
 /* 228 */ "xx_case_clause ::= CASE xx_literal_expr COLON xx_statement_list",
 /* 229 */ "xx_case_clause ::= DEFAULT COLON xx_statement_list",
 /* 230 */ "xx_loop_statement ::= LOOP BRACKET_OPEN BRACKET_CLOSE",
 /* 231 */ "xx_loop_statement ::= LOOP BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 232 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 233 */ "xx_while_statement ::= WHILE xx_eval_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 234 */ "xx_do_while_statement ::= DO BRACKET_OPEN BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 235 */ "xx_do_while_statement ::= DO BRACKET_OPEN xx_statement_list BRACKET_CLOSE WHILE xx_eval_expr DOTCOMMA",
 /* 236 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN BRACKET_CLOSE",
 /* 237 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 238 */ "xx_try_catch_statement ::= TRY BRACKET_OPEN xx_statement_list BRACKET_CLOSE xx_catch_statement_list",
 /* 239 */ "xx_catch_statement_list ::= xx_catch_statement_list xx_catch_statement",
 /* 240 */ "xx_catch_statement_list ::= xx_catch_statement",
 /* 241 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 242 */ "xx_catch_statement ::= CATCH xx_catch_classes_list BRACKET_OPEN BRACKET_CLOSE",
 /* 243 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN BRACKET_CLOSE",
 /* 244 */ "xx_catch_statement ::= CATCH xx_catch_classes_list COMMA IDENTIFIER BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 245 */ "xx_catch_classes_list ::= xx_catch_classes_list BITWISE_OR xx_catch_class",
 /* 246 */ "xx_catch_classes_list ::= xx_catch_class",
 /* 247 */ "xx_catch_class ::= IDENTIFIER",
 /* 248 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 249 */ "xx_for_statement ::= FOR IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 250 */ "xx_for_statement ::= FOR IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 251 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 252 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN xx_common_expr BRACKET_OPEN BRACKET_CLOSE",
 /* 253 */ "xx_for_statement ::= FOR IDENTIFIER COMMA IDENTIFIER IN REVERSE xx_common_expr BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 254 */ "xx_let_statement ::= LET xx_let_assignments DOTCOMMA",
 /* 255 */ "xx_let_assignments ::= xx_let_assignments COMMA xx_let_assignment",
 /* 256 */ "xx_let_assignments ::= xx_let_assignment",
 /* 257 */ "xx_assignment_operator ::= ASSIGN",
 /* 258 */ "xx_assignment_operator ::= ADDASSIGN",
 /* 259 */ "xx_assignment_operator ::= SUBASSIGN",
 /* 260 */ "xx_assignment_operator ::= MULASSIGN",
 /* 261 */ "xx_assignment_operator ::= DIVASSIGN",
 /* 262 */ "xx_assignment_operator ::= CONCATASSIGN",
 /* 263 */ "xx_assignment_operator ::= MODASSIGN",
 /* 264 */ "xx_let_assignment ::= IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 265 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 266 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 267 */ "xx_let_assignment ::= IDENTIFIER ARROW BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 268 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 269 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 270 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 271 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_assignment_operator xx_assign_expr",
 /* 272 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 273 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 274 */ "xx_let_assignment ::= IDENTIFIER DOUBLECOLON IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 275 */ "xx_let_assignment ::= IDENTIFIER SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 276 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list xx_assignment_operator xx_assign_expr",
 /* 277 */ "xx_let_assignment ::= IDENTIFIER xx_array_offset_list SBRACKET_OPEN SBRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 278 */ "xx_array_offset_list ::= xx_array_offset_list xx_array_offset",
 /* 279 */ "xx_array_offset_list ::= xx_array_offset",
 /* 280 */ "xx_array_offset ::= SBRACKET_OPEN xx_index_expr SBRACKET_CLOSE",
 /* 281 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER INCR",
 /* 282 */ "xx_let_assignment ::= IDENTIFIER ARROW IDENTIFIER DECR",
 /* 283 */ "xx_let_assignment ::= IDENTIFIER INCR",
 /* 284 */ "xx_let_assignment ::= INCR IDENTIFIER",
 /* 285 */ "xx_let_assignment ::= IDENTIFIER DECR",
 /* 286 */ "xx_let_assignment ::= DECR IDENTIFIER",
 /* 287 */ "xx_let_assignment ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 288 */ "xx_let_assignment ::= BRACKET_OPEN STRING BRACKET_CLOSE xx_assignment_operator xx_assign_expr",
 /* 289 */ "xx_index_expr ::= xx_common_expr",
 /* 290 */ "xx_echo_statement ::= ECHO xx_echo_expressions DOTCOMMA",
 /* 291 */ "xx_echo_expressions ::= xx_echo_expressions COMMA xx_echo_expression",
 /* 292 */ "xx_echo_expressions ::= xx_echo_expression",
 /* 293 */ "xx_echo_expression ::= xx_common_expr",
 /* 294 */ "xx_mcall_statement ::= xx_mcall_expr DOTCOMMA",
 /* 295 */ "xx_fcall_statement ::= xx_fcall_expr DOTCOMMA",
 /* 296 */ "xx_scall_statement ::= xx_scall_expr DOTCOMMA",
 /* 297 */ "xx_fetch_statement ::= xx_fetch_expr DOTCOMMA",
 /* 298 */ "xx_return_statement ::= RETURN xx_common_expr DOTCOMMA",
 /* 299 */ "xx_return_statement ::= RETURN DOTCOMMA",
 /* 300 */ "xx_require_statement ::= REQUIRE xx_common_expr DOTCOMMA",
 /* 301 */ "xx_unset_statement ::= UNSET xx_common_expr DOTCOMMA",
 /* 302 */ "xx_throw_statement ::= THROW xx_common_expr DOTCOMMA",
 /* 303 */ "xx_declare_statement ::= TYPE_INTEGER xx_declare_variable_list DOTCOMMA",
 /* 304 */ "xx_declare_statement ::= TYPE_UINTEGER xx_declare_variable_list DOTCOMMA",
 /* 305 */ "xx_declare_statement ::= TYPE_CHAR xx_declare_variable_list DOTCOMMA",
 /* 306 */ "xx_declare_statement ::= TYPE_UCHAR xx_declare_variable_list DOTCOMMA",
 /* 307 */ "xx_declare_statement ::= TYPE_LONG xx_declare_variable_list DOTCOMMA",
 /* 308 */ "xx_declare_statement ::= TYPE_ULONG xx_declare_variable_list DOTCOMMA",
 /* 309 */ "xx_declare_statement ::= TYPE_DOUBLE xx_declare_variable_list DOTCOMMA",
 /* 310 */ "xx_declare_statement ::= TYPE_STRING xx_declare_variable_list DOTCOMMA",
 /* 311 */ "xx_declare_statement ::= TYPE_BOOL xx_declare_variable_list DOTCOMMA",
 /* 312 */ "xx_declare_statement ::= TYPE_VAR xx_declare_variable_list DOTCOMMA",
 /* 313 */ "xx_declare_statement ::= TYPE_ARRAY xx_declare_variable_list DOTCOMMA",
 /* 314 */ "xx_declare_variable_list ::= xx_declare_variable_list COMMA xx_declare_variable",
 /* 315 */ "xx_declare_variable_list ::= xx_declare_variable",
 /* 316 */ "xx_declare_variable ::= IDENTIFIER",
 /* 317 */ "xx_declare_variable ::= IDENTIFIER ASSIGN xx_literal_expr",
 /* 318 */ "xx_assign_expr ::= xx_common_expr",
 /* 319 */ "xx_common_expr ::= NOT xx_common_expr",
 /* 320 */ "xx_common_expr ::= SUB xx_common_expr",
 /* 321 */ "xx_common_expr ::= ISSET xx_common_expr",
 /* 322 */ "xx_common_expr ::= REQUIRE xx_common_expr",
 /* 323 */ "xx_common_expr ::= CLONE xx_common_expr",
 /* 324 */ "xx_common_expr ::= EMPTY xx_common_expr",
 /* 325 */ "xx_common_expr ::= LIKELY xx_common_expr",
 /* 326 */ "xx_common_expr ::= UNLIKELY xx_common_expr",
 /* 327 */ "xx_common_expr ::= xx_common_expr EQUALS xx_common_expr",
 /* 328 */ "xx_common_expr ::= xx_common_expr NOTEQUALS xx_common_expr",
 /* 329 */ "xx_common_expr ::= xx_common_expr IDENTICAL xx_common_expr",
 /* 330 */ "xx_common_expr ::= xx_common_expr NOTIDENTICAL xx_common_expr",
 /* 331 */ "xx_common_expr ::= xx_common_expr LESS xx_common_expr",
 /* 332 */ "xx_common_expr ::= xx_common_expr GREATER xx_common_expr",
 /* 333 */ "xx_common_expr ::= xx_common_expr LESSEQUAL xx_common_expr",
 /* 334 */ "xx_common_expr ::= xx_common_expr GREATEREQUAL xx_common_expr",
 /* 335 */ "xx_common_expr ::= PARENTHESES_OPEN xx_common_expr PARENTHESES_CLOSE",
 /* 336 */ "xx_common_expr ::= PARENTHESES_OPEN xx_parameter_type PARENTHESES_CLOSE xx_common_expr",
 /* 337 */ "xx_common_expr ::= LESS IDENTIFIER GREATER xx_common_expr",
 /* 338 */ "xx_common_expr ::= xx_common_expr ARROW IDENTIFIER",
 /* 339 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 340 */ "xx_common_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE",
 /* 341 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER",
 /* 342 */ "xx_common_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 343 */ "xx_common_expr ::= xx_common_expr SBRACKET_OPEN xx_common_expr SBRACKET_CLOSE",
 /* 344 */ "xx_common_expr ::= xx_common_expr ADD xx_common_expr",
 /* 345 */ "xx_common_expr ::= xx_common_expr SUB xx_common_expr",
 /* 346 */ "xx_common_expr ::= xx_common_expr MUL xx_common_expr",
 /* 347 */ "xx_common_expr ::= xx_common_expr DIV xx_common_expr",
 /* 348 */ "xx_common_expr ::= xx_common_expr MOD xx_common_expr",
 /* 349 */ "xx_common_expr ::= xx_common_expr CONCAT xx_common_expr",
 /* 350 */ "xx_common_expr ::= xx_common_expr AND xx_common_expr",
 /* 351 */ "xx_common_expr ::= xx_common_expr OR xx_common_expr",
 /* 352 */ "xx_common_expr ::= xx_common_expr BITWISE_AND xx_common_expr",
 /* 353 */ "xx_common_expr ::= xx_common_expr BITWISE_OR xx_common_expr",
 /* 354 */ "xx_common_expr ::= xx_common_expr BITWISE_XOR xx_common_expr",
 /* 355 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTLEFT xx_common_expr",
 /* 356 */ "xx_common_expr ::= xx_common_expr BITWISE_SHIFTRIGHT xx_common_expr",
 /* 357 */ "xx_common_expr ::= xx_common_expr INSTANCEOF xx_common_expr",
 /* 358 */ "xx_fetch_expr ::= FETCH IDENTIFIER COMMA xx_common_expr",
 /* 359 */ "xx_common_expr ::= xx_fetch_expr",
 /* 360 */ "xx_common_expr ::= TYPEOF xx_common_expr",
 /* 361 */ "xx_common_expr ::= IDENTIFIER",
 /* 362 */ "xx_common_expr ::= INTEGER",
 /* 363 */ "xx_common_expr ::= STRING",
 /* 364 */ "xx_common_expr ::= CHAR",
 /* 365 */ "xx_common_expr ::= DOUBLE",
 /* 366 */ "xx_common_expr ::= NULL",
 /* 367 */ "xx_common_expr ::= TRUE",
 /* 368 */ "xx_common_expr ::= FALSE",
 /* 369 */ "xx_common_expr ::= CONSTANT",
 /* 370 */ "xx_common_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 371 */ "xx_common_expr ::= SBRACKET_OPEN xx_array_list SBRACKET_CLOSE",
 /* 372 */ "xx_common_expr ::= NEW IDENTIFIER",
 /* 373 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 374 */ "xx_common_expr ::= NEW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 375 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE",
 /* 376 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 377 */ "xx_common_expr ::= NEW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 378 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 379 */ "xx_fcall_expr ::= IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 380 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 381 */ "xx_fcall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 382 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 383 */ "xx_scall_expr ::= IDENTIFIER DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 384 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 385 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 386 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 387 */ "xx_scall_expr ::= BRACKET_OPEN IDENTIFIER BRACKET_CLOSE DOUBLECOLON BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 388 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 389 */ "xx_mcall_expr ::= xx_common_expr ARROW IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 390 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 391 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN IDENTIFIER BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 392 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN xx_call_parameters PARENTHESES_CLOSE",
 /* 393 */ "xx_mcall_expr ::= xx_common_expr ARROW BRACKET_OPEN STRING BRACKET_CLOSE PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 394 */ "xx_common_expr ::= xx_mcall_expr",
 /* 395 */ "xx_common_expr ::= xx_scall_expr",
 /* 396 */ "xx_common_expr ::= xx_fcall_expr",
 /* 397 */ "xx_common_expr ::= xx_common_expr QUESTION xx_common_expr COLON xx_common_expr",
 /* 398 */ "xx_call_parameters ::= xx_call_parameters COMMA xx_call_parameter",
 /* 399 */ "xx_call_parameters ::= xx_call_parameter",
 /* 400 */ "xx_call_parameter ::= xx_common_expr",
 /* 401 */ "xx_call_parameter ::= IDENTIFIER COLON xx_common_expr",
 /* 402 */ "xx_call_parameter ::= BITWISE_AND xx_common_expr",
 /* 403 */ "xx_call_parameter ::= IDENTIFIER COLON BITWISE_AND xx_common_expr",
 /* 404 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 405 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 406 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN BRACKET_CLOSE",
 /* 407 */ "xx_common_expr ::= FUNCTION PARENTHESES_OPEN xx_parameter_list PARENTHESES_CLOSE BRACKET_OPEN xx_statement_list BRACKET_CLOSE",
 /* 408 */ "xx_array_list ::= xx_array_list COMMA xx_array_item",
 /* 409 */ "xx_array_list ::= xx_array_item",
 /* 410 */ "xx_array_item ::= xx_array_key COLON xx_array_value",
 /* 411 */ "xx_array_item ::= xx_array_value",
 /* 412 */ "xx_array_key ::= CONSTANT",
 /* 413 */ "xx_array_key ::= IDENTIFIER",
 /* 414 */ "xx_array_key ::= STRING",
 /* 415 */ "xx_array_key ::= INTEGER",
 /* 416 */ "xx_array_value ::= xx_common_expr",
 /* 417 */ "xx_literal_expr ::= INTEGER",
 /* 418 */ "xx_literal_expr ::= CHAR",
 /* 419 */ "xx_literal_expr ::= STRING",
 /* 420 */ "xx_literal_expr ::= DOUBLE",
 /* 421 */ "xx_literal_expr ::= NULL",
 /* 422 */ "xx_literal_expr ::= FALSE",
 /* 423 */ "xx_literal_expr ::= TRUE",
 /* 424 */ "xx_literal_expr ::= IDENTIFIER DOUBLECOLON CONSTANT",
 /* 425 */ "xx_literal_expr ::= CONSTANT",
 /* 426 */ "xx_literal_expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /* 427 */ "xx_literal_expr ::= SBRACKET_OPEN xx_literal_array_list SBRACKET_CLOSE",
 /* 428 */ "xx_literal_array_list ::= xx_literal_array_list COMMA xx_literal_array_item",
 /* 429 */ "xx_literal_array_list ::= xx_literal_array_item",
 /* 430 */ "xx_literal_array_item ::= xx_literal_array_key COLON xx_literal_array_value",
 /* 431 */ "xx_literal_array_item ::= xx_literal_array_value",
 /* 432 */ "xx_literal_array_key ::= IDENTIFIER",
 /* 433 */ "xx_literal_array_key ::= STRING",
 /* 434 */ "xx_literal_array_key ::= INTEGER",
 /* 435 */ "xx_literal_array_value ::= xx_literal_expr",
 /* 436 */ "xx_eval_expr ::= xx_common_expr",
 /* 437 */ "xx_comment ::= COMMENT",
 /* 438 */ "xx_cblock ::= CBLOCK",
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
// 1338 "parser.lemon"
{
	if ((yypminor->yy0)) {
		if ((yypminor->yy0)->free_flag) {
			
		}
		delete (yypminor->yy0);
	}
}
// 4518 "parser.cpp"
      break;
    case 122:
// 1351 "parser.lemon"
{ delete (yypminor->yy262); }
// 4523 "parser.cpp"
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
  { 125, 3 },
  { 125, 3 },
  { 150, 3 },
  { 150, 1 },
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
  { 152, 2 },
  { 152, 3 },
  { 153, 3 },
  { 153, 1 },
  { 155, 1 },
  { 151, 2 },
  { 151, 3 },
  { 154, 1 },
  { 154, 1 },
  { 154, 1 },
  { 154, 2 },
  { 154, 2 },
  { 154, 2 },
  { 154, 2 },
  { 154, 3 },
  { 154, 3 },
  { 156, 1 },
  { 156, 1 },
  { 156, 2 },
  { 157, 2 },
  { 157, 1 },
  { 161, 4 },
  { 161, 3 },
  { 161, 6 },
  { 161, 5 },
  { 161, 5 },
  { 161, 4 },
  { 161, 7 },
  { 161, 6 },
  { 164, 2 },
  { 164, 3 },
  { 165, 3 },
  { 165, 1 },
  { 166, 1 },
  { 166, 2 },
  { 158, 2 },
  { 158, 1 },
  { 159, 2 },
  { 159, 1 },
  { 160, 2 },
  { 160, 1 },
  { 167, 6 },
  { 167, 5 },
  { 167, 6 },
  { 167, 5 },
  { 168, 7 },
  { 168, 6 },
  { 168, 8 },
  { 168, 7 },
  { 168, 8 },
  { 168, 9 },
  { 168, 8 },
  { 168, 7 },
  { 168, 9 },
  { 168, 8 },
  { 168, 9 },
  { 168, 10 },
  { 168, 9 },
  { 168, 8 },
  { 168, 10 },
  { 168, 9 },
  { 168, 10 },
  { 168, 11 },
  { 168, 10 },
  { 168, 9 },
  { 168, 11 },
  { 168, 10 },
  { 168, 11 },
  { 168, 12 },
  { 169, 8 },
  { 169, 9 },
  { 169, 9 },
  { 169, 10 },
  { 169, 6 },
  { 169, 7 },
  { 169, 7 },
  { 169, 8 },
  { 162, 2 },
  { 162, 1 },
  { 173, 1 },
  { 173, 1 },
  { 173, 1 },
  { 173, 1 },
  { 173, 1 },
  { 173, 1 },
  { 173, 1 },
  { 173, 1 },
  { 173, 1 },
  { 172, 1 },
  { 172, 1 },
  { 174, 3 },
  { 174, 1 },
  { 175, 1 },
  { 175, 1 },
  { 175, 1 },
  { 175, 2 },
  { 175, 1 },
  { 175, 1 },
  { 170, 3 },
  { 170, 1 },
  { 179, 1 },
  { 179, 2 },
  { 179, 2 },
  { 179, 3 },
  { 179, 3 },
  { 179, 4 },
  { 179, 2 },
  { 179, 3 },
  { 179, 3 },
  { 179, 4 },
  { 179, 4 },
  { 179, 5 },
  { 179, 5 },
  { 179, 6 },
  { 179, 4 },
  { 179, 5 },
  { 177, 3 },
  { 178, 5 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 176, 1 },
  { 171, 2 },
  { 171, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 180, 1 },
  { 182, 1 },
  { 143, 2 },
  { 144, 2 },
  { 132, 4 },
  { 132, 5 },
  { 132, 7 },
  { 132, 8 },
  { 132, 5 },
  { 132, 6 },
  { 132, 9 },
  { 132, 10 },
  { 132, 8 },
  { 132, 9 },
  { 132, 8 },
  { 184, 2 },
  { 184, 1 },
  { 185, 4 },
  { 185, 5 },
  { 148, 4 },
  { 148, 5 },
  { 186, 2 },
  { 186, 1 },
  { 187, 3 },
  { 187, 4 },
  { 187, 3 },
  { 133, 3 },
  { 133, 4 },
  { 145, 4 },
  { 145, 5 },
  { 146, 6 },
  { 146, 7 },
  { 147, 3 },
  { 147, 4 },
  { 147, 5 },
  { 188, 2 },
  { 188, 1 },
  { 189, 5 },
  { 189, 4 },
  { 189, 6 },
  { 189, 7 },
  { 190, 3 },
  { 190, 1 },
  { 191, 1 },
  { 149, 7 },
  { 149, 6 },
  { 149, 8 },
  { 149, 9 },
  { 149, 8 },
  { 149, 10 },
  { 131, 3 },
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
  { 194, 2 },
  { 194, 2 },
  { 194, 5 },
  { 194, 5 },
  { 199, 1 },
  { 134, 3 },
  { 200, 3 },
  { 200, 1 },
  { 201, 1 },
  { 181, 2 },
  { 138, 2 },
  { 139, 2 },
  { 137, 2 },
  { 135, 3 },
  { 135, 2 },
  { 136, 3 },
  { 140, 3 },
  { 141, 3 },
  { 142, 3 },
  { 142, 3 },
  { 142, 3 },
  { 142, 3 },
  { 142, 3 },
  { 142, 3 },
  { 142, 3 },
  { 142, 3 },
  { 142, 3 },
  { 142, 3 },
  { 142, 3 },
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
  { 163, 1 },
  { 163, 1 },
  { 163, 1 },
  { 163, 1 },
  { 163, 1 },
  { 163, 1 },
  { 163, 1 },
  { 163, 3 },
  { 163, 1 },
  { 163, 2 },
  { 163, 3 },
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
// 1347 "parser.lemon"
{
	status->ret = yymsp[0].minor.yy262;
}
// 5179 "parser.cpp"
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
      case 182:
      case 183:
      case 184:
      case 185:
      case 186:
      case 187:
      case 188:
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
      case 289:
      case 293:
      case 318:
      case 359:
      case 394:
      case 395:
      case 396:
      case 416:
      case 435:
      case 436:
// 1353 "parser.lemon"
{
	yygotominor.yy262 = yymsp[0].minor.yy262;
}
// 5244 "parser.cpp"
        break;
      case 2:
      case 67:
      case 83:
      case 85:
      case 87:
      case 125:
      case 180:
      case 219:
      case 225:
      case 239:
      case 278:
// 1357 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_list(yymsp[-1].minor.yy262, yymsp[0].minor.yy262);
}
// 5261 "parser.cpp"
        break;
      case 3:
      case 32:
      case 51:
      case 68:
      case 80:
      case 84:
      case 86:
      case 88:
      case 126:
      case 139:
      case 147:
      case 181:
      case 220:
      case 226:
      case 240:
      case 246:
      case 256:
      case 279:
      case 292:
      case 315:
      case 399:
      case 409:
      case 429:
// 1361 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_list(NULL, yymsp[0].minor.yy262);
}
// 5290 "parser.cpp"
        break;
      case 29:
// 1465 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_namespace(yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(43,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5299 "parser.cpp"
        break;
      case 30:
// 1469 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_use_aliases(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(46,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5308 "parser.cpp"
        break;
      case 31:
      case 50:
      case 79:
      case 146:
      case 255:
      case 291:
      case 314:
      case 398:
      case 408:
      case 428:
// 1473 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_list(yymsp[-2].minor.yy262, yymsp[0].minor.yy262);
  yy_destructor(6,&yymsp[-1].minor);
}
// 5325 "parser.cpp"
        break;
      case 33:
// 1481 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_use_aliases_item(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 5332 "parser.cpp"
        break;
      case 34:
// 1485 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_use_aliases_item(yymsp[-2].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
  yy_destructor(47,&yymsp[-1].minor);
}
// 5340 "parser.cpp"
        break;
      case 35:
// 1489 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_interface(yymsp[-1].minor.yy0, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(48,&yymsp[-2].minor);
}
// 5348 "parser.cpp"
        break;
      case 36:
// 1493 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_interface(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(48,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5357 "parser.cpp"
        break;
      case 37:
// 1497 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy262, 0, 0, NULL, NULL, status->scanner_state);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5365 "parser.cpp"
        break;
      case 38:
// 1501 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 0, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5374 "parser.cpp"
        break;
      case 39:
// 1505 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 0, 0, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5383 "parser.cpp"
        break;
      case 40:
// 1509 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy262, 0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(50,&yymsp[-6].minor);
  yy_destructor(49,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5393 "parser.cpp"
        break;
      case 41:
// 1513 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy262, 1, 0, NULL, NULL, status->scanner_state);
  yy_destructor(52,&yymsp[-3].minor);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5402 "parser.cpp"
        break;
      case 42:
// 1517 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 1, 0, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(52,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5412 "parser.cpp"
        break;
      case 43:
// 1521 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 1, 0, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(52,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5422 "parser.cpp"
        break;
      case 44:
// 1525 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-5].minor.yy0, yymsp[0].minor.yy262, 1, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(52,&yymsp[-7].minor);
  yy_destructor(50,&yymsp[-6].minor);
  yy_destructor(49,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5433 "parser.cpp"
        break;
      case 45:
// 1529 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-1].minor.yy0, yymsp[0].minor.yy262, 0, 1, NULL, NULL, status->scanner_state);
  yy_destructor(53,&yymsp[-3].minor);
  yy_destructor(50,&yymsp[-2].minor);
}
// 5442 "parser.cpp"
        break;
      case 46:
// 1533 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 0, 1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(53,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(49,&yymsp[-2].minor);
}
// 5452 "parser.cpp"
        break;
      case 47:
// 1537 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, 0, 1, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(53,&yymsp[-5].minor);
  yy_destructor(50,&yymsp[-4].minor);
  yy_destructor(51,&yymsp[-2].minor);
}
// 5462 "parser.cpp"
        break;
      case 48:
      case 77:
// 1541 "parser.lemon"
{
	yygotominor.yy262 = NULL;
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5472 "parser.cpp"
        break;
      case 49:
      case 78:
// 1545 "parser.lemon"
{
	yygotominor.yy262 = yymsp[-1].minor.yy262;
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5482 "parser.cpp"
        break;
      case 52:
      case 247:
      case 361:
      case 413:
      case 432:
// 1557 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state);
}
// 5493 "parser.cpp"
        break;
      case 53:
// 1561 "parser.lemon"
{
  yygotominor.yy262 = NULL;
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5502 "parser.cpp"
        break;
      case 54:
// 1565 "parser.lemon"
{
  yygotominor.yy262 = yymsp[-1].minor.yy262;
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5511 "parser.cpp"
        break;
      case 55:
// 1569 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
}
// 5518 "parser.cpp"
        break;
      case 56:
// 1573 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 5525 "parser.cpp"
        break;
      case 57:
// 1577 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(NULL, yymsp[0].minor.yy262, NULL, status->scanner_state);
}
// 5532 "parser.cpp"
        break;
      case 58:
// 1581 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[-1].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
}
// 5539 "parser.cpp"
        break;
      case 59:
// 1585 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[-1].minor.yy262, NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 5546 "parser.cpp"
        break;
      case 60:
// 1589 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[0].minor.yy262, NULL, yymsp[-1].minor.yy262, status->scanner_state);
}
// 5553 "parser.cpp"
        break;
      case 61:
// 1593 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(NULL, yymsp[0].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
}
// 5560 "parser.cpp"
        break;
      case 62:
// 1597 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[-2].minor.yy262, yymsp[0].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
}
// 5567 "parser.cpp"
        break;
      case 63:
// 1601 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_definition(yymsp[-1].minor.yy262, yymsp[0].minor.yy262, yymsp[-2].minor.yy262, status->scanner_state);
}
// 5574 "parser.cpp"
        break;
      case 64:
// 1605 "parser.lemon"
{
  yygotominor.yy262 = xx_ret_interface_definition(NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 5581 "parser.cpp"
        break;
      case 65:
// 1609 "parser.lemon"
{
  yygotominor.yy262 = xx_ret_interface_definition(yymsp[0].minor.yy262, NULL, status->scanner_state);
}
// 5588 "parser.cpp"
        break;
      case 66:
// 1613 "parser.lemon"
{
  yygotominor.yy262 = xx_ret_interface_definition(yymsp[0].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
}
// 5595 "parser.cpp"
        break;
      case 69:
// 1626 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-2].minor.yy262, yymsp[-1].minor.yy0, NULL, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5603 "parser.cpp"
        break;
      case 70:
// 1630 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-2].minor.yy262, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5611 "parser.cpp"
        break;
      case 71:
// 1634 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-4].minor.yy262, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, NULL, status->scanner_state);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5620 "parser.cpp"
        break;
      case 72:
// 1638 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-4].minor.yy262, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5629 "parser.cpp"
        break;
      case 73:
// 1642 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-3].minor.yy262, yymsp[-2].minor.yy0, NULL, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5637 "parser.cpp"
        break;
      case 74:
// 1646 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-3].minor.yy262, yymsp[-2].minor.yy0, NULL, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 5645 "parser.cpp"
        break;
      case 75:
// 1650 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-5].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy262, yymsp[-6].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(57,&yymsp[-3].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5654 "parser.cpp"
        break;
      case 76:
// 1654 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_property(yymsp[-5].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy262, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(57,&yymsp[-3].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5663 "parser.cpp"
        break;
      case 81:
// 1674 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_property_shortcut(NULL, yymsp[0].minor.yy0, status->scanner_state);
}
// 5670 "parser.cpp"
        break;
      case 82:
// 1678 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_property_shortcut(yymsp[-1].minor.yy0, yymsp[0].minor.yy0, status->scanner_state);
}
// 5677 "parser.cpp"
        break;
      case 89:
      case 91:
// 1707 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5688 "parser.cpp"
        break;
      case 90:
      case 92:
// 1711 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_const(yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5699 "parser.cpp"
        break;
      case 93:
// 1727 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-6].minor.yy262, yymsp[-4].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5711 "parser.cpp"
        break;
      case 94:
      case 121:
// 1732 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-5].minor.yy262, yymsp[-3].minor.yy0, NULL, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5723 "parser.cpp"
        break;
      case 95:
// 1737 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, yymsp[-3].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5735 "parser.cpp"
        break;
      case 96:
      case 122:
// 1742 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-6].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5747 "parser.cpp"
        break;
      case 97:
// 1747 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5759 "parser.cpp"
        break;
      case 98:
// 1751 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy262, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5771 "parser.cpp"
        break;
      case 99:
// 1755 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-6].minor.yy262, yymsp[-4].minor.yy0, NULL, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5783 "parser.cpp"
        break;
      case 100:
      case 123:
// 1759 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-5].minor.yy262, yymsp[-3].minor.yy0, NULL, NULL, yymsp[-6].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5795 "parser.cpp"
        break;
      case 101:
// 1763 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, yymsp[-3].minor.yy262, NULL, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5807 "parser.cpp"
        break;
      case 102:
      case 124:
// 1767 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-6].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy262, NULL, yymsp[-7].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5819 "parser.cpp"
        break;
      case 103:
// 1771 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, NULL, yymsp[-1].minor.yy262, yymsp[-8].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5831 "parser.cpp"
        break;
      case 104:
// 1775 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy262, yymsp[-1].minor.yy262, yymsp[-9].minor.yy0, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5843 "parser.cpp"
        break;
      case 105:
// 1779 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, NULL, NULL, NULL, yymsp[-2].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5856 "parser.cpp"
        break;
      case 106:
      case 117:
// 1783 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, NULL, NULL, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5869 "parser.cpp"
        break;
      case 107:
// 1787 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-9].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy262, NULL, NULL, yymsp[-2].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5882 "parser.cpp"
        break;
      case 108:
      case 118:
// 1791 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy262, NULL, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5895 "parser.cpp"
        break;
      case 109:
// 1795 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-9].minor.yy262, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy262, NULL, yymsp[-3].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5908 "parser.cpp"
        break;
      case 110:
// 1799 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-10].minor.yy262, yymsp[-8].minor.yy0, yymsp[-6].minor.yy262, yymsp[-1].minor.yy262, NULL, yymsp[-3].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-9].minor);
  yy_destructor(61,&yymsp[-7].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5921 "parser.cpp"
        break;
      case 111:
// 1803 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, NULL, NULL, yymsp[-9].minor.yy0, yymsp[-2].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5934 "parser.cpp"
        break;
      case 112:
      case 119:
// 1807 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-7].minor.yy262, yymsp[-5].minor.yy0, NULL, NULL, yymsp[-8].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5947 "parser.cpp"
        break;
      case 113:
// 1811 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-9].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy262, NULL, yymsp[-10].minor.yy0, yymsp[-2].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-4].minor);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5960 "parser.cpp"
        break;
      case 114:
      case 120:
// 1815 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-8].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy262, NULL, yymsp[-9].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-7].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 5973 "parser.cpp"
        break;
      case 115:
// 1819 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-9].minor.yy262, yymsp[-7].minor.yy0, NULL, yymsp[-1].minor.yy262, yymsp[-10].minor.yy0, yymsp[-3].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-8].minor);
  yy_destructor(61,&yymsp[-6].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5986 "parser.cpp"
        break;
      case 116:
// 1823 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_class_method(yymsp[-10].minor.yy262, yymsp[-8].minor.yy0, yymsp[-6].minor.yy262, yymsp[-1].minor.yy262, yymsp[-11].minor.yy0, yymsp[-3].minor.yy262, status->scanner_state);
  yy_destructor(60,&yymsp[-9].minor);
  yy_destructor(61,&yymsp[-7].minor);
  yy_destructor(40,&yymsp[-5].minor);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 5999 "parser.cpp"
        break;
      case 127:
// 1869 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("public");
  yy_destructor(1,&yymsp[0].minor);
}
// 6007 "parser.cpp"
        break;
      case 128:
// 1873 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("protected");
  yy_destructor(2,&yymsp[0].minor);
}
// 6015 "parser.cpp"
        break;
      case 129:
// 1877 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("private");
  yy_destructor(4,&yymsp[0].minor);
}
// 6023 "parser.cpp"
        break;
      case 130:
// 1881 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("static");
  yy_destructor(3,&yymsp[0].minor);
}
// 6031 "parser.cpp"
        break;
      case 131:
// 1885 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("scoped");
  yy_destructor(5,&yymsp[0].minor);
}
// 6039 "parser.cpp"
        break;
      case 132:
// 1889 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("inline");
  yy_destructor(62,&yymsp[0].minor);
}
// 6047 "parser.cpp"
        break;
      case 133:
// 1893 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("deprecated");
  yy_destructor(63,&yymsp[0].minor);
}
// 6055 "parser.cpp"
        break;
      case 134:
// 1897 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("abstract");
  yy_destructor(52,&yymsp[0].minor);
}
// 6063 "parser.cpp"
        break;
      case 135:
// 1901 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("final");
  yy_destructor(53,&yymsp[0].minor);
}
// 6071 "parser.cpp"
        break;
      case 136:
// 1906 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type(1, NULL, status->scanner_state);
  yy_destructor(64,&yymsp[0].minor);
}
// 6079 "parser.cpp"
        break;
      case 137:
// 1910 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type(0, yymsp[0].minor.yy262, status->scanner_state);
}
// 6086 "parser.cpp"
        break;
      case 138:
      case 245:
// 1914 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_list(yymsp[-2].minor.yy262, yymsp[0].minor.yy262);
  yy_destructor(14,&yymsp[-1].minor);
}
// 6095 "parser.cpp"
        break;
      case 140:
// 1922 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(yymsp[0].minor.yy262, NULL, 0, 0, status->scanner_state);
}
// 6102 "parser.cpp"
        break;
      case 141:
// 1926 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_NULL), NULL, 0, 0, status->scanner_state);
  yy_destructor(65,&yymsp[0].minor);
}
// 6110 "parser.cpp"
        break;
      case 142:
// 1930 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(xx_ret_type(XX_T_TYPE_THIS), NULL, 0, 0, status->scanner_state);
  yy_destructor(66,&yymsp[0].minor);
}
// 6118 "parser.cpp"
        break;
      case 143:
// 1934 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(yymsp[-1].minor.yy262, NULL, 1, 0, status->scanner_state);
  yy_destructor(39,&yymsp[0].minor);
}
// 6126 "parser.cpp"
        break;
      case 144:
// 1938 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy262, 0, 0, status->scanner_state);
}
// 6133 "parser.cpp"
        break;
      case 145:
// 1942 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_type_item(NULL, yymsp[0].minor.yy262, 0, 1, status->scanner_state);
}
// 6140 "parser.cpp"
        break;
      case 148:
// 1956 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6147 "parser.cpp"
        break;
      case 149:
// 1960 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, NULL, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-1].minor);
}
// 6155 "parser.cpp"
        break;
      case 150:
// 1964 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, yymsp[-1].minor.yy262, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6162 "parser.cpp"
        break;
      case 151:
// 1968 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, yymsp[-1].minor.yy262, NULL, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-2].minor);
}
// 6170 "parser.cpp"
        break;
      case 152:
// 1972 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, yymsp[-2].minor.yy262, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(39,&yymsp[-1].minor);
}
// 6178 "parser.cpp"
        break;
      case 153:
// 1976 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, yymsp[-2].minor.yy262, NULL, yymsp[0].minor.yy0, NULL, 1, status->scanner_state);
  yy_destructor(58,&yymsp[-3].minor);
  yy_destructor(39,&yymsp[-1].minor);
}
// 6187 "parser.cpp"
        break;
      case 154:
// 1980 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, NULL, yymsp[-1].minor.yy262, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
}
// 6194 "parser.cpp"
        break;
      case 155:
// 1984 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, NULL, yymsp[-1].minor.yy262, yymsp[0].minor.yy0, NULL, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-2].minor);
}
// 6202 "parser.cpp"
        break;
      case 156:
// 1988 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6210 "parser.cpp"
        break;
      case 157:
// 1992 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, NULL, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6219 "parser.cpp"
        break;
      case 158:
// 1996 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, yymsp[-3].minor.yy262, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6227 "parser.cpp"
        break;
      case 159:
// 2000 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, yymsp[-3].minor.yy262, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6236 "parser.cpp"
        break;
      case 160:
// 2004 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, yymsp[-4].minor.yy262, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 1, status->scanner_state);
  yy_destructor(39,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6245 "parser.cpp"
        break;
      case 161:
// 2008 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, yymsp[-4].minor.yy262, NULL, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 1, status->scanner_state);
  yy_destructor(58,&yymsp[-5].minor);
  yy_destructor(39,&yymsp[-3].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6255 "parser.cpp"
        break;
      case 162:
// 2012 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(0, NULL, yymsp[-3].minor.yy262, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6263 "parser.cpp"
        break;
      case 163:
// 2016 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_parameter(1, NULL, yymsp[-3].minor.yy262, yymsp[-2].minor.yy0, yymsp[0].minor.yy262, 0, status->scanner_state);
  yy_destructor(58,&yymsp[-4].minor);
  yy_destructor(57,&yymsp[-1].minor);
}
// 6272 "parser.cpp"
        break;
      case 164:
// 2021 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state);
  yy_destructor(21,&yymsp[-2].minor);
  yy_destructor(22,&yymsp[0].minor);
}
// 6281 "parser.cpp"
        break;
      case 165:
// 2025 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state);
  yy_destructor(21,&yymsp[-4].minor);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[-1].minor);
  yy_destructor(22,&yymsp[0].minor);
}
// 6292 "parser.cpp"
        break;
      case 166:
// 2029 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_INTEGER);
  yy_destructor(68,&yymsp[0].minor);
}
// 6300 "parser.cpp"
        break;
      case 167:
// 2033 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_UINTEGER);
  yy_destructor(69,&yymsp[0].minor);
}
// 6308 "parser.cpp"
        break;
      case 168:
// 2037 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_LONG);
  yy_destructor(70,&yymsp[0].minor);
}
// 6316 "parser.cpp"
        break;
      case 169:
// 2041 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_ULONG);
  yy_destructor(71,&yymsp[0].minor);
}
// 6324 "parser.cpp"
        break;
      case 170:
// 2045 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_CHAR);
  yy_destructor(72,&yymsp[0].minor);
}
// 6332 "parser.cpp"
        break;
      case 171:
// 2049 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_UCHAR);
  yy_destructor(73,&yymsp[0].minor);
}
// 6340 "parser.cpp"
        break;
      case 172:
// 2053 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_DOUBLE);
  yy_destructor(74,&yymsp[0].minor);
}
// 6348 "parser.cpp"
        break;
      case 173:
// 2057 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_BOOL);
  yy_destructor(75,&yymsp[0].minor);
}
// 6356 "parser.cpp"
        break;
      case 174:
// 2061 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_STRING);
  yy_destructor(76,&yymsp[0].minor);
}
// 6364 "parser.cpp"
        break;
      case 175:
// 2065 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_ARRAY);
  yy_destructor(77,&yymsp[0].minor);
}
// 6372 "parser.cpp"
        break;
      case 176:
// 2069 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_VAR);
  yy_destructor(78,&yymsp[0].minor);
}
// 6380 "parser.cpp"
        break;
      case 177:
// 2073 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_CALLABLE);
  yy_destructor(79,&yymsp[0].minor);
}
// 6388 "parser.cpp"
        break;
      case 178:
// 2077 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_RESOURCE);
  yy_destructor(80,&yymsp[0].minor);
}
// 6396 "parser.cpp"
        break;
      case 179:
// 2081 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_type(XX_TYPE_OBJECT);
  yy_destructor(81,&yymsp[0].minor);
}
// 6404 "parser.cpp"
        break;
      case 205:
// 2185 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_empty_statement(status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 6412 "parser.cpp"
        break;
      case 206:
// 2189 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_break_statement(status->scanner_state);
  yy_destructor(82,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6421 "parser.cpp"
        break;
      case 207:
// 2193 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_continue_statement(status->scanner_state);
  yy_destructor(83,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6430 "parser.cpp"
        break;
      case 208:
// 2198 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-2].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6440 "parser.cpp"
        break;
      case 209:
// 2203 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-3].minor.yy262, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(84,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6450 "parser.cpp"
        break;
      case 210:
// 2208 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-5].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6463 "parser.cpp"
        break;
      case 211:
// 2213 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-6].minor.yy262, NULL, yymsp[-3].minor.yy262, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6476 "parser.cpp"
        break;
      case 212:
// 2218 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6486 "parser.cpp"
        break;
      case 213:
// 2223 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-4].minor.yy262, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-3].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6496 "parser.cpp"
        break;
      case 214:
// 2228 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-7].minor.yy262, yymsp[-5].minor.yy262, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(84,&yymsp[-8].minor);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6509 "parser.cpp"
        break;
      case 215:
// 2233 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-8].minor.yy262, yymsp[-6].minor.yy262, yymsp[-4].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(84,&yymsp[-9].minor);
  yy_destructor(54,&yymsp[-7].minor);
  yy_destructor(55,&yymsp[-5].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6522 "parser.cpp"
        break;
      case 216:
// 2238 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-6].minor.yy262, yymsp[-4].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6535 "parser.cpp"
        break;
      case 217:
// 2243 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-7].minor.yy262, yymsp[-5].minor.yy262, yymsp[-3].minor.yy262, NULL, status->scanner_state);
  yy_destructor(84,&yymsp[-8].minor);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6548 "parser.cpp"
        break;
      case 218:
// 2248 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-6].minor.yy262, NULL, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(84,&yymsp[-7].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(85,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6561 "parser.cpp"
        break;
      case 221:
// 2261 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-2].minor.yy262, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(86,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6571 "parser.cpp"
        break;
      case 222:
// 2266 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_if_statement(yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(86,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6581 "parser.cpp"
        break;
      case 223:
// 2270 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_switch_statement(yymsp[-2].minor.yy262, NULL, status->scanner_state);
  yy_destructor(87,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6591 "parser.cpp"
        break;
      case 224:
// 2274 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_switch_statement(yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(87,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6601 "parser.cpp"
        break;
      case 227:
// 2286 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_case_clause(yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(88,&yymsp[-2].minor);
  yy_destructor(89,&yymsp[0].minor);
}
// 6610 "parser.cpp"
        break;
      case 228:
// 2290 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_case_clause(yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(88,&yymsp[-3].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 6619 "parser.cpp"
        break;
      case 229:
// 2294 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_case_clause(NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(90,&yymsp[-2].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 6628 "parser.cpp"
        break;
      case 230:
// 2298 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_loop_statement(NULL, status->scanner_state);
  yy_destructor(91,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6638 "parser.cpp"
        break;
      case 231:
// 2302 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_loop_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(91,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6648 "parser.cpp"
        break;
      case 232:
// 2306 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_while_statement(yymsp[-2].minor.yy262, NULL, status->scanner_state);
  yy_destructor(92,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6658 "parser.cpp"
        break;
      case 233:
// 2310 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_while_statement(yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(92,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6668 "parser.cpp"
        break;
      case 234:
// 2314 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_do_while_statement(yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(93,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(92,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6680 "parser.cpp"
        break;
      case 235:
// 2318 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_do_while_statement(yymsp[-1].minor.yy262, yymsp[-4].minor.yy262, status->scanner_state);
  yy_destructor(93,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(92,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6692 "parser.cpp"
        break;
      case 236:
// 2322 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_try_catch_statement(NULL, NULL, status->scanner_state);
  yy_destructor(94,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6702 "parser.cpp"
        break;
      case 237:
// 2326 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_try_catch_statement(yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(94,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6712 "parser.cpp"
        break;
      case 238:
// 2330 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_try_catch_statement(yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(94,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-3].minor);
  yy_destructor(55,&yymsp[-1].minor);
}
// 6722 "parser.cpp"
        break;
      case 241:
// 2342 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_catch_statement(yymsp[-3].minor.yy262, NULL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(95,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6732 "parser.cpp"
        break;
      case 242:
// 2346 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_catch_statement(yymsp[-2].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(95,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6742 "parser.cpp"
        break;
      case 243:
// 2350 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_catch_statement(yymsp[-4].minor.yy262, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(95,&yymsp[-5].minor);
  yy_destructor(6,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6753 "parser.cpp"
        break;
      case 244:
// 2354 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_catch_statement(yymsp[-5].minor.yy262, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-3].minor.yy0, status->scanner_state), yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(95,&yymsp[-6].minor);
  yy_destructor(6,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6764 "parser.cpp"
        break;
      case 248:
// 2370 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-3].minor.yy262, NULL, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(96,&yymsp[-6].minor);
  yy_destructor(97,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6775 "parser.cpp"
        break;
      case 249:
// 2374 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-2].minor.yy262, NULL, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(96,&yymsp[-5].minor);
  yy_destructor(97,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6786 "parser.cpp"
        break;
      case 250:
// 2378 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-3].minor.yy262, NULL, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(96,&yymsp[-7].minor);
  yy_destructor(97,&yymsp[-5].minor);
  yy_destructor(98,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6798 "parser.cpp"
        break;
      case 251:
// 2382 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-3].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, 0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(96,&yymsp[-8].minor);
  yy_destructor(6,&yymsp[-6].minor);
  yy_destructor(97,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6810 "parser.cpp"
        break;
      case 252:
// 2386 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-2].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, 0, NULL, status->scanner_state);
  yy_destructor(96,&yymsp[-7].minor);
  yy_destructor(6,&yymsp[-5].minor);
  yy_destructor(97,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6822 "parser.cpp"
        break;
      case 253:
// 2390 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_for_statement(yymsp[-3].minor.yy262, yymsp[-8].minor.yy0, yymsp[-6].minor.yy0, 1, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(96,&yymsp[-9].minor);
  yy_destructor(6,&yymsp[-7].minor);
  yy_destructor(97,&yymsp[-5].minor);
  yy_destructor(98,&yymsp[-4].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 6835 "parser.cpp"
        break;
      case 254:
// 2394 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(99,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 6844 "parser.cpp"
        break;
      case 257:
// 2407 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("assign");
  yy_destructor(57,&yymsp[0].minor);
}
// 6852 "parser.cpp"
        break;
      case 258:
// 2412 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("add-assign");
  yy_destructor(100,&yymsp[0].minor);
}
// 6860 "parser.cpp"
        break;
      case 259:
// 2417 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("sub-assign");
  yy_destructor(101,&yymsp[0].minor);
}
// 6868 "parser.cpp"
        break;
      case 260:
// 2421 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("mul-assign");
  yy_destructor(102,&yymsp[0].minor);
}
// 6876 "parser.cpp"
        break;
      case 261:
// 2425 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("div-assign");
  yy_destructor(103,&yymsp[0].minor);
}
// 6884 "parser.cpp"
        break;
      case 262:
// 2429 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("concat-assign");
  yy_destructor(104,&yymsp[0].minor);
}
// 6892 "parser.cpp"
        break;
      case 263:
// 2433 "parser.lemon"
{
	yygotominor.yy262 = new Json::Value("mod-assign");
  yy_destructor(105,&yymsp[0].minor);
}
// 6900 "parser.cpp"
        break;
      case 264:
// 2438 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("variable", yymsp[-1].minor.yy262, yymsp[-2].minor.yy0, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 6907 "parser.cpp"
        break;
      case 265:
// 2443 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property", yymsp[-1].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
}
// 6915 "parser.cpp"
        break;
      case 266:
// 2448 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("variable-dynamic-object-property", yymsp[-1].minor.yy262, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 6925 "parser.cpp"
        break;
      case 267:
// 2453 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("string-dynamic-object-property", yymsp[-1].minor.yy262, yymsp[-6].minor.yy0, yymsp[-3].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 6935 "parser.cpp"
        break;
      case 268:
// 2458 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-append", yymsp[-1].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6945 "parser.cpp"
        break;
      case 269:
// 2463 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-array-index", yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-4].minor);
}
// 6953 "parser.cpp"
        break;
      case 270:
// 2467 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-array-index-append", yymsp[-1].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6963 "parser.cpp"
        break;
      case 271:
// 2472 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("static-property", yymsp[-1].minor.yy262, yymsp[-4].minor.yy0, yymsp[-2].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-3].minor);
}
// 6971 "parser.cpp"
        break;
      case 272:
// 2477 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("static-property-append", yymsp[-1].minor.yy262, yymsp[-6].minor.yy0, yymsp[-4].minor.yy0, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-5].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6981 "parser.cpp"
        break;
      case 273:
// 2482 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("static-property-array-index", yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, yymsp[-3].minor.yy0, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-4].minor);
}
// 6989 "parser.cpp"
        break;
      case 274:
// 2487 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("static-property-array-index-append", yymsp[-1].minor.yy262, yymsp[-7].minor.yy0, yymsp[-5].minor.yy0, yymsp[-4].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-6].minor);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 6999 "parser.cpp"
        break;
      case 275:
// 2492 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("variable-append", yymsp[-1].minor.yy262, yymsp[-4].minor.yy0, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7008 "parser.cpp"
        break;
      case 276:
// 2497 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("array-index", yymsp[-1].minor.yy262, yymsp[-3].minor.yy0, NULL, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
}
// 7015 "parser.cpp"
        break;
      case 277:
// 2502 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("array-index-append", yymsp[-1].minor.yy262, yymsp[-5].minor.yy0, NULL, yymsp[-4].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(41,&yymsp[-3].minor);
  yy_destructor(67,&yymsp[-2].minor);
}
// 7024 "parser.cpp"
        break;
      case 280:
// 2514 "parser.lemon"
{
	yygotominor.yy262 = yymsp[-1].minor.yy262;
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7033 "parser.cpp"
        break;
      case 281:
// 2519 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-incr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(108,&yymsp[0].minor);
}
// 7042 "parser.cpp"
        break;
      case 282:
// 2524 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("object-property-decr", NULL, yymsp[-3].minor.yy0, yymsp[-1].minor.yy0, NULL, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-2].minor);
  yy_destructor(109,&yymsp[0].minor);
}
// 7051 "parser.cpp"
        break;
      case 283:
// 2529 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("incr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(108,&yymsp[0].minor);
}
// 7059 "parser.cpp"
        break;
      case 284:
// 2534 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("incr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(108,&yymsp[-1].minor);
}
// 7067 "parser.cpp"
        break;
      case 285:
// 2539 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("decr", NULL, yymsp[-1].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(109,&yymsp[0].minor);
}
// 7075 "parser.cpp"
        break;
      case 286:
// 2544 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("decr", NULL, yymsp[0].minor.yy0, NULL, NULL, NULL, status->scanner_state);
  yy_destructor(109,&yymsp[-1].minor);
}
// 7083 "parser.cpp"
        break;
      case 287:
// 2549 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("dynamic-variable", yymsp[-1].minor.yy262, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7092 "parser.cpp"
        break;
      case 288:
// 2554 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_let_assignment("dynamic-variable-string", yymsp[-1].minor.yy262, yymsp[-3].minor.yy0, NULL, NULL, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
}
// 7101 "parser.cpp"
        break;
      case 290:
// 2562 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_echo_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(110,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7110 "parser.cpp"
        break;
      case 294:
// 2579 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7118 "parser.cpp"
        break;
      case 295:
// 2584 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7126 "parser.cpp"
        break;
      case 296:
// 2589 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7134 "parser.cpp"
        break;
      case 297:
// 2594 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fetch_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(45,&yymsp[0].minor);
}
// 7142 "parser.cpp"
        break;
      case 298:
// 2599 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(111,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7151 "parser.cpp"
        break;
      case 299:
// 2604 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_return_statement(NULL, status->scanner_state);
  yy_destructor(111,&yymsp[-1].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7160 "parser.cpp"
        break;
      case 300:
// 2609 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_require_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(7,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7169 "parser.cpp"
        break;
      case 301:
// 2614 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_unset_statement(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(112,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7178 "parser.cpp"
        break;
      case 302:
// 2619 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_throw_exception(yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(113,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7187 "parser.cpp"
        break;
      case 303:
// 2623 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_INTEGER, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(68,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7196 "parser.cpp"
        break;
      case 304:
// 2627 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_UINTEGER, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(69,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7205 "parser.cpp"
        break;
      case 305:
// 2631 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_CHAR, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(72,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7214 "parser.cpp"
        break;
      case 306:
// 2635 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_UCHAR, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(73,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7223 "parser.cpp"
        break;
      case 307:
// 2639 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_LONG, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(70,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7232 "parser.cpp"
        break;
      case 308:
// 2643 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_ULONG, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(71,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7241 "parser.cpp"
        break;
      case 309:
// 2647 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_DOUBLE, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(74,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7250 "parser.cpp"
        break;
      case 310:
// 2651 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_STRING, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(76,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7259 "parser.cpp"
        break;
      case 311:
// 2655 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_BOOL, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(75,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7268 "parser.cpp"
        break;
      case 312:
// 2659 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_VAR, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(78,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7277 "parser.cpp"
        break;
      case 313:
// 2663 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_statement(XX_T_TYPE_ARRAY, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(77,&yymsp[-2].minor);
  yy_destructor(45,&yymsp[0].minor);
}
// 7286 "parser.cpp"
        break;
      case 316:
// 2675 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_variable(yymsp[0].minor.yy0, NULL, status->scanner_state);
}
// 7293 "parser.cpp"
        break;
      case 317:
// 2679 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_declare_variable(yymsp[-2].minor.yy0, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(57,&yymsp[-1].minor);
}
// 7301 "parser.cpp"
        break;
      case 319:
// 2687 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("not", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(39,&yymsp[-1].minor);
}
// 7309 "parser.cpp"
        break;
      case 320:
// 2691 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("minus", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(28,&yymsp[-1].minor);
}
// 7317 "parser.cpp"
        break;
      case 321:
// 2695 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("isset", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(33,&yymsp[-1].minor);
}
// 7325 "parser.cpp"
        break;
      case 322:
// 2699 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("require", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(7,&yymsp[-1].minor);
}
// 7333 "parser.cpp"
        break;
      case 323:
// 2703 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("clone", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(37,&yymsp[-1].minor);
}
// 7341 "parser.cpp"
        break;
      case 324:
// 2707 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("empty", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(35,&yymsp[-1].minor);
}
// 7349 "parser.cpp"
        break;
      case 325:
// 2711 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("likely", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(9,&yymsp[-1].minor);
}
// 7357 "parser.cpp"
        break;
      case 326:
// 2715 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("unlikely", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(10,&yymsp[-1].minor);
}
// 7365 "parser.cpp"
        break;
      case 327:
// 2719 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("equals", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(19,&yymsp[-1].minor);
}
// 7373 "parser.cpp"
        break;
      case 328:
// 2723 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("not-equals", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(26,&yymsp[-1].minor);
}
// 7381 "parser.cpp"
        break;
      case 329:
// 2727 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("identical", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(20,&yymsp[-1].minor);
}
// 7389 "parser.cpp"
        break;
      case 330:
// 2731 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("not-identical", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(25,&yymsp[-1].minor);
}
// 7397 "parser.cpp"
        break;
      case 331:
// 2735 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("less", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(21,&yymsp[-1].minor);
}
// 7405 "parser.cpp"
        break;
      case 332:
// 2739 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("greater", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(22,&yymsp[-1].minor);
}
// 7413 "parser.cpp"
        break;
      case 333:
// 2743 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("less-equal", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(23,&yymsp[-1].minor);
}
// 7421 "parser.cpp"
        break;
      case 334:
// 2747 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("greater-equal", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(24,&yymsp[-1].minor);
}
// 7429 "parser.cpp"
        break;
      case 335:
// 2751 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("list", yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7438 "parser.cpp"
        break;
      case 336:
// 2755 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("cast", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-1].minor);
}
// 7447 "parser.cpp"
        break;
      case 337:
// 2759 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("type-hint", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(21,&yymsp[-3].minor);
  yy_destructor(22,&yymsp[-1].minor);
}
// 7456 "parser.cpp"
        break;
      case 338:
// 2763 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("property-access", yymsp[-2].minor.yy262, xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-1].minor);
}
// 7464 "parser.cpp"
        break;
      case 339:
// 2767 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("property-dynamic-access", yymsp[-4].minor.yy262, xx_ret_literal(XX_T_IDENTIFIER, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7474 "parser.cpp"
        break;
      case 340:
// 2771 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("property-string-access", yymsp[-4].minor.yy262, xx_ret_literal(XX_T_STRING, yymsp[-1].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7484 "parser.cpp"
        break;
      case 341:
// 2775 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("static-property-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-1].minor);
}
// 7492 "parser.cpp"
        break;
      case 342:
      case 424:
// 2779 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("static-constant-access", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), xx_ret_literal(XX_T_IDENTIFIER, yymsp[0].minor.yy0, status->scanner_state), NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-1].minor);
}
// 7501 "parser.cpp"
        break;
      case 343:
// 2788 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("array-access", yymsp[-3].minor.yy262, yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7510 "parser.cpp"
        break;
      case 344:
// 2793 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("add", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(27,&yymsp[-1].minor);
}
// 7518 "parser.cpp"
        break;
      case 345:
// 2798 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("sub", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(28,&yymsp[-1].minor);
}
// 7526 "parser.cpp"
        break;
      case 346:
// 2803 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("mul", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(30,&yymsp[-1].minor);
}
// 7534 "parser.cpp"
        break;
      case 347:
// 2808 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("div", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(31,&yymsp[-1].minor);
}
// 7542 "parser.cpp"
        break;
      case 348:
// 2813 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("mod", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(32,&yymsp[-1].minor);
}
// 7550 "parser.cpp"
        break;
      case 349:
// 2818 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("concat", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(29,&yymsp[-1].minor);
}
// 7558 "parser.cpp"
        break;
      case 350:
// 2823 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("and", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(13,&yymsp[-1].minor);
}
// 7566 "parser.cpp"
        break;
      case 351:
// 2828 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("or", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(12,&yymsp[-1].minor);
}
// 7574 "parser.cpp"
        break;
      case 352:
// 2833 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_and", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(15,&yymsp[-1].minor);
}
// 7582 "parser.cpp"
        break;
      case 353:
// 2838 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_or", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(14,&yymsp[-1].minor);
}
// 7590 "parser.cpp"
        break;
      case 354:
// 2843 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_xor", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(16,&yymsp[-1].minor);
}
// 7598 "parser.cpp"
        break;
      case 355:
// 2848 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_shiftleft", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(17,&yymsp[-1].minor);
}
// 7606 "parser.cpp"
        break;
      case 356:
// 2853 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("bitwise_shiftright", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(18,&yymsp[-1].minor);
}
// 7614 "parser.cpp"
        break;
      case 357:
// 2858 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("instanceof", yymsp[-2].minor.yy262, yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(11,&yymsp[-1].minor);
}
// 7622 "parser.cpp"
        break;
      case 358:
// 2863 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("fetch", xx_ret_literal(XX_T_IDENTIFIER, yymsp[-2].minor.yy0, status->scanner_state), yymsp[0].minor.yy262, NULL, status->scanner_state);
  yy_destructor(34,&yymsp[-3].minor);
  yy_destructor(6,&yymsp[-1].minor);
}
// 7631 "parser.cpp"
        break;
      case 360:
// 2873 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("typeof", yymsp[0].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(36,&yymsp[-1].minor);
}
// 7639 "parser.cpp"
        break;
      case 362:
      case 415:
      case 417:
      case 434:
// 2883 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_INTEGER, yymsp[0].minor.yy0, status->scanner_state);
}
// 7649 "parser.cpp"
        break;
      case 363:
      case 414:
      case 419:
      case 433:
// 2888 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_STRING, yymsp[0].minor.yy0, status->scanner_state);
}
// 7659 "parser.cpp"
        break;
      case 364:
      case 418:
// 2893 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_CHAR, yymsp[0].minor.yy0, status->scanner_state);
}
// 7667 "parser.cpp"
        break;
      case 365:
      case 420:
// 2898 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_DOUBLE, yymsp[0].minor.yy0, status->scanner_state);
}
// 7675 "parser.cpp"
        break;
      case 366:
      case 421:
// 2903 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_NULL, NULL, status->scanner_state);
  yy_destructor(65,&yymsp[0].minor);
}
// 7684 "parser.cpp"
        break;
      case 367:
      case 423:
// 2908 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_TRUE, NULL, status->scanner_state);
  yy_destructor(117,&yymsp[0].minor);
}
// 7693 "parser.cpp"
        break;
      case 368:
      case 422:
// 2913 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_FALSE, NULL, status->scanner_state);
  yy_destructor(118,&yymsp[0].minor);
}
// 7702 "parser.cpp"
        break;
      case 369:
      case 412:
      case 425:
// 2918 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_literal(XX_T_CONSTANT, yymsp[0].minor.yy0, status->scanner_state);
}
// 7711 "parser.cpp"
        break;
      case 370:
      case 426:
// 2923 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("empty-array", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-1].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7721 "parser.cpp"
        break;
      case 371:
      case 427:
// 2928 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("array", yymsp[-1].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(41,&yymsp[-2].minor);
  yy_destructor(67,&yymsp[0].minor);
}
// 7731 "parser.cpp"
        break;
      case 372:
// 2933 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(0, yymsp[0].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-1].minor);
}
// 7739 "parser.cpp"
        break;
      case 373:
// 2938 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7749 "parser.cpp"
        break;
      case 374:
// 2943 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(38,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7759 "parser.cpp"
        break;
      case 375:
// 2948 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(1, yymsp[-1].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 7769 "parser.cpp"
        break;
      case 376:
// 2953 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(1, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(38,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7781 "parser.cpp"
        break;
      case 377:
// 2958 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_new_instance(1, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(38,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7793 "parser.cpp"
        break;
      case 378:
// 2963 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall(1, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7802 "parser.cpp"
        break;
      case 379:
// 2968 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall(1, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7811 "parser.cpp"
        break;
      case 380:
// 2973 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall(2, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7822 "parser.cpp"
        break;
      case 381:
// 2978 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_fcall(2, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7833 "parser.cpp"
        break;
      case 382:
// 2983 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(0, yymsp[-5].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(107,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7843 "parser.cpp"
        break;
      case 383:
// 2988 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(0, yymsp[-4].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(107,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7853 "parser.cpp"
        break;
      case 384:
// 2993 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(1, yymsp[-6].minor.yy0, 0, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(54,&yymsp[-7].minor);
  yy_destructor(55,&yymsp[-5].minor);
  yy_destructor(107,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7865 "parser.cpp"
        break;
      case 385:
// 2998 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_scall(1, yymsp[-5].minor.yy0, 0, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(54,&yymsp[-6].minor);
  yy_destructor(55,&yymsp[-4].minor);
  yy_destructor(107,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7877 "parser.cpp"
        break;
      case 386:
// 3003 "parser.lemon"
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
// 7891 "parser.cpp"
        break;
      case 387:
// 3008 "parser.lemon"
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
// 7905 "parser.cpp"
        break;
      case 388:
// 3013 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(1, yymsp[-5].minor.yy262, yymsp[-3].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7915 "parser.cpp"
        break;
      case 389:
// 3018 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(1, yymsp[-4].minor.yy262, yymsp[-2].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7925 "parser.cpp"
        break;
      case 390:
// 3023 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(2, yymsp[-7].minor.yy262, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7937 "parser.cpp"
        break;
      case 391:
// 3028 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(2, yymsp[-6].minor.yy262, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7949 "parser.cpp"
        break;
      case 392:
// 3033 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(3, yymsp[-7].minor.yy262, yymsp[-4].minor.yy0, yymsp[-1].minor.yy262, status->scanner_state);
  yy_destructor(42,&yymsp[-6].minor);
  yy_destructor(54,&yymsp[-5].minor);
  yy_destructor(55,&yymsp[-3].minor);
  yy_destructor(61,&yymsp[-2].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7961 "parser.cpp"
        break;
      case 393:
// 3038 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_mcall(3, yymsp[-6].minor.yy262, yymsp[-3].minor.yy0, NULL, status->scanner_state);
  yy_destructor(42,&yymsp[-5].minor);
  yy_destructor(54,&yymsp[-4].minor);
  yy_destructor(55,&yymsp[-2].minor);
  yy_destructor(61,&yymsp[-1].minor);
  yy_destructor(40,&yymsp[0].minor);
}
// 7973 "parser.cpp"
        break;
      case 397:
// 3058 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("ternary", yymsp[-4].minor.yy262, yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(8,&yymsp[-3].minor);
  yy_destructor(89,&yymsp[-1].minor);
}
// 7982 "parser.cpp"
        break;
      case 400:
// 3071 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy262, status->scanner_state, 0);
}
// 7989 "parser.cpp"
        break;
      case 401:
// 3076 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_call_parameter(yymsp[-2].minor.yy0, yymsp[0].minor.yy262, status->scanner_state, 0);
  yy_destructor(89,&yymsp[-1].minor);
}
// 7997 "parser.cpp"
        break;
      case 402:
// 3081 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_call_parameter(NULL, yymsp[0].minor.yy262, status->scanner_state, 1);
  yy_destructor(15,&yymsp[-1].minor);
}
// 8005 "parser.cpp"
        break;
      case 403:
// 3086 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_call_parameter(yymsp[-3].minor.yy0, yymsp[0].minor.yy262, status->scanner_state, 0);
  yy_destructor(89,&yymsp[-2].minor);
  yy_destructor(15,&yymsp[-1].minor);
}
// 8014 "parser.cpp"
        break;
      case 404:
// 3091 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("closure", NULL, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-4].minor);
  yy_destructor(61,&yymsp[-3].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8026 "parser.cpp"
        break;
      case 405:
// 3096 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("closure", NULL, yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8038 "parser.cpp"
        break;
      case 406:
// 3101 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("closure", yymsp[-3].minor.yy262, NULL, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-5].minor);
  yy_destructor(61,&yymsp[-4].minor);
  yy_destructor(40,&yymsp[-2].minor);
  yy_destructor(54,&yymsp[-1].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8050 "parser.cpp"
        break;
      case 407:
// 3106 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_expr("closure", yymsp[-4].minor.yy262, yymsp[-1].minor.yy262, NULL, status->scanner_state);
  yy_destructor(60,&yymsp[-6].minor);
  yy_destructor(61,&yymsp[-5].minor);
  yy_destructor(40,&yymsp[-3].minor);
  yy_destructor(54,&yymsp[-2].minor);
  yy_destructor(55,&yymsp[0].minor);
}
// 8062 "parser.cpp"
        break;
      case 410:
      case 430:
// 3118 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_array_item(yymsp[-2].minor.yy262, yymsp[0].minor.yy262, status->scanner_state);
  yy_destructor(89,&yymsp[-1].minor);
}
// 8071 "parser.cpp"
        break;
      case 411:
      case 431:
// 3122 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_array_item(NULL, yymsp[0].minor.yy262, status->scanner_state);
}
// 8079 "parser.cpp"
        break;
      case 437:
// 3227 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_comment(yymsp[0].minor.yy0, status->scanner_state);
}
// 8086 "parser.cpp"
        break;
      case 438:
// 3231 "parser.lemon"
{
	yygotominor.yy262 = xx_ret_cblock(yymsp[0].minor.yy0, status->scanner_state);
}
// 8093 "parser.cpp"
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
// 1270 "parser.lemon"

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

// 8203 "parser.cpp"
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
