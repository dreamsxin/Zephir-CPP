/* 
 * File:   StatementResult.cpp
 * Author: Dreamszhu
 * 
 * Created on September 18, 2014, 9:57 AM
 */

#include "interpreter/StatementResult.h"

StatementResult::StatementResult() {
}

StatementResult::StatementResult(const StatementResult& orig) {
	this->_result_type = orig._result_type;
	this->_result_value_type = orig._result_value_type;
	this->_result_value = orig._result_value;
}

StatementResult::~StatementResult() {
}

void StatementResult::setResultType(const StatementResult::ResultType type) {
	this->_result_type = type;
}

void StatementResult::setResultValueType(const StatementResult::ResultValueType type) {
	this->_result_value_type = type;
}

void StatementResult::setResultValue(const Json::Value& value) {
	this->_result_value = value;
}

StatementResult::ResultType StatementResult::getResultType() {
	return this->_result_type;
}

StatementResult::ResultValueType StatementResult::getResultValueType() {
	return this->_result_value_type;
}

Json::Value StatementResult::getResultValue() {
	switch (this->_result_value_type) {
		case StatementResult::ResultValueType::BOOLEAN_VALUE:
			break;
		case StatementResult::ResultValueType::INT_VALUE:
			break;
		case StatementResult::ResultValueType::DOUBLE_VALUE:
			break;
		case StatementResult::ResultValueType::STRING_VALUE:
			break;
		case StatementResult::ResultValueType::NATIVE_POINTER_VALUE:
			break;
		case StatementResult::ResultValueType::NULL_VALUE:
			break;
		case StatementResult::ResultValueType::ARRAY_VALUE:
			break;
		case StatementResult::ResultValueType::ASSOC_VALUE:
			break;
	}

	return this->_result_value;
}

