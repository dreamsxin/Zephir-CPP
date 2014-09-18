/* 
 * File:   StatementResult.h
 * Author: Dreamszhu
 *
 * Created on September 18, 2014, 9:57 AM
 */

#ifndef STATEMENTRESULT_H
#define	STATEMENTRESULT_H

#include <string>

#include "json/json.h"

class StatementResult {

public:
	enum ResultType {
		NORMAL_RESULT = 1,
		RETURN_RESULT,
		BREAK_RESULT,
		CONTINUE_RESULT
	};
	
	enum ResultValueType {
		BOOLEAN_VALUE = 1,
		INT_VALUE,
		DOUBLE_VALUE,
		STRING_VALUE,
		NATIVE_POINTER_VALUE,
		NULL_VALUE,
		ARRAY_VALUE,
		ASSOC_VALUE
	};

private:
	StatementResult::ResultType _result_type;
	StatementResult::ResultValueType _result_value_type;
	Json::Value _result_value;

public:
	StatementResult();
	StatementResult(const StatementResult& orig);
	virtual ~StatementResult();

	void setResultType(StatementResult::ResultType type);
	void setResultValueType(StatementResult::ResultValueType type);
	void setResultValue(const Json::Value& value);

	StatementResult::ResultType getResultType();
	StatementResult::ResultValueType getResultValueType();
	Json::Value getResultValue();
	

};

#endif	/* STATEMENTRESULT_H */

