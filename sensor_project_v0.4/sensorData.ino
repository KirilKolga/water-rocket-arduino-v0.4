int TMinusTime = 11000; // Time that passes during countdown (not T+)

int endFlightModeTime = TMinusTime + 20000; // 20s
int manualParachuteEjectionTime = TMinusTime + 12000; // 12s

// Function to eject the parachute using servo motor
void ejectParachute()
{
  parachuteEjected = true;

  String output = "PARACHUTE EJECTED";
  pNotifyCharacteristic->setValue(output.c_str());
  pNotifyCharacteristic->notify();
}

// Function to check if parachute ejection conditions are met
bool sensorParachuteEjection()
{
  // Check if the altitude is above 10m and decreasing
  return (lastAltitudeAverage1 + 1) < lastAltitudeAverage2 && lastAltitudeAverage1 > 10;
}

bool eventLaunch()
{
  return lastAccelAverage > 50;
}

void altitudeCalibration()
{
  altitudeZeroTerm = bmp.readAltitude();
}

void accelCalibration()
{
  sensors_event_t event; 
  accel.getEvent(&event);

  xAccel = event.acceleration.x;
  yAccel = event.acceleration.y;
  zAccel = event.acceleration.z;

  totalAccel = sqrt((xAccel*xAccel) + (yAccel*yAccel) + (zAccel*zAccel)) - totalAccelZeroTerm;

  totalAccelZeroTerm = totalAccel;
}

void masterCalibration()
{
  accelCalibration();
  altitudeCalibration();
}

void setVariables()
{
  sensors_event_t event; 
  accel.getEvent(&event);

  xAccel = event.acceleration.x;
  yAccel = event.acceleration.y;
  zAccel = event.acceleration.z;

  totalAccel = sqrt((xAccel*xAccel) + (yAccel*yAccel) + (zAccel*zAccel)) - totalAccelZeroTerm;

  if (lastAccelValues.size() < accelIterations)
  {
    lastAccelValues.push_back(totalAccel);
  }
  else
  {
    lastAccelValues.erase(lastAccelValues.begin()); // Remove the first element
    lastAccelValues.push_back(totalAccel); // Add new element

    double accelSum = accumulate(lastAccelValues.begin(), lastAccelValues.end(), 0); // Calculate the sum
    lastAccelAverage = accelSum / lastAccelValues.size(); // Calculate the average value

  }

  temperature = bmp.readTemperature();
  pressure = bmp.readPressure();
  altitude = bmp.readAltitude() - altitudeZeroTerm;


  if (lastAltitudeValues1.size() < altitudeIterations) // Check if altitude memory vector1 is full
  {
    // If vector is not full, add altitude values to it
    lastAltitudeValues1.push_back(altitude); 
  }
  else
  {
    if (lastAltitudeValues2.size() < altitudeIterations) // Check if altitude memory vector2 is full
    {
      // If vector is not full, add altitude values to it
      lastAltitudeValues2.push_back(lastAltitudeValues1.back());
    }
    else
    {
      // If vector2 is full erase oldest value and add new value to the end
      lastAltitudeValues2.erase(lastAltitudeValues2.begin());
      lastAltitudeValues2.push_back(lastAltitudeValues1.back());

      // Calculate average value of vector elements
      double altitudeSum2 = accumulate(lastAltitudeValues2.begin(), lastAltitudeValues2.end(), 0);
      lastAltitudeAverage2 = altitudeSum2 / lastAltitudeValues2.size();
    }

    // If vector1 is full erase oldest value and add new value to the end
    lastAltitudeValues1.erase(lastAltitudeValues1.begin());
    lastAltitudeValues1.push_back(altitude);

    // Calculate average value of vector elements
    double altitudeSum1 = accumulate(lastAltitudeValues1.begin(), lastAltitudeValues1.end(), 0);
    lastAltitudeAverage1 = altitudeSum1 / lastAltitudeValues1.size();

  }

  // Serial.println(String(lastAltitudeAverage1) + " - " + String(lastAltitudeAverage2));
  
}

void prePressureTest()
{
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


  String output = "\n= PRE-PRESSURIZATION TEST =\n";

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

  if (SDStatus == "CONNECTED" && ADXL345Status == "CONNECTED" && BMP280Status == "CONNECTED" && 
      ADXL345Values[3] < 1 && ADXL345Values[3] > -1 && BMP280Values[2] < 1 && BMP280Values[2] > -1)
  {
    output += "\nGO TO PRESSURIZATION: YES\n";
  }
  else
  {
    output += "\nGO TO PRESSURIZATION: NO\n";
  }

  // Send results to the client
  pNotifyCharacteristic->setValue(output.c_str());
  pNotifyCharacteristic->notify();
}

void preLaunchTest()
{
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


  String output = "\n= PRE-PRESSURIZATION TEST =\n";

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

  if (SDStatus == "CONNECTED" && ADXL345Status == "CONNECTED" && BMP280Status == "CONNECTED" && 
      ADXL345Values[3] < 1 && ADXL345Values[3] > -1 && BMP280Values[2] < 1 && BMP280Values[2] > -1)
  {
    output += "\nGO TO LAUNCH: YES\n";
  }
  else
  {
    output += "\nGO TO LAUNCH: NO\n";
  }

  // Send results to the client
  pNotifyCharacteristic->setValue(output.c_str());
  pNotifyCharacteristic->notify();
}

void readSensorData()
{
  // Create an event for ADXL345
  sensors_event_t event; 
  accel.getEvent(&event);

  
  // Set raw acceleration data variables
  xAccel = event.acceleration.x;
  yAccel = event.acceleration.y;
  zAccel = event.acceleration.z;

  // Calculate total acceleration (scalar)
  totalAccel = sqrt((xAccel*xAccel) + (yAccel*yAccel) + (zAccel*zAccel)) - totalAccelZeroTerm;

  // Set BMP280 variables
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure();
  altitude = bmp.readAltitude() - altitudeZeroTerm;

  
  // Save certain amount of acceleration data in one vector
  if (lastAccelValues.size() < accelIterations)
  {
    lastAccelValues.push_back(totalAccel);
  }
  else
  {
    lastAccelValues.erase(lastAccelValues.begin()); // Remove the first element
    lastAccelValues.push_back(totalAccel); // Add new element

    double accelSum = accumulate(lastAccelValues.begin(), lastAccelValues.end(), 0); // Calculate the sum
    lastAccelAverage = accelSum / lastAccelValues.size(); // Calculate the average value

  }

  // Save certain amount of altitude data in two vectors
  if (lastAltitudeValues1.size() < altitudeIterations) // Check if altitude memory vector1 is full
  {
    // If vector is not full, add altitude values to it
    lastAltitudeValues1.push_back(altitude); 
  }
  else
  {
    if (lastAltitudeValues2.size() < altitudeIterations) // Check if altitude memory vector2 is full
    {
      // If vector is not full, add altitude values to it
      lastAltitudeValues2.push_back(*lastAltitudeValues1.begin());
    }
    else
    {
      // If vector2 is full erase oldest value and add new value to the end
      lastAltitudeValues2.erase(lastAltitudeValues2.begin());
      lastAltitudeValues2.push_back(*lastAltitudeValues1.begin());

      // Calculate average value of vector elements
      double altitudeSum2 = accumulate(lastAltitudeValues2.begin(), lastAltitudeValues2.end(), 0);
      lastAltitudeAverage2 = altitudeSum2 / lastAltitudeValues2.size();
    }

    // If vector1 is full erase oldest value and add new value to the end
    lastAltitudeValues1.erase(lastAltitudeValues1.begin());
    lastAltitudeValues1.push_back(altitude);

    // Calculate average value of vector elements
    double altitudeSum1 = accumulate(lastAltitudeValues1.begin(), lastAltitudeValues1.end(), 0);
    lastAltitudeAverage1 = altitudeSum1 / lastAltitudeValues1.size();
  }
  
}

// Simulation function

void readFileData(const String &path)
{
  if (!fileExists)
  {
    Serial.println("No such file as " + path);
    while(1);
  }

  File dataFile = SD.open(path);

  if (!dataFile)
  {
    Serial.println("Can't open the file " + path);
    while(1);
  }

  int lineIndex = 0;
  char line[128];

  while (dataFile.available())
  {
    char c = dataFile.read();

    if (c == '\n')
    {
      line[lineIndex] = '\0';
      break;
    }

    line[lineIndex++] = c;
  }
  
  char* token;

  token = strtok(line, ",");

  token = strtok(nullptr, ",");
  temperature = atof(token);

  token = strtok(nullptr, ",");
  pressure = atof(token);

  token = strtok(nullptr, ",");
  altitude = atof(token);

  Serial.println(altitude);
}


// FLIGHT LOOP

char bufferBMP280[4096]; // Store data chunks in RAM using a data buffer and flush them to SD card when full. (Fills in ~1s)
int bufferIndexBMP280 = 0; // The index tells where in the buffer to write the next data log

char bufferADXL345[4096]; // Store data chunks in RAM using a data buffer and flush them to SD card when full. (Fills in ~1s)
int bufferIndexADXL345 = 0; // The index tells where in the buffer to write the next data log

void launchRunning()
{
  unsigned long newTime = millis() - startTime; // Calculate the time passed from session initialization (null the time using startTime variable)

  // Data is logged to the files at intervals (logInterval)
  if (newTime - passedLogTime >= logInterval) // Check if the new time interval has begun (passedLogTime is counted discretely at intervals: 1*logInterval, 2*logInterval, 3*logInterval...)
  {
    passedLogTime += logInterval; // Add passed interval to passedLogTime variable

    readSensorData(); // Read the sensors

    // Write the sensor values into the data buffers
    bufferIndexBMP280 += snprintf(bufferBMP280 + bufferIndexBMP280, sizeof(bufferBMP280) - bufferIndexBMP280, "%lu,%.2f,%.2f,%.2f\n", newTime, temperature, pressure, altitude);

    bufferIndexADXL345 += snprintf(bufferADXL345 + bufferIndexADXL345, sizeof(bufferADXL345) - bufferIndexADXL345, "%lu,%.2f,%.2f,%.2f,%.2f", newTime, xAccel, yAccel, zAccel, totalAccel);

    // Flush the buffer data into the log files when they are nearly full.
    // Also save the datafiles so the contents don't get lost in the case of power loss
    if (bufferIndexBMP280 > sizeof(bufferBMP280) - 200)
    {
      logFileBMP280.write((const uint8_t*)bufferBMP280, bufferIndexBMP280);
      bufferIndexBMP280 = 0; // Return the index to the start, so it writes over old data

      logFileBMP280.flush(); // flush the data file, so it saves the contents
      Serial.println("Flushed");
    }

    if (bufferIndexADXL345 > sizeof(bufferADXL345) - 200)
    {
      logFileADXL345.write((const uint8_t*)bufferADXL345, bufferIndexADXL345);
      bufferIndexADXL345 = 0; // Return the index to the start, so it writes over old data

      logFileADXL345.flush(); // flush the data file, so it saves the contents
    }
  }

  // Perform launch countdown  
  if (newTime - passedCountdownTime >= 1000 && newTime < 12000) // Count while the passed time is under 12s
  {
    passedCountdownTime += 1000; // Add passed interval (1s) to passed time

    // On the last count (when time is 11s) print 'LAUNCH' instead of T-
    if (newTime >= 11000)
    {
      manualLaunchDetected = true; // Detect the launch manually

      String output = "LAUNCH";

      pNotifyCharacteristic->setValue(output.c_str());
      pNotifyCharacteristic->notify();
    }
    // Print T- when the passed time is 1 - 10s
    else
    {
      String output = String(11 - (newTime / 1000)); // Get T- by calculating 11s and the passed time difference

      pNotifyCharacteristic->setValue(output.c_str());
      pNotifyCharacteristic->notify();
    }
  }

  // Eject the parachute manually when certain amount of time has passed
  if (newTime >= manualParachuteEjectionTime && parachuteEjected == false) ejectParachute();

  // Eject parachute if right conditions are met (only eject after T+ 3s, check if altitude is decreasing)
  if (newTime > (TMinusTime + 3000) && sensorParachuteEjection() && parachuteEjected == false) ejectParachute();

  // End the flight mode when 10min has passed
  if (newTime >= endFlightModeTime)
  {
    BLECommand = "OFF";

    String output = "Flight mode ended automatically. Flight data saved successfully.";
    pNotifyCharacteristic->setValue(output.c_str());
    pNotifyCharacteristic->notify();
  }
  

}



