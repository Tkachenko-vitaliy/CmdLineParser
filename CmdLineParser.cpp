#include "CmdLineParser.h"

#include <limits>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <assert.h>
#include <string.h>

#ifdef  WIN32
    #define strcasecmp _stricmp
#endif

CmdLineParser::CmdLineParser():
        mListRadixPrefix({ "0b", "0o", "0x" })
        , mListRadixBase({ 2, 8, 16 })
        , mBoolYesValue("Yes")
        , mBoolNoValue("No")
        , mIgnoreUnknownParams(true)
        , mParamNameCaseSensitive(false)
{
}

CmdLineParser::CmdLineParser(std::initializer_list<const char*> listParamFlag) :
        mListRadixPrefix({ "0b", "0o", "0x" })
        , mListRadixBase({ 2, 8, 16 })
        , mBoolYesValue("Yes")
        , mBoolNoValue("No")
        , mIgnoreUnknownParams(true)
        , mParamNameCaseSensitive(false)
        , mListFlag(listParamFlag)
{
}

CmdLineParser::~CmdLineParser()
{

}

bool IsNegativeNumeric(const char* szValue)
{
    if (szValue[0] != '-')
    {
        return false;
    }

    if (szValue[1] != '\0')
    {
        if (szValue[1] == '.' || isdigit(szValue[1]))
        {
            return true;
        }
    }

    return false;
}

void CmdLineParser::ParseItem(const char* stringItem)
{
    bool bFlag = false;

    auto it = std::find_if(mListFlag.begin(), mListFlag.end(),
                           [stringItem](const char* keySign)
                           {
                               return strstr(stringItem, keySign) == stringItem && //key sign must be placed at the begin of the parameter
                                      !IsNegativeNumeric(stringItem);                         //Value is not a negative numeric (in order not to confuse with key)
                           }
    );

    if (it != mListFlag.end())
    {
        // remove flag specifier
        bFlag = true;
        stringItem += strlen(*it);
        if (*stringItem == '\0') //only param sign without definition at the end of the string.
        {
            return;
        }
    }

    Analyze(stringItem, bFlag);
}

void CmdLineParser::Reset()
{
    for (auto &descr : mListParamDescriptors)
    {
        descr.isAssign = false;
    }

    mCurrentParam = nullptr;
}

void CmdLineParser::Parse(int argc, const char* const argv[], int argStart)
{
    Reset();

    for (int i = argStart; i < argc; i++)
    {
        const char* pszParam = argv[i];
        ParseItem(pszParam);
    }
    OnFinish();
}

void CmdLineParser::Parse(const char* cmdLineString)
{
    Reset();

    enum { st_start = 0, st_enter = 1, st_quote, st_end } state = st_start;

    unsigned int position = 0;
    std::string stringAccum;

    while (cmdLineString[position] != '\0')
    {
        switch (state)
        {
            case st_start:
                switch (cmdLineString[position])
                {
                    case '"':
                    {
                        state = st_quote;
                    }
                        break;
                    default:
                        if (cmdLineString[position] != ' ')
                        {
                            stringAccum.push_back(cmdLineString[position]);
                            state = st_enter;
                        }
                }
                break;

            case st_enter:
                switch (cmdLineString[position])
                {
                    case ' ':
                        state = st_end;
                        break;
                    default:
                        stringAccum.push_back(cmdLineString[position]);
                }
                break;

            case st_quote:
                switch (cmdLineString[position])
                {
                    case '"':
                        state = st_end;
                        break;
                    default:
                        stringAccum.push_back(cmdLineString[position]);
                }
                break;

            case st_end:
                //it will be processed further. We process this case in order to eliminate warning
                break;
        }

        if (state == st_end)
        {
            unsigned int shifted = position + 1;

            while (cmdLineString[shifted] != '\0')
            {
                if (cmdLineString[shifted] != ' ')
                {
                    break;
                }
                shifted++;
            }

            ParseItem(stringAccum.c_str());
            state = st_start;
            stringAccum.clear();
        }

        position++;
    }

    if (!stringAccum.empty())
    {
        ParseItem(stringAccum.c_str());
    }

    OnFinish();
}

CmdLineParser::radix_t CmdLineParser::GetNumberRadix(const char* szValue)
{
    radix_t radix;
    radix.radix = 10;
    radix.prefix_size = 0;

    for (unsigned int i = 0; i < mListRadixPrefix.size(); i++)
    {
        const char* pstr = strstr(szValue, mListRadixPrefix[i]);
        if (pstr == szValue) //prefix mist be placed in the begin position
        {
            radix.radix = mListRadixBase[i];
            radix.prefix_size = strlen(mListRadixPrefix[i]);
            break;
        }
    }

    return radix;
}

void CmdLineParser::LongFromString(const char* paramName, const char* numberString, long& number, int radix)
{
    if (numberString[0] == '\0')
    {
        //string empty can be in case if only radix prefix is entered without value
        throw CmdLineParseException(paramName, E_NOT_NUMERIC);
    }

    char* stringstop;
    errno = 0;

    number = strtol(numberString, &stringstop, radix);

    if (errno == ERANGE)
    {
        throw CmdLineParseException(paramName, E_OVERLOAD);
    }

    while (*stringstop != '\0' && (*stringstop == ' ' || iscntrl(*stringstop)))
        stringstop++;

    if (*stringstop != '\0')
    {
        throw CmdLineParseException(paramName, E_NOT_NUMERIC);
    }
}

void CmdLineParser::ULongFromString(const char* paramName, const char* numberString, unsigned long& number, int radix)
{
    if (numberString[0] == '\0')
    {
        //string empty can be in case if only radix prefix is entered without value
        throw CmdLineParseException(paramName, E_NOT_NUMERIC);
    }

    char* stringstop;
    errno = 0;

    number = strtoul(numberString, &stringstop, radix);

    if (errno == ERANGE)
    {
        throw CmdLineParseException(paramName, E_OVERLOAD);
    }

    while (*stringstop != '\0' && (*stringstop == ' ' || iscntrl(*stringstop)))
        stringstop++;

    if (*stringstop != '\0')
    {
        throw CmdLineParseException(paramName, E_NOT_NUMERIC);
    }

}

void CmdLineParser::DoubleFromString(const char* paramName, const char* numberString, double& number)
{
    if (numberString[0] == '\0')
    {
        //string empty can be in case if only radix prefix is entered without value
        throw CmdLineParseException(paramName, E_NOT_NUMERIC);
    }

    char* stringstop;
    errno = 0;

    number = strtod(numberString, &stringstop);

    if (errno == ERANGE)
    {
        throw CmdLineParseException(paramName, E_OVERLOAD);
    }

    while (*stringstop != '\0' && (*stringstop == ' ' || iscntrl(*stringstop)))
        stringstop++;

    if (*stringstop != '\0')
    {
        throw CmdLineParseException(paramName, E_NOT_NUMERIC);
    }

}

void CmdLineParser::CheckValueConstrains(ParamDescriptor* descriptor, const char* value)
{
    if ((descriptor->constrainRules & CN_LIST_VALUE) == 0)
    {
        return;
    }

    for (auto& i : descriptor->valueConstrains)
    {
        int res;
        if (descriptor->constrainRules & CN_CASE_SENSITIVE)
        {
            res = strcmp(value, i);
        }
        else
        {
            res = strcasecmp(value, i);
        }

        if (res == 0)
        {
            return;
        }
    }

    if (descriptor->valueConstrains.size() > 0)
    {
        throw CmdLineParseException(descriptor->paramName, E_INVALID_VALUE);
    }

}

void CmdLineParser::CheckNumericValueConstrains(ParamDescriptor* descriptor, double value)
{
    if ((descriptor->constrainRules & CN_LIST_VALUE) == 0)
    {
        return;
    }

    for (auto i = descriptor->numericValueConstrains.begin(); i != descriptor->numericValueConstrains.end(); i++)
    {
        if (*i == value)
        {
            return;
        }
    }

    if (descriptor->numericValueConstrains.size() > 0)
    {
        throw CmdLineParseException(descriptor->paramName, E_INVALID_VALUE);
    }
}

void CmdLineParser::AssignParamFlags(std::initializer_list<const char*> listParamFlag)
{
    mListFlag = listParamFlag;
}

std::pair<const char**, size_t> CmdLineParser::GetParamFlags()
{
    return { mListFlag.data(), mListFlag.size() };
}

void CmdLineParser::AddParamFlag(const char* paramFlag)
{
    auto findIT = std::find_if(mListFlag.begin(), mListFlag.end(), [paramFlag](const char* item_mListFlag) {return strcmp(paramFlag, item_mListFlag) == 0; });

    if (findIT == mListFlag.end())
    {
        mListFlag.push_back(paramFlag);
    }

}

void CmdLineParser::DeleteParamFlag(const char* paramFlag)
{
    auto findIT = std::find_if(mListFlag.begin(), mListFlag.end(), [paramFlag](const char* item_mListFlag) {return strcmp(paramFlag, item_mListFlag) == 0; });

    if (findIT != mListFlag.end())
    {
        mListFlag.erase(findIT);
    }
}

bool CmdLineParser::IsFlagExist(const char* paramFlag) const
{
    auto findIT = std::find_if(mListFlag.begin(), mListFlag.end(), [paramFlag](const char* item_mListFlag) {return strcmp(paramFlag, item_mListFlag) == 0; });

    return findIT != mListFlag.end();
}

void CmdLineParser::ClearParamFlags()
{
    mListFlag.clear();
}

void CmdLineParser::BindParam(const char* paramName, char& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_CHAR;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, unsigned char& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_UCHAR;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, bool& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_BOOL;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, short& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_SHORT;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, unsigned short& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_USHORT;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, int& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_INT;
    descr.paramData = &paramValue;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, unsigned int& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_UINT;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);

}

void CmdLineParser::BindParam(const char* paramName, long& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_LONG;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, unsigned long& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_ULONG;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, float& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_FLOAT;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, double& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_DOUBLE;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}


void CmdLineParser::BindParam(const char* paramName, std::string& paramValue, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_STRING;
    descr.paramData = &paramValue;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, char* paramValue, unsigned long length, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_CHAR_BUFFER;
    descr.paramData = paramValue;
    descr.length = length;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParamIsSet(const char* paramName, bool& isParamSet)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_FLAG;
    descr.paramData = &isParamSet;
    AssignDescriptor(descr);
}


void CmdLineParser::BindParam(const char* paramName, callback_string_t callbackFunction, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_STRING;
    descr.callbackContainer.funcCharPtr = callbackFunction;
    descr.isCallback = true;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, callback_bool_t callbackFunction, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_BOOL;
    descr.callbackContainer.funcBool = callbackFunction;
    descr.isCallback = true;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, callback_char_t callbackFunction, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_CHAR;
    descr.length = 1;
    descr.callbackContainer.funcChar = callbackFunction;
    descr.isCallback = true;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}


void CmdLineParser::BindParam(const char* paramName, callback_long_t callbackFunction, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_LONG;
    descr.callbackContainer.funcLong = callbackFunction;
    descr.isCallback = true;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, callback_ulong_t callbackFunction, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_ULONG;
    descr.callbackContainer.funcULong = callbackFunction;
    descr.isCallback = true;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, callback_double_t callbackFunction, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_DOUBLE;
    descr.callbackContainer.funcDouble = callbackFunction;
    descr.isCallback = true;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::BindParam(const char* paramName, callback_param_set_t callbackFunction, rule_mask_t ruleMask)
{
    ParamDescriptor descr;
    descr.paramName = paramName;
    descr.paramType = P_FLAG;
    descr.callbackContainer.funcIsSet= callbackFunction;
    descr.isCallback = true;
    descr.constrainRules = ruleMask;
    AssignDescriptor(descr);
}

void CmdLineParser::AssignConstrains(const char* paramName, rule_mask_t rulesMask)
{
    GetParamDescriptor(paramName)->constrainRules = rulesMask;
}

void CmdLineParser::AddConstrains(const char* paramName, rule_mask_t rulesMask)
{
    GetParamDescriptor(paramName)->constrainRules |= rulesMask;
}

void CmdLineParser::DeleteConstrains(const char* paramName, rule_mask_t rulesMask)
{
    GetParamDescriptor(paramName)->constrainRules &= ~rulesMask;
}

void CmdLineParser::ClearConstrains()
{
    for (ParamDescriptor& param: mListParamDescriptors)
    {
        param.constrainRules = CN_NONE;
    }
}

void CmdLineParser::SetIgnoreUnknownParams(bool bIgnore)
{
    mIgnoreUnknownParams = bIgnore;
}

bool CmdLineParser::GetIgnoreUnknownParams() const
{
    return mIgnoreUnknownParams;
}

void CmdLineParser::SetParamNameCaseSensitive(bool bSensitive)
{
    mParamNameCaseSensitive = bSensitive;
}

bool CmdLineParser::GetParamNameCaseSensitive() const
{
    return mParamNameCaseSensitive;
}

void CmdLineParser::SetBoolYesValue(const char* szValue)
{
    mBoolYesValue = szValue;
}

void CmdLineParser::SetBoolNoValue(const char* szValue)
{
    mBoolNoValue = szValue;
}

const char* CmdLineParser::GetBoolYesValue() const
{
    return mBoolYesValue;
}

const char* CmdLineParser::GetBoolNoValue() const
{
    return mBoolNoValue;
}

void CmdLineParser::SetRadixPrefix(numeric_radix_t radix_type, const char* szPrefix)
{
    mListRadixPrefix[radix_type] = szPrefix;
}

const char*  CmdLineParser::GetRadixPrefix(numeric_radix_t radix_type) const
{
    return  mListRadixPrefix[radix_type];
}

void CmdLineParser::SetRangeConstrain(const char* paramName, double minValue, double maxValue)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    descr->constrainRules |= CN_RANGE;
    descr->minValue = minValue;
    descr->maxValue = maxValue;
}

void CmdLineParser::SetLengthConstrain(const char* paramName, unsigned long length)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    descr->constrainRules |= CN_LENGTH;
    descr->length = length;
}

void CmdLineParser::AssignValueConstrains(const char* paramName, std::initializer_list<const char*> listValues)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    descr->constrainRules |= CN_LIST_VALUE;
    descr->valueConstrains = listValues;
}

void CmdLineParser::AssignValueConstrains(const char* paramName, std::initializer_list<double> listValues)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    descr->constrainRules |= CN_LIST_VALUE;
    descr->numericValueConstrains = listValues;
}

void CmdLineParser::AddValueConstrain(const char* paramName, const char* value)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    descr->constrainRules |= CN_LIST_VALUE;
    descr->valueConstrains.push_back(value);
}

void CmdLineParser::AddValueConstrain(const char* paramName, double value)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    descr->constrainRules |= CN_LIST_VALUE;
    descr->numericValueConstrains.push_back(value);
}

void CmdLineParser::DeleteValueConstrain(const char* paramName, const char* value)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    for (auto index = descr->valueConstrains.begin(); index != descr->valueConstrains.end(); index++)
    {
        int compare;
        if (descr->constrainRules & CN_CASE_SENSITIVE)
        {
            compare = strcmp(*index, value);
        }
        else
        {
            compare = strcasecmp(*index, value);
        }
        if (compare == 0)
        {
            descr->valueConstrains.erase(index);
            break;
        }
    }
}

void CmdLineParser::DeleteValueConstrain(const char* paramName, double value)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    for (auto index = descr->numericValueConstrains.begin(); index != descr->numericValueConstrains.end(); index++)
    {
        if (*index == value)
        {
            descr->numericValueConstrains.erase(index);
            break;
        }
    }
}

std::pair<const char**, size_t> CmdLineParser::GetValueConstrains(const char* paramName) const
{
    ParamDescriptor* descr = const_cast<CmdLineParser*>(this)->GetParamDescriptor(paramName);
    return { descr->valueConstrains.data(), descr->valueConstrains.size() };
}

std::pair<double*, size_t> CmdLineParser::GetNumericValueConstrains(const char* paramName) const
{
    ParamDescriptor* descr = const_cast<CmdLineParser*>(this)->GetParamDescriptor(paramName);
    return { descr->numericValueConstrains.data(), descr->numericValueConstrains.size() };
}

void CmdLineParser::ClearValueConstrains(const char* paramName)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    descr->valueConstrains.clear();
    descr->numericValueConstrains.clear();
}

bool CmdLineParser::IsValueAssigned(const char* paramName) const
{
    return const_cast<CmdLineParser*>(this)->GetParamDescriptor(paramName)->isAssign;
}

void CmdLineParser::ResetValueAssigned(const char* paramName)
{
    ParamDescriptor* descr = GetParamDescriptor(paramName);
    descr->isAssign = false;
}

void CmdLineParser::ClearValueAssigned()
{
    for (auto &descr : mListParamDescriptors)
    {
        descr.isAssign = false;
    }
}

bool CmdLineParser::IsParamExist(const char* paramName) const
{
    return const_cast<CmdLineParser*>(this)->FindParamDescriptor(paramName) != nullptr;
}

bool CmdLineParser::DeleteParam(const char* paramName)
{
    auto it = std::find_if(mListParamDescriptors.begin(), mListParamDescriptors.end(), [paramName](ParamDescriptor& descrInList)
    {return strcasecmp(descrInList.paramName, paramName) == 0; });

    if (it != mListParamDescriptors.end())
    {
        if (mCurrentParam == it.operator->())
        {
            mCurrentParam = nullptr;
        }
        mListParamDescriptors.erase(it);
        return true;
    }
    else
    {
        return false;
    }
}

void CmdLineParser::ClearParams()
{
    mListParamDescriptors.clear();
    mCurrentParam = nullptr;
}

CmdLineParser::ParamDescriptor* CmdLineParser::GetParamDescriptor(const char* paramName)
{
    bool bSencitive = mParamNameCaseSensitive;
    auto it = std::find_if(mListParamDescriptors.begin(), mListParamDescriptors.end(), [paramName, bSencitive](ParamDescriptor& descrInList)
    {
        if (bSencitive)
            return strcmp(descrInList.paramName, paramName) == 0;
        else
            return strcasecmp(descrInList.paramName, paramName) == 0;
    });

    if (it != mListParamDescriptors.end())
    {
        return it.operator->();
    }

    throw CmdLineParseException(paramName, E_UNKNOWN_KEY);

    return &mDummyDescriptor;
}

CmdLineParser::ParamDescriptor* CmdLineParser::FindParamDescriptor(const char* paramName)
{
    bool bSencitive = mParamNameCaseSensitive;
    auto it = std::find_if(mListParamDescriptors.begin(), mListParamDescriptors.end(), [paramName, bSencitive](ParamDescriptor& descrInList)
    {
        if (bSencitive)
            return strcmp(descrInList.paramName, paramName) == 0;
        else
            return strcasecmp(descrInList.paramName, paramName) == 0;
    });

    if (it != mListParamDescriptors.end())
    {
        return it.operator->();
    }
    else
    {
        return nullptr;
    }
}

void CmdLineParser::AssignDescriptor(const ParamDescriptor& descr)
{
    ParamDescriptor* pFoundDescriptor = FindParamDescriptor(descr.paramName);

    if (pFoundDescriptor)
    {
        *pFoundDescriptor = descr;
    }
    else
    {
        mListParamDescriptors.push_back(descr);
    }
}

void CmdLineParser::Analyze(const char *pszParam, bool bFlag)
{
    if (bFlag)
    {
        AnalyzeParam(pszParam);
    }
    else
    {
        if (mCurrentParam == nullptr)
        {
            return; //Ignore value without flag
        }
        AnalyzeValue(pszParam);
        mCurrentParam = nullptr;
    }
}

void CmdLineParser::OnFinish()
{
    for (auto& descr : mListParamDescriptors)
    {
        if ((descr.constrainRules & CN_MANDATORY) && !descr.isAssign)
        {
            throw CmdLineParseException(descr.paramName, E_NOT_DEFINED);
        }
    }
}

void CmdLineParser::AnalyzeParam(const char* pszParam)
{
    ParamDescriptor* descr = FindParamDescriptor(pszParam);

    if (descr == nullptr)
    {
        if (!mIgnoreUnknownParams)
            throw CmdLineParseException(pszParam, E_UNKNOWN_KEY);
        return;
    }

    if (descr->isAssign && (descr->constrainRules &  CN_NO_DUPLICATE))
    {
        throw CmdLineParseException(pszParam, E_DUPLICATE);
    }

    if (descr->paramType == P_FLAG)
    {
        descr->isAssign = true;
        if (descr->isCallback)
        {
            descr->callbackContainer.funcIsSet(pszParam);
        }
        else
        {
            *((bool*)descr->paramData) = true;
        }
        mCurrentParam = nullptr;
    }
    else
    {
        mCurrentParam = descr;
    }
}

void CmdLineParser::CheckLength(ParamDescriptor* descriptor, const char* value)
{
    if ((descriptor->constrainRules & CN_LENGTH) && strlen(value) > descriptor->length)
    {
        throw CmdLineParseException(descriptor->paramName, E_TOO_LONG);
    }
}

void CmdLineParser::AnalyzeValue(const char* pszValue)
{
    switch (mCurrentParam->paramType)
    {
        case P_CHAR:
        {
            CheckLength(mCurrentParam, pszValue);

            if (mCurrentParam->constrainRules & CN_CHAR_AS_NUMBER)
            {
                radix_t radix = GetNumberRadix(pszValue);
                pszValue += radix.prefix_size;

                long val;
                LongFromString(mCurrentParam->paramName, pszValue, val, radix.radix);

                if ((val < std::numeric_limits<char>::min()) || (val > std::numeric_limits<char>::max()))
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
                }

                if (mCurrentParam->constrainRules & CN_RANGE)
                {
                    if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                    {
                        throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                    }
                }

                CheckValueConstrains(mCurrentParam, pszValue);
                CheckNumericValueConstrains(mCurrentParam, val);

                if (mCurrentParam->isCallback)
                {
                    mCurrentParam->callbackContainer.funcChar(mCurrentParam->paramName, (char)val);
                }
                else
                {
                    *((char*)mCurrentParam->paramData) = (char)val;
                }
            }
            else
            {
                if (strlen(pszValue) > 1)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_TOO_LONG);
                }
                else
                {
                    CheckValueConstrains(mCurrentParam, pszValue);

                    if (mCurrentParam->isCallback)
                    {
                        mCurrentParam->callbackContainer.funcChar(mCurrentParam->paramName, pszValue[0]);
                    }
                    else
                    {
                        *((char*)mCurrentParam->paramData) = pszValue[0];
                    }

                }
            }
            mCurrentParam->isAssign = true;
        }
            break;

        case P_UCHAR:
        {
            CheckLength(mCurrentParam, pszValue);

            if (mCurrentParam->constrainRules & CN_CHAR_AS_NUMBER)
            {
                radix_t radix = GetNumberRadix(pszValue);
                pszValue += radix.prefix_size;

                unsigned long val;
                if ((mCurrentParam->constrainRules & CN_NO_NEGATIVE_UNSIGNED) && IsNegativeNumeric(pszValue))
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_NEGATIVE);
                }

                if (IsNegativeNumeric(pszValue))
                {
                    long negative;
                    LongFromString(mCurrentParam->paramName, pszValue, negative, radix.radix);
                    if (negative < std::numeric_limits<char>::min())
                    {
                        throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
                    }
                    else
                    {
                        val = negative;
                    }
                }
                else
                {
                    ULongFromString(mCurrentParam->paramName, pszValue, val, radix.radix);
                    if (val > std::numeric_limits<unsigned char>::max())
                    {
                        throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
                    }
                }

                if (mCurrentParam->constrainRules & CN_RANGE)
                {
                    if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                    {
                        throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                    }
                }

                CheckValueConstrains(mCurrentParam, pszValue);
                CheckNumericValueConstrains(mCurrentParam, val);

                *((unsigned char*)mCurrentParam->paramData) = (unsigned char)val;
            }
            else
            {

                if (strlen(pszValue) > 1)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_TOO_LONG);
                }
                else
                {
                    CheckValueConstrains(mCurrentParam, pszValue);

                    if (mCurrentParam->isCallback)
                    {
                        mCurrentParam->callbackContainer.funcChar(mCurrentParam->paramName, (unsigned char)pszValue[0]);
                    }
                    else
                    {
                        *((char*)mCurrentParam->paramData) = (unsigned char)pszValue[0];
                    }

                }
            }
            mCurrentParam->isAssign = true;
        }
            break;

        case P_BOOL:
        {
            bool res;

            if (strcasecmp(pszValue, mBoolYesValue) == 0)
            {
                res = true;
            }
            else
            {
                if (strcasecmp(pszValue, mBoolNoValue) == 0)
                {
                    res = false;
                }
                else
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_INVALID_VALUE);
                }
            }

            if (mCurrentParam->isCallback)
            {
                mCurrentParam->callbackContainer.funcBool(mCurrentParam->paramName, true);
            }
            else
            {
                *((bool*)mCurrentParam->paramData) = res;
            }

            mCurrentParam->isAssign = true;
        }
            break;

        case P_SHORT:
        {
            CheckLength(mCurrentParam, pszValue);

            radix_t radix = GetNumberRadix(pszValue);
            pszValue += radix.prefix_size;

            long val;
            LongFromString(mCurrentParam->paramName, pszValue, val, radix.radix);

            if ((val < std::numeric_limits<short>::min()) || (val > std::numeric_limits<short>::max()))
            {
                throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
            }
            if (mCurrentParam->constrainRules & CN_RANGE)
            {
                if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                }
            }

            CheckValueConstrains(mCurrentParam, pszValue);
            CheckNumericValueConstrains(mCurrentParam, val);

            *((short*)mCurrentParam->paramData) = (short)val;

            mCurrentParam->isAssign = true;
        }
            break;

        case P_USHORT:
        {
            CheckLength(mCurrentParam, pszValue);

            radix_t radix = GetNumberRadix(pszValue);
            pszValue += radix.prefix_size;

            unsigned long val;
            if ((mCurrentParam->constrainRules & CN_NO_NEGATIVE_UNSIGNED) && IsNegativeNumeric(pszValue))
            {
                throw CmdLineParseException(mCurrentParam->paramName, E_NEGATIVE);
            }

            if (IsNegativeNumeric(pszValue))
            {
                long negative;
                LongFromString(mCurrentParam->paramName, pszValue, negative, radix.radix);
                if (negative < std::numeric_limits<short>::min())
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
                }
                else
                {
                    val = negative;
                }
            }
            else
            {
                ULongFromString(mCurrentParam->paramName, pszValue, val, radix.radix);
                if (val > std::numeric_limits<unsigned short>::max())
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
                }
            }

            if (mCurrentParam->constrainRules & CN_RANGE)
            {
                if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                }
            }

            CheckValueConstrains(mCurrentParam, pszValue);
            CheckNumericValueConstrains(mCurrentParam, val);

            *((unsigned short*)mCurrentParam->paramData) = (unsigned short)val;

            mCurrentParam->isAssign = true;
        }
            break;

        case P_INT:
        {
            CheckLength(mCurrentParam, pszValue);

            radix_t radix = GetNumberRadix(pszValue);
            pszValue += radix.prefix_size;

            long val;
            LongFromString(mCurrentParam->paramName, pszValue, val, radix.radix);

            if ((val < std::numeric_limits<int>::min()) || (val > std::numeric_limits<int>::max()))
            {
                throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
            }

            if (mCurrentParam->constrainRules & CN_RANGE)
            {
                if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                }
            }

            CheckValueConstrains(mCurrentParam, pszValue);
            CheckNumericValueConstrains(mCurrentParam, val);

            *((int*)mCurrentParam->paramData) = (int)val;

            mCurrentParam->isAssign = true;
        }
            break;

        case P_UINT:
        {
            CheckLength(mCurrentParam, pszValue);

            radix_t radix = GetNumberRadix(pszValue);
            pszValue += radix.prefix_size;

            unsigned long val;
            if ((mCurrentParam->constrainRules & CN_NO_NEGATIVE_UNSIGNED) && IsNegativeNumeric(pszValue))
            {
                throw CmdLineParseException(mCurrentParam->paramName, E_NEGATIVE);
            }

            if (IsNegativeNumeric(pszValue))
            {
                long negative;
                LongFromString(mCurrentParam->paramName, pszValue, negative, radix.radix);
                if (negative < std::numeric_limits<int>::min())
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
                }
                else
                {
                    val = negative;
                }
            }
            else
            {
                ULongFromString(mCurrentParam->paramName, pszValue, val, radix.radix);
                if (val > std::numeric_limits<unsigned int>::max())
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
                }
            }

            if (mCurrentParam->constrainRules & CN_RANGE)
            {
                if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                }
            }

            CheckValueConstrains(mCurrentParam, pszValue);
            CheckNumericValueConstrains(mCurrentParam, val);

            *((unsigned int*)mCurrentParam->paramData) = (unsigned int)val;

            mCurrentParam->isAssign = true;
        }
            break;

        case P_LONG:
        {
            CheckLength(mCurrentParam, pszValue);

            radix_t radix = GetNumberRadix(pszValue);
            pszValue += radix.prefix_size;

            long val;
            LongFromString(mCurrentParam->paramName, pszValue, val, radix.radix);

            if (mCurrentParam->constrainRules & CN_RANGE)
            {
                if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                }
            }

            CheckValueConstrains(mCurrentParam, pszValue);
            CheckNumericValueConstrains(mCurrentParam, val);

            if (mCurrentParam->isCallback)
            {
                mCurrentParam->callbackContainer.funcLong(mCurrentParam->paramName, val);
            }
            else
            {
                *((long*)mCurrentParam->paramData) = val;
            }

            mCurrentParam->isAssign = true;
        }
            break;

        case P_ULONG:
        {
            CheckLength(mCurrentParam, pszValue);

            radix_t radix = GetNumberRadix(pszValue);
            pszValue += radix.prefix_size;

            unsigned long val;
            if ((mCurrentParam->constrainRules & CN_NO_NEGATIVE_UNSIGNED) && pszValue[0] == '-')
            {
                throw CmdLineParseException(mCurrentParam->paramName, E_NEGATIVE);
            }

            ULongFromString(mCurrentParam->paramName, pszValue, val, radix.radix);

            if (mCurrentParam->constrainRules & CN_RANGE)
            {
                if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                }
            }

            CheckValueConstrains(mCurrentParam, pszValue);
            CheckNumericValueConstrains(mCurrentParam, val);

            if (mCurrentParam->isCallback)
            {
                mCurrentParam->callbackContainer.funcULong(mCurrentParam->paramName, val);
            }
            else
            {
                *((unsigned long*)mCurrentParam->paramData) = val;
            }

            mCurrentParam->isAssign = true;
        }
            break;

        case P_FLOAT:
        {
            CheckLength(mCurrentParam, pszValue);

            double val;
            DoubleFromString(mCurrentParam->paramName, pszValue, val);

            if ((val < std::numeric_limits<float>::min()) || (val > std::numeric_limits<float>::max()))
            {
                throw CmdLineParseException(mCurrentParam->paramName, E_OVERLOAD);
            }

            if (mCurrentParam->constrainRules & CN_RANGE)
            {
                if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                }
            }

            CheckValueConstrains(mCurrentParam, pszValue);
            CheckNumericValueConstrains(mCurrentParam, val);

            *((float*)mCurrentParam->paramData) = (float)val;

            mCurrentParam->isAssign = true;
        }
            break;

        case P_DOUBLE:
        {
            CheckLength(mCurrentParam, pszValue);

            double val;
            DoubleFromString(mCurrentParam->paramName, pszValue, val);

            if (mCurrentParam->constrainRules & CN_RANGE)
            {
                if (val < mCurrentParam->minValue || val > mCurrentParam->maxValue)
                {
                    throw CmdLineParseException(mCurrentParam->paramName, E_OUT_OF_RANGE);
                }
            }

            CheckValueConstrains(mCurrentParam, pszValue);
            CheckNumericValueConstrains(mCurrentParam, val);

            if (mCurrentParam->isCallback)
            {
                mCurrentParam->callbackContainer.funcDouble(mCurrentParam->paramName, val);
            }
            else
            {
                *((double*)mCurrentParam->paramData) = val;
            }

            mCurrentParam->isAssign = true;
        }
            break;

        case P_STRING:
        {
            CheckLength(mCurrentParam, pszValue);

            CheckValueConstrains(mCurrentParam, pszValue);

            mCurrentParam->isAssign = true;

            if (mCurrentParam->isCallback)
            {
                mCurrentParam->callbackContainer.funcCharPtr(mCurrentParam->paramName, pszValue);
            }
            else
            {
                *((std::string*)mCurrentParam->paramData) = pszValue;
            }
        }
            break;

        case P_CHAR_BUFFER:
        {
            CheckLength(mCurrentParam, pszValue);

            CheckValueConstrains(mCurrentParam, pszValue);

            strncpy((char*)mCurrentParam->paramData, pszValue, mCurrentParam->length);

            mCurrentParam->isAssign = true;
        }
            break;

        default:
            break;
    }
}

const char* CmdLineParseException::GetParamName() const
{
    return paramName;
}

parser_error_t CmdLineParseException::GetErrorCode() const
{
    return errorCode;
}

CmdLineParseException::CmdLineParseException(const char* szparamName, parser_error_t errorCode)
{
    this->paramName = szparamName;
    this->errorCode = errorCode;
}

size_t CmdLineParseException::GetMaxStringSize(const char* stringArray[], size_t arraySize)
{
    size_t maxSize = 0;
    for (size_t i = 0; i < arraySize; i++)
    {
        size_t length = strlen(stringArray[i]);
        if (length > maxSize)
        {
            maxSize = length;
        }
    }

    return maxSize;
}

const char* CmdLineParseException::what() const noexcept
{
    if (errorString)
    {
        return errorString.get();
    }

    static const char* errorStringArray[] =
            {
                    "unknown key name",     //E_UNKNOWN_KEY
                    "invalid value",        //E_INVALID_VALUE
                    "duplicate value",      //E_DUPLICATE
                    "value is too long",    //E_TOO_LONG,
                    "not numeric value",    //E_NOT_NUMERIC,
                    "value is too large",   //E_OVERLOAD
                    "value is out of range",//E_OUT_OF_RANGE
                    "value is not defined", //E_NOT_DEFINED,
                    "value cannot be negative" //E_NEGATIVE
            };

    static const unsigned int cMaxSizeErrorString = GetMaxStringSize(errorStringArray, std::extent <decltype(errorStringArray)>::value);
    static const unsigned int cReserved = 64;

    size_t sizeBuffer = cMaxSizeErrorString + cReserved + strlen(paramName);
    errorString.reset(new char[sizeBuffer]);

    snprintf(errorString.get(), sizeBuffer, "Parameter '%s': %s", paramName, errorStringArray[errorCode]);

    return errorString.get();
}

