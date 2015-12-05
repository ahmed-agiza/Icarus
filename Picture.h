#ifndef PICTURE_H
#define PICTURE_H
#include "Common/sqlite3.h"
#include "Common/CppSQLite.h"

class Picture{
	private:
		string PictureID = NULL;
		string Path_pic = NULL;
		string OwnerID = NULL;
		struct Permission{
			string TransID = NULL;
			string UID = NULL;
			int RCount = NULL;
		};
		vector <Permission> Perms;
		CppSQLiteDB myFriends;
		
	public:
		Picture(string gPath_pic);
		Picture(string gPath_pic, string gPictureID, string gOwnerID, vector <Permission> gPerms);
		bool removeImage();
		string getPath_pic();
		string getOwnerID();
		bool viewImage(string gUID); //would call delete if RCount becomes zero
		vector <Permission> getPermissions();
		bool addPermission(string UID, int RCount);
		bool deletePermission(string UID);
		bool mofidyPermissions(string UID, int RCount);
		~Picture();
}