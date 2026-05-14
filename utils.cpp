#include "utils.h"
#include <iostream>
#include <mysql.h>
#include <sstream>
#include <cstring>
#include <ctime>

using namespace std;

string hashPassword(const string &p)
{
    long long total = 0;
    string result = "";
    for (int i = 0; i < p.length(); i++)
    {
        total = total * 31 + (int)p[i];
    }
    string chars = "#0$1&2!3|45678UVWX9ABCDEFGHIJKLMNOPQRSTYZabcdefghijklmnopqrstuvwxyz";
    int base = chars.length();
    int counter = 0;
    while (result.length() < 100)
    {
        if (total == 0)
        {
            total = (counter * 7919) + (p.length() * 1009);
            counter++;
        }
        result = chars[total % base] + result;
        total = total / base;
    }
    return result;
}

bool connectDatabase(MYSQL *&conn)
{
    conn = mysql_init(NULL);
    if (!conn)
    {
        cout << "MySQL initialization failed!\n";
        return false;
    }
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "root", "parking_system", 3306, NULL, 0))
    {
        cout << "Connection failed: " << mysql_error(conn) << endl;
        return false;
    }
    cout << "Connected to database successfully!\n";
    return true;
}

void disconnectDatabase(MYSQL *conn)
{
    if (conn)
        mysql_close(conn);
}

bool isFirstUser(MYSQL *conn)
{
    string q = "SELECT COUNT(*) FROM User";
    if (mysql_query(conn, q.c_str()))
        return true;
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r)
        return true;
    MYSQL_ROW row = mysql_fetch_row(r);
    int count = row ? atoi(row[0]) : 0;
    mysql_free_result(r);
    return (count == 0);
}

bool loginUser(MYSQL *conn, string email, string password, string &currentUser, int &currentUserId, string &role)
{
    string hashed = hashPassword(password);
    string q = "SELECT user_id, full_name, role FROM User WHERE email='" + email + "' AND password_hash='" + hashed + "' AND status='active'";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Query error!\n";
        return false;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r)
        return false;
    MYSQL_ROW row = mysql_fetch_row(r);
    if (row)
    {
        currentUserId = atoi(row[0]);
        currentUser = row[1];
        role = row[2];
        mysql_free_result(r);
        cout << "\nLOGIN SUCCESSFUL!\n";
        cout << "Welcome, " << currentUser << " (Role: " << role << ")\n";
        return true;
    }
    mysql_free_result(r);
    cout << "\nLogin failed! Wrong email or password.\n";
    return false;
}

bool registerUser(MYSQL *conn, string name, string email, string phone, string password, bool isVip)
{
    bool firstUser = isFirstUser(conn);
    string hashed = hashPassword(password);
    string vip = isVip ? "TRUE" : "FALSE";
    string role = firstUser ? "admin" : "customer";

    string q = "INSERT INTO User(password_hash,role,full_name,email,phone_number,is_vip,status) VALUES('" + hashed + "','" + role + "','" + name + "','" + email + "','" + phone + "'," + vip + ",'active')";

    if (mysql_query(conn, q.c_str()))
    {
        cout << "Registration failed: " << mysql_error(conn) << endl;
        return false;
    }

    cout << "\nREGISTRATION SUCCESSFUL!\n";

    if (firstUser)
    {
        cout << "You are registered as the FIRST USER and ADMINISTRATOR!\n";
        cout << "You have full system access including:\n";
        cout << "- Slot Lifecycle Management\n";
        cout << "- Dynamic Financial Configuration\n";
        cout << "- Inventory Categorization\n";
        cout << "- Comprehensive Monitoring\n";
    }
    else
    {
        if (role == "admin")
        {
            cout << "You are registered as ADMINISTRATOR!\n";
        }
        else
        {
            cout << "You are registered as CUSTOMER!\n";
            if (isVip)
            {
                cout << "VIP Member - Access to premium spots!\n";
            }
            else
            {
                cout << "Standard Member - Upgrade to VIP anytime!\n";
            }
        }
    }
    return true;
}

void logoutUser(string &currentUser, int &currentUserId, bool &isLoggedIn)
{
    currentUser = "";
    currentUserId = 0;
    isLoggedIn = false;
    cout << "\nYou have been logged out.\n";
}