/* 
 * File:   StatementResult.cpp
 * Author: Dreamszhu
 * 
 * Created on September 18, 2014, 9:57 AM
 */

#include "interpreter/StatementResult.h"

StatementResult::StatementResult() {
	this->type = NORMAL_RESULT;
}

StatementResult::StatementResult(const StatementResult& orig) {
	this->type = orig.type;
	this->value = orig.value;
}

StatementResult::~StatementResult() {
}

void StatementResult::setType(const StatementResult::TYPE type) {
	this->type = type;
}

void StatementResult::setValue(const ZephirValue& value) {
	this->value = value;
}

StatementResult::TYPE StatementResult::getType() {
	return this->type;
}

ZephirValue StatementResult::getValue() {
	return this->value;
}

