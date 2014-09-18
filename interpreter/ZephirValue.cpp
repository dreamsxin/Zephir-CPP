/* 
 * File:   ZephirValue.cpp
 * Author: Dreamszhu
 * 
 * Created on September 18, 2014, 3:53 PM
 */

#include "interpreter/ZephirValue.h"

ZephirValue::ZephirValue() {
}

ZephirValue::ZephirValue(const ZephirValue::TYPE type, const boost::any& value) {
	this->type = type;
	this->value = value;
}

ZephirValue::ZephirValue(const ZephirValue& orig) {
	this->type = type;
	this->value = value;
}

ZephirValue::~ZephirValue() {
}

void ZephirValue::setType(const ZephirValue::TYPE type) {
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
