#include "CmdLineParser.h"

#include <gtest/gtest.h>

#include <string.h>

struct Parameters
{
    char vChar;
    unsigned char vUChar;
    bool  vBool;
    short vShort;
    unsigned short vUShort;
    int vInt;
    unsigned int vUInt;
    long vLong;
    unsigned long vULong;
    float vFloat;
    double vDouble;
    std::string vString;
    static const int size_string_buffer = 50;
    char vStringBuffer[size_string_buffer];
    bool vFlag;

    Parameters()
    {
        reset();
    }

    void reset()
    {
        vChar = 0;
        vUChar = 0;
        vBool = false;
        vShort = 0;
        vUShort = 0;
        vInt = 0;
        vUInt = 0;
        vLong = 0;
        vULong = 0;
        vFloat = 0.0f;
        vDouble = 0.0f;
        vString = "";
        vStringBuffer[0] = '\0' ;
        vFlag = false;
    }
};

const char* g_BaseParamString = "/Char A -UChar 1 +++Bool yes /Short \"-100\" /UShort 100 /Int 0b1010 -uint  0xFFFFFFFF /Long -1000 +++ulong 0o144 /Float 10.5 /Double 1e+3 /String \"123 456\" /CharArray 123456 /Flag";

void BindParams(CmdLineParser& parser, Parameters& params)
{
    parser.BindParam("Char", params.vChar);
    parser.BindParam("UChar", params.vUChar);
    parser.BindParam("Bool", params.vBool);
    parser.BindParam("Short", params.vShort);
    parser.BindParam("UShort", params.vUShort);
    parser.BindParam("Int", params.vInt);
    parser.BindParam("UInt", params.vUInt);
    parser.BindParam("Long", params.vLong);
    parser.BindParam("ULong", params.vULong);
    parser.BindParam("Float", params.vFloat);
    parser.BindParam("double", params.vDouble);
    parser.BindParam("string", params.vString);
    parser.BindParam("CharArray", params.vStringBuffer, params.size_string_buffer);
    parser.BindParamIsSet("Flag", params.vFlag);
}

void CheckParams(const Parameters& params)
{
    EXPECT_EQ(params.vChar, 'A');
    EXPECT_EQ(params.vUChar, '1');
    EXPECT_EQ(params.vBool, true);
    EXPECT_EQ(params.vShort, -100);
    EXPECT_EQ(params.vUShort, 100);
    EXPECT_EQ(params.vInt, 10);
    EXPECT_EQ(params.vUInt, 4294967295);
    EXPECT_EQ(params.vLong, -1000);
    EXPECT_EQ(params.vULong, 100);
    EXPECT_EQ(params.vFloat, 10.5);
    EXPECT_EQ(params.vDouble, 1e+3);
    EXPECT_EQ(params.vString, "123 456");
    EXPECT_EQ(strcmp(params.vStringBuffer, "123456"), 0);
    EXPECT_EQ(params.vFlag, true);
}

TEST(CmdLineParserTest, DeleteParams)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);
    int add;
    parser.BindParam("Add", add);
    EXPECT_EQ(parser.IsParamExist("add"), true);
    parser.DeleteParam("add");
    EXPECT_EQ(parser.IsParamExist("add"), false);
}

TEST(CmdLineParserTest, SetParamFlag)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    auto list_flags = parser.GetParamFlags();

    EXPECT_EQ(list_flags.second, 3);

    for (size_t i = 0; i < list_flags.second; i++)
    {
        switch (i)
        {
            case 0:
                EXPECT_EQ(strcmp(list_flags.first[i], "/"), 0);
                break;
            case 1:
                EXPECT_EQ(strcmp(list_flags.first[i], "-"), 0);
                break;
            case 2:
                EXPECT_EQ(strcmp(list_flags.first[i], "+++"), 0);
                break;
        }
    }

    int value;
    parser.AddParamFlag( "?");
    EXPECT_EQ(parser.IsFlagExist("?"), true);
    parser.BindParam("Check Int", value);
    EXPECT_EQ(parser.IsParamExist("Check Int"), true);
    parser.Parse("\"?Check Int\" 1000");
    EXPECT_EQ(value, 1000);
    parser.DeleteParamFlag("?");
    EXPECT_EQ(parser.IsFlagExist("?"), false);
    EXPECT_EQ(parser.DeleteParam("Check Int"), true);
    EXPECT_EQ(parser.IsParamExist("Check Int"), false);
}

TEST(CmdLineParserTest, SimpleParsing)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    EXPECT_NO_THROW(parser.Parse(g_BaseParamString));

    CheckParams(params);
}

TEST(CmdLineParserTest, SimpleArgumentParsing)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    const char*argv[]=
            {
                    "/char", "A",
                    "-uchar", "1",
                    "+++bool", "yes",
                    "/short", "-100",
                    "/ushort", "100",
                    "/int", "0b1010",
                    "/uint", "0xFFFFFFFF",
                    "/long", "-1000",
                    "/ulong", "0o144",
                    "/Float",  "10.5",
                    "/Double", "1e+3",
                    "/String", "123 456",
                    "/CharArray", "123456",
                    "/Flag"
            };

    int argc = std::extent <decltype(argv)>::value;
    EXPECT_NO_THROW(parser.Parse(argc,argv));

    CheckParams(params);
}

TEST(CmdLineParserTest, CnCharAsNumber)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.AddConstrains("Char", CN_CHAR_AS_NUMBER);
    parser.AddConstrains("UChar", CN_CHAR_AS_NUMBER);
    parser.Parse("/Char 0x64 /UChar 0xFF");

    EXPECT_EQ(params.vChar, 0x64);
    EXPECT_EQ(params.vUChar, 0xFF);
}

TEST(CmdLineParserTest, CnMandatory)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    bool bException = false;
    int mandatory;

    parser.BindParam("mandatory", mandatory);
    parser.AddConstrains("mandatory", CN_MANDATORY);

    try
    {
        parser.Parse("/Int 1000");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_NOT_DEFINED);
    }

    EXPECT_EQ(bException, true);
    EXPECT_EQ(parser.IsValueAssigned("mandatory"), false);

    parser.DeleteConstrains("mandatory", CN_MANDATORY);
    EXPECT_NO_THROW(parser.Parse("/Int 1000"));
}

TEST(CmdLineParserTest, CnNoDuplicate)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    bool bException = false;
    parser.Parse("/Int 1000 /Int 1001");
    EXPECT_EQ(params.vInt, 1001);
    parser.AddConstrains("Int", CN_NO_DUPLICATE);

    try
    {
        parser.Parse("/Int 1000 /Int 1001");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_DUPLICATE);
    }

    EXPECT_EQ(bException, true);
}

TEST(CmdLineParserTest, CnRange)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.SetRangeConstrain("Int", -100, 100);
    bool bException = false;
    try
    {
        parser.Parse("/Int 1000");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_OUT_OF_RANGE);
    }
    EXPECT_EQ(bException, true);

    parser.SetRangeConstrain("ULong", 0, 1000);
    bException = false;
    try
    {
        parser.Parse("/ULong 0xFFFFFFFF");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_OUT_OF_RANGE);
    }
    EXPECT_EQ(bException, true);

    parser.SetRangeConstrain("Double", 1, 10);
    bException = false;
    try
    {
        parser.Parse("/Double 0.5");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_OUT_OF_RANGE);
    }
    EXPECT_EQ(bException, true);

    parser.ClearConstrains();
    EXPECT_NO_THROW(parser.Parse("/Int -1000 -ULong 0xFFFFFFFF -Double 0.5"));

    EXPECT_EQ(params.vInt, -1000);
    EXPECT_EQ(params.vULong, 0xFFFFFFFF);
    EXPECT_EQ(params.vDouble, 0.5);
}

TEST(CmdLineParserTest, CnLength)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.AddConstrains("Char", CN_CHAR_AS_NUMBER);
    parser.SetLengthConstrain("Char", 2);
    parser.SetLengthConstrain("Long", 5);

    bool bException = false;
    try
    {
        parser.Parse("/Char 123 ");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_TOO_LONG);
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Long 123456 ");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_TOO_LONG);
    }
    EXPECT_EQ(bException, true);

    bException = false;
    parser.DeleteConstrains("Char", CN_LENGTH);
    parser.DeleteConstrains("Long", CN_LENGTH);
    EXPECT_NO_THROW(parser.Parse("/Char 123 /Long 123456 "));
    EXPECT_EQ(params.vChar, 123);
    EXPECT_EQ(params.vLong, 123456);

}

TEST(CmdLineParserTest, CnListValue)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.AssignConstrains("Char", CN_NONE);
    parser.AddValueConstrain("Char", "A");
    parser.AddValueConstrain("Char", "B");
    parser.AddValueConstrain("Char", "C");

    bool bException = false;
    try
    {
        parser.Parse("/Char D ");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_INVALID_VALUE);
    }
    EXPECT_EQ(bException, true);

    EXPECT_NO_THROW(parser.Parse("/Char c "));

    parser.DeleteConstrains("Char", CN_LIST_VALUE);
    EXPECT_NO_THROW(parser.Parse("/Char X "));
    EXPECT_EQ(params.vChar, 'X');

    parser.AddValueConstrain("Int", 1);
    parser.AddValueConstrain("Int", 2);
    parser.AddValueConstrain("Int", 3);
    bException = false;
    try
    {
        parser.Parse("/Int 10");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_INVALID_VALUE);
    }
    EXPECT_EQ(bException, true);

    bException = false;
    EXPECT_NO_THROW(parser.Parse("/Int 1"));
    EXPECT_EQ(params.vInt, 1);

    parser.DeleteConstrains("Int", CN_LIST_VALUE);
    EXPECT_NO_THROW(parser.Parse("/Int 10"));
    EXPECT_EQ(params.vInt, 10);
}

TEST(CmdLineParserTest, CnCaseSensitive)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.AssignConstrains("Char", CN_LIST_VALUE | CN_CASE_SENSITIVE);

    parser.AddValueConstrain("Char", "A");
    parser.AddValueConstrain("Char", "B");
    parser.AddValueConstrain("Char", "C");

    bool bException = false;
    try
    {
        parser.Parse("/Char c ");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_INVALID_VALUE);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    EXPECT_NO_THROW(parser.Parse("/Char C "));

    parser.DeleteConstrains("Char", CN_CASE_SENSITIVE);
    EXPECT_NO_THROW(parser.Parse("/Char C "));
}

TEST(CmdLineParserTest, CnNoNegativeUnsigned)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.AddConstrains("uchar", CN_CHAR_AS_NUMBER);
    parser.AddConstrains("uchar", CN_NO_NEGATIVE_UNSIGNED);
    parser.AddConstrains("ushort", CN_NO_NEGATIVE_UNSIGNED);
    parser.AddConstrains("uint", CN_NO_NEGATIVE_UNSIGNED);
    parser.AddConstrains("ulong", CN_NO_NEGATIVE_UNSIGNED);

    bool bException;
    bException = false;
    try
    {
        parser.Parse("/UChar -123 ");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NEGATIVE);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/UShort -123 ");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NEGATIVE);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Uint -123 ");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NEGATIVE);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Ulong -123 ");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NEGATIVE);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    parser.DeleteConstrains("uchar", CN_NO_NEGATIVE_UNSIGNED);
    parser.DeleteConstrains("ushort", CN_NO_NEGATIVE_UNSIGNED);
    parser.DeleteConstrains("uint", CN_NO_NEGATIVE_UNSIGNED);
    parser.DeleteConstrains("ulong", CN_NO_NEGATIVE_UNSIGNED);

    EXPECT_NO_THROW(parser.Parse("/Uchar -123 /Ushort -123 /Uint -123 /Ulong -123 "));
}

TEST(CmdLineParserTest, SetNegativeUnsigned)
{
    CmdLineParser parser({"/", "-", "+++"});
    Parameters params;
    BindParams(parser, params);
    parser.AddConstrains("uchar", CN_CHAR_AS_NUMBER);

    parser.Parse("/Uchar -123 /Ushort -123 /Uint -123 /Ulong -123 ");

    unsigned char ch = -123;
    EXPECT_EQ(params.vUChar,  ch);
    unsigned short sh = -123;
    EXPECT_EQ(params.vUShort, sh);
    unsigned int ui = -123;
    EXPECT_EQ(params.vUInt, ui);
    unsigned long ul = -123;
    EXPECT_EQ(params.vULong,  ul);
}

class classFunc
{
public:
    classFunc(Parameters& p): params(p) {}

    void operator() (const char* paramName, const char* paramValue)
    {
        strncpy(params.vStringBuffer, paramValue, Parameters::size_string_buffer);
    }
    void operator() (const char* paramName)
    {
        params.vFlag = true;
    }

    Parameters& params;
};

TEST(CmdLineParserTest, BindCallback)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.BindParam("Char", CmdLineParser::callback_string_t(
            [&params](const char* paramName, const char* paramValue) {params.vChar = paramValue[0]; }
    ));
    parser.BindParam("UChar", CmdLineParser::callback_string_t(
            [&params](const char* paramName, const char* paramValue) {params.vUChar = paramValue[0]; }
    ));

    parser.BindParam("BOOL", CmdLineParser::callback_bool_t(
            [&params](const char* paramName, bool paramValue) {params.vBool = paramValue; }
    ));

    parser.BindParam("short", CmdLineParser::callback_long_t(
            [&params](const char* paramName, long paramValue) {params.vShort = (short)paramValue; }
    ));

    parser.BindParam("ushort", CmdLineParser::callback_ulong_t(
            [&params](const char* paramName, unsigned long paramValue) {params.vUShort = (unsigned short) paramValue; }
    ));

    parser.BindParam("int", CmdLineParser::callback_long_t(
            [&params](const char* paramName, long paramValue) {params.vInt = (int)paramValue; }
    ));

    parser.BindParam("uint", CmdLineParser::callback_ulong_t(
            [&params](const char* paramName, unsigned long paramValue) {params.vUInt = (unsigned int)paramValue; }
    ));

    parser.BindParam("long", CmdLineParser::callback_long_t(
            [&params](const char* paramName, long paramValue) {params.vLong = paramValue; }
    ));

    parser.BindParam("ulong", CmdLineParser::callback_ulong_t(
            [&params](const char* paramName, unsigned long paramValue) {params.vULong = paramValue; }
    ));

    parser.BindParam("float", CmdLineParser::callback_double_t(
            [&params](const char* paramName, double paramValue) {params.vFloat = (float)paramValue; }
    ));

    parser.BindParam("double", CmdLineParser::callback_double_t(
            [&params](const char* paramName, double paramValue) {params.vDouble = paramValue; }
    ));

    parser.BindParam("string", CmdLineParser::callback_string_t(
            [&params](const char* paramName, const char* paramValue) {params.vString = paramValue; }
    ));

    parser.BindParam("CharArray", CmdLineParser::callback_string_t(classFunc(params)));
    parser.BindParam("Flag", CmdLineParser::callback_param_set_t(classFunc(params)));

    EXPECT_NO_THROW(parser.Parse(g_BaseParamString));
    CheckParams(params);
}

TEST(CmdLineParserTest, RadixPrefix)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.SetRadixPrefix(CmdLineParser::BIN, "?");
    parser.SetRadixPrefix(CmdLineParser::OCT, "!");
    parser.SetRadixPrefix(CmdLineParser::HEX, "#-#");
    parser.AddConstrains("UChar", CN_CHAR_AS_NUMBER);
    params.vULong = 98;

    EXPECT_NO_THROW(parser.Parse("/UChar #-#FF /Int ?1010 /Long !1750 -Double -.3E-5 /String #"));

    EXPECT_EQ(params.vUChar, 0xFF);
    EXPECT_EQ(params.vInt, 10);
    EXPECT_EQ(params.vLong, 1000);
    EXPECT_EQ(params.vString, "#");
    EXPECT_EQ(params.vDouble, -.3e-5);
    EXPECT_EQ(params.vULong, 98);
}

TEST(CmdLineParserTest, NotNumeric)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.AddConstrains("Char", CN_CHAR_AS_NUMBER);
    bool bException = false;
    try
    {
        parser.Parse("/Char &");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NOT_NUMERIC);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Short 123#");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NOT_NUMERIC);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/UShort 12X3");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NOT_NUMERIC);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Long 1.5");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NOT_NUMERIC);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/ULong x1000");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NOT_NUMERIC);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Double -18.3e+7?");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NOT_NUMERIC);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    parser.SetRadixPrefix(CmdLineParser::HEX, "0x");
    try
    {
        parser.Parse("/Long 0x");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_NOT_NUMERIC);
        bException = true;
    }
    EXPECT_EQ(bException, true);
}

TEST(CmdLineParserTest, Overload)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.AddConstrains("Char", CN_CHAR_AS_NUMBER);
    parser.AddConstrains("UChar", CN_CHAR_AS_NUMBER);
    parser.SetRadixPrefix(CmdLineParser::HEX, "0x");
    bool bException;

    bException = false;
    try
    {
        parser.Parse("/Char -200");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_OVERLOAD);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Uchar 999");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_OVERLOAD);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Short 0xFFFF");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_OVERLOAD);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/UShort 0xFFFFF");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_OVERLOAD);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Int 0xFFFFFFFF");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_OVERLOAD);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/UINT 4294967297");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_OVERLOAD);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Long 42949672954294967295");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_OVERLOAD);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    bException = false;
    try
    {
        parser.Parse("/Double 1e1000");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_OVERLOAD);
        bException = true;
    }
    EXPECT_EQ(bException, true);

}

TEST(CmdLineParserTest, Errors)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    //Unknown parameter
    parser.SetIgnoreUnknownParams(false);
    bool bException = false;
    try
    {
        parser.Parse("/Unknown");
    }
    catch (CmdLineParseException& e)
    {
        EXPECT_EQ(e.GetErrorCode(), E_UNKNOWN_KEY);
        bException = true;
    }
    EXPECT_EQ(bException, true);

    parser.SetIgnoreUnknownParams(true);
    EXPECT_NO_THROW(parser.Parse("/Unknown"));

    //Wrong Yes/No value
    parser.SetBoolYesValue("Ok");
    parser.SetBoolNoValue("Cancel");
    bool bool1, bool2;
    parser.BindParam("bool1", bool1);
    parser.BindParam("bool2", bool2);
    bException = false;
    try
    {
        parser.Parse("/bool1 Ok /bool2 No");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_INVALID_VALUE);
    }
    EXPECT_EQ(bException, true);

    EXPECT_NO_THROW(parser.Parse("/bool1 Ok /bool2 Cancel"));
    EXPECT_EQ(bool1, true);
    EXPECT_EQ(bool2, false);

    //Empty string
    EXPECT_NO_THROW(parser.Parse(""));

    //Empty with mandatory
    int integer;
    parser.BindParam("Int", integer);
    parser.AddConstrains("Int", CN_MANDATORY);
    try
    {
        parser.Parse("");
    }
    catch (CmdLineParseException& e)
    {
        bException = true;
        EXPECT_EQ(e.GetErrorCode(), E_NOT_DEFINED);
    }
    EXPECT_EQ(bException, true);

    //Only flags
    parser.DeleteConstrains("Int", CN_MANDATORY);
    EXPECT_NO_THROW(parser.Parse("/ -"));
    EXPECT_NO_THROW(parser.Parse("/-"));
    EXPECT_NO_THROW(parser.Parse("/"));
}

TEST(CmdLineParserTest, ParamNameSensitive)
{
    CmdLineParser parser({ "/", "-", "+++" });
    Parameters params;
    BindParams(parser, params);

    parser.SetParamNameCaseSensitive(true);
    int val1 = 0, val2 = 0;
    parser.BindParam("c",val1);
    parser.BindParam("C", val2);

    parser.Parse("/c 1 /C 2");
    EXPECT_EQ(val1, 1);
    EXPECT_EQ(val2, 2);
}

