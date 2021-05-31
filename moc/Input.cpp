/*
 * See Readme, Notice and License files.
 */
#include "Input.h"
#include "MOC.h"
#include "MOCInternal.h"
#include "MOCTokens.h"
#include <ctype.h>

UserCode *newUserCodeWithFields(char* value, char* filename, int lineno);

static int EvalArgList(ufList(Arg)& list,
                       ufStringReadonlyCursor& key, ufString& resultToOwn);
static int EvalKeyValuePairList(ufList(KeyValuePair)& list,
                                ufStringReadonlyCursor& key,
                                ufString& resultToOwn);
static int EvalFieldList(ufList(Field)& list,
                         ufStringReadonlyCursor& key, ufString& resultToOwn);
static int EvalOperationList(ufList(Operation)& list,
                             ufStringReadonlyCursor& key,
                             ufString& resultToOwn);
static int EvalClassList(ufList(Class)& list,
                         ufStringReadonlyCursor& key, ufString& resultToOwn);

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
    return newUserCodeWithFields(s, ufStrDup(gFilename), lineno);
}

UserCode *newUserCodeWithFields(char* value, char* filename, int lineno) {
    UserCode *uc = new UserCode;
    uc->fCodeLen = strlen(value);
    uc->fCode = value;
    uc->fFilename = filename;
    uc->fLineno = lineno;
    return uc;
}

UserCode *newUserCode(const char *message) {
    return newUserCode2(message, 0);
}

UserCode *newUserCode2(const char *message, const char *message2) {
    return newUserCodeWithFields(ufStrCat(message, message2),
                                 ufStrDup(gFilename), lineno);
}

UserCode *newUserCode(ufTmpBuf tmp, const char* filename, int lineno) {
    return newUserCodeWithFields(ufStrDup(tmp.fBuf),
                                 ufStrDup(filename), lineno);
}

inline char toChar(char h, char l)
{
    char ret;
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

//JMM
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
        ufTmpBuf buf(1024);
        sprintf(buf.fBuf,
                "expected a '{' following the code "
                "keyword instead found a '%c'",
                c);
        mocError2(buf.fBuf);
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
            ufTmpBuf buf(1024);
            sprintf(buf.fBuf,
                    "expected a '{' following the code "
                    "keyword instead found a '%c'",
                    c);
            mocError2(buf.fBuf);
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

        tmp.Buf(i++, 50000) = c;

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
                    tmp[i] = '\0';
                    tmp.ConcatBuf(tmp2);
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
                tmp[i++] = c;
            }
        } else if (c == '*' && (c = getc(in)) != EOF) {
            if (c != '/') {
                ungetc(c, in);
            } else {
                isComment = 0;
                tmp[i++] = c;
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
            tmp[i - 1] = c;
        } else if (c == '\'' && !isDoubleQuoted && !isComment &&
                   (c = getc(in)) != EOF && (c2 = getc(in)) != EOF) {
            tmp[i++] = c;
            tmp[i++] = c2;
        }

        isFirst = 0;
    }

    tmp[i] = '\0';

    UserCode *uc = newUserCode(tmp, fnm, line);

    return uc;
}

BOOL mocIsModule(const char *module) {
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

    theModule->fKey = module;
    theModule->fValue = fullPath;

    gModule = theModule->fKey;
    gModuleFullFilename = theModule->fValue;

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

    theConst->fKey = name;
    theConst->fValue.MakeOwner(ConvertToString(num));

    GlobalScope::fgConstants.InsertLast(theConst);
    GlobalScope::fgConstantCatalog.Insert(theConst->fKey, theConst);
}

void DeclareVariable(const char *name, const char *value) {
    ufPtr(KeyValuePair) theVar = KeyValuePair::New();

    theVar->fKey = name;

    // Strip double quotes
    if (*value == '"') {
        value++;
        theVar->fValue = value;
        char* v = theVar->fValue.CStrForMod();
        int len = strlen(v);
        if (len > 0 && v[len - 1] == '"') {
            v[len - 1] = '\0';
            len--;
        }
    } else {
        theVar->fValue = value;
    }

    GlobalScope::fgVariables.InsertLast(theVar);
    GlobalScope::fgVariableCatalog.Insert(theVar->fKey, theVar);
}

BOOL mocIsConstant(const char *name) {
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
    theCurrentType->fTypeString = name;
}

void AddTypeEnumEntry(const char *name) { theCurrentType->AddEnumEntry(name); }

void AddTypeEnumEntryPair(const char *name, int e) {
    theCurrentType->AddEnumEntryPair(name, e);
}

void AddStructField(const char *typeName, const char *symbolName) {
    theCurrentType->AddField(typeName, symbolName);
}

void SetTypeHeader(const char *headerName) {
    theCurrentType->fHeader = headerName;
}

void SetTypeKeyValue(const char *key, UserCode *value) {
    theCurrentType->AddKeyValue(key, value);
}

void SetTypeDef(const char *realType) {
    theCurrentType->fIsTypeDef = 1;
    theCurrentType->fTypeDefString = realType;
}

BOOL mocIsTypeDefined(const char *n) {
    ufPtr(Type) theType;

    return Type::Find(n, theType);
}

BOOL mocIsClassDefined(const char *n) {
    ufPtr(Class) theClass;

    return Class::Find(n, theClass);
}

static void SetTopLevelCodeBlock(const char* nameOfCodeBlock,
                                 ufPtr(KeyValuePair)& codeBlockPtr,
                                 UserCode *uc) {
    if (uc) {
        ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();
        theKeyValue->fKey.MakeStringLiteral( nameOfCodeBlock );
        theKeyValue->fValue.TakeOwnership(uc->fCode);
        theKeyValue->fFilename.TakeOwnership(uc->fFilename);
        theKeyValue->fLineno = uc->fLineno;
        codeBlockPtr = theKeyValue;
    }
}

void SetHeaderCode(UserCode *uc) {
    SetTopLevelCodeBlock("HEADER_CODE", GlobalScope::fgHeaderCode, uc);
    delete uc;
}

void SetHeaderEndCode(UserCode *uc) {
    SetTopLevelCodeBlock("HEADER_END_CODE", GlobalScope::fgHeaderEndCode, uc);
    delete uc;
}

void SetSourceCode(UserCode *uc) {
    SetTopLevelCodeBlock("SOURCE_CODE", GlobalScope::fgSourceCode, uc);
    delete uc;
}

/*
 * Class stuff
 */
static ufPtr(Class) theCurrentClass;
static ufPtr(Field) theCurrentField;
static ufPtr(Operation) theCurrentOp;
static ufPtr(Arg) theCurrentOpArg;

void WarnIfNotASubclassOf(const char *n, const char *p) {
    ufStringLiteral className(n);
    if (className == "Start"){
        return;
    }

    ufPtr(Class) theClass;
    if (!Class::Find(className, theClass)) {
        const int sz = 200 - 1;
        ufTmpBuf buf(sz + 1);
        buf += "\"";
        buf += className;
        buf += "\" is not even a class. It must be a subclass of ";
        buf += p;
        mocWarning(buf.fBuf);
        return;
    }

    if (!theClass->HasParent(p)) {
        const int sz = 200 - 1;
        ufTmpBuf buf(sz + 1);
        buf += "\"";
        buf += className;
        buf += "\" must be a subclass of ";
        buf += p;
        mocWarning(buf.fBuf);
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
    : fFilename(gFilename), fLineno(lineno), fLinenoBuf(0),
      fName(name), fUserCode(0), fUserCodeFilename(0),
      fUserCodeLineno(1), fUserCodeLinenoBuf(0) {}

InputBlock::InputBlock(const InputBlock &o){ufASSERT(0)}

InputBlock::~InputBlock() {
}

const char *InputBlock::GetClassName() { return "InputBlock"; }

BOOL InputBlock::Eval(const char *key, ufString& resultToOwn) {
    int wasFound;
    wasFound = LocalEval(key, resultToOwn);
    if (!wasFound) {
        // failed to find it in InputBlock::eval(), look in global scope
        wasFound = GlobalScope::fgSelf->Eval(key, resultToOwn);
    }
    return wasFound;
}

BOOL InputBlock::LocalEval(const char *k,
                           ufString& resultToOwn) {
    ufStringReadonlyCursor key(k);

    if (key.IsEqual( "Name" )) {
        resultToOwn = fName.Literal();
        return TRUE;
    }
    if (key.IsEqual( "Filename")) {
        resultToOwn = fFilename.Literal();
        return TRUE;
    }
    if (key.IsEqual("Lineno") ) {
        if (!fLinenoBuf) {
            fLinenoBuf.MakeOwner( ConvertToString(fLineno) );
        }
        resultToOwn = fLinenoBuf.Literal();
        return TRUE;
    }
    if (strcmp(key, "TypeCode") == 0) {
        // hardcode this to be 0 (doesn't exist) and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("0");
        return TRUE;
    }
    if (strcmp(key, "TypeString") == 0) {
        // hardcode this to be "N/A" and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("N/A");
        return TRUE;
    }
    if (strcmp(key, "IsType") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("0");
        return TRUE;
    }
    if (strcmp(key, "IsClass") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("0");
        return TRUE;
    }
    if (strcmp(key, "IsEnum") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("0");
        return TRUE;
    }
    if (strcmp(key, "IsAggregate") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("0");
        return TRUE;
    }
    if (strcmp(key, "IsTypeDef") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("0");
        return TRUE;
    }
    if (strcmp(key, "IsArg") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("0");
        return TRUE;
    }
    if (strcmp(key, "IsMethod") == 0) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("0");
        return TRUE;
    }
    if (strcmp(key, "code") == 0) {
        resultToOwn = fUserCode.Literal();
        return TRUE;
    }
    if (strcmp(key, "code.Filename") == 0) {
        if (fUserCodeFilename) {
            resultToOwn = fUserCodeFilename.Literal();
        } else {
            resultToOwn = fFilename;
        }
        return TRUE;
    }
    if (strcmp(key, "code.Lineno") == 0) {
        if (!fUserCodeLinenoBuf) {
            fUserCodeLinenoBuf.MakeOwner( ConvertToString(fUserCodeLineno) );
        }
        resultToOwn = fUserCodeLinenoBuf.Literal();
        return TRUE;
    }

    return EvalKeyValuePairList(fKeyValues, key, resultToOwn);
}

BOOL InputBlock::HasList(const char *k) {
    ufStringReadonlyCursor key(k);
    if (key.IsEqual("annotations")) {
        return TRUE;
    }
    if (key.IsEqual("ALL_CLASSES")) {
        return TRUE;
    }
    if (key.IsEqual("ALL_CONSTANTS")) {
        return TRUE;
    }
    if (key.IsEqual("ALL_TYPES")) {
        return TRUE;
    }
    if (key.IsEqual("key-value-list")) {
        return TRUE;
    }
    if (key.IsEqual("ALL_MODULES")) {
        return TRUE;
    }
    return FALSE;
}

BOOL InputBlock::HasNonEmptyList(const char *k) {
    ufStringReadonlyCursor key(k);
    if (key.IsEqual("annotations")) {
        return fKeyValues;
    }
    if (key.IsEqual("ALL_CLASSES")) {
        return Class::fgClasses != 0;
    }
    if (key.IsEqual("ALL_CONSTANTS")) {
        return GlobalScope::fgConstants != 0;
    }
    if (key.IsEqual("ALL_TYPES")) {
        return Type::fgTypes != 0;
    }
    if (key.IsEqual("key-value-list")) {
        return fKeyValues;
    }
    if (key.IsEqual("ALL_MODULES")) {
        return GlobalScope::fgModules != 0;
    }
    return FALSE;
}

BOOL InputBlock::HasParent(const char *key) {
    if (fName != 0 && fName == key) {
        return TRUE;
    }

    return FALSE;
}

BOOL InputBlock::IsFromCurrentModule() {
    return fFilename == gModuleFullFilename;
}

void InputBlock::AddKeyValue(const char *key, UserCode *value) {
    if (value != 0) {
        ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();

        theKeyValue->fKey = key;
        theKeyValue->fValue.TakeOwnership(value->fCode);
        theKeyValue->fFilename.TakeOwnership(value->fFilename);
        theKeyValue->fLineno = value->fLineno;
        fKeyValues.InsertLast(theKeyValue);
    } else {
        ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();
        theKeyValue->fKey = key;
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

    theKeyValue->fKey = key;
    theKeyValue->fValue = value;
    theKeyValue->fFilename.MakeStringLiteral("");
    theKeyValue->fLineno = 0;
}

BOOL InputBlock::FindKeyValue(const char *key, ufPtr(KeyValuePair) & res) {
    int keyLen = strlen(key);
    for (ufListIter(KeyValuePair) i = fKeyValues; i; i++) {
        if (strcmp(key, i.fData->fKey) == 0) {
            res = i.fData;
            return TRUE;
        }
        int newLen = strlen(i.fData->fKey);

        if (newLen <= keyLen && key[newLen] == '.' &&
            strncmp(key, i.fData->fKey, newLen) == 0) {
            return i.fData->FindKeyValue(&key[newLen + 1], res);
        }
    }

    return FALSE;
}

InputBlockIterator *InputBlock::CreateIterator(const char *k) {
    ufStringReadonlyCursor key(k);
    if (key.IsEqual("annotations")) {
        InputBlockIterator *i = new InputBlockIterator(fKeyValues);
        return i;
    } else if (key.IsEqual("ALL_CLASSES")) {
        InputBlockIterator *i = Class::fgClasses
                                    ? new InputBlockIterator(Class::fgClasses)
                                    : new InputBlockIterator;
        return i;
    } else if (key.IsEqual("ALL_CONSTANTS")) {
        InputBlockIterator *i =
            GlobalScope::fgConstants
                ? new InputBlockIterator(GlobalScope::fgConstants)
                : new InputBlockIterator;
        return i;
    } else if (key.IsEqual("ALL_TYPES")) {
        InputBlockIterator *i = Type::fgTypes
                                    ? new InputBlockIterator(Type::fgTypes)
                                    : new InputBlockIterator;
        return i;
    } else if (key.IsEqual("key-value-list")) {
        InputBlockIterator *i = new InputBlockIterator(fKeyValues);
        return i;
    } else if (key.IsEqual("ALL_MODULES")) {
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

    fName = o.fName;
    fFilename = o.fFilename;
    fLineno = o.fLineno;

    /*
    ufString fLinenoBuf;
    ufList(KeyValuePair) fKeyValues;
    ufString fUserCode;
    ufString fUserCodeFilename;
    int fUserCodeLineno;
    ufString fUserCodeLinenoBuf;
    */

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

const char *KeyValuePair::GetClassName() {
    return "KeyValue";
}

KeyValuePair::KeyValuePair() :
    InputBlock("KeyValue"), fKey(0), fValue(0) {
}

KeyValuePair::KeyValuePair(const KeyValuePair &o)
    : InputBlock(o) {
    ufASSERT(0)
}

KeyValuePair::~KeyValuePair() {
}

void KeyValuePair::PrintDebug() {
    printf("         (key=\"%s\",value=\"%s\")\n", fKey.CStr(), fValue.CStr());
}

BOOL KeyValuePair::Eval(const char *k, ufString& resultToOwn) {
    ufStringReadonlyCursor key(k);
    if (fKey == key && fKey) {
        resultToOwn = fValue.Literal();
        return TRUE;
    }
    if (key && fKey && key.IsEqual("Value")) {
        resultToOwn = fValue.Literal();
        return TRUE;
    }
    if (key && fKey && key.IsEqual("Key")) {
        resultToOwn = fKey.Literal();
        return TRUE;
    }
    if (key && fKey && key.IsEqual("Name")) {
        resultToOwn = fKey.Literal();
        return TRUE;
    }
    // If caller request Type.XXX , find the Type
    // which has the Name equal to the value of this KeyValuePair
    if (key && key.IsEqual("Type.", 5)) {
        ufPtr(Type) theType;

        if (Type::Find(fValue, theType)) {
            theType->Eval(key + 5, resultToOwn);
            return TRUE;
        }

        ufPtr(Class) theClass;
        if (Class::Find(fValue, theClass)) {
            theClass->Eval(key + 5, resultToOwn);
            return TRUE;
        }
        return TRUE;
    }
    // If caller request Class.XXX , find the Class
    // which has the Name equal to the value of this KeyValuePair
    if (key && key.IsEqual("Class.", 6)) {
        ufPtr(Class) theClass;

        if (!Class::Find(fValue, theClass)) {
            return TRUE;
        }
        theClass->Eval(key + 6, resultToOwn);
        return TRUE;
    }
    // failed to find it in KeyValuePair::Eval(), look elsewhere
    return InputBlock::Eval(key, resultToOwn);
}

static int EvalKeyValuePairList(ufList(KeyValuePair)& list,
                                ufStringReadonlyCursor& key, ufString& resultToOwn) {
    int keyLen = strlen(key);
    for (ufListIter(KeyValuePair) i = list; i; i++) {
        if (key.IsEqual(i.fData->fKey)) {
            resultToOwn = i.fData->fValue.Literal();
            return TRUE;
        }
        int newLen = strlen(i.fData->fKey);
        if (newLen <= keyLen && key[newLen] == '.' &&
            key.IsEqual(i.fData->fKey, newLen)) {
            i.fData->Eval(&key[newLen + 1], resultToOwn);
            return TRUE;
        }
    }
    return FALSE;
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
    fFilename.MakeStringLiteral("-builtin-");
    fLineno = 0;

    ufASSERT(gBuiltinTypes[btc].fTypeCode == btc);

    fTypeString.MakeStringLiteral(gBuiltinTypes[btc].fTypeString);
    fTypeCodeString.MakeStringLiteral(gBuiltinTypes[btc].fTypeCodeString);
    fTypeCode = gBuiltinTypes[btc].fTypeCode;

    Register(fSelf);
}

Type::~Type() {
}

BOOL Type::Eval(const char *k, ufString& resultToOwn) {
    ufStringReadonlyCursor key(k);
    if (key == 0) {
        return FALSE;
    }

    if (key.IsEqual("TypeCode")) {
        resultToOwn = fTypeCodeString.Literal();
        return TRUE;
    }
    if (key.IsEqual("TypeString")) {
        if (fTypeString) {
            resultToOwn = fTypeString.Literal();
        } else {
            resultToOwn = fName.Literal();
        }
        return TRUE;
    }
    if (key.IsEqual("IsType")) {
        resultToOwn.MakeStringLiteral("1");
        return TRUE;
    }
    if (key.IsEqual("Header")) {
        if (!fHeader) {
            ufTmpBuf tmp(strlen(gModule) + 20);
            sprintf(tmp.fBuf, "Gen/%s.h", gModule);
            fHeader = tmp.fBuf;
        }
        resultToOwn = fHeader.Literal();
        return TRUE;
    }
    if (key.IsEqual("IsEnum")) {
        resultToOwn.MakeStringLiteral(fIsEnum ? "1" : "0");
        return TRUE;
    }
    if (key.IsEqual("IsAggregate")) {
        resultToOwn.MakeStringLiteral(fIsAggregate ? "1" : "0");
        return TRUE;
    }
    if (key.IsEqual("IsTypeDef")) {
        resultToOwn.MakeStringLiteral(fIsTypeDef ? "1" : "0");
        return TRUE;
    }
    if (key.IsEqual("TypeDefString")) {
        resultToOwn = fTypeDefString.Literal();
        return TRUE;
    }
    if (key.IsEqual("TypeDefString.", 14)) {
        ufPtr(Type) theType;
        key += 14;
        if (!Type::Find(fTypeDefString, theType)) {
            ufPtr(Class) theClass;
            if (Class::Find(fTypeDefString, theClass)) {
                theClass->Eval(key, resultToOwn);
            }
            return TRUE;
        }
        theType->Eval(key, resultToOwn);
        return TRUE;
    }
    if (key.IsEqual("properties.", 11) ||
        key.IsEqual("fields.", 11)) {
        key += 11;
        EvalFieldList(fFields, key, resultToOwn);
        return TRUE;
    }
    if (key.IsEqual("enumentries.", 12)) {
        key += 12;
        EvalKeyValuePairList(fEnumEntries, key, resultToOwn);
        return TRUE;
    }

    // failed to find it in Type::Eval(), look elsewhere
    return InputBlock::Eval(key, resultToOwn);
}

void Type::Register(ufPtr(Type) & theType) {
    fgTypeCatalog.Insert(theType->fName, theType);
    fgTypes.InsertLast(theType);
}

BOOL Type::Find(const char *name, ufPtr(Type) & theType) {
    return fgTypeCatalog.Find(name, theType);
}

BOOL Type::HasList(const char *k) {
    ufStringReadonlyCursor key(k);
    if (key.IsEqual("enumentries")) {
        return fIsEnum ? TRUE : FALSE;
    }
    if (key.IsEqual("properties") ||
        key.IsEqual("fields")) {
        return fIsAggregate ? TRUE : FALSE;
    }
    return InputBlock::HasList(key);
}

BOOL Type::HasNonEmptyList(const char *k) {
    ufStringReadonlyCursor key(k);
    if (key.IsEqual("enumentries")) {
        return fIsEnum && fEnumEntries;
    }
    if (key.IsEqual("properties") ||
        key.IsEqual("fields")) {
        return fIsAggregate && fFields;
    }

    return InputBlock::HasNonEmptyList(key);
}

InputBlockIterator *Type::CreateIterator(const char *k) {
    ufStringReadonlyCursor key(k);
    if (key.IsEqual("enumentries")) {
        InputBlockIterator *i = new InputBlockIterator(fEnumEntries);
        return i;
    }
    if (key.IsEqual("properties") ||
        key.IsEqual("fields")) {
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
           fName.CStr(), fTypeString.CStr(), fTypeCodeString.CStr(),
           fHeader.CStr(), fFilename.CStr(), fLineno);
    for (ufListIter(KeyValuePair) i = fKeyValues; i; i++) {
        i.fData->PrintDebug();
    }
}

void Type::Validate() {
    if (fTypeCode == -1) {
        fTypeCode = gLastCode++;
        fTypeCodeString.MakeOwner( ConvertToString(fTypeCode) );
    }
}

void Type::AddEnumEntry(const char *key) {
    ufPtr(KeyValuePair) theEntry = KeyValuePair::New();
    theEntry->fKey = key;
    theEntry->fValue.MakeStringLiteral("");
    fIsEnum = 1;
    fLastEnumValue++;

    ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();
    theKeyValue->fKey.MakeStringLiteral("CalculatedValue");
    theKeyValue->fValue.MakeOwner(ConvertToString(fLastEnumValue));

    theEntry->fKeyValues.InsertLast(theKeyValue);

    fEnumEntries.InsertLast(theEntry);
}

void Type::AddEnumEntryPair(const char *key, int v) {
    ufTmpBuf buf(40);

    sprintf(buf.fBuf, "= %lu", (unsigned long)v);
    ufPtr(KeyValuePair) theEntry = KeyValuePair::New();
    theEntry->fKey = key;
    theEntry->fValue = buf.fBuf;
    fIsEnum = 1;
    fLastEnumValue = v;

    ufPtr(KeyValuePair) theKeyValue = KeyValuePair::New();
    theKeyValue->fKey.MakeStringLiteral("CalculatedValue");
    theKeyValue->fValue.MakeOwner(ConvertToString(fLastEnumValue));

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
                  typeName, symbolName, fName.CStr()));
        return;
    }

    if (FindField(symbolName, theField)) {
        MOCERROR((errmsg,
                  "The field \"%s\" already exists "
                  "in class \"%s\" ",
                  symbolName, fName.CStr()));
        return;
    }

    theField = Field::New(theType, symbolName);
    theCurrentField = theField;
    fFields.InsertLast(theField);
    fIsAggregate = TRUE;
}

BOOL Type::FindField(const char *symbolName, ufPtr(Field) & theField) {
    for (ufListIter(Field) j = fFields; j; j++) {
        if (strcmp(j.fData->fName, symbolName) == 0) {
            theField = j.fData;
            return TRUE;
        }
    }
    return FALSE;
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
      fArrayLength(-1), fArrayLengthString(0), fSelf(this, kSelfPtrShared)
    {
}

Field::Field(const Field &o)
    : InputBlock(o){
    ufASSERT(0)
}

Field::~Field() {
}

void Field::Validate() {
    if (fTypeCode == -1) {
        fTypeCode = gLastCode++;
        fTypeCodeString.MakeOwner(ConvertToString(fTypeCode));
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
    fTypeCodeString = o.fTypeCodeString;
    fArrayLengthString = o.fArrayLengthString;

    return *this;
}

void Field::PrintDebug() {
    printf("    Field={ \n"
           "      fName=\"%s\"\n"
           "      fTypeCodeString=\"%s\"\n"
           "      fFilename=\"%s\"\n"
           "      fLineno=%d\n"
           "      fType=\n",
           fName.CStr(), fTypeCodeString.CStr(), fFilename.CStr(), fLineno);
    fType->PrintDebug();
}

BOOL Field::Eval(const char *k, ufString& resultToOwn) {
    ufStringReadonlyCursor key(k);
    if (key == 0) {
        return FALSE;
    }
    if (key.IsEqual("TypeCode")) {
        resultToOwn = fTypeCodeString.Literal();
        return TRUE;
    }
    if (key.IsEqual("Type.", 5)) {
        fType->Eval(key + 5, resultToOwn);
        return TRUE;
    }
    if (key.IsEqual("ArrayLength")) {
        if (fArrayLength <= 0) {
            return FALSE;
        }
        if (!fArrayLengthString) {
            fArrayLengthString.MakeOwner( ConvertToString(fArrayLength) );
        }
        resultToOwn = fArrayLengthString.Literal();
        return TRUE;
    }

    // failed to find it in Field::Eval(), look elsewhere
    return InputBlock::Eval(key, resultToOwn);
}

static int EvalFieldList(ufList(Field)& list,
                         ufStringReadonlyCursor& key, ufString& resultToOwn) {
    for (ufListIter(Field) i = list; i; i++) {
        if (key.IsEqual(i.fData->fName)) {
            resultToOwn = i.fData->fName.Literal();
            return TRUE;
        }
    }
    return FALSE;
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
    : InputBlock(o) {
    ufASSERT(0)
}

Arg::~Arg() {
}

void Arg::PrintDebug() {
    printf("      Arg={ \n"
           "        fName=\"%s\"\n",
           fName.CStr());
    if (fType.isValid()) {
        fType->PrintDebug();
    }
    if (fClass.isValid()) {
        fClass->PrintDebug();
    }

    printf("      }\n");
}

BOOL Arg::HasParent(const char *key) {
    if (fClass.isValid()) {
        return fClass->HasParent(key);
    }
    if (fType.isValid()) {
        return fType->HasParent(key);
    }

    return InputBlock::HasParent(key);
}

BOOL Arg::Eval(const char *k, ufString& resultToOwn) {
    ufStringReadonlyCursor key(k);
    if (key == 0) {
        return FALSE;
    }

    if (key.IsEqual("IsArg")) {
        resultToOwn.MakeStringLiteral("1");
        return TRUE;
    }
    if (key.IsEqual("Type.", 5)) {
        key += 5;
        if (fType.isValid()) {
            fType->Eval(key, resultToOwn);
        } else if (fClass.isValid()) {
            fClass->Eval(key, resultToOwn);
        }
        return TRUE;
    }

    // failed to find it in Arg::Eval(), look elsewhere
    return InputBlock::Eval(key, resultToOwn);
}

static int EvalArgList(ufList(Arg)& list,
                       ufStringReadonlyCursor& key, ufString& resultToOwn) {
    int keyLen = strlen(key);
    for (ufListIter(Arg) i = list; i; i++) {
        if (key.IsEqual(i.fData->fName)) {
            resultToOwn = i.fData->fName.Literal();
            return TRUE;
        }
        int newLen = strlen(i.fData->fName);
        if (newLen < keyLen && key[newLen] == '.') {
            i.fData->Eval(key + newLen, resultToOwn);
            return TRUE;
        }
    }
    return FALSE;
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
      fSelf(this, kSelfPtrShared) {
}

Operation::Operation(const Operation &o)
    : InputBlock(o) {
    ufASSERT(0)
}

Operation::~Operation() {
}

void Operation::Validate() {
    if (fTypeCode == -1) {
        fTypeCode = gLastCode++;
        fTypeCodeString.MakeOwner(ConvertToString(fTypeCode));
    }
}

void Operation::PrintDebug() {
    printf("    Operation={ \n"
           "      fName=\"%s\"\n"
           "      fTypeCodeString=\"%s\"\n"
           "      fFilename=\"%s\"\n"
           "      fLineno=%d\n"
           "      fInArguments = \n",
           fName.CStr(), fTypeCodeString.CStr(), fFilename.CStr(), fLineno);
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


BOOL Operation::Eval(const char *k, ufString& resultToOwn) {
    ufStringReadonlyCursor key(k);
    if (key == 0) {
        return FALSE;
    }

    if (key.IsEqual("IsMethod")) {
        // hardcode this to be 0 and let
        // subclasses override the value if they need to do so.
        resultToOwn.MakeStringLiteral("1");
        return TRUE;
    }
    if (key.IsEqual("TypeCode")) {
        resultToOwn = fTypeCodeString.Literal();
        return TRUE;
    }
    if (key.IsEqual("in-arguments.", 13)) {
        key += 13;
        EvalArgList(fInArguments, key, resultToOwn);
        return TRUE;
    }
    if (key.IsEqual("out-arguments.", 14)) {
        key += 14;
        EvalArgList(fOutArguments, key, resultToOwn);
        return TRUE;
    }
    if (key.IsEqual("error-arguments.", 16)) {
        key += 16;
        EvalArgList(fErrorArguments, key, resultToOwn);
        return TRUE;
    }

    // failed to find it in Operation::Eval(), look elsewhere
    return InputBlock::Eval(key, resultToOwn);
}

BOOL Operation::HasList(const char *k) {
    ufStringReadonlyCursor key(k);
    if (key.IsEqual("in-arguments")) {
        return TRUE;
    } else if (key.IsEqual("out-arguments")) {
        return TRUE;
    } else if (key.IsEqual("error-arguments")) {
        return TRUE;
    }

    return InputBlock::HasList(key);
}

BOOL Operation::HasNonEmptyList(const char *key) {
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

BOOL Operation::MakeArgument(const char *typeName, const char *symbolName,
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
                  typeName, symbolName, fName.CStr()));
        return FALSE;
    }

    return TRUE;
}

Operation &Operation::operator=(const Operation &o) {
    if (this == &o) {
        return *this;
    }

    InputBlock::operator=(o);

    fAssocClass = o.fAssocClass;
    fTypeCode = o.fTypeCode;
    fTypeCodeString = o.fTypeCodeString;

    return *this;
}

static int EvalOperationList(ufList(Operation)& list,
                             ufStringReadonlyCursor& key, ufString& resultToOwn) {
    for (ufListIter(Operation) i = list; i; i++) {
        if (key.IsEqual(i.fData->fName)) {
            resultToOwn = i.fData->fName.Literal();
            return TRUE;
        }
    }
    return FALSE;
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
    : InputBlock(o) {
    ufASSERT(0)
}

Class::~Class() {
}

void Class::Register(ufPtr(Class) & theClass) {
    fgClassCatalog.Insert(theClass->fName, theClass);
    fgClasses.InsertLast(theClass);
}

BOOL Class::Find(const char *name, ufPtr(Class) & theClass) {
    return fgClassCatalog.Find(name, theClass);
}

BOOL Class::FindOp(const char *n, ufPtr(Operation) & theOp) {
    ufStringReadonlyCursor name(n);
    ufListIter(Operation) i;
    for (i = fOperations; i; i++) {
        if (name.IsEqual(i.fData->fName)) {
            theOp = i.fData;
            return TRUE;
        }
    }
    for (i = fInheritedOperations; i; i++) {
        if (name.IsEqual(i.fData->fName)) {
            theOp = i.fData;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL Class::HasParent(const char *k) {
    ufStringReadonlyCursor key(k);
    if (key.IsEqual("Interface")) {
        return fIsInterface ? TRUE : FALSE;
    }
    if (key.IsEqual(fName)) {
        return TRUE;
    }

    for (ufListIter(Class) i = fParents; i; i++) {
        if (key.IsEqual(i.fData->fName) || i.fData->HasParent(key)) {
            return TRUE;
        }
    }

    return FALSE;
}

void Class::AddParent(const char *name) {
    ufPtr(Class) theParent;
    if (Class::Find(name, theParent)) {
        fParents.InsertLast(theParent);
    } else {
        MOCERROR((errmsg,
                  "Can't find parent \"%s\" for "
                  "class \"%s\"",
                  name, fName.CStr()));
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
                  fName.CStr()));
        exit(kBadSchema);
    }

    fIsBusy = 1;

    for (ufListIter(Class) i = fParents; i; i++) {
        i.fData->Validate();
    }

    if (fTypeCode == -1) {
        fTypeCode = gLastCode++;
        fTypeCodeString.MakeOwner(ConvertToString(fTypeCode));
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
           fName.CStr(), fTypeCodeString.CStr(), fFilename.CStr(), fLineno);
    for (ufListIter(Class) i = fParents; i; i++) {
        printf("    \"%s\"\n", i.fData->fName.CStr());
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

BOOL Class::Eval(const char *k, ufString& resultToOwn) {
    ufStringReadonlyCursor key(k);
    if (key == 0) {
        return FALSE;
    }

    if (key.IsEqual("TypeCode")) {
        resultToOwn = fTypeCodeString.Literal();
        return TRUE;
    }
    if (key.IsEqual("TypeString")) {
        resultToOwn = fName.Literal();
        return TRUE;
    }
    if (key.IsEqual("IsClass")) {
        resultToOwn.MakeStringLiteral("1");
        return TRUE;
    }
    if (key.IsEqual("IsAbstract")) {
        resultToOwn.MakeStringLiteral( fIsAbstract ? "1" : "0" );
        return TRUE;
    }

    if (key.IsEqual("properties.", 11) ||
        key.IsEqual("fields.", 11)) {
        key += 11;
        EvalFieldList(fFields, key, resultToOwn);
        return TRUE;
    }
    if (key.IsEqual("inherited-properties.", 21)) {
        key += 21;
        EvalFieldList(fInheritedFields, key, resultToOwn);
        return TRUE;
    } else if (key.IsEqual("inherited-fields.", 17)) {
        key += 17;
        EvalFieldList(fInheritedFields, key, resultToOwn);
        return TRUE;
    }
    if (key.IsEqual("operations.", 11)) {
        key += 11;
        EvalOperationList(fOperations, key, resultToOwn);
        return TRUE;
    } else if (key.IsEqual("methods.", 8)) {
        key += 8;
        EvalOperationList(fOperations, key, resultToOwn);
        return TRUE;
    }
    if (key.IsEqual("inherited-operations.", 21)) {
        key += 21;
        EvalOperationList(fInheritedOperations, key, resultToOwn);
        return TRUE;
    } else if (key.IsEqual("inherited-methods.", 18)) {
        key += 18;
        EvalOperationList(fInheritedOperations, key, resultToOwn);
        return TRUE;
    }
    if (key.IsEqual("parents.", 8)) {
        key += 8;
        return EvalClassList(fParents, key, resultToOwn);
    }
    if (key.IsEqual("all-parents.", 12)) {
        key += 12;
        return EvalClassList(fAllParents, key, resultToOwn);
    }
    if (key.IsEqual("IsAggregate")) {
        resultToOwn.MakeStringLiteral("1");
        return TRUE;
    }

    // failed to find it in Class::Eval(), look elsewhere
    int wasFound = InputBlock::Eval(key, resultToOwn);
    if (wasFound) {
        return TRUE;
    }

    // see if any superclass has the key
    for (ufListIter(Class) i = fParents; i; i++) {
        if (i.fData->Eval(key,resultToOwn)) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL Class::HasList(const char *key) {
    if (strcmp(key, "properties") == 0) {
        return TRUE;
    } else if (strcmp(key, "inherited-properties") == 0) {
        return TRUE;
    } else if (strcmp(key, "methods") == 0) {
        return TRUE;
    } else if (strcmp(key, "inherited-methods") == 0) {
        return TRUE;
    } else if (strcmp(key, "parents") == 0) {
        return TRUE;
    } else if (strcmp(key, "all-parents") == 0) {
        return TRUE;
    } else if (strcmp(key, "fields") == 0) {
        return TRUE;
    } else if (strcmp(key, "inherited-fields") == 0) {
        return TRUE;
    } else if (strcmp(key, "operations") == 0) {
        return TRUE;
    } else if (strcmp(key, "inherited-operations") == 0) {
        return TRUE;
    }

    return InputBlock::HasList(key);
}

BOOL Class::HasNonEmptyList(const char *key) {
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
                  typeName, symbolName, fName.CStr()));
        return;
    }

    if (FindField(symbolName, theField)) {
        MOCERROR((errmsg,
                  "The field \"%s\" already exists "
                  "in class \"%s\" ",
                  symbolName, fName.CStr()));
        return;
    }

    theField = Field::New(theType, symbolName);
    theCurrentField = theField;
    fFields.InsertLast(theField);
}

BOOL Class::FindField(const char *symbolName, ufPtr(Field) & theField) {
    for (ufListIter(Field) j = fFields; j; j++) {
        if (j.fData->fName == symbolName) {
            theField = j.fData;
            return TRUE;
        }
    }
    for (ufListIter(Class) i = fParents; i; i++) {
        if (i.fData->FindField(symbolName, theField)) {
            return TRUE;
        }
    }
    return FALSE;
}

void Class::AddInheritedField(ufPtr(Field) & theField) {
    int found = 0;
    for (ufListIter(Field) i = fFields; i && !found; i++) {
        if (i.fData->fName == theField->fName) {
            found = 1;
            break;
        }
    }
    for (ufListIter(Field) j = fInheritedFields; j && !found; j++) {
        if (j.fData->fName == theField->fName) {
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
        if (i.fData->fName == theOp->fName) {
            found = 1;
            break;
        }
    }
    for (ufListIter(Operation) j = fInheritedOperations; j && !found; j++) {
        if (j.fData->fName == theOp->fName) {
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
        if (i.fData->fName == theClass->fName) {
            found = 1;
            break;
        }
    }
    if (!found) {
        fAllParents.InsertLast(theClass);
    }
}

static int EvalClassList(ufList(Class)& list,
                         ufStringReadonlyCursor& key, ufString& resultToOwn) {
    for (ufListIter(Class) i = list; i; i++) {
        if (key.IsEqual(i.fData->fName)) {
            resultToOwn = i.fData->fName.Literal();
            return TRUE;
        }
    }
    return FALSE;
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

static int KeyValueEval(ufStringReadonlyCursor& key,
                        const char* keyValueName,
                        ufPtr(KeyValuePair)& keyValue,
                        ufString& resultToOwn) {
    int len = strlen(keyValueName);
    int keyLen = strlen(key);

    if (!keyValue) {
        return FALSE;
    }
    if (!key.IsEqual(keyValueName, len)) {
        return FALSE;
    }

    if (keyLen == len) {
        resultToOwn = keyValue->fValue.Literal();
        return TRUE;
    }

    if (key[len] == '.') {
        return keyValue->Eval(key + len + 1, resultToOwn);
    }

    return TRUE;
}

GlobalScope::GlobalScope(const GlobalScope &o)
    : InputBlock(o){ufASSERT(0)}

      GlobalScope::~GlobalScope() {}

BOOL GlobalScope::Eval(const char *k, ufString& resultToOwn) {
    ufStringReadonlyCursor key(k);
    if (key.IsEqual("MODULE")) {
        resultToOwn.MakeStringLiteral(gModule ? gModule : "");
        return TRUE;
    }
    const char* result=0;
    if (KeyValueEval(key, "HEADER_CODE", GlobalScope::fgHeaderCode,
                     resultToOwn)) {
        return TRUE;
    }
    if (KeyValueEval(key, "HEADER_END_CODE", GlobalScope::fgHeaderEndCode,
                     resultToOwn)) {
        return TRUE;
    }
    if (KeyValueEval(key, "SOURCE_CODE", GlobalScope::fgSourceCode,
                     resultToOwn)) {
        return TRUE;
    }

    int wasFound = InputBlock::LocalEval(key, resultToOwn);
    if (wasFound) {
        return TRUE;
    }

    ufPtr(KeyValuePair) theVar;
    if (GlobalScope::fgVariableCatalog &&
        GlobalScope::fgVariableCatalog.Find(key, theVar)) {
        resultToOwn = theVar->fValue.Literal();
        return TRUE;
    }

    return FALSE;
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

BOOL InputBlockIterator::IsFirst() { return fCount == 1; }

BOOL InputBlockIterator::IsLast() { return fCount >= GetTotal(); }

BOOL InputBlockIterator::IsExhausted() { return fCount > GetTotal(); }

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
        tmp[i++] = c;
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
    tmp[i] = tmp[i + 1] = '\0';
    return i > 0 ? tmp.fBuf : 0;
}

static char *ConvertToString(int l) {
    char *s;
    char buf[20];
    sprintf(buf, "%d", l);
    s = ufStrDup(buf);
    return s;
}

void CleanupInput() {
    Type::fgTypeCatalog.MakeEmpty();
    Type::fgTypes.MakeEmpty();
    Class::fgClassCatalog.MakeEmpty();
    Class::fgClasses.MakeEmpty();
    GlobalScope::fgHeaderCode.MakeEmpty();
    GlobalScope::fgHeaderEndCode.MakeEmpty();
    GlobalScope::fgSourceCode.MakeEmpty();
    GlobalScope::fgConstantCatalog.MakeEmpty();
    GlobalScope::fgConstants.MakeEmpty();
    GlobalScope::fgVariableCatalog.MakeEmpty();
    GlobalScope::fgVariables.MakeEmpty();

    GlobalScope::fgModuleCatalog.MakeEmpty();
    GlobalScope::fgModules.MakeEmpty();

    GlobalScope::fgSelf.MakeEmpty();
}
