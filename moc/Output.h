/*
 * See Readme, Notice and License files.
 */
#ifndef _OUTPUT_H
#define _OUTPUT_H
#include "Input.h"
#include "ufList.h"
#include "ufPtr.h"
#include "ufSHshT.h"
#include "ufString.h"

class ufTmpBuf;

class OutputBlock;
declareclass(ufPtr, OutputBlock);
declare(ufList, OutputBlock);

class Sequence;
declareclass(ufPtr, Sequence);
declare(ufList, Sequence);
class SequenceEntry;
class ufPtr(SequenceEntry);
class ufList(SequenceEntry);
class ufStrHash(SequenceEntry);
declareclass(ufPtr, SequenceEntry);
declare(ufList, SequenceEntry);
declare(ufStrHash, SequenceEntry);
class SubsequenceRange;
declareclass(ufPtr, SubsequenceRange);
declare(ufList, SubsequenceRange);
declare(ufStrHash, SubsequenceRange);

// template control statements for the blocks look like
// NORMAL STYLE COMMAND BLOCK:
// @{ <command> - starts a block. The <command> identifies
//                the kind of block.
// @| <additional directives for the current block.
// @}    - closes a block.
//
// ERROR/WARNING DIRECTIVE:
// @> {error|warning} <error statement>
//
// EXAMPLES :
// The following is some examples of the normal style command
// blocks.
//
// EXAMPLE OPEN FILE COMMAND BLOCK:
// @{ open @{class}Imp.C
// @}
//
// EXAMPLE FOREACH COMMAND BLOCK:
// @{ foreach <variable name> in <variable-list>
// @}
//
// EXAMPLE IF COMMAND BLOCK:
// @{ if <if-expression>
// @| else if <if-expression>
// @| else
// @}
// where the <if-expression> is made up of &&s and ||s of
// basic expressions of the form
// {exists|is-a|empty|is-from-current-module|first|last} <variable>
//
// VARIABLE SUBSTITUTION
//  Any sequence @(<string>) represents a variable substitution.
//  It may be a formatted or unformatted variable substitution.
//  Both are explained below:
//
// UNFORMATTED VARIABLE SUBSTITUTION
//  The unformatted variable substitution is of the form
//   @( [<variable name>::] <key>)
//   Where key specifies the Name of the object, the Lineno, ...
//   The variable name is optional and is a foreach iterator variable,
//   'file' or '' for global scope.
//
// FORMATTED VARIABLE SUBSTITUTION
//  The formatted substitution is an extension of the unformatted
//  variable substitution.
//  Formatted substitution is similiar to unformatted except
//  that before the [<variable name>::]<key> is a format specifier
//   @(:<format-specifier> <formatter arguments> )
//
//  The formatter arguments vary depending on the formatter that
//  is used. The standard formatters are:
//    format-list
//    length (or len)
//    uppercase-prefix (or upp)
//    uppercase-all (or up)
//    uppercase-first (or up1)
//    lowercase-all (or low)
//    generate-sequence-number (or g)
//    fill (or f)
//    if
//    ? - ( format is :? <variable>:<value-if-variable-not-set> )
//    nth - the nth element in a list. format is :nth <index> <list> <variable>
//  See existing templates for the arguments to these formatter
//  functions.
//

// for the IsA member function
const int kLineBlock = 1;
const int kBasicBlock = 2;
const int kIfCaseBlock = 3;
const int kIfBlock = 4;
const int kLoopBlock = 5;
const int kFileBlock = 6;
const int kControlBlock = 7;

enum MessageOutputLevel {
    kNormalMessage = 0,
    kWarningMessage = 1,
    kErrorMessage = 2
};
typedef void (*WordFormatter)(ufTmpBuf& dest,
                              int wordCounter,
                              ufTmpBuf& src);
typedef bool (*WordTokenizer)(const char *&var,
                              int wordCounter,
                              ufTmpBuf& tmp);

class OutputBlock {
public:
    virtual int IsA(int);
    virtual void SetInput(ufPtr(InputBlock) & p);
    virtual void GenerateOutput() = 0;
    virtual int CanOutput();
    virtual FILE *GetOutputFile();
    virtual void AddBlock(ufPtr(OutputBlock) & blk);
    virtual void AddDirective(const char *directive);
    virtual void EndBlock(const char *);
    virtual const char *Eval(const char *var, char *&strToDel);
    virtual int IsFirst(const char *var);
    virtual int IsLast(const char *var);
    virtual int IsFromCurrentModule(char *);
    virtual int HasEmptyList(const char *var);
    virtual int HasParent(const char *var);
    const char *EvalFormatList(const char *var, char *&strToDel);
    const char *EvalUpperCasePrefix(const char *var, char *&strToDel);
    const char *EvalUpperCaseFirst(const char *var, char *&strToDel);
    const char *EvalUpperCaseAll(const char *var, char *&strToDel);
    const char *EvalLowerCaseAll(const char *var, char *&strToDel);
    const char *EvalGenerateSequenceNumber(const char *var, char *&strToDel);
    const char *EvalColumnFill(const char *var, char *&strToDel);
    const char *EvalIf(const char *var, char *&strToDel);
    const char *EvalDefaultSubst(const char *var, char *&strToDel);
    const char *EvalNth(const char *var, char *&strToDel);
    const char *EvalLength(const char *var, char *&strToDel);
    const char *EvalSnakeCase(const char *var, char *&strToDel);
    const char *EvalCamelCase(const char *var, char *&strToDel);
    const char *EvalPascalCase(const char *var, char *&strToDel);
    virtual int Resolve(const char *scopedName, ufPtr(InputBlock) &);

    int DoesExist(char *);
    int SetKeyValue(char *, char *);
    void Error(const char *msg);
    void Warning(const char *msg);

    static ufPtr(OutputBlock) fgTop;

    ufPtr(OutputBlock) fParent;
    ufPtr(InputBlock) fInput;

protected:
    friend class ufPtr(OutputBlock);

    OutputBlock();
    OutputBlock(const OutputBlock &) { ufASSERT(0) }
    virtual ~OutputBlock();

    ufPtr(InputBlock) GetInput(const char *var, const char *&base);
    void EvalAndWrite(FILE *out, const char *str);
    void EvalString(ufTmpBuf &, const char *str, int &pos);
    int EvalControlCriteria(const char *);
    int EvalIfCriteria(const char *);
    const char *FormattedEval(const char *var, char *&strToDel);
    const char* ApplyFuncToWordComponents(
                 const char* formatterName,
                 const char *var,
                 char *&sToDel,
                 WordTokenizer nextWord,
                 WordFormatter reformatWord);

    ufPtr(OutputBlock) fSelf;
    static int fgPos;
    char *fFilename;
    int fLineno;

private:
    void SubstituteVariable(ufTmpBuf &, const char *&s, int &, int &pos);
    const char *GetNthDefault(const char *, char *&);
    void WriteMessage(const char *msg,
                      /* 0= normal, 1=warning, 2=error*/
                      MessageOutputLevel level);
};
declareinlines(ufPtr, OutputBlock);

class ControlBlock : public OutputBlock {
public:
    static ufPtr(OutputBlock) New(const char *string);
    virtual int IsA(int);

    virtual void GenerateOutput();

protected:
    ControlBlock(const char *string);
    ControlBlock(const ControlBlock &) { ufASSERT(0) }
    virtual ~ControlBlock();

    char *fDirective;
};

class LineBlock : public OutputBlock {
public:
    static ufPtr(OutputBlock) New(const char *string);
    virtual int IsA(int);

    virtual void GenerateOutput();

protected:
    LineBlock(const char *string);
    LineBlock(const LineBlock &) { ufASSERT(0) }
    virtual ~LineBlock();

    char *fLine;
};

class BasicBlock : public OutputBlock {
public:
    static ufPtr(OutputBlock) New();
    virtual int IsA(int);

    virtual void SetInput(ufPtr(InputBlock) & p);
    virtual void GenerateOutput();
    virtual void AddBlock(ufPtr(OutputBlock) & blk);

    ufList(OutputBlock) fLines;

protected:
    BasicBlock();
    BasicBlock(const BasicBlock &) { ufASSERT(0) }
    virtual ~BasicBlock();
};

class IfCaseBlock : public BasicBlock {
public:
    static ufPtr(OutputBlock) New(const char *caseCrtr);
    virtual int IsA(int);

    // generates output only if the case criteria evaluates
    // to true
    virtual int CanOutput();

protected:
    IfCaseBlock(const char *caseCriteria);
    IfCaseBlock(const IfCaseBlock &) { ufASSERT(0) }
    virtual ~IfCaseBlock();

    char *fCriteria;
    int fUseNot;
};

class IfBlock : public OutputBlock {
public:
    static ufPtr(OutputBlock) New(const char *ifCrtria);

    virtual int IsA(int);
    virtual void SetInput(ufPtr(InputBlock) & p);
    virtual void GenerateOutput();
    virtual void AddBlock(ufPtr(OutputBlock) & blk);
    virtual void AddDirective(const char *directive);

protected:
    IfBlock(const char *filename);
    IfBlock(const IfBlock &) { ufASSERT(0) }
    virtual ~IfBlock();

    ufList(OutputBlock) fIfCaseBlocks;
};

class LoopIterator;

class LoopBlock : public BasicBlock {
public:
    LoopBlock(const char *loopCriteria);
    static ufPtr(OutputBlock) New(const char *loopCrtr);
    virtual int IsA(int);

    virtual const char *Eval(const char *var, char *&stringToDel);
    virtual int IsFirst(const char *var);
    virtual int IsLast(const char *var);
    virtual int HasEmptyList(const char *var);
    virtual int Resolve(const char *scopedNm, ufPtr(InputBlock) &);
    // generates output only if the loop criteria evaluates
    // to true
    virtual void GenerateOutput();

protected:
    LoopBlock();
    LoopBlock(const LoopBlock &) { ufASSERT(0) }
    void Init(const char *loopCrtr);
    InputBlockIterator *CreateIterator();

    virtual ~LoopBlock();

    char *fIterVar;
    char *fListVar;
    InputBlockIterator *fIterator;

private:
    void IllFormed(const char *);
    int GetNextListVar(ufTmpBuf &, int i);
};

class FileBlock : public BasicBlock {
public:
    static ufPtr(OutputBlock) New(const char *filename);

    virtual int IsA(int);
    virtual FILE *GetOutputFile();
    virtual void GenerateOutput();
    virtual int Resolve(const char *scopedNm, ufPtr(InputBlock) &);

protected:
    FileBlock(const char *filename);
    virtual ~FileBlock();

protected:
    FileBlock(const FileBlock &) { ufASSERT(0) }

    void MakeDir(const char *filnm);
    void MakeParentDir(const char *dirspc);
    char *fFilename;
    FILE *fFile;
    ufPtr(KeyValuePair) fFileKeyValues;
};

//
// The following is used to do the :format-list formatted
// variable substitution. It inherits from LoopBlock only
// as a matter of convenience (YUK!!) and should not be
// used as a OutputBlock as it breaks the semantics of
// OutputBlock.

enum SepType { kNoSep, kPrefixSep, kSuffixSep };

class MultiLineFormatter : public LoopBlock // Implementation
                                            // Inheritance!!
{
public:
    // WARNING!! The MultiLineFormatter is a HACK!!
    // It is not a true subtype of LoopBlock.
    // The functions Eval, IsFirst, IsLast,
    // HasEmptyList, Resolve and GenerateOutput will
    // not behave properly.
    static ufPtr(OutputBlock) New();

    MultiLineFormatter();
    virtual ~MultiLineFormatter();

    virtual int IsA(int);
    virtual void SetInput(ufPtr(InputBlock) & p);

    void AddList(const char *controlStr, const char *outBlkStr);
    virtual void WriteOutputToString(SepType, char sepChar, int startPos,
                                     ufTmpBuf &, int &isFirst);

protected:
    MultiLineFormatter(const MultiLineFormatter &) { ufASSERT(0) }

    struct ListVarFormatStringPair {
        char *fListVar;
        char *fString;
        InputBlockIterator *fIterator;
    };

    ListVarFormatStringPair fLists[10];
    char *fAllListVars;
    int fNumListVars;
};

class SequenceEntry {
public:
    static ufPtr(SequenceEntry) New(const char *, int num);

    char *fName;
    int fNum;

protected:
    friend class ufPtr(SequenceEntry);

    SequenceEntry(const char *, int);
    SequenceEntry(const SequenceEntry &);
    ~SequenceEntry();

    ufPtr(SequenceEntry) fSelf;
};
declareinlines(ufPtr, SequenceEntry);

class SubsequenceRange {
public:
    static ufPtr(SubsequenceRange) New(const char *, int start, int end);

    char *fName;
    int fStart;
    int fEnd;

protected:
    friend class ufPtr(SubsequenceRange);

    SubsequenceRange(const char *, int start, int end);
    SubsequenceRange(const SubsequenceRange &);
    ~SubsequenceRange();

    ufPtr(SubsequenceRange) fSelf;
};
declareinlines(ufPtr, SubsequenceRange);

class Sequence {
public:
    static ufPtr(Sequence) New(const char *);
    static void Load();
    static void Save();

    static ufPtr(Sequence) Find(const char *name);

    const char *Generate(const char *, char *&sToDel);
    const char *GenerateFromSubsequence(const char *subseq, const char *key,
                                        char *&sToDel);

    void Add(const char *, int);
    void AddSubsequence(const char *, int start, int end);

    char *fName;
    int fStart;
    int fEnd;
    ufStrHash(SubsequenceRange) fRanges;
    ufList(SubsequenceRange) fSortedRanges;
    ufStrHash(SequenceEntry) fEntries;
    ufList(SequenceEntry) fSortedEntries;
    ufPtr(Sequence) fSelf;

protected:
    friend class ufPtr(Sequence);

    Sequence(const char *);
    Sequence(const Sequence &);
    ~Sequence();

    const char *GenEntry(const char *, int start, int end, char *&sToDel);

    static void Register(ufPtr(Sequence) & a);

    static ufList(Sequence) * fgSequences;
};

declareinlines(ufPtr, Sequence);

#endif
