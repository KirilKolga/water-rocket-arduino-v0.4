
// Create ADXL345 sensor object and define unique ID for it
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Function to initialize ADXL345 sensor
void ADXL345Setup()
{
  for (int i = 0; i < 5; i++) // Try to initialize ADXL345 for 5 times
  {
    if (accel.begin()) // If initialized successfully, return
    {
      accel.setRange(ADXL345_RANGE_16_G); // Set the G range to maximum (16 G)
      accel.setDataRate(ADXL345_DATARATE_3200_HZ); // Set the datarate to maximum (3200 Hz)
      Serial.println("ADXL345 initialized");
      return;
    }
  }
  Serial.println("ERROR. ADXL345 couldn't be initialized...");
  while(1);
}
