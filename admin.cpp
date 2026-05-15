#include "admin.h"
#include "utils.h"
#include <iostream>
#include <mysql.h>
#include <string>
using namespace std;

void addParkingSlot(MYSQL *conn)
{
    cout << "\nADD PARKING SLOT\n";
    string num, type, status = "available";
    float rate;
    cout << "Slot Number: ";
    cin >> num;
    cout << "Type (normal/vip): ";
    cin >> type;
    cout << "Price per Hour (ETB): ";
    cin >> rate;
    string q = "INSERT INTO ParkingSlot(slot_number,type,price_per_hour,status,is_occupied) VALUES('" + num + "','" + type + "'," + to_string(rate) + ",'" + status + "',FALSE)";
    if (!mysql_query(conn, q.c_str()))
    {
        cout << "\nSlot added successfully!\n";
    }
    else
    {
        cout << "\nFailed: " << mysql_error(conn) << "\n";
    }
}

void modifyParkingSlot(MYSQL *conn)
{
    cout << "\nMODIFY PARKING SLOT\n";
    int sid;
    cout << "Slot ID to modify: ";
    cin >> sid;
    string q = "SELECT slot_number,type,price_per_hour FROM ParkingSlot WHERE slot_id=" + to_string(sid);
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error!\n";
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r || mysql_num_rows(r) == 0)
    {
        cout << "Slot not found!\n";
        if (r)
            mysql_free_result(r);
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(r);
    cout << "\nCurrent: Slot #" << row[0] << " | Type: " << row[1] << " | Rate: " << row[2] << " ETB\n";
    mysql_free_result(r);
    string num, type;
    float rate;
    cout << "\nNew Slot Number (press Enter to keep same): ";
    cin.ignore();
    getline(cin, num);
    cout << "New Type (normal/vip, press Enter to keep same): ";
    getline(cin, type);
    cout << "New Price (press 0 to keep same): ";
    cin >> rate;
    string up = "UPDATE ParkingSlot SET ";
    if (!num.empty())
        up += "slot_number='" + num + "',";
    if (!type.empty())
        up += "type='" + type + "',";
    if (rate > 0)
        up += "price_per_hour=" + to_string(rate) + ",";
    if (up.back() == ',')
        up.pop_back();
    up += " WHERE slot_id=" + to_string(sid);
    if (!mysql_query(conn, up.c_str()))
    {
        cout << "\nSlot modified successfully!\n";
    }
    else
    {
        cout << "\nModification failed!\n";
    }
}

void removeParkingSlot(MYSQL *conn)
{
    cout << "\nREMOVE PARKING SLOT\n";
    int sid;
    cout << "Slot ID to remove: ";
    cin >> sid;
    cout << "\nWARNING: This will delete the slot permanently!\n";
    cout << "Confirm? (yes/no): ";
    string confirm;
    cin >> confirm;
    if (confirm != "yes")
    {
        cout << "Cancelled.\n";
        return;
    }
    string q = "DELETE FROM ParkingSlot WHERE slot_id=" + to_string(sid);
    if (!mysql_query(conn, q.c_str()))
    {
        cout << "\nSlot removed successfully!\n";
    }
    else
    {
        cout << "\nRemove failed: " << mysql_error(conn) << "\n";
    }
}

void setRateConfig(MYSQL *conn)
{
    cout << "\nDYNAMIC RATE CONFIGURATION\n";
    cout << "\nCurrent Rates:\n";
    string q = "SELECT slot_id,slot_number,type,price_per_hour FROM ParkingSlot ORDER BY type,slot_number";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error!\n";
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (r)
    {
        MYSQL_ROW row;
        cout << "ID\tSlot#\tType\t\tCurrent Rate(ETB)\n";
        while ((row = mysql_fetch_row(r)))
        {
            cout << row[0] << "\t" << row[1] << "\t" << row[2] << "\t\t" << atof(row[3]) << "\n";
        }
        mysql_free_result(r);
    }
    cout << "\nConfigure rates:\n";
    cout << "1. Update specific slot\n";
    cout << "2. Update all slots by type\n";
    cout << "Choice: ";
    int opt;
    cin >> opt;
    if (opt == 1)
    {
        int sid;
        float rate;
        cout << "Slot ID: ";
        cin >> sid;
        cout << "New Rate (ETB/hour): ";
        cin >> rate;
        string up = "UPDATE ParkingSlot SET price_per_hour=" + to_string(rate) + " WHERE slot_id=" + to_string(sid);
        if (!mysql_query(conn, up.c_str()))
        {
            cout << "\nRate updated successfully!\n";
        }
        else
        {
            cout << "\nUpdate failed!\n";
        }
    }
    else if (opt == 2)
    {
        string type;
        float rate;
        cout << "Type (normal/vip): ";
        cin >> type;
        cout << "New Rate (ETB/hour): ";
        cin >> rate;
        string up = "UPDATE ParkingSlot SET price_per_hour=" + to_string(rate) + " WHERE type='" + type + "'";
        if (!mysql_query(conn, up.c_str()))
        {
            cout << "\nAll " << type << " slots updated to " << rate << " ETB/hour!\n";
        }
        else
        {
            cout << "\nUpdate failed!\n";
        }
    }
}

void viewAllSlots(MYSQL *conn)
{
    cout << "\nCOMPREHENSIVE SLOT MONITORING\n";
    string q = "SELECT slot_id,slot_number,type,price_per_hour,status,is_occupied FROM ParkingSlot ORDER BY slot_number";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error!\n";
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r)
    {
        cout << "No slots found!\n";
        return;
    }
    MYSQL_ROW row;
    int total = 0, available = 0, occupied = 0;
    cout << "\nID\tSlot#\tType\tRate\tStatus\t\tOccupied\n";
    while ((row = mysql_fetch_row(r)))
    {
        total++;
        string status = row[4];
        bool occ = atoi(row[5]);
        if (status == "available" && !occ)
            available++;
        else if (status == "occupied" || occ)
            occupied++;
        cout << row[0] << "\t" << row[1] << "\t" << row[2] << "\t" << atof(row[3]) << "\t" << status << "\t\t" << (occ ? "Yes" : "No") << "\n";
    }
    cout << "\nSUMMARY\n";
    cout << "Total Slots: " << total << "\n";
    cout << "Available: " << available << "\n";
    cout << "Occupied: " << occupied << "\n";
    mysql_free_result(r);
}

void viewAllUsers(MYSQL *conn)
{
    cout << "\nALL REGISTERED USERS\n";
    string q = "SELECT user_id, full_name, email, phone_number, role, is_vip, status FROM User ORDER BY user_id";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r || mysql_num_rows(r) == 0)
    {
        cout << "No users found!\n";
        if (r)
            mysql_free_result(r);
        return;
    }
    MYSQL_ROW row;
    cout << "\nID\tName\t\tEmail\t\tPhone\t\tRole\t\tVIP\tStatus\n";
    while ((row = mysql_fetch_row(r)))
    {
        cout << row[0] << "\t" << row[1] << "\t" << row[2] << "\t" << row[3] << "\t" << row[4] << "\t\t" << (atoi(row[5]) ? "Yes" : "No") << "\t" << row[6] << "\n";
    }
    mysql_free_result(r);
}

void viewUserVehicles(MYSQL *conn)
{
    viewAllUsers(conn);
    int uid;
    cout << "\nEnter User ID to view vehicles: ";
    cin >> uid;
    
    string check = "SELECT full_name FROM User WHERE user_id=" + to_string(uid);
    if (mysql_query(conn, check.c_str()))
    {
        cout << "Error!\n";
        return;
    }
    MYSQL_RES *cr = mysql_store_result(conn);
    if (!cr || mysql_num_rows(cr) == 0)
    {
        cout << "User not found!\n";
        if (cr)
            mysql_free_result(cr);
        return;
    }
    MYSQL_ROW crow = mysql_fetch_row(cr);
    cout << "\nVEHICLES FOR USER: " << crow[0] << "\n";
    mysql_free_result(cr);
    
    string q = "SELECT vehicle_id, plate_number, vehicle_type, brand, color FROM Vehicle WHERE user_id=" + to_string(uid);
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r || mysql_num_rows(r) == 0)
    {
        cout << "No vehicles registered for this user.\n";
        if (r)
            mysql_free_result(r);
        return;
    }
    cout << "\nID\tPlate Number\tType\t\tBrand\t\tColor\n";
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(r)))
    {
        cout << row[0] << "\t" << row[1] << "\t\t" << row[2] << "\t\t" << row[3] << "\t\t" << row[4] << "\n";
    }
    mysql_free_result(r);
}

void deleteUserVehicle(MYSQL *conn)
{
    viewAllUsers(conn);
    int uid, vid;
    cout << "\nEnter User ID: ";
    cin >> uid;
    
    string check = "SELECT full_name FROM User WHERE user_id=" + to_string(uid);
    if (mysql_query(conn, check.c_str()))
    {
        cout << "Error!\n";
        return;
    }
    MYSQL_RES *cr = mysql_store_result(conn);
    if (!cr || mysql_num_rows(cr) == 0)
    {
        cout << "User not found!\n";
        if (cr)
            mysql_free_result(cr);
        return;
    }
    mysql_free_result(cr);
    
    string vq = "SELECT vehicle_id, plate_number, vehicle_type FROM Vehicle WHERE user_id=" + to_string(uid);
    if (mysql_query(conn, vq.c_str()))
    {
        cout << "Error!\n";
        return;
    }
    MYSQL_RES *vr = mysql_store_result(conn);
    if (!vr || mysql_num_rows(vr) == 0)
    {
        cout << "No vehicles for this user.\n";
        if (vr)
            mysql_free_result(vr);
        return;
    }
    cout << "\nVehicles for User ID " << uid << ":\n";
    MYSQL_ROW vrow;
    while ((vrow = mysql_fetch_row(vr)))
    {
        cout << "ID: " << vrow[0] << " | Plate: " << vrow[1] << " | Type: " << vrow[2] << "\n";
    }
    mysql_free_result(vr);
    
    cout << "\nEnter Vehicle ID to delete: ";
    cin >> vid;
    
    cout << "Are you sure? (yes/no): ";
    string confirm;
    cin >> confirm;
    if (confirm != "yes")
    {
        cout << "Cancelled.\n";
        return;
    }
    
    string del = "DELETE FROM Vehicle WHERE vehicle_id=" + to_string(vid);
    if (!mysql_query(conn, del.c_str()))
    {
        cout << "\nVehicle deleted successfully!\n";
    }
    else
    {
        cout << "\nDelete failed: " << mysql_error(conn) << "\n";
    }
}

void viewAllBookings(MYSQL *conn)
{
    cout << "\nALL BOOKINGS\n";
    string q = "SELECT b.booking_id, u.full_name, ps.slot_number, b.start_time, b.end_time, b.total_fee, b.booking_status, b.payment_status FROM Booking b JOIN User u ON b.user_id=u.user_id JOIN ParkingSlot ps ON b.slot_id=ps.slot_id ORDER BY b.created_at DESC";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r || mysql_num_rows(r) == 0)
    {
        cout << "No bookings found!\n";
        if (r)
            mysql_free_result(r);
        return;
    }
    MYSQL_ROW row;
    cout << "\nID\tCustomer\tSlot\tStart Time\t\tEnd Time\t\tFee\tStatus\tPayment\n";
    while ((row = mysql_fetch_row(r)))
    {
        cout << row[0] << "\t" << row[1] << "\t" << row[2] << "\t" << row[3] << "\t" << (row[4] ? row[4] : "Active") << "\t" << (row[5] ? row[5] : "Pending") << "\t" << row[6] << "\t" << row[7] << "\n";
    }
    mysql_free_result(r);
}

void viewUnpaidBookings(MYSQL *conn)
{
    cout << "\nUNPAID BOOKINGS\n";
    string q = "SELECT b.booking_id, u.full_name, ps.slot_number, b.start_time, b.total_fee, b.booking_status FROM Booking b JOIN User u ON b.user_id=u.user_id JOIN ParkingSlot ps ON b.slot_id=ps.slot_id WHERE b.payment_status='pending' ORDER BY b.created_at DESC";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r || mysql_num_rows(r) == 0)
    {
        cout << "No unpaid bookings found!\n";
        if (r)
            mysql_free_result(r);
        return;
    }
    MYSQL_ROW row;
    cout << "\nID\tCustomer\tSlot\tStart Time\t\tFee\tStatus\n";
    while ((row = mysql_fetch_row(r)))
    {
        cout << row[0] << "\t" << row[1] << "\t" << row[2] << "\t" << row[3] << "\t" << (row[4] ? row[4] : "Pending") << "\t" << row[5] << "\n";
    }
    mysql_free_result(r);
}

void viewPaidBookings(MYSQL *conn)
{
    cout << "\nPAID BOOKINGS\n";
    string q = "SELECT b.booking_id, u.full_name, ps.slot_number, b.start_time, b.end_time, b.total_fee, b.payment_status FROM Booking b JOIN User u ON b.user_id=u.user_id JOIN ParkingSlot ps ON b.slot_id=ps.slot_id WHERE b.payment_status='paid' ORDER BY b.created_at DESC";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r || mysql_num_rows(r) == 0)
    {
        cout << "No paid bookings found!\n";
        if (r)
            mysql_free_result(r);
        return;
    }
    MYSQL_ROW row;
    cout << "\nID\tCustomer\tSlot\tStart Time\t\tEnd Time\t\tFee\tPayment\n";
    while ((row = mysql_fetch_row(r)))
    {
        cout << row[0] << "\t" << row[1] << "\t" << row[2] << "\t" << row[3] << "\t" << (row[4] ? row[4] : "Active") << "\t" << row[5] << "\t" << row[6] << "\n";
    }
    mysql_free_result(r);
}

void viewCompletedBookings(MYSQL *conn)
{
    cout << "\nCOMPLETED BOOKINGS\n";
    string q = "SELECT b.booking_id, u.full_name, ps.slot_number, b.start_time, b.end_time, b.total_fee, b.payment_status FROM Booking b JOIN User u ON b.user_id=u.user_id JOIN ParkingSlot ps ON b.slot_id=ps.slot_id WHERE b.booking_status='completed' ORDER BY b.created_at DESC";
    if (mysql_query(conn, q.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }
    MYSQL_RES *r = mysql_store_result(conn);
    if (!r || mysql_num_rows(r) == 0)
    {
        cout << "No completed bookings found!\n";
        if (r)
            mysql_free_result(r);
        return;
    }
    MYSQL_ROW row;
    cout << "\nID\tCustomer\tSlot\tStart Time\t\tEnd Time\t\tFee\tPayment\n";
    while ((row = mysql_fetch_row(r)))
    {
        cout << row[0] << "\t" << row[1] << "\t" << row[2] << "\t" << row[3] << "\t" << (row[4] ? row[4] : "Active") << "\t" << row[5] << "\t" << row[6] << "\n";
    }
    mysql_free_result(r);
}

void adminDashboard(MYSQL *conn, string &adminUser, int &adminId, bool &loggedIn)
{
    int ch;
    bool back = false;
    while (!back)
    {
        cout << "\nADMIN DASHBOARD\n";
        cout << "Welcome, Administrator: " << adminUser << "\n";
        cout << "\nSLOT MANAGEMENT\n";
        cout << "1. Add Parking Slot\n";
        cout << "2. Modify Parking Slot\n";
        cout << "3. Remove Parking Slot\n";
        cout << "4. Configure Rates\n";
        cout << "5. View All Slots\n";
        cout << "\nUSER MANAGEMENT\n";
        cout << "6. View All Users\n";
        cout << "7. View User Vehicles\n";
        cout << "8. Delete User Vehicle\n";
        cout << "\nBOOKING MANAGEMENT\n";
        cout << "9. View All Bookings\n";
        cout << "10. View Unpaid Bookings\n";
        cout << "11. View Paid Bookings\n";
        cout << "12. View Completed Bookings\n";
        cout << "\n13. Logout\n";
        cout << "\nEnter choice: ";
        cin >> ch;
        
        if (ch == 1)
            addParkingSlot(conn);
        else if (ch == 2)
            modifyParkingSlot(conn);
        else if (ch == 3)
            removeParkingSlot(conn);
        else if (ch == 4)
            setRateConfig(conn);
        else if (ch == 5)
            viewAllSlots(conn);
        else if (ch == 6)
            viewAllUsers(conn);
        else if (ch == 7)
            viewUserVehicles(conn);
        else if (ch == 8)
            deleteUserVehicle(conn);
        else if (ch == 9)
            viewAllBookings(conn);
        else if (ch == 10)
            viewUnpaidBookings(conn);
        else if (ch == 11)
            viewPaidBookings(conn);
        else if (ch == 12)
            viewCompletedBookings(conn);
        else if (ch == 13)
        {
            logoutUser(adminUser, adminId, loggedIn);
            back = true;
        }
        else
            cout << "\nInvalid choice!\n";
        
        if (!back && ch >= 1 && ch <= 12)
        {
            cout << "\nPress Enter...";
            cin.ignore();
            cin.get();
        }
    }
}