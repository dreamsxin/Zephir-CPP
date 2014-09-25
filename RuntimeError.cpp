/* 
 * File:   RuntimeError.cpp
 * Author: Dreamszhu
 * 
 * Created on September 19, 2014, 8:23 AM
 */
#include <iostream>

#include "RuntimeError.h"

RuntimeError::RuntimeError(const RuntimeError::TYPE t, const Json::Value& i) : type(t), info(i) {
	this->generate();
}

RuntimeError::RuntimeError(const RuntimeError& orig) {
	this->type = orig.type;
	this->info = orig.info;
	this->generate();
}

RuntimeError::~RuntimeError() throw() {
}

const char* RuntimeError::what() const throw () {
	return this->message.c_str();
}

void RuntimeError::generate() {
	std::ostringstream oss;
	oss << "[RuntimeError] " << this->messages[this->type];
	if (this->info.isMember("file")) {
		oss << " in " << this->info["file"].asString() << " on line " << this->info["line"].asString();
	}
	
	this->message = oss.str();	
}

