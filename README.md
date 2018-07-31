# Command Line Parser
Command line parser is designed for command line parsing. 
Command line is consist of set of pairs [name] [value], where [name] is a  parameter name (marked by flag); [value] is a parameter value. It is standard way to pass parameters to the executing file.

Example of command line:
/File D:\File1 /Count  354 -string "Calculate value"

# Features:
- simple integration: you need only single class. No third-party libraries, only STL is used;
- simple interface: you need only bind variable to the parameter name;
- flexible: you can setup a sign of flag, prefix od radix, name for bool values, etc;
- a lot of validation rules are supported.

To use it, include CmdLineParser.h and CmdLineParser.cpp in your project. 

# Usage
Add files CmdLineParser.h,cpp to your project. Assign signs of param flags. Bind parameter to variable. Assign constrains to the allowed parameter value. Call **Parse** method, and passed parameter values will be written to the bounded variables. 
If parameter value is not passed validation rules, the exception **CmdLineParseException** will be thrown. 
 
Main methods are the following:

void **BindParam**(const char* paramName, void BindParam(const char* paramName, {bool|int| double…}& paramValue, rule_mask_t ruleMask = CN_NONE);
Binds parameter with appropriating variable. When parsing, corresponding parameter value will be converted to the assigned variable.

void **BindParam**(const char* paramName, {callback_bool_t|callback_long_t|callback_double_t...}  callbackFunction, rule_mask_t ruleMask = CN_NONE);
Binds parameter with appropriating callback function. When parsing, corresponding parameter value will be passed to the assigned function.

void **AssignConstrains**(const char* paramName, rule_mask_t rulesMask);
Assign rules for parameter validation value.

void **Parse**(int argc, const char* argv[], int argStart = 0);
Parses input parameters to be passed to the starting function main(int argc, _TCHAR* argv[])

void **Parse**(const char* cmdLineString); 
Parses input parameters from the text line.

void **AddParamFlag**(const char* paramFlag); 
Add a sign of parameter name (flag)

A lot of examples of use case you can find in CmdLineParserTest.cpp.

If you have questions, write me to Tkachenko_vitaliy@hotmail.com

