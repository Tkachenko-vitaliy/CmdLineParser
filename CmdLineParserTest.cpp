// CmdLineParserTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CmdLineParser.h"
#include <assert.h>

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

Parameters params;
CmdLineParser parser({ "/", "-", "+++" });

const char* g_BaseParamString = "/Char A -UChar 1 +++Bool yes /Short \"-100\" /UShort 100 /Int 0b1010 -uint  0xFFFFFFFF /Long -1000 +++ulong 0o144 /Float 10.5 /Double 1e+3 /String \"123 456\" /CharArray 123456 /Flag";

void BindParams()
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

void CheckParams()
{
    assert(params.vChar == 'A');
    assert(params.vUChar == '1');
    assert(params.vBool == true);
    assert(params.vShort == -100);
    assert(params.vUShort == 100);
    assert(params.vInt == 10);
    assert(params.vUInt == 4294967295);
    assert(params.vLong == -1000);
    assert(params.vULong == 100);
    assert(params.vFloat == 10.5);
    assert(params.vDouble == 1e+3);
    assert(params.vString == "123 456");
    assert(strcmp(params.vStringBuffer, "123456") == 0);
    assert(params.vFlag == true);
}

void T_IterateParams()
{
    CmdLineParser::ParamIterator iter = parser.GetParamIterator();
    unsigned int index = 0;
    while (iter)
    {
        switch (index)
        {
        case 0:
            assert(_stricmp(iter.paramName, "Char") == 0);
            break;
        case 1:
            assert(_stricmp(iter.paramName, "UChar") == 0);
            break;
        case 2:
            assert(_stricmp(iter.paramName, "Bool") == 0);
            break;
        case 3:
            assert(_stricmp(iter.paramName, "Short") == 0);
            break;
        case 4:
            assert(_stricmp(iter.paramName, "UShort") == 0);
            break;
        case 5:
            assert(_stricmp(iter.paramName, "Int") == 0);
            break;
        case 6:
            assert(_stricmp(iter.paramName, "UInt") == 0);
            break;
        case 7:
            assert(_stricmp(iter.paramName, "Long") == 0);
            break;
        case 8:
            assert(_stricmp(iter.paramName, "ULong") == 0);
            break;
        case 9:
            assert(_stricmp(iter.paramName, "Float") == 0);
            break;
        case 10:
            assert(_stricmp(iter.paramName, "Double") == 0);
            break;
        case 11:
            assert(_stricmp(iter.paramName, "String") == 0);
            break;
        case 12:
            assert(_stricmp(iter.paramName, "CharArray") == 0);
            break;
        case 13:
            assert(_stricmp(iter.paramName, "Flag") == 0);
            break;
        }
        index++;
        iter++;
    }
}

void T_DeleteParams()
{
    int add;
    parser.BindParam("Add", add);
    assert(parser.IsParamExist("add"));
    parser.DeleteParam("add");
    assert(!parser.IsParamExist("add"));
    parser.BindParam("Add", add);
    auto it = parser.GetParamIterator("add");
    assert(it);
    parser.DeleteParam(it);
    assert(!parser.IsParamExist("add"));
}

void T_SetParamFlag()
{
    //CmdLineParser parser({ "/", "-", "+++" });
    auto list_flags = parser.GetParamFlags();

    assert(list_flags.second == 3);

    for (size_t i = 0; i < list_flags.second; i++)
    {
        switch (i)
        {
        case 0:
            assert(strcmp(list_flags.first[i], "/") == 0);
            break;
        case 1:
            assert(strcmp(list_flags.first[i], "-") == 0);
            break;
        case 2:
            assert(strcmp(list_flags.first[i], "+++") == 0);
            break;
        }
    }

    int value;
    parser.AddParamFlag({ "?" });
    assert(parser.IsFlagExist("?"));
    parser.BindParam("Check Int", value);
    assert(parser.IsParamExist("Check Int"));
    parser.Parse("\"?Check Int\" 1000");
    assert(value == 1000);
    parser.DeleteParamFlag("?");
    assert(!parser.IsFlagExist("?"));
    assert(parser.DeleteParam("Check Int"));
    assert(!parser.IsParamExist("Check Int"));
}

void T_SimpleParsing()
{
    params.reset();

    try
    {
        parser.Parse(g_BaseParamString);
    }
    catch (std::exception& e)
    {
        const char* error = e.what();
        assert(false);
    }

    CheckParams();
}

void T_SimpleArgumentParsing()
{
    params.reset();

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

    try
    {
        parser.Parse(argc,argv);
    }
    catch (std::exception& e)
    {
        const char* error = e.what();
        assert(false);
    }
    CheckParams();
}

void T_CN_CHAR_AS_NUMBER()
{
    parser.AddConstrains("Char", CmdLineParser::CN_CHAR_AS_NUMBER);
    parser.AddConstrains("UChar", CmdLineParser::CN_CHAR_AS_NUMBER);
    parser.Parse("/Char 0x64 /UChar 0xFF");
    assert(params.vChar == 0x64);
    assert(params.vUChar == 0xFF);
    parser.DeleteConstrains("Char", CmdLineParser::CN_CHAR_AS_NUMBER);
    parser.DeleteConstrains("UChar", CmdLineParser::CN_CHAR_AS_NUMBER);
}

void T_CN_MANDATORY()
{
    bool bException = false;
    int mandatory;
    parser.BindParam("mandatory", mandatory);
    parser.AddConstrains("mandatory", CmdLineParser::CN_MANDATORY);
    try
    {
        parser.Parse("/Int 1000");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_NOT_DEFINED);
    }
    assert(bException);
    assert(!parser.IsValueAssigned("mandatory"));

    bException = false;
    parser.DeleteConstrains("mandatory", CmdLineParser::CN_MANDATORY);
    try
    {
        parser.Parse("/Int 1000");
    }
    catch (CmdLineParser::CmdLineParseException&)
    {
        bException = true;
    }
    assert(!bException);
}

void T_CN_NO_DUPLICATE()
{
    bool bException = false;
    parser.Parse("/Int 1000 /Int 1001");
    assert(params.vInt == 1001);
    parser.AddConstrains("Int", CmdLineParser::CN_NO_DUPLICATE);
    try
    {
        parser.Parse("/Int 1000 /Int 1001");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_DUPLICATE);
        const char* errorString = e.what();
    }
    assert(bException);
}

void T_CN_RANGE()
{
    parser.SetRangeConstrain("Int", -100, 100);
    bool bException = false;
    try
    {
        parser.Parse("/Int 1000");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_OUT_OF_RANGE);
    }
    assert(bException);

    parser.SetRangeConstrain("ULong", 0, 1000);
    bException = false;
    try
    {
        parser.Parse("/ULong 0xFFFFFFFF");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_OUT_OF_RANGE);
    }
    assert(bException);

    parser.SetRangeConstrain("Double", 1, 10);
    bException = false;
    try
    {
        parser.Parse("/Double 0.5");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_OUT_OF_RANGE);
    }
    assert(bException);

    bException = false;
    parser.ClearConstrains();
    try
    {
        parser.Parse("/Int -1000 -ULong 0xFFFFFFFF -Double 0.5");
    }
    catch (std::exception&)
    {
        bException = true;
    }
    assert(!bException);
    assert(params.vInt == -1000);
    assert(params.vULong == 0xFFFFFFFF);
    assert(params.vDouble == 0.5);
}

void T_CN_LENGTH()
{
    parser.AddConstrains("Char", CmdLineParser::CN_CHAR_AS_NUMBER);
    parser.SetLengthConstrain("Char", 2);
    parser.SetLengthConstrain("Long", 5);
    bool bException = false;
    try
    {
        parser.Parse("/Char 123 ");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_TOO_LONG);
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Long 123456 ");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_TOO_LONG);
    }
    assert(bException);

    bException = false;
    parser.DeleteConstrains("Char", CmdLineParser::CN_LENGTH);
    parser.DeleteConstrains("Long", CmdLineParser::CN_LENGTH);
    try
    {
        parser.Parse("/Char 123 /Long 123456 ");
    }
    catch (CmdLineParser::CmdLineParseException&)
    {
        bException = true;
    }
    assert(!bException);
    assert(params.vChar == 123);
    assert(params.vLong == 123456);

}

void T_CN_LIST_VALUE()
{
    parser.AssignConstrains("Char", CmdLineParser::CN_NONE);
    parser.AddValueConstrain("Char", "A");
    parser.AddValueConstrain("Char", "B");
    parser.AddValueConstrain("Char", "C");

    bool bException = false;
    try
    {
        parser.Parse("/Char D ");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_INVALID_VALUE);
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Char c ");
    }
    catch (CmdLineParser::CmdLineParseException&)
    {
        bException = true;
    }
    assert(!bException);

    parser.DeleteConstrains("Char", CmdLineParser::CN_LIST_VALUE);
    bException = false;
    try
    {
        parser.Parse("/Char X ");
    }
    catch (CmdLineParser::CmdLineParseException&)
    {
        bException = true;
    }
    assert(!bException);
    assert(params.vChar == 'X');

    parser.AddValueConstrain("Int", 1);
    parser.AddValueConstrain("Int", 2);
    parser.AddValueConstrain("Int", 3);
    bException = false;
    try
    {
        parser.Parse("/Int 10");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_INVALID_VALUE);
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Int 1");
    }
    catch (CmdLineParser::CmdLineParseException&)
    {
        bException = true;
    }
    assert(!bException);
    assert(params.vInt == 1);

    parser.DeleteConstrains("Int", CmdLineParser::CN_LIST_VALUE);
    bException = false;
    try
    {
        parser.Parse("/Int 10");
    }
    catch (CmdLineParser::CmdLineParseException&)
    {
        bException = true;
    }
    assert(!bException);
    assert(params.vInt == 10);
}

void T_CN_CASE_SENSITIVE()
{
    parser.AssignConstrains("Char", CmdLineParser::CN_LIST_VALUE | CmdLineParser::CN_CASE_SENSITIVE);
    
    parser.AddValueConstrain("Char", "A");
    parser.AddValueConstrain("Char", "B");
    parser.AddValueConstrain("Char", "C");

    bool bException = false;
    try
    {
        parser.Parse("/Char c ");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_INVALID_VALUE);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Char C ");
    }
    catch (CmdLineParser::CmdLineParseException&)
    {
        bException = true;
    }
    assert(!bException);

    parser.DeleteConstrains("Char", CmdLineParser::CN_CASE_SENSITIVE);
    bException = false;
    try
    {
        parser.Parse("/Char C ");
    }
    catch (CmdLineParser::CmdLineParseException&)
    {
        bException = true;
    }
    assert(!bException);
}

void T_CN_NO_NEGATIVE_UNSIGNED()
{
    parser.AddConstrains("uchar", CmdLineParser::CN_CHAR_AS_NUMBER);
    parser.AddConstrains("uchar", CmdLineParser::CN_NO_NEGATIVE_UNSIGNED);
    parser.AddConstrains("ushort", CmdLineParser::CN_NO_NEGATIVE_UNSIGNED);
    parser.AddConstrains("uint", CmdLineParser::CN_NO_NEGATIVE_UNSIGNED);
    parser.AddConstrains("ulong", CmdLineParser::CN_NO_NEGATIVE_UNSIGNED);

    bool bException;
    bException = false;
    try
    {
        parser.Parse("/UChar -123 ");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NEGATIVE);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/UShort -123 ");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NEGATIVE);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Uint -123 ");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NEGATIVE);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Ulong -123 ");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NEGATIVE);
        bException = true;
    }
    assert(bException);

    parser.DeleteConstrains("uchar", CmdLineParser::CN_NO_NEGATIVE_UNSIGNED);
    parser.DeleteConstrains("ushort", CmdLineParser::CN_NO_NEGATIVE_UNSIGNED);
    parser.DeleteConstrains("uint", CmdLineParser::CN_NO_NEGATIVE_UNSIGNED);
    parser.DeleteConstrains("ulong", CmdLineParser::CN_NO_NEGATIVE_UNSIGNED);

    bException = false;
    try
    {
        parser.Parse("/Uint -123 /Ulong -123 ");
    }
    catch (CmdLineParser::CmdLineParseException&)
    {
        bException = true;
    }
    assert(!bException);

    //assert(params.vUChar == -123);
    //assert(params.vUShort == -123);
    assert(params.vUInt == -123);
    assert(params.vULong == -123);

}


void OnString(const char* paramName, const char* paramValue)
{
    params.vString = paramValue;
}

class classFunc
{
public:
    void operator() (const char* paramName, const char* paramValue)
    {
        strcpy_s(params.vStringBuffer, Parameters::size_string_buffer, paramValue);
    }
    void operator() (const char* paramName)
    {
        params.vFlag = true;
    }
};

void T_BindCallback()
{
    params.reset();

    parser.BindParam("Char", CmdLineParser::callback_string_t(
        [](const char* paramName, const char* paramValue) {params.vChar = paramValue[0]; }
    ));
    parser.BindParam("UChar", CmdLineParser::callback_string_t(
        [](const char* paramName, const char* paramValue) {params.vUChar = paramValue[0]; }
    ));

    parser.BindParam("BOOL", CmdLineParser::callback_bool_t(
        [](const char* paramName, bool paramValue) {params.vBool = paramValue; }
    ));

    parser.BindParam("short", CmdLineParser::callback_long_t(
        [](const char* paramName, long paramValue) {params.vShort = (short)paramValue; }
    ));

    parser.BindParam("ushort", CmdLineParser::callback_ulong_t(
        [](const char* paramName, unsigned long paramValue) {params.vUShort = (unsigned short) paramValue; }
    ));

    parser.BindParam("int", CmdLineParser::callback_long_t(
        [](const char* paramName, long paramValue) {params.vInt = (int)paramValue; }
    ));

    parser.BindParam("uint", CmdLineParser::callback_ulong_t(
        [](const char* paramName, unsigned long paramValue) {params.vUInt = (unsigned int)paramValue; }
    ));

    parser.BindParam("long", CmdLineParser::callback_long_t(
        [](const char* paramName, long paramValue) {params.vLong = paramValue; }
    ));

    parser.BindParam("ulong", CmdLineParser::callback_ulong_t(
        [](const char* paramName, unsigned long paramValue) {params.vULong = paramValue; }
    ));

    parser.BindParam("float", CmdLineParser::callback_double_t(
        [](const char* paramName, double paramValue) {params.vFloat = (float)paramValue; }
    ));

    parser.BindParam("double", CmdLineParser::callback_double_t(
        [](const char* paramName, double paramValue) {params.vDouble = paramValue; }
    ));

    parser.BindParam("string", CmdLineParser::callback_string_t(OnString));

    parser.BindParam("CharArray", CmdLineParser::callback_string_t(classFunc{}));
    parser.BindParam("Flag", CmdLineParser::callback_param_set_t(classFunc{}));
    
    bool bException = false;
    try
    {
        parser.Parse(g_BaseParamString);
    }
    catch (std::exception&)
    {
        bException = true;
    }
    assert(!bException);
    CheckParams();
}

void T_RadixPrefix()
{
    parser.SetRadixPrefix(CmdLineParser::BIN, "?");
    parser.SetRadixPrefix(CmdLineParser::OCT, "!");
    parser.SetRadixPrefix(CmdLineParser::HEX, "#-#");
    parser.AddConstrains("UChar", CmdLineParser::CN_CHAR_AS_NUMBER);
    params.vULong = 98;
    
    try
    {
        parser.Parse("/UChar #-#FF /Int ?1010 /Long !1750 -Double -.3E-5 /String #");
    }
    catch (std::exception&)
    {
        assert(false);
    }
    assert(params.vUChar == 0xFF);
    assert(params.vInt == 10);
    assert(params.vLong == 1000);
    assert(params.vString == "#");
    assert(params.vDouble == -.3e-5);
    assert(params.vULong == 98);
}

void T_NotNumeric()
{
    parser.AddConstrains("Char", CmdLineParser::CN_CHAR_AS_NUMBER);
    bool bException = false;
    try
    {
        parser.Parse("/Char &");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NOT_NUMERIC);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Short 123#");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NOT_NUMERIC);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/UShort 12X3");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NOT_NUMERIC);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Long 1.5");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NOT_NUMERIC);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/ULong x1000");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NOT_NUMERIC);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Double -18.3e+7?");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NOT_NUMERIC);
        bException = true;
    }
    assert(bException);

    bException = false;
    parser.SetRadixPrefix(CmdLineParser::HEX, "0x");
    try
    {
        parser.Parse("/Long 0x");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_NOT_NUMERIC);
        bException = true;
    }
    assert(bException);
}

void T_Overload()
{
    parser.AddConstrains("Char", CmdLineParser::CN_CHAR_AS_NUMBER);
    parser.AddConstrains("UChar", CmdLineParser::CN_CHAR_AS_NUMBER);
    parser.SetRadixPrefix(CmdLineParser::HEX, "0x");
    bool bException;

    bException = false;
    try
    {
        parser.Parse("/Char -200");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_OVERLOAD);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Uchar 999");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_OVERLOAD);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Short 0xFFFF");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_OVERLOAD);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/UShort 0xFFFFF");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_OVERLOAD);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Int 0xFFFFFFFF");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_OVERLOAD);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/UINT 4294967297");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_OVERLOAD);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Long 4294967295");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_OVERLOAD);
        bException = true;
    }
    assert(bException);

    bException = false;
    try
    {
        parser.Parse("/Double 1e1000");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_OVERLOAD);
        bException = true;
    }
    assert(bException);

}

void T_Errors()
{
    //Unknown parameter
    parser.SetIgnoreUnknownParams(false);
    bool bException = false;
    try
    {
        parser.Parse("/Unknown");
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        assert(e.GetErrorCode() == CmdLineParser::E_UNKNOWN_KEY);
        bException = true;
    }
    assert(bException);

    parser.SetIgnoreUnknownParams(true);
    bException = false;
    try
    {
        parser.Parse("/Unknown");
    }
    catch (std::exception&)
    {
        bException = true;
    }
    assert(!bException);

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
    catch (CmdLineParser::CmdLineParseException& e)
    {
        bException = true;
        assert(e.GetErrorCode() == CmdLineParser::E_INVALID_VALUE);
    }
    assert(bException);
    try
    {
        parser.Parse("/bool1 Ok /bool2 Cancel");
    }
    catch (std::exception&)
    {
        assert(false);
    }
    assert(bool1 = true);
    assert(bool2 == false);

    //Not numeric
    T_NotNumeric();

    //Overload
    T_Overload();

    //Empty string
    parser.Parse("");

    //Only flags
    parser.Parse("/ -");
    parser.Parse("/-");
    parser.Parse("/");
   
}

int _tmain(int argc, _TCHAR* argv[])
{
    BindParams();

    T_IterateParams();
    T_DeleteParams();
    T_SetParamFlag();
    T_SimpleParsing();
    T_SimpleArgumentParsing();
    T_CN_CHAR_AS_NUMBER();
    T_CN_MANDATORY();
    T_CN_NO_DUPLICATE();
    T_CN_RANGE();
    T_CN_LENGTH();
    T_CN_LIST_VALUE();
    T_CN_CASE_SENSITIVE();
    T_CN_NO_NEGATIVE_UNSIGNED();
    T_BindCallback();
    BindParams();
    T_RadixPrefix();
    T_NotNumeric();
    T_Overload();
    T_Errors();

	return 0;
}

