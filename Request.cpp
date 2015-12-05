#include "Request.h"
#include "Common/sqlite3.h"
#include "Common/CppSQLite.h"

Request::Request(){

}
void Request::insert(){
	CppSQLiteBuffer bufSQL;
	
	if(RequestID!=NULL && OwnderID!=NULL && SenderID!=NULL && Timestamp!=NULL){
		bufSQL.format("INSERT INTO Request (RequestID, OwnderID, SenderID, Timestamp) VALUES (‘%s’,’%s’,’%s’,’%s’);", RequestID, OwnderID, SenderID, Timestamp);
		int nRows = myFriends.execDML(bufSQL);
	}
	else{
		return -1;
	}
}

bool setRequestID(string sRequestID){
	if (sRequestID == NULL) return -1;
	RequestID = sRequestID;
	return true;
}

bool setOwnerID(string sOwnerID){
	if (sOwnerID == NULL) return -1;
	OwnerID = sOwnerID;
	return true;
}

bool setSenderID(string sSenderID){
	if (SenderID == NULL) return -1;
	SenderID = SenderID;
	return true;
}

bool setTimestamp(string sTimesamp){
	if (sTimesamp == NULL) return -1;
	Timesamp = sTimesamp;
	return true;
}

string getRequestID(string sRequestID){
	return RequestID;
}

string getOwnerID(string sOwnerID){
	return OwnderID;
}

string getSenderID(string sSenderID){
	return SenderID;
}


string getTimestamp(string sTimesamp){
	return Timesamp;
}


Peer::~Peer(){

}
