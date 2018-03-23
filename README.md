# CmdLineParser
Command line parser is designed for command line parsing. 
Command line is consist of set of pairs [name] [value], where [name] is a  parameter name (marked by flag); [value] is a parameter value. It is standard way to pass parameters to the executing file.

Example of command line:
/File D:\File1 /Count  354 -string "Calculate value"

Features:
- simple integration: you need only single class. No third-party libraries, only STL is used;
- simple interface: you need only bind variable to the parameter name;
- flexible: you can setup a sign of flag, prefix od radix, name for bool values, etc;
- a lot of validation rules are supported.

To use it, include CmdLineParser.h and CmdLineParser.cpp in your project. 

Main class methods are the following:

void Parse(int argc, const char* argv[], int argStart = 0);
Parsing of input parameters to be passed to the starting function main(int argc, _TCHAR* argv[])

void Parse(const char* cmdLineString); 
Parsing of input parameters from the text line.

void AddParamFlag(const char* paramFlag); 
add a sign of parameter name (flag);


void BindParam(const char* paramName, [bool, int, double…] & paramValue);
Bind parameter with appropriating variable. When parsing, corresponding parameter value will be converted to the assigned variable

void BindParam(const char* paramName, [callback_...]  callbackFunction);
Bind parameter with appropriating callback function. When parsing, corresponding parameter value will be passed to the assigned function.

void AssignConstrains(const char* paramName, rule_mask_t rulesMask);
Assign rules for parameter validation value.

A lot of examples of using you can find in CmdLineParserTest.cpp.

If you have questions, write me to Tkachenko_vitaliy@hotmail.com

