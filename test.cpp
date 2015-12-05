#include "CppSQLite.h"
#include <ctime>
#include <iostream>

using namespace std;

const char* gszFile = "C:\\test.db";

int main(int argc, char** argv)
{
    try
    {
        int i, fld;
        time_t tmStart, tmEnd;
        CppSQLiteDB db;

        cout << "SQLite Version: " << db.SQLiteVersion() << endl;

        remove(gszFile);
        db.open(gszFile);
