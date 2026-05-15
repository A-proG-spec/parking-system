#include "customer.h"
#include "utils.h"
#include <iostream>
#include <mysql.h>
#include <ctime>
#include <cmath>
using namespace std;

bool isUserVIP(MYSQL *conn, int uid)
{
    string q = "SELECT is_vip FROM User WHERE user_id=" + to_string(uid);
    if (mysql_query(conn, q.c_str()))
        return false;
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r)
        return false;
    MYSQL_ROW row = mysql_fetch_row(r);
    bool vip = row && (atoi(row[0]) == 1);
    mysql_free_result(r);
    return vip;
}

void viewAvailableSlots(MYSQL *conn, int uid)
{
    cout << "\nAVAILABLE PARKING SLOTS\n";
    bool vip = isUserVIP(conn, uid);
    string q = "SELECT slot_id,slot_number,type,price_per_hour FROM ParkingSlot WHERE status='available' AND is_occupied=FALSE" + (string)(vip ? "" : " AND type!='vip'") + " ORDER BY slot_number";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r)
    {
        cout << "No slots found!\n";
        return;
    }
    MYSQL_ROW row;
    int cnt = 0;
    cout << "\nID\tSlot #\t\tType\t\tRate(ETB/hr)\n";
    while ((row = mysql_fetch_row(r)))
    {
        cnt++;
        cout << row[0] << "\t" << row[1] << "\t\t" << row[2] << "\t\t" << atof(row[3]) << "\n";
    }
    cout << "\nTotal Available: " << cnt << " slots\n";
    cout << (vip ? "VIP: Access to ALL slots including premium spots" : "Standard: Upgrade to VIP for premium spots") << "\n";
    mysql_free_result(r);
}

void bookParkingSpot(MYSQL *conn, int uid)
{
    cout << "\nBOOK PARKING SPOT\n";
    bool vip = isUserVIP(conn, uid);
    if (!vip)
        cout << "Note: Standard customers can only book standard slots\n";
    viewAvailableSlots(conn, uid);
    string vq = "SELECT vehicle_id,plate_number,vehicle_type FROM Vehicle WHERE user_id=" + to_string(uid);
    if (mysql_query(conn, vq.c_str()))
    {
        cout << "Query failed!\n";
        return;
    }
    MYSQL_RES *vr = mysql_store_result(conn);
    if (!vr || mysql_num_rows(vr) == 0)
    {
        cout << "\nNo vehicles registered!\nPlease register a vehicle first.\n";
        string plate, type, brand, color;
        cout << "\nREGISTER VEHICLE \nPlate: ";
        cin >> plate;
        cout << "Type(car/motorcycle/truck): ";
        cin >> type;
        cout << "Brand: ";
        cin >> brand;
        cout << "Color: ";
        cin >> color;
        string ins = "INSERT INTO Vehicle(plate_number,user_id,vehicle_type,brand,color) VALUES('" + plate + "'," + to_string(uid) + ",'" + type + "','" + brand + "','" + color + "')";
        if (mysql_query(conn, ins.c_str()))
        {
            cout << "Registration failed!\n";
            if (vr)
                mysql_free_result(vr);
            return;
        }
        cout << "Vehicle registered!\n";
        cin.clear();
        cin.ignore(10000, '\n');
        if (mysql_query(conn, vq.c_str()))
        {
            cout << "Query failed!\n";
            return;
        }
        if (vr)
            mysql_free_result(vr);
        vr = mysql_store_result(conn);
    }
    cout << "\nYOUR VEHICLES\n";
    MYSQL_ROW vrow;
    while ((vrow = mysql_fetch_row(vr)))
        cout << "ID:" << vrow[0] << " | " << vrow[1] << " | " << vrow[2] << endl;
    mysql_data_seek(vr, 0);
    int sid, vid;
    cout << "\nEnter Slot ID: ";
    cin >> sid;
    cout << "Enter Vehicle ID: ";
    cin >> vid;
    string stq = "SELECT type FROM ParkingSlot WHERE slot_id=" + to_string(sid);
    string stype = "standard";
    if (!mysql_query(conn, stq.c_str()))
    {
        MYSQL_RES *tr = mysql_store_result(conn);
        if (tr)
        {
            MYSQL_ROW trow = mysql_fetch_row(tr);
            if (trow)
                stype = trow[0];
            mysql_free_result(tr);
        }
    }
    if (stype == "vip" && !vip)
    {
        cout << "\nERROR: VIP slots only for VIP members!\n";
        mysql_free_result(vr);
        return;
    }
    string pq = "SELECT price_per_hour FROM ParkingSlot WHERE slot_id=" + to_string(sid);
    float rate = 50;
    if (!mysql_query(conn, pq.c_str()))
    {
        MYSQL_RES *pr = mysql_store_result(conn);
        if (pr)
        {
            MYSQL_ROW prow = mysql_fetch_row(pr);
            if (prow)
                rate = atof(prow[0]);
            mysql_free_result(pr);
        }
    }
    string bq = "INSERT INTO Booking(user_id,slot_id,vehicle_id,start_time,booking_status,payment_status) VALUES(" + to_string(uid) + "," + to_string(sid) + "," + to_string(vid) + ",NOW(),'active','pending')";
    if (!mysql_query(conn, bq.c_str()))
    {
        cout << "\nParking spot booked!\nRate: " << rate << " ETB/hour\n";
        mysql_query(conn, ("UPDATE ParkingSlot SET is_occupied=TRUE,status='occupied' WHERE slot_id=" + to_string(sid)).c_str());
    }
    else
        cout << "\nBooking failed: " << mysql_error(conn) << endl;
    mysql_free_result(vr);
}

void checkOutParking(MYSQL *conn, int uid)
{
    cout << "\nCHECK OUT & BILLING\n";
    string q = "SELECT b.booking_id,ps.slot_number,b.start_time,ps.price_per_hour,b.slot_id FROM Booking b JOIN ParkingSlot ps ON b.slot_id=ps.slot_id WHERE b.user_id=" + to_string(uid) + " AND b.booking_status='active'";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r || mysql_num_rows(r) == 0)
    {
        cout << "\nNo active bookings\n";
        if (r)
            mysql_free_result(r);
        return;
    }
    cout << "\nActive Bookings:\n";
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(r)))
        cout << "Booking #" << row[0] << " | Slot:" << row[1] << " | Started:" << row[2] << " | Rate:" << row[3] << " ETB/hr\n";

    int bid;
    cout << "\nEnter Booking ID: ";
    cin >> bid;

    string dq = "SELECT start_time,price_per_hour,b.slot_id FROM Booking b JOIN ParkingSlot ps ON b.slot_id=ps.slot_id WHERE b.booking_id=" + to_string(bid) + " AND b.user_id=" + to_string(uid);
    if (mysql_query(conn, dq.c_str()))
    {
        cout << "Error!\n";
        mysql_free_result(r);
        return;
    }
    MYSQL_RES *dr = mysql_store_result(conn);
    if (!dr || mysql_num_rows(dr) == 0)
    {
        cout << "Booking not found!\n";
        if (dr)
            mysql_free_result(dr);
        mysql_free_result(r);
        return;
    }

    MYSQL_ROW drow = mysql_fetch_row(dr);
    string startTimeStr = drow[0];
    float rate = atof(drow[1]);
    int sid = atoi(drow[2]);

    time_t now = time(nullptr);
    struct tm startTm = {0};
    int year, month, day, hour, minute, second;
    sscanf(startTimeStr.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    startTm.tm_year = year - 1900;
    startTm.tm_mon = month - 1;
    startTm.tm_mday = day;
    startTm.tm_hour = hour;
    startTm.tm_min = minute;
    startTm.tm_sec = second;
    time_t startTime = mktime(&startTm);

    double secondsParked = difftime(now, startTime);
    float hoursParked = (secondsParked > 0) ? (secondsParked / 3600.0) : 0;
    float billableHours = ceil(hoursParked * 100) / 100;
    if (billableHours < 0.5 && secondsParked > 0)
        billableHours = 0.5;
    float total = billableHours * rate;

    cout << "\nINVOICE\n";
    cout << "Start Time: " << startTimeStr << "\n";
    cout << "Duration: " << hoursParked << " hours\n";
    cout << "Billable: " << billableHours << " hours @ " << rate << " ETB/hr\n";
    cout << "Total Due: " << total << " ETB\n";

    string method;
    cout << "\nPayment Method (cash/credit_card/telebirr): ";
    cin >> method;

    string ub = "UPDATE Booking SET end_time=NOW(),duration=" + to_string(billableHours) + ",total_fee=" + to_string(total) + ",payment_status='paid',booking_status='completed' WHERE booking_id=" + to_string(bid);
    if (!mysql_query(conn, ub.c_str()))
    {
        mysql_query(conn, ("INSERT INTO Payment(booking_id,amount,payment_method,payment_status) VALUES(" + to_string(bid) + "," + to_string(total) + ",'" + method + "','completed')").c_str());
        mysql_query(conn, ("UPDATE ParkingSlot SET is_occupied=FALSE,status='available' WHERE slot_id=" + to_string(sid)).c_str());
        cout << "\nCheckout successful! Invoice generated.\n";
    }
    else
        cout << "\nCheckout failed: " << mysql_error(conn) << endl;

    mysql_free_result(dr);
    mysql_free_result(r);
}

void viewMyBookings(MYSQL *conn, int uid)
{
    cout << "\nMY BOOKINGS\n";
    string q = "SELECT b.booking_id,ps.slot_number,b.start_time,b.end_time,b.total_fee,b.booking_status,b.payment_status FROM Booking b JOIN ParkingSlot ps ON b.slot_id=ps.slot_id WHERE b.user_id=" + to_string(uid) + " ORDER BY b.created_at DESC";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r)
    {
        cout << "No bookings\n";
        return;
    }
    MYSQL_ROW row;
    int cnt = 0;
    while ((row = mysql_fetch_row(r)))
    {
        cnt++;
        cout << "\nBooking #" << row[0] << "\n   Slot: " << row[1] << "\n   Start: " << row[2] << "\n   End: " << (row[3] ? row[3] : "Still Active") << "\n   Fee: " << (row[4] ? string(row[4]) + " ETB" : "Pending") << "\n   Status: " << row[5] << " | Payment: " << row[6] << endl;
    }
    if (cnt == 0)
        cout << "\nNo bookings yet\n";
    mysql_free_result(r);
}

void viewMyProfile(MYSQL *conn, int uid)
{
    cout << "\nMY PROFILE\n";
    string q = "SELECT full_name,email,phone_number,is_vip,created_at,status FROM User WHERE user_id=" + to_string(uid);
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error!\n";
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r)
    {
        cout << "Profile not found\n";
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(r);
    if (row)
    {
        cout << "\nName: " << row[0] << "\nEmail: " << row[1] << "\nPhone: " << row[2];
        cout << "\nVIP: " << (atoi(row[3]) == 1 ? "Yes (Premium)" : "No (Standard)");
        cout << "\nMember Since: " << row[4] << "\nStatus: " << row[5] << endl;
    }
    mysql_free_result(r);
}

void viewMyVehicles(MYSQL *conn, int uid)
{
    cout << "\nMY VEHICLES\n";
    string vq = "SELECT vehicle_id, plate_number, vehicle_type, brand, color FROM Vehicle WHERE user_id=" + to_string(uid);
    if (mysql_query(conn, vq.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *vr = mysql_store_result(conn);
    if (!vr || mysql_num_rows(vr) == 0)
    {
        cout << "No vehicles registered\n";
        if (vr)
            mysql_free_result(vr);
        return;
    }
    cout << "\nID\tPlate Number\tType\t\tBrand\t\tColor\n";
    MYSQL_ROW vrow;
    while ((vrow = mysql_fetch_row(vr)))
    {
        cout << vrow[0] << "\t" << vrow[1] << "\t\t" << vrow[2] << "\t\t" << vrow[3] << "\t\t" << vrow[4] << endl;
    }
    mysql_free_result(vr);
}

void registerVehicle(MYSQL *conn, int currentUserId)
{
    cout << "\nREGISTER VEHICLE\n";
    string plateNumber, vehicleType, brand, color;
    cout << "Enter Plate Number: ";
    cin >> plateNumber;
    cout << "Vehicle Type (car/motorcycle/truck): ";
    cin >> vehicleType;
    cout << "Brand: ";
    cin >> brand;
    cout << "Color: ";
    cin >> color;
    
    string insertQuery = "INSERT INTO Vehicle (plate_number, user_id, vehicle_type, brand, color) VALUES ('";
    insertQuery += plateNumber + "', " + to_string(currentUserId) + ", '" + vehicleType + "', '" + brand + "', '" + color + "')";
    
    if (mysql_query(conn, insertQuery.c_str()) == 0)
    {
        cout << "\nVEHICLE REGISTERED SUCCESSFULLY!\n";
        cout << "Plate: " << plateNumber << "\nType: " << vehicleType << "\nBrand: " << brand << "\nColor: " << color << endl;
    }
    else
    {
        cout << "\nFailed to register vehicle: " << mysql_error(conn) << endl;
    }
    cin.clear();
    cin.ignore(10000, '\n');
}

void updateVehicle(MYSQL *conn, int uid)
{
    viewMyVehicles(conn, uid);
    
    int vid;
    cout << "\nEnter Vehicle ID to update: ";
    cin >> vid;
    
    string check = "SELECT vehicle_id FROM Vehicle WHERE vehicle_id=" + to_string(vid) + " AND user_id=" + to_string(uid);
    if (mysql_query(conn, check.c_str()))
    {
        cout << "Error checking vehicle!\n";
        return;
    }
    MYSQL_RES *cr = mysql_store_result(conn);
    if (!cr || mysql_num_rows(cr) == 0)
    {
        cout << "Vehicle not found or doesn't belong to you!\n";
        if (cr)
            mysql_free_result(cr);
        return;
    }
    mysql_free_result(cr);
    
    string plate, type, brand, color;
    cout << "\nEnter new Plate Number: ";
    cin >> plate;
    cout << "Enter new Vehicle Type (car/motorcycle/truck): ";
    cin >> type;
    cout << "Enter new Brand: ";
    cin >> brand;
    cout << "Enter new Color: ";
    cin >> color;
    
    string up = "UPDATE Vehicle SET plate_number='" + plate + "', vehicle_type='" + type + "', brand='" + brand + "', color='" + color + "' WHERE vehicle_id=" + to_string(vid);
    
    if (!mysql_query(conn, up.c_str()))
        cout << "\nVehicle updated successfully!\n";
    else
        cout << "\nUpdate failed: " << mysql_error(conn) << endl;
    
    cin.clear();
    cin.ignore(10000, '\n');
}

void deleteVehicle(MYSQL *conn, int uid)
{
    viewMyVehicles(conn, uid);
    
    int vid;
    cout << "\nEnter Vehicle ID to delete: ";
    cin >> vid;
    
    string check = "SELECT vehicle_id FROM Vehicle WHERE vehicle_id=" + to_string(vid) + " AND user_id=" + to_string(uid);
    if (mysql_query(conn, check.c_str()))
    {
        cout << "Error checking vehicle!\n";
        return;
    }
    MYSQL_RES *cr = mysql_store_result(conn);
    if (!cr || mysql_num_rows(cr) == 0)
    {
        cout << "Vehicle not found or doesn't belong to you!\n";
        if (cr)
            mysql_free_result(cr);
        return;
    }
    mysql_free_result(cr);
    
    cout << "Are you sure? (1=Yes, 0=No): ";
    int confirm;
    cin >> confirm;
    
    if (confirm == 1)
    {
        string del = "DELETE FROM Vehicle WHERE vehicle_id=" + to_string(vid);
        if (!mysql_query(conn, del.c_str()))
            cout << "\nVehicle deleted successfully!\n";
        else
            cout << "\nDelete failed: " << mysql_error(conn) << endl;
    }
    else
    {
        cout << "\nDeletion cancelled.\n";
    }
    cin.clear();
    cin.ignore(10000, '\n');
}

void updatePassword(MYSQL *conn, int uid)
{
    cout << "\nUPDATE PASSWORD\n";
    string oldp, newp, conf;
    cout << "Current Password: ";
    cin >> oldp;
    cout << "New Password: ";
    cin >> newp;
    cout << "Confirm: ";
    cin >> conf;
    if (newp != conf)
    {
        cout << "\nPasswords don't match!\n";
        return;
    }
    string hashedOld = hashPassword(oldp);
    string cq = "SELECT user_id FROM User WHERE user_id=" + to_string(uid) + " AND password_hash='" + hashedOld + "'";
    if (mysql_query(conn, cq.c_str()))
    {
        cout << "Error!\n";
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r || mysql_num_rows(r) == 0)
    {
        cout << "\nCurrent password incorrect\n";
        if (r)
            mysql_free_result(r);
        return;
    }
    mysql_free_result(r);
    string hashedNew = hashPassword(newp);
    string uq = "UPDATE User SET password_hash='" + hashedNew + "' WHERE user_id=" + to_string(uid);
    if (!mysql_query(conn, uq.c_str()))
        cout << "\nPassword updated!\n";
    else
        cout << "\nUpdate failed!\n";
}

void customerDashboard(MYSQL *conn, string &user, int &uid, bool &loggedIn)
{
    int ch;
    bool back = false;
    while (!back)
    {
        cout << "\nCUSTOMER DASHBOARD\n";
        bool vip = isUserVIP(conn, uid);
        cout << "Welcome, " << user << (vip ? " (VIP)" : " (Standard)") << "\n";
        cout << "1. View Available Slots\n";
        cout << "2. Book Parking Spot\n";
        cout << "3. Check Out & Get Invoice\n";
        cout << "4. View My Bookings\n";
        cout << "5. View My Vehicles\n";
        cout << "6. Register Vehicle\n";
        cout << "7. Update Vehicle\n";
        cout << "8. Delete Vehicle\n";
        cout << "9. My Profile\n";
        cout << "10. Update Password\n";
        cout << "11. Logout\n";
        cout << "Choice: ";
        cin >> ch;
        
        if (ch == 1)
            viewAvailableSlots(conn, uid);
        else if (ch == 2)
            bookParkingSpot(conn, uid);
        else if (ch == 3)
            checkOutParking(conn, uid);
        else if (ch == 4)
            viewMyBookings(conn, uid);
        else if (ch == 5)
            viewMyVehicles(conn, uid);
        else if (ch == 6)
            registerVehicle(conn, uid);
        else if (ch == 7)
            updateVehicle(conn, uid);
        else if (ch == 8)
            deleteVehicle(conn, uid);
        else if (ch == 9)
            viewMyProfile(conn, uid);
        else if (ch == 10)
            updatePassword(conn, uid);
        else if (ch == 11)
        {
            logoutUser(user, uid, loggedIn);
            back = true;
        }
        else
            cout << "\nInvalid choice!\n";
        
        if (!back && ch >= 1 && ch <= 10)
        {
            cout << "\nPress Enter...";
            cin.ignore(10000, '\n');
            cin.get();
        }
    }
}