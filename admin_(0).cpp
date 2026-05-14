#include "admin.h"
#include "utils.h"
#include <iostream>
#include <mysql.h>
#include <string>
#include <cstdio>
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

void viewSystemStats(MYSQL *conn)
{
    cout << "\nSYSTEM STATISTICS\n";
    cout << "\nUSER STATISTICS\n";
    string uq = "SELECT COUNT(*),SUM(is_vip) FROM User WHERE role='customer'";
    if (!mysql_query(conn, uq.c_str()))
    {
        MYSQL_RES *r = mysql_store_result(conn);
        if (r)
        {
            MYSQL_ROW row = mysql_fetch_row(r);
            cout << "Total Customers: " << (row[0] ? row[0] : "0") << "\n";
            cout << "VIP Members: " << (row[1] ? row[1] : "0") << "\n";
            mysql_free_result(r);
        }
    }
    cout << "\nREVENUE STATISTICS\n";
    string rq = "SELECT COUNT(*),SUM(amount),AVG(amount) FROM Payment WHERE payment_status='completed'";
    if (!mysql_query(conn, rq.c_str()))
    {
        MYSQL_RES *r = mysql_store_result(conn);
        if (r)
        {
            MYSQL_ROW row = mysql_fetch_row(r);
            cout << "Total Transactions: " << (row[0] ? row[0] : "0") << "\n";
            cout << "Total Revenue: " << (row[1] ? string(row[1]) + " ETB" : "0 ETB") << "\n";
            cout << "Average Transaction: " << (row[2] ? string(row[2]) + " ETB" : "0 ETB") << "\n";
            mysql_free_result(r);
        }
    }
    cout << "\nBOOKING STATISTICS\n";
    string bq = "SELECT COUNT(*),SUM(CASE WHEN booking_status='active' THEN 1 ELSE 0 END),SUM(CASE WHEN booking_status='completed' THEN 1 ELSE 0 END) FROM Booking";
    if (!mysql_query(conn, bq.c_str()))
    {
        MYSQL_RES *r = mysql_store_result(conn);
        if (r)
        {
            MYSQL_ROW row = mysql_fetch_row(r);
            cout << "Total Bookings: " << (row[0] ? row[0] : "0") << "\n";
            cout << "Active: " << (row[1] ? row[1] : "0") << "\n";
            cout << "Completed: " << (row[2] ? row[2] : "0") << "\n";
            mysql_free_result(r);
        }
    }
    cout << "\nSLOT UTILIZATION\n";
    string sq = "SELECT type,COUNT(*),SUM(CASE WHEN is_occupied=TRUE THEN 1 ELSE 0 END) FROM ParkingSlot GROUP BY type";
    if (!mysql_query(conn, sq.c_str()))
    {
        MYSQL_RES *r = mysql_store_result(conn);
        if (r)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(r)))
            {
                int total_slots = atoi(row[1]);
                int occupied_slots = atoi(row[2]);
                int utilization = (occupied_slots * 100) / total_slots;
                cout << row[0] << " Slots: " << total_slots << " total, " << occupied_slots << " occupied (" << utilization << "% utilization)\n";
            }
            mysql_free_result(r);
        }
    }
}

void adminDashboard(MYSQL *conn, string &adminUser, int &adminId, bool &loggedIn)
{
    int ch;
    bool back = false;
    while (!back)
    {
        cout << "\nADMIN DASHBOARD\n";
        cout << "Welcome, Administrator: " << adminUser << "\n\n";
        cout << "SLOT LIFECYCLE MANAGEMENT\n";
        cout << "1. Add Parking Slot\n";
        cout << "2. Modify Parking Slot\n";
        cout << "3. Remove Parking Slot\n\n";
        cout << "DYNAMIC FINANCIAL CONFIGURATION\n";
        cout << "4. Configure Rates\n\n";
        cout << "INVENTORY CATEGORIZATION\n";
        cout << "5. View All Slots (Normal/VIP)\n\n";
        cout << "COMPREHENSIVE MONITORING\n";
        cout << "6. View System Statistics\n";
        cout << "7. Logout\n\n";
        cout << "Enter choice: ";
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
            viewSystemStats(conn);
        else if (ch == 7)
        {
            logoutUser(adminUser, adminId, loggedIn);
            back = true;
        }
        else
            cout << "\nInvalid choice!\n";
        if (!back && ch >= 1 && ch <= 6)
        {
            cout << "\nPress Enter...";
            cin.ignore();
            cin.get();
        }
    }
}