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

class LocalEnvironment {
public:
	LocalEnvironment();
	LocalEnvironment(const LocalEnvironment& orig);
	virtual ~LocalEnvironment();

	void addVariable(const std::string& name, const boost::any& value);
	boost::any *getVariable(const std::string& name);

private:	
	boost::unordered_map<std::string, boost::any> variables;

};

#endif	/* LOCALENVIRONMENT_H */

