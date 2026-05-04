int totalStorage; // Variable to store results of SDUsedStorage() function

// Functions to perform a pre-flight check for ADXL345 and BMP280 sensors

// Function to check connection with SD card
bool SDConnectionCheck()
{
  return SD.begin(SD_CS_PIN);
}

// Function to check connection with ADXL345
bool ADXL345ConnectionCheck()
{
  return accel.begin();
}

// Function to check connection with BMP280
bool BMP280ConnectionCheck()
{
  unsigned status;
  status = bmp.begin(0x76);
  return status;
}

void calculateUsedStorage(const String &dir = "/", uint8_t levels = 100)
{
  File root = SD.open(dir);
  if (!root || !root.isDirectory()) return;

  File file = root.openNextFile();

  while (file)
  {
    if (file.isDirectory() && levels)
    {
      String path = String(dir);
      if (path != "/") path += "/";
      path += file.name();

      calculateUsedStorage(path, levels - 1);
    }
    else
    {
      totalStorage += file.size();
    }

    file = root.openNextFile();
  }
}

int getUsedStorage()
{
  totalStorage = 0;
  calculateUsedStorage();

  return totalStorage;
}

// Function to perform a value test for ADXL345 sensor
std::vector<double> ADXL345ValueTest()
{
  accelCalibration(); // Ensure proper calibration

  int iterations = 1000; // How many different values are collected

  std::vector<double> averageValuesXYZ; // Vector to store the results of the test

  // Variables to store the results
  double xAverage;
  double yAverage;
  double zAverage;
  double totalAverage;

  sensors_event_t event; 
  accel.getEvent(&event);

  // Collect the sum of all iterations
  for (int i = 0; i < iterations; i++)
  {
    xAverage += event.acceleration.x;
    yAverage += event.acceleration.y;
    zAverage += event.acceleration.z;

    totalAverage += (sqrt((event.acceleration.x*event.acceleration.x) + (event.acceleration.y*event.acceleration.y) + (event.acceleration.z*event.acceleration.z)) - totalAccelZeroTerm);
  }

  // Divide the sum by iteration number to get the average result
  averageValuesXYZ.push_back(xAverage /= iterations);
  averageValuesXYZ.push_back(yAverage /= iterations);
  averageValuesXYZ.push_back(zAverage /= iterations);
  averageValuesXYZ.push_back(totalAverage /= iterations);

  return averageValuesXYZ;
}

// Function to perform a value test for BMP280 sensor
std::vector<double> BMP280ValueTest()
{
  int iterations = 1000; // How many different values are collected

  std::vector<double> averageValues; // Vector to store the results of the test

  // Variables to store the results
  double temperatureAverage;
  double pressureAverage;
  double altitudeAverage;

  // Collect the sum of all iterations
  for (int i = 0; i < iterations; i++)
  {
    temperatureAverage += bmp.readTemperature();
    pressureAverage += bmp.readPressure();
    altitudeAverage += bmp.readAltitude() - altitudeZeroTerm;
  }

  // Divide the sum by iteration number to get the average result
  averageValues.push_back(temperatureAverage /= iterations);
  averageValues.push_back(pressureAverage /= iterations);
  averageValues.push_back(altitudeAverage /= iterations);

  return averageValues;
}

// Function to print out the results of sensor check
void instrumentCheck()
{
  accelCalibration();
  altitudeCalibration();

  // Connection status
  String SDStatus = SDConnectionCheck() ? "CONNECTED" : "NOT CONNECTED"; // Check if SD is connected
  String ADXL345Status = ADXL345ConnectionCheck() ? "CONNECTED" : "NOT CONNECTED"; // Check if ADXL345 is connected
  String BMP280Status = BMP280ConnectionCheck() ? "CONNECTED" : "NOT CONNECTED"; // Check if BMP280 is connected

  // Sensor values
  auto ADXL345Values = ADXL345ValueTest(); // Perform a value test for ADXL345
  auto BMP280Values = BMP280ValueTest(); // Perform a value test for BMP280

  // SD storage
  String usedStorage = String(getUsedStorage());
  String usedStorageRelative = String((getUsedStorage() / SDSize) * 100);

  // Assemble an output
  String output = "\n= INSTRUMENT CHECK =\n";

  output += "\n[ SD CARD ]\n";
  output += "Status: " + SDStatus + "\n";
  output += "Used: " + usedStorage + " bytes (" + usedStorageRelative + "%)\n";

  output += "\n[ ADXL345 ]\n";
  output += "Status: " + ADXL345Status + "\n";
  output += "Values (m/s^2):\n";
  output += "  Raw X: " + String(ADXL345Values[0]) + "\n";
  output += "  Raw Y: " + String(ADXL345Values[1]) + "\n";
  output += "  Raw Z: " + String(ADXL345Values[2]) + "\n";
  output += "  TOTAL: " + String(ADXL345Values[3]) + " (nulled)\n";

  output += "\n[ BMP280 ]\n";
  output += "Status: " + BMP280Status + "\n";
  output += "Values:\n";
  output += "  Temp: " + String(BMP280Values[0]) + " °C\n";
  output += "  Pressure: " + String(BMP280Values[1]) + " Pa\n";
  output += "  Altitude: " + String(BMP280Values[2]) + " m\n";

  // Send results to the client
  pNotifyCharacteristic->setValue(output.c_str());
  pNotifyCharacteristic->notify();
  
}


