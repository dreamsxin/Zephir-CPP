/* 
 * File:   ZephirValue.h
 * Author: Dreamszhu
 *
 * Created on September 18, 2014, 3:53 PM
 */

#ifndef ZEPHIRVALUE_H
#define	ZEPHIRVALUE_H

#include <ostream>

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

	void setType(const ZephirValue::TYPE& type);
	void setValue(const boost::any& value);

	ZephirValue::TYPE getType();
	boost::any getValue();

	std::string asString() const;
	int asInt() const;
	double asDouble() const;
	bool asBool() const;

	ZephirValue& operator+=(const ZephirValue &right) {
		switch (this->type) {
			case ZephirValue::TYPE::BOOLEAN_VALUE:
				this->value = this->asBool() + right.asBool();
				break;
			case ZephirValue::TYPE::INT_VALUE:
				this->value = this->asInt() + right.asInt();
				break;
			case ZephirValue::TYPE::DOUBLE_VALUE:
				this->value = this->asDouble() + right.asDouble();
				break;
			case ZephirValue::TYPE::STRING_VALUE:
				boost::any_cast<std::string>(value).append(right.asString());
				break;
			case ZephirValue::TYPE::NATIVE_POINTER_VALUE:
			case ZephirValue::TYPE::NULL_VALUE:
			case ZephirValue::TYPE::ARRAY_VALUE:
			case ZephirValue::TYPE::ASSOC_VALUE:
			default:
				break;
		}
		return *this;
	}
	
	friend ZephirValue operator+(const ZephirValue &left, const ZephirValue &right);

	friend std::ostream& operator<<(std::ostream& out, const ZephirValue& value);

private:
	ZephirValue::TYPE type;
	boost::any value;

};

#endif	/* ZEPHIRVALUE_H */

