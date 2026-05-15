#include <iostream>
#include <mysql.h>
#include <string>
#include "customer.h"
#include "admin.h"
#include "utils.h"

using namespace std;

string currentUser = "";
int currentUserId = 0;
string currentRole = "";
bool isLoggedIn = false;

void showMainMenu()
{
    cout << "\nSMART PARKING MANAGEMENT SYSTEM\n";
    cout << "1. Login\n";
    cout << "2. Register\n";
    cout << "3. Exit\n";
    cout << "Enter choice: ";
}

int main()
{
    MYSQL *conn = NULL;
    if (!connectDatabase(conn))
    {
        cout << "\nCannot connect to database\n";
        return 1;
    }

    int choice;
    bool running = true;

    while (running)
    {
        if (!isLoggedIn)
        {
            showMainMenu();
            cin >> choice;
            string email, password, name, phone;

            if (choice == 1)
            {
                cout << "\nLOGIN\n";
                cout << "Email: ";
                cin >> email;
                cout << "Password: ";
                cin >> password;
                if (loginUser(conn, email, password, currentUser, currentUserId, currentRole))
                {
                    isLoggedIn = true;
                }
            }
            else if (choice == 2)
            {
                cout << "\nREGISTER\n";

                bool firstUser = isFirstUser(conn);
                if (firstUser)
                {
                    cout << "You are the FIRST user to register!\n";
                    cout << "You will be registered as ADMINISTRATOR.\n\n";
                }

                cout << "Full Name: ";
                cin.ignore();
                getline(cin, name);
                cout << "Email: ";
                getline(cin, email);
                cout << "Phone: ";
                getline(cin, phone);
                cout << "Password: ";
                getline(cin, password);
                int vipChoice;
                if (!firstUser)
                {

                    cout << "\nVIP Membership?\n";
                    cout << "1. Yes (Premium)\n";
                    cout << "2. No (Standard)\n";
                    cout << "Choice: ";
                    cin >> vipChoice;
                }
                registerUser(conn, name, email, phone, password, vipChoice);
            }
            else if (choice == 3)
            {
                running = false;
                cout << "\nThank you for using Smart Parking System!\n";
            }
            else
            {
                cout << "\nInvalid choice!\n";
            }
        }
        else
        {
            if (currentRole == "admin")
            {
                adminDashboard(conn, currentUser, currentUserId, isLoggedIn);
            }
            else
            {
                customerDashboard(conn, currentUser, currentUserId, isLoggedIn);
            }
        }
    }

    disconnectDatabase(conn);
    return 0;
}