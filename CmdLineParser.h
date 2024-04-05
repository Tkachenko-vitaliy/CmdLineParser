#pragma once

#include <vector>
#include <list>
#include <functional>
#include <array>
#include <memory>

using parser_error_t = enum {
    E_UNKNOWN_KEY = 0,
    E_INVALID_VALUE = 1,
    E_DUPLICATE = 2,
    E_TOO_LONG = 3,
    E_NOT_NUMERIC = 4,
    E_OVERLOAD = 5,
    E_OUT_OF_RANGE = 6,
    E_NOT_DEFINED = 7,
    E_NEGATIVE = 8
};

using param_type_t = enum {
    P_UNDEF = 0,
    P_FLAG,
    P_CHAR,
    P_UCHAR,
    P_BOOL,
    P_SHORT,
    P_USHORT,
    P_INT,
    P_UINT,
    P_LONG,
    P_ULONG,
    P_FLOAT,
    P_DOUBLE,
    P_STRING,
    P_CHAR_BUFFER
};

using constrain_rules_t = enum {
    CN_NONE = 0,
    CN_MANDATORY = 0x01,
    CN_NO_DUPLICATE = 0x02,
    CN_RANGE = 0x04,
    CN_LENGTH = 0x08,
    CN_LIST_VALUE = 0x10,
    CN_CASE_SENSITIVE = 0x20,
    CN_CHAR_AS_NUMBER = 0x40,
    CN_NO_NEGATIVE_UNSIGNED = 0x80
} ;

using rule_mask_t = unsigned int ;

class CmdLineParser
{
public:
    CmdLineParser();
    CmdLineParser(std::initializer_list<const char *> listParamFlag);
    virtual ~CmdLineParser();

    void Parse(int argc, const char *const argv[], int argStart = 0);
    void Parse(const char *cmdLineString);

    void AssignParamFlags(std::initializer_list<const char *> listParamFlag);
    std::pair<const char **, size_t> GetParamFlags();
    void AddParamFlag(const char *paramFlag);
    void DeleteParamFlag(const char *paramFlag);
    bool IsFlagExist(const char *paramFlag) const;
    void ClearParamFlags();

    void BindParam(const char *paramName, bool &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, char &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, unsigned char &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, short &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, unsigned short &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, int &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, unsigned int &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, long &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, unsigned long &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, float &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, double &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, std::string &paramValue, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, char *paramValue, unsigned long length, rule_mask_t ruleMask = CN_NONE);

    void BindParamIsSet(const char *paramName, bool &isParamSet);

    using callback_string_t = std::function<void(const char *paramName, const char *paramValue)> ;
    using callback_bool_t = std::function<void(const char *paramName, bool paramValue)> ;
    using callback_char_t = std::function<void(const char *paramName, char paramValue)> ;
    using callback_long_t = std::function<void(const char *paramName, long paramValue)> ;
    using callback_ulong_t = std::function<void(const char *paramName, unsigned long paramValue)> ;
    using callback_double_t = std::function<void(const char *paramName, double paramValue)> ;
    using callback_param_set_t = std::function<void(const char *paramName)> ;

    void BindParam(const char *paramName, callback_string_t callbackFunction, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, callback_bool_t callbackFunction, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, callback_char_t callbackFunction, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, callback_long_t callbackFunction, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, callback_ulong_t callbackFunction, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, callback_double_t callbackFunction, rule_mask_t ruleMask = CN_NONE);
    void BindParam(const char *paramName, callback_param_set_t callbackFunction, rule_mask_t ruleMask = CN_NONE);

    bool IsParamExist(const char *paramName) const;
    bool DeleteParam(const char *paramName);
    void ClearParams();

    void SetIgnoreUnknownParams(bool bIgnore);
    bool GetIgnoreUnknownParams() const;
    void SetParamNameCaseSensitive(bool bSensitive);
    bool GetParamNameCaseSensitive() const;
    void SetBoolYesValue(const char *szValue);
    void SetBoolNoValue(const char *szValue);
    const char *GetBoolYesValue() const;
    const char *GetBoolNoValue() const;

    using numeric_radix_t = enum { BIN = 0, OCT = 1, HEX = 2 } ;

    void SetRadixPrefix(numeric_radix_t radix_type, const char *szPrefix);
    const char *GetRadixPrefix(numeric_radix_t radix_type) const;

    void AssignConstrains(const char *paramName, rule_mask_t rulesMask);
    void AddConstrains(const char *paramName, rule_mask_t rulesMask);
    void DeleteConstrains(const char *paramName, rule_mask_t rulesMask);
    void ClearConstrains();
    void SetRangeConstrain(const char *paramName, double minValue, double maxValue);
    void SetLengthConstrain(const char *paramName, unsigned long length);
    void AssignValueConstrains(const char *paramName, std::initializer_list<const char *> listValues);
    void AssignValueConstrains(const char *paramName, std::initializer_list<double> listValues);
    void AddValueConstrain(const char *paramName, const char *value);
    void AddValueConstrain(const char *paramName, double value);
    void DeleteValueConstrain(const char *paramName, const char *value);
    void DeleteValueConstrain(const char *paramName, double value);
    std::pair<const char **, size_t> GetValueConstrains(const char *paramName) const;
    std::pair<double *, size_t> GetNumericValueConstrains(const char *paramName) const;
    void ClearValueConstrains(const char *paramName);

    bool IsValueAssigned(const char *paramName) const;
    void ResetValueAssigned(const char *paramName);
    void ClearValueAssigned();

protected:
    virtual void Analyze(const char *pszParam, bool bFlag);

private:
    struct CallbackContainer
    {
        callback_string_t funcCharPtr;
        callback_bool_t funcBool;
        callback_char_t funcChar;
        callback_long_t funcLong;
        callback_ulong_t funcULong;
        callback_double_t funcDouble;
        callback_param_set_t funcIsSet;
    };

    struct ParamDescriptor
    {
        const char *paramName = nullptr;
        param_type_t paramType = P_UNDEF;
        void *paramData = nullptr;
        bool isAssign = false;
        bool isCallback = false;
        rule_mask_t constrainRules = CN_NONE;
        double minValue = 0.0;
        double maxValue = 0.0;
        unsigned long length = 0;
        std::vector<const char *> valueConstrains;
        std::vector<double> numericValueConstrains;
        CallbackContainer callbackContainer;
    };

    struct radix_t
    {
        int radix;
        int prefix_size;
    };

    ParamDescriptor *mCurrentParam = nullptr;
    ParamDescriptor mDummyDescriptor;

    static const int count_numeric_radix_prefix = 3;
    std::vector<ParamDescriptor> mListParamDescriptors;
    std::vector<const char *> mListFlag;
    std::array<const char *, count_numeric_radix_prefix> mListRadixPrefix;
    std::array<int, count_numeric_radix_prefix> mListRadixBase;
    const char *mBoolYesValue;
    const char *mBoolNoValue;

    bool mIgnoreUnknownParams;
    bool mParamNameCaseSensitive;

    void LongFromString(const char *paramName, const char *numberString, long &number, int radix);
    void ULongFromString(const char *paramName, const char *numberString, unsigned long &number, int radix);
    void DoubleFromString(const char *paramName, const char *numberString, double &number);

    ParamDescriptor *GetParamDescriptor(const char *paramName);
    ParamDescriptor *FindParamDescriptor(const char *paramName);
    void AssignDescriptor(const ParamDescriptor &descr);

    void CheckLength(ParamDescriptor *descriptor, const char *value);
    void CheckValueConstrains(ParamDescriptor *descriptor, const char *value);
    void CheckNumericValueConstrains(ParamDescriptor *descriptor, double value);

    void ParseItem(const char *stringItem);
    void Reset();
    void OnFinish();
    void AnalyzeParam(const char *pszParam);
    void AnalyzeValue(const char *pszValue);

    radix_t GetNumberRadix(const char *szValue);
};

class CmdLineParseException : public std::exception
{
public:
    CmdLineParseException(const char *szparamName, parser_error_t errorCode);
    const char *GetParamName() const;
    parser_error_t GetErrorCode() const;
    const char *what() const noexcept override;

private:
    const char *paramName;
    parser_error_t errorCode;
    mutable std::shared_ptr<char> errorString;

    static size_t GetMaxStringSize(const char *stringArray[], size_t arraySize);
};

