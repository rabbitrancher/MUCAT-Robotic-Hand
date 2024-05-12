/* 
"The Hand" refers to the 3D printed arm, and "The Glove" refers to the glove with sensors attached to it.
This code is written for an Arduino Mega, and calculates the bends of each finger of The Glove
by using 5 voltage dividers made up of a 820 ohm resistor and a ZD10-100 flex sensor, and then mapping this
data from an analog range to a percent range. It then writes this data to an atatched HC-05 Bluetooth module, 
which is paired with another HC-05 Bluetooth module on The Hand, which is modeled from the open-source InMoov
project. The Blueooth Modules use a system of confirmation characters sent from The Hand in order to make 
sure they are full synced when sending data.
*/


// Constants
const int FLEX_MIN = 0;               // Minimum analogRead value
const int FLEX_MAX = 1023;            // Maximum analogRead value
const int PERCENT_MIN = 0;            // Minimum percentage value
const int PERCENT_MAX = 100;          // Maximum percentage value
// TODO see if transmission interval can be shorted
const unsigned long TRANSMISSION_INTERVAL = 500L; // Delay between data transmissions (milliseconds)

// Variables
byte thumbFlexPercent;
byte pointFlexPercent;
byte midFlexPercent;
byte ringFlexPercent;
byte pinkFlexPercent;
byte wristRotation = 90.0;    // Initial position assumed to be centered
unsigned long prevTime = 0L;
byte isOn = 0;

// Pins for flexsensors
int thumbFlexPin = A11;
int pointFlexPin = A3;
int midFlexPin = A6;
int ringFlexPin = A0;
int pinkFlexPin = A15;


void setup() {
  Serial.begin(9600);       // Serial is used for debugging print statements.
  Serial2.begin(38400);     // Serial2 on the MEGA is used to communicate with the Bluetooth module.
}


void loop() {
  unsigned long currentTime = millis();

  // Check if it's time to send data by seeing if TRANSMISSION_INTERVAL ms have been passed, as well as
  //    if Serial2 has recieved a confirmation char.
  if (currentTime - prevTime >= TRANSMISSION_INTERVAL && (Serial2.available() > 0)) {     // refered to as "active sending sequence"
    // remove confirmation char from queue.
    Serial2.read();
    // Read finger flexion percentages by using the analog value gathered from each flex sensor, which
    //    uses a voltage divider to calculate its resistance and thus its percent bend by a simple map. 
    thumbFlexPercent = map(analogRead(thumbFlexPin), FLEX_MIN, FLEX_MAX, PERCENT_MIN, PERCENT_MAX);
    pointFlexPercent = map(analogRead(pointFlexPin), FLEX_MIN, FLEX_MAX, PERCENT_MIN, PERCENT_MAX);
    midFlexPercent = map(analogRead(midFlexPin), FLEX_MIN, FLEX_MAX, PERCENT_MIN, PERCENT_MAX);
    ringFlexPercent = map(analogRead(ringFlexPin), FLEX_MIN, FLEX_MAX, PERCENT_MIN, PERCENT_MAX);
    pinkFlexPercent = map(analogRead(pinkFlexPin), FLEX_MIN, FLEX_MAX, PERCENT_MIN, PERCENT_MAX);

    // TODO implement a sensor to set the rotation of the wrist servo.
    wristRotation = 90;

    // Send data
    // TODO create a toggleable interface that controls whether The Hand will update based off the new data or
    //    remain in its previous state, using isOn = 0 as a "pause" and isOn = 1 as a "go".
    isOn = 1;
    byte valueArray[] = {isOn, thumbFlexPercent, pointFlexPercent, midFlexPercent, ringFlexPercent, pinkFlexPercent, wristRotation};

    // Send each byte in the array over the Bluetooth module.
    for (int i = 0; i < 7; i++) {
      Serial2.write(valueArray[i]);
      // TODO shorten this delay time in between data transfer if possible.
      delay(100);
    }

    // Update previous time
    prevTime = currentTime;

    // Debugging: Print sent data
    Serial.print("Sent data: ");
    Serial.print("Active?: "); Serial.print(isOn);
    Serial.print(", Thumb Flex: "); Serial.print(thumbFlexPercent);
    Serial.print(", Point Flex: "); Serial.print(pointFlexPercent);
    Serial.print(", Mid Flex: "); Serial.print(midFlexPercent);
    Serial.print(", Ring Flex: "); Serial.print(ringFlexPercent);
    Serial.print(", Pink Flex: "); Serial.print(pinkFlexPercent);
    Serial.print(", Wrist Rotation: "); Serial.print(wristRotation);
    Serial.println();

  } // end "active sending sequence"
}
