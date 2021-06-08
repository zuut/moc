/*
 * See Readme, Notice and License files.
 */
#include "Input.h"
#include "MOC.h"
#include "MOCInternal.h"
#include "Version.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int gLastToken = 0;
int gParser = 0;
bool gIsFirst = 1;

int lineno = 1;
int gLastCharRead = ' ';

int gExitOnError = 0;
char gDirectorySeparator = '/';
const char *gProgram;
const char *gFilename = "stdin";
int gNumErrors = 0;
const char *gOutputDirectory = ".";
const char *gTemplate = "moc.tpl";
const char *gSequenceLogFile = "moc.log";
const char *gModule = "-";
const char *gModuleFullFilename = "";
const char *gX = "";

extern "C" {
extern int mocGetToken();
#ifndef MAKE_PROC_SOL4
extern FILE *popen(__const char *, __const char *);
#else
extern FILE *popen(const char *, const char *);
#endif
};

extern char *FGets(ufTmpBuf &buf, FILE *in);

static void mocGetcReset();

static int Execute(int argc, char **argv);

static int GetOpts(int argc, char **argv, char *cmd);

static int ReadInput(int argc, char **argv, char *cmd);

static const char *gHelp =
    " %s [<options>] <filename>\n"
    "       -v version\n"
    "       -T <template>: specifies the MO compiler template\n"
    "       -I <includedir>: specifies the include path \n"
    "       -S <sequence log file>: specifies the log file for \n"
    "               the generated sequences \n"
    "       -V <variable>=<value>: specifies a global variable \n"
    "       -d <directory separator>: a single character used to\n"
    "               separate directories\n"
    "\n"
    "version: %s\n";

void Usage() {
    printf(gHelp, gProgram, VERSION_STRING);
}

void Version() {
    printf("%s version %s\n", gProgram, VERSION_STRING);
}

int main(int argc, char **argv) {
    ufTmpBufInit();

    ufPtr(InputBlock) global = GlobalScope::New();

    int exitcode = Execute(argc, argv);

    CleanupOutput();
    CleanupInput();
    ufTmpBufDestroy();

    return exitcode;
}

int Execute(int argc, char **argv) {
    char cmd[1000];
    // strcpy( cmd, "/lib/cpp -D_OC_ ");
    strcpy(cmd, "/usr/bin/cpp -D_OC_ ");
#ifdef MAKE_PROC_LINUX
    strcat(cmd, "-DLINUX_CPP ");
#else
#ifdef MAKE_PROC_SOL4
    strcat(cmd, "-DSUN4_CPP ");
#endif
#endif
    gProgram = argv[0];
    // create predefined types
    LoadBuiltinTypes();

    int res = GetOpts(argc, argv, cmd);

    if (res) {
        return res > 0 ? res : 0;
    }

    LoadTemplate();
    if (gNumErrors) {
        fprintf(stderr,
                "%s detected %d errors during "
                "template loading\n",
                gProgram, gNumErrors);
        return kBadTemplate;
    }

    LoadSequences();
    if (gNumErrors) {
        fprintf(stderr,
                "%s detected %d errors during "
                "sequence log loading\n",
                gProgram, gNumErrors);
        return kBadSequenceLog;
    }

    ReadInput(argc, argv, cmd);

    if (gNumErrors) {
        fprintf(stderr,
                "%s detected %d errors during "
                "compilation\n",
                gProgram, gNumErrors);
        return kBadSchema;
    }

    gX = "";
    GenerateTypeCodes();
    if (gNumErrors) {
        fprintf(stderr,
                "%s detected %d errors during "
                "type code generation\n",
                gProgram, gNumErrors);
        return kBadTypeCodeGeneration;
    }

    GenerateOutput();
    if (gNumErrors) {
        fprintf(stderr,
                "%s detected %d errors during "
                "output generation\n",
                gProgram, gNumErrors);
        return kBadOutputGeneration;
    }

    SaveSequences();
    if (gNumErrors) {
        fprintf(stderr,
                "%s detected %d errors during "
                "saving sequence log\n",
                gProgram, gNumErrors);
        return kBadSequenceLogSaving;
    }
    return gNumErrors ? 1 : 0;
}

static int ReadInput(int argc, char **argv, char *cmd) {
    int found = 0;

    while (--argc > 0) {
        ++argv;

        if (*argv != 0 && **argv != '-' && *(*argv + 1) != '\0') {
            FILE *in;

            char buf[2000];
            sprintf(buf, "%s %s", cmd, *argv);

            ufTmpBuf moduleFullName(100);
            moduleFullName.Copy(*argv);
            mocGetcReset();

            int len = moduleFullName.GetLength();
            int n;
            for (n = len - 1; n > -1; n--) {
                if (moduleFullName[n] == '/'
                    || moduleFullName[n] == '\\'
                    || moduleFullName[n] == ':') {
                    break;
                }
            }

            ufTmpBuf moduleBaseName(100);
            moduleBaseName.Copy(&moduleFullName[n + 1]);
            ufTmpBufCursor mc(moduleBaseName);
            mc.SkipTo('.');
            *mc = '\0';

            if ((in = popen(buf, "r")) == 0) {
                fprintf(stderr,
                        "%s: can't open "
                        "file \"%s\"\n",
                        gProgram, *argv);
                Usage();
                return kBadArgs;
            }

            found = 1;
            DeclareModule(moduleBaseName.fBuf, moduleFullName.fBuf);
            mocParseInput(in, *argv);
        }
    }

    if (!found) {
        mocParseInput(stdin, 0);
    }

    return found;
}

static int GetOpts(int argc, char **argv, char *cmd) {
    char *ap;
    char *optVal = 0;

    while (--argc > 0 && *++argv != 0) {
        if (**argv == '-' && *(*argv + 1) != '\0') {
            char **oldargv = argv;
            int done = 0;
            for (ap = ++*argv; done == 0 && *ap != '\0'; ap++) {
                switch (*ap) {
                case 'h':
                    Usage();
                    return kExitImmediatelyWithOk;
                case 'v':
                    Version();
                    return kExitImmediatelyWithOk;
                case 'I':
                    optVal = 0;
                    if (ap[1] != 0) {
                        optVal = ap + 1;
                    } else if (--argc < 1 || *++argv == 0) {
                        Usage();
                        fprintf(stderr, "Missing include directory");
                        return kBadArgs;
                    } else {
                        optVal = *argv;
                    }
#ifdef MAKE_PROC_SOL4
                    strcat(cmd, "-I");
#else
                    // some older compilers didn't accept
                    // -I others required a space.
                    // Need to add those exceptions here
                    strcat(cmd, "-I");
#endif
                    strcat(cmd, optVal);
                    strcat(cmd, " ");
                    *argv = 0;
                    done = 1;
                    break;
                case 'o':
                    if (--argc < 1 || *++argv == 0) {
                        Usage();
                        fprintf(stderr, "Missing output directory");
                        return kBadArgs;
                    }
                    gOutputDirectory = *argv;
                    *argv = 0;
                    break;
                case 'T':
                    if (--argc < 1 || *++argv == 0) {
                        Usage();
                        fprintf(stderr, "Missing template");
                        return kBadArgs;
                    }
                    gTemplate = *argv;
                    *argv = 0;
                    break;
                case 'S':
                    if (--argc < 1 || *++argv == 0) {
                        Usage();
                        fprintf(stderr, "Missing sequence log file");
                        return kBadArgs;
                    }
                    gSequenceLogFile = *argv;
                    *argv = 0;
                    break;
                case 'V':
                    if (--argc < 1 || *++argv == 0) {
                        Usage();
                        fprintf(stderr, "Missing variable to define");
                        return kBadArgs;
                    }
                    {
                        const char *var = *argv;
                        *argv = 0;
                        const char *s;
                        for (s = var; *s != '\0' && *s != '='; s++)
                            ;
                        const char *val = (*s != '\0' ? s + 1 : s);
                        *((char *)s) = '\0';
                        DeclareVariable(var, val);
                    }
                    break;
                case '-':
                    // --long-option
                    optVal = 0;
                    if (ap[1] == 0) {
                        // treat this as the end of option processing
                        return 0;
                    } else if (strcmp(ap+1, "version") == 0) {
                        Version();
                        return kExitImmediatelyWithOk;
                    } else if (strcmp(ap+1, "help") == 0) {
                        Usage();
                        return kExitImmediatelyWithOk;
                    }
                    fprintf(stderr, "Unknown long-option --%s\n", ap+1);
                    Usage();
                    return kBadArgs;

                default:
                    // bad option
                    fprintf(stderr, "Unknown short option -%c\n", *ap);
                    Usage();
                    return kBadArgs;
                }
            }

            *oldargv = 0;
        }
    }

    return 0;
}

#include "MOC.h"
#include "MOCTokens.h"

void yyerror(const char *msg) { fprintf(stderr, "%d:%s\n", lineno, msg); }
static void mocGetcReset() {
    gIsFirst = 1;
    lineno = 1;
    gLastToken = 0;
    gLastCharRead = ' ';
}
int mocGetc(FILE *in) {
    char c;

    int done = 0;

    do {
        c = getc(in);
        if (c == '\r') {
            // What to do with MS-DOS? Skipping these for now
            continue;
        }

        gLastCharRead = c;
        if (c == '#' && gIsFirst) {
            ufTmpBuf tmp(2000);
            if (FGets(tmp, in)) {
                static char fnm[1024];
                int lno;
                if (sscanf(tmp.fBuf, "%d \"%[^\"]", &lno, fnm) == 2) {
                    gFilename = fnm;
                    lineno = lno;
                }
            }
            continue;
        }

        done = 1;

        gIsFirst = 0;

        if (c == '\n') {
            gIsFirst = 1;
            lineno++;
        }
    } while (!done);

    return c;
}

const char *mocPrintToken(int x) {
    switch (gParser) {
    case kINPUT_PARSER:
        return mocPrintInputToken(x);
    case kIF_PARSER:
        return mocPrintIfToken(x);
    default:
        break;
    }

    return "";
}

const char *mocPrintTokenValue(int x) {
    switch (gParser) {
    case kINPUT_PARSER:
        return mocPrintInputTokenValue(x);
    case kIF_PARSER:
        return mocPrintIfTokenValue(x);
    default:
        break;
    }

    return "";
}

void mocError3(const char *msg) {
    if (strcmp(gFilename, "???") != 0) {
        fprintf(stderr, "%s:%d: %s : %s\n", gFilename, lineno, gProgram, msg);
    } else {
        fprintf(stderr, "???:NN: %s : %s\n", gProgram, msg);
    }
    fflush(stderr);
}

void mocError(const char *msg) {
    gNumErrors++;
    if (strcmp(gFilename, "???") != 0) {
        fprintf(stderr, "%s:%d: Error in %s input: %s\n", gFilename, lineno,
                gProgram, msg);
    } else {
        fprintf(stderr, "Error in %s input: %s\n", gProgram, msg);
    }
    if (*gX != '\0') {
        fprintf(stderr, "Was expecting %s \n", gX);
    }
    if (gParser == kINPUT_PARSER || gParser == kIF_PARSER) {
        fprintf(stderr,
                "    but instead found%s(%d) "
                " %s\n",
                mocPrintToken(gLastToken), gLastToken,
                mocPrintTokenValue(gLastToken));
        fprintf(stderr,
                "last token%d='%c', last "
                "character read '%c' \n",
                gLastToken, gLastToken, gLastCharRead);
    }
    if (gExitOnError) {
        exit(10);
    }
    fflush(stderr);
}

void mocError2(const char *msg) {
    gNumErrors++;
    if (strcmp(gFilename, "???") != 0) {
        fprintf(stderr, "%s:%d: Error in %s input: %s\n", gFilename, lineno,
                gProgram, msg);
    } else {
        fprintf(stderr, "Error in %s input: %s\n", gProgram, msg);
    }
    if (gExitOnError) {
        exit(10);
    }
}

void mocWarning(const char *msg) {
    if (strcmp(gFilename, "???") != 0) {
        fprintf(stderr, "%s:%d: Warning: %s\n", gFilename, lineno, msg);
    } else {
        fprintf(stderr, "Warning: %s\n", msg);
    }
}

void mocNormal(const char *msg) {
    if (strcmp(gFilename, "???") != 0) {
        fprintf(stdout, "%s:%d: %s\n", gFilename, lineno, msg);
    } else {
        fprintf(stdout, "%s\n", msg);
    }
}
