#include <iostream>
#include <libpq-fe.h>
#include <string>
#include <vector>
#include <cstdlib>


void printUserRow(PGresult* res, int row) {
    std::string user_id = PQgetvalue(res, row, 0);
    std::string name = PQgetvalue(res, row, 1);
    std::string email = PQgetvalue(res, row, 2);
    std::string phone = PQgetvalue(res, row, 3);
    std::string created_at = PQgetvalue(res, row, 4);

    std::cout << user_id << "\t"
        << "\"" << name << "\"" << "\t"
        << "\"" << email << "\"" << "\t"
        << "\"" << phone << "\"" << "\t"
        << "\"" << created_at << "\"" << "\n";
}



void checkConn(PGconn* conn) {
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        exit(1);
    }
}

void listUsers(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT user_id, full_name, email, phone_number, created_at FROM Users ORDER BY user_id;");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "List failed: " << PQerrorMessage(conn) << "\n";
        PQclear(res);
        return;
    }
    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        printUserRow(res, i);
    }
    PQclear(res);
}




void createUser(PGconn* conn) {
    std::string name, email, phone, created_at;
    std::cout << "Enter full name: "; std::getline(std::cin >> std::ws, name);
    std::cout << "Enter email: "; std::getline(std::cin, email);
    std::cout << "Enter phone: "; std::getline(std::cin, phone);
    std::cout << "Enter created_at (or empty to use now): ";
    std::getline(std::cin, created_at);

    if (created_at.empty()) {
        const char* params[3] = { name.c_str(), email.c_str(), phone.c_str() };
        PGresult* res = PQexecParams(conn,
            "INSERT INTO Users (full_name, email, phone_number) VALUES ($1, $2, $3) RETURNING user_id;",
            3, nullptr, params, nullptr, nullptr, 0);
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            std::cerr << "Insert failed: " << PQerrorMessage(conn) << "\n";
        }
        else {
            std::cout << "Inserted user_id: " << PQgetvalue(res, 0, 0) << "\n";
        }
        PQclear(res);
    }
    else {
        const char* params[4] = { name.c_str(), email.c_str(), phone.c_str(), created_at.c_str() };
        PGresult* res = PQexecParams(conn,
            "INSERT INTO Users (full_name, email, phone_number, created_at) VALUES ($1, $2, $3, $4) RETURNING user_id;",
            4, nullptr, params, nullptr, nullptr, 0);
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            std::cerr << "Insert with created_at failed: " << PQerrorMessage(conn) << "\n";
        }
        else {
            std::cout << "Inserted user_id: " << PQgetvalue(res, 0, 0) << "\n";
        }
        PQclear(res);
    }
}


void readUser(PGconn* conn) {
    int user_id;
    std::cout << "Enter user_id: ";
    std::cin >> user_id;
    std::string idStr = std::to_string(user_id);
    const char* paramValues[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn,
        "SELECT user_id, full_name, email, phone_number, created_at FROM Users WHERE user_id=$1;",
        1, nullptr, paramValues, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Read failed: " << PQerrorMessage(conn) << "\n";
        PQclear(res);
        return;
    }
    if (PQntuples(res) == 0) {
        std::cout << "(not found)\n";
    }
    else {
        printUserRow(res, 0);
    }
    PQclear(res);
}




void updateUser(PGconn* conn) {
    int user_id;
    std::cout << "Enter user_id to update: ";
    std::cin >> user_id;

    // Clear leftover newline in input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string newEmail, newPhone;
    std::cout << "New email (leave empty to keep current): ";
    std::getline(std::cin, newEmail);

    std::cout << "New phone (leave empty to keep current): ";
    std::getline(std::cin, newPhone);
    if (newEmail.empty() && newPhone.empty()) {
        std::cout << "(nothing to update)\n";
        return;
    }

    // Show before
    std::cout << "Before:\n";
    {
        std::string idStr = std::to_string(user_id);
        const char* pv[1] = { idStr.c_str() };
        PGresult* res = PQexecParams(conn,
            "SELECT user_id, full_name, email, phone_number, created_at FROM Users WHERE user_id=$1;",
            1, nullptr, pv, nullptr, nullptr, 0);
        if (PQntuples(res) == 0) {
            std::cout << "(not found)\n";
            PQclear(res);
            return;
        }
        printUserRow(res, 0);
        PQclear(res);
    }

    // Build dynamic UPDATE query
    std::string sql = "UPDATE Users SET ";
    std::vector<const char*> params;
    int paramIndex = 1;
    bool hasUpdate = false;

    if (!newEmail.empty()) {
        sql += "email = $" + std::to_string(paramIndex++);
        params.push_back(newEmail.c_str());
        hasUpdate = true;
    }
    if (!newPhone.empty()) {
        if (hasUpdate) sql += ", ";
        sql += "phone_number = $" + std::to_string(paramIndex++);
        params.push_back(newPhone.c_str());
        hasUpdate = true;
    }

    if (!hasUpdate) {
        std::cout << "(nothing to update)\n";
    }
    else {
        // Add user_id parameter
        std::string idStr = std::to_string(user_id);
        sql += " WHERE user_id = $" + std::to_string(paramIndex) + " RETURNING user_id;";
        params.push_back(idStr.c_str());

        PGresult* res = PQexecParams(conn, sql.c_str(), (int)params.size(), nullptr, params.data(), nullptr, nullptr, 0);
        if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Update failed: " << PQerrorMessage(conn) << "\n";
        }
        PQclear(res);
    }

    // Show after
    std::cout << "After:\n";
    {
        std::string idStr = std::to_string(user_id);
        const char* pv[1] = { idStr.c_str() };
        PGresult* res = PQexecParams(conn,
            "SELECT user_id, full_name, email, phone_number, created_at FROM Users WHERE user_id=$1;",
            1, nullptr, pv, nullptr, nullptr, 0);
        if (PQntuples(res) == 0) std::cout << "(not found)\n";
        else printUserRow(res, 0);
        PQclear(res);
    }
}


void deleteUser(PGconn* conn) {
    int user_id;
    std::cout << "Enter user_id to delete: ";
    std::cin >> user_id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Show before
    std::string idStr = std::to_string(user_id);
    const char* param[1] = { idStr.c_str() };
    PGresult* res = PQexecParams(conn,
        "SELECT user_id, full_name, email, phone_number, created_at FROM Users WHERE user_id=$1;",
        1, nullptr, param, nullptr, nullptr, 0);

    if (PQntuples(res) == 0) {
        std::cout << "Before: (not found)\n";
        PQclear(res);
        return;
    }
    else {
        std::cout << "Before:\n";
        printUserRow(res, 0);
    }
    PQclear(res);

    // Delete
    res = PQexecParams(conn,
        "DELETE FROM Users WHERE user_id=$1;",
        1, nullptr, param, nullptr, nullptr, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Delete failed: " << PQerrorMessage(conn) << "\n";
    }
    PQclear(res);

    // Show after
    res = PQexecParams(conn,
        "SELECT user_id, full_name, email, phone_number, created_at FROM Users WHERE user_id=$1;",
        1, nullptr, param, nullptr, nullptr, 0);

    if (PQntuples(res) == 0) {
        std::cout << "After: (deleted successfully)\n";
    }
    else {
        std::cout << "After: deletion failed?\n";
        printUserRow(res, 0);
    }
    PQclear(res);
}



void complexSelect(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT user_id, full_name, email FROM Users "
        "WHERE (full_name LIKE 'A%' OR email LIKE '%example.com');");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Complex SELECT failed: " << PQerrorMessage(conn) << std::endl;
    }
    else {
        int rows = PQntuples(res);
        std::cout << "\nComplex query results:\n";
        for (int i = 0; i < rows; i++) {
            std::cout << PQgetvalue(res, i, 0) << " | "
                << PQgetvalue(res, i, 1) << " | "
                << PQgetvalue(res, i, 2) << " | "
                << PQgetvalue(res, i, 3) << "\n";
        }
    }
    PQclear(res);
}

void setupCascadeConstraints(PGconn* conn) {
    const char* sql = R"(
        -- Users ? Drivers
        ALTER TABLE Drivers
        DROP CONSTRAINT IF EXISTS drivers_user_id_fkey;
        ALTER TABLE Drivers
        ADD CONSTRAINT drivers_user_id_fkey
        FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE;

        -- Drivers ? Rides
        ALTER TABLE Rides
        DROP CONSTRAINT IF EXISTS rides_driver_id_fkey;
        ALTER TABLE Rides
        ADD CONSTRAINT rides_driver_id_fkey
        FOREIGN KEY (driver_id) REFERENCES Drivers(driver_id) ON DELETE CASCADE;

        -- Riders ? Rides
        ALTER TABLE Rides
        DROP CONSTRAINT IF EXISTS rides_rider_id_fkey;
        ALTER TABLE Rides
        ADD CONSTRAINT rides_rider_id_fkey
        FOREIGN KEY (rider_id) REFERENCES Riders(rider_id) ON DELETE CASCADE;

        -- Payments ? Rides
        ALTER TABLE Payments
        DROP CONSTRAINT IF EXISTS payments_ride_id_fkey;
        ALTER TABLE Payments
        ADD CONSTRAINT payments_ride_id_fkey
        FOREIGN KEY (ride_id) REFERENCES Rides(ride_id) ON DELETE CASCADE;
    )";

    PGresult* res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Failed to set cascade constraints: " << PQerrorMessage(conn) << "\n";
    }
    else {
        std::cout << "Cascade constraints applied successfully.\n";
    }

    PQclear(res);
}

int main() {
    const char* conninfo = "host=localhost dbname=ride_sharing_sys user=postgres password=551737";
    PGconn* conn = PQconnectdb(conninfo);
    checkConn(conn);

    int choice;
    do {
        std::cout << "\n--- MENU ---\n";
        std::cout << "1. List all users\n";
        std::cout << "2. Create user\n";
        std::cout << "3. Read user by ID\n";
        std::cout << "4. Update user by ID\n";
        std::cout << "5. Delete user by ID\n";
        std::cout << "6. Complex SELECT\n";
        std::cout << "0. Exit\n";
        std::cout << "Choose: ";
        std::cin >> choice;

        switch (choice) {
        case 1: listUsers(conn); break;
        case 2: createUser(conn); break;
        case 3: readUser(conn); break;
        case 4: updateUser(conn); break;
        case 5: deleteUser(conn); break;
        case 6: complexSelect(conn); break;
        case 7: setupCascadeConstraints(conn); break; //For setting up cascade constraints
        }
    } while (choice != 0);

    PQfinish(conn);
    return 0;
}
