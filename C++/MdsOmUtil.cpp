
#ifndef WIN32
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <iostream>
	#include <fstream>
#endif

#include "MdsOm.h"
#include "MdsOmInternal.h"

namespace MdsOmNs {

#ifndef WIN32
/* Macros for extract VSIZE and RSS from /proc/pid/stats files */
#define bytetok(x)      (((x) + 512) >> 10)

static char* skip_ws(char* p)
{
    while (isspace(*p))
        p++;
    return p;
}

static char* skip_token(char* p)
{
    while (isspace(*p))
        p++;
    while (*p && !isspace(*p))
        p++;
    return p;
}

static long pagetok(long x)
{
/*
 * This replaces:
 * #include <asm/page.h>
 * #define pagetok(x)    ((x) << (PAGE_SHIFT - 10))
 * which is deprecated according to RedHat
 */ 
    static int initDone = 0;
    static int pageShift = 0;

    if (!initDone) {
    	int pgSize = sysconf(_SC_PAGESIZE);

    	/* We have to calc the power-of-2 that the page size represents */
    	for (pageShift = 0; !(pgSize & 0x01); pageShift++, pgSize >>= 1)
        	;
	
    	/* Convert to 1K units */
    	pageShift -= 10;
    	initDone = 1;
    }
    return (x << pageShift);
}

static long getTotalSystemMem()
{
	long pages = sysconf(_SC_PHYS_PAGES);
	long page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
}
#endif

// ----------------------------------------------------------------------
MamaMsg* newMamaMsg(char payload)
{
	try {
		MamaMsg* msg = new MamaMsg();
		msg->createForPayload(payload);
		return msg;
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "getMamaMsg: error payload=%c %s", payload, status.toString());
		throw;
	}
}

void deleteMamaMsg(MamaMsg* msg)
{
	try {
		delete msg;
	} catch (MamaStatus& status) {
		mama_log(MAMA_LOG_LEVEL_ERROR, "destroyMamaMsg: error %s", status.toString());
		throw;
	}
}

// ----------------------------------------------------------------------
mama_status convertMsgStatusToMamaStatus(mamaMsgStatus msgStatus)
{
	switch (msgStatus) {
	case MAMA_MSG_STATUS_NOT_PERMISSIONED:
		return MAMA_STATUS_NOT_PERMISSIONED;
		break;
	case MAMA_MSG_STATUS_NOT_ENTITLED:
		return MAMA_STATUS_NOT_ENTITLED;
		break;
	case MAMA_MSG_STATUS_NOT_FOUND:
		return MAMA_STATUS_NOT_FOUND;
		break;
	case MAMA_MSG_STATUS_BAD_SYMBOL:
		return MAMA_STATUS_BAD_SYMBOL;
		break;
	case MAMA_MSG_STATUS_OK:
		return MAMA_STATUS_OK;
		break;
	case MAMA_MSG_STATUS_TIMEOUT:
		return MAMA_STATUS_TIMEOUT;
	default:
		return MAMA_STATUS_SYSTEM_ERROR;
		break;
	}
}

// ----------------------------------------------------------------------
mamaMsgStatus convertMamaStatusToMsgStatus(mama_status status)
{
	switch (status) {
	case MAMA_STATUS_NOT_PERMISSIONED:
		return MAMA_MSG_STATUS_NOT_PERMISSIONED;
	case MAMA_STATUS_NOT_ENTITLED:
		return MAMA_MSG_STATUS_NOT_ENTITLED;
		break;
	case MAMA_STATUS_NOT_FOUND:
		return MAMA_MSG_STATUS_NOT_FOUND;
		break;
	case MAMA_STATUS_BAD_SYMBOL:
		return MAMA_MSG_STATUS_BAD_SYMBOL;
		break;
	case MAMA_STATUS_OK:
		return MAMA_MSG_STATUS_OK;
		break;
	case MAMA_STATUS_TIMEOUT:
		return MAMA_MSG_STATUS_TIMEOUT;
		break;
	default:
		return MAMA_MSG_STATUS_UNKNOWN;
		break;
	}
}

// ----------------------------------------------------------------------
char *trim(char *str)
{
	// Trim leading space
	while (isspace(*str)) str++;

	if (*str == '\0')  // All spaces?
		return str;

	// Trim trailing space
	char *end = str + strlen(str) - 1;
	while (end > str && isspace(*end)) end--;

	// Write new null terminator
	*(end+1) = '\0';

	return str;
}

// ----------------------------------------------------------------------
pair<string, string> getSourceAndSymbol(const char* topic)
{
	pair<string, string> ret;

	if (topic == NULL || *topic == '\0') return ret;

	char* p = strdup(topic);
	char* cp = strchr(p, '.');		// get first '.'
	if (cp != NULL) {
		*cp = '\0';
		ret.first = p;
		ret.second = ++cp;
	}
	free(p);
	return ret;
}

// ----------------------------------------------------------------------
const std::string MdsWhiteSpace = " \n\r\t";

std::string MdsTrimLeft(const std::string& s)
{
    size_t startpos = s.find_first_not_of(MdsWhiteSpace);
    return (startpos == std::string::npos) ? "" : s.substr(startpos);
}

std::string MdsTrimRight(const std::string& s)
{
    size_t endpos = s.find_last_not_of(MdsWhiteSpace);
    return (endpos == std::string::npos) ? "" : s.substr(0, endpos+1);
}

MDSOMExpDLL std::string MdsTrim(const std::string& s)
{
    return MdsTrimRight(MdsTrimLeft(s));
}

// ----------------------------------------------------------------------
mama_u64_t getNow()
{
#ifndef WIN32
	// CLOCK_MONOTONIC CLOCK_MONOTONIC_HR
	struct timespec tp;
	long ret = clock_gettime(CLOCK_MONOTONIC , &tp);
	if (ret != 0) mama_log(MAMA_LOG_LEVEL_ERROR, "getNow: cannot get CLOCK_MONOTONIC clock"); 
	mama_u64_t t = (((mama_u64_t) tp.tv_sec) * 1000000000) + tp.tv_nsec;
#else
    SYSTEMTIME tx;
    GetSystemTime(&tx);
	mama_u64_t t = (((mama_u64_t) tx.wSecond) * 1000000000) + tx.wMilliseconds * 1000000;
#endif
	return t;
}

// ----------------------------------------------------------------------
char* getElapsed(char* buf) {
	static time_t start = 0;
	if (start == 0) time(&start);

	time_t now;
	time(&now);

	time_t diff = now - start;

	int days = (int) (diff / 60 / 60 / 24);
	int hours = (int) ((diff / 60 / 60) % 24);
	int mins = (int) ((diff / 60) % 60);
	int secs = (int) ( diff % 60);

	sprintf(buf, "%02d:%02d:%02d:%02d", days, hours, mins, secs);

	return buf;
}

void mdsOmGetMemVals(int pid, MdsOmMemVals* mv)
{
	if (!mv) return;

	mv->vsize = 0;
	mv->rss = 0;
	mv->memPercent = 0;

#ifdef WIN32
	return;
#if 0
    int error = 0;
    char buffer[64];

    TCHAR      memCounterPath[64];
    HCOUNTER   memCounter;
    PDH_STATUS  pdhStatus;          /* return status of PDH functions */
    LPCTSTR     szDataSource = NULL;/* NULL for dynamic registry entries */
    HQUERY      phQuery;            /* query Handle */
    PDH_FMT_COUNTERVALUE pValue={0};

    snprintf(memCounterPath, sizeof(memCounterPath), "\\Process(%s)\\Private Bytes", buffer);

    /* create query */
    pdhStatus = PdhOpenQuery(NULL, 0, &phQuery);
    if (pdhStatus != ERROR_SUCCESS)  {
        /* Print the error value. */
        mama_log(MAMA_LOG_LEVEL_ERROR, "PdhOpenQuery failed with %ld\n", pdhStatus);
        error = 1;
    }

    /* add counter for MEM */
    pdhStatus = PdhAddCounter(phQuery,memCounterPath, 0, &memCounter);
    if (pdhStatus != ERROR_SUCCESS) {
        /* Print the error value. */
        mama_log(MAMA_LOG_LEVEL_ERROR, "PdhAddCounter Failed with 0x%x\n", pdhStatus);
        error = 1;
    }

    /* collect Data */
    pdhStatus = PdhCollectQueryData(phQuery);
    if (pdhStatus != ERROR_SUCCESS) {
        /* Print the error value. */
        mama_log(MAMA_LOG_LEVEL_ERROR, "PdhCollectQueryData Failed with 0x%x\n", pdhStatus);
        error = 1;
    }

    pdhStatus = PdhGetFormattedCounterValue(memCounter,
            PDH_FMT_DOUBLE,
            NULL,
            &pValue);
    if (pdhStatus == ERROR_SUCCESS) {
        mv->rss = (long)(pValue.doubleValue/1.0e3);
    } else {
        /* Print the error value. */
        mama_log(MAMA_LOG_LEVEL_ERROR, "PdhGetFormattedCounterValue Failed with 0x%x\n", pdhStatus);
        error = 1;
    }

    PdhCloseQuery(phQuery);
#endif
#else
    unsigned long vsize;
	long rss;
    std::string ignore;

    std::ifstream ifs("/proc/self/stat", std::ios_base::in);
    ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> vsize >> rss;
	ifs.close();

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    mv->vsize = (double) vsize / 1024.0;
    mv->rss = rss * page_size_kb;
#endif
}

}
