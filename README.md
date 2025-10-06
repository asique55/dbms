--Project Overview

Brief description of the console app, features, and database connection.

--Prerequisites

PostgreSQL 17 installed

libpq library installed

Visual Studio 2022 (or any C++ compiler)

Windows OS (or Linux, adapt paths)

Database name: ride_sharing_system

--Setup Steps

Database

Run ride_sharing_schema.sql in pgAdmin or psql to create tables and seed data:

psql -U postgres -d ride_sharing_system -f ride_sharing_schema.sql


--Build C++ App

Open project in Visual Studio

Add libpq.lib to linker inputs

Ensure libpq.dll and required OpenSSL DLLs are in the executable folder or PATH

Compile the project

Run App

Execute the compiled binary

Console menu appears

Follow menu prompts for CRUD operations

--Notes

SQL injection is prevented using parameterized queries

Cascade deletes are enabled for Users → Drivers → Rides → Payments

created_at timestamps are automatically generated
