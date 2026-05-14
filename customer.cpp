#include <iostream>
#include <mysql.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <ctime>

using namespace std;

// ============ DECLARATIONS ============
void clearScreen();
void logoutUser(string &currentUser, string &currentRole, int &currentUserId, bool &isLoggedIn);

// ============ VIEW AVAILABLE SLOTS ============

void viewAvailableSlots(MYSQL *conn)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "         AVAILABLE PARKING SLOTS" << endl;
    cout << "==========================================" << endl;

    string query = "SELECT ps.slot_id, ps.slot_number, pl.lot_name, ps.type, "
                   "ps.price_per_hour, ps.location_zone, ps.floor_number "
                   "FROM ParkingSlot ps "
                   "JOIN ParkingLot pl ON ps.lot_id = pl.lot_id "
                   "WHERE ps.status = 'available' AND ps.is_occupied = FALSE "
                   "ORDER BY pl.lot_name, ps.slot_number";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (!result)
    {
        cout << "No slots found!" << endl;
        return;
    }

    MYSQL_ROW row;
    int count = 0;

    cout << "\n";
    cout << "Slot # | Lot Name       | Type      | Price/hr | Zone  | Floor" << endl;
    cout << "------+----------------+-----------+----------+-------+-------" << endl;

    while ((row = mysql_fetch_row(result)))
    {
        count++;
        printf("%-6s | %-14s | %-9s | %-8s ETB | %-5s | %-5s\n",
               row[1], row[2], row[3], row[4], row[5], row[6]);
    }

    if (count == 0)
    {
        cout << "\nNo available slots at the moment." << endl;
    }
    else
    {
        cout << "\nTotal available: " << count << " slots" << endl;
    }

    mysql_free_result(result);
}

// ============ BOOK PARKING SPOT ============

void bookParkingSpot(MYSQL *conn, int currentUserId)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "          BOOK PARKING SPOT" << endl;
    cout << "==========================================" << endl;

    viewAvailableSlots(conn);

    string vehicleQuery = "SELECT vehicle_id, plate_number, vehicle_type FROM Vehicle WHERE user_id = " + to_string(currentUserId);

    if (mysql_query(conn, vehicleQuery.c_str()) != 0)
    {
        cout << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES *vehicleResult = mysql_store_result(conn);

    if (!vehicleResult || mysql_num_rows(vehicleResult) == 0)
    {
        cout << "\nYou have no vehicles registered!" << endl;
        cout << "Please register a vehicle first." << endl;

        string plateNumber, vehicleType, brand, color;

        cout << "\n===== REGISTER VEHICLE =====" << endl;
        cout << "Plate Number: ";
        cin >> plateNumber;
        cout << "Vehicle Type (car/motorcycle/truck): ";
        cin >> vehicleType;
        cout << "Brand: ";
        cin >> brand;
        cout << "Color: ";
        cin >> color;

        string insertVehicle = "INSERT INTO Vehicle (plate_number, user_id, vehicle_type, brand, color) VALUES ('";
        insertVehicle += plateNumber + "', " + to_string(currentUserId) + ", '" + vehicleType + "', '" + brand + "', '" + color + "')";

        if (mysql_query(conn, insertVehicle.c_str()) == 0)
        {
            cout << "Vehicle registered successfully!" << endl;
        }
        else
        {
            cout << "Failed to register vehicle: " << mysql_error(conn) << endl;
            if (vehicleResult) mysql_free_result(vehicleResult);
            return;
        }

        if (mysql_query(conn, vehicleQuery.c_str()) != 0)
        {
            cout << "Query failed!" << endl;
            return;
        }
        if (vehicleResult) mysql_free_result(vehicleResult);
        vehicleResult = mysql_store_result(conn);
    }

    cout << "\n===== YOUR VEHICLES =====" << endl;
    MYSQL_ROW vRow;
    while ((vRow = mysql_fetch_row(vehicleResult)))
    {
        cout << "ID: " << vRow[0] << " | " << vRow[1] << " | " << vRow[2] << endl;
    }

    mysql_data_seek(vehicleResult, 0);

    int slotId, vehicleId;

    cout << "\nEnter Slot Number from list above: ";
    cin >> slotId;
    cout << "Enter Vehicle ID: ";
    cin >> vehicleId;

    string priceQuery = "SELECT price_per_hour, lot_id FROM ParkingSlot WHERE slot_id = " + to_string(slotId);
    float pricePerHour = 50;

    if (mysql_query(conn, priceQuery.c_str()) == 0)
    {
        MYSQL_RES *priceResult = mysql_store_result(conn);
        if (priceResult)
        {
            MYSQL_ROW priceRow = mysql_fetch_row(priceResult);
            if (priceRow)
            {
                pricePerHour = atof(priceRow[0]);
            }
            mysql_free_result(priceResult);
        }
    }

    string bookQuery = "INSERT INTO Booking (user_id, slot_id, vehicle_id, start_time, booking_status, payment_status) VALUES (";
    bookQuery += to_string(currentUserId) + ", " + to_string(slotId) + ", " + to_string(vehicleId) + ", NOW(), 'active', 'pending')";

    if (mysql_query(conn, bookQuery.c_str()) == 0)
    {
        cout << "\nParking spot booked successfully!" << endl;
        cout << "Price: " << pricePerHour << " ETB per hour" << endl;

        string updateSlot = "UPDATE ParkingSlot SET is_occupied = TRUE, status = 'occupied' WHERE slot_id = " + to_string(slotId);
        mysql_query(conn, updateSlot.c_str());
    }
    else
    {
        cout << "\nBooking failed: " << mysql_error(conn) << endl;
    }

    mysql_free_result(vehicleResult);
}

// ============ VIEW MY BOOKINGS ============

void viewMyBookings(MYSQL *conn, int currentUserId)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "           MY BOOKINGS" << endl;
    cout << "==========================================" << endl;

    string query = "SELECT b.booking_id, ps.slot_number, pl.lot_name, b.start_time, "
                   "b.end_time, b.total_fee, b.booking_status, b.payment_status, b.slot_id "
                   "FROM Booking b "
                   "JOIN ParkingSlot ps ON b.slot_id = ps.slot_id "
                   "JOIN ParkingLot pl ON ps.lot_id = pl.lot_id "
                   "WHERE b.user_id = " + to_string(currentUserId) +
                   " ORDER BY b.created_at DESC";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (!result)
    {
        cout << "No bookings found!" << endl;
        return;
    }

    MYSQL_ROW row;
    int count = 0;

    cout << "\n";
    while ((row = mysql_fetch_row(result)))
    {
        count++;
        cout << "Booking #" << row[0] << " | Slot: " << row[1] << " (" << row[2] << ")" << endl;
        cout << "  Start: " << row[3] << endl;
        cout << "  End: " << (row[4] ? row[4] : "Still parked") << endl;
        cout << "  Fee: " << (row[5] ? string(row[5]) + " ETB" : "Pending") << endl;
        cout << "  Booking: " << row[6] << " | Payment: " << row[7] << endl;
        cout << "  -----------------------------" << endl;
    }

    if (count == 0)
    {
        cout << "\nYou have no bookings yet." << endl;
    }

    mysql_free_result(result);
}

// ============ CHECK OUT ============

void checkOutParking(MYSQL *conn, int currentUserId)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "           CHECK OUT" << endl;
    cout << "==========================================" << endl;

    string query = "SELECT b.booking_id, ps.slot_number, pl.lot_name, b.start_time, ps.price_per_hour, b.slot_id "
                   "FROM Booking b "
                   "JOIN ParkingSlot ps ON b.slot_id = ps.slot_id "
                   "JOIN ParkingLot pl ON ps.lot_id = pl.lot_id "
                   "WHERE b.user_id = " + to_string(currentUserId) +
                   " AND b.booking_status = 'active'";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (!result || mysql_num_rows(result) == 0)
    {
        cout << "\nYou have no active bookings to check out." << endl;
        if (result) mysql_free_result(result);
        return;
    }

    cout << "\nYour Active Bookings:" << endl;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        cout << "Booking #" << row[0] << " | Slot: " << row[1] << " | Lot: " << row[2]
             << " | Started: " << row[3] << " | Rate: " << row[4] << " ETB/hr" << endl;
    }

    int bookingId;
    cout << "\nEnter Booking ID to check out: ";
    cin >> bookingId;

    string detailQuery = "SELECT start_time, price_per_hour, b.slot_id "
                         "FROM Booking b "
                         "JOIN ParkingSlot ps ON b.slot_id = ps.slot_id "
                         "WHERE b.booking_id = " + to_string(bookingId) +
                         " AND b.user_id = " + to_string(currentUserId);

    if (mysql_query(conn, detailQuery.c_str()) != 0)
    {
        cout << "Error: " << mysql_error(conn) << endl;
        mysql_free_result(result);
        return;
    }

    MYSQL_RES *detailResult = mysql_store_result(conn);
    if (!detailResult || mysql_num_rows(detailResult) == 0)
    {
        cout << "Booking not found!" << endl;
        if (detailResult) mysql_free_result(detailResult);
        mysql_free_result(result);
        return;
    }

    MYSQL_ROW detailRow = mysql_fetch_row(detailResult);
    float pricePerHour = atof(detailRow[1]);
    int slotId = atoi(detailRow[2]);

    float hours;
    cout << "Enter hours parked: ";
    cin >> hours;

    float totalFee = hours * pricePerHour;

    cout << "\n===== PAYMENT =====" << endl;
    cout << "Hours parked: " << hours << endl;
    cout << "Rate: " << pricePerHour << " ETB/hour" << endl;
    cout << "Total Fee: " << totalFee << " ETB" << endl;

    string paymentMethod;
    cout << "\nPayment Method (cash/credit_card/telebirr): ";
    cin >> paymentMethod;

    string updateBooking = "UPDATE Booking SET end_time = NOW(), duration = " + to_string(hours) +
                           ", total_fee = " + to_string(totalFee) +
                           ", payment_status = 'paid', booking_status = 'completed' " +
                           "WHERE booking_id = " + to_string(bookingId);

    if (mysql_query(conn, updateBooking.c_str()) == 0)
    {
        string paymentQuery = "INSERT INTO Payment (booking_id, amount, payment_method, payment_status) VALUES (";
        paymentQuery += to_string(bookingId) + ", " + to_string(totalFee) + ", '" + paymentMethod + "', 'completed')";
        mysql_query(conn, paymentQuery.c_str());

        string freeSlot = "UPDATE ParkingSlot SET is_occupied = FALSE, status = 'available' WHERE slot_id = " + to_string(slotId);
        mysql_query(conn, freeSlot.c_str());

        cout << "\nCheck out successful!" << endl;
        cout << "Thank you for parking with us!" << endl;
    }
    else
    {
        cout << "\nCheck out failed: " << mysql_error(conn) << endl;
    }

    mysql_free_result(detailResult);
    mysql_free_result(result);
}

// ============ VIEW MY PROFILE ============

void viewMyProfile(MYSQL *conn, int currentUserId)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "            MY PROFILE" << endl;
    cout << "==========================================" << endl;

    string query = "SELECT full_name, email, phone_number, role, is_vip, created_at, status FROM User WHERE user_id = " + to_string(currentUserId);

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (!result)
    {
        cout << "Profile not found!" << endl;
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row)
    {
        cout << "\n  Name: " << row[0] << endl;
        cout << "  Email: " << row[1] << endl;
        cout << "  Phone: " << row[2] << endl;
        cout << "  Role: " << row[3] << endl;
        cout << "  VIP: " << (atoi(row[4]) == 1 ? "Yes" : "No") << endl;
        cout << "  Member Since: " << row[5] << endl;
        cout << "  Status: " << row[6] << endl;
    }

    mysql_free_result(result);

    cout << "\n===== YOUR VEHICLES =====" << endl;
    string vehicleQuery = "SELECT vehicle_id, plate_number, vehicle_type, brand, color FROM Vehicle WHERE user_id = " + to_string(currentUserId);

    if (mysql_query(conn, vehicleQuery.c_str()) == 0)
    {
        MYSQL_RES *vResult = mysql_store_result(conn);
        if (vResult && mysql_num_rows(vResult) > 0)
        {
            MYSQL_ROW vRow;
            while ((vRow = mysql_fetch_row(vResult)))
            {
                cout << "  ID:" << vRow[0] << " | " << vRow[1] << " | " << vRow[2] << " | " << vRow[3] << " | " << vRow[4] << endl;
            }
        }
        else
        {
            cout << "  No vehicles registered." << endl;
        }
        if (vResult) mysql_free_result(vResult);
    }
}

// ============ MAKE RESERVATION ============

void makeReservation(MYSQL *conn, int currentUserId)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "        MAKE RESERVATION" << endl;
    cout << "==========================================" << endl;

    viewAvailableSlots(conn);

    string vehicleQuery = "SELECT vehicle_id, plate_number, vehicle_type FROM Vehicle WHERE user_id = " + to_string(currentUserId);

    if (mysql_query(conn, vehicleQuery.c_str()) != 0)
    {
        cout << "Error loading vehicles!" << endl;
        return;
    }

    MYSQL_RES *vehicleResult = mysql_store_result(conn);

    if (!vehicleResult || mysql_num_rows(vehicleResult) == 0)
    {
        cout << "\nYou need to register a vehicle first (use Book Parking Spot option)!" << endl;
        if (vehicleResult) mysql_free_result(vehicleResult);
        return;
    }

    cout << "\n===== YOUR VEHICLES =====" << endl;
    MYSQL_ROW vRow;
    while ((vRow = mysql_fetch_row(vehicleResult)))
    {
        cout << "ID: " << vRow[0] << " | " << vRow[1] << " | " << vRow[2] << endl;
    }

    int slotId, vehicleId;
    string expiryDate, expiryTime;

    cout << "\nEnter Slot ID: ";
    cin >> slotId;
    cout << "Enter Vehicle ID: ";
    cin >> vehicleId;
    cout << "Reservation Expiry Date (YYYY-MM-DD): ";
    cin >> expiryDate;
    cout << "Reservation Expiry Time (HH:MM:SS): ";
    cin >> expiryTime;

    string expiryDatetime = expiryDate + " " + expiryTime;

    string reserveQuery = "INSERT INTO Reservation (user_id, slot_id, vehicle_id, expiry_time, status) VALUES (";
    reserveQuery += to_string(currentUserId) + ", " + to_string(slotId) + ", " + to_string(vehicleId) + ", '" + expiryDatetime + "', 'active')";

    if (mysql_query(conn, reserveQuery.c_str()) == 0)
    {
        cout << "\nReservation made successfully!" << endl;
        cout << "Your slot is reserved until: " << expiryDatetime << endl;

        string updateSlot = "UPDATE ParkingSlot SET status = 'reserved' WHERE slot_id = " + to_string(slotId);
        mysql_query(conn, updateSlot.c_str());
    }
    else
    {
        cout << "\nReservation failed: " << mysql_error(conn) << endl;
    }

    mysql_free_result(vehicleResult);
}

// ============ VIEW MY RESERVATIONS ============

void viewMyReservations(MYSQL *conn, int currentUserId)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "         MY RESERVATIONS" << endl;
    cout << "==========================================" << endl;

    string query = "SELECT r.reservation_id, ps.slot_number, pl.lot_name, v.plate_number, "
                   "r.reservation_time, r.expiry_time, r.status "
                   "FROM Reservation r "
                   "JOIN ParkingSlot ps ON r.slot_id = ps.slot_id "
                   "JOIN ParkingLot pl ON ps.lot_id = pl.lot_id "
                   "JOIN Vehicle v ON r.vehicle_id = v.vehicle_id "
                   "WHERE r.user_id = " + to_string(currentUserId) +
                   " ORDER BY r.reservation_time DESC";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (!result || mysql_num_rows(result) == 0)
    {
        cout << "\nNo reservations found." << endl;
        if (result) mysql_free_result(result);
        return;
    }

    MYSQL_ROW row;
    cout << "\n";
    while ((row = mysql_fetch_row(result)))
    {
        cout << "Reservation #" << row[0] << endl;
        cout << "  Slot: " << row[1] << " (" << row[2] << ")" << endl;
        cout << "  Vehicle: " << row[3] << endl;
        cout << "  Made: " << row[4] << endl;
        cout << "  Expires: " << row[5] << endl;
        cout << "  Status: " << row[6] << endl;
        cout << "  -----------------------------" << endl;
    }

    mysql_free_result(result);
}

// ============ UPDATE PASSWORD ============

void updatePassword(MYSQL *conn, int currentUserId)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "          UPDATE PASSWORD" << endl;
    cout << "==========================================" << endl;

    string oldPassword, newPassword, confirmPassword;

    cout << "\nCurrent Password: ";
    cin >> oldPassword;
    cout << "New Password: ";
    cin >> newPassword;
    cout << "Confirm New Password: ";
    cin >> confirmPassword;

    if (newPassword != confirmPassword)
    {
        cout << "\nPasswords don't match!" << endl;
        return;
    }

    string checkQuery = "SELECT user_id FROM User WHERE user_id = " + to_string(currentUserId) + " AND password_hash = '" + oldPassword + "'";

    if (mysql_query(conn, checkQuery.c_str()) != 0)
    {
        cout << "Error!" << endl;
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (!result || mysql_num_rows(result) == 0)
    {
        cout << "\nCurrent password is incorrect!" << endl;
        if (result) mysql_free_result(result);
        return;
    }
    mysql_free_result(result);

    string updateQuery = "UPDATE User SET password_hash = '" + newPassword + "' WHERE user_id = " + to_string(currentUserId);

    if (mysql_query(conn, updateQuery.c_str()) == 0)
    {
        cout << "\nPassword updated successfully!" << endl;
    }
    else
    {
        cout << "\nUpdate failed!" << endl;
    }
}

// ============ VIEW PAYMENT HISTORY ============

void viewPaymentHistory(MYSQL *conn, int currentUserId)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "         PAYMENT HISTORY" << endl;
    cout << "==========================================" << endl;

    string query = "SELECT p.payment_id, p.amount, p.payment_method, p.payment_date, p.payment_status, "
                   "b.booking_id, ps.slot_number "
                   "FROM Payment p "
                   "JOIN Booking b ON p.booking_id = b.booking_id "
                   "JOIN ParkingSlot ps ON b.slot_id = ps.slot_id "
                   "WHERE b.user_id = " + to_string(currentUserId) +
                   " ORDER BY p.payment_date DESC";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (!result || mysql_num_rows(result) == 0)
    {
        cout << "\nNo payment history." << endl;
        if (result) mysql_free_result(result);
        return;
    }

    MYSQL_ROW row;
    float totalSpent = 0;

    cout << "\n";
    while ((row = mysql_fetch_row(result)))
    {
        float amount = atof(row[1]);
        totalSpent += amount;

        cout << "Payment #" << row[0] << " | Booking #" << row[5] << endl;
        cout << "  Amount: " << row[1] << " ETB" << endl;
        cout << "  Method: " << row[2] << endl;
        cout << "  Date: " << row[3] << endl;
        cout << "  Status: " << row[4] << " | Slot: " << row[6] << endl;
        cout << "  -----------------------------" << endl;
    }

    cout << "\nTotal Spent: " << totalSpent << " ETB" << endl;

    mysql_free_result(result);
}

// ============ SEARCH SLOTS ============

void searchSlots(MYSQL *conn)
{
    clearScreen();
    cout << "\n==========================================" << endl;
    cout << "          SEARCH PARKING SLOTS" << endl;
    cout << "==========================================" << endl;

    cout << "\nFilter by:" << endl;
    cout << "  1. By Type (standard/vip/handicap/etc)" << endl;
    cout << "  2. By Price Range" << endl;
    cout << "  3. By Location/Zone" << endl;
    cout << "\n  Enter choice: ";

    int choice;
    cin >> choice;

    string query = "SELECT ps.slot_id, ps.slot_number, pl.lot_name, ps.type, "
                   "ps.price_per_hour, ps.location_zone, ps.floor_number "
                   "FROM ParkingSlot ps "
                   "JOIN ParkingLot pl ON ps.lot_id = pl.lot_id "
                   "WHERE ps.status = 'available' AND ps.is_occupied = FALSE ";

    if (choice == 1)
    {
        string type;
        cout << "Enter type: ";
        cin >> type;
        query += "AND ps.type = '" + type + "' ";
    }
    else if (choice == 2)
    {
        float minPrice, maxPrice;
        cout << "Min price (ETB): ";
        cin >> minPrice;
        cout << "Max price (ETB): ";
        cin >> maxPrice;
        query += "AND ps.price_per_hour BETWEEN " + to_string(minPrice) + " AND " + to_string(maxPrice) + " ";
    }
    else if (choice == 3)
    {
        string zone;
        cout << "Enter zone: ";
        cin >> zone;
        query += "AND ps.location_zone = '" + zone + "' ";
    }

    query += "ORDER BY pl.lot_name, ps.slot_number";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Query failed: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (!result || mysql_num_rows(result) == 0)
    {
        cout << "\nNo slots found matching your criteria." << endl;
        if (result) mysql_free_result(result);
        return;
    }

    MYSQL_ROW row;
    int count = 0;

    cout << "\n";
    cout << "Slot # | Lot Name       | Type      | Price/hr | Zone  | Floor" << endl;
    cout << "------+----------------+-----------+----------+-------+-------" << endl;

    while ((row = mysql_fetch_row(result)))
    {
        count++;
        printf("%-6s | %-14s | %-9s | %-8s ETB | %-5s | %-5s\n",
               row[1], row[2], row[3], row[4], row[5], row[6]);
    }

    cout << "\nFound: " << count << " matching slots" << endl;

    mysql_free_result(result);
}

// ============ CUSTOMER DASHBOARD ============

void customerDashboard(MYSQL *conn, string &currentUser, string &currentRole,
                       int &currentUserId, bool &isLoggedIn)
{
    int choice;
    bool goBack = false;

    while (!goBack)
    {
        clearScreen();

        string vipQuery = "SELECT is_vip FROM User WHERE user_id = " + to_string(currentUserId);
        bool isVIP = false;
        if (mysql_query(conn, vipQuery.c_str()) == 0)
        {
            MYSQL_RES *vipResult = mysql_store_result(conn);
            if (vipResult)
            {
                MYSQL_ROW vipRow = mysql_fetch_row(vipResult);
                if (vipRow && atoi(vipRow[0]) == 1)
                {
                    isVIP = true;
                }
                mysql_free_result(vipResult);
            }
        }

        cout << "\n";
        cout << "==========================================" << endl;
        cout << "         CUSTOMER DASHBOARD" << endl;
        cout << "==========================================" << endl;
        cout << "\n";
        cout << "  Welcome, " << currentUser;
        if (isVIP) cout << " VIP";
        cout << "!" << endl;
        cout << "\n";
        cout << "  --- PARKING ---" << endl;
        cout << "  1. View Available Slots" << endl;
        cout << "  2. Search/Filter Slots" << endl;
        cout << "  3. Book Parking Spot" << endl;
        cout << "  4. Check Out (Exit Parking)" << endl;
        cout << "\n";
        cout << "  --- RESERVATIONS ---" << endl;
        cout << "  5. Make Reservation" << endl;
        cout << "  6. View My Reservations" << endl;
        cout << "\n";
        cout << "  --- HISTORY ---" << endl;
        cout << "  7. View My Bookings" << endl;
        cout << "  8. Payment History" << endl;
        cout << "\n";
        cout << "  --- ACCOUNT ---" << endl;
        cout << "  9. My Profile" << endl;
        cout << "  10. Update Password" << endl;
        cout << "  11. Logout" << endl;
        cout << "\n";
        cout << "  Enter choice: ";

        cin >> choice;

        if (choice == 1) viewAvailableSlots(conn);
        else if (choice == 2) searchSlots(conn);
        else if (choice == 3) bookParkingSpot(conn, currentUserId);
        else if (choice == 4) checkOutParking(conn, currentUserId);
        else if (choice == 5) makeReservation(conn, currentUserId);
        else if (choice == 6) viewMyReservations(conn, currentUserId);
        else if (choice == 7) viewMyBookings(conn, currentUserId);
        else if (choice == 8) viewPaymentHistory(conn, currentUserId);
        else if (choice == 9) viewMyProfile(conn, currentUserId);
        else if (choice == 10) updatePassword(conn, currentUserId);
        else if (choice == 11)
        {
            logoutUser(currentUser, currentRole, currentUserId, isLoggedIn);
            goBack = true;
        }
        else cout << "\nInvalid choice!" << endl;

        if (!goBack)
        {
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        }
    }
}