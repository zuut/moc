/*
 * See Readme, Notice and License files.
 */
#ifndef GENERICH
#define GENERICH

#define name2(a, b) a##b
#define name3(a, b, c) a##b##c

#define declare(a, b) a##declare(b)
#define implement(a, b) a##implement(b)

#define declareclass(a, b) a##declareclass(b)
#define declareinlines(a, b) a##declareinlines(b)

#endif
