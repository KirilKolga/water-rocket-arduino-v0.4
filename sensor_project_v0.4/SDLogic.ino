
// Functions to help operate the SD card files

// Function to check if the file path exists
bool fileExists(const String &path)
{
  if (SD.exists(path)) // Use built in SD.exists() function to check if the file exists
  {
    return true;
  }
  return false;
}

// Function to print out contents of the file
void readFile(const String &path)
{
  if (!fileExists(path)) // Check if the file exists
  {
    Serial.println("ERROR. File " + path + " doesn't exist");
    return;
  }

  File file = SD.open(path); // Open the file 

  if (!file) // Check if the file is opened
  {
    Serial.println("Error occurred while trying to open " + path);
    return;
  }

  Serial.println("\nContent of the file " + path + " line by line:");
  while(file.available()) // Read the file until the end
  {
    Serial.write(file.read()); // Print out the results in Serial monitor
  }

  file.close();
}

// Function to ist all the files on SD card (can be applied to any directory by entering its path)
void listDir(String &output, const char* dirname = "/", uint8_t levels = 4, String indent = "") // Default directory is "/", default depth level is 4, String indent helps to print the results
{
  File root = SD.open(dirname); // Open the directory or folder
  if (!root || !root.isDirectory()) return; // Check if it crash and is it actually directory/folder

  File file = root.openNextFile(); // Open the first file in root directory

  while (file) // Continue, while there is more files in the directory
  {
    if (file.isDirectory()) // Check if the file is itself a directory
    { // If so, print its name
      output += indent;
      output += "[DIR] ";
      output += (String(file.name()) + "\n");

      // Check how deep user wanted to go in directories

      if (levels) 
      { // Start recursion by opening the directory through that function
        String path = String(dirname);
        if (path != "/") path += "/";
        path += file.name();

        listDir(output, path.c_str(), levels - 1, indent + "  "); // Take one level off and make indent longer
      }

    } 
    else // If the file isn't a directory print it out
    {
      output += indent;
      output += "FILE: ";
      output += file.name();
      output += " (";
      output += String(file.size());
      output += " bytes)\n";
    }

    file = root.openNextFile(); // Go to the next file
  }
}

// Helper function to send SD card contents to the client one directory at a time
void sendByDir(String &text)
{
  std::string output = text.c_str(); // Convert the output to C++ string format
  int dirAmount = 0; // Count the amount of directories been sent
  
  while(output.find("[DIR]") != std::string::npos) // Check if there is a directory left
  {
    int newDirIndex = output.find("[DIR]"); // Find the index of directory in output string

    std::string dir = output.substr(0, newDirIndex + 1); // Collect the output until another directory (take also another directorys first character "[" for correct next index)
    if (dirAmount != 0) dir = "[" + dir; // If the directory isn't first, add "[" character back
    dir.pop_back(); // Remove the extra "[" character at the end of the directory string

    output.erase(0, newDirIndex + 1); // Remove printed directory from original output string

    // Send the output to the client
    pNotifyCharacteristic->setValue(dir.c_str());
    pNotifyCharacteristic->notify();
    
    dirAmount++; // Increase directory amount
  }

  // If there is no full "[DIR]" string left, just print the remaining output
  std::string dir = output;
  dir = "[" + dir; // Remember to add "[" character at the back

  // Send the output to the client
  pNotifyCharacteristic->setValue(dir.c_str());
  pNotifyCharacteristic->notify();

}

// Function to send the contents of the SD card to the client
void showSDContents()
{
  String output = ""; // Create a string in which output will be stored

  listDir(output); // Assemble an output by using listDir function
  sendByDir(output); // Send the output one directory at a time (otherwise the output could be too large)
}

void wipeSD(fs::FS &fs, const char * dirname, uint8_t levels) {
    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        String fileName = file.name();
        String fullPath = String(dirname) + "/" + fileName;
        
        // Handle Directories
        if (file.isDirectory()) {
            if (levels) {
                // Recursive call to empty the folder first
                wipeSD(fs, fullPath.c_str(), levels - 1);
            }
            // Remove the now-empty directory
            if (fs.rmdir(fullPath.c_str())) {
                Serial.printf("Deleted Folder: %s\n", fullPath.c_str());
            } else {
                Serial.printf("Failed to delete folder: %s\n", fullPath.c_str());
            }
        } 
        // Handle Files
        else {
            if (fs.remove(fullPath.c_str())) {
                Serial.printf("Deleted File: %s\n", fullPath.c_str());
            } else {
                Serial.printf("Failed to delete file: %s\n", fullPath.c_str());
            }
        }
        file = root.openNextFile();
    }
}

