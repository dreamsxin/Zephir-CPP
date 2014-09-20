/* 
 * File:   LocalEnvironment.h
 * Author: Dreamszhu
 *
 * Created on September 18, 2014, 2:41 PM
 */

#ifndef LOCALENVIRONMENT_H
#define	LOCALENVIRONMENT_H

#include <string>

#include <boost/any.hpp>
#include <boost/unordered_map.hpp>

#include "interpreter/ZephirValue.h"

class LocalEnvironment {
public:
	LocalEnvironment();
	LocalEnvironment(const LocalEnvironment& orig);
	virtual ~LocalEnvironment();

	void addVariable(const std::string& name, const ZephirValue& value);
	ZephirValue getVariable(const std::string& name);

private:
	boost::unordered_map<std::string, ZephirValue> variables;
};

#endif	/* LOCALENVIRONMENT_H */

