
// Function to compare received BLE command to existing commands

void readBLECommand()
{
  // If command is 0 (default): do nothing
  if (BLECommand == "0") {}

  // If command is OFF: set command to default and stop possible data logging sessions
  else if (BLECommand == "OFF")
  {
    BLECommand = "0";

    // Set all flight boolean variables to false
    manualLaunchDetected = false;
    sensorLaunchDetected = false;
    parachuteEjected = false;

    // Save the log files by closing them
    stopLogging(logFileADXL345);
    stopLogging(logFileBMP280);
  }

  // If command is SDcontents: print out all the files located on the SD card
  else if (BLECommand == "SDContents")
  {
    BLECommand = "0"; // Set BLE command to default to prevent running in loop
    if (SDConnectionCheck()) showSDContents();
    else
    {
      String output = "\nERROR! SD card is not connected";
      pNotifyCharacteristic->setValue(output.c_str());
      pNotifyCharacteristic->notify();
    }
  }

  // If command is instrumentCheck: perform a pre-flight check for instruments
  else if (BLECommand == "instrumentCheck")
  {
    BLECommand = "0"; // Set BLE command to default to prevent running in loop
    instrumentCheck();
  }

  // If command is dataLog: start new datalogging session for sensor data
  else if (BLECommand == "dataLog")
  {
    int index = getNextDataLogIndex(); // Get a new index for the dataLog folder

    String logFolderPath = "/dataLog" + String(index); // Store the path for the new dataLog folder in a String variable

    createDataLog(logFolderPath); // Create new dataLog folder on SD card

    startLogging(logFolderPath + "/xyz.csv", logFileADXL345); // Start logging session for ADXL345 data
    startLogging(logFolderPath + "/bmp.csv", logFileBMP280); // Start logging session for BMP280 data

    startTime = millis(); // Measure the time that has passed from setup initialization
    passedLogTime = 0; // Zero the logging session time (this way we can log data starting from 0s)

    masterCalibration();

    BLECommand = "dataLogRunning"; // Set BLE command for dataLogRunning to start data logging
  }

  // If command is dataLogRunning: start logging sensor data
  else if (BLECommand == "dataLogRunning")
  {
    dataLogRunning();
  }

  else if (BLECommand == "prePressureTest")
  {
    BLECommand = "0";
    masterCalibration(); // Perform calibration for more accurate sensor data
    prePressureTest();
  }

  else if (BLECommand == "preLaunchTest")
  {
    BLECommand = "0";
    masterCalibration(); // Perform calibration for more accurate sensor data
    preLaunchTest();
  }

  else if (BLECommand == "launch")
  {
    int index = getNextDataLogIndex(); // Get a new index for the dataLog folder

    String logFolderPath = "/dataLog" + String(index); // Store the path for the new dataLog folder in a String variable

    createDataLog(logFolderPath); // Create new dataLog folder on SD card

    startLogging(logFolderPath + "/xyz.csv", logFileADXL345); // Start logging session for ADXL345 data
    startLogging(logFolderPath + "/bmp.csv", logFileBMP280); // Start logging session for BMP280 data

    startTime = millis(); // Measure the time that has passed from setup initialization
    passedLogTime = 0; // Zero the logging session time (this way we can log data starting from 0s)
    passedCountdownTime = 0;

    masterCalibration();

    BLECommand = "launchRunning";
  }
  else if (BLECommand == "launchRunning")
  {
    launchRunning();
  }

  // If command is not recognized: reject the command
  else
  {
    BLECommand = "OFF";
    Serial.println("ERROR. Command not recognized");
  }
}