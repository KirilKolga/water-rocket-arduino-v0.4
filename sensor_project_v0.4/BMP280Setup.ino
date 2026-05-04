
// Create BMP280 sensor object
Adafruit_BMP280 bmp;


// Function to initialize BMP280 sensor
void BMP280Setup()
{
  for (int i = 0; i < 5; i++) // Try to initialize BMP280 for 5 times
  {
    unsigned status; // Create variable for BMP280 sensor
    status = bmp.begin(0x76); // Select the BMP280 sensor

    if (status) // If initialized successfully, return
    {
      bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                 Adafruit_BMP280::SAMPLING_X2,    /* Temp. oversampling */
                 Adafruit_BMP280::SAMPLING_X16,   /* Pressure oversampling */
                 Adafruit_BMP280::FILTER_X16,     /* Filtering. */
                 Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
      Serial.println("BMP280 initialized");
      return;
    }
  }
  Serial.println("ERROR. BMP280 couldn't be initialized...");
  while(1);
}