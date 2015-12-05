#ifndef PEER_H
#define PEER_H
#include "Common/sqlite3.h"
#include "Common/CppSQLite.h"

class Peer{
	private:
		string ID = NULL;
		string UID = NULL;
		string Username = NULL;
		string PublicKey= NULL;
		string Key2= NULL;
		CppSQLiteDB myFriends;

	public:
		Peer();
		Peer(string gUID);
		void insert();
		bool setUID(string sUID);
		bool setUsername(string sUsername);
		bool setPublicKey(string sPublicKey);
		bool setKey2(string sKey2);
		string getUsername();
		string getPublicKey();
		string getKey2();
		~Peer();
}