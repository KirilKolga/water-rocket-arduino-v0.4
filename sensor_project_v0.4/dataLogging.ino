
// Functions to perform data logging session for sensor data

// Function to start a data logging session (requires specific file path and file variable)
void startLogging(const String &path, File &logFile)
{
  logFile = SD.open(path, FILE_APPEND);
}

// Function to stop a data logging session (requires specific file variable)
void stopLogging(File &logFile)
{
  if (logFile) logFile.close();
}

// Function to get a new index for the dataLog folder
int getNextDataLogIndex()
{
  // We go through existing dataLog folders to find the largest index already used

  int maxIndex = 0; // Store the largest index

  File root = SD.open("/"); // Open a root directory (in which are all dataLog folders)

  File entry = root.openNextFile(); // Open first file ine the root

  while(entry) // Check all the files until there is nothing left
  {
    if (entry.isDirectory()) // Check if the file is a directory
    {
      String name = entry.name();
      if (name.startsWith("dataLog")) // Check if its name starts with "dataLog"
      {
        int index = name.substring(7).toInt(); // Store its index

        if (index > maxIndex) // Compare the index to the largest
        {
          maxIndex = index; // Switch the largest index
        }
      }
    }
    entry = root.openNextFile(); // Go to the next file
  }

  return maxIndex + 1; // Return the next index
}

// Function to create a new dataLog folder and data files (requires an assembled path for the dataLog folder being created)
void createDataLog(const String &folderPath)
{
  SD.mkdir(folderPath); // Create new dataLog folder

  File xyz = SD.open(folderPath + "/xyz.csv", FILE_WRITE); // Create a CSV file for ADXL345 sensor data
  if (xyz) 
  {
    xyz.println("time,x,y,z,total"); // Write headers for stored values (time,x,y,z)
    xyz.close();
  }

  File bmp = SD.open(folderPath + "/bmp.csv", FILE_WRITE); // Create a CSV file for BMP280 sensor data
  if (bmp)
  {
    bmp.println("time,temperature,pressure,altitude"); // Write headers for stored values (time,altitude)
    bmp.close();
  }
}

// Function to perform data logging
void dataLogRunning()
{
  unsigned long newTime = millis() - startTime; // Calculate the time passed from initializing the session (null the time using startTime variable)

  // Data is logged to the files at intervals (logInterval)

  if (newTime - passedLogTime >= logInterval) // Check if the new time interval has begun (passedLogTime is counted discretely at intervals: 1*logInterval, 2*logInterval, 3*logInterval...)
  {
    passedLogTime += logInterval; // Add passed interval to passedLogTime variable

    
    sensors_event_t event; // create an event for ADXL345
    accel.getEvent(&event);

    xAccel = event.acceleration.x;
    yAccel = event.acceleration.y;
    zAccel = event.acceleration.z;

    totalAccel = sqrt((xAccel*xAccel) + (yAccel*yAccel) + (zAccel*zAccel)) - totalAccelZeroTerm;

    temperature = bmp.readTemperature();
    pressure = bmp.readPressure();
    altitude = bmp.readAltitude() - altitudeZeroTerm;
    

    // Log the ADXL345 sensor data to the data file (format: time,x,y,z)
    logFileADXL345.print(newTime);
    logFileADXL345.print(",");
    logFileADXL345.print(xAccel);
    logFileADXL345.print(",");
    logFileADXL345.print(yAccel);
    logFileADXL345.print(",");
    logFileADXL345.print(zAccel);
    logFileADXL345.print(",");
    logFileADXL345.println(totalAccel);


    // Log the BMP280 sensor data to the data file (format: time,altitude)
    logFileBMP280.print(newTime);
    logFileBMP280.print(",");
    logFileBMP280.print(temperature);
    logFileBMP280.print(",");
    logFileBMP280.print(pressure);
    logFileBMP280.print(",");
    logFileBMP280.println(altitude);

    //Confirm logging data by printing in Serial monitor
    Serial.println("Successfully logged data!");
  }
}

void dataLog(const unsigned long &newTime)
{
  logFileADXL345.print(newTime);
  logFileADXL345.print(",");
  logFileADXL345.print(xAccel);
  logFileADXL345.print(",");
  logFileADXL345.print(yAccel);
  logFileADXL345.print(",");
  logFileADXL345.print(zAccel);
  logFileADXL345.print(",");
  logFileADXL345.println(totalAccel);

  // Log the BMP280 sensor data to the data file (format: time,altitude)
  logFileBMP280.print(newTime);
  logFileBMP280.print(",");
  logFileBMP280.print(temperature);
  logFileBMP280.print(",");
  logFileBMP280.print(pressure);
  logFileBMP280.print(",");
  logFileBMP280.println(altitude);
}