#ifndef PEER_H
#define PEER_H
#include "Common/sqlite3.h"
#include "Common/CppSQLite.h"

class Request{
private:
	string RequestID = NULL;
	string OwnerID = NULL;
	string SenderID = NULL;
	string Timestamp = NULL;
	CppSQLiteDB myFriends;

public:
	Request();
	void insert();
	bool setRequestID(string sRequestID);
	bool setOwnerID(string sOwnerID);
	bool setSenderID(string sSenderID);
	bool setTimestamp(string sTimesamp);
	string getRequestID(string sRequestID);
	string getOwnerID(string sOwnerID);
	string getSenderID(string sSenderID);
	string getTimestamp(string sTimesamp);
	~Request();
}