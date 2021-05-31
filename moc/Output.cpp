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

static FILE *OpenTemplate();

static void StartBlock(const char *buf, ufList(OutputBlock) &);

static void MidBlock(const char *buf, ufList(OutputBlock) &);

static void CtrlBlock(const char *buf, ufList(OutputBlock) &);

static void LnBlock(const char *buf, ufList(OutputBlock) &);

static void EndBlock(const char *buf, ufList(OutputBlock) &);

void CleanupOutput() {
    OutputBlock::fgTop.MakeEmpty();
    delete Sequence::fgSequences;
    Sequence::fgSequences = 0;
}

void LoadTemplate() {
    ufTmpBuf tmp(2048);

    ufList(OutputBlock) allBlocks;

    FILE *in = OpenTemplate();

    while (FGets(tmp, in)) {

        if (tmp[0] == '@' && tmp[1] != '@' && tmp[1] != '(') {
            ufTmpBufCursor b(tmp, 2);
            b.SkipSpaces();
            
            if (tmp[1] == '{') {
                StartBlock(b, allBlocks);
            } else if (tmp[1] == '}') {
                EndBlock(b, allBlocks);
            } else if (tmp[1] == '|') {
                MidBlock(b, allBlocks);
            } else if (tmp[1] == '>') {
                CtrlBlock(b, allBlocks);
            } else if (tmp[1] != '#') {
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

static void StartBlock(const char *block, ufList(OutputBlock) & allBlocks) {
    if (!allBlocks && gTopOutputBlock != 0) {
        MOCERROR2((errmsg, "You can have only one toplevel block"));
        return;
    }

    ufPtr(OutputBlock) theCurrentBlock;
    ufStringReadonlyCursor b(block);
    ufTmpBuf tokenBuf(100);
    b.SkipSpaces();
    b.ParseNextToken(tokenBuf);
    ufTmpBufCursor token(tokenBuf);

    if (token.IsEqual("open")) {
        theCurrentBlock = FileBlock::New(block);
    } else if (token.IsEqual("if")) {
        theCurrentBlock = IfBlock::New(block);
    } else if (token.IsEqual("foreach")) {
        theCurrentBlock = LoopBlock::New(block);
    } else {
        MOCERROR2((errmsg,
                   "Don't understand beginning "
                   "block directive \"%s\"",
                   block));
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

static void CtrlBlock(const char *block, ufList(OutputBlock) & allBlocks) {
    if (!allBlocks) {
        ufStringReadonlyCursor b(block);
        if (!b.IsBlankLine()) {
            mocError2("found line outside of the "
                      "top block");
            return;
        }
        return;
    }
    ufPtr(OutputBlock) theCurrentBlock = allBlocks.PeekFirst();
    ufPtr(OutputBlock) theControlBlock = ControlBlock::New(block);

    theCurrentBlock->AddBlock(theControlBlock);
}

static void LnBlock(const char *block, ufList(OutputBlock) & allBlocks) {
    if (!allBlocks) {
        ufStringReadonlyCursor b(block);
        if (!b.IsBlankLine()) {
            mocError2("found line outside of the "
                      "top block");
            return;
        }
        return;
    }
    ufPtr(OutputBlock) theCurrentBlock = allBlocks.PeekFirst();
    ufPtr(OutputBlock) theLineBlock = LineBlock::New(block);

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
    : fSelf(this, kSelfPtrShared),
      fFilename(gFilename),
      fLineno(lineno) {
}

OutputBlock::~OutputBlock() {
}

void OutputBlock::SetInput(ufPtr(InputBlock) & p) {
    fInput = p;
}

void OutputBlock::WriteMessage(const char *msg, MessageOutputLevel level) {
    gParser = kOUTPUT_PARSER;

    int l = msg != 0 ? strlen(msg) : 0;
    ufTmpBuf tmp2(200 + l);

    EvalString(tmp2, msg, fgPos);

    const char *levelAsString = "";
    if (level == kErrorMessage) {
        levelAsString = "ERROR:";
    } else if (level == kWarningMessage) {
        levelAsString = "WARNING: ";
    }

    ufTmpBuf tmp(200 + tmp2.GetSize());
    if (fInput != 0) {
        gFilename = fInput->fFilename.Literal();
        lineno = fInput->fLineno;
        sprintf(tmp.fBuf, "processing input %s(\"%s\")\n%s:%d: %s %s",
                fInput->GetClassName(), fInput->fName.CStr(),
                fFilename.CStr(), fLineno,
                levelAsString, tmp2.fBuf);
    } else {
        gFilename = fFilename.Literal();
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

const char *OutputBlock::GetClassName() { return "OutputBlock"; }

BOOL OutputBlock::IsA(int) { return FALSE; }

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

BOOL OutputBlock::CanOutput() { return TRUE; }

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
            tmp[n] = '\0';

            Resolve(tmp.fBuf, theInput);

            base = v + 1;
            return theInput;
        }
    }

    base = var;

    return fInput;
}

BOOL OutputBlock::SetKeyValue(const char *var, const char *val) {
    ufString expandedVar;
    const char *rest;
    expandedVar.MakeStringLiteral(var);
    ufStringReadonlyCursor v(expandedVar);

    if (v.Contains('@')) {
        Eval(var, expandedVar);
    }
    ufPtr(InputBlock) theInput = GetInput(var, rest);

    if (theInput != 0) {
        theInput->SetKeyValue(rest, val);
        return TRUE;
    }
    return FALSE;
}

BOOL OutputBlock::Eval(const char *key, ufString& resultToOwn) {
    ufStringReadonlyCursor var(key);
    var.SkipSpaces();

    gFilename = fFilename.Literal();
    lineno = fLineno;

    if (var.IsEmpty()) {
        return FALSE;
    }

    if (var.IsEqual("OUTPUT_FILENAME")) {
        // gOutputFilename can change so don't use a literal
        resultToOwn = gOutputFilename;
        return TRUE;
    }
    if (var.IsEqual("OUTPUT_LINENO")) {
        // gOutputLinenoBuf can change so don't use a literal
        sprintf(gOutputLinenoBuf, "%d", gOutputLineno);
        resultToOwn = gOutputLinenoBuf;
        return TRUE;
    }
    if (var.IsEqual("OUTPUT_NEXT_LINENO")) {
        // gOutputLinenoBuf can change so don't use a literal
        sprintf(gOutputLinenoBuf, "%d", gOutputLineno + 1);
        resultToOwn = gOutputLinenoBuf;
        return TRUE;
    }

    if (*var == ':' && var[1] != ':') {
        return FormattedEval(var + 1, resultToOwn);
    }

    const char *rest;
    ufPtr(InputBlock) theInput = GetInput(var, rest);

    BOOL found = FALSE;
    if (theInput != 0) {
        found = theInput->Eval(rest, resultToOwn);
    }

    if (!found && fParent != 0) {
        found = fParent->Eval(var, resultToOwn);
    }

    return found;
}

typedef BOOL (OutputBlock::*FormatFncPtr)(const char *, ufString&);

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

BOOL OutputBlock::FormattedEval(const char *key, ufString& resultToOwn) {
    resultToOwn.MakeEmpty();
    ufStringReadonlyCursor var(key);
    var.SkipSpaces();

    gFilename = fFilename.Literal();
    lineno = fLineno;

    FormatFunctionPair *p;

    for (p = gFormatFunctions; p->fName; p++) {
        if (var.IsEqual(p->fName, p->fLen) && !isalnum(var[p->fLen])) {
            var += p->fLen;
            var.SkipSpaces();

            return (this->*(p->fFunc))(var, resultToOwn);
        }
    }

    return 0;
}

BOOL OutputBlock::EvalIf(const char *key, ufString &resultToOwn) {
    resultToOwn.MakeEmpty();
    ufStringReadonlyCursor v(key);

    v.SkipSpaces();

    const int LEN = strlen(v);
    const char * const START = v;
    const char * const END = v + LEN;

    const char *ifend = START;
    const char *thenstart = START;
    const char *thenend = END;
    const char *elsestart = END;
    const char *elseend = END;

    int pos;
    int level = 0;
    int quote = 0;
    for (; *v; v+=1) {
        if (*v == '(') {
            level++;
        }
        if (*v == ')') {
            level = level > 0 ? level - 1 : 0;
        } else if (*v == '\\' && v[1] != '\0') {
            v+=1;
            continue;
        } else if (*v == '"') {
            quote = (quote ? 0 : 1);
        } else if (v.StartsWith(":else") && level == 0 && quote == 0) {
            elsestart = (v[5] != '\0' && v[6] != '\0' ? v + 6 : END);
            thenend = v;
            break;
        } else if (v.StartsWith(":then") && level == 0 && quote == 0) {
            thenstart = (v[5] != '\0' && v[6] != '\0' ? v + 6 : END);
            ifend = v;
        }
    }
    ufTmpBuf tmp(ifend - START + 2);
    if (ifend > START) {
        strncpy(tmp.fBuf, START, ifend - START);
    }
    tmp[ifend - START] = '\0';

    ufTmpBuf tmp2(202);
    if (EvalIfCriteria(tmp.fBuf) != 0) {
        if (thenend > thenstart) {
            tmp.Copy( thenstart, thenend - thenstart );
            EvalString(tmp2, tmp.fBuf, pos);
            resultToOwn = tmp2.fBuf;
        } else {
            resultToOwn.MakeEmpty();
        }
    } else {
        if (elseend > elsestart) {
            tmp.Copy(elsestart, elseend - elsestart);
            EvalString(tmp2, tmp.fBuf, pos);
            resultToOwn = tmp2.fBuf;
        } else {
            resultToOwn.MakeEmpty();
        }
    }

    return TRUE;
}

BOOL OutputBlock::EvalDefaultSubst(const char *key,
                                   ufString& resultToOwn) {
    ufStringReadonlyCursor ro(key);
    ro.SkipSpaces();
    const int LEN = strlen(ro);
    ufTmpBuf tmp(20 + LEN);
    strcpy(tmp.fBuf, ro);

    ufTmpBufCursor v(tmp);
    v.SkipTo(',');
    *v++ = '\0';
    
    resultToOwn.MakeEmpty();
    if ( Eval(tmp.fBuf, resultToOwn) ) {
        return TRUE;
    }

    ufTmpBuf tmp2(tmp.GetSize() + 100);
    int pos;
    EvalString(tmp2, v, pos);
    resultToOwn = tmp2.fBuf;
    return TRUE;
}

BOOL OutputBlock::EvalNth(const char *key,
                          ufString& resultToOwn) {
    ufStringReadonlyCursor var(key);
    ufTmpBuf list(202);
    ufTmpBuf field(202);
    int nth;

    resultToOwn.MakeEmpty();
    if (sscanf(var, " %d %s %[^,] ", &nth, list.fBuf, field.fBuf) != 3) {
        ufTmpBuf tmp(2002);
        sprintf(tmp.fBuf, "invalid nth statement. Must be"
                          " of the form:\n\t@@(:nth <index> <list>"
                          " <variable>\n");
        Error(tmp.fBuf);
        return FALSE;
    }
    const char *rest;
    ufPtr(InputBlock) theInput = GetInput(list.fBuf, rest);
    if (theInput != 0) {
        if (!theInput->HasList(rest)) {
            return GetNthDefault(var, resultToOwn);
        }
        InputBlockIterator *iter = theInput->CreateIterator(rest);
        if (!iter || iter->GetTotal() <= nth) {
            delete iter;
            return GetNthDefault(var, resultToOwn);
        }
        while (nth > 0) {
            (*iter)++;
            nth--;
        }
        BOOL wasFound = iter->fData->Eval(field.fBuf, resultToOwn);
        delete iter;
        if (!wasFound) {
            var = key;
            var.SkipTo(',');
            resultToOwn = var;
        }
        return TRUE;
    }

    return GetNthDefault(var, resultToOwn);
}

BOOL OutputBlock::EvalLength(const char *key,
                             ufString& resultToOwn) {
    ufStringReadonlyCursor var(key);
    ufTmpBuf list(202);

    resultToOwn.MakeEmpty();
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
            resultToOwn.MakeStringLiteral("0");
            return TRUE;
        }
        InputBlockIterator *iter = theInput->CreateIterator(rest);
        if (!iter) {
            resultToOwn.MakeStringLiteral("0");
            return TRUE;
        }
        ufTmpBuf tmp(2002);
        sprintf(tmp.fBuf, "%d", iter->GetTotal());
        delete iter;
        resultToOwn = tmp.fBuf;
        return TRUE;
    }

    resultToOwn.MakeStringLiteral("0");
    return TRUE;
}

BOOL OutputBlock::GetNthDefault(const char *var, ufString& resultToOwn) {
    ufStringReadonlyCursor ro(var);

    if (ro.SkipTo(',') == ',') {
        resultToOwn = ro + 1;
        return TRUE;
    }

    resultToOwn.MakeEmpty();
    return TRUE;
}

BOOL OutputBlock::EvalColumnFill(const char *key,
                                 ufString& resultToOwn) {
    ufStringReadonlyCursor var(key);
    ufTmpBuf tmp(202);
    int pos;

    if (sscanf(var, "%d", &pos) != 1 || pos > 200 || pos < 1) {
        Error("invalid fill "
              "format. \nMust be of the "
              "format:\n@@(:fill <column number>) "
              "column number is between 0 and 200\n");
        resultToOwn.MakeEmpty();
        return FALSE;
    }

    if (pos > fgPos) {
        memset(tmp.fBuf, ' ', pos - fgPos);
        tmp[pos - fgPos] = '\0';
        resultToOwn = tmp.fBuf;
        return TRUE;
    }

    resultToOwn.MakeEmpty();
    return TRUE;
}

BOOL OutputBlock::EvalGenerateSequenceNumber(const char *key,
                                             ufString& resultToOwn) {
    ufStringReadonlyCursor var(key);
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
                var.Buf(), tmp.fBuf);
        Error(err.fBuf);
        resultToOwn.MakeEmpty();
        return FALSE;
    }

    gFilename = fFilename;
    lineno = fLineno;
    ufPtr(Sequence) theSeq = Sequence::Find(seq.fBuf);

    if (theSeq != 0) {
        if (num == 2) {
            return theSeq->Generate(t1.fBuf, resultToOwn);
        } else {
            return theSeq->GenerateFromSubsequence(t1.fBuf, t2.fBuf, resultToOwn);
        }
    }

    return 0;
}

BOOL GetNextWord(ufTmpBufCursor& var,
                 int wordCount,
                 ufTmpBuf& tmp) {
    tmp[0] = '\0';

    // skip leading spaces and underscores
    if (var.SkipSpacesAnd(' ', '_') == '\0') {
        return FALSE;
    }

    int len = strlen(var);

    char* s = tmp.fBuf;
    char* e = tmp.fBuf + tmp.GetSize() - 1;

    // go to the first lowercased char or '_'
    while (!var.IsLower() && *var != '_' && s != e
           && !var.IsEmpty()) {
      *s++ = *var++;
    }
    // go to the first uppercased char or '_'
    while (!var.IsUpper() && !var.IsEqual('_') && s != e
           && !var.IsEmpty()) {
      *s++ = *var++;
    }
    // this is a word component
    *s = '\0';
    ufTmpBufCursor t(tmp);
    t.TrimSpaces();
    return tmp[0] != '\0'? TRUE : FALSE;
}

BOOL GetAllWords(ufTmpBufCursor& var,
                 int wordCount,
                 ufTmpBuf& tmp) {
    if (tmp.GetSize() < 1) {
        return FALSE;
    }
    tmp[0] = '\0';

    if (var.SkipSpaces() == '\0') {
        return FALSE;
    }

    int i = 0;
    for (; !var.IsEmpty() ; i++) {
        tmp[i] = *var++;
    }
    tmp[i] = '\0';
    return TRUE;
}

BOOL GetOneThenAllWords(ufTmpBufCursor& var,
                        int wordCount,
                        ufTmpBuf& tmp) {
    return wordCount == 0
        ? GetNextWord(var, wordCount, tmp)
        : GetAllWords(var, wordCount, tmp);
}

void UppercasePrefix(ufTmpBuf& dest, int wordCounter, ufTmpBuf& src) {
    dest.Reallocate(src.GetSize() + 1);

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
    dest.Reallocate(src.GetSize() + 1);

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
    dest.Reallocate(src.GetSize() + 1);

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
    dest.Reallocate(src.GetSize() + 1);

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
    dest.Reallocate(src.GetSize() + 1);

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
    dest.Reallocate(src.GetSize() + 1);

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

BOOL OutputBlock::ApplyFuncToWordComponents(
                 const char* formatterName,
                 const char* rawVar,
                 ufString&   resultToOwn,
                 WordTokenizer nextWord,
                 WordFormatter reformatWord) {
    ufTmpBuf var(2000);
    int pos;
    EvalString(var, rawVar, pos);
    if (var.IsEmpty()) {
        resultToOwn.MakeEmpty();
        return TRUE;
    }

    ufTmpBufCursor v(var);
    int len = var.GetLength();

    ufTmpBuf dest(2 * len + 10);
    ufTmpBuf word(len + 1);
    ufTmpBuf reformattedWord(dest.GetSize());

    BOOL isDone = FALSE;

    ufTmpBufCursor d(dest);
    *d = '\0';

    int wordCount = -1;
    while (TRUE) {
        ++wordCount;
        if (!nextWord(v, wordCount, word)) {
            resultToOwn = dest.fBuf;
            return TRUE;
        }
        reformattedWord[0] = '\0';
        reformatWord(reformattedWord, wordCount, word);
        const char *s = reformattedWord.fBuf;
        for (; *s != '\0' ; *d++ = *s++);
        *d = '\0';
    };
    *d = '\0';
    resultToOwn = dest.fBuf;
    return TRUE;
}

BOOL OutputBlock::EvalUpperCasePrefix(const char *var,
                                      ufString& resultToOwn) {
    return ApplyFuncToWordComponents("uppercase-prefix",
                                     var,
                                     resultToOwn,
                                     GetOneThenAllWords,
                                     UppercasePrefix);
}

BOOL OutputBlock::EvalUpperCaseAll(const char *var,
                                   ufString& resultToOwn) {
    return ApplyFuncToWordComponents("uppercase-all",
                                     var,
                                     resultToOwn,
                                     GetAllWords,
                                     UppercaseAll);
}

BOOL OutputBlock::EvalUpperCaseFirst(const char *key,
                                     ufString& resultToOwn) {
    ufStringReadonlyCursor var(key);
    ufTmpBuf tmp(2000);
    int pos;

    EvalString(tmp, var, pos);
    if (*tmp.fBuf == '\0') {
        resultToOwn.MakeEmpty();
        return TRUE;
    }

    int len = strlen(tmp.fBuf);

    ufTmpBuf uc(2 * len + 10);
    uc[0] = (islower(tmp[0]) ? toupper(tmp[0]) : tmp[0]);
    strcpy(uc.fBuf + 1, tmp.fBuf + 1);

    resultToOwn = uc.fBuf;
    return TRUE;
}

BOOL OutputBlock::EvalLowerCaseAll(const char *var,
                                   ufString& resultToOwn) {
    return ApplyFuncToWordComponents("lowercase-all",
                                     var,
                                     resultToOwn,
                                     GetAllWords,
                                     LowercaseAll);
}

BOOL OutputBlock::EvalSnakeCase(const char *var,
                                ufString& resultToOwn) {
    return ApplyFuncToWordComponents("snake-case",
                                     var,
                                     resultToOwn,
                                     GetNextWord,
                                     SnakeCase);
}

BOOL OutputBlock::EvalCamelCase(const char *var,
                                ufString& resultToOwn) {
    return ApplyFuncToWordComponents("camelCase",
                                     var,
                                     resultToOwn,
                                     GetNextWord,
                                     CamelCase);
}

BOOL OutputBlock::EvalPascalCase(const char *var,
                                 ufString& resultToOwn) {
    return ApplyFuncToWordComponents("PascalCase",
                                     var,
                                     resultToOwn,
                                     GetNextWord,
                                     PascalCase);
}

BOOL OutputBlock::EvalFormatList(const char *key,
                                 ufString& resultToOwn) {
    ufStringReadonlyCursor var(key);
    ufASSERT(*var == ':');
    var++;
    var.SkipSpaces();

    SepType sepType = kNoSep;
    char sepChar = ' ';

    resultToOwn.MakeEmpty();
    if (!var.IsEqual(':')) {
        if (var.StartsWith("prefix-sep")) {
            var += 10;
            sepType = kPrefixSep;
        } else if (var.StartsWith("suffix-sep")) {
            var += 10;
            sepType = kSuffixSep;
        } else {
            // TBD: error
            return FALSE;
        }

        var.SkipSpaces();

        if (!var.IsEqual('=')) {
            // TBD: error
            return FALSE;
        }
        var++;

        var.SkipSpaces();

        if (!var.IsEqual('\'')) {
            // TBD: error
            return FALSE;
        }
        var++;

        if (var.IsEqual('\\')) {
            var++;
        }

        sepChar = *var;
        var++;

        if (!var.IsEqual('\'')) {
            // TBD: error
            return FALSE;
        }
        var++;
        var.SkipSpaces();

        if (!var.IsEqual(':')) {
            // TBD: error
            return FALSE;
        }
        var++;

        var.SkipSpaces();
    }

    ufTmpBuf controlString(1024);
    ufTmpBuf outputString(1024);

    MultiLineFormatter lblock;
    while (!var.IsEmpty()) {
        int i;
        for (i = 0; !var.IsEmpty() && !var.IsEqual(':'); var++, i++) {
            if (var.IsEqual('\\') && var[1] != '\0') {
                var++; // skip to next char
            }
            controlString[i] = *var;
        }
        controlString[i] = '\0';
        if (*var == ':') {
            var++;
        }

        var.SkipSpaces();

        int level = 1;
        for (i = 0; *var != '\0' && *var != ':' && level > 0; var++, i++) {
            if (*var == '\\' && var[1] != '\0') {
                outputString[i] = *++var;
                continue;
            }

            if (*var == '(') {
                level++;
            } else if (*var == ')') {
                level--;
            }

            outputString[i] = *var;
        }
        outputString[i] = '\0';

        ufTmpBufCursor output(outputString);
        output.TrimSpaces();

        lblock.AddList(controlString.fBuf, output);

        if (*var == ':') {
            var++;
        }

        var.SkipSpaces();
    }

    BOOL isFirst = TRUE;
    lblock.fParent = fSelf;
    lblock.SetInput(fInput);
    lblock.WriteOutputToString(sepType, sepChar, fgPos, outputString, isFirst);

    resultToOwn = outputString.fBuf;

    return TRUE;
}

BOOL OutputBlock::IsFirst(const char *var) {
    BOOL res = TRUE;
    if (fParent != 0) {
        res = fParent->IsFirst(var);
    }

    return res;
}

BOOL OutputBlock::IsLast(const char *var) {
    BOOL res = TRUE;
    if (fParent != 0) {
        res = fParent->IsLast(var);
    }

    return res;
}

BOOL OutputBlock::HasEmptyList(const char *var) {
    BOOL hasListButIsEmpty = FALSE;
    if (var == 0) {
        return hasListButIsEmpty;
    }

    if (*var == ':' && var[1] != ':') {
        ufASSERT(0);
        // BAD
        return hasListButIsEmpty;
    }

    const char *rest;
    ufPtr(InputBlock) theInput = GetInput(var, rest);

    BOOL found = FALSE;

    if (theInput != 0) {
        found = theInput->HasList(rest);
        if (found) {
            hasListButIsEmpty = !theInput->HasNonEmptyList(rest);
        }
    }

    if (!found && fParent != 0) {
        hasListButIsEmpty = fParent->HasEmptyList(var);
    }

    return hasListButIsEmpty;
}

BOOL OutputBlock::HasParent(const char *var) {
    BOOL hasParent = FALSE;

    if (var == 0) {
        return hasParent;
    }

    if (*var == ':' && var[1] != ':') {
        ufASSERT(0);
        // BAD
        return hasParent;
    }

    const char *rest;
    ufPtr(InputBlock) theInput = GetInput(var, rest);

    if (theInput != 0) {
        hasParent = theInput->HasParent(var);
    }

    if (!hasParent && fParent != 0) {
        hasParent = fParent->HasParent(var);
    }

    return hasParent;
}

BOOL OutputBlock::IsFromCurrentModule(const char *v) {
    if (fInput != 0) {
        return fInput->IsFromCurrentModule();
    }

    return fParent->IsFromCurrentModule(v);
}

BOOL OutputBlock::Resolve(const char *var, ufPtr(InputBlock) & p) {
    BOOL res = FALSE;
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
    ufStringReadonlyCursor s(string);

    for (; !s.IsEmpty(); s++) {
        if ((s[0] == '\\' && s[1] == '@') || (s[0] == '@' && s[1] == '@')) {
            s++;
            tmp[i++] = *s;
            /*current*/pos = s.CalculateNewPos(/*previous */pos);
        } else if (*s == '@') {
            if (*++s == '(') {
                SubstituteVariable(tmp, s, i, pos);
            } else {
                ufTmpBuf errmsg(100 + strlen(string));
                sprintf(errmsg.fBuf,
                        "Expected variable of "
                        "the form @@(name) in input '%s'",
                        s.Buf());
                Error(errmsg.fBuf);
            }
        } else {
            tmp[i++] = *s;
            /*current*/pos = s.CalculateNewPos(/*previous*/pos);
        }
    }

    tmp[i] = '\0';
}

void OutputBlock::SubstituteVariable(ufTmpBuf &tmp,
                                     ufStringReadonlyCursor& s, int &i,
                                     int &pos) {
    ufTmpBuf tmp2(1024);
    char *b = tmp2.fBuf;
    int level = 1;
    s++;

    // TBD: need to eventually handle nested variables
    for (; !s.IsEmpty() && level != 0; s++) {
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
    ufString result;
    if (Eval(tmp2.fBuf, result)) {
        ufStringReadonlyCursor value(result);
        for (; !value.IsEmpty(); value++) {
            tmp[i++] = *value;
            /*current*/pos = value.CalculateNewPos(/*previous*/pos);
        }
    } else {
        ufTmpBuf tmp3(100 + strlen(tmp2.fBuf));
        sprintf(tmp3.fBuf,
                "Can't expand variable "
                "\"%s\"\n",
                tmp2.fBuf);
        Error(tmp3.fBuf);
    }

    tmp[i] = '\0';
}

BOOL OutputBlock::DoesExist(const char *v) {
    ufString result;
    BOOL wasFound = Eval(v, result);

    return wasFound && !result.IsEmpty();
}

BOOL OutputBlock::EvalControlCriteria(const char *criteria) {
    ufStringReadonlyCursor controlstatement(criteria);
    gOutputBlock = fSelf;
    BOOL res = FALSE;

    ufTmpBuf tmp(1024);
    int pos = 0;

    if (controlstatement.IsEqual("error", 5)) {
        EvalString(tmp, controlstatement + 5, pos);
        Error(controlstatement);
    } else if (controlstatement.IsEqual("warning", 7)) {
        EvalString(tmp, controlstatement + 7, pos);
        Warning(controlstatement);
    } else if (controlstatement.IsEqual("message", 7)) {
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

BOOL ControlEvalSetString(char *input, char *s) {
    ufString key;
    key.TakeOwnership(input);
    ufString value;
    value.TakeOwnership(s);

    return gOutputBlock->SetKeyValue(key, value);
}

BOOL ControlEvalSetInteger(char *input, int i) {
    ufString key;
    key.TakeOwnership(input);

    char buf[20];
    sprintf(buf, "%d", i);

    return gOutputBlock->SetKeyValue(key, buf);
}

BOOL IfEvalExists(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    ufString key;
    key.TakeOwnership(input);

    ufString value;
    value.TakeOwnership(s);

    return gOutputBlock->DoesExist(value);
}

BOOL IfEvalEmpty(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    ufString key;
    key.TakeOwnership(input);

    ufString value;
    value.TakeOwnership(s);
    return gOutputBlock->HasEmptyList(value);
}

BOOL IfEvalIsFromCurrentModule(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    ufString key;
    key.TakeOwnership(input);

    ufString value;
    value.TakeOwnership(s);
    return gOutputBlock->IsFromCurrentModule(value);
}

BOOL IfEvalIsA(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    ufString key;
    key.TakeOwnership(input);

    ufString value;
    value.TakeOwnership(s);

    return gOutputBlock->HasParent(value);
}

BOOL IfEvalFirst(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    ufString key;
    key.TakeOwnership(input);

    ufString value;
    value.TakeOwnership(s);
    return gOutputBlock->IsFirst(value);
}

BOOL IfEvalLast(char *input, char *s) {
    // TBD: use the input to determine which input block
    // is being referenced.
    ufString key;
    key.TakeOwnership(input);

    ufString value;
    value.TakeOwnership(s);

    return gOutputBlock->IsLast(value);
}

BOOL IfEvalStrequal(char *input, char *s1, char *s2) {
    // TBD: use the input to determine which input block
    // is being referenced.
    ufString key;
    key.TakeOwnership(input);

    ufString value1;
    value1.TakeOwnership(s1);
    ufString value2;
    value2.TakeOwnership(s2);

    return value1 == value2;
}

LineBlock::LineBlock(const char *string) : fLine(ufStrDup(string)) {
}

LineBlock::~LineBlock() {
}

const char *LineBlock::GetClassName() { return "LineBlock"; }

BOOL LineBlock::IsA(int t) {
    return t == kLineBlock;
}

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
ControlBlock::ControlBlock(const char *string) :
    fDirective(string) {
}

ControlBlock::~ControlBlock() {
}

const char *ControlBlock::GetClassName() { return "ControlBlock"; }

BOOL ControlBlock::IsA(int t) {
    return t == kControlBlock;
}

ufPtr(OutputBlock) ControlBlock::New(const char *string) {
    return (new ControlBlock(string))->fSelf;
}

void ControlBlock::GenerateOutput() {
    EvalControlCriteria(fDirective);
}

ufPtr(OutputBlock) BasicBlock::New() {
    return (new BasicBlock())->fSelf;
}

BasicBlock::BasicBlock() {}

BasicBlock::~BasicBlock() {}

const char *BasicBlock::GetClassName() { return "BasicBlock"; }

BOOL BasicBlock::IsA(int t) { return t == kBasicBlock; }

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

IfCaseBlock::IfCaseBlock(const char *block) : fUseNot(0) {
    ufStringReadonlyCursor c(block);
    c.SkipSpaces();

    if (c[0] == 'i' && c[1] == 'f') {
        c += 2;
    }

    c.SkipSpaces();

    if (c.IsEmpty()) {
        fCriteria.MakeStringLiteral("1");
    } else {
        ufTmpBuf tmp(40);
        tmp.Copy(c);
        ufTmpBufCursor c2(tmp);
        c2.TrimSpaces();
        fCriteria = c2;
    }
}

IfCaseBlock::~IfCaseBlock() {
}

const char *IfCaseBlock::GetClassName() { return "IfCaseBlock"; }

BOOL IfCaseBlock::IsA(int t) { return t == kIfCaseBlock; }

BOOL IfCaseBlock::CanOutput() {
    return EvalIfCriteria(fCriteria) != 0;
}

ufPtr(OutputBlock) IfBlock::New(const char *ifCriteria) {
    return (new IfBlock(ifCriteria))->fSelf;
}

IfBlock::IfBlock(const char *ifCriteria) {
    ufPtr(OutputBlock) theFirstCase = IfCaseBlock::New(ifCriteria);

    theFirstCase->fParent = fSelf;
    fIfCaseBlocks.InsertLast(theFirstCase);
}

IfBlock::~IfBlock() {}

const char *IfBlock::GetClassName() { return "IfBlock"; }

BOOL IfBlock::IsA(int t) { return t == kIfBlock; }

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
    ufStringReadonlyCursor d(directive);
    d.SkipSpaces();
    d.ParseNextToken(token);

    if (token == "else") {
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
    fIterVar.MakeEmpty();
    fListVar.MakeEmpty();
    delete fIterator;
    fIterator = 0;

    ufTmpBuf criteria(100);
    criteria.Copy(loopCriteria);
    ufTmpBufCursor cursor(criteria);
    cursor.TrimSpaces();

    ufTmpBuf tokenBuf(100);

    cursor.ParseNextToken(tokenBuf);

    if (tokenBuf != "foreach") {
        IllFormed(loopCriteria);
        return;
    }

    // $iter-name
    ufTmpBuf iterVar(100);
    cursor.ParseNextToken(iterVar);

    // in
    cursor.ParseNextToken(tokenBuf);
    if (tokenBuf != "in" || cursor.IsEmpty()) {
        IllFormed(loopCriteria);
        return;
    }

    // $list1 [$list2 ...]
    cursor.SkipSpaces();

    fIterVar = iterVar.fBuf;
    fListVar = cursor;
}

LoopBlock::~LoopBlock() {
    delete fIterator;
    fIterator = 0;
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

const char *LoopBlock::GetClassName() { return "LoopBlock"; }

BOOL LoopBlock::IsA(int t) { return t == kLoopBlock; }

BOOL LoopBlock::Eval(const char *key, ufString& resultToOwn) {
    resultToOwn.MakeEmpty();

    ufStringReadonlyCursor var(key);
    var.SkipSpaces();
    if (var.IsEmpty()) {
        return FALSE;
    }

    if (*var == ':' && var[1] != ':') {
        return FormattedEval(var + 1, resultToOwn);
    }

    BOOL wasFound = FALSE;
    if (fIterator != 0 && fIterator->fData != 0) {
        wasFound = fIterator->fData->Eval(var, resultToOwn);
    }
    if (!wasFound || resultToOwn.IsEmpty()) {
        wasFound = BasicBlock::Eval(var, resultToOwn);
    }

    return wasFound;
}

BOOL LoopBlock::IsFirst(const char *key) {
    ufStringReadonlyCursor var(key);
    var.SkipSpaces();
    if (var.IsEqual(fIterVar)) {
        if (fIterator) {
            return fIterator->IsFirst();
        }
        return FALSE;
    }
    return OutputBlock::IsFirst(var);
}

BOOL LoopBlock::IsLast(const char *key) {
    ufStringReadonlyCursor var(key);
    var.SkipSpaces();
    if (var.IsEqual(fIterVar)) {
        if (fIterator) {
            return fIterator->IsLast();
        }
        return FALSE;
    }
    return OutputBlock::IsLast(var);
}

BOOL LoopBlock::HasEmptyList(const char *key) {
    ufStringReadonlyCursor var(key);
    int value = 0;

    var.SkipSpaces();
    if (var.IsEmpty()) {
        return FALSE;
    }

    if (*var == ':' && var[1] != ':') {
        ufASSERT(0);
        return FALSE;
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

BOOL LoopBlock::Resolve(const char *key, ufPtr(InputBlock) & p) {
    ufStringReadonlyCursor n(key);
    p.MakeEmpty();

    n.SkipSpaces();
    if (n.IsEqual(fIterVar)) {
        if (fIterator != 0 && fIterator->fData != 0) {
            p = fIterator->fData;
        }

        return TRUE;
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

BOOL LoopBlock::GetNextListVar(ufTmpBuf &tmp, int i) {
    ufStringReadonlyCursor l(fListVar);
    do {
        if (! l.ParseNextToken(tmp)) {
            // failed to get token
            return FALSE;
        }
    } while (--i >= 0);
    
    return TRUE;
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
        if (tmp[0] != pathSep && tmp[0] != altPathSep) {
            tmp2.Copy(gOutputDirectory);
            int n = tmp2.GetLength(); // strlen()
            if (tmp2[n - 1] != pathSep && tmp2[n - 1] != altPathSep) {
                tmp2.Concat("/");
            }
        } else {
            tmp2.Copy("");
        }
        tmp2.ConcatBuf(tmp);
        fFilename = tmp2.fBuf;
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

FileBlock::~FileBlock() {
}

const char *FileBlock::GetClassName() { return "FileBlock"; }

BOOL FileBlock::IsA(int t) { return t == kFileBlock; }

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

BOOL FileBlock::Resolve(const char *key, ufPtr(InputBlock) & p) {
    p.MakeEmpty();
    ufStringReadonlyCursor n(key);
    if (n.IsEqual("file")) {
        p = (ufPtr(InputBlock) &)fFileKeyValues;

        return TRUE;
    }

    return OutputBlock::Resolve(n, p);
}

void FileBlock::MakeDir(const char *filename) {
    for (int n = strlen(filename); n >= 0; n--) {
        if (filename[n] == gDirectorySeparator) {
            ufTmpBuf tmp(n + 10);
            strncpy(tmp.fBuf, filename, n);
            tmp[n] = '\0';
            MakeParentDir(tmp.fBuf);
            break;
        }
    }
}

void FileBlock::MakeParentDir(const char *d) {
    ufStringReadonlyCursor dirspec(d);
    if (dirspec.IsEqual(".") || dirspec.IsEmpty()) {
        return;
    }
    for (int n = strlen(dirspec); n >= 0; n--) {
        if (dirspec[n] == gDirectorySeparator) {
            ufTmpBuf tmp(n + 10);
            strncpy(tmp.fBuf, dirspec, n);
            tmp[n] = '\0';
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

const char *MultiLineFormatter::GetClassName() { return "MultiLineFormatter"; }

BOOL MultiLineFormatter::IsA(int t) {
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
    tmp[0] = '\0';

    delete fIterator;
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
            tmp2[0] = '\0';
            while (count != 0 && pos < startPos) {
                tmp2[pos++] = ' ';
            }
            tmp2[pos] = '\0';
            tmp.ConcatBuf(tmp2);

            tmp2[0] = '\0';
            if (t == kPrefixSep && count != 0) {
                strncpy(tmp2.fBuf, sepString, 3);
                pos++;
            }
            tmp.ConcatBuf(tmp2);

            tmp2[0] = '\0';
            EvalString(tmp2, fLists[i].fString, pos);
            count++;
            tmp.ConcatBuf(tmp2);

            if ((count != total) && t == kSuffixSep) {
                tmp2[0] = '\0';
                EvalString(tmp2, sepString, pos);
                tmp.ConcatBuf(tmp2);
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
    : fName(n), fStart(10), fEnd(64000), fEntries(),
      fSelf(this, kSelfPtrShared) {
    Register(fSelf);
}

Sequence::Sequence(const Sequence &o)
    : fName(o.fName), fStart(10), fEnd(64000), fEntries(),
      fSelf(this, kSelfPtrShared) {
    ufASSERT(0)
}

Sequence::~Sequence() {
}

void Sequence::Register(ufPtr(Sequence) & a) {
    if (fgSequences == 0) {
        fgSequences = new ufList(Sequence);
    }

    fgSequences->InsertLast(a);
}

ufPtr(Sequence) Sequence::Find(const char *name) {
    ufPtr(Sequence) theSeq;

    if (fgSequences == 0) {
        fgSequences = new ufList(Sequence);
    }

    ufListIter(Sequence) i;
    ufStringReadonlyCursor n(name);
    for (i = *fgSequences; i.fData != 0; i++) {
        if (n.IsEqual(i.fData->fName)) {
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

    while (FGets(tmp, in)) {
        ufTmpBufCursor s(tmp);
        s.SkipSpaces();

        if (s.IsEmpty() || s.IsEqual('#')) {
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
                       s.Buf()));
        }

        ufPtr(Sequence) theSeq = Sequence::New(seqName.fBuf);

        theSeq->fStart = start;
        theSeq->fEnd = stop;
        ReadSequence(theSeq, in);
    }
    fclose(in);
}

static void ReadSequence(ufPtr(Sequence) & theSeq, FILE *in) {
    ufTmpBuf tmp(200);
    ufTmpBuf seqEntryName(1024);
    int num;

    while (FGets(tmp, in)) {
        ufTmpBufCursor s(tmp);
        s.SkipSpaces();
        if (s.IsEmpty() || *s == '#') {
            continue;
        }

        if (s.StartsWith("end")) {
            break;
        }
        if (s.StartsWith("subsequence")) {
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
                          s.Buf()));
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
                       s.Buf()));
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
    int res = (*i1)->fName.Compare((*i2)->fName);
    if (res < 0) {
        return LessThan;
    } else if (res > 0) {
        return GreaterThan;
    }
    return EqualTo;
}

ufCompareResult CompareSequence(ufPtr(Sequence) * i1, ufPtr(Sequence) * i2) {
    int res = (*i1)->fName.Compare((*i2)->fName);
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
        fprintf(out, "#\t sequence \"%s\"\n", i0.fData->fName.CStr());

        ufListIter(SubsequenceRange) j;
        i0.fData->fSortedRanges.SortInIncreasingOrder(CompareSequenceRange);

        for (j = i0.fData->fSortedRanges; j.fData != 0; j++) {
            fprintf(out, "#\t\tsubsequence \"%s\" \n", j.fData->fName.CStr());
        }
    }
    fprintf(out, "#\n#\n");

    for (ufListIter(Sequence) i = *fgSequences; i.fData != 0; i++) {
        fprintf(out, "\n\n#\n# Sequence %s\n#\n\n", i.fData->fName.CStr());
        fprintf(out, "begin \"%s\" %d %d\n",
                i.fData->fName.CStr(), i.fData->fStart,
                i.fData->fEnd);

        ufListIter(SubsequenceRange) j;
        i.fData->fSortedRanges.SortInIncreasingOrder(CompareSequenceRange);

        for (j = i.fData->fSortedRanges; j.fData != 0; j++) {
            fprintf(out,
                    "\tsubsequence \"%s\" %d "
                    "%d\n",
                    j.fData->fName.CStr(), j.fData->fStart, j.fData->fEnd);
        }

        ufListIter(SequenceEntry) k;
        int lastE = -2;
        i.fData->fSortedEntries.SortInIncreasingOrder(CompareSequenceEntry);
        for (k = i.fData->fSortedEntries; k.fData != 0; k++) {
            if (k.fData->fNum > lastE + 1) {
                fprintf(out, "\n\n");
            }
            lastE = k.fData->fNum;
            fprintf(out, "\t\"%s\" %d\n",
                    k.fData->fName.CStr(), k.fData->fNum);
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
                   e, fName.CStr()));
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
                   e, fName.CStr()));
        return;
    }

    i = SubsequenceRange::New(e, start, end);

    fRanges.Insert(i->fName, i);
    fSortedRanges.InsertFirst(i);

    return;
}

BOOL Sequence::Generate(const char *e, ufString& resultToOwn) {
    return GenEntry(e, fStart, fEnd, resultToOwn);
}

BOOL Sequence::GenerateFromSubsequence(const char *key, const char *e,
                                       ufString& resultToOwn) {
    ufPtr(SubsequenceRange) i;
    ufStringReadonlyCursor subseq(key);
    if (subseq.IsEqual("default")) {
        return GenEntry(e, fStart, fEnd, resultToOwn);
    }
    if (!fRanges.Find(subseq, i)) {
        MOCERROR2((errmsg,
                   "Can't find subsequence \"%s\""
                   " in sequence \"%s\"",
                   subseq.Buf(), fName.CStr()));
        return FALSE;
    }
    return GenEntry(e, i->fStart, i->fEnd, resultToOwn);
}

BOOL Sequence::GenEntry(const char *e, int start, int end,
                        ufString& resultToOwn) {
    resultToOwn.MakeEmpty();

    ufPtr(SequenceEntry) i;
    if (fEntries.Find(e, i)) {
        ufTmpBuf tmp(20);
        sprintf(tmp.fBuf, "%d", i->fNum);
        resultToOwn = tmp.fBuf;
        return TRUE;
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
            return Generate(e, resultToOwn);
        }
    }
    {
        MOCERROR2((errmsg, "Out of sequence numbers [%d:%d] for \"%s\"", start,
                   end, e));
    }

    return FALSE;
}

implement(ufPtr, SequenceEntry);
implement(ufList, SequenceEntry);
implement(ufStrHash, SequenceEntry);

ufPtr(SequenceEntry) SequenceEntry::New(const char *n, int num) {
    return (new SequenceEntry(n, num))->fSelf;
}

SequenceEntry::SequenceEntry(const char *n, int num)
    : fName(n), fNum(num), fSelf(this, kSelfPtrShared) {}

SequenceEntry::SequenceEntry(const SequenceEntry &o)
    : fName(o.fName), fNum(o.fNum),
      fSelf(this, kSelfPtrShared) {
    ufASSERT(0)
}

SequenceEntry::~SequenceEntry() {
}

implement(ufPtr, SubsequenceRange);
implement(ufList, SubsequenceRange);
implement(ufStrHash, SubsequenceRange);

ufPtr(SubsequenceRange) SubsequenceRange::New(const char *n, int start,
                                              int end) {
    return (new SubsequenceRange(n, start, end))->fSelf;
}

SubsequenceRange::SubsequenceRange(const char *n, int start, int end)
    : fName(n), fStart(start), fEnd(end),
      fSelf(this, kSelfPtrShared) {
}

SubsequenceRange::SubsequenceRange(const SubsequenceRange &o)
    : fName(o.fName), fStart(o.fStart), fEnd(o.fEnd),
      fSelf(this, kSelfPtrShared) {
    ufASSERT(0)
}

SubsequenceRange::~SubsequenceRange() {
}
