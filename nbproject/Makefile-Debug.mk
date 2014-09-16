#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1585107349/parser.o \
	${OBJECTDIR}/_ext/1585107349/scanner.o \
	${OBJECTDIR}/_ext/842322087/CustomOptionDescription.o \
	${OBJECTDIR}/_ext/842322087/OptionPrinter.o \
	${OBJECTDIR}/Compiler.o \
	${OBJECTDIR}/json/jsoncpp.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-std=c++11
CXXFLAGS=-std=c++11

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lboost_program_options -lboost_system -lboost_filesystem -lboost_regex

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk bin/zephir-cpp

bin/zephir-cpp: ${OBJECTFILES}
	${MKDIR} -p bin
	${LINK.cc} -o bin/zephir-cpp ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/1585107349/parser.o: /home/zhuzx/work/Zephir-CPP/parser.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1585107349
	${RM} "$@.d"
	$(COMPILE.cc) -g -Iinclude -I/usr/include -I/usr/include/c++/4.8.2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1585107349/parser.o /home/zhuzx/work/Zephir-CPP/parser.cpp

${OBJECTDIR}/_ext/1585107349/scanner.o: /home/zhuzx/work/Zephir-CPP/scanner.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1585107349
	${RM} "$@.d"
	$(COMPILE.cc) -g -Iinclude -I/usr/include -I/usr/include/c++/4.8.2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1585107349/scanner.o /home/zhuzx/work/Zephir-CPP/scanner.cpp

${OBJECTDIR}/_ext/842322087/CustomOptionDescription.o: /home/zhuzx/work/Zephir-CPP/usage/CustomOptionDescription.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/842322087
	${RM} "$@.d"
	$(COMPILE.cc) -g -Iinclude -I/usr/include -I/usr/include/c++/4.8.2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/842322087/CustomOptionDescription.o /home/zhuzx/work/Zephir-CPP/usage/CustomOptionDescription.cpp

${OBJECTDIR}/_ext/842322087/OptionPrinter.o: /home/zhuzx/work/Zephir-CPP/usage/OptionPrinter.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/842322087
	${RM} "$@.d"
	$(COMPILE.cc) -g -Iinclude -I/usr/include -I/usr/include/c++/4.8.2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/842322087/OptionPrinter.o /home/zhuzx/work/Zephir-CPP/usage/OptionPrinter.cpp

${OBJECTDIR}/Compiler.o: Compiler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Iinclude -I/usr/include -I/usr/include/c++/4.8.2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Compiler.o Compiler.cpp

${OBJECTDIR}/json/jsoncpp.o: json/jsoncpp.cpp 
	${MKDIR} -p ${OBJECTDIR}/json
	${RM} "$@.d"
	$(COMPILE.cc) -g -Iinclude -I/usr/include -I/usr/include/c++/4.8.2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/json/jsoncpp.o json/jsoncpp.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -Iinclude -I/usr/include -I/usr/include/c++/4.8.2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} bin/zephir-cpp

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
