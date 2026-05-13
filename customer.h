#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <mysql.h>
#include <string>

using namespace std;

void viewAvailableSlots(MYSQL *conn, int currentUserId);
void bookParkingSpot(MYSQL *conn, int currentUserId);
void checkOutParking(MYSQL *conn, int currentUserId);
void viewMyBookings(MYSQL *conn, int currentUserId);
void viewMyProfile(MYSQL *conn, int currentUserId);
void updatePassword(MYSQL *conn, int currentUserId);
void customerDashboard(MYSQL *conn, string &currentUser, int &currentUserId, bool &isLoggedIn);

#endif