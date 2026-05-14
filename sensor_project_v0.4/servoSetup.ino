
#define SERVO_COM_PIN 17

// Function to set up communication with the servo motor
void servoSetup()
{
  servo1.setPeriodHertz(50); // Set the pulse frequency

  servo1.attach(SERVO_COM_PIN, 1000, 1900); // Connect the servo with the com pin and set the pulse width range (500 = 0°, 2400 = 180°)
}

void servoTest()
{
  servo1.write(0);
  delay(1000);
  servo1.write(90);
  delay(1000);
  servo1.write(180);
  delay(1000);
}