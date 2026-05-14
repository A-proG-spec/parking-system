#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <mysql.h>
#include <string>
using namespace std;

bool isUserVIP(MYSQL *conn, int uid);
void viewAvailableSlots(MYSQL *conn, int uid);
void bookParkingSpot(MYSQL *conn, int uid);
void checkOutParking(MYSQL *conn, int uid);
void viewMyBookings(MYSQL *conn, int uid);
void viewMyProfile(MYSQL *conn, int uid);
void updatePassword(MYSQL *conn, int uid);
void customerDashboard(MYSQL *conn, string &user, int &uid, bool &loggedIn);

#endif