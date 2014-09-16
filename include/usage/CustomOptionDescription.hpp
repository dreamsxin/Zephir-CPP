#ifndef MYLEFT_CUSTOMOPTIONDESCRIPTION_HPP
#define MYLEFT_CUSTOMOPTIONDESCRIPTION_HPP

#include "boost/program_options.hpp"

#include <string>

namespace myleft {

	class CustomOptionDescription {
	public:
		CustomOptionDescription(boost::shared_ptr<boost::program_options::option_description> option);

		void checkIfPositional(const boost::program_options::positional_options_description& positionalDesc);

		std::string getOptionUsageString();

	public:
		std::string optionID_;
		std::string optionDisplayName_;
		std::string optionDescription_;
		std::string optionFormatName_;

		bool required_;
		bool hasShort_;
		bool hasArgument_;
		bool isPositional_;

	};
}

#endif
