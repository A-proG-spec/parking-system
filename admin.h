#ifndef ADMIN_H
#define ADMIN_H

#include <mysql.h>
#include <string>
using namespace std;

void addParkingSlot(MYSQL *conn);
void modifyParkingSlot(MYSQL *conn);
void removeParkingSlot(MYSQL *conn);
void setRateConfig(MYSQL *conn);
void viewAllSlots(MYSQL *conn);
void adminDashboard(MYSQL *conn, string &adminUser, int &adminId, bool &loggedIn);

#endif