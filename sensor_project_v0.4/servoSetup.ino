
#define SERVO_COM_PIN 18

// Function to set up communication with the servo motor
void servoSetup()
{
  Servo servo1; // Create servo object

  servo1.setPeriodHertz(50); // Set the pulse frequency

  servo1.attach(SERVO_COM_PIN, 500, 2400); // Connect the servo with the com pin and set the pulse width range (500 = 0°, 2400 = 180°)
}