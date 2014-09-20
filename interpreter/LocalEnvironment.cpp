/* 
 * File:   LocalEnvironment.cpp
 * Author: Dreamszhu
 * 
 * Created on September 18, 2014, 2:41 PM
 */

#include "interpreter/LocalEnvironment.h"

LocalEnvironment::LocalEnvironment() {
}

LocalEnvironment::LocalEnvironment(const LocalEnvironment& orig) {
}

LocalEnvironment::~LocalEnvironment() {
}

void LocalEnvironment::addVariable(const std::string& name, const ZephirValue& value) {
	this->variables[name] = value; 
}

ZephirValue LocalEnvironment::getVariable(const std::string& name) {
	ZephirValue value;
	if (this->variables.find(name) != this->variables.end()) {
		value = this->variables[name];
	} else {
		value.setType(ZephirValue::UNDEFINED_VALUE);
	}

	return value;
}

