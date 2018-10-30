#include "common_include.h"
#include "rbtime.h"
#include "rblog.h"

// ---------------------------------------------------------
// |   FATAL  |   ERROR  |   WARN   |   INFO   |   DEBUG   |
// ------------------------------------------------------------------
// | *.stderr | *.stderr | *.stdwrn | *.stdout | *.stderr  |  FILE  |
// ------------------------------------------------------------------
// |    O     |    X     |    O     |    X     |     O     | SRCLOC |
// ------------------------------------------------------------------
// |    X     |    X     |    X     |    X     | O(stderr) |  PRINT |
// ------------------------------------------------------------------

#define SZ_LOG_FILE_PATH 512
#define SZ_LOG_FILE_NAME 128
#define SZ_LOG_FILE_CHUNK (1024 * 1024 * 512); // 512MB Log Size
#define SZ_LOG  2048
#define SZ_FMT  1024
#define SZ_SRC  1024
#define SZ_TIME 32

static const BYTE gAB_FATAL[] = "FAT"; // Logging to 'error log' file with src location, Exit program.
static const BYTE gAB_ERROR[] = "ERR"; // Logging to 'error log' file.
static const BYTE gAB_WARN [] = "WAN"; // Logging to 'warn log' file with src location.
static const BYTE gAB_INFO [] = "INF"; // Logging to 'common log' file.
static const BYTE gAB_DEBUG[] = "DBG"; // Logging to stderr with src location.

static const BYTE gAB_STDOUT_EXT[] = "_stdout";
static const BYTE gAB_STDWRN_EXT[] = "_stdwrn";
static const BYTE gAB_STDERR_EXT[] = "_stderr";

static BYTE gAB_LOG_PATH[SZ_LOG_FILE_PATH]; // "Path" of log file. (ex: "C:/Program Files/Program/log")
static BYTE gAB_LOG_NAME[SZ_LOG_FILE_NAME]; // 'Name' of log file. (ex: 'Hello'.stderr.YYYYMMDD_0000)
static BYTE gAB_LOG_FULL_NAME[3][SZ_LOG_FILE_PATH + SZ_LOG_FILE_NAME]; // [stdout,stdwrn,stderr][Path + Name]

static FILE *gPST_LOG_ERROR; // *.stderr (LOG_ERROR)
static FILE *gPST_LOG_WARN;  // *.stdwrn (LOG_WARN)
static FILE *gPST_LOG_INFO;  // *.stdout (LOG_INFO)

static void CloseLogAll();
static int CheckLogFile(FILE **pstLogFile, BYTE abTime[SZ_TIME]);

void InitLog(const BYTE *pbLogPath, int szLogPath, const BYTE *pbLogName, int szLogName)
{
    static BYTE AB_DefaultLogName[] = "default_log";
    static BYTE AB_DefaultLogPath[] = "./";
    BYTE        abLogFullName[SZ_LOG_FILE_PATH + SZ_LOG_FILE_NAME];
    int         i;

    // Init Globals
    CloseLogAll();

    // Set Log Path
    memset(gAB_LOG_PATH, 0x00, sizeof(gAB_LOG_PATH));

    if (pbLogPath == NULL || szLogPath < 1)
    {
        strncpy(gAB_LOG_PATH, AB_DefaultLogPath, strlen(AB_DefaultLogPath));
    }
    else
    {
        strncpy(gAB_LOG_PATH, pbLogPath, min(sizeof(gAB_LOG_PATH), szLogPath));
    }

    // Set Log Name
    memset(gAB_LOG_NAME, 0x00, sizeof(gAB_LOG_NAME));
    
    if (pbLogName == NULL || szLogName < 1)
    {
        strncpy(gAB_LOG_NAME, AB_DefaultLogName, strlen(AB_DefaultLogName));
    }
    else
    {
        strncpy(gAB_LOG_NAME, pbLogName, min(sizeof(gAB_LOG_NAME), szLogName));
    }

    // Concat log file full name
    memset(abLogFullName, 0x00, sizeof(abLogFullName));
    strcat(abLogFullName, gAB_LOG_PATH);
    strcat(abLogFullName, gAB_LOG_NAME);
    
    for (i = 0; i < 3; ++i)
    {
        memset(gAB_LOG_FULL_NAME[i], 0x00, sizeof(gAB_LOG_FULL_NAME[i]));
        strncpy(gAB_LOG_FULL_NAME[i], abLogFullName, strlen(abLogFullName)); // Path + Name
        
        if      (i == 0) { strcat(gAB_LOG_FULL_NAME[i], gAB_STDOUT_EXT); } // [0] : .stdout
        else if (i == 1) { strcat(gAB_LOG_FULL_NAME[i], gAB_STDWRN_EXT); } // [1] : .stdwrn
        else if (i == 2) { strcat(gAB_LOG_FULL_NAME[i], gAB_STDERR_EXT); } // [2] : .stderr
    }
}

void Log(const RBLOG_LEVEL eLv, const BYTE *pbSrc, const int nLine, const BYTE *pbFunc, const BYTE *pbFmt, ...)
{
    va_list     vargs;
    BYTE        abLog[SZ_LOG];      // abLog = LOG_LEVEL + TIME + PID + abFmt + (abSrc)
    BYTE        abFmt[SZ_FMT];      // abFmt = pbFmt + ...
    BYTE        abSrc[SZ_SRC];      // abSrc = pbSrc + nLine + pbFunc
    BYTE        abTime[SZ_TIME];    // YYYY-MM-DD HH:mm:SS
    const BYTE  *pbLogLvStr = NULL;
    int         nPid, isSrcOn = 0;

    // abTime : Time value
    GetTimeToString(TIME_YYYYMMDDHHmmSS, abTime, sizeof(abTime));

    // abFmt : Formats
    va_start(vargs, pbFmt);
    vsnprintf(abFmt, SZ_FMT, pbFmt, vargs);
    va_end(vargs);

    // abSrc : Source location
    if (eLv == LOG_FATAL || eLv == LOG_WARN || eLv == LOG_DEBUG)
    {
        snprintf(abSrc, SZ_SRC, " (Src:%s, Line:%d, Func:%s())", pbSrc, nLine, pbFunc);
        isSrcOn = 1;
    }

    // pLogLvStr : Log Level String
    if      (eLv == LOG_FATAL) { pbLogLvStr = gAB_FATAL; }
    else if (eLv == LOG_ERROR) { pbLogLvStr = gAB_ERROR; }
    else if (eLv == LOG_WARN ) { pbLogLvStr = gAB_WARN;  }
    else if (eLv == LOG_INFO ) { pbLogLvStr = gAB_INFO;  }
    else                       { pbLogLvStr = gAB_DEBUG; }

    // abLog : Make log
    #ifdef _WINDOWS_OS_
        nPid = GetCurrentProcessId();
    #else
        nPid = getpid();
    #endif
    
    snprintf(abLog, SZ_LOG, "[%s][%s][PID:%d] %s", pbLogLvStr, abTime, nPid, abFmt);

    if (isSrcOn == 1) { strcat(abLog, abSrc); }

    // Check Logfile
    if (eLv == LOG_INFO)
    {
        // *.stdout.YYYYMMDD_04N
        CheckLogFile(&gPST_LOG_INFO, abTime);
    }
    else if (eLv == LOG_WARN)
    {
        // *.stdwrn.YYYYMMDD_04N
        CheckLogFile(&gPST_LOG_WARN, abTime);
    }
    else // (eLv == LOG_FATAL || eLv == LOG_ERROR || eLv == LOG_DEBUG)
    {
        // *.stderr.YYYYMMDD_04N
        CheckLogFile(&gPST_LOG_ERROR, abTime);
    }

    // Write Log
    if      (eLv == LOG_FATAL) { fprintf(gPST_LOG_ERROR, "%s\n", abLog); fflush(gPST_LOG_ERROR); exit(-1); }
    else if (eLv == LOG_ERROR) { fprintf(gPST_LOG_ERROR, "%s\n", abLog); fflush(gPST_LOG_ERROR);           }
    else if (eLv == LOG_WARN ) { fprintf(gPST_LOG_WARN,  "%s\n", abLog); fflush(gPST_LOG_WARN);            }
    else if (eLv == LOG_INFO ) { fprintf(gPST_LOG_INFO,  "%s\n", abLog); fflush(gPST_LOG_INFO);            }
    else                       { fprintf(stderr,         "%s\n", abLog); fflush(stderr);                   }
}

static void CloseLogAll()
{
    if (gPST_LOG_ERROR != NULL) { fclose(gPST_LOG_ERROR); gPST_LOG_ERROR = NULL; }
    if (gPST_LOG_WARN  != NULL) { fclose(gPST_LOG_WARN ); gPST_LOG_WARN  = NULL; }
    if (gPST_LOG_INFO  != NULL) { fclose(gPST_LOG_INFO ); gPST_LOG_INFO  = NULL; }
}

static int CheckLogFile(FILE **pstLogFile, BYTE abTime[SZ_TIME])
{
    static BYTE AB_LastDate[2], AB_LogFullName[SZ_LOG_FILE_PATH + SZ_LOG_FILE_NAME];
    BYTE        abYearMonDay[10];
    int         isDateChanged = 0, rtVal;

    isDateChanged = memcmp(AB_LastDate, &abTime[8], 2) != 0 : 1 : 0;

    // File isn't exist or Datetime is changed
    if (*pstLogFile == NULL || isDateChanged == 1) // YYYY-MM-(DD) HH:mm:SS\0
    {
        if (isDateChanged == 1) { CloseLogAll(); }

        memcpy(AB_LastDate, &abTime[8], 2);
        memset(AB_LogFullName, 0x00, sizeof(AB_LogFullName));
        memset(abYearMonDay,   0x00, sizeof(abYearMonDay)  );
        abYearMonDay[0] = '.';                   // .
        memcpy(&abYearMonDay[1], &abTime[0], 4); // YYYY
        memcpy(&abYearMonDay[5], &abTime[5], 2); // MM
        memcpy(&abYearMonDay[7], &abTime[8], 2); // DD

        // Get logfile name
        if      (pstLogFile == &gPST_LOG_ERROR) { strcat(AB_LogFullName, gAB_LOG_FULL_NAME[2]); }
        else if (pstLogFile == &gPST_LOG_WARN ) { strcat(AB_LogFullName, gAB_LOG_FULL_NAME[1]); }
        else if (pstLogFile == &gPST_LOG_INFO ) { strcat(AB_LogFullName, gAB_LOG_FULL_NAME[0]); }

        strcat(AB_LogFullName, abYearMonDay);

        // Open logfile
        if ((*pstLogFile = fopen(AB_LogFullName, "at")) == NULL) // (AppendOnly + Text) mode
        {
            // !The only hard-coded 'Fatal Log' in rblog function!
            fprintf(stderr, "[FAT][%s] Critical Error! Fail to open/create logfile '%s'. (Src:%s, Line:%d, Func:%s())\n", 
                    abTime, AB_LogFullName, __FILE__, __LINE__, __FUNCTION__);
            exit(-1);
        }
    }

    return 0;
}