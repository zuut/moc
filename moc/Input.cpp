/*
 * See Readme, Notice and License files.
 */
#include "Input.h"
#include "MOC.h"
#include "MOCInternal.h"
#include "MOCTokens.h"
#include <ctype.h>

ufDEBUG_FILE
#define NUM_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))
static int gLastCode = 1000;
static char *ConvertToString(int l);

class ModuleIterator : public InputBlockIterator {
public:
    ModuleIterator(ufList(KeyValuePair) &kvList) :
      InputBlockIterator(kvList) { DataChanged(); }
    virtual ~ModuleIterator() {}
    virtual void DataChanged() {
        ufPtr(KeyValuePair) kv = (ufPtr(KeyValuePair) &)fData;
        gModule = kv->fKey;
        gModuleFullFilename = kv->fValue;
    };
};

struct BuiltinType {
    const char *fName;
    const char *fTypeString;
    const char *fTypeCodeString;
    int fTypeCode;
};

static BuiltinType gBuiltinTypes[] = {
    {"", "", "kInvalidTypeCode", kInvalidTypeCode},
    {"STRING", "String", "kStringTypeCode", kStringTypeCode},
    {"INTEGER", "int", "kIntegerTypeCode", kIntegerTypeCode},
    {"BOOLEAN", "boolean", "kBooleanTypeCode", kBooleanTypeCode},
    {"REAL", "double", "kRealTypeCode", kRealTypeCode},
    {0}};

void LoadBuiltinTypes() {
    for (int i = 1; gBuiltinTypes[i].fTypeString; i++) {
        Type::New(gBuiltinTypes[i].fTypeCode);
    }
}

void GenerateTypeCodes() { Class::ValidateAll(); }

UserCode *CreateUserCode(char *s) {
    UserCode *uc = new UserCode;
    uc->fCodeLen = strlen(s);
    uc->fCode = s;
    uc->fFilename = ufStrDup(gFilename);
    uc->fLineno = lineno;

    return uc;
}

inline char toChar(char h, char l)
{
    uint8_t ret;
    if (h == EOF || l == EOF) {
        return EOF;
    }

    if (h - '0' < 10)
        ret = h - '0';
    else if (h - 'A' < 6)
        ret = h - 'A' + 0x0A;
    else if (h - 'a' < 6)
        ret = h - 'a' + 0x0A;

    ret = ret << 4;

    if (l - '0' < 10)
        ret |= l - '0';
    else if (l - 'A' < 6)
        ret |= l - 'A' + 0x0A;
    else if (l - 'a' < 6)
        ret |= l - 'a' + 0x0A;
    return  char(ret);
}

const char *WriteCodeFrag(UserCode *uc, char *buf, int len) {
    buf[0] = '\0';
    buf[--len] = '\0';
    len--;
    while (len >= 0) {
        buf[len] = uc->fCode[len];
        len--;
    }
    return buf;
}

UserCode *newUserCode(const char *message) { return newUserCode2(message, 0); }

UserCode *newUserCode2(const char *message, const char *message2) {
    UserCode *uc = new UserCode;
    char *m = ufStrCat(message, message2);
    uc->fCodeLen = strlen(m);
    uc->fCode = m;
    uc->fFilename = ufStrDup(gFilename);
    uc->fLineno = lineno;
    return uc;
}

KeyValue *ReadKeyValue(KeyValue *resultBuf, const char *inputBuf,
                       FILE *in) {
    const int MaxKeyLength =
        sizeof(KeyValue::fKey) / sizeof(KeyValue::fKey[0]) - 1;
    const char *fnm = gFilename;
    resultBuf->fUserCode = ReadCodeString(in);
    if (inputBuf[0] == '[' && inputBuf[1] == '[') {
        inputBuf += 2;
    }
    while (isspace(*inputBuf))
        inputBuf++;
    strncpy(resultBuf->fKey, inputBuf, MaxKeyLength);
    resultBuf->fKey[MaxKeyLength] = '\0';
    // Now find the closing ']]'
    char c;
    int isFirst = 0;
    while (isspace(c = getc(in))) {
        isFirst = 0;
        if (c == '\n') {
            lineno++;
            isFirst = 1;
        }
    }
    if (c == ']') {
        c = getc(in);
    }
    if (c != ']') {
        char buf[1024];
        sprintf(buf,
                "expected a '{' following the code "
                "keyword instead found a '%c'",
                c);
        mocError2(buf);
        ungetc(c, in);
        return 0;
    }
    return resultBuf;
}

UserCode *ReadCodeString(FILE *in) {
    const char *fnm = gFilename;
    char c;
    int isFirst = 0;
    if (gLastCharRead != '{') {
        while (isspace(c = getc(in))) {
            isFirst = 0;
            if (c == '\n') {
                lineno++;
                isFirst = 1;
            }
        }
        if (c != '{') {
            char buf[1024];
            sprintf(buf,
                    "expected a '{' following the code "
                    "keyword instead found a '%c'",
                    c);
            mocError2(buf);
            return 0;
        }
    }

    int line = lineno;

    ufTmpBuf tmp(50000);
    int level = 1;
    int isDoubleQuoted = 0;
    int isComment = 0;
    int i = 0;
    char c2;

    for (c = getc(in); c != EOF; c = getc(in)) {
        if (tmp.GetSize() < i + 250) {
            tmp.Reallocate(i + 50000);
        }

        if (isFirst && c == '!') {
            isFirst = 0;
            continue;
        }
        if (c == '\\' || c == '*' || c == '/') {
            char c3 = getc(in);
            if (c3 != '!') {
                ungetc(c3, in);
            }
        }

        tmp.fBuf[i++] = c;

        if (c == '\n') {
            lineno++;
            isFirst = 1;
            continue;
        } else if (c == '#' && isFirst) {
            ufTmpBuf tmp2(2000);
            if (FGets(tmp2, in)) {
                static char f[1024];
                int lno;
                if (sscanf(tmp2.fBuf, "%d \"%[^\"]", &lno, f) == 2) {
                    sprintf(tmp2.fBuf,
                            "line %d \"%s\""
                            "\n",
                            lno, f);
                    int len = strlen(tmp2.fBuf);
                    if (tmp.GetSize() < i + len) {
                        tmp.Reallocate(i + len + 50);
                    }
                    tmp.fBuf[i] = '\0';
                    strcat(&tmp.fBuf[i], tmp2.fBuf);
                    i += len;
                } else {
                    i--;
                }
                isFirst = 1;
                continue;
            }
        } else if (c == '{' && !isDoubleQuoted && !isComment) {
            level++;
        } else if (c == '}' && !isDoubleQuoted && !isComment) {
            if (--level == 0) {
                i--;
                break;
            }
        } else if (c == '\"') //" emacs hilight bug
        {
            isDoubleQuoted = isDoubleQuoted ? 0 : 1;
        } else if (c == '/' && (c = getc(in)) != EOF) {
            if (c != '*') {
                ungetc(c, in);
            } else {
                isComment = 1;
                tmp.fBuf[i++] = c;
            }
        } else if (c == '*' && (c = getc(in)) != EOF) {
            if (c != '/') {
                ungetc(c, in);
            } else {
                isComment = 0;
                tmp.fBuf[i++] = c;
            }
        } else if (c == '\\' && (c = getc(in)) != EOF) {
            if ( c == 'x') {
                char c2 = getc(in);
                char c3 = getc(in);
                c = toChar(c2, c3);
            } else if (c == '\n') {
                lineno++;
                isFirst = 1;
            }
            ufASSERT( i > 0 );
            tmp.fBuf[i - 1] = c;
        } else if (c == '\'' && !isDoubleQuoted && !isComment &&
                   (c = getc(in)) != EOF && (c2 = getc(in)) != EOF) {
            tmp.fBuf[i++] = c;
            tmp.fBuf[i++] = c2;
        }

        isFirst = 0;
    }

    tmp.fBuf[i] = '\0';

    UserCode *uc = new UserCode;
    uc->fCodeLen = strlen(tmp.fBuf);
    uc->fCode = ufStrDup(tmp.fBuf);
    uc->fFilename = ufStrDup(fnm);
    uc->fLineno = line;

    return uc;
}

int mocIsModule(const char *module) {
    ufPtr(KeyValuePair) theModule;

    return GlobalScope::fgModuleCatalog != 0 &&
           GlobalScope::fgModuleCatalog.Find(module, theModule);
}

void DeclareModule(const char* module, const char* fullPath) {
    ufPtr(KeyValuePair) theModule = KeyValuePair::New();

    if (mocIsModule(module)) {
        MOCERROR((errmsg,
                  "You already have a module with the name \"%s\"",
                  module));
        return;
    }

    theModule->fKey = ufStrDup(module);
    theModule->fValue = ufStrDup(fullPath);

    GlobalScope::fgModules.InsertLast(theModule);
    GlobalScope::fgConstantCatalog.Insert(theModule->fKey, theModule);
}

void DeclareConstant(const char *name, int num) {
    ufPtr(KeyValuePair) theConst = KeyValuePair::New();

    if (mocIsConstant(name) || mocIsTypeDefined(name)) {
        MOCERROR((errmsg,
                  "You already have a constant or "
                  "type with the name \"%s\"",
                  name));
        return;
    }

    ufTmpBuf tmp(40);
    sprintf(tmp.fBuf, "%d", num);
    theConst->fKey = ufStrDup(name);
    theConst->fValue = ufStrDup(tmp.fBuf);

    GlobalScope::fgConstants.InsertLast(theConst);
    GlobalScope::fgConstantCatalog.Insert(theConst->fKey, theConst);
}

void DeclareVariable(const char *name, const char *value) {
    ufPtr(KeyValuePair) theVar = KeyValuePair::New();

    theVar->fKey = ufStrDup(name);
    int len = 0;
    // Strip double quotes
    if (*value == '"') {
        value++;
        len = strlen(value);
        if (len > 0 && value[len - 1] == '"') {
            *((char *)value) = '\0';
        } else {
            len = 0;
        }
    }
    theVar->fValue = ufStrDup(value);
    if (len > 0) {
        *((char *)value) = '"';
    }

    GlobalScope::fgVariables.InsertLast(theVar);
    GlobalScope::fgVariableCatalog.Insert(theVar->fKey, theVar);
}

int mocIsConstant(const char *name) {
    ufPtr(KeyValuePair) theConst;

    return GlobalScope::fgConstantCatalog != 0 &&
           GlobalScope::fgConstantCatalog.Find(name, theConst);
}

extern int mocGetConstant(const char *name) {
    ufPtr(KeyValuePair) theConst;

    if (GlobalScope::fgConstantCatalog == 0 ||
        !GlobalScope::fgConstantCatalog.Find(name, theConst)) {
        return 0;
    }

    int i = 0;
    sscanf(theConst->fValue, "%d", &i);
    return i;
}

static ufPtr(Type) theCurrentType;

void DeclareType(const char *name) { theCurrentType = Type::New(name); }

void SetTypeClassName(const char *name) {
    theCurrentType->fTypeString = ufStrDup(name);
}

void AddTypeEnumEntry(const char *name) { theCurrentType->AddEnumEntry(name); }

void AddTypeEnumEntryPair(const char *name, int e) {
    theCurrentType->AddEnumEntryPair(name, e);
}

void AddStructField(const char *typeName, const char *symbolName) {
    theCurrentType->AddField(typeName, symbolName);
}

void SetTypeHeader(const char *headerName) {
    theCurrentType->fHeader = ufStrDup(headerName);
}

void SetTypeKeyValue(const char *key, UserCode *value) {
    theCurrentType->AddKeyValue(key, value);
}

void SetTypeDef(const char *realType) {
    theCurrentType->fIsTypeDef = 1;
    theCurrentType->fTypeDefString = ufStrDup(realType);
}

int mocIsTypeDefined(const char *n) {
    ufPtr(Type) theType;

    return Type::Find(n, theType);
}

int mocIsClassDefined(const char *n) {
    ufPtr(Class) theClass;

    return Class::Find(n, theClass);
}

void SetTopLevelCodeBlock(const char* nameOfCodeBlock,
                          ufPtr(KeyValuePair)& codeBlockPtr,
                          UserCode *uc) {
    if (uc) {
        ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();
        theKeyValue->fKey = ufStrDup(nameOfCodeBlock);
        theKeyValue->fValue = uc->fCode;
        theKeyValue->fFilename = uc->fFilename;
        theKeyValue->fLineno = uc->fLineno;
        codeBlockPtr = theKeyValue;
    }
}

void SetHeaderCode(UserCode *uc) {
    SetTopLevelCodeBlock("HEADER_CODE", GlobalScope::fgHeaderCode, uc);
}

void SetHeaderEndCode(UserCode *uc) {
    SetTopLevelCodeBlock("HEADER_END_CODE", GlobalScope::fgHeaderEndCode, uc);
}

void SetSourceCode(UserCode *uc) {
    SetTopLevelCodeBlock("SOURCE_CODE", GlobalScope::fgSourceCode, uc);
}

/*
 * Class stuff
 */
static ufPtr(Class) theCurrentClass;
static ufPtr(Field) theCurrentField;
static ufPtr(Operation) theCurrentOp;
static ufPtr(Arg) theCurrentOpArg;

void WarnIfNotASubclassOf(const char *n, const char *p) {
    if (strcmp(n, "Start") == 0) {
        return;
    }

    ufPtr(Class) theClass;
    if (!Class::Find(n, theClass)) {
        char buf[200];
        const int sz = NUM_ELEMENTS(buf) - 1;
        strncpy(buf, "\"", sz);
        strncat(buf, n, sz);
        strncat(buf,
                "\" is not even a class. "
                "It must be a subclass of ",
                sz);
        strncat(buf, p, sz);
        mocWarning(buf);
        return;
    }

    if (!theClass->HasParent(p)) {
        char buf[200];
        const int sz = NUM_ELEMENTS(buf) - 1;
        strncpy(buf, "\"", sz);
        strncat(buf, n, sz);
        strncat(buf,
                "\" must be a "
                "subclass of ",
                sz);
        strncat(buf, p, sz);
        mocWarning(buf);
    }
}

void DeclareClass(const char *name) { theCurrentClass = Class::New(name); }

void SetInterface() { theCurrentClass->fIsInterface = 1; }

void SetClassKeyValue(const char *key, UserCode *value) {
    theCurrentClass->AddKeyValue(key, value);
}

void AddClassParent(const char *name) { theCurrentClass->AddParent(name); }

void AddField(const char *typeName, const char *symbolName) {
    theCurrentClass->AddField(typeName, symbolName);
}

void AddFieldArrayIndex(int l) { theCurrentField->fArrayLength = l; }

void DeclareOperation(const char *name) { theCurrentClass->AddOperation(name); }

void DeclareOperationImp(const char *className, const char *opName) {
    ufPtr(Class) theClass;
    if (!Class::Find(className, theClass)) {
        ufTmpBuf tmp(100 + 2 * strlen(className) + strlen(opName));
        sprintf(tmp.fBuf,
                "Cannot find class with name \""
                "%s\" in declaration %s:: on %s \n",
                className, className, opName);
        mocError(tmp.fBuf);
        return;
    }

    ufPtr(Operation) theOp;
    if (!theClass->FindOp(opName, theOp)) {
        ufTmpBuf tmp(100 + strlen(className) + 2 * strlen(opName));
        sprintf(tmp.fBuf,
                "Cannot find operation with "
                "name \"%s\" in declaration "
                "%s:: on %s \n",
                opName, className, opName);
        mocError(tmp.fBuf);
        return;
    }

    theCurrentClass = theClass;
    theCurrentOp = theOp;
}

void SetOpKeyValue(const char *key, UserCode *value) {
    theCurrentOp->AddKeyValue(key, value);
}

void SetOpArgKeyValue(const char *key, UserCode *value) {
    theCurrentOpArg->AddKeyValue(key, value);
}

void SetOpCodeString(UserCode *uc) {
    if (uc) {
        theCurrentOp->fUserCode = uc->fCode;
        theCurrentOp->fUserCodeFilename = uc->fFilename;
        theCurrentOp->fUserCodeLineno = uc->fLineno;
    }
}

void SetFieldKeyValue(const char *key, UserCode *value) {
    theCurrentField->AddKeyValue(key, value);
}

void AddInArgument(const char *typeName, const char *symbolName) {
    theCurrentOp->AddInArgument(typeName, symbolName);
}

void AddOutArgument(const char *typeName, const char *symbolName) {
    theCurrentOp->AddOutArgument(typeName, symbolName);
}

void AddErrorArgument(const char *typeName, const char *symbolName) {
    theCurrentOp->AddErrorArgument(typeName, symbolName);
}

/*
 * InputBlock implementation
 */
implement(ufPtr, InputBlock);

InputBlock::InputBlock(const char *name)
    : fFilename(ufStrDup(gFilename)), fLineno(lineno), fLinenoBuf(0),
      fName(ufStrDup(name)), fUserCode(0), fUserCodeFilename(0),
      fUserCodeLineno(1), fUserCodeLinenoBuf(0) {}

InputBlock::InputBlock(const InputBlock &o){ufASSERT(0)}

InputBlock::~InputBlock() {
    delete[] fName;
    delete[] fFilename;
    delete[] fLinenoBuf;
    delete[] fUserCode;
    delete[] fUserCodeFilename;
    delete[] fUserCodeLinenoBuf;
}

const char *InputBlock::GetClassName() { return "InputBlock"; }

const char *InputBlock::Eval(const char *key) {
    bool wasFound;
    const char* result = LocalEval(key, wasFound);
    // failed to find it in InputBlock::eval(), look in global scope
    return wasFound ? result : GlobalScope::fgSelf->Eval(key);
}

const char *InputBlock::LocalEval(const char *key, bool& wasFound) {
    wasFound = 1;
    if (strcmp(key, "Name") == 0) {
        return fName;
    }
    if (strcmp(key, "Filename") == 0) {
        return fFilename;
    }
    if (strcmp(key, "Lineno") == 0) {
        if (!fLinenoBuf) {
            fLinenoBuf = ConvertToString(fLineno);
        }
        return fLinenoBuf;
    }
    if (strcmp(key, "TypeCode") == 0) {
        // hardcode this to be 0 (doesn't exist) and let
        // subclasses override the value if they need to do so.
        return "0";
    }
    if (strcmp(key, "TypeString") == 0) {
        // hardcode this to be "N/A" and let
        // subclasses override the value if they need to do so.
        return "N/A";
    }
    if (strcmp(key, "IsType") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        return "0";
    }
    if (strcmp(key, "IsClass") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        return "0";
    }
    if (strcmp(key, "IsEnum") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        return "0";
    }
    if (strcmp(key, "IsAggregate") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        return "0";
    }
    if (strcmp(key, "IsTypeDef") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        return "0";
    }
    if (strcmp(key, "IsArg") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        return "0";
    }
    if (strcmp(key, "IsMethod") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        return "0";
    }
    if (strcmp(key, "code") == 0) {
        return fUserCode ? fUserCode : "";
    }
    if (strcmp(key, "code.Filename") == 0) {
        return fUserCodeFilename ? fUserCodeFilename : fFilename;
    }
    if (strcmp(key, "code.Lineno") == 0) {
        if (!fUserCodeLinenoBuf) {
            fUserCodeLinenoBuf = ConvertToString(fUserCodeLineno);
        }
        return fUserCodeLinenoBuf;
    }

    int keyLen = strlen(key);
    for (ufListIter(KeyValuePair) i = fKeyValues; i; i++) {
        if (strcmp(key, i.fData->fKey) == 0) {
            return i.fData->fValue;
        }
        int newLen = strlen(i.fData->fKey);

        if (newLen <= keyLen && key[newLen] == '.' &&
            strncmp(key, i.fData->fKey, newLen) == 0) {
            return i.fData->Eval(&key[newLen + 1]);
        }
    }
    wasFound = 0;
    return 0;
}

int InputBlock::HasList(const char *key) {
    if (strcmp(key, "annotations") == 0) {
        return 1;
    }
    if (strcmp(key, "ALL_CLASSES") == 0) {
        return 1;
    }
    if (strcmp(key, "ALL_CONSTANTS") == 0) {
        return 1;
    }
    if (strcmp(key, "ALL_TYPES") == 0) {
        return 1;
    }
    if (strcmp(key, "key-value-list") == 0) {
        return 1;
    }
    if (strcmp(key, "ALL_MODULES") == 0) {
        return 1;
    }
    return 0;
}

int InputBlock::HasNonEmptyList(const char *key) {
    if (strcmp(key, "annotations") == 0) {
        return fKeyValues;
    }
    if (strcmp(key, "ALL_CLASSES") == 0) {
        return Class::fgClasses != 0;
    }
    if (strcmp(key, "ALL_CONSTANTS") == 0) {
        return GlobalScope::fgConstants != 0;
    }
    if (strcmp(key, "ALL_TYPES") == 0) {
        return Type::fgTypes != 0;
    }
    if (strcmp(key, "key-value-list") == 0) {
        return fKeyValues;
    }
    if (strcmp(key, "ALL_MODULES") == 0) {
        return GlobalScope::fgModules != 0;
    }
    return 0;
}

int InputBlock::HasParent(const char *key) {
    if (fName != 0 && strcmp(key, fName) == 0) {
        return 1;
    }

    return 0;
}

int InputBlock::IsFromCurrentModule() {
    return fFilename ? strcmp(fFilename, gModuleFullFilename) == 0 : 0;
}

void InputBlock::AddKeyValue(const char *key, UserCode *value) {
    if (value != 0) {
        ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();

        theKeyValue->fKey = ufStrDup(key);
        theKeyValue->fValue = value->fCode;
        theKeyValue->fFilename = value->fFilename;
        theKeyValue->fLineno = value->fLineno;
        fKeyValues.InsertLast(theKeyValue);
    } else {
        ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();
        theKeyValue->fKey = ufStrDup(key);
        fKeyValues.InsertLast(theKeyValue);

        MOCERROR((errmsg, "Can't find the value for the \"%s\" ", key));
    }
}

void InputBlock::SetKeyValue(const char *key, const char *value) {
    ufPtr(KeyValuePair) theKeyValue;
    if (!FindKeyValue(key, theKeyValue)) {
        theKeyValue = KeyValuePair::New();
        fKeyValues.InsertLast(theKeyValue);
    }

    theKeyValue->fKey = ufStrDup(key);
    theKeyValue->fValue = ufStrDup(value);
    theKeyValue->fFilename = ufStrDup("");
    theKeyValue->fLineno = 0;
}

int InputBlock::FindKeyValue(const char *key, ufPtr(KeyValuePair) & res) {
    int keyLen = strlen(key);
    for (ufListIter(KeyValuePair) i = fKeyValues; i; i++) {
        if (strcmp(key, i.fData->fKey) == 0) {
            res = i.fData;
            return 1;
        }
        int newLen = strlen(i.fData->fKey);

        if (newLen <= keyLen && key[newLen] == '.' &&
            strncmp(key, i.fData->fKey, newLen) == 0) {
            return i.fData->FindKeyValue(&key[newLen + 1], res);
        }
    }

    return 0;
}

InputBlockIterator *InputBlock::CreateIterator(const char *key) {
    if (strcmp(key, "annotations") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fKeyValues);
        return i;
    } else if (strcmp(key, "ALL_CLASSES") == 0) {
        InputBlockIterator *i = Class::fgClasses
                                    ? new InputBlockIterator(Class::fgClasses)
                                    : new InputBlockIterator;
        return i;
    } else if (strcmp(key, "ALL_CONSTANTS") == 0) {
        InputBlockIterator *i =
            GlobalScope::fgConstants
                ? new InputBlockIterator(GlobalScope::fgConstants)
                : new InputBlockIterator;
        return i;
    } else if (strcmp(key, "ALL_TYPES") == 0) {
        InputBlockIterator *i = Type::fgTypes
                                    ? new InputBlockIterator(Type::fgTypes)
                                    : new InputBlockIterator;
        return i;
    } else if (strcmp(key, "key-value-list") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fKeyValues);
        return i;
    } else if (strcmp(key, "ALL_MODULES") == 0) {
        InputBlockIterator *i =
            GlobalScope::fgModules
                ? new ModuleIterator(GlobalScope::fgModules)
                : new InputBlockIterator;
        return i;
    }

    return 0;
}

InputBlock &InputBlock::operator=(const InputBlock &o) {
    if (this == &o) {
        return *this;
    }

    delete[] fName;
    fName = ufStrDup(o.fName);

    delete[] fFilename;
    fFilename = ufStrDup(o.fFilename);

    fLineno = o.fLineno;

    return *this;
}

/*
 * KeyValuePair implementation
 */

implement(ufPtr, KeyValuePair);
implement(ufList, KeyValuePair);
implement(ufStrHash, KeyValuePair);

/*
 * Type implementation
 */
ufPtr(KeyValuePair) KeyValuePair::New() {
    return ufPtr(KeyValuePair)(new KeyValuePair);
}

const char *KeyValuePair::GetClassName() { return "KeyValue"; }

KeyValuePair::KeyValuePair() : InputBlock("KeyValue"), fKey(0), fValue(0) {}

KeyValuePair::KeyValuePair(const KeyValuePair &o)
    : InputBlock(o){ufASSERT(0)}

      KeyValuePair::~KeyValuePair() {
    delete[] fKey;
    delete[] fValue;
}

void KeyValuePair::PrintDebug() {
    printf("         (key=\"%s\",value=\"%s\")\n", fKey, fValue);
}

const char *KeyValuePair::Eval(const char *key) {
    if (key && fKey && strcmp(key, fKey) == 0) {
        return fValue;
    }
    if (key && fKey && strcmp(key, "Value") == 0) {
        return fValue;
    }
    if (key && fKey && strcmp(key, "Key") == 0) {
        return fKey;
    }
    if (key && fKey && strcmp(key, "Name") == 0) {
        return fKey;
    }
    // If caller request Type.XXX , find the Type
    // which has the Name equal to the value of this KeyValuePair
    if (key && strncmp(key, "Type.", 5) == 0) {
        ufPtr(Type) theType;

        if (Type::Find(fValue, theType)) {
            return theType->Eval(key + 5);
        }

        ufPtr(Class) theClass;
        if (Class::Find(fValue, theClass)) {
            return theClass->Eval(key + 5);
        }
        return 0;
    }
    // If caller request Class.XXX , find the Class
    // which has the Name equal to the value of this KeyValuePair
    if (key && strncmp(key, "Class.", 6) == 0) {
        ufPtr(Class) theClass;

        if (!Class::Find(fValue, theClass)) {
            return 0;
        }
        return theClass->Eval(key + 6);
    }
    // failed to find it in KeyValuePair::Eval(), look elsewhere
    return InputBlock::Eval(key);
}

implement(ufPtr, Type);
implement(ufList, Type);
implement(ufStrHash, Type);

ufStrHash(Type) Type::fgTypeCatalog;
ufList(Type) Type::fgTypes;

ufPtr(Type) Type::New(const char *n) { return (new Type(n))->fSelf; }

ufPtr(Type) Type::New(int btc) { return (new Type(btc))->fSelf; }

const char *Type::GetClassName() { return "Type"; }

Type::Type(const char *name)
    : InputBlock(name), fTypeDefString(0), fTypeString(0), fHeader(0),
      fTypeCode(-1), fTypeCodeString(0), fIsEnum(0), fIsTypeDef(0),
      fIsAggregate(0), fEnumEntries(),
      fLastEnumValue(-1), /* so that start value is 0 */
      fSelf(this, kSelfPtrShared) {
    Register(fSelf);
}

Type::Type(const Type &o)
    : InputBlock(o){ufASSERT(0)}

      Type::Type(int btc)
    : InputBlock(gBuiltinTypes[btc].fName), fTypeDefString(0), fTypeString(0),
      fHeader(0), fTypeCode(-1), fTypeCodeString(0), fIsEnum(0), fIsTypeDef(0),
      fIsAggregate(0), fEnumEntries(), fLastEnumValue(-1),
      fSelf(this, kSelfPtrShared) {
    fFilename = ufStrDup("-builtin-");
    fLineno = 0;

    ufASSERT(gBuiltinTypes[btc].fTypeCode == btc);

    fTypeString = ufStrDup(gBuiltinTypes[btc].fTypeString);
    fTypeCodeString = ufStrDup(gBuiltinTypes[btc].fTypeCodeString);
    fTypeCode = gBuiltinTypes[btc].fTypeCode;

    Register(fSelf);
}

Type::~Type() {
    delete[] fTypeString;
    delete[] fTypeCodeString;
}

const char *Type::Eval(const char *key) {
    if (key == 0)
        return 0;

    if (strcmp(key, "TypeCode") == 0) {
        return fTypeCodeString;
    }
    if (strcmp(key, "TypeString") == 0) {
        return fTypeString ? fTypeString : fName;
    }
    if (strcmp(key, "IsType") == 0) {
        return "1";
    }
    if (strcmp(key, "Header") == 0) {
        if (!fHeader) {
            ufTmpBuf tmp(strlen(gModule) + 20);
            sprintf(tmp.fBuf, "Gen/%s.h", gModule);
            fHeader = ufStrDup(tmp.fBuf);
        }
        return fHeader;
    }
    if (strcmp(key, "IsEnum") == 0) {
        return fIsEnum ? "1" : "0";
    }
    if (strcmp(key, "IsAggregate") == 0) {
        return fIsAggregate ? "1" : "0";
    }
    if (strcmp(key, "IsTypeDef") == 0) {
        return fIsTypeDef ? "1" : "0";
    }
    if (strcmp(key, "TypeDefString") == 0) {
        return fTypeDefString;
    }
    if (strncmp(key, "TypeDefString.", 14) == 0) {
        ufPtr(Type) theType;

        if (!Type::Find(fTypeDefString, theType)) {
            ufPtr(Class) theClass;
            if (Class::Find(fTypeDefString, theClass)) {
                return theClass->Eval(key + 14);
            }
            return 0;
        }
        return theType->Eval(key + 14);
    }
    if (strncmp(key, "properties.", 11) == 0 ||
        strncmp(key, "fields.", 11) == 0) {
        for (ufListIter(Field) i = fFields; i; i++) {
            if (strcmp(key + 11, i.fData->fName) == 0) {
                return key + 11;
            }
        }
        return 0;
    }
    if (strncmp(key, "enumentries.", 12) == 0) {
        for (ufListIter(KeyValuePair) i = fEnumEntries; i; i++) {
            if (strcmp(key + 12, i.fData->fName) == 0) {
                return key + 12;
            }
        }
        return 0;
    }

    // failed to find it in Type::Eval(), look elsewhere
    return InputBlock::Eval(key);
}

void Type::Register(ufPtr(Type) & theType) {
    fgTypeCatalog.Insert(theType->fName, theType);
    fgTypes.InsertLast(theType);
}

int Type::Find(const char *name, ufPtr(Type) & theType) {
    return fgTypeCatalog.Find(name, theType);
}

int Type::HasList(const char *key) {
    if (strcmp(key, "enumentries") == 0) {
        return fIsEnum ? 1 : 0;
    }
    if (strcmp(key, "properties") == 0 ||
        strcmp(key, "fields") == 0) {
        return fIsAggregate ? 1 : 0;
    }
    return InputBlock::HasList(key);
}

int Type::HasNonEmptyList(const char *key) {
    if (strcmp(key, "enumentries") == 0) {
        return fIsEnum && fEnumEntries;
    }
    if (strcmp(key, "properties") == 0 ||
        strcmp(key, "fields") == 0) {
        return fIsAggregate && fFields;
    }

    return InputBlock::HasNonEmptyList(key);
}

InputBlockIterator *Type::CreateIterator(const char *key) {
    if (strcmp(key, "enumentries") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fEnumEntries);
        return i;
    }
    if (strcmp(key, "properties") == 0 ||
        strcmp(key, "fields") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fFields);
        return i;
    }

    return InputBlock::CreateIterator(key);
}

void Type::PrintDebug() {
    printf("      Type={ \n"
           "        fName=\"%s\"\n"
           "        fTypeString=\"%s\"\n"
           "        fTypeCodeString=\"%s\"\n"
           "        fHeader=\"%s\"\n"
           "        fFilename=\"%s\"\n"
           "        fLineno=%d\n"
           "        fKeyValues =\n",
           fName, fTypeString, fTypeCodeString, fHeader, fFilename, fLineno);
    for (ufListIter(KeyValuePair) i = fKeyValues; i; i++) {
        i.fData->PrintDebug();
    }
}

void Type::Validate() {
    if (fTypeCode == -1) {
        fTypeCode = gLastCode++;
        fTypeCodeString = ConvertToString(fTypeCode);
    }
}

void Type::AddEnumEntry(const char *key) {
    ufPtr(KeyValuePair) theEntry = KeyValuePair::New();
    theEntry->fKey = ufStrDup(key);
    theEntry->fValue = ufStrDup("");
    fIsEnum = 1;
    fLastEnumValue++;

    ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();
    theKeyValue->fKey = ufStrDup("CalculatedValue");
    char buf[40];
    sprintf(buf, "%d", fLastEnumValue);
    theKeyValue->fValue = ufStrDup(buf);
    theEntry->fKeyValues.InsertLast(theKeyValue);

    fEnumEntries.InsertLast(theEntry);
}

void Type::AddEnumEntryPair(const char *key, int v) {
    char buf[40];

    sprintf(buf, "= %lu", (unsigned long)v);
    ufPtr(KeyValuePair) theEntry = KeyValuePair::New();
    theEntry->fKey = ufStrDup(key);
    theEntry->fValue = ufStrDup(buf);
    fIsEnum = 1;
    fLastEnumValue = v;

    ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();
    theKeyValue->fKey = ufStrDup("CalculatedValue");
    sprintf(buf, "%d", fLastEnumValue);
    theKeyValue->fValue = ufStrDup(buf);
    theEntry->fKeyValues.InsertLast(theKeyValue);

    fEnumEntries.InsertLast(theEntry);
}

void Type::AddField(const char *typeName, const char *symbolName) {
    ufPtr(Type) theType;
    ufPtr(Field) theField;

    if (!Type::Find(typeName, theType)) {
        MOCERROR((errmsg,
                  "Can't find the type \"%s\" "
                  "for field \"%s\" in class \"%s\" ",
                  typeName, symbolName, fName));
        return;
    }

    if (FindField(symbolName, theField)) {
        MOCERROR((errmsg,
                  "The field \"%s\" already exists "
                  "in class \"%s\" ",
                  symbolName, fName));
        return;
    }

    theField = Field::New(theType, symbolName);
    theCurrentField = theField;
    fFields.InsertLast(theField);
    fIsAggregate = 1;
}

int Type::FindField(const char *symbolName, ufPtr(Field) & theField) {
    for (ufListIter(Field) j = fFields; j; j++) {
        if (strcmp(j.fData->fName, symbolName) == 0) {
            theField = j.fData;
            return 1;
        }
    }
    return 0;
}

/*
 * Field Implementation
 */
implement(ufPtr, Field);
implement(ufList, Field);
implement(ufStrHash, Field);

ufPtr(Field) Field::New(ufPtr(Type) & t, const char *n) {
    Field *a = new Field(t, n);
    return a->fSelf;
}

const char *Field::GetClassName() { return "Field"; }

Field::Field(ufPtr(Type) & t, const char *n)
    : InputBlock(n), fType(t), fTypeCode(-1), fTypeCodeString(0),
      fArrayLength(-1), fArrayLengthString(0), fSelf(this, kSelfPtrShared) {}

Field::Field(const Field &o)
    : InputBlock(o){ufASSERT(0)}

      Field::~Field() {
    delete[] fTypeCodeString;
    delete[] fArrayLengthString;
}

void Field::Validate() {
    if (fTypeCode == -1) {
        fTypeCode = gLastCode++;
        fTypeCodeString = ConvertToString(fTypeCode);
    }

    if (fType != 0) {
        fType->Validate();
    }
}

Field &Field::operator=(const Field &o) {
    if (this == &o) {
        return *this;
    }

    InputBlock::operator=(o);

    fTypeCode = o.fTypeCode;

    delete[] fTypeCodeString;
    fTypeCodeString = ufStrDup(o.fTypeCodeString);

    delete[] fArrayLengthString;
    fArrayLengthString = ufStrDup(o.fArrayLengthString);

    return *this;
}

void Field::PrintDebug() {
    printf("    Field={ \n"
           "      fName=\"%s\"\n"
           "      fTypeCodeString=\"%s\"\n"
           "      fFilename=\"%s\"\n"
           "      fLineno=%d\n"
           "      fType=\n",
           fName, fTypeCodeString, fFilename, fLineno);
    fType->PrintDebug();
}

const char *Field::Eval(const char *key) {
    if (key == 0) {
        return 0;
    }
    if (strcmp(key, "TypeCode") == 0) {
        return fTypeCodeString;
    }
    if (strncmp(key, "Type.", 5) == 0) {
        return fType->Eval(key + 5);
    }
    if (strcmp(key, "ArrayLength") == 0) {
        if (fArrayLength <= 0) {
            return 0;
        }
        if (!fArrayLengthString) {
            fArrayLengthString = ConvertToString(fArrayLength);
        }
        return fArrayLengthString;
    }

    // failed to find it in Field::Eval(), look elsewhere
    return InputBlock::Eval(key);
}
/*
 * Operation Implementation
 */
implement(ufPtr, Arg);
implement(ufList, Arg);

ufPtr(Arg) Arg::New(ufPtr(Type) & t, const char *n) {
    return ufPtr(Arg)(new Arg(t, n));
}

ufPtr(Arg) Arg::New(ufPtr(Class) & t, const char *n) {
    return ufPtr(Arg)(new Arg(t, n));
}

const char *Arg::GetClassName() { return "Argument"; }

Arg::Arg(ufPtr(Type) & t, const char *n) : InputBlock(n), fType(t) {}

Arg::Arg(ufPtr(Class) & t, const char *n) : InputBlock(n), fClass(t) {}

Arg::Arg(const Arg &o)
    : InputBlock(o){ufASSERT(0)}

      Arg::~Arg() {}

void Arg::PrintDebug() {
    printf("      Arg={ \n"
           "        fName=\"%s\"\n",
           fName);
    if (fType.isValid()) {
        fType->PrintDebug();
    }
    if (fClass.isValid()) {
        fClass->PrintDebug();
    }

    printf("      }\n");
}

int Arg::HasParent(const char *key) {
    if (fClass.isValid()) {
        return fClass->HasParent(key);
    }
    if (fType.isValid()) {
        return fType->HasParent(key);
    }

    return InputBlock::HasParent(key);
}

const char *Arg::Eval(const char *key) {
    if (key == 0) {
        return 0;
    }

    if (strcmp(key, "IsArg") == 0) {
        return "1";
    }
    if (strncmp(key, "Type.", 5) == 0) {
        if (fType.isValid()) {
            return fType->Eval(key + 5);
        } else if (fClass.isValid()) {
            return fClass->Eval(key + 5);
        } else {
            return 0;
        }
    }

    // failed to find it in Arg::Eval(), look elsewhere
    return InputBlock::Eval(key);
}

implement(ufPtr, Operation);
implement(ufList, Operation);
implement(ufStrHash, Operation);

ufPtr(Operation) Operation::New(const char *nm) {
    return (new Operation(nm))->fSelf;
}

const char *Operation::GetClassName() { return "Operation"; }

Operation::Operation(const char *name)
    : InputBlock(name), fTypeCode(-1), fTypeCodeString(0),
      fSelf(this, kSelfPtrShared) {}

Operation::Operation(const Operation &o)
    : InputBlock(o){ufASSERT(0)}

      Operation::~Operation() {
    delete[] fTypeCodeString;
}

void Operation::Validate() {
    if (fTypeCode == -1) {
        fTypeCode = gLastCode++;
        fTypeCodeString = ConvertToString(fTypeCode);
    }
}

void Operation::PrintDebug() {
    printf("    Operation={ \n"
           "      fName=\"%s\"\n"
           "      fTypeCodeString=\"%s\"\n"
           "      fFilename=\"%s\"\n"
           "      fLineno=%d\n"
           "      fInArguments = \n",
           fName, fTypeCodeString, fFilename, fLineno);
    for (ufListIter(Arg) i = fInArguments; i; i++) {
        i.fData->PrintDebug();
    }

    printf("      fOutArguments = \n");
    for (ufListIter(Arg) o = fOutArguments; o; o++) {
        o.fData->PrintDebug();
    }

    printf("      fErrorArguments = \n");
    for (ufListIter(Arg) e = fErrorArguments; e; e++) {
        e.fData->PrintDebug();
    }

    printf("      }\n");
}

const char *Operation::Eval(const char *key) {
    if (key == 0) {
        return 0;
    }

    if (strcmp(key, "IsMethod") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        return "1";
    }
    if (strcmp(key, "TypeCode") == 0) {
        return fTypeCodeString;
    }

    if (strncmp(key, "in-arguments.", 13) == 0) {
        int keyLen = strlen(key + 13);
        for (ufListIter(Arg) i = fInArguments; i; i++) {
            if (strcmp(key + 13, i.fData->fName) == 0) {
                return key + 13;
            }
            int newLen = strlen(i.fData->fName);
            if (newLen < keyLen && key[newLen + 13] == '.') {
                return i.fData->Eval(key + 13 + newLen);
            }
        }
        return 0;
    }
    if (strncmp(key, "out-arguments.", 14) == 0) {
        int keyLen = strlen(key + 14);
        for (ufListIter(Arg) i = fOutArguments; i; i++) {
            if (strcmp(key + 14, i.fData->fName) == 0) {
                return key + 14;
            }
            int newLen = strlen(i.fData->fName);
            if (newLen < keyLen && key[newLen + 14] == '.') {
                return i.fData->Eval(key + 14 + newLen);
            }
        }
        return 0;
    }
    if (strncmp(key, "error-arguments.", 16) == 0) {
        int keyLen = strlen(key + 16);
        for (ufListIter(Arg) i = fErrorArguments; i; i++) {
            if (strcmp(key + 16, i.fData->fName) == 0) {
                return key + 16;
            }
            int newLen = strlen(i.fData->fName);
            if (newLen < keyLen && key[newLen + 16] == '.') {
                return i.fData->Eval(key + 16 + newLen);
            }
        }
        return 0;
    }
    // failed to find it in Operation::Eval(), look elsewhere
    return InputBlock::Eval(key);
}

int Operation::HasList(const char *key) {
    if (strcmp(key, "in-arguments") == 0) {
        return 1;
    } else if (strcmp(key, "out-arguments") == 0) {
        return 1;
    } else if (strcmp(key, "error-arguments") == 0) {
        return 1;
    }

    return InputBlock::HasList(key);
}

int Operation::HasNonEmptyList(const char *key) {
    if (strcmp(key, "in-arguments") == 0) {
        return fInArguments;
    } else if (strcmp(key, "out-arguments") == 0) {
        return fOutArguments;
    } else if (strcmp(key, "error-arguments") == 0) {
        return fErrorArguments;
    }

    return InputBlock::HasNonEmptyList(key);
}

InputBlockIterator *Operation::CreateIterator(const char *key) {
    if (strcmp(key, "in-arguments") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fInArguments);
        return i;
    } else if (strcmp(key, "out-arguments") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fOutArguments);
        return i;
    } else if (strcmp(key, "error-arguments") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fErrorArguments);
        return i;
    }

    return InputBlock::CreateIterator(key);
}

void Operation::AddInArgument(const char *typeName, const char *symbolName) {
    ufPtr(Arg) theArg;
    theCurrentOpArg.MakeEmpty();
    if (MakeArgument(typeName, symbolName, theArg)) {
        fInArguments.InsertLast(theArg);
        theCurrentOpArg = theArg;
    }
}

void Operation::AddOutArgument(const char *typeName, const char *symbolName) {
    ufPtr(Arg) theArg;
    theCurrentOpArg.MakeEmpty();

    if (MakeArgument(typeName, symbolName, theArg)) {
        fOutArguments.InsertLast(theArg);
        theCurrentOpArg = theArg;
    }
}

void Operation::AddErrorArgument(const char *typeName, const char *symbolName) {
    ufPtr(Arg) theArg;
    theCurrentOpArg.MakeEmpty();

    if (MakeArgument(typeName, symbolName, theArg)) {
        fErrorArguments.InsertLast(theArg);
        theCurrentOpArg = theArg;
    }
}

int Operation::MakeArgument(const char *typeName, const char *symbolName,
                            ufPtr(Arg) & theArg) {
    ufPtr(Type) theType;
    ufPtr(Class) theClass;

    if (Type::Find(typeName, theType)) {
        theArg = Arg::New(theType, symbolName);
    } else if (Class::Find(typeName, theClass)) {
        theArg = Arg::New(theClass, symbolName);
    } else {
        MOCERROR((errmsg,
                  "Can't find the type or class \"%s\" for "
                  "argument/exception \"%s\" in operation \"%s\" ",
                  typeName, symbolName, fName));
        return 0;
    }

    return 1;
}

Operation &Operation::operator=(const Operation &o) {
    if (this == &o) {
        return *this;
    }

    InputBlock::operator=(o);
    fAssocClass = o.fAssocClass;

    fTypeCode = o.fTypeCode;

    delete[] fTypeCodeString;
    fTypeCodeString = ufStrDup(o.fTypeCodeString);

    return *this;
}

/*
 * Class Implementation
 */
implement(ufPtr, Class) implement(ufList, Class) implement(ufStrHash, Class)

ufStrHash(Class) Class::fgClassCatalog;
ufList(Class) Class::fgClasses;

ufPtr(Class) Class::New(const char *name) { return (new Class(name))->fSelf; }

const char *Class::GetClassName() { return "Class"; }

Class::Class(const char *name)
    : InputBlock(name), fTypeCode(-1), fTypeCodeString(0), fIsAbstract(0),
      fIsBusy(0), fIsValidated(0), fIsInterface(0),
      fSelf(this, kSelfPtrShared) {
    Register(fSelf);
}

Class::Class(const Class &o)
    : InputBlock(o){ufASSERT(0)}

      Class::~Class() {
    delete[] fTypeCodeString;
}

void Class::Register(ufPtr(Class) & theClass) {
    fgClassCatalog.Insert(theClass->fName, theClass);
    fgClasses.InsertLast(theClass);
}

int Class::Find(const char *name, ufPtr(Class) & theClass) {
    return fgClassCatalog.Find(name, theClass);
}

int Class::FindOp(const char *name, ufPtr(Operation) & theOp) {
    ufListIter(Operation) i;
    for (i = fOperations; i; i++) {
        if (strcmp(name, i.fData->fName) == 0) {
            theOp = i.fData;
            return 1;
        }
    }
    for (i = fInheritedOperations; i; i++) {
        if (strcmp(name, i.fData->fName) == 0) {
            theOp = i.fData;
            return 1;
        }
    }
    return 0;
}

int Class::HasParent(const char *key) {
    if (strcmp(key, "Interface") == 0) {
        return fIsInterface ? 1 : 0;
    }
    if (strcmp(key, fName) == 0) {
        return 1;
    }

    for (ufListIter(Class) i = fParents; i; i++) {
        if (strcmp(key, i.fData->fName) == 0 || i.fData->HasParent(key)) {
            return 1;
        }
    }

    return 0;
}

void Class::AddParent(const char *name) {
    ufPtr(Class) theParent;
    if (Class::Find(name, theParent)) {
        fParents.InsertLast(theParent);
    } else {
        MOCERROR((errmsg,
                  "Can't find parent \"%s\" for "
                  "class \"%s\"",
                  name, fName));
    }
}

void Class::AddChild(ufPtr(Class) & theChild) {
    fChildren.InsertLast(theChild);
    for (ufListIter(Class) i = fParents; i; i++) {
        i.fData->AddInheritedToChild(theChild);
    }
    AddInheritedToChild(theChild);
}

void Class::AddInheritedToChild(ufPtr(Class) & theChild) {
    ufListIter(Field) j;
    ufListIter(Operation) l;
    ufListIter(Class) k;

    for (j = fInheritedFields; j; j++) {
        theChild->AddInheritedField(j.fData);
    }
    for (j = fFields; j; j++) {
        theChild->AddInheritedField(j.fData);
    }
    for (l = fInheritedOperations; l; l++) {
        theChild->AddInheritedOperation(l.fData);
    }
    for (l = fOperations; l; l++) {
        theChild->AddInheritedOperation(l.fData);
    }
    for (k = fParents; k; k++) {
        theChild->AddInheritedParent(k.fData);
    }
    for (k = fAllParents; k; k++) {
        theChild->AddInheritedParent(k.fData);
    }
    theChild->AddInheritedParent(fSelf);
}

void Class::ValidateAll() {
    ufListIter(Class) i(fgClasses);
    for (; i; i++) {
        i.fData->Validate();
    }
}

void Class::Validate() {
    if (fIsValidated) {
        return;
    }

    if (fIsBusy) {
        MOCERROR((errmsg,
                  "Circular inheritance dependency "
                  "for class \"%s\"",
                  fName));
        exit(kBadSchema);
    }

    fIsBusy = 1;

    for (ufListIter(Class) i = fParents; i; i++) {
        i.fData->Validate();
    }

    if (fTypeCode == -1) {
        fTypeCode = gLastCode++;
        fTypeCodeString = ConvertToString(fTypeCode);
    }

    for (ufListIter(Class) pi = fParents; pi; pi++) {
        pi.fData->AddChild(fSelf);
    }

    for (ufListIter(Operation) oi = fOperations; oi; oi++) {
        oi.fData->Validate();
    }

    for (ufListIter(Field) ai = fFields; ai; ai++) {
        ai.fData->Validate();
    }

    fIsValidated = 1;
}

void Class::PrintDebug() {
    printf("Class={ \n"
           "  fName=\"%s\"\n"
           "  fTypeCodeString=\"%s\"\n"
           "  fFilename=\"%s\"\n"
           "  fLineno=%d\n"
           "  fParents= \n",
           fName, fTypeCodeString, fFilename, fLineno);
    for (ufListIter(Class) i = fParents; i; i++) {
        printf("    \"%s\"\n", i.fData->fName);
    }

    printf("  fOperations = \n");
    for (ufListIter(Operation) oi = fOperations; oi; oi++) {
        oi.fData->PrintDebug();
    }

    printf("  fInheritedOperations = \n");
    for (ufListIter(Operation) oj = fInheritedOperations; oj; oj++) {
        oj.fData->PrintDebug();
    }

    printf("  fFields = \n");
    for (ufListIter(Field) ai = fFields; ai; ai++) {
        ai.fData->PrintDebug();
    }

    printf("  fInheritedFields = \n");
    for (ufListIter(Field) aj = fInheritedFields; aj; aj++) {
        aj.fData->PrintDebug();
    }

    printf("}\n");
}

const char *Class::Eval(const char *key) {
    if (key == 0) {
        return 0;
    }

    if (strcmp(key, "TypeCode") == 0) {
        return fTypeCodeString;
    }
    if (strcmp(key, "TypeString") == 0) {
        return fName;
    }
    if (strcmp(key, "IsClass") == 0) {
        return "1";
    }
    if (strcmp(key, "IsAbstract") == 0) {
        return fIsAbstract ? "1" : "0";
    }

    if (strncmp(key, "properties.", 11) == 0 ||
        strncmp(key, "fields.", 11) == 0) {
        for (ufListIter(Field) i = fFields; i; i++) {
            if (strcmp(key + 11, i.fData->fName) == 0) {
                return key + 11;
            }
        }
        return 0;
    }
    if (strncmp(key, "inherited-properties.", 21) == 0 ||
        strncmp(key, "inherited-fields.", 21) == 0) {
        for (ufListIter(Field) i = fInheritedFields; i; i++) {
            if (strcmp(key + 21, i.fData->fName) == 0) {
                return key + 21;
            }
        }
        return 0;
    }
    if (strncmp(key, "operations.", 11) == 0 ||
        strncmp(key, "methods.", 8) == 0) {
        for (ufListIter(Operation) i = fOperations; i; i++) {
            if (strcmp(key + 11, i.fData->fName) == 0) {
                return key + 11;
            }
        }
        return 0;
    }
    if (strncmp(key, "inherited-operations.", 21) == 0 ||
        strncmp(key, "inherited-methods.", 18) == 0) {
        for (ufListIter(Operation) i = fInheritedOperations; i; i++) {
            if (strcmp(key + 21, i.fData->fName) == 0) {
                return key + 21;
            }
        }
        return 0;
    }
    if (strncmp(key, "parents.", 8) == 0) {
        for (ufListIter(Class) i = fParents; i; i++) {
            if (strcmp(key + 8, i.fData->fName) == 0) {
                return key + 8;
            }
        }
        return 0;
    }
    if (strncmp(key, "all-parents.", 8) == 0) {
        for (ufListIter(Class) i = fAllParents; i; i++) {
            if (strcmp(key + 8, i.fData->fName) == 0) {
                return key + 8;
            }
        }
        return 0;
    }
    if (strcmp(key, "IsAggregate") == 0) {
        return "1";
    }

    // failed to find it in Class::Eval(), look elsewhere
    const char *v;
    if ((v = InputBlock::Eval(key)) != 0) {
        return v;
    }

    for (ufListIter(Class) i = fParents; i; i++) {
        if ((v = i.fData->Eval(key)) != 0) {
            return v;
        }
    }

    return 0;
}

int Class::HasList(const char *key) {
    if (strcmp(key, "properties") == 0) {
        return 1;
    } else if (strcmp(key, "inherited-properties") == 0) {
        return 1;
    } else if (strcmp(key, "methods") == 0) {
        return 1;
    } else if (strcmp(key, "inherited-methods") == 0) {
        return 1;
    } else if (strcmp(key, "parents") == 0) {
        return 1;
    } else if (strcmp(key, "all-parents") == 0) {
        return 1;
    } else if (strcmp(key, "fields") == 0) {
        return 1;
    } else if (strcmp(key, "inherited-fields") == 0) {
        return 1;
    } else if (strcmp(key, "operations") == 0) {
        return 1;
    } else if (strcmp(key, "inherited-operations") == 0) {
        return 1;
    }

    return InputBlock::HasList(key);
}

int Class::HasNonEmptyList(const char *key) {
    if (strcmp(key, "properties") == 0) {
        return fFields;
    } else if (strcmp(key, "inherited-properties") == 0) {
        return fInheritedFields;
    } else if (strcmp(key, "methods") == 0) {
        return fOperations;
    } else if (strcmp(key, "inherited-methods") == 0) {
        return fInheritedOperations;
    } else if (strcmp(key, "parents") == 0) {
        return fParents;
    } else if (strcmp(key, "all-parents") == 0) {
        return fAllParents;
    } else if (strcmp(key, "fields") == 0) {
        return fFields;
    } else if (strcmp(key, "inherited-fields") == 0) {
        return fInheritedFields;
    } else if (strcmp(key, "operations") == 0) {
        return fOperations;
    } else if (strcmp(key, "inherited-operations") == 0) {
        return fInheritedOperations;
    }
    return InputBlock::HasNonEmptyList(key);
}

InputBlockIterator *Class::CreateIterator(const char *key) {
    if (strcmp(key, "properties") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fFields);
        return i;
    } else if (strcmp(key, "inherited-properties") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fInheritedFields);
        return i;
    } else if (strcmp(key, "methods") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fOperations);
        return i;
    } else if (strcmp(key, "inherited-methods") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fInheritedOperations);
        return i;
    } else if (strcmp(key, "parents") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fParents);
        return i;
    } else if (strcmp(key, "all-parents") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fAllParents);
        return i;
    } else     if (strcmp(key, "fields") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fFields);
        return i;
    } else if (strcmp(key, "inherited-fields") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fInheritedFields);
        return i;
    } else if (strcmp(key, "operations") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fOperations);
        return i;
    } else if (strcmp(key, "inherited-operations") == 0) {
        InputBlockIterator *i = new InputBlockIterator(fInheritedOperations);
        return i;
    }

    return InputBlock::CreateIterator(key);
}

void Class::AddOperation(const char *name) {
    ufPtr(Operation) theOp = Operation::New(name);
    theOp->fAssocClass = fSelf;
    fOperations.InsertLast(theOp);
    theCurrentOp = theOp;
}

void Class::AddField(const char *typeName, const char *symbolName) {
    ufPtr(Type) theType;
    ufPtr(Field) theField;

    if (!Type::Find(typeName, theType)) {
        MOCERROR((errmsg,
                  "Can't find the type \"%s\" "
                  "for field \"%s\" in class \"%s\" ",
                  typeName, symbolName, fName));
        return;
    }

    if (FindField(symbolName, theField)) {
        MOCERROR((errmsg,
                  "The field \"%s\" already exists "
                  "in class \"%s\" ",
                  symbolName, fName));
        return;
    }

    theField = Field::New(theType, symbolName);
    theCurrentField = theField;
    fFields.InsertLast(theField);
}

int Class::FindField(const char *symbolName, ufPtr(Field) & theField) {
    for (ufListIter(Field) j = fFields; j; j++) {
        if (strcmp(j.fData->fName, symbolName) == 0) {
            theField = j.fData;
            return 1;
        }
    }
    for (ufListIter(Class) i = fParents; i; i++) {
        if (i.fData->FindField(symbolName, theField)) {
            return 1;
        }
    }
    return 0;
}

void Class::AddInheritedField(ufPtr(Field) & theField) {
    int found = 0;
    for (ufListIter(Field) i = fFields; i && !found; i++) {
        if (strcmp(i.fData->fName, theField->fName) == 0) {
            found = 1;
            break;
        }
    }
    for (ufListIter(Field) j = fInheritedFields; j && !found; j++) {
        if (strcmp(j.fData->fName, theField->fName) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        fInheritedFields.InsertLast(theField);
    }
}

void Class::AddInheritedOperation(ufPtr(Operation) & theOp) {
    int found = 0;
    for (ufListIter(Operation) i = fOperations; i && !found; i++) {
        if (strcmp(i.fData->fName, theOp->fName) == 0) {
            found = 1;
            break;
        }
    }
    for (ufListIter(Operation) j = fInheritedOperations; j && !found; j++) {
        if (strcmp(j.fData->fName, theOp->fName) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        fInheritedOperations.InsertLast(theOp);
    }
}

void Class::AddInheritedParent(ufPtr(Class) & theClass) {
    int found = 0;
    ufListIter(Class) i;
    for (i = fAllParents; i && !found; i++) {
        if (strcmp(i.fData->fName, theClass->fName) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        fAllParents.InsertLast(theClass);
    }
}

implement(ufPtr, GlobalScope);

ufPtr(KeyValuePair) GlobalScope::fgHeaderCode;
ufPtr(KeyValuePair) GlobalScope::fgHeaderEndCode;
ufPtr(KeyValuePair) GlobalScope::fgSourceCode;

ufStrHash(KeyValuePair) GlobalScope::fgConstantCatalog;
ufList(KeyValuePair) GlobalScope::fgConstants;
ufStrHash(KeyValuePair) GlobalScope::fgVariableCatalog;
ufList(KeyValuePair) GlobalScope::fgVariables;
ufStrHash(KeyValuePair) GlobalScope::fgModuleCatalog;
ufList(KeyValuePair) GlobalScope::fgModules;

ufPtr(InputBlock) GlobalScope::fgSelf;

const char *GlobalScope::GetClassName() { return "Global Scope"; }

ufPtr(InputBlock) GlobalScope::New() {
    if (fgSelf == 0) {
        GlobalScope *self = new GlobalScope;
        ufPtr(GlobalScope) p(self);
        fgSelf = (ufPtr(InputBlock) &)p;
    }
    return fgSelf;
}

GlobalScope::GlobalScope() : InputBlock("GLOBAL") {}

static bool KeyValueEval(const char* desiredName,
                         const char* keyValueName,
                         ufPtr(KeyValuePair)& keyValue,
                         const char*& result) {
    int len = strlen(keyValueName);
    int lenOfDesiredName = strlen(desiredName);
    int matched = 0;
    result = 0;
    bool verbose =  lenOfDesiredName > 2 && (
                                             (desiredName[0] == 'H' &&
                                              desiredName[1] == 'E')
                                             ||
                                             desiredName[0] == 'S');
    if (!keyValue) {
        return matched;
    }
    if (strncmp(desiredName, keyValueName, len) != 0) {
        return matched;
    }
    matched = 1;
    if (lenOfDesiredName == len) {
        result = keyValue->fValue;
        return matched;
    }
    if (desiredName[len] == '.') {
        result = keyValue->Eval(desiredName + len + 1);
    }
    return matched;
}

GlobalScope::GlobalScope(const GlobalScope &o)
    : InputBlock(o){ufASSERT(0)}

      GlobalScope::~GlobalScope() {}

const char *GlobalScope::Eval(const char *key) {
    if (strcmp(key, "MODULE") == 0) {
        return gModule ? gModule : "";
    }
    const char* result=0;
    if (KeyValueEval(key, "HEADER_CODE", GlobalScope::fgHeaderCode, result)) {
        return result;
    }
    if (KeyValueEval(key, "HEADER_END_CODE", GlobalScope::fgHeaderEndCode, result)) {
        return result;
    }
    if (KeyValueEval(key, "SOURCE_CODE", GlobalScope::fgSourceCode, result)) {
        return result;
    }
    bool wasFound;
    result = InputBlock::LocalEval(key, wasFound);
    if (wasFound) {
        return result;
    }

    ufPtr(KeyValuePair) theVar;
    if (GlobalScope::fgVariableCatalog &&
        GlobalScope::fgVariableCatalog.Find(key, theVar)) {
        return theVar->fValue;
    }

    return 0;
}
/*
 * InputBlockIterator implementation
 */
InputBlockIterator::InputBlockIterator(ufList(Class) & l)
    : fCount(1), fTotal(0), fClasses(0), fTypes(0), fFields(0),
      fOperations(0), fArgs(0), fKeyValues(0) {
    fClasses = new ufListIter(Class)(l);
    fTotal = l.GetNumElements();
    fData = (ufPtr(InputBlock) &)fClasses->fData;
    // DataChanged(); //virt method so subclasses need to call this
    // in their constructors
}

InputBlockIterator::InputBlockIterator(ufList(Type) & l)
    : fCount(1), fTotal(0), fClasses(0), fTypes(0), fFields(0),
      fOperations(0), fArgs(0), fKeyValues(0) {
    fTypes = new ufListIter(Type)(l);
    fTotal = l.GetNumElements();
    fData = (ufPtr(InputBlock) &)fTypes->fData;
    // DataChanged(); //virt method so subclasses need to call this
    // in their constructors
}

InputBlockIterator::InputBlockIterator(ufList(Field) & l)
    : fCount(1), fTotal(0), fClasses(0), fTypes(0), fFields(0),
      fOperations(0), fArgs(0), fKeyValues(0) {
    fFields = new ufListIter(Field)(l);
    fTotal = l.GetNumElements();
    fData = (ufPtr(InputBlock) &)fFields->fData;
    // DataChanged(); //virt method so subclasses need to call this
    // in their constructors
}

InputBlockIterator::InputBlockIterator(ufList(Operation) & l)
    : fCount(1), fTotal(0), fClasses(0), fTypes(0), fFields(0),
      fOperations(0), fArgs(0), fKeyValues(0) {
    fOperations = new ufListIter(Operation)(l);
    fTotal = l.GetNumElements();
    fData = (ufPtr(InputBlock) &)fOperations->fData;
    // DataChanged(); //virt method so subclasses need to call this
    // in their constructors
}

InputBlockIterator::InputBlockIterator(ufList(Arg) & l)
    : fCount(1), fTotal(0), fClasses(0), fTypes(0), fFields(0),
      fOperations(0), fArgs(0), fKeyValues(0) {
    fArgs = new ufListIter(Arg)(l);
    fTotal = l.GetNumElements();
    fData = (ufPtr(InputBlock) &)fArgs->fData;
    // DataChanged(); //virt method so subclasses need to call this
    // in their constructors
}

InputBlockIterator::InputBlockIterator(ufList(KeyValuePair) & l)
    : fCount(1), fTotal(0), fClasses(0), fTypes(0), fFields(0),
      fOperations(0), fArgs(0), fKeyValues(0) {
    fKeyValues = new ufListIter(KeyValuePair)(l);
    fTotal = l.GetNumElements();
    fData = (ufPtr(InputBlock) &)fKeyValues->fData;
    // DataChanged(); //virt method so subclasses need to call this
    // in their constructors
}

InputBlockIterator::InputBlockIterator()
    : fCount(1), fTotal(0), fClasses(0), fTypes(0), fFields(0),
      fOperations(0), fArgs(0), fKeyValues(0) {}

InputBlockIterator::~InputBlockIterator() {}

int InputBlockIterator::IsFirst() { return fCount == 1; }

int InputBlockIterator::IsLast() { return fCount >= GetTotal(); }

int InputBlockIterator::IsExhausted() { return fCount > GetTotal(); }

InputBlockIterator &InputBlockIterator::operator++() { return operator++(1); }

InputBlockIterator &InputBlockIterator::operator++(int) {
    if (fCount > GetTotal()) {
        return *this;
    }
    fCount++;
    if (fClasses) {
        (*fClasses)++;
        fData = (ufPtr(InputBlock) &)fClasses->fData;
        DataChanged();
    } else if (fTypes) {
        (*fTypes)++;
        fData = (ufPtr(InputBlock) &)fTypes->fData;
        DataChanged();
    } else if (fFields) {
        (*fFields)++;
        fData = (ufPtr(InputBlock) &)fFields->fData;
        DataChanged();
    } else if (fArgs) {
        (*fArgs)++;
        fData = (ufPtr(InputBlock) &)fArgs->fData;
        DataChanged();
    } else if (fOperations) {
        (*fOperations)++;
        fData = (ufPtr(InputBlock) &)fOperations->fData;
        DataChanged();
    } else if (fKeyValues) {
        (*fKeyValues)++;
        fData = (ufPtr(InputBlock) &)fKeyValues->fData;
        DataChanged();
    }
    return *this;
}

void InputBlockIterator::DataChanged() {
}

InputBlockIterator::operator PTR_INT() const {
    int t = GetTotal();
    return t && fCount <= t;
}

int InputBlockIterator::GetTotal() const { return fTotal; }

/*
 * MultiListInputBlockIterator implementation
 */
MultiListInputBlockIterator::MultiListInputBlockIterator()
    : InputBlockIterator(), fNumSubiterators(0), fCurrentSubiterator(0) {
    for (int i = 0; i < kMaxSubiterators; i++) {
        fSubiterators[i] = 0;
    }
}

MultiListInputBlockIterator::~MultiListInputBlockIterator() {
    for (int i = 0; i < kMaxSubiterators; i++) {
        delete fSubiterators[i];
        fSubiterators[i] = 0;
    }
}

void MultiListInputBlockIterator::AddSubiterator(InputBlockIterator *it) {
    ufASSERT(it != 0) if (fNumSubiterators + 1 >= kMaxSubiterators) { return; }
    if (!fData) {
        fCurrentSubiterator = fNumSubiterators;
        fData = it->fData;
    }
    fSubiterators[fNumSubiterators++] = it;
}

int MultiListInputBlockIterator::GetTotal() const {
    int t = 0;
    for (int i = 0; i < kMaxSubiterators; i++) {
        t += fSubiterators[i] ? fSubiterators[i]->GetTotal() : 0;
    }
    return t;
}

InputBlockIterator &MultiListInputBlockIterator::operator++() {
    return operator++(1);
}

InputBlockIterator &MultiListInputBlockIterator::operator++(int) {
    int isNew = 0;
    int t = GetTotal();

    while (fCount <= t) {
        if (!fSubiterators[fCurrentSubiterator]) {
            break;
        }
        if ((isNew && fSubiterators[fCurrentSubiterator]->IsFirst()) ||
            fSubiterators[fCurrentSubiterator]->IsLast() == 0) {
            if (!isNew) {
                (*fSubiterators[fCurrentSubiterator])++;
            }
            fData = fSubiterators[fCurrentSubiterator]->fData;
            if (fData != 0) {
                fCount++;
                return *this;
            }
        }
        fCurrentSubiterator++;
        isNew = 1;
    }

    fData.MakeEmpty();
    return *this;
}

static int getcNoReturn(FILE *in) {
    int c;
    do {
        c = getc(in);
    } while (c == '\r');
    return c;
}

/* reads a line in. will handle line continuation if the @\ is used
 * deals with either @\ followed either \n or \r\n (windows)
 */
char *FGets(ufTmpBuf &tmp, FILE *in) {
    if (gNumErrors || in == 0) {
        return 0;
    }

    char c;
    char p3 = '\0';
    char p2 = '\0';
    char p1 = '\0';
    int i;
    for (i = 0; (c = fgetc(in)) != EOF;) {
        tmp.fBuf[i++] = c;
        if (tmp.GetSize() < i + 10) {
            tmp.Reallocate(i + 1024);
        }
        if (c == '\n') {
            lineno++;

            if (p3 == '@' && p2 == '\\' && p1 == '\r') {
                i -= 4;
            } else if (p2 == '@' && p1 == '\\') {
                i -= 3;
            } else {
                break;
            }
        }
        p3 = p2;
        p2 = p1;
        p1 = c;
    }
    tmp.fBuf[i] = tmp.fBuf[i + 1] = '\0';
    return i > 0 ? tmp.fBuf : 0;
}

static char *ConvertToString(int l) {
    char *s;
    char buf[20];
    sprintf(buf, "%d", l);
    s = ufStrDup(buf);
    return s;
}
