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
#include "interpreter/ZephirValue.h"

class StatementResult {
public:
	enum TYPE {
		NORMAL_RESULT = 1,
		RETURN_RESULT,
		BREAK_RESULT,
		CONTINUE_RESULT
	};

private:
	StatementResult::TYPE type;
	ZephirValue value;

public:
	StatementResult();
	StatementResult(const StatementResult& orig);
	virtual ~StatementResult();

	void setType(StatementResult::TYPE type);
	void setValue(const ZephirValue& value);

	StatementResult::TYPE getType();
	ZephirValue getValue();
};

#endif	/* STATEMENTRESULT_H */

