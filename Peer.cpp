#include "Peer.h"
#include "Common/sqlite3.h"
#include "Common/CppSQLite.h"

Peer::Peer(){

}

Peer::Peer(string gUID){
	string xUsername, xPublicKey, xKey2;

	//sql fetch the row from the database
	CppSQLiteBuffer bufSQL;
	bufSQL.format("insert into bindata values ('variable', %Q);", gUID);
	myFriends.execDML(bufSQL);
	CppSQLiteQuery q = myFriends.execQuery("SELECT * FROM Peers WHERE UID = 'variable';");
	if (!q.eof())
	xUsername = q.fieldValue(2);
	xPublicKey = q.fieldValue(3);
	xKey2.fieldValue(4);

	if(!setUID(gUID)) {
		return -1;
	}
	if(!setUsername(xUsername)) {
		return -1;
	}
	if(!setPublicKey(xPublicKey)) {
		return -1;
	}
	if(!setKey2(xKey2)) {
		return -1;
	}

}

void Peer::insert(){
	CppSQLiteBuffer bufSQL;
	
	
	if(UID!=NULL && Username!=NULL && PublicKey!=NULL && Key2!=NULL){
		bufSQL.format("INSERT INTO Peers (UID, Username, PublicKey, Key2) VALUES (‘%s’,’%s’,’%s’,’%s’);", UID, Username, PublicKey, Key2);
		int nRows = myFriends.execDML(bufSQL);
	}
	else{
		return -1;
	}
}

bool Peer::setUID(string sUID){
	if (sUID == NULL) return -1;
	UID = sUID;
	return true;
}

bool Peer::setUsername(string sUsername){
	if (sUsername == NULL) return -1;
	Username = sUsername;
	return true;
}

bool Peer::setPublicKey(string sPublicKey){
	if (sPublicKey == NULL) return -1;
	PublicKey = sPublicKey;
	return true;
}

bool Peer::setKey2(string sKey2){
	if (sKey2 == NULL) return -1;
	Key2 = sKey2;
	return true;
}

string Peer::getUsername(){
	return Username;
}

string Peer::getPublicKey(){
	return PublicKey;
}
	
string Peer::getKey2(){
	return Key2;
}

Peer::~Peer(){

}