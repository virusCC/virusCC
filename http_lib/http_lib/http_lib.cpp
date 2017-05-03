#include "http_lib.h"

HINSTANCE hInst;
WSADATA wsaData;
void mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename);
SOCKET connectToServer(char *szServerName, WORD portNum);
int getHeaderLength(char *content);
char *readUrl2Get(char *szUrl, long &bytesReturnedOut, char **headerOut);
char *readUrl2Post(char *szUrl, char *post, long &bytesReturnedOut, char **headerOut);
char *readUrl2PostFile(char *szUrl, char *post, char* fname, long &bytesReturnedOut, char **headerOut);

char *http_get(char *szUrl){
	const int bufLen = 1024;
	long fileSize;
	char *memBuffer, *headerBuffer;
	memBuffer = headerBuffer = NULL;
	if (WSAStartup(0x101, &wsaData) != 0)
		return NULL;
	memBuffer = readUrl2Get(szUrl, fileSize, &headerBuffer);
	WSACleanup();
	return memBuffer;
}
char *http_post(char *szUrl, char *post){
	const int bufLen = 1024;
	long fileSize;
	char *memBuffer, *headerBuffer;
	memBuffer = headerBuffer = NULL;
	if (WSAStartup(0x101, &wsaData) != 0)
		return NULL;
	memBuffer = readUrl2Post(szUrl, post, fileSize, &headerBuffer);
	WSACleanup();
	return memBuffer;
}
char *http_post_file(char *szUrl, char *fname, char *post){
	const int bufLen = 1024;
	long fileSize;
	char *memBuffer, *headerBuffer;
	memBuffer = headerBuffer = NULL;
	if (WSAStartup(0x101, &wsaData) != 0)
		return NULL;
	memBuffer =  readUrl2PostFile(szUrl, post, fname, fileSize, &headerBuffer);
	WSACleanup();
	return memBuffer;
}
char *readUrl2Get(char *szUrl, long &bytesReturnedOut, char **headerOut){
	const int bufSize = 512;
	char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize];
	char *tmpResult = NULL, *result;
	SOCKET conn;
	string server, filepath, filename;
	long totalBytesRead, thisReadSize, headerLen;

	mParseUrl(szUrl, server, filepath, filename);

	///////////// step 1, connect //////////////////////
	conn = connectToServer((char*)server.c_str(), 80);

	///////////// step 2, send GET request /////////////
	sprintf(tmpBuffer, "GET %s HTTP/1.0", filepath.c_str());
	strcpy(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	sprintf(tmpBuffer, "Host: %s", server.c_str());
	strcat(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	strcat(sendBuffer, "\r\n");
	send(conn, sendBuffer, strlen(sendBuffer), 0);

	///////////// step 3 - get received bytes ////////////////
	// Receive until the peer closes the connection
	totalBytesRead = 0;
	while (1)
	{
		memset(readBuffer, 0, bufSize);
		thisReadSize = recv(conn, readBuffer, bufSize, 0);

		if (thisReadSize <= 0)
			break;

		tmpResult = (char*)realloc(tmpResult, thisReadSize + totalBytesRead);

		memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
		totalBytesRead += thisReadSize;
	}

	headerLen = getHeaderLength(tmpResult);
	long contenLen = totalBytesRead - headerLen;
	result = new char[contenLen + 1];
	memcpy(result, tmpResult + headerLen, contenLen);
	result[contenLen] = 0x0;
	char *myTmp;

	myTmp = new char[headerLen + 1];
	strncpy(myTmp, tmpResult, headerLen);
	myTmp[headerLen] = NULL;
	delete(tmpResult);
	*headerOut = myTmp;

	bytesReturnedOut = contenLen;
	closesocket(conn);
	return(result);
}
char *readUrl2Post(char *szUrl, char *post,  long &bytesReturnedOut, char **headerOut){
	const int bufSize = 512;
	char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize];
	char *tmpResult = NULL, *result;
	SOCKET conn;
	string server, filepath, filename;
	long totalBytesRead, thisReadSize, headerLen;

	mParseUrl(szUrl, server, filepath, filename);
	///////////// step 1, connect //////////////////////
	conn = connectToServer((char*)server.c_str(), 80);

	///////////// step 2, send POST request /////////////
	sprintf(tmpBuffer, "POST %s HTTP/1.0", filepath.c_str());
	strcpy(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	sprintf(tmpBuffer, "Host: %s", server.c_str());
	strcat(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	sprintf(tmpBuffer, "Content-Type: application/x-www-form-urlencoded");
	strcat(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	sprintf(tmpBuffer, "Content-Length: %d", strlen(post));
	strcat(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	strcat(sendBuffer, "\r\n");
	sprintf(tmpBuffer, "%s", post);
	strcat(sendBuffer, tmpBuffer);
	send(conn, sendBuffer, strlen(sendBuffer), 0);
	///////////// step 3 - get received bytes ////////////////
	// Receive until the peer closes the connection
	totalBytesRead = 0;
	while (1)
	{
		memset(readBuffer, 0, bufSize);
		thisReadSize = recv(conn, readBuffer, bufSize, 0);

		if (thisReadSize <= 0)
			break;

		tmpResult = (char*)realloc(tmpResult, thisReadSize + totalBytesRead);

		memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
		totalBytesRead += thisReadSize;
	}

	headerLen = getHeaderLength(tmpResult);
	long contenLen = totalBytesRead - headerLen;
	result = new char[contenLen + 1];
	memcpy(result, tmpResult + headerLen, contenLen);
	result[contenLen] = 0x0;
	char *myTmp;

	myTmp = new char[headerLen + 1];
	strncpy(myTmp, tmpResult, headerLen);
	myTmp[headerLen] = NULL;
	delete(tmpResult);
	*headerOut = myTmp;

	bytesReturnedOut = contenLen;
	closesocket(conn);
	return(result);
}
char *readUrl2PostFile(char *szUrl, char *post, char* fname, long &bytesReturnedOut, char **headerOut){
	char *fileBuf = new char[1*1024*1024]; // 1MB
	FILE *fp = fopen(fname, "r");
    if(fp == NULL){
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if(fread(fileBuf, 1, file_size, fp) == -1){
        return NULL;
    }
	const int bufSize = 512;
	char readBuffer[bufSize];
	char* sendBuffer = new char[1*1024*1024];
	char* tmpBuffer =  new char[1*1024*1024];
	char *tmpResult = NULL, *result;
	SOCKET conn;
	string server, filepath, filename;
	long totalBytesRead, thisReadSize, headerLen;
	mParseUrl(szUrl, server, filepath, filename);
	///////////// step 1, connect //////////////////////
	conn = connectToServer((char*)server.c_str(), 80);
	///////////// step 2, send POST request /////////////
	sprintf(tmpBuffer, "POST /server_vr/test_upload.php HTTP/1.1\r\n");
	strcpy(sendBuffer, tmpBuffer);
	sprintf(tmpBuffer, "Host: localhost\r\n");
	strcat(sendBuffer, tmpBuffer);
	sprintf(tmpBuffer, "Content-Length: %d\r\n", 
		strlen("--AaB03x\r\nContent-Disposition: form-data; name=\"fileToUpload\"; filename=\"file1.txt\"\r\nContent-Type: application/octet-stream\r\n")
		+strlen("Content-Type: text/plain\r\n\r\n123\r\n--AaB03x\r\n"));
	strcat(sendBuffer, tmpBuffer);
	sprintf(tmpBuffer, "Content-Type: multipart/form-data; boundary=AaB03x\r\n");
	strcat(sendBuffer, tmpBuffer);
	sprintf(tmpBuffer, "--AaB03x\r\nContent-Disposition: form-data; name=\"fileToUpload\"; filename=\"file1.txt\"\r\nContent-Type: application/octet-stream\r\n", 3);
	strcat(sendBuffer, tmpBuffer);
	sprintf(tmpBuffer, "Content-Type: text/plain\r\n%s\r\n--AaB03x\r\n", "123");
	strcat(sendBuffer, tmpBuffer);
	send(conn, sendBuffer, strlen(sendBuffer), 0);
	printf(sendBuffer);
	///////////// step 3 - get received bytes ////////////////
	// Receive until the peer closes the connection
	totalBytesRead = 0;
	while (1)
	{
		memset(readBuffer, 0, bufSize);
		thisReadSize = recv(conn, readBuffer, bufSize, 0);

		if (thisReadSize <= 0)
			break;

		tmpResult = (char*)realloc(tmpResult, thisReadSize + totalBytesRead);

		memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
		totalBytesRead += thisReadSize;
	}
	headerLen = getHeaderLength(tmpResult);
	long contenLen = totalBytesRead - headerLen;
	result = new char[contenLen + 1];
	memcpy(result, tmpResult + headerLen, contenLen);
	result[contenLen] = 0x0;
	char *myTmp;

	myTmp = new char[headerLen + 1];
	strncpy(myTmp, tmpResult, headerLen);
	myTmp[headerLen] = NULL;
	delete(tmpResult);
	*headerOut = myTmp;

	bytesReturnedOut = contenLen;
	closesocket(conn);
	return(result);
}
void mParseUrl(char *mUrl, string &serverName, string &filepath, string &filename){
	string::size_type n;
	string url = mUrl;

	if (url.substr(0, 7) == "http://")
		url.erase(0, 7);

	if (url.substr(0, 8) == "https://")
		url.erase(0, 8);

	n = url.find('/');
	if (n != string::npos)
	{
		serverName = url.substr(0, n);
		filepath = url.substr(n);
		n = filepath.rfind('/');
		filename = filepath.substr(n + 1);
	}

	else
	{
		serverName = url;
		filepath = "/";
		filename = "";
	}
}

SOCKET connectToServer(char *szServerName, WORD portNum){
	struct hostent *hp;
	unsigned int addr;
	struct sockaddr_in server;
	SOCKET conn;

	conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn == INVALID_SOCKET)
		return NULL;

	if (inet_addr(szServerName) == INADDR_NONE)
	{
		hp = gethostbyname(szServerName);
	}
	else
	{
		addr = inet_addr(szServerName);
		hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
	}

	if (hp == NULL)
	{
		closesocket(conn);
		return NULL;
	}

	server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(portNum);
	if (connect(conn, (struct sockaddr*)&server, sizeof(server)))
	{
		closesocket(conn);
		return NULL;
	}
	return conn;
}

int getHeaderLength(char *content){
	const char *srchStr1 = "\r\n\r\n", *srchStr2 = "\n\r\n\r";
	char *findPos;
	int ofset = -1;

	findPos = strstr(content, srchStr1);
	if (findPos != NULL)
	{
		ofset = findPos - content;
		ofset += strlen(srchStr1);
	}

	else
	{
		findPos = strstr(content, srchStr2);
		if (findPos != NULL)
		{
			ofset = findPos - content;
			ofset += strlen(srchStr2);
		}
	}
	return ofset;
}