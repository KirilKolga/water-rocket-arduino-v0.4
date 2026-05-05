// Include C++ libraries
#include <string>
#include <cmath>
#include <algorithm>
#include <vector>
#include <numeric>

// Include BLE libraries
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// Include libraries for sensors
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_BMP280.h>

// Include libraries for SD adapter
#include <SD.h>
#include <SPI.h>

// Include libraries for servo motor
#include <ESP32Servo.h>

// Store global variables:

const long SDSize = 16*pow(10, 9); // SD card size in bytes

// BLE stuff

bool BLEDeviceConnected = false; // Variable to check if another device is connected via BLE

bool restartServerAdvertising = false; // Variable to check if server advertising is wanted to restart

std::string BLECommand = "0"; // Variable to store the command sent via BLE (default 0)

// Data logging stuff
unsigned long startTime; // Variable for time passed counting from setup initialization

File logFileADXL345; // Variable for file logging
File logFileBMP280; // Variable for file logging

unsigned long passedLogTime; // Time that has passed during datalog (goes by intervals of logInterval)
const int logInterval = 10; // Interval in which data is logged (unit: ms)

// Launch sequence stuff
unsigned long passedCountdownTime;

// Variables to tell if certain event was detected
bool manualLaunchDetected = false;
bool sensorLaunchDetected = false;

bool parachuteEjected = false;

// Store sensor variables

// ADXL345 stuff
double xAccel; // Variable for raw ADXL345 data
double yAccel; // Variable for raw ADXL345 data
double zAccel; // Variable for raw ADXL345 data

double totalAccel; // Variable to store calculated total acceleration (can be calibrated)

double totalAccelZeroTerm = 0; // Variable to calibrate total acceleration

std::vector<double> lastAccelValues; // Vector to store certain amount of previous acceleration values
int accelIterations = 250; // How many previous values are stored
double lastAccelAverage = 0; // Variable to store the average of previous acceleration values

// BMP280 stuff
double temperature; // Variable for raw BMP280 data
double pressure; // Variable for raw BMP280 data
double altitude; // Variable to store altitude value (can be calibrated)

double altitudeZeroTerm = 0; // Variable to calibrate the altitude

std::vector<double> lastAltitudeValues1; // Vector to store certain amount of previous altitude values
std::vector<double> lastAltitudeValues2; // Vector to store certain amount of altitude values even older than in vector1
int altitudeIterations = 250; // How many previous values are stored (per vector)
double lastAltitudeAverage1 = 0; // Variable to store the average of previous altitude values
double lastAltitudeAverage2 = 0; // Variable to store the average of previous altitude values

// Initialize setup

void setup() 
{
  Serial.begin(115200); // Initialize the Serial monitor
  delay(2000); // Wait 2s for the correct initialization of the Serial monitor

  // Setup BLE communication
  BLESetup();

  // Setup SD card adapter
  SDSetup();

  // Setup ADXL345 sensor
  ADXL345Setup();

  // Setup BMP280 sensor
  BMP280Setup();

  // Setup the servo motor
  servoSetup();

  masterCalibration(); // Perform the calibration of the sensors

  // Function to clean the SD card
  // wipeSD(SD, "/", 10);
}

void loop() 
{
  needServerAdvertising(); // Restart server advertising if needed (e.g. client disconnection)

  if (BLECommand != "0") // Check if BLE command have been sent
  {
    readBLECommand(); // Read BLE command
  }
}










