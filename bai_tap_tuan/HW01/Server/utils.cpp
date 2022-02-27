#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "stdio.h"
#include "ws2tcpip.h"
#include "time.h"
/*
* @function getSubStr : get sub string
* @param buff : a pointer to a big string
* @param from : start index of substring
* @param length : length substring
*
* @return :  a sub string from index 'from' to 'from+length'
*/

char* getSubStr(char * buff, int from, int length) {
	char* subbuff = (char *)malloc((length + 1) * sizeof(char));
	memcpy(subbuff, &buff[from], length);
	subbuff[length] = '\0';
	return subbuff;
}




/*
* @function getStrTimeNow : get string time
*
* @return :  a string contain time format 'dd-MM-yyyy HH:mm:ss'
*/
char * getStrTimeNow() {
	char * result = (char *)malloc(sizeof(char) * 20);
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(result, "%02d-%02d-%d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return result;
}