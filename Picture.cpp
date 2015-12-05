#include "Picture.h"
#include "Common/sqlite3.h"
#include "Common/CppSQLite.h"

Picture::Picture(string gPath_pic){
	CppSQLiteBuffer bufSQL;
	Path_pic = gPath_pic
	OwnerID= "MyID"; //EDIT

}

Picture::Picture(string gPath_pic, string gPictureID, string gOwnerID, vector <Permission> gPerms){
	Path_pic = gPath_pic;
	PictureID = gPictureID;
	OwnerID = gOwnerID;
	Perms = gPerms
	bufSQL.format("INSERT INTO Pictures (PictureID, Path_pic, OwnerID) VALUES ('%s', '%s', '%s');", PictureID, Path_pic, OwnerID);
	int nRows = myFriends.execDML(bufSQL);
}

string getPath_pic(){
	return Path_pic;
}

string getOwnerID(){
	return OwnerID;
}

vector <Permission> getPermissions(){
	return Perms;
}

bool Picture::removeImage(){
	bufSQL.format("DELETE FROM Permissions WHERE PictureID = '%s';" , PictureID);
	int nRows = myFriends.execDML(bufSQL);
	bufSQL.format("DELETE FROM Pictures WHERE PictureID = '%s';" , PictureID);
	int nRows = myFriends.execDML(bufSQL);
	Perms.clear();
}
		
bool viewImage(string gUID) { //would call delete if RCount becomes zero
	if (Perms[TransID].RCount >= 1){
		Perms[TransID].RCount = Perms[TransID].RCount-1;
		return Path_pic;
	}
	else{

		return -2; //Error
	}
	
} 
		
bool addPermission(string gUID, int gRCount){
	string sTransID;
	CppSQLiteQuery q = myFriends.execQuery("SELECT TransID FROM Permissions ORDERED BY TransID DESC LIMIT 1;");
	if (!q.eof())
	sTransID = q+1;
	if ((gUID !=NULL) && (gRCount!=NULL)){
		Perms[sTransID].UID = gUID;
		Perms[sTransID].RCount = gRCount;
	}
	bufSQL.format("INSERT INTO Permissions (UID, RCount) VALUES ('%s', '%i');", Perms[sTransID].UID, Perms[sTransID].RCount);
	int nRows = myFriends.execDML(bufSQL);
}

bool deletePermission(string UID){
	//Remove from vector
	CppSQLiteQuery q = myFriends.execQuery("SELECT TransID FROM Permissions WHERE PictureID = '%s';" , PictureID);
	sTransID = q;
	Perms[sTransID].erase();
	bufSQL.format("DELETE FROM Permissions WHERE PictureID = '%s';" , PictureID);
	int nRows = myFriends.execDML(bufSQL);
}

bool mofidyPermissions(string UID, int RCount){
	string sTransID;
	CppSQLiteQuery q = myFriends.execQuery("SELECT TransID FROM Permissions WHERE PictureID = '%s';" , PictureID);
	sTransID = q;
	Perms[sTransID].erase();
	bufSQL.format("DELETE FROM Permissions WHERE PictureID = '%s';" , PictureID);
	int nRows = myFriends.execDML(bufSQL);
	CppSQLiteQuery q = myFriends.execQuery("SELECT TransID FROM Permissions ORDERED BY TransID DESC LIMIT 1;");
	if (!q.eof())
	sTransID = q+1;
	if ((gUID !=NULL) && (gRCount!=NULL)){
		Perms[sTransID].UID = gUID;
		Perms[sTransID].RCount = gRCount;
	}
	bufSQL.format("INSERT INTO Permissions (UID, RCount) VALUES ('%s', '%i');", Perms[sTransID].UID, Perms[sTransID].RCount);
	int nRows = myFriends.execDML(bufSQL);
}

Picture::~Picture(){

}