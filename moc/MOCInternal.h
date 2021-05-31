/*
 * See Readme, Notice and License files.
 *
 */
#ifndef __MCINTERNAL__
#define __MCINTERNAL__

#define register 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UserCode {
    int fCodeLen;
    char *fCode;
    char *fFilename;
    int fLineno;
} UserCode;

typedef struct KeyValue {
    UserCode *fUserCode;
    char fKey[1024];
} KeyValue;

#define kINPUT_PARSER 1
#define kOUTPUT_PARSER 2
#define kIF_PARSER 3
#define kCONTROL_PARSER 4

#ifndef BOOL_DEFINED
typedef int BOOL;
#endif

extern int gExitOnError;
extern int gParser;
extern int gLastToken;
extern const char *gX;
extern int gLastCharRead;

extern int gNumErrors;
extern const char *gProgram;
extern int lineno;

extern const char *gModule;
extern const char *gModuleFullFilename;
extern const char *gFilename;
extern const char *gOutputDirectory;
extern const char *gTemplate;
extern const char *gSequenceLogFile;
extern char gDirectorySeparator;

extern const char *mocPrintToken(int token);
extern void mocNormal(const char *msg);
extern void mocWarning(const char *msg);
extern void mocError(const char *msg);
extern void mocError2(const char *msg);
extern void mocError3(const char *msg);
extern int mocGetc(FILE *);
#define MOCERROR(x)                                                            \
    do {                                                                       \
        char errmsg[9000];                                                     \
        sprintf x;                                                             \
        mocError(errmsg);                                                      \
    } while (0)
#define MOCERROR2(x)                                                           \
    do {                                                                       \
        char errmsg[9000];                                                     \
        sprintf x;                                                             \
        mocError2(errmsg);                                                     \
    } while (0)
#define MOCERROR3(x)                                                           \
    do {                                                                       \
        char errmsg[9000];                                                     \
        sprintf x;                                                             \
        mocError3(errmsg);                                                     \
    } while (0)
/*
 * General routines
 */

extern void LoadBuiltinTypes(void);
extern void LoadTemplate(void);
extern void LoadSequences(void);
extern void SaveSequences(void);
extern void GenerateTypeCodes(void);
extern void GenerateOutput(void);

/*
 * Input Parser Routines and stuff
 */
extern FILE *gInputFile;
extern void WarnIfNotASubclassOf(const char *n, const char *p);
extern void mocParseInput(FILE *in, const char *fileName);
extern void mocParseReset();
const char *mocPrintInputToken(int token);
const char *mocPrintInputTokenValue(int token);
const char *WriteCodeFrag(UserCode *uc, char *buf, int len);

extern UserCode *CreateUserCode(char *s);
extern KeyValue *ReadKeyValue(KeyValue *resultBuf,
                              const char *inputBuf, FILE *in);
extern UserCode *ReadCodeString(FILE *in);
extern UserCode *newUserCode(const char *);
extern UserCode *newUserCode2(const char *, const char *);
extern BOOL mocIsTypeDefined(const char *);
extern BOOL mocIsClassDefined(const char *);
extern BOOL mocIsConstant(const char *);
extern int mocGetConstant(const char *);
extern void SetHeaderCode(UserCode *uc);
extern void SetHeaderEndCode(UserCode *uc);
extern void SetSourceCode(UserCode *uc);

extern void DeclareModule(const char* module, const char* fullPath);
extern void DeclareType(const char *name);
extern void DeclareConstant(const char *name, int value);
extern void DeclareVariable(const char *name, const char *val);
extern void SetInterface(void);
extern void SetTypeClassName(const char *name);
extern void AddTypeEnumEntry(const char *nm);
extern void AddTypeEnumEntryPair(const char *name, int e);
extern void AddStructField(const char *typeName, const char *symbolName);
extern void SetTypeHeader(const char *headerName);
extern void SetTypeDef(const char *headerName);
extern void SetTypeKeyValue(const char *key, UserCode *value);
extern void DeclareClass(const char *name);
extern void SetClassKeyValue(const char *key, UserCode *value);
extern void AddClassParent(const char *name);
extern void AddField(const char *typeName, const char *symbolName);
extern void AddFieldArrayIndex(int arrayLength);
extern void DeclareOperation(const char *name);
extern void DeclareOperationImp(const char *className, const char *opName);
extern void SetOpArgKeyValue(const char *key, UserCode *value);
extern void SetOpKeyValue(const char *key, UserCode *value);
extern void SetOpCodeString(UserCode *value);
extern void SetFieldKeyValue(const char *key, UserCode *value);
extern void AddInArgument(const char *typeName, const char *symbolName);
extern void AddOutArgument(const char *typeName, const char *symbolName);
extern void AddErrorArgument(const char *typeName, const char *symbolName);

/*
 * If Statement Parser Routines and stuff
 */
extern const char *gIfString;

extern void mocIfReset(void);
extern int mocParseIf(const char *string, const char *file, int line);
extern const char *mocPrintIfToken(int token);
extern const char *mocPrintIfTokenValue(int token);

extern BOOL IfEvalExists(char *, char *);
extern BOOL IfEvalEmpty(char *, char *);
extern BOOL IfEvalIsFromCurrentModule(char *, char *);
extern BOOL IfEvalIsA(char *, char *);
extern BOOL IfEvalFirst(char *, char *);
extern BOOL IfEvalLast(char *, char *);
extern BOOL IfEvalStrequal(char *, char *, char *);

/*
 * Control Statement Parser Routines and stuff
 */
extern const char *gControlString;
extern void mocControlReset(void);
extern int mocParseControl(const char *string, const char *file, int line);
extern const char *mocPrintControlToken(int token);
extern const char *mocPrintControlTokenValue(int token);
extern int ControlEvalSetInteger(char *, int);
extern int ControlEvalSetString(char *, char *);
/*
 * ufStrDup
 */
extern char *ufStrDup(const char *);

extern int mocGetInputToken(void);
extern int mocInputlex(void);
extern int mocGetIfToken(void);
extern int mocGetControlToken(void);
extern int mocIflex(void);
extern int mocIfwrap(void);
extern int mocControllex(void);
extern int mocControlwrap(void);

#ifdef __cplusplus
};
#endif

#endif
