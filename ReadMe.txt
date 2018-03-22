CmdLineParser is a parser of command line.

The command line is a standard way to pass parameters to the executing file. It consists of set of pairs 

<parameter1> <value-of-parameter1>  <parameter2>  <value-of parameter2> …

where <parameter> is a parameter name (with flag); <value-of parameter> is a parameter value.

Example:
/File D:\File1   /Count  354

To use the parser, include files “CmdLineParser.h” and “CmdLineParser.cpp” into your project.

The class supports input parameter validation.

Main class methods are the following:

void Parse(int argc, const char* argv[], int argStart = 0);
Parsing of input parameters to be passed to the starting function main(int argc, _TCHAR* argv[])

void Parse(const char* cmdLineString); 
Parsing of input parameters from the text line.

void AddParamFlag(const char* paramFlag); 
add a sign of parameter name (flag);

void BindParam(const char* paramName, [bool, int, double…] & paramValue);
Bind parameter with appropriating variable. When parsing, corresponding parameter value will be converted to the assigned variable

void BindParam(const char* paramName, [callback_...]  callbackFunction);
Bind parameter with appropriating callback function. When parsing, corresponding parameter value will be passed to the assigned function.

void AssignConstrains(const char* paramName, rule_mask_t rulesMask);
Assign rules for parameter validation value.

A lot of examples of use you can find in CmdLineParserTest.cpp.

If you have questions, write me to Tkachenko_vitaliy@hotmail.com


