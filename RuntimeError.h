/* 
 * File:   RuntimeError.h
 * Author: Dreamszhu
 *
 * Created on September 19, 2014, 8:23 AM
 */

#ifndef RUNTIMEERROR_H
#define	RUNTIMEERROR_H

#include <exception>

#include <boost/unordered_map.hpp>

#include "json/json.h"

class RuntimeError : public std::exception {
public:

	enum TYPE {
		VARIABLE_NOT_FOUND = 1,
		ARGUMENT_TOO_MANY,
		ARGUMENT_TOO_FEW,
		NOT_BOOLEAN_TYPE,
		MINUS_OPERAND_TYPE,
		BAD_OPERAND_TYPE,
		LOGICAL_OP_DOUBLE_OPERAND,
		LOGICAL_OP_INTEGER_OPERAND,
		NOT_BOOLEAN_OPERATOR,
		NOT_NULL_OPERATOR,
		NOT_LVALUE,
		INDEX_OPERAND_NOT_ARRAY,
		INDEX_OPERAND_NOT_INT,
		ARRAY_INDEX_OUT_OF_BOUNDS,
		NO_SUCH_METHOD,
		INC_DEC_OPERAND_TYPE,
		INC_DEC_OPERAND_NOT_EXIST,
		NOT_FUNCTION,
		NOT_OBJECT_MEMBER_UPDATE,
		NOT_OBJECT_MEMBER_ASSIGN,
		NO_SUCH_MEMBER,
		NO_MEMBER_TYPE,
		BAD_OPERATOR_FOR_STRING,
		DIVISION_BY_ZERO,
		GLOBAL_VARIABLE_NOT_FOUND,
		GLOBAL_STATEMENT_IN_TOPLEVEL,
		FUNCTION_EXISTS,
		ARRAY_RESIZE_ARGUMENT,
		ARRAY_INSERT_ARGUMENT,
		ARRAY_REMOVE_ARGUMENT,
		STRING_POS_OUT_OF_BOUNDS,
		STRING_SUBSTR_LEN,
		STRING_SUBSTR_ARGUMENT,
		EXCEPTION_HAS_NO_MESSAGE,
		EXCEPTION_MESSAGE_IS_NOT_STRING,
		EXCEPTION_HAS_NO_STACK_TRACE,
		STACK_TRACE_IS_NOT_ARRAY,
		STACK_TRACE_LINE_IS_NOT_ASSOC,
		STACK_TRACE_LINE_HAS_NO_LINE_NUMBER,
		STACK_TRACE_LINE_HAS_NO_FUNC_NAME,
		EXCEPTION_IS_NOT_ASSOC,
		EXCEPTION_HAS_NO_PRINT_STACK_TRACE_METHOD,
		PRINT_STACK_TRACE_IS_NOT_CLOSURE,
		BAD_MULTIBYTE_CHARACTER,
		EXCEPTION_CLASS_IS_NOT_ASSOC,
		EXCEPTION_CLASS_HAS_NO_CREATE_METHOD,
		ARGUMENT_TYPE_MISMATCH,
		UNEXPECTED_WIDE_STRING,
		ONIG_SEARCH_FAIL,
		GROUP_INDEX_OVERFLOW,
		NO_SUCH_GROUP_INDEX,
		BREAK_OR_CONTINUE_REACHED_TOPLEVEL,
		ASSIGN_TO_FINAL_VARIABLE,
		FUNCTION_NOT_FOUND
	};

	const std::array<std::string, 56> messages = {
		{
			"dummy",
			"VariableNotFoundException",
			"ArgumentTooManyException",
			"ArgumentTooFewException",
			"NotBooleanException",
			"MinusOperandTypeException",
			"BadOperandTypeException",
			"LogicalOperatorDoubleOperandException",
			"LogicalOperatorIntegerOperandException",
			"NotBooleanOperatorException",
			"NotNullOperatorException",
			"NotLValueException",
			"IndexOperandNotArrayException",
			"IndexOperandNotIntException",
			"ArrayIndexOutOfBoundsException",
			"NoSuchMethodException",
			"IncDecOperandTypeException",
			"IncDecOperandNotExistException",
			"NotFunctionException",
			"NotObjectMemberUpdateException",
			"NotObjectMemberAssignException",
			"NoSuchMemberException",
			"NoMemberTypeException",
			"BadOperatorForStringException",
			"DivisionByZeroException",
			"GlobalVariableNotFoundException",
			"GlobalStatementInToplevelException",
			"FunctionExistsException",
			"ArrayResizeArgumentException",
			"ArrayInsertArgumentException",
			"ArrayRemoveArgumentException",
			"StringPositionOutOfBoundsException",
			"StringSubstrLengthException",
			"StringSubstrArgumentException",
			"ExceptionHasNoMessageException",
			"ExceptionMessageIsNotStringException",
			"ExceptionHasNoStackTraceException",
			"StackTraceIsNotArrayException",
			"StackTraceLineIsNotAssocException",
			"StackTraceLineHasNoLineNumberException",
			"StackTraceLineHasNoFuncNameException",
			"ExceptionIsNotAssocException",
			"ExceptionHasNoPrintStackTraceMethodException",
			"PrintStackTraceIsNotClosureException",
			"BadMultibyteCharacterException",
			"ExceptionClassIsNotAssocException",
			"ExceptionClassHasNoCreateMethodException",
			"ArgumentTypeMismatchException",
			"UnexpectedWideStringException",
			"OnigSearchFailException",
			"GroupIndexOverflowException",
			"GroupIndexOverflowException",
			"BreakOrContinueReachedToplevelException",
			"AssignToFinalVariableException",
			"FunctionNotFoundException",
			"dummy"
		}
	};

public:
	RuntimeError(const RuntimeError::TYPE type, const Json::Value& info);
	RuntimeError(const RuntimeError& orig);
	virtual ~RuntimeError();

	virtual const char* what() const throw ();

private:
	void generate();

protected:
     std::string message;
private:
	RuntimeError::TYPE type;
	Json::Value info;

};

#endif	/* RUNTIMEERROR_H */

