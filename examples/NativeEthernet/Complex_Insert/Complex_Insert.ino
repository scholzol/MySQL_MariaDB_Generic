/*********************************************************************************************************************************
  Complex_Insert.ino
      
  Library for communicating with a MySQL or MariaDB Server
  
  Based on and modified from Dr. Charles A. Bell's MySQL_Connector_Arduino Library https://github.com/ChuckBell/MySQL_Connector_Arduino
  to support nRF52, SAMD21/SAMD51, SAM DUE, STM32F/L/H/G/WB/MP1, ESP8266, ESP32, etc. boards using W5x00, ENC28J60, LAM8742A Ethernet,
  WiFiNINA, ESP-AT, built-in ESP8266/ESP32 WiFi.

  The library provides simple and easy Client interface to MySQL or MariaDB Server.
  
  Built by Khoi Hoang https://github.com/khoih-prog/MySQL_MariaDB_Generic
  Licensed under MIT license
 **********************************************************************************************************************************/
/*
  MySQL Connector/Arduino Example : complex insert

  This example demonstrates how to issue an INSERT query to store data in a
  table using data from variables in our sketch. In this case, we supply the
  values dynamically.

  This sketch simulates storing data from a sensor in a table.

  For this, we will create a special database and table for testing.
  The following are the SQL commands you will need to run in order to setup
  your database for running this sketch.

  CREATE DATABASE test_arduino;
  CREATE TABLE test_arduino.hello_sensor (
    num integer primary key auto_increment,
    message char(40),
    sensor_num integer,
    value float,
    recorded timestamp
  );

  Here we have a table that contains an auto_increment primary key, a text
  field, a field to identify the sensor, the value read, and timestamp of
  the recorded data.

  Note: Since this sketch uses test data, we place the INSERT in the setup()
        method so that it runs only once. Typically, you would have the
        INSERT in the loop() method after your code to read from the sensor.

  For more information and documentation, visit the wiki:
  https://github.com/ChuckBell/MySQL_Connector_Arduino/wiki.

  INSTRUCTIONS FOR USE

  1) Create the database and table as shown above.
  2) Change the address of the server to the IP address of the MySQL server
  3) Change the user and password to a valid MySQL user and password
  4) Connect a USB cable to your Arduino
  5) Select the correct board and port
  6) Compile and upload the sketch to your Arduino
  7) Once uploaded, open Serial Monitor (use 115200 speed) and observe
  8) After the sketch has run for some time, open a mysql client and issue
     the command: "SELECT * FROM test_arduino.hello_sensor" to see the data
     recorded. Note the field values and how the database handles both the
     auto_increment and timestamp fields for us. You can clear the data with
     "DELETE FROM test_arduino.hello_sensor".

  Note: The MAC address can be anything so long as it is unique on your network.

  Created by: Dr. Charles A. Bell
*/

#include "defines.h"

#include <MySQL_Generic.h>

// Select the static Local IP address according to your local network
IPAddress ip(192, 168, 2, 222);

IPAddress server_addr(192, 168, 2, 112);
uint16_t server_port = 5698;    //3306;

char user[]             = "invited-guest";              // MySQL user login username
char password[]         = "the-invited-guest";          // MySQL user login password

char default_database[] = "test_arduino";           //"test_arduino";
char default_table[]    = "hello_sensor";         //"test_arduino";

// Sample query
char INSERT_DATA[] = "INSERT INTO %s.%s (message, sensor_num, value) VALUES ('%s',%d,%s)";
                 
char query[128];
char temperature[10];

MySQL_Connection conn((Client *)&client);

#if !( ESP32 || ESP8266 || defined(CORE_TEENSY) || defined(STM32F1) || defined(STM32F2) || defined(STM32F3) || defined(STM32F4) || defined(STM32F7) || ( defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED) ) ) 

char *dtostrf(double val, signed char width, unsigned char prec, char *sout)
{
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}
#endif

void setup()
{
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect

  MYSQL_DISPLAY3("\nStarting Complex_Insert on", BOARD_NAME, ", with", SHIELD_TYPE);
  MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

  MYSQL_LOGERROR(F("========================================="));
  MYSQL_LOGERROR(F("Default SPI pinout:"));
  MYSQL_LOGERROR1(F("MOSI:"), MOSI);
  MYSQL_LOGERROR1(F("MISO:"), MISO);
  MYSQL_LOGERROR1(F("SCK:"),  SCK);
  MYSQL_LOGERROR1(F("SS:"),   SS);
  MYSQL_LOGERROR(F("========================================="));

  // use default SS = 10
  #ifndef USE_THIS_SS_PIN
    #define USE_THIS_SS_PIN   10    // For other boards
  #endif

  MYSQL_LOGERROR3(F("Board :"), BOARD_NAME, F(", setCsPin:"), USE_THIS_SS_PIN);

  // For other boards, to change if necessary
 
  Ethernet.init (USE_THIS_SS_PIN);

  // start the ethernet connection and the server:
  // Use DHCP dynamic IP and random mac
  uint16_t index = millis() % NUMBER_OF_MAC;
  // Use Static IP
  //Ethernet.begin(mac[index], ip);
  Ethernet.begin(mac[index]);

  // Just info to know how to connect correctly
  MYSQL_LOGERROR(F("========================================="));
  MYSQL_LOGERROR(F("Currently Used SPI pinout:"));
  MYSQL_LOGERROR1(F("MOSI:"), MOSI);
  MYSQL_LOGERROR1(F("MISO:"), MISO);
  MYSQL_LOGERROR1(F("SCK:"),  SCK);
  MYSQL_LOGERROR1(F("SS:"),   SS);
  MYSQL_LOGERROR(F("========================================="));

  MYSQL_DISPLAY1("Using mac index =", index);
  MYSQL_DISPLAY1("Connected! IP address:", Ethernet.localIP());

  MYSQL_DISPLAY3("Connecting to SQL Server @", server_addr, ", Port =", server_port);
  MYSQL_DISPLAY3("User =", user, ", PW =", password);
  MYSQL_DISPLAY3("DB =", default_database, ", Table =", default_table);
}

void runInsert()
{
  // Initiate the query class instance
  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected())
  {
    // Save
    dtostrf(50.125, 1, 1, temperature);
    sprintf(query, INSERT_DATA, default_database, default_table, "test sensor", 24, temperature);
    
    // Execute the query
    MYSQL_DISPLAY(query);

    // KH, check if valid before fetching
    if ( !query_mem.execute(query) )
    {
      MYSQL_DISPLAY("Complex Insert error");
    }
    else
    {
      MYSQL_DISPLAY("Complex Data Inserted.");
    }
  }
  else
  {
    MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
  }
}

void loop()
{
  MYSQL_DISPLAY("Connecting...");
  
  //if (conn.connect(server_addr, server_port, user, password))
  if (conn.connectNonBlocking(server_addr, server_port, user, password) != RESULT_FAIL)
  {
    delay(500);
    runInsert();
    conn.close();                     // close the connection
  } 
  else 
  {
    MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
  }

  MYSQL_DISPLAY("\nSleeping...");
  MYSQL_DISPLAY("================================================");
 
  delay(60000);
}
