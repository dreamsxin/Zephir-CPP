/* 
 * File:   ZephirValue.h
 * Author: Dreamszhu
 *
 * Created on September 18, 2014, 3:53 PM
 */

#ifndef ZEPHIRVALUE_H
#define	ZEPHIRVALUE_H

#include <boost/any.hpp>

class ZephirValue {
public:	
	enum TYPE {
		BOOLEAN_VALUE = 1,
		INT_VALUE,
		DOUBLE_VALUE,
		STRING_VALUE,
		NATIVE_POINTER_VALUE,
		NULL_VALUE,
		ARRAY_VALUE,
		ASSOC_VALUE
	};
	
public:
	ZephirValue();
	ZephirValue(const ZephirValue::TYPE type, const boost::any& value);
	ZephirValue(const ZephirValue& orig);
	virtual ~ZephirValue();

	void setType(const ZephirValue::TYPE type);
	void setValue(const boost::any& value);

	ZephirValue::TYPE getType();
	boost::any getValue();
private:
	ZephirValue::TYPE type;
	boost::any value;

};

#endif	/* ZEPHIRVALUE_H */

