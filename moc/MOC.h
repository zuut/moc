/*
 * See Readme, Notice and License files.
 */

#ifndef _MC_H
#define _MC_H

/*
 * Program Errors
 */
const int kBadArgs = 10;
const int kBadTemplate = 11;
const int kBadSequenceLog = 12;
const int kBadSchema = 13;
const int kBadTypeCodeGeneration = 14;
const int kBadOutputGeneration = 15;
const int kBadSequenceLogSaving = 16;

// Negative error codes will terminate processing immediate
// and exit with exit code 0
const int kExitImmediatelyWithOk=-1;

#endif
