/*
 * See Readme, Notice and License files.
 */
#ifndef _INPUT_H
#define _INPUT_H

#include <stdio.h>

#include "ufList.h"
#include "ufPtr.h"
#include "ufString.h"
#include "ufSHshT.h"

const int kMaxSubiterators = 10;

const int kStringTypeCode = 1;
const int kIntegerTypeCode = 2;
const int kBooleanTypeCode = 3;
const int kRealTypeCode = 4;
const int kInvalidTypeCode = -1;

const int kTypeInputBlock = 100;
const int kFieldInputBlock = 101;
const int kOperationInputBlock = 102;
const int kClassInputBlock = 103;
const int kKeyValueInputBlock = 104;
const int kArgInputBlock = 105;
const int kGlobalScopeInputBlock = 103;

class GlobalScope;
class ufPtr(GlobalScope);
declareclass(ufPtr, GlobalScope);

class Class;
class ufPtr(Class);
class ufList(Class);
class ufStrHash(Class);
declareclass(ufPtr, Class);
declare(ufList, Class);
class ufListIter(Class);
declare(ufStrHash, Class);

class Type;
class ufPtr(Type);
class ufList(Type);
class ufStrHash(Type);
declareclass(ufPtr, Type);
declare(ufList, Type);
declare(ufStrHash, Type);

class Field;
class ufList(Field);
class ufStrHash(Field);
declareclass(ufPtr, Field);
declare(ufList, Field);
declare(ufStrHash, Field);

class KeyValuePair;
class ufPtr(KeyValuePair);
class ufList(KeyValuePair);
class ufStrHash(KeyValuePair);
declareclass(ufPtr, KeyValuePair);
declare(ufList, KeyValuePair);
declare(ufStrHash, KeyValuePair);

class Operation;
class ufList(Operation);
class ufStrHash(Operation);
declareclass(ufPtr, Operation);
declare(ufList, Operation);
declare(ufStrHash, Operation);

class Arg;
class ufPtr(Arg);
class ufList(Arg);
declareclass(ufPtr, Arg);
declare(ufList, Arg);

class InputBlock;
declareclass(ufPtr, InputBlock);
struct UserCode;
class InputBlockIterator {
public:
    InputBlockIterator();
    InputBlockIterator(ufList(Class) &);
    InputBlockIterator(ufList(Type) &);
    InputBlockIterator(ufList(Field) &);
    InputBlockIterator(ufList(Operation) &);
    InputBlockIterator(ufList(Arg) &);
    InputBlockIterator(ufList(KeyValuePair) &);
    virtual ~InputBlockIterator();

    virtual BOOL IsFirst();
    virtual BOOL IsLast();
    virtual BOOL IsExhausted();
    virtual InputBlockIterator &operator++();
    virtual InputBlockIterator &operator++(int);
    virtual operator PTR_INT() const;

    ufPtr(InputBlock) fData;
    virtual int GetTotal() const;
    virtual void DataChanged();

protected:
    int fCount;
    int fTotal;
    ufListIter(Class) * fClasses;
    ufListIter(Type) * fTypes;
    ufListIter(Field) * fFields;
    ufListIter(Operation) * fOperations;
    ufListIter(Arg) * fArgs;
    ufListIter(KeyValuePair) * fKeyValues;
};

class MultiListInputBlockIterator : public InputBlockIterator {
public:
    MultiListInputBlockIterator();
    ~MultiListInputBlockIterator();

    void AddSubiterator(InputBlockIterator *);
    virtual int GetTotal() const;

    virtual InputBlockIterator &operator++();
    virtual InputBlockIterator &operator++(int);

protected:
    int fNumSubiterators;
    int fCurrentSubiterator;
    InputBlockIterator *fSubiterators[kMaxSubiterators];
};

class InputBlock {
public:
    virtual BOOL IsA(int t) { return 0; }
    virtual const char *GetClassName();
    virtual BOOL Eval(const char *key, ufString& resultToOwn);
    virtual BOOL HasList(const char *key);
    virtual BOOL HasNonEmptyList(const char *key);
    virtual BOOL IsFromCurrentModule();
    virtual BOOL HasParent(const char *key);
    virtual InputBlockIterator *CreateIterator(const char *key);
    void AddKeyValue(const char* key, UserCode * value);
    void SetKeyValue(const char *key, const char* value);

    virtual void PrintDebug(){};
    InputBlock &operator=(const InputBlock &);

    ufString fFilename;
    int fLineno;
    ufString fLinenoBuf;
    ufString fName;
    ufList(KeyValuePair) fKeyValues;
    ufString fUserCode;
    ufString fUserCodeFilename;
    int fUserCodeLineno;
    ufString fUserCodeLinenoBuf;

protected:
    friend class ufPtr(InputBlock);
    InputBlock(const InputBlock &o);
    InputBlock(const char *name);
    virtual ~InputBlock();

    BOOL FindKeyValue(const char *key, ufPtr(KeyValuePair) &);
    BOOL LocalEval(const char *key, ufString& resultToOwn);
};
declareinlines(ufPtr, InputBlock);

class KeyValuePair : public InputBlock {
public:
    static ufPtr(KeyValuePair) New();

    virtual const char *GetClassName();
    virtual BOOL Eval(const char *key, ufString& resultToOwn);
    virtual BOOL IsA(int t) { return t == kKeyValueInputBlock; }
    void PrintDebug();

    ufString fKey;
    ufString fValue;

protected:
    friend class ufPtr(KeyValuePair);

    KeyValuePair();
    KeyValuePair(const KeyValuePair &);
    ~KeyValuePair();
};
declareinlines(ufPtr, KeyValuePair);

class Type : public InputBlock {
public:
    static ufPtr(Type) New(const char *name);
    static ufPtr(Type) New(int btc);

    static void Register(ufPtr(Type) & a);
    static BOOL Find(const char *name, ufPtr(Type) & a);

    virtual const char *GetClassName();
    virtual BOOL Eval(const char *key, ufString& resultToOwn);
    virtual BOOL IsA(int t) { return t == kTypeInputBlock; }
    void Validate();
    void PrintDebug();
    virtual BOOL HasList(const char *key);
    virtual BOOL HasNonEmptyList(const char *key);
    virtual InputBlockIterator *CreateIterator(const char *key);

    void AddEnumEntry(const char* name);
    void AddEnumEntryPair(const char* key, int value);
    void AddField(const char* typeName, const char* symbolName);

    BOOL FindField(const char *symbolNm, ufPtr(Field) &);

    ufList(Field) fFields;
    ufString fTypeDefString;
    ufString fTypeString;
    ufString fHeader;
    int fTypeCode;
    ufString fTypeCodeString;

    int fIsEnum : 1;
    int fIsTypeDef : 1;
    int fIsAggregate : 1;
    ufList(KeyValuePair) fEnumEntries;
    int fLastEnumValue;

    static ufStrHash(Type) fgTypeCatalog;
    static ufList(Type) fgTypes;

protected:
    friend class ufPtr(Type);
    Type(const char *name);
    Type(const Type &);
    Type(int btc);
    ~Type();

    ufPtr(Type) fSelf;
};

declareinlines(ufPtr, Type);

class Field : public InputBlock {
public:
    static ufPtr(Field) New(ufPtr(Type) & t, const char *n);

    virtual const char *GetClassName();
    virtual BOOL Eval(const char *key, ufString& resultToOwn);
    virtual BOOL IsA(int t) { return t == kFieldInputBlock; }
    void Validate();
    void PrintDebug();

    ufPtr(Type) fType;
    int fTypeCode;
    ufString fTypeCodeString;
    int fArrayLength;
    ufString fArrayLengthString;

    Field &operator=(const Field &);

protected:
    friend class ufPtr(Field);
    Field(ufPtr(Type) & t, const char *n);
    Field(const Field &);
    ~Field();

    ufPtr(Field) fSelf;
};

declareinlines(ufPtr, Field);

class Arg : public InputBlock {
public:
    static ufPtr(Arg) New(ufPtr(Type) & t, const char *n);
    static ufPtr(Arg) New(ufPtr(Class) & t, const char *n);
    virtual const char *GetClassName();
    virtual BOOL Eval(const char *key, ufString& resultToOwn);
    virtual BOOL HasParent(const char *key);
    virtual BOOL IsA(int t) { return t == kArgInputBlock; }
    void PrintDebug();

    ufPtr(Type) fType;
    ufPtr(Class) fClass;

protected:
    friend class ufPtr(Arg);

    Arg(ufPtr(Type) & t, const char *n);
    Arg(ufPtr(Class) & t, const char *n);
    Arg(const Arg &);
    ~Arg();
};

declareinlines(ufPtr, Arg);

class Operation : public InputBlock {
public:
    static ufPtr(Operation) New(const char *n);

    virtual const char *GetClassName();
    virtual BOOL HasList(const char *key);
    virtual BOOL HasNonEmptyList(const char *key);
    virtual InputBlockIterator *CreateIterator(const char *key);
    virtual BOOL Eval(const char *key, ufString& resultToOwn);
    virtual BOOL IsA(int t) { return t == kOperationInputBlock; }
    void Validate();
    void AddInArgument(const char* typeName, const char* symbolName);
    void AddOutArgument(const char* typeName, const char* symbolName);
    void AddErrorArgument(const char* typeName, const char* symbolName);
    void PrintDebug();

    ufPtr(Class) fAssocClass;
    int fTypeCode;
    ufString fTypeCodeString;
    ufList(Arg) fInArguments;
    ufList(Arg) fOutArguments;
    ufList(Arg) fErrorArguments;

    Operation &operator=(const Operation &);

protected:
    friend class ufPtr(Operation);
    Operation(const char *n);
    Operation(const Operation &);
    ~Operation();
    BOOL MakeArgument(const char *typeName, const char *name, ufPtr(Arg) &);

    ufPtr(Operation) fSelf;
};

declareinlines(ufPtr, Operation);

class Class : public InputBlock {
public:
    static ufPtr(Class) New(const char *name);

    static void Register(ufPtr(Class) & a);
    static BOOL Find(const char *name, ufPtr(Class) & a);
    BOOL FindOp(const char *name, ufPtr(Operation) & a);
    virtual const char *GetClassName();
    virtual BOOL HasParent(const char *key);
    virtual BOOL HasList(const char *key);
    virtual BOOL HasNonEmptyList(const char *key);
    virtual InputBlockIterator *CreateIterator(const char *key);
    virtual BOOL Eval(const char *key, ufString& resultToOwn);
    virtual BOOL IsA(int t) { return t == kClassInputBlock; }
    void AddChild(ufPtr(Class) & child);
    void AddInheritedToChild(ufPtr(Class) & child);
    static void ValidateAll();
    void Validate();
    void AddParent(const char *name);
    void AddOperation(const char* name);
    void AddField(const char* typeName, const char* symbolName);
    BOOL FindField(const char *symbolNm, ufPtr(Field) &);
    void PrintDebug();

    int fTypeCode;
    ufString fTypeCodeString;
    int fIsAbstract : 1;
    int fIsBusy : 1;
    int fIsValidated : 1;
    int fIsInterface : 1;

    ufList(Class) fParents;
    ufList(Class) fAllParents;
    ufList(Class) fChildren;
    ufList(Operation) fOperations;
    ufList(Operation) fInheritedOperations;
    ufList(Field) fFields;
    ufList(Field) fInheritedFields;

    static ufStrHash(Class) fgClassCatalog;
    static ufList(Class) fgClasses;

protected:
    friend class ufPtr(Class);
    Class(const char *name);
    Class(const Class &);
    ~Class();

    void AddInheritedField(ufPtr(Field) & a);
    void AddInheritedOperation(ufPtr(Operation) & a);
    void AddInheritedParent(ufPtr(Class) & c);

    ufPtr(Class) fSelf;
};

declareinlines(ufPtr, Class);

class GlobalScope : public InputBlock {
public:
    static ufPtr(InputBlock) New();

    virtual const char *GetClassName();
    virtual BOOL IsA(int t) { return t == kGlobalScopeInputBlock; }

    virtual BOOL Eval(const char *key, ufString& resultToOwn);

    static ufPtr(KeyValuePair) fgHeaderCode;
    static ufPtr(KeyValuePair) fgHeaderEndCode;
    static ufPtr(KeyValuePair) fgSourceCode;

    static ufStrHash(KeyValuePair) fgConstantCatalog;
    static ufList(KeyValuePair) fgConstants;
    static ufStrHash(KeyValuePair) fgVariableCatalog;
    static ufList(KeyValuePair) fgVariables;

    static ufStrHash(KeyValuePair) fgModuleCatalog;
    static ufList(KeyValuePair) fgModules;

    static ufPtr(InputBlock) fgSelf;

protected:
    friend class ufPtr(GlobalScope);
    GlobalScope();
    ~GlobalScope();
    GlobalScope(const GlobalScope &);
};
declareinlines(ufPtr, GlobalScope);

extern char *FGets(ufTmpBuf &tmp, FILE *in);

#endif
