
// Define unique IDs for BLE service and characteristic
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab" // Unique UUID for BLE service
#define CMD_CHARACTERISTIC_UUID "abcd1234-ab12-cd34-ef56-1234567890ab" // Unique UUID for BLE command characteristic
#define NOTIFY_CHARACTERISTIC_UUID "5fd77936-eaee-4a6a-a0c0-52fa829e6432" // Unique UUID for BLE notify characteristic

// Pointers for BLE setup
BLEServer* pServer;
BLEService* pService;
BLECharacteristic* pCommandCharacteristic;
BLECharacteristic* pNotifyCharacteristic;

// Custon BLE callback class to handle device connection sessions
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) // onConnect() function is called automatically, when client connects
  {
    BLEDeviceConnected = true; // Set connection varible to true
    Serial.println("Client connected"); // For debugging
  }

  void onDisconnect(BLEServer* pServer)  // onDisconnect() function is called automatically, when client disconnects
  {
    BLEDeviceConnected = false; // Set connection varible to false
    Serial.println("Client disconnected"); // For debugging
    
    restartServerAdvertising = true; // Restart advertising to enable future client connections
  }
};


// Custon BLE Callback class to handle commands
class CommandCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic* pChar) // onWrite() function is called automatically, when BLE command is sent from another device
  {
    String value = pChar->getValue(); // Store received command in String variable

    Serial.println("Received command: " + value); // Print received command

    BLECommand = value.c_str(); // Set BLECommand variable to sent value
  }
};

// Function to setup the BLE communication
void BLESetup()
{
  BLEDevice::init("ESP_sensor_device"); // Initialize BLE device

  pServer = BLEDevice::createServer(); // Create BLE Server
  pServer->setCallbacks(new MyServerCallbacks());

  pService = pServer->createService(SERVICE_UUID); // Create BLE Service

  pCommandCharacteristic = pService->createCharacteristic(CMD_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE); // Create BLE command characteristic with writing property
  pCommandCharacteristic->setCallbacks(new CommandCallbacks()); // Set custom callback class for the command characteristic

  pNotifyCharacteristic = pService->createCharacteristic(NOTIFY_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY); // Create BLE notify characteristic with notify property
  pNotifyCharacteristic->addDescriptor(new BLE2902()); // Set predifined callback class for the notify characteristic

  pService->start(); // Start the service


  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising(); // Initialize advertising for our device
  pAdvertising->addServiceUUID(SERVICE_UUID); // Add our service to advertising list
  pAdvertising->start(); // Start advertising
}

// Function to check if restarting server advertising is needed
void needServerAdvertising()
{
  if (restartServerAdvertising)
  {
    restartServerAdvertising = false;
    delay(500);
    pServer->startAdvertising();
    Serial.println("Restarted advertising");
  }
}

