
// Define a CS communication pin for SD adapter
#define SD_CS_PIN 4

// Function to initialize SD card
void SDSetup()
{
  for (int i = 0; i < 5; i++) // Try to initialize SD card for 5 times
  {
    if (SD.begin(SD_CS_PIN)) // If initialized successfully, return
    {
      Serial.println("SD card initialized");
      return;
    }
  }
  Serial.println("ERROR. SD card couldn't be initialized...");
  //while(1);
}