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

void LocalEnvironment::addVariable(const std::string& name, const boost::any& value) {
	this->variables[name] = value; 
}

boost::any *LocalEnvironment::getVariable(const std::string& name) {
	if (this->variables.find(name) != this->variables.end()) {
		return &this->variables[name];
	} else {
		return nullptr;
	}
}

