#ifndef UTILS_H
#define UTILS_H

#include <mysql.h>
#include <string>
using namespace std;

bool connectDatabase(MYSQL*& conn);
void disconnectDatabase(MYSQL* conn);
string hashPassword(const string& p);
bool isFirstUser(MYSQL *conn);
bool loginUser(MYSQL* conn, string email, string password, string& currentUser, int& currentUserId, string& role);
bool registerUser(MYSQL* conn, string name, string email, string phone, string password, bool isVip);
void logoutUser(string& currentUser, int& currentUserId, bool& isLoggedIn);

#endif