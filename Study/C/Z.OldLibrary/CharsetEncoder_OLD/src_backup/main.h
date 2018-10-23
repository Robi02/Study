#ifndef __MAIN_H__
#define __MAIN_H__

static char *g_pPwd, *g_pCharset, *g_pOptions, *g_pString;

int main(int argc, char **argv);

static int CheckArgc(int argc, char **argv);
static void PrintUsage();
static void PrintCharset();
static void PrintOptions();

#endif