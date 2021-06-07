/*
 * See Readme, Notice and License files.
 */
#include "Input.h"
#include "MOC.h"
#include "MOCInternal.h"
#include "MOCTokens.h"
#include "Output.h"

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

/* this is only for mkdir */

#include <sys/stat.h>

extern char *FGets(ufTmpBuf &buf, FILE *in);

ufDEBUG_FILE

static char *gOutputFilename;
static int gOutputLineno;
static char gOutputLinenoBuf[40];
static ufPtr(OutputBlock) gOutputBlock;

static ufPtr(OutputBlock) gTopOutputBlock;

#define NUM_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

inline char* SkipSpaces(char* c) {
    if (c != 0) {
        while (isspace(*c))
            c++;
    }      
    return c;
}

inline const char* SkipSpaces(const char* c) {
    return SkipSpaces((char*)c);
}

inline char* TrimSpaces(char* c) {
    if (c != 0) {
        int len = strlen(c);
        char* e = c + len - 1;
        while (isspace(*e) && e != c)
            e--;
        e[1] = '\0';
        c = SkipSpaces(c);
    }
    return c;
}

inline bool IsBlankLine(const char* c) {
    return *SkipSpaces(c) == '\0';
}

inline bool IsEqual(const char* c1, const char*c2, int n) {
    return !strncmp(c1, c2, n);
}

inline bool IsEqual(const char* c1, const char*c2) {
    return !strcmp(c1, c2);
}

// return the new column position after char c
inline int AdvPos(char c, int curPos) {
    if (c != '\t' && c != '\n') {
        return curPos + 1;
    }
    if (c == '\t') {
        return curPos + 8;
    }
    return 0;
}

inline bool StartsWith(const char* c1, const char* c2) {
    while (*c1 == *c2 && *c1 != '\0' && *c2 != '\0') {
        c1++;
        c2++;
    }
    bool c1IsDone = *c1 == '\0' || isspace(*c1) || *c1 == '=';
    bool c2IsDone = *c2 == '\0' || isspace(*c2) || *c2 == '=';
    return c1IsDone && c2IsDone;
}

inline void ConcatBuf(ufTmpBuf &d, ufTmpBuf &s) {
    int l2 = strlen(s.fBuf);
    int l1 = strlen(d.fBuf);
    if (l1 + l2 + 20 > d.GetSize()) {
        d.Reallocate(l1 + l2 + 1024);
    }
    strncat(d.fBuf, s.fBuf, l1 + l2 + 10);
}

inline const char* GetNextToken(const char* input, ufTmpBuf &tokenBuf) {
    if (input == 0 || *input == '\0') {
        if (tokenBuf.GetSize() > 0) {
            tokenBuf.fBuf[0] = '\0';
        }
        return 0;
    }

    input = SkipSpaces(input);
    int len = tokenBuf.GetSize();
    char* s = tokenBuf.fBuf;
    char* e = tokenBuf.fBuf + tokenBuf.GetSize() - 1;
    int   j = 0;
    while (*input != '\0' && !isspace(*input) && s != e) {
        *s++ = *input++;
        if ( j >= len ) {
            // need a larger buf
            tokenBuf.Reallocate(len + 100);
            len = tokenBuf.GetSize();
            if (j >= len ) {
                // reallocate failed - aborting
                if (len > 0) {
                    tokenBuf.fBuf[0] = '\0';
                }
                return 0;
            }
        }
        j++;
    };
    *s = '\0';
    return SkipSpaces(input);
}

static FILE *OpenTemplate();

static void StartBlock(const char *buf, ufList(OutputBlock) &);

static void MidBlock(const char *buf, ufList(OutputBlock) &);

static void CtrlBlock(const char *buf, ufList(OutputBlock) &);

static void LnBlock(const char *buf, ufList(OutputBlock) &);

static void EndBlock(const char *buf, ufList(OutputBlock) &);

void LoadTemplate() {
    ufTmpBuf tmp(2048);

    ufList(OutputBlock) allBlocks;

    FILE *in = OpenTemplate();

    while (FGets(tmp, in)) {

        if (tmp.fBuf[0] == '@' && tmp.fBuf[1] != '@' && tmp.fBuf[1] != '(') {
            const char *b = SkipSpaces(tmp.fBuf + 2);

            if (tmp.fBuf[1] == '{') {
                StartBlock(b, allBlocks);
            } else if (tmp.fBuf[1] == '}') {
                EndBlock(b, allBlocks);
            } else if (tmp.fBuf[1] == '|') {
                MidBlock(b, allBlocks);
            } else if (tmp.fBuf[1] == '>') {
                CtrlBlock(b, allBlocks);
            } else if (tmp.fBuf[1] != '#') {
                MOCERROR2((errmsg,
                           "Don't under"
                           "stand directive \"%s\"",
                           tmp.fBuf));
                return;
            }
        } else {
            LnBlock(tmp.fBuf, allBlocks);
        }
    }
}

static FILE *OpenTemplate() {
    gFilename = gTemplate;
    lineno = 0;

    if (!gTemplate) {
        MOCERROR2((errmsg, "Can't open template file"));
        return 0;
    }

    FILE *in = fopen(gTemplate, "r");
    if (in == 0) {
        MOCERROR2((errmsg,
                   "Can't open template file "
                   "\"%s\"",
                   gTemplate));
    }

    return in;
}

static void StartBlock(const char *b, ufList(OutputBlock) & allBlocks) {
    if (!allBlocks && gTopOutputBlock != 0) {
        MOCERROR2((errmsg, "You can have only one toplevel block"));
        return;
    }

    ufPtr(OutputBlock) theCurrentBlock;
    ufTmpBuf token(100);
    GetNextToken(b, token);

    if (IsEqual("open", token.fBuf)) {
        theCurrentBlock = FileBlock::New(b);
    } else if (IsEqual("if", token.fBuf)) {
        theCurrentBlock = IfBlock::New(b);
    } else if (IsEqual("foreach", token.fBuf)) {
        theCurrentBlock = LoopBlock::New(b);
    } else {
        MOCERROR2((errmsg,
                   "Don't understand beginning "
                   "block directive \"%s\"",
                   b));
        return;
    }
    if (gNumErrors != 0) {
        return;
    }
    if (!gTopOutputBlock.isValid()) {
        gTopOutputBlock = theCurrentBlock;
    }
    if (allBlocks) {
        ufPtr(OutputBlock) theParentBlock = allBlocks.PeekFirst();
        theParentBlock->AddBlock(theCurrentBlock);
    }
    allBlocks.InsertFirst(theCurrentBlock);
}

static void MidBlock(const char *b, ufList(OutputBlock) & allBlocks) {
    if (!allBlocks) {
        mocError2("found intermediate block directive "
                  "before start block directive");
        return;
    }
    ufPtr(OutputBlock) theCurrentBlock = allBlocks.PeekFirst();
    theCurrentBlock->AddDirective(b);
}

static void EndBlock(const char *b, ufList(OutputBlock) & allBlocks) {
    if (!allBlocks) {
        mocError2("found end block before start block "
                  "directive");
        return;
    }
    ufPtr(OutputBlock) theCurrentBlock = allBlocks.RemoveFirst();
    theCurrentBlock->EndBlock(b);
}

static void CtrlBlock(const char *b, ufList(OutputBlock) & allBlocks) {
    if (!allBlocks) {
        if (!IsBlankLine(b)) {
            mocError2("found line outside of the "
                      "top block");
            return;
        }
        return;
    }
    ufPtr(OutputBlock) theCurrentBlock = allBlocks.PeekFirst();
    ufPtr(OutputBlock) theControlBlock = ControlBlock::New(b);

    theCurrentBlock->AddBlock(theControlBlock);
}

static void LnBlock(const char *b, ufList(OutputBlock) & allBlocks) {
    if (!allBlocks) {
        if (!IsBlankLine(b)) {
            mocError2("found line outside of the "
                      "top block");
            return;
        }
        return;
    }
    ufPtr(OutputBlock) theCurrentBlock = allBlocks.PeekFirst();
    ufPtr(OutputBlock) theLineBlock = LineBlock::New(b);

    theCurrentBlock->AddBlock(theLineBlock);
}

void GenerateOutput() {
    if (gNumErrors != 0) {
        return;
    }

    if (gTopOutputBlock == 0) {
        mocError2("No template file was parsed");
        return;
    }

    ufPtr(InputBlock) theTop = GlobalScope::New();

    gTopOutputBlock->SetInput(theTop);
    gTopOutputBlock->GenerateOutput();

#if 0
    for (ufListIter(Class) i = *Class::fgClasses; i ; i++)
    {
        i.fData->PrintDebug();
    }
#endif
}

implement(ufPtr, OutputBlock);
implement(ufList, OutputBlock);

ufPtr(OutputBlock) OutputBlock::fgTop;

int OutputBlock::fgPos = 0;

OutputBlock::OutputBlock()
    : fSelf(this, kSelfPtrShared), fFilename(ufStrDup(gFilename)),
      fLineno(lineno) {}

OutputBlock::~OutputBlock() { delete[] fFilename; }

void OutputBlock::SetInput(ufPtr(InputBlock) & p) { fInput = p; }

void OutputBlock::WriteMessage(const char *msg, MessageOutputLevel level) {
    ufTmpBuf tmp(200);
    int l = strlen(msg);
    tmp.Reallocate(400 + l);

    gParser = kOUTPUT_PARSER;

    ufTmpBuf tmp2(200);
    EvalString(tmp2, msg, fgPos);
    const char *levelAsString = "";
    if (level == kErrorMessage) {
        levelAsString = "ERROR:";
    } else if (level == kWarningMessage) {
        levelAsString = "WARNING: ";
    }

    if (fInput != 0) {
        gFilename = fInput->fFilename;
        lineno = fInput->fLineno;
        sprintf(tmp.fBuf, "processing input %s(\"%s\")\n%s:%d: %s %s",
                fInput->GetClassName(), fInput->fName, fFilename, fLineno,
                levelAsString, tmp2.fBuf);
    } else {
        gFilename = fFilename;
        lineno = fLineno;
        sprintf(tmp.fBuf, "%s\t%s", levelAsString, msg);
    }
    switch (level) {
    case kErrorMessage:
        mocError2(tmp.fBuf);
        break;
    case kWarningMessage:
        mocWarning(tmp.fBuf);
        break;
    case kNormalMessage:
        mocNormal(tmp.fBuf);
        break;
    }
}

void OutputBlock::Error(const char *msg) { WriteMessage(msg, kErrorMessage); }

void OutputBlock::Warning(const char *msg) {
    WriteMessage(msg, kWarningMessage);
}

int OutputBlock::IsA(int) { return 0; }

FILE *OutputBlock::GetOutputFile() {
    FILE *out = 0;
    if (fParent != 0) {
        out = fParent->GetOutputFile();
    }

    return out;
}

void OutputBlock::AddBlock(ufPtr(OutputBlock) & block) {
    Error("Unexpected block in file");
}

void OutputBlock::AddDirective(const char *directive) {
    Error("Unexpected directive in file");
}

void OutputBlock::EndBlock(const char *) {}

int OutputBlock::CanOutput() { return 1; }

ufPtr(InputBlock) OutputBlock::GetInput(const char *var, const char *&base) {
    if (*var == ':' && var[1] == ':') {
        var += 2;
        base = var;
        return GlobalScope::fgSelf;
    }

    for (const char *v = &(var[strlen(var)]); v != var; v--) {
        if (*v == ':' && v != var && v[-1] == ':') {
            ufPtr(InputBlock) theInput;
            int n = v - 1 - var;
            ufTmpBuf tmp(n + 20);
            ufASSERT(n >= 0);
            strncpy(tmp.fBuf, var, n);
            tmp.fBuf[n] = '\0';

            Resolve(tmp.fBuf, theInput);

            base = v + 1;
            return theInput;
        }
    }

    base = var;

    return fInput;
}

int OutputBlock::SetKeyValue(char *var, char *val) {
    char *expandedVar = var;
    char *sToDel = 0;
    const char *rest;
    const char *v = var;
    while (*v != '\0') {
        if (*v == '@') {
            var = (char *)Eval(var, sToDel);
            break;
        }
        v++;
    }
    ufPtr(InputBlock) theInput = GetInput(var, rest);

    if (theInput != 0) {
        theInput->SetKeyValue(rest, val);
        delete[] sToDel;
        return 1;
    }
    delete[] sToDel;
    return 0;
}

const char *OutputBlock::Eval(const char *var, char *&sToDel) {
    const char *value = 0;
    sToDel = 0;
    var = SkipSpaces(var);

    gFilename = fFilename;
    lineno = fLineno;

    if (var == 0 || *var == '\0') {
        return 0;
    }

    if (IsEqual(var, "OUTPUT_FILENAME")) {
        return gOutputFilename;
    }
    if (IsEqual(var, "OUTPUT_LINENO")) {
        sprintf(gOutputLinenoBuf, "%d", gOutputLineno);
        return gOutputLinenoBuf;
    }
    if (IsEqual(var, "OUTPUT_NEXT_LINENO")) {
        sprintf(gOutputLinenoBuf, "%d", gOutputLineno + 1);
        return gOutputLinenoBuf;
    }

    if (*var == ':' && var[1] != ':') {
        return FormattedEval(var + 1, sToDel);
    }

    const char *rest;
    ufPtr(InputBlock) theInput = GetInput(var, rest);

    if (theInput != 0) {
        value = theInput->Eval(rest);
    }

    if (!value && fParent != 0) {
        value = fParent->Eval(var, sToDel);
    }

    return value;
}

typedef const char *(OutputBlock::*FormatFncPtr)(const char *, char *&);

struct FormatFunctionPair {
    const char *fName;
    int fLen;
    FormatFncPtr fFunc;
};

static FormatFunctionPair gFormatFunctions[] = {
    {"generate-sequence-number", 24, &OutputBlock::EvalGenerateSequenceNumber},
    {"uppercase-prefix", 16, &OutputBlock::EvalUpperCasePrefix},
    {"uppercase-first", 15, &OutputBlock::EvalUpperCaseFirst},
    {"uppercase-all", 13, &OutputBlock::EvalUpperCaseAll},
    {"lowercase-all", 13, &OutputBlock::EvalLowerCaseAll},
    {"format-list", 11, &OutputBlock::EvalFormatList},
    {"length", 6, &OutputBlock::EvalLength},
    {"fill", 4, &OutputBlock::EvalColumnFill},
    {"nth", 3, &OutputBlock::EvalNth},
    {"upp", 3, &OutputBlock::EvalUpperCasePrefix},
    {"up1", 3, &OutputBlock::EvalUpperCaseFirst},
    {"low", 3, &OutputBlock::EvalLowerCaseAll},
    {"len", 3, &OutputBlock::EvalLength},
    {"up", 2, &OutputBlock::EvalUpperCaseAll},
    {"if", 2, &OutputBlock::EvalIf},
    {"?", 1, &OutputBlock::EvalDefaultSubst},
    {"f", 1, &OutputBlock::EvalColumnFill},
    {"g", 1, &OutputBlock::EvalGenerateSequenceNumber},
    {"snake-case", 10, &OutputBlock::EvalSnakeCase},
    {"camelCase", 9, &OutputBlock::EvalCamelCase},
    {"PascalCase", 10, &OutputBlock::EvalPascalCase},
    {0, 0, 0}};

const char *OutputBlock::FormattedEval(const char *var, char *&sToDel) {
    sToDel = 0;
    var = SkipSpaces(var);

    gFilename = fFilename;
    lineno = fLineno;

    FormatFunctionPair *p;

    for (p = gFormatFunctions; p->fName; p++) {
        if (IsEqual(var, p->fName, p->fLen) && !isalnum(var[p->fLen])) {
            var += p->fLen;
            var = SkipSpaces(var);

            return (this->*(p->fFunc))(var, sToDel);
        }
    }

    return 0;
}

const char *OutputBlock::EvalIf(const char *var, char *&sToDel) {
    ufTmpBuf tmp(202);
    int pos;

    sToDel = 0;
    int level = 0;
    int quote = 0;
    int len = strlen(var);
    const char *ifend = var;
    const char *thenstart = var;
    const char *thenend = var + len;
    const char *elsestart = var + len;
    const char *elseend = var + len;
    for (const char *s = var; *s; s++) {
        if (*s == '(') {
            level++;
        }
        if (*s == ')') {
            level = level > 0 ? level - 1 : 0;
        } else if (*s == '\\') {
            s++;
            continue;
        } else if (*s == '"') {
            quote = (quote ? 0 : 1);
        } else if (StartsWith(s, ":else") && level == 0 && quote == 0) {
            elsestart = (s[5] != '\0' && s[6] != '\0' ? s + 6 : var + len);
            thenend = s;
            break;
        } else if (StartsWith(s, ":then") && level == 0 && quote == 0) {
            thenstart = (s[5] != '\0' && s[6] != '\0' ? s + 6 : var + len);
            ifend = s;
        }
    }
    tmp.Reallocate(ifend - var + 2);
    if (ifend > var) {
        strncpy(tmp.fBuf, var, ifend - var);
    }
    tmp.fBuf[ifend - var] = '\0';

    ufTmpBuf tmp2(202);
    if (EvalIfCriteria(tmp.fBuf) != 0) {
        tmp.Reallocate(thenend - thenstart + 2);
        if (thenend > thenstart) {
            strncpy(tmp.fBuf, thenstart, thenend - thenstart);
            tmp.fBuf[thenend - thenstart] = '\0';
            EvalString(tmp2, tmp.fBuf, pos);
            sToDel = ufStrDup(tmp2.fBuf);
        } else {
            return "";
        }
    } else {
        tmp.Reallocate(elseend - elsestart + 2);
        if (elseend > elsestart) {
            strncpy(tmp.fBuf, elsestart, elseend - elsestart);
            tmp.fBuf[elseend - elsestart] = '\0';
            EvalString(tmp2, tmp.fBuf, pos);
            sToDel = ufStrDup(tmp2.fBuf);
        } else {
            return "";
        }
    }

    return sToDel;
}

const char *OutputBlock::EvalDefaultSubst(const char *var, char *&sToDel) {
    sToDel = 0;
    ufTmpBuf tmp(202);
    int pos;
    const char *s = var;
    const char *modified = 0;
    char oldVal;
    while (*s != '\0' && *s != ',')
        s++;
    if (*s != '\0') {
        oldVal = *s;
        *((char *)s) = '\0';
        modified = s;
        s++;
    }

    const char *def = s;
    const char *value = Eval(var, sToDel);

    if (modified != 0) {
        *((char *)modified) = oldVal;
    }

    if (value && *value != '\0') {
        return value;
    }
    delete[] sToDel;
    EvalString(tmp, def, pos);
    sToDel = ufStrDup(tmp.fBuf);
    return sToDel;
}

const char *OutputBlock::EvalNth(const char *var, char *&sToDel) {
    ufTmpBuf list(202);
    ufTmpBuf field(202);
    int nth;

    sToDel = 0;
    if (sscanf(var, " %d %s %[^,] ", &nth, list.fBuf, field.fBuf) != 3) {
        ufTmpBuf tmp(2002);
        sprintf(tmp.fBuf, "invalid nth statement. Must be"
                          " of the form:\n\t@@(:nth <index> <list>"
                          " <variable>\n");
        Error(tmp.fBuf);
        return 0;
    }
    const char *rest;
    ufPtr(InputBlock) theInput = GetInput(list.fBuf, rest);
    if (theInput != 0) {
        if (!theInput->HasList(rest)) {
            return GetNthDefault(var, sToDel);
        }
        InputBlockIterator *iter = theInput->CreateIterator(rest);
        if (!iter || iter->GetTotal() <= nth) {
            return GetNthDefault(var, sToDel);
        }
        while (nth > 0) {
            (*iter)++;
            nth--;
        }
        const char *s = iter->fData->Eval(field.fBuf);
        delete iter;
        if (!s) {
            const char *defs = var;
            while (*defs != '\0' && *defs != ',')
                defs++;
            sToDel = ufStrDup(defs);
            return sToDel;
        }
        return s;
    }

    return GetNthDefault(var, sToDel);
}

const char *OutputBlock::EvalLength(const char *var, char *&sToDel) {
    ufTmpBuf list(202);

    sToDel = 0;
    if (sscanf(var, " %s  ", list.fBuf) != 1) {
        ufTmpBuf tmp(2002);
        sprintf(tmp.fBuf, "invalid length statement. Must be"
                          " of the form:\n\t@@(:length <list>)\n");
        Error(tmp.fBuf);
        return 0;
    }
    const char *rest;
    ufPtr(InputBlock) theInput = GetInput(list.fBuf, rest);
    if (theInput != 0) {
        if (!theInput->HasList(rest)) {
            return "0";
        }
        InputBlockIterator *iter = theInput->CreateIterator(rest);
        if (!iter) {
            return "0";
        }
        ufTmpBuf tmp(2002);
        sprintf(tmp.fBuf, "%d", iter->GetTotal());
        sToDel = ufStrDup(tmp.fBuf);
        return sToDel;
    }

    return "0";
}

const char *OutputBlock::GetNthDefault(const char *var, char *&sToDel) {
    const char *s = var;
    while (*s != '\0' && *s != ',')
        s++;
    if (*s == ',') {
        sToDel = ufStrDup(s + 1);
        return sToDel;
    }

    return "";
}

const char *OutputBlock::EvalColumnFill(const char *var, char *&sToDel) {
    ufTmpBuf tmp(202);
    int pos;
    sToDel = 0;

    if (sscanf(var, "%d", &pos) != 1 || pos > 200 || pos < 1) {
        Error("invalid fill "
              "format. \nMust be of the "
              "format:\n@@(:fill <column number>) "
              "column number is between 0 and 200\n");
        return "";
    }

    if (pos > fgPos) {
        memset(tmp.fBuf, ' ', pos - fgPos);
        tmp.fBuf[pos - fgPos] = '\0';
        sToDel = ufStrDup(tmp.fBuf);
        return sToDel;
    }

    return "";
}

const char *OutputBlock::EvalGenerateSequenceNumber(const char *var,
                                                    char *&sToDel) {
    ufTmpBuf tmp(2000);
    int pos;

    EvalString(tmp, var, pos);

    ufTmpBuf seq(2000);
    ufTmpBuf t1(2000);
    ufTmpBuf t2(2000);
    int num;
    if ((num = sscanf(tmp.fBuf, " %s %s %s ", seq.fBuf, t1.fBuf, t2.fBuf)) !=
            3 &&
        num != 2) {
        ufTmpBuf err(2000);
        sprintf(err.fBuf,
                "invalid generate-sequence-"
                "number format \"%s\"(expanded to "
                "\"%s\")\nMust be of the "
                "format:\n@@(:generate-sequence-number "
                "<SEQUENCE NAME> <item name>)\n",
                var, tmp.fBuf);
        Error(err.fBuf);
        return "";
    }

    gFilename = fFilename;
    lineno = fLineno;
    ufPtr(Sequence) theSeq = Sequence::Find(seq.fBuf);

    if (theSeq != 0) {
        if (num == 2) {
            return theSeq->Generate(t1.fBuf, sToDel);
        } else {
            return theSeq->GenerateFromSubsequence(t1.fBuf, t2.fBuf, sToDel);
        }
    }

    return 0;
}

bool GetNextWord(const char *&var,
                 int wordCount,
                 ufTmpBuf& tmp) {
    if (tmp.GetSize() < 1) {
        return false;
    }
    tmp.fBuf[0] = '\0';

    while (isspace(*var) || *var == '_') var++;
    if (*var == '\0') {
        return false;
    }

    int len = strlen(var);

    char* s = tmp.fBuf;
    char* e = tmp.fBuf + tmp.GetSize() - 1;

    // go to the first lowercased char or '_'
    while (!islower(*var) && *var != '_' && s != e
           && *var != '\0') {
      *s++ = *var++;
    }
    // go to the first uppercased char or '_'
    while (!isupper(*var) && *var != '_' && s != e
           && *var != '\0') {
      *s++ = *var++;
    }
    // this is a word component
    *s = '\0';
    return true;
}

bool GetAllWords(const char *&var,
                 int wordCount,
                 ufTmpBuf& tmp) {
    if (tmp.GetSize() < 1) {
        return false;
    }
    tmp.fBuf[0] = '\0';
    var = SkipSpaces(var);
    if (*var == '\0') {
        return false;
    }
    char* s = tmp.fBuf;
    char* e = tmp.fBuf + tmp.GetSize();
    while (*var != '\0' && s != e) {
      *s++ = *var++;
    }
    *s = '\0';
    return true;
}

bool GetOneThenAllWords(const char *&var,
                        int wordCount,
                        ufTmpBuf& tmp) {
    return wordCount == 0
        ? GetNextWord(var, wordCount, tmp)
        : GetAllWords(var, wordCount, tmp);
}

void UppercasePrefix(ufTmpBuf& dest, int wordCounter, ufTmpBuf& src) {
    char* ds = dest.fBuf;
    char* de = dest.fBuf + dest.GetSize();
    char* ss = src.fBuf;
    char* se = src.fBuf + src.GetSize();
    if (wordCounter == 0) {
        for (; ds != de && ss != se && *ss != '\0';
             *ds++ = toupper(*ss++));
        *ds = '\0';
        return;
    } else if (wordCounter == 1) {
        *ds++ = '_';
        // fall thru 
    }
    for (; ds != de && ss != se && *ss != '\0';
         *ds++ = *ss++);
    *ds = '\0';
    return;
}

void UppercaseAll(ufTmpBuf& dest, int wordCounter, ufTmpBuf& src) {
    char* ds = dest.fBuf;
    char* de = dest.fBuf + dest.GetSize();
    char* ss = src.fBuf;
    char* se = src.fBuf + src.GetSize();
    if (wordCounter > 0) {
        *ds++ = '_';
        // fall thru 
    }
    for (; ds != de && ss != se && *ss != '\0';
         *ds++ = toupper(*ss++));
    *ds = '\0';
    return;
}

void LowercaseAll(ufTmpBuf& dest, int wordCounter, ufTmpBuf& src) {
    char* ds = dest.fBuf;
    char* de = dest.fBuf + dest.GetSize();
    char* ss = src.fBuf;
    char* se = src.fBuf + src.GetSize();
    for (; ds != de && ss != se && *ss != '\0';
         *ds++ = tolower(*ss++));
    *ds = '\0';
    return;
}

void CamelCase(ufTmpBuf& dest, int wordCounter, ufTmpBuf& src) {
    char* ds = dest.fBuf;
    char* de = dest.fBuf + dest.GetSize();
    char* ss = src.fBuf;
    char* se = src.fBuf + src.GetSize();
    if (ds != de && ss != se && *ss != '\0') {
        if (wordCounter == 0) {
            *ds++ = tolower(*ss++);
        } else {
            *ds++ = toupper(*ss++);
        }
    }
    for (; ds != de && ss != se && *ss != '\0';
         *ds++ = tolower(*ss++));
    *ds = '\0';
    return;
}

void PascalCase(ufTmpBuf& dest, int wordCounter, ufTmpBuf& src) {
    char* ds = dest.fBuf;
    char* de = dest.fBuf + dest.GetSize();
    char* ss = src.fBuf;
    char* se = src.fBuf + src.GetSize();
    if (ds != de && ss != se && *ss != '\0') {
        *ds++ = toupper(*ss++);
    }
    for (; ds != de && ss != se && *ss != '\0';
         *ds++ = tolower(*ss++));
    *ds = '\0';
    return;
}

void SnakeCase(ufTmpBuf& dest, int wordCounter, ufTmpBuf& src) {
    char* ds = dest.fBuf;
    char* de = dest.fBuf + dest.GetSize();
    char* ss = src.fBuf;
    char* se = src.fBuf + src.GetSize();
    if (wordCounter > 0) {
        *ds++ = '_';
        // fall thru 
    }
    for (; ds != de && ss != se && *ss != '\0';
         *ds++ = tolower(*ss++));
    *ds = '\0';
    return;
}

const char* OutputBlock::ApplyFuncToWordComponents(
                 const char* formatterName,
                 const char* rawVar,
                 char *&sToDel,
                 WordTokenizer nextWord,
                 WordFormatter reformatWord) {
    ufTmpBuf var(2000);
    int pos;
    EvalString(var, rawVar, pos);
    if (*var.fBuf == '\0') {
        return (sToDel = ufStrDup(var.fBuf));
    }

    const char* v = var.fBuf;
    int len = strlen(var.fBuf);

    ufTmpBuf dest(2 * len + 10);
    ufTmpBuf word(len + 1);
    ufTmpBuf reformattedWord(dest.GetSize());
    bool isDone = false;

    char *d = dest.fBuf;

    *d = '\0';

    int wordCount = -1;
    while (true) {
        ++wordCount;
        if (!nextWord(v, wordCount, word)) {
            return (sToDel = ufStrDup(dest.fBuf));
        }
        reformattedWord.fBuf[0] = '\0';
        reformatWord(reformattedWord, wordCount, word);
        const char *s = reformattedWord.fBuf;
        for (; *s != '\0' ; *d++ = *s++);
        *d = '\0';
    };
    *d = '\0';
    return (sToDel = ufStrDup(dest.fBuf));
}

const char *OutputBlock::EvalUpperCasePrefix(const char *var, char *&sToDel) {
    return ApplyFuncToWordComponents("uppercase-prefix",
                                     var,
                                     sToDel,
                                     GetOneThenAllWords,
                                     UppercasePrefix);
}

const char *OutputBlock::EvalUpperCaseAll(const char *var, char *&sToDel) {
    return ApplyFuncToWordComponents("uppercase-all",
                                     var,
                                     sToDel,
                                     GetAllWords,
                                     UppercaseAll);
}

const char *OutputBlock::EvalUpperCaseFirst(const char *var, char *&sToDel) {
    ufTmpBuf tmp(2000);
    int pos;

    EvalString(tmp, var, pos);
    if (*tmp.fBuf == '\0') {
        return "";
    }

    int len = strlen(tmp.fBuf);

    ufTmpBuf uc(2 * len + 10);
    uc.fBuf[0] = (islower(tmp.fBuf[0]) ? toupper(tmp.fBuf[0]) : tmp.fBuf[0]);
    strcpy(uc.fBuf + 1, tmp.fBuf + 1);

    return (sToDel = ufStrDup(uc.fBuf));
}

const char *OutputBlock::EvalLowerCaseAll(const char *var, char *&sToDel) {
    return ApplyFuncToWordComponents("lowercase-all",
                                     var,
                                     sToDel,
                                     GetAllWords,
                                     LowercaseAll);
}

const char *OutputBlock::EvalSnakeCase(const char *var, char *&sToDel) {
    return ApplyFuncToWordComponents("snake-case",
                                     var,
                                     sToDel,
                                     GetNextWord,
                                     SnakeCase);
}

const char *OutputBlock::EvalCamelCase(const char *var, char *&sToDel) {
    return ApplyFuncToWordComponents("camelCase",
                                     var,
                                     sToDel,
                                     GetNextWord,
                                     CamelCase);
}

const char *OutputBlock::EvalPascalCase(const char *var, char *&sToDel) {
    return ApplyFuncToWordComponents("PascalCase",
                                     var,
                                     sToDel,
                                     GetNextWord,
                                     PascalCase);
}

const char *OutputBlock::EvalFormatList(const char *var, char *&sToDel) {
    ufASSERT(*var == ':');
    var++;
    var = SkipSpaces(var);

    SepType sepType = kNoSep;
    char sepChar = ' ';

    if (*var != ':') {
        if (StartsWith(var, "prefix-sep")) {
            var += 10;
            sepType = kPrefixSep;
        } else if (StartsWith(var, "suffix-sep")) {
            var += 10;
            sepType = kSuffixSep;
        } else {
            // TBD: error
            return 0;
        }

        var = SkipSpaces(var);

        if (*var != '=') {
            // TBD: error
            return 0;
        }
        var++;

        var = SkipSpaces(var);

        if (*var != '\'') {
            // TBD: error
            return 0;
        }
        var++;

        if (*var == '\\') {
            var++;
        }

        sepChar = *var;
        var++;

        if (*var != '\'') {
            // TBD: error
            return 0;
        }
        var++;
        var = SkipSpaces(var);

        if (*var != ':') {
            // TBD: error
            return 0;
        }
        var++;

        var = SkipSpaces(var);
    }

    ufTmpBuf controlString(1024);
    ufTmpBuf outputString(1024);

    MultiLineFormatter lblock;
    while (*var != 0) {
        char *cs;
        for (cs = controlString.fBuf; *var != '\0' && *var != ':';
             var++, cs++) {
            if (cs - controlString.fBuf + 10 > controlString.GetSize()) {
                controlString.Reallocate(controlString.GetSize() + 1024);
            }
            if (*var == '\\' && var[1] != '\0') {
                *cs = *++var;
                continue;
            }
            *cs = *var;
        }
        *cs = '\0';
        if (*var == ':') {
            var++;
        }

        var = SkipSpaces(var);

        int level = 1;
        char *os;
        for (os = outputString.fBuf; *var != '\0' && *var != ':' && level > 0;
             var++, os++) {

            if (os - outputString.fBuf + 10 > outputString.GetSize()) {
                outputString.Reallocate(outputString.GetSize() + 1024);
            }

            if (*var == '\\' && var[1] != '\0') {
                *os = *++var;
                continue;
            }

            if (*var == '(') {
                level++;
            } else if (*var == ')') {
                level--;
            }

            *os = *var;
        }
        *os = '\0';
        TrimSpaces(outputString.fBuf);

        lblock.AddList(controlString.fBuf, outputString.fBuf);

        if (*var == ':') {
            var++;
        }

        var = SkipSpaces(var);
    }

    int isFirst = 1;
    lblock.fParent = fSelf;
    lblock.SetInput(fInput);
    lblock.WriteOutputToString(sepType, sepChar, fgPos, outputString, isFirst);

    sToDel = ufStrDup(outputString.fBuf);

    return sToDel;
}

int OutputBlock::IsFirst(const char *var) {
    int res = 1;
    if (fParent != 0) {
        res = fParent->IsFirst(var);
    }

    return res;
}

int OutputBlock::IsLast(const char *var) {
    int res = 1;
    if (fParent != 0) {
        res = fParent->IsLast(var);
    }

    return res;
}

int OutputBlock::HasEmptyList(const char *var) {
    int value = 0;
    int found = 0;

    if (var == 0) {
        return 0;
    }

    if (*var == ':' && var[1] != ':') {
        ufASSERT(0);
        return 1;
    }

    const char *rest;
    ufPtr(InputBlock) theInput = GetInput(var, rest);

    if (theInput != 0) {
        found = theInput->HasList(rest);
        if (found) {
            value = !theInput->HasNonEmptyList(rest);
        }
    }

    if (!found && fParent != 0) {
        value = fParent->HasEmptyList(var);
    }

    return value;
}

int OutputBlock::HasParent(const char *var) {
    int value = 0;

    if (var == 0) {
        return 0;
    }

    if (*var == ':' && var[1] != ':') {
        ufASSERT(0);
        return 1;
    }

    const char *rest;
    ufPtr(InputBlock) theInput = GetInput(var, rest);

    if (theInput != 0) {
        value = theInput->HasParent(var);
    }

    if (value != 1 && value != 0) {
        value = fParent->HasParent(var);
    }

    return value;
}

int OutputBlock::IsFromCurrentModule(char *v) {
    if (fInput != 0) {
        return fInput->IsFromCurrentModule();
    }

    return fParent->IsFromCurrentModule(v);
}

int OutputBlock::Resolve(const char *var, ufPtr(InputBlock) & p) {
    int res = 0;
    if (fParent != 0) {
        res = fParent->Resolve(var, p);
    }
    return res;
}

void OutputBlock::EvalAndWrite(FILE *output, const char *string) {
    ufTmpBuf tmp(1024);

    EvalString(tmp, string, fgPos);

    for (char *s = tmp.fBuf; *s != '\0'; s++) {
        putc(*s, output);
        if (*s == '\n') {
            gOutputLineno++;
        }
    }
}

void OutputBlock::EvalString(ufTmpBuf &tmp, const char *string, int &pos) {
    int i = 0;
    const char *s;

    for (s = string; s != 0 && *s != '\0'; s++) {
        if ((s[0] == '\\' && s[1] == '@') || (s[0] == '@' && s[1] == '@')) {
            s++;
            if (tmp.GetSize() <= i) {
                tmp.Reallocate(i + 1024);
            }
            tmp.fBuf[i++] = *s;
            pos = AdvPos(*s, pos);
        } else if (*s == '@') {
            if (*++s == '(') {
                SubstituteVariable(tmp, s, i, pos);
            } else {
                ufTmpBuf errmsg(100 + strlen(string));
                sprintf(errmsg.fBuf,
                        "Expected variable of "
                        "the form @@(name) in input '%s'",
                        s);
                Error(errmsg.fBuf);
            }
        } else {
            if (tmp.GetSize() <= i) {
                tmp.Reallocate(i + 1024);
            }
            tmp.fBuf[i++] = *s;
            pos = AdvPos(*s, pos);
        }
    }

    tmp.fBuf[i] = '\0';
}

void OutputBlock::SubstituteVariable(ufTmpBuf &tmp, const char *&s, int &i,
                                     int &pos) {
    ufTmpBuf tmp2(1024);
    char *b = tmp2.fBuf;
    int level = 1;
    s++;

    // TBD: need to eventually handle nested variables

    for (; *s != '\0' && level != 0; s++) {
        if (*s == '\\') {
            // We remove the excess back slashes
            // at the very end so as to not forget
            // what we are escaping.
            *b++ = *s;
            continue;
        }

        if (*s == '(') {
            level++;
        }

        if (*s == ')' && --level == 0) {
            break;
        }
        *b++ = *s;
    }

    *b = '\0';
    char *sToDel = 0;
    const char *value = Eval(tmp2.fBuf, sToDel);
    if (value) {
        for (; *value != '\0'; value++) {
            if (tmp.GetSize() <= i) {
                tmp.Reallocate(i + 1024);
            }
            tmp.fBuf[i++] = *value;
            pos = AdvPos(*value, pos);
        }
    } else {
        ufTmpBuf tmp3(100 + strlen(tmp2.fBuf));
        sprintf(tmp3.fBuf,
                "Can't expand variable "
                "\"%s\"\n",
                tmp2.fBuf);
        Error(tmp3.fBuf);
    }

    tmp.fBuf[i] = '\0';

    delete[] sToDel;
}

int OutputBlock::DoesExist(char *v) {
    char *sToDel;
    const char *value = Eval(v, sToDel);

    return value && *value != '\0' ? 1 : 0;
}

int OutputBlock::EvalControlCriteria(const char *controlstatement) {
    gOutputBlock = fSelf;
    int res;

    ufTmpBuf tmp(1024);
    int pos = 0;
    res = 0;

    if (strncmp("error", controlstatement, 5) == 0) {
        EvalString(tmp, controlstatement + 5, pos);
        Error(controlstatement);
    } else if (strncmp("warning", controlstatement, 7) == 0) {
        EvalString(tmp, controlstatement + 7, pos);
        Warning(controlstatement);
    } else if (strncmp("message", controlstatement, 7) == 0) {
        EvalString(tmp, controlstatement + 7, pos);
        WriteMessage(controlstatement + 7, kNormalMessage);
    } else {
        EvalString(tmp, controlstatement, pos);
        res = mocParseControl(tmp.fBuf, fFilename, fLineno);
    }
    return res;
}

int OutputBlock::EvalIfCriteria(const char *ifstatement) {
    int res = 0;
    gOutputBlock = fSelf;

    ufTmpBuf tmp(1024);
    int pos = 0;
    EvalString(tmp, ifstatement, pos);

    res = mocParseIf(tmp.fBuf, fFilename, fLineno);

    return res;
}

int ControlEvalSetString(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    char *s2 = s;
    // s2 = SkipSpaces(s2);
    if (s2 != s) {
        s2 = ufStrDup(s2);
        delete[] s;
        s = s2;
    }
    int x = gOutputBlock->SetKeyValue(input, s);
    delete[] input;
    delete[] s;

    return x;
}

int ControlEvalSetInteger(char *input, int i) {
    // TBD: use the input to determine which input block
    // is being referenced.

    char buf[20];
    sprintf(buf, "%d", i);
    int x = gOutputBlock->SetKeyValue(input, buf);
    delete[] input;

    return x;
}

int IfEvalExists(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    char safeInput[300];
    char safeS[300];
    if (input != 0) {
        strncpy(safeInput, input, sizeof(safeInput) - 1);
        safeInput[sizeof(safeInput) - 1] = '\0';
    } else {
        safeInput[0] = '\0';
    }

    if (s != 0) {
        strncpy(safeS, input, sizeof(safeS) - 1);
        safeS[sizeof(safeS) - 1] = '\0';
    } else {
        safeS[0] = '\0';
    }

    if (input != 0)
        delete[] input;
    int x = gOutputBlock->DoesExist(s);

    delete[] s;
    return x;
}

int IfEvalEmpty(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    if (input != 0)
        delete[] input;

    int x = gOutputBlock->HasEmptyList(s);
    delete[] s;
    return x;
}

int IfEvalIsFromCurrentModule(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    if (input != 0)
        delete[] input;

    int x = gOutputBlock->IsFromCurrentModule(s);
    delete[] s;
    return x;
}

int IfEvalIsA(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    if (input != 0)
        delete[] input;

    int x = gOutputBlock->HasParent(s);
    delete[] s;
    return x;
}

int IfEvalFirst(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    if (input != 0)
        delete[] input;

    int x = gOutputBlock->IsFirst(s);
    delete[] s;
    return x;
}

int IfEvalLast(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    if (input != 0)
        delete[] input;

    int x = gOutputBlock->IsLast(s);
    delete[] s;
    return x;
}

int IfEvalStrequal(char *input, char *s1, char *s2) {
    // TBD: use the input to determine which input block
    // is being referenced.
    if (input != 0)
        delete[] input;

    int x = (IsEqual(s1, s2) ? 1 : 0);
    delete[] s1;
    delete[] s2;
    return x;
}

LineBlock::LineBlock(const char *string) : fLine(ufStrDup(string)) {}

LineBlock::~LineBlock() {
    delete[] fLine;
    fLine = 0;
}

int LineBlock::IsA(int t) { return t == kLineBlock; }

ufPtr(OutputBlock) LineBlock::New(const char *string) {
    return (new LineBlock(string))->fSelf;
}

void LineBlock::GenerateOutput() {
    FILE *out = GetOutputFile();

    if (out) {
        EvalAndWrite(out, fLine);
    }
}

/*
 */
ControlBlock::ControlBlock(const char *string) : fDirective(0) {
    const char *s = SkipSpaces(string);
    fDirective = ufStrDup(s);
}

ControlBlock::~ControlBlock() {
    delete[] fDirective;
    fDirective = 0;
}

int ControlBlock::IsA(int t) { return t == kControlBlock; }

ufPtr(OutputBlock) ControlBlock::New(const char *string) {
    return (new ControlBlock(string))->fSelf;
}

void ControlBlock::GenerateOutput() { EvalControlCriteria(fDirective); }

ufPtr(OutputBlock) BasicBlock::New() { return (new BasicBlock())->fSelf; }

BasicBlock::BasicBlock() {}

BasicBlock::~BasicBlock() {}

int BasicBlock::IsA(int t) { return t == kBasicBlock; }

void BasicBlock::GenerateOutput() {
    for (ufListIter(OutputBlock) i = fLines; i; i++) {
        i.fData->fParent = fSelf;
        i.fData->GenerateOutput();
    }
}

void BasicBlock::SetInput(ufPtr(InputBlock) & p) {
    OutputBlock::SetInput(p);

    for (ufListIter(OutputBlock) i = fLines; i; i++) {
        i.fData->SetInput(p);
    }
}

void BasicBlock::AddBlock(ufPtr(OutputBlock) & block) {
    block->fParent = fSelf;
    fLines.InsertLast(block);
}

ufPtr(OutputBlock) IfCaseBlock::New(const char *caseCriteria) {
    return (new IfCaseBlock(caseCriteria))->fSelf;
}

IfCaseBlock::IfCaseBlock(const char *c) : fUseNot(0) {
    c = SkipSpaces(c);

    if (c[0] == 'i' && c[1] == 'f') {
        c += 2;
    }

    c = SkipSpaces(c);

    if (!c || *c == '\0') {
        c = "1";
    }
    fCriteria = TrimSpaces( ufStrDup(c) );
}

IfCaseBlock::~IfCaseBlock() { delete[] fCriteria; }

int IfCaseBlock::IsA(int t) { return t == kIfCaseBlock; }

int IfCaseBlock::CanOutput() { return EvalIfCriteria(fCriteria); }

ufPtr(OutputBlock) IfBlock::New(const char *ifCriteria) {
    return (new IfBlock(ifCriteria))->fSelf;
}

IfBlock::IfBlock(const char *ifCriteria) {
    ufPtr(OutputBlock) theFirstCase = IfCaseBlock::New(ifCriteria);

    theFirstCase->fParent = fSelf;
    fIfCaseBlocks.InsertLast(theFirstCase);
}

IfBlock::~IfBlock() {}

int IfBlock::IsA(int t) { return t == kIfBlock; }

void IfBlock::SetInput(ufPtr(InputBlock) & p) {
    OutputBlock::SetInput(p);

    ufListIter(OutputBlock) i;

    for (i = fIfCaseBlocks; i; i++) {
        i.fData->SetInput(p);
    }
}

void IfBlock::GenerateOutput() {
    for (ufListIter(OutputBlock) i = fIfCaseBlocks; i; i++) {
        if (i.fData->CanOutput()) {
            i.fData->GenerateOutput();
            break;
        }
    }
}

void IfBlock::AddBlock(ufPtr(OutputBlock) & block) {
    ufPtr(OutputBlock) theLastCaseBlock = fIfCaseBlocks.PeekLast();

    theLastCaseBlock->AddBlock(block);
}

void IfBlock::AddDirective(const char *directive) {
    ufTmpBuf token(100);
    GetNextToken(directive, token);
    if (IsEqual("else", token.fBuf)) {
        const char* d = directive + 5;
        ufPtr(OutputBlock) theNextCaseBlock = IfCaseBlock::New(d);
        theNextCaseBlock->fParent = fSelf;
        fIfCaseBlocks.InsertLast(theNextCaseBlock);
    }
}

ufPtr(OutputBlock) LoopBlock::New(const char *loopCriteria) {
    return (new LoopBlock(loopCriteria))->fSelf;
}

LoopBlock::LoopBlock() : fIterVar(0), fListVar(0), fIterator(0) {}

LoopBlock::LoopBlock(const char *loopCriteria)
    : fIterVar(0), fListVar(0), fIterator(0) {
    Init(loopCriteria);
}

void LoopBlock::Init(const char *loopCriteria) {
    delete[] fIterVar;
    fIterVar = 0;
    delete[] fListVar;
    fListVar = 0;
    delete fIterator;
    fIterator = 0;

    ufTmpBuf tokenBuf(100);
    // foreach
    const char* cursor = GetNextToken(loopCriteria, tokenBuf);
    if (!IsEqual(tokenBuf.fBuf, "foreach")) {
        IllFormed(loopCriteria);
        return;
    }

    // $iter-name
    ufTmpBuf iterVar(100);
    cursor = GetNextToken(cursor, iterVar);

    // in
    cursor = GetNextToken(cursor, tokenBuf);
    if (!IsEqual(tokenBuf.fBuf, "in") || cursor == 0) {
        IllFormed(loopCriteria);
        return;
    }

    // $list1 [$list2 ...]
    cursor = SkipSpaces(cursor);

    fIterVar = ufStrDup(iterVar.fBuf);
    fListVar = ufStrDup(cursor);
}

LoopBlock::~LoopBlock() {
    delete[] fIterVar;
    delete[] fListVar;
    delete fIterator;
}

void LoopBlock::IllFormed(const char *loopCriteria) {
    ufTmpBuf tmp(1024);
    sprintf(tmp.fBuf,
            "ill-formed for block \"%s\","
            "Must be of the form \n"
            "\t\"@@{foreach <iterator var name> "
            "in <list var name>",
            loopCriteria);
    Error(tmp.fBuf);
    return;
}

int LoopBlock::IsA(int t) { return t == kLoopBlock; }

const char *LoopBlock::Eval(const char *var, char *&sToDel) {
    const char *value = 0;
    sToDel = 0;

    if (var == 0) {
        return 0;
    }

    if (*var == ':' && var[1] != ':') {
        return FormattedEval(var + 1, sToDel);
    }

    if (fIterator != 0 && fIterator->fData != 0) {
        value = fIterator->fData->Eval(var);
    }
    if (!value) {
        value = BasicBlock::Eval(var, sToDel);
    }

    return value;
}

int LoopBlock::IsFirst(const char *var) {
    if (IsEqual(var, fIterVar)) {
        if (fIterator) {
            return fIterator->IsFirst();
        }
        return 0;
    }
    return OutputBlock::IsFirst(var);
}

int LoopBlock::IsLast(const char *var) {
    if (IsEqual(var, fIterVar)) {
        if (fIterator) {
            return fIterator->IsLast();
        }
        return 0;
    }
    return OutputBlock::IsLast(var);
}

int LoopBlock::HasEmptyList(const char *var) {
    int value = 0;

    if (var == 0) {
        return 0;
    }

    if (*var == ':' && var[1] != ':') {
        ufASSERT(0);
        return 0;
    }
    if (*var == ':' && var[1] == ':') {
        var += 2;
    }

    if (fIterator != 0 && fIterator->fData != 0 &&
        fIterator->fData->HasList(var)) {
        value = !fIterator->fData->HasNonEmptyList(var);
    } else {
        value = BasicBlock::HasEmptyList(var);
    }

    return value;
}

int LoopBlock::Resolve(const char *n, ufPtr(InputBlock) & p) {
    p.MakeEmpty();

    if (IsEqual(n, fIterVar)) {
        if (fIterator != 0 && fIterator->fData != 0) {
            p = fIterator->fData;
        }

        return 1;
    }

    return OutputBlock::Resolve(n, p);
}

InputBlockIterator *LoopBlock::CreateIterator() {
    MultiListInputBlockIterator *iter = new MultiListInputBlockIterator;

    int j = 0;
    ufTmpBuf tmp(100);

    while (GetNextListVar(tmp, j)) {
        if (fInput->HasList(tmp.fBuf)) {
            iter->AddSubiterator(fInput->CreateIterator(tmp.fBuf));
        } else {
            ufTmpBuf tmp3(100 + strlen(tmp.fBuf) +
                          strlen(fInput->GetClassName()));
            sprintf(tmp3.fBuf,
                    "The list \"%s\" is "
                    "not a field of \"%s\"\n",
                    tmp.fBuf, fInput->GetClassName());
            Error(tmp3.fBuf);
            delete iter;
            return 0;
        }
        j++;
    }

    return iter;
}

void LoopBlock::GenerateOutput() {
    delete fIterator;
    fIterator = CreateIterator();

    for (; fIterator != 0 && fIterator->fData != 0; (*fIterator)++) {
        for (ufListIter(OutputBlock) i = fLines; i; i++) {
            i.fData->SetInput(fIterator->fData);
            i.fData->GenerateOutput();
        }
    }
}

int LoopBlock::GetNextListVar(ufTmpBuf &tmp, int i) {
    const char *l = fListVar;
    do {
        l = GetNextToken(l, tmp);
        if ( l == 0 ) {
            // failed to get token
            return 0;
        }
    } while (--i >= 0);
    
    return 1;
}

ufPtr(OutputBlock) FileBlock::New(const char *filename) {
    return (new FileBlock(filename))->fSelf;
}

FileBlock::FileBlock(const char *filename) : fFilename(0) {
    ufTmpBuf tmp(1024);
    if (sscanf(filename, "open %s", tmp.fBuf) == 1) {
        const char pathSep = '/';
        const char altPathSep = '\\';
        ufTmpBuf tmp2(tmp.GetSize());
        if (tmp.fBuf[0] != pathSep && tmp.fBuf[0] != altPathSep) {
            strncpy(tmp2.fBuf, gOutputDirectory, tmp2.GetSize());
            int n = strnlen(tmp2.fBuf, tmp2.GetSize());
            if (tmp2.fBuf[n - 1] != pathSep && tmp2.fBuf[n - 1] != altPathSep) {
                strncat(tmp2.fBuf, "/", tmp2.GetSize());
            }
        } else {
            strncpy(tmp2.fBuf, "", tmp2.GetSize());
        }
        strncat(tmp2.fBuf, tmp.fBuf, tmp2.GetSize());
        fFilename = ufStrDup(tmp2.fBuf);
    } else {
        ufTmpBuf tmp2(2024);
        sprintf(tmp2.fBuf,
                "Don't understand file in "
                "command \"open <filename>\", "
                "cmd=\"%s\"",
                filename);
    }
    fFile = NULL;
}

FileBlock::~FileBlock() { delete[] fFilename; }

int FileBlock::IsA(int t) { return t == kFileBlock; }

FILE *FileBlock::GetOutputFile() { return fFile; }

void FileBlock::GenerateOutput() {
    if (fFile) {
        fclose(fFile);
    }

    ufTmpBuf tmp(1000);
    int pos = 0;

    EvalString(tmp, fFilename, pos);
    MakeDir(tmp.fBuf);

    fFile = fopen(tmp.fBuf, "w");
    gOutputFilename = tmp.fBuf;
    gOutputLineno = 1;
    fFileKeyValues = KeyValuePair::New();

    BasicBlock::GenerateOutput();
}

int FileBlock::Resolve(const char *n, ufPtr(InputBlock) & p) {
    p.MakeEmpty();

    if (IsEqual(n, "file")) {
        p = (ufPtr(InputBlock) &)fFileKeyValues;

        return 1;
    }

    return OutputBlock::Resolve(n, p);
}

void FileBlock::MakeDir(const char *filename) {
    for (int n = strlen(filename); n >= 0; n--) {
        if (filename[n] == gDirectorySeparator) {
            ufTmpBuf tmp(n + 10);
            strncpy(tmp.fBuf, filename, n);
            tmp.fBuf[n] = '\0';
            MakeParentDir(tmp.fBuf);
            break;
        }
    }
}

void FileBlock::MakeParentDir(const char *dirspec) {
    if (IsEqual(dirspec, ".") || *dirspec == '\0') {
        return;
    }
    for (int n = strlen(dirspec); n >= 0; n--) {
        if (dirspec[n] == gDirectorySeparator) {
            ufTmpBuf tmp(n + 10);
            strncpy(tmp.fBuf, dirspec, n);
            tmp.fBuf[n] = '\0';
            MakeParentDir(tmp.fBuf);
            break;
        }
    }

    mkdir(dirspec, 0777);
}

ufPtr(OutputBlock) MultiLineFormatter::New() {
    return (new MultiLineFormatter)->fSelf;
}

// This is not an output block at all
const int kMultiLineFormatter = -1;

int MultiLineFormatter::IsA(int t) {
    return t == kMultiLineFormatter || LoopBlock::IsA(t);
}

MultiLineFormatter::MultiLineFormatter() : LoopBlock() {
    memset(&fLists, 0, sizeof(fLists));
}

MultiLineFormatter::~MultiLineFormatter() {
    fIterator = 0;

    for (unsigned int i = 0; i < NUM_ELEMENTS(fLists); i++) {
        delete[] fLists[i].fListVar;
        delete[] fLists[i].fString;
        delete fLists[i].fIterator;
    }
    memset(&fLists, 0, sizeof(fLists));
}

void MultiLineFormatter::SetInput(ufPtr(InputBlock) & p) {
    LoopBlock::SetInput(p);
}

void MultiLineFormatter::AddList(const char *controlString,
                                 const char *output) {
    for (unsigned int i = 0; i < NUM_ELEMENTS(fLists); i++) {
        if (fLists[i].fListVar == 0) {
            fLists[i].fListVar = ufStrDup(controlString);
            fLists[i].fString = ufStrDup(output);
            return;
        }
    }
}

void MultiLineFormatter::WriteOutputToString(SepType t, char sepChar,
                                             int startPos, ufTmpBuf &tmp,
                                             int &cmpBug) {
    int total = 0;
    int count = 0;
    tmp.fBuf[0] = '\0';
    // TBD: delete fIterator;
    fIterator = 0;
    char sepString[2];
    sepString[0] = sepChar;
    sepString[1] = '\0';
    int pos = startPos;

    unsigned int i;
    for (i = 0; i < NUM_ELEMENTS(fLists); i++) {
        if (fLists[i].fListVar == 0) {
            break;
        }

        Init(fLists[i].fListVar);
        fLists[i].fIterator = CreateIterator();
        if (!fLists[i].fIterator) {
            return;
        }

        total += fLists[i].fIterator->GetTotal();
    }

    FILE *out = GetOutputFile();

    if (!out) {
        return;
    }

    for (i = 0; i < NUM_ELEMENTS(fLists) && fLists[i].fIterator; i++) {
        Init(fLists[i].fListVar);

        fIterator = fLists[i].fIterator;

        for (; fIterator->fData != 0; (*fIterator)++) {
            ufTmpBuf tmp2(startPos + 100);
            tmp2.fBuf[0] = '\0';
            while (count != 0 && pos < startPos) {
                tmp2.fBuf[pos++] = ' ';
            }
            tmp2.fBuf[pos] = '\0';
            ConcatBuf(tmp, tmp2);

            tmp2.fBuf[0] = '\0';
            if (t == kPrefixSep && count != 0) {
                strncpy(tmp2.fBuf, sepString, 3);
                pos++;
            }
            ConcatBuf(tmp, tmp2);

            tmp2.fBuf[0] = '\0';
            EvalString(tmp2, fLists[i].fString, pos);
            count++;
            ConcatBuf(tmp, tmp2);

            if ((count != total) && t == kSuffixSep) {
                tmp2.fBuf[0] = '\0';
                EvalString(tmp2, sepString, pos);
                ConcatBuf(tmp, tmp2);
            }

            if (count != total && t != kNoSep) {
                strcat(tmp.fBuf, "\n");
                pos = 0;
            }
        }
        fIterator = 0;
    }
    cmpBug = 0;
}

/*
 * Sequence implementation
 */

implement(ufPtr, Sequence);
implement(ufList, Sequence);

void LoadSequences() { Sequence::Load(); }

void SaveSequences() { Sequence::Save(); }

ufList(Sequence) * Sequence::fgSequences;

ufPtr(Sequence) Sequence::New(const char *n) {
    return (new Sequence(n))->fSelf;
}

Sequence::Sequence(const char *n)
    : fName(ufStrDup(n)), fStart(10), fEnd(64000), fEntries(),
      fSelf(this, kSelfPtrShared) {
    Register(fSelf);
}

Sequence::Sequence(const Sequence &o)
    : fName(ufStrDup(o.fName)), fStart(10), fEnd(64000), fEntries(),
      fSelf(this, kSelfPtrShared){ufASSERT(0)}

      Sequence::~Sequence() {
    delete[] fName;
}

void Sequence::Register(ufPtr(Sequence) & a) {
    if (fgSequences == 0) {
        fgSequences = new ufList(Sequence);
    }

    fgSequences->InsertLast(a);
}

ufPtr(Sequence) Sequence::Find(const char *n) {
    ufPtr(Sequence) theSeq;

    if (fgSequences == 0) {
        fgSequences = new ufList(Sequence);
    }

    ufListIter(Sequence) i;
    for (i = *fgSequences; i.fData != 0; i++) {
        if (IsEqual(i.fData->fName, n)) {
            return i.fData;
        }
    }

    theSeq = Sequence::New(n);

    return theSeq;
}

static void ReadSequence(ufPtr(Sequence) &, FILE *);

void Sequence::Load() {
    FILE *in;

    gFilename = gSequenceLogFile;
    lineno = 0;

    if (!gSequenceLogFile || (in = fopen(gSequenceLogFile, "r")) == 0) {
        if (access(gSequenceLogFile, 0)) {
            /* file does not exist */
            return;
        }

        MOCERROR2((errmsg,
                   "Can not open sequence "
                   "log file \"%s\"",
                   gSequenceLogFile));
        return;
    }

    ufTmpBuf tmp(200);
    ufTmpBuf seqName(1024);
    int start;
    int stop;
    char *s;

    while (FGets(tmp, in)) {
        s = SkipSpaces(tmp.fBuf);

        if (*s == '\0' || *s == '#') {
            continue;
        }

        if (sscanf(s, "begin \"%[^\"]\" %d %d", seqName.fBuf, &start, &stop) !=
            3) {
            MOCERROR2((errmsg,
                       "invalid sequence "
                       "begin statement \"%s\"\n"
                       "must be of the form "
                       "begin \"<sequence name>\" "
                       " <start number> <end number>",
                       s));
        }

        ufPtr(Sequence) theSeq = Sequence::New(seqName.fBuf);

        theSeq->fStart = start;
        theSeq->fEnd = stop;
        ReadSequence(theSeq, in);
    }
    fclose(in);
}

static void ReadSequence(ufPtr(Sequence) & theSeq, FILE *in) {
    char *s;
    ufTmpBuf tmp(200);
    ufTmpBuf seqEntryName(1024);
    int num;

    while (FGets(tmp, in)) {
        s = SkipSpaces(tmp.fBuf);
        if (*s == '\0' || *s == '#') {
            continue;
        }

        if (StartsWith(s, "end")) {
            break;
        }
        if (StartsWith(s, "subsequence")) {
            int start;
            int end;
            if (sscanf(s + 11, " \"%[^\"]\" %d %d ", seqEntryName.fBuf, &start,
                       &end) != 3) {
                MOCERROR((errmsg,
                          "invalid sub"
                          "sequence range state"
                          "ment \"%s\"\n"
                          "must be of the form:\n "
                          "\tsubsequence "
                          " \"<subsequence name>\" "
                          " <start> <end>",
                          s));
            }
            theSeq->AddSubsequence(seqEntryName.fBuf, start, end);
            continue;
        }

        if (sscanf(s, " \"%[^\"]\" %d ", seqEntryName.fBuf, &num) != 2) {
            MOCERROR2((errmsg,
                       "invalid sequence "
                       "entry statement \"%s\"\n"
                       "must be of the form "
                       " \"<sequence entry name>\" "
                       " <number>",
                       s));
        }
        theSeq->Add(seqEntryName.fBuf, num);
    }
}

ufCompareResult CompareSequenceEntry(ufPtr(SequenceEntry) * i1,
                                     ufPtr(SequenceEntry) * i2) {
    if ((*i1)->fNum < (*i2)->fNum) {
        return LessThan;
    } else if ((*i1)->fNum > (*i2)->fNum) {
        return GreaterThan;
    }
    return EqualTo;
}

ufCompareResult CompareSequenceRange(ufPtr(SubsequenceRange) * i1,
                                     ufPtr(SubsequenceRange) * i2) {
    int res = strcmp((*i1)->fName, (*i2)->fName);
    if (res < 0) {
        return LessThan;
    } else if (res > 0) {
        return GreaterThan;
    }
    return EqualTo;
}

ufCompareResult CompareSequence(ufPtr(Sequence) * i1, ufPtr(Sequence) * i2) {
    int res = strcmp((*i1)->fName, (*i2)->fName);
    if (res < 0) {
        return LessThan;
    } else if (res > 0) {
        return GreaterThan;
    }
    return EqualTo;
}

void Sequence::Save() {
    FILE *out;

    gFilename = gSequenceLogFile;
    lineno = 1;

    if (!gSequenceLogFile || (out = fopen(gSequenceLogFile, "w")) == 0) {
        MOCERROR2((errmsg,
                   "Can not open sequence "
                   "log file \"%s\"",
                   gSequenceLogFile));
        return;
    }

    if (!fgSequences) {
        fclose(out);
        return;
    }

    fprintf(out, "#\n#\n#This file is generated by the moc\n"
                 "# compiler. It contains the following "
                 "sequences:\n");

    fgSequences->SortInIncreasingOrder(CompareSequence);

    for (ufListIter(Sequence) i0 = *fgSequences; i0.fData != 0; i0++) {
        fprintf(out, "#\t sequence \"%s\"\n", i0.fData->fName);

        ufListIter(SubsequenceRange) j;
        i0.fData->fSortedRanges.SortInIncreasingOrder(CompareSequenceRange);

        for (j = i0.fData->fSortedRanges; j.fData != 0; j++) {
            fprintf(out, "#\t\tsubsequence \"%s\" \n", j.fData->fName);
        }
    }
    fprintf(out, "#\n#\n");

    for (ufListIter(Sequence) i = *fgSequences; i.fData != 0; i++) {
        fprintf(out, "\n\n#\n# Sequence %s\n#\n\n", i.fData->fName);
        fprintf(out, "begin \"%s\" %d %d\n", i.fData->fName, i.fData->fStart,
                i.fData->fEnd);

        ufListIter(SubsequenceRange) j;
        i.fData->fSortedRanges.SortInIncreasingOrder(CompareSequenceRange);

        for (j = i.fData->fSortedRanges; j.fData != 0; j++) {
            fprintf(out,
                    "\tsubsequence \"%s\" %d "
                    "%d\n",
                    j.fData->fName, j.fData->fStart, j.fData->fEnd);
        }

        ufListIter(SequenceEntry) k;
        int lastE = -2;
        i.fData->fSortedEntries.SortInIncreasingOrder(CompareSequenceEntry);
        for (k = i.fData->fSortedEntries; k.fData != 0; k++) {
            if (k.fData->fNum > lastE + 1) {
                fprintf(out, "\n\n");
            }
            lastE = k.fData->fNum;
            fprintf(out, "\t\"%s\" %d\n", k.fData->fName, k.fData->fNum);
        }

        fprintf(out, "end\n");
    }

    fclose(out);
}

void Sequence::Add(const char *e, int num) {
    ufPtr(SequenceEntry) i;
    if (fEntries.Find(e, i)) {
        MOCERROR2((errmsg,
                   "already have an entry \"%s\""
                   " in sequence \"%s\"",
                   e, fName));
        return;
    }

    i = SequenceEntry::New(e, num);

    fEntries.Insert(i->fName, i);
    fSortedEntries.InsertFirst(i);
    return;
}

void Sequence::AddSubsequence(const char *e, int start, int end) {
    ufPtr(SubsequenceRange) i;
    if (fRanges.Find(e, i)) {
        MOCERROR2((errmsg,
                   "already have a subsequence "
                   "\"%s\" in sequence \"%s\"",
                   e, fName));
        return;
    }

    i = SubsequenceRange::New(e, start, end);

    fRanges.Insert(i->fName, i);
    fSortedRanges.InsertFirst(i);

    return;
}

const char *Sequence::Generate(const char *e, char *&sToDel) {
    return GenEntry(e, fStart, fEnd, sToDel);
}

const char *Sequence::GenerateFromSubsequence(const char *subseq, const char *e,
                                              char *&sToDel) {
    ufPtr(SubsequenceRange) i;
    if (strcmp(subseq, "default") == 0) {
        return GenEntry(e, fStart, fEnd, sToDel);
    }
    if (!fRanges.Find(subseq, i)) {
        MOCERROR2((errmsg,
                   "Can't find subsequence \"%s\""
                   " in sequence \"%s\"",
                   subseq, fName));
        return "-1";
    }
    return GenEntry(e, i->fStart, i->fEnd, sToDel);
}

const char *Sequence::GenEntry(const char *e, int start, int end,
                               char *&sToDel) {
    sToDel = 0;

    ufPtr(SequenceEntry) i;
    if (fEntries.Find(e, i)) {
        ufTmpBuf tmp(20);
        sprintf(tmp.fBuf, "%d", i->fNum);
        sToDel = ufStrDup(tmp.fBuf);
        return sToDel;
    }

    int found;
    for (int j = start; j < end; j++) {
        found = 0;
        ufStrHashIter(SequenceEntry) k;

        for (k = fEntries; k.fData != 0; k++) {
            if (k.fData->fNum == j) {
                found = 1;
                break;
            }
        }
        if (!found) {
            Add(e, j);
            return Generate(e, sToDel);
        }
    }
    {
        MOCERROR2((errmsg, "Out of sequence numbers [%d:%d] for \"%s\"", start,
                   end, e));
    }

    return "-1";
}

implement(ufPtr, SequenceEntry);
implement(ufList, SequenceEntry);
implement(ufStrHash, SequenceEntry);

ufPtr(SequenceEntry) SequenceEntry::New(const char *n, int num) {
    return (new SequenceEntry(n, num))->fSelf;
}

SequenceEntry::SequenceEntry(const char *n, int num)
    : fName(ufStrDup(n)), fNum(num), fSelf(this, kSelfPtrShared) {}

SequenceEntry::SequenceEntry(const SequenceEntry &o)
    : fName(ufStrDup(o.fName)), fNum(o.fNum),
      fSelf(this, kSelfPtrShared){ufASSERT(0)}

      SequenceEntry::~SequenceEntry() {
    delete[] fName;
}

implement(ufPtr, SubsequenceRange);
implement(ufList, SubsequenceRange);
implement(ufStrHash, SubsequenceRange);

ufPtr(SubsequenceRange) SubsequenceRange::New(const char *n, int start,
                                              int end) {
    return (new SubsequenceRange(n, start, end))->fSelf;
}

SubsequenceRange::SubsequenceRange(const char *n, int start, int end)
    : fName(ufStrDup(n)), fStart(start), fEnd(end),
      fSelf(this, kSelfPtrShared) {}

SubsequenceRange::SubsequenceRange(const SubsequenceRange &o)
    : fName(ufStrDup(o.fName)), fStart(o.fStart), fEnd(o.fEnd),
      fSelf(this, kSelfPtrShared){ufASSERT(0)}

      SubsequenceRange::~SubsequenceRange() {
    delete[] fName;
}
