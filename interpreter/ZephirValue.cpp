/* 
 * File:   ZephirValue.cpp
 * Author: Dreamszhu
 * 
 * Created on September 18, 2014, 3:53 PM
 */

#include <iostream>

#include "interpreter/ZephirValue.h"

ZephirValue::ZephirValue() {
	this->type = ZephirValue::TYPE::NULL_VALUE;
	this->value = NULL;
}

ZephirValue::ZephirValue(const ZephirValue::TYPE& type, const boost::any& value) {
	this->type = type;
	this->value = value;
}

ZephirValue::ZephirValue(const ZephirValue& orig) {
	this->type = orig.type;
	this->value = orig.value;
}

ZephirValue::~ZephirValue() {
}

void ZephirValue::setType(const ZephirValue::TYPE& type) {
	this->type = type;
}

void ZephirValue::setValue(const boost::any& value) {
	this->value = value;
}

ZephirValue::TYPE ZephirValue::getType() {
	return this->type;
}

boost::any ZephirValue::getValue() {
	return this->value;
}

bool ZephirValue::isNULL() const {
	return type == ZephirValue::TYPE::NULL_VALUE;
}

bool ZephirValue::isString() const {
	return type == ZephirValue::TYPE::STRING_VALUE;
}

std::string ZephirValue::asString() const {
	switch (type) {
		case ZephirValue::TYPE::BOOLEAN_VALUE:
			return boost::any_cast<bool>(value) ? "true" : "false";
		case ZephirValue::TYPE::INT_VALUE:
			return std::to_string(boost::any_cast<int>(value));
		case ZephirValue::TYPE::DOUBLE_VALUE:
			return std::to_string(boost::any_cast<double>(value));
		case ZephirValue::TYPE::STRING_VALUE:
			return boost::any_cast<std::string>(value);
		case ZephirValue::TYPE::NATIVE_POINTER_VALUE:;
		case ZephirValue::TYPE::NULL_VALUE:
			return "";
		case ZephirValue::TYPE::ARRAY_VALUE:
			return "array";
		case ZephirValue::TYPE::ASSOC_VALUE:
			return "assoc";
		default:
			return "";
	}
}

int ZephirValue::asInt() const {
	switch (type) {
		case ZephirValue::TYPE::BOOLEAN_VALUE:
			return boost::any_cast<bool>(value) ? 1 : 0;
		case ZephirValue::TYPE::INT_VALUE:
			return boost::any_cast<int>(value);
		case ZephirValue::TYPE::DOUBLE_VALUE:
			return static_cast<int>(boost::any_cast<double>(value));
		case ZephirValue::TYPE::STRING_VALUE:
			return std::stoi(boost::any_cast<std::string>(value));
		case ZephirValue::TYPE::NATIVE_POINTER_VALUE:
		case ZephirValue::TYPE::NULL_VALUE:
		case ZephirValue::TYPE::ARRAY_VALUE:
		default:
			return 0;
	}
}

double ZephirValue::asDouble() const {
	switch (type) {
		case ZephirValue::TYPE::BOOLEAN_VALUE:
			return boost::any_cast<bool>(value) ? 1 : 0;
		case ZephirValue::TYPE::INT_VALUE:
			return static_cast<double>(boost::any_cast<int>(value));
		case ZephirValue::TYPE::DOUBLE_VALUE:
			return boost::any_cast<double>(value);
		case ZephirValue::TYPE::STRING_VALUE:
			return std::stod(boost::any_cast<std::string>(value));
		case ZephirValue::TYPE::NATIVE_POINTER_VALUE:
		case ZephirValue::TYPE::NULL_VALUE:
		case ZephirValue::TYPE::ARRAY_VALUE:
		case ZephirValue::TYPE::ASSOC_VALUE:
		default:
			return 0;
	}
}

bool ZephirValue::asBool() const {
	switch (type) {
		case ZephirValue::TYPE::BOOLEAN_VALUE:
			return boost::any_cast<bool>(value);
		case ZephirValue::TYPE::INT_VALUE:
			return boost::any_cast<int>(value) ? true : false;
		case ZephirValue::TYPE::DOUBLE_VALUE:
			return boost::any_cast<double>(value) ? true : false;
		case ZephirValue::TYPE::STRING_VALUE:
			return boost::any_cast<std::string>(value).empty() ? false : true;
		case ZephirValue::TYPE::NATIVE_POINTER_VALUE:
		case ZephirValue::TYPE::NULL_VALUE:
		case ZephirValue::TYPE::ARRAY_VALUE:
		case ZephirValue::TYPE::ASSOC_VALUE:
		default:
			return false;
	}
}

ZephirValue operator+(const ZephirValue &left, const ZephirValue &right) {
	ZephirValue value;
	switch (left.type) {
		case ZephirValue::TYPE::BOOLEAN_VALUE:
			value.setType(ZephirValue::TYPE::BOOLEAN_VALUE);
			value.setValue(left.asBool() + right.asBool());
			break;
		case ZephirValue::TYPE::INT_VALUE:
			value.setType(ZephirValue::TYPE::INT_VALUE);
			value.setValue(left.asInt() + right.asInt());
			break;
		case ZephirValue::TYPE::DOUBLE_VALUE:
			value.setType(ZephirValue::TYPE::DOUBLE_VALUE);
			value.setValue(left.asDouble() + right.asDouble());
			break;
		case ZephirValue::TYPE::STRING_VALUE:
			value.setType(ZephirValue::TYPE::STRING_VALUE);
			value.setValue(left.asString().append(right.asString()));
			break;
		case ZephirValue::TYPE::NATIVE_POINTER_VALUE:
		case ZephirValue::TYPE::NULL_VALUE:
		case ZephirValue::TYPE::ARRAY_VALUE:
		case ZephirValue::TYPE::ASSOC_VALUE:
		default:
			value.setType(ZephirValue::TYPE::NULL_VALUE);
			break;
	}

	return value;
}

std::ostream& operator<<(std::ostream& out, const ZephirValue& value) {
	switch (value.type) {
		case ZephirValue::TYPE::BOOLEAN_VALUE:
			out << boost::any_cast<bool>(value.value) << std::endl;
			break;
		case ZephirValue::TYPE::INT_VALUE:
			out << boost::any_cast<int>(value.value) << std::endl;
			break;
		case ZephirValue::TYPE::DOUBLE_VALUE:
			out << boost::any_cast<double>(value.value) << std::endl;
			break;
		case ZephirValue::TYPE::STRING_VALUE:
			out << boost::any_cast<std::string>(value.value) << std::endl;
			break;
		default:
			break;
	}
	return out;
}
