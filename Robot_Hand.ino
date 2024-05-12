/* 
"The Hand" refers to the 3D printed arm, and "The Glove" refers to the glove with sensors attached to it.
This code is written for an Arduino Uno, and turns MG996R servos on The Hand (modeled from the open-source 
InMoov project) to be a degree value matching a percent sent over from The Glove, in order to replicate the
bend of indivudal fingers. It receives this data from an atatched HC-05 Bluetooth module, which is paired 
with another HC-05 Bluetooth module on The Glove. The Blueooth Modules use a system of confirmation 
characters sent from The Hand in order to make sure they are full synced when sending data.
*/

// libraries
#include "SoftwareSerial.h"
#include <Servo.h>

// Constants
const byte THRESHOLD = 5;

// Variables
byte numbers[7];  // {isOn, thumbFlexPercent, pointFlexPercent, midFlexPercent, ringFlexPercent, pinkFlexPercent, wristRotation}
byte prevNum[7] = {0, 0, 0, 0, 0, 0, 0};
byte dataAmount = 0;

// A SoftwareSerial object used to communicate with the attached Bluetooth Module.
SoftwareSerial Glove(2, 3);  // RX | TX

// Servo Variables
Servo thumbServo;
Servo pointServo;
Servo midServo;
Servo ringServo;
Servo pinkServo;
Servo wristServo;
Servo servos[] = {thumbServo, pointServo, midServo, ringServo, pinkServo, wristServo};


void setup() {
  Serial.begin(9600);  // Initialize serial communication for debugging
  Glove.begin(38400);  // Initialize SoftwareSerial for communication with Bluetooth Module on The Hand.



  // Pins for servos
  thumbServo.attach(6);
  pointServo.attach(7);
  midServo.attach(8);
  ringServo.attach(9);
  pinkServo.attach(10);
  wristServo.attach(11);

  // Reset all fingers to flat
  for (int i = 0; i < 6; i++) {
    if (i != 0) {
      servos[i].write(170);
    } else {
      servos[i].write(10); // thumb's rotations are flipped due to servo setup.
    }
    
  }
  wristServo.write(90); //Reset wrist to a centered position.

  // debugging statement to say that The Hand is ready, and send a initial confirmation
  //    character to The Glove.
  Serial.println("Prepped");
  Glove.write('A');
}


void loop() {
  // if The Glove has sent data, add it to the data array.
  if (Glove.available() > 0) {
    numbers[dataAmount] = Glove.read();
    dataAmount++;
  }

  // if the data array has been fully filled, set the servos to the proper rotations based
  //    off the recieved data.
  if (dataAmount >= 7) { // referred to as "data fully recieved"
    dataAmount = 0;

    // Check if the hand is active, and if it is update the servos.
    if (numbers[0] == 1) { // referred to as "active state"

      // manual adjustments for each finger, discovered through trial and error.
      numbers[1] = map(numbers[1], 0, 60, 100, 0); // thumb (flipped)
      numbers[2] = map(numbers[2], 0, 66, 0, 100); // point
      numbers[3] = map(numbers[3], 10, 80, 0, 100); // middle (TODO see if this should be flipped)
      numbers[4] = map(numbers[4], 10, 50, 0, 100); // ring
      numbers[5] = map(numbers[5], 0, 80, 0, 100); // pinkie

      // cap each percent value at 100% or 0% to make sure it doesn't go outside the working range of
      for (int i = 0; i < 5; i++) {
        if (numbers[i+1] > 100 ){
          numbers[i+1] = 100;
        } else if (numbers[i+1] < 0) {
          numbers[i+1] = 0;
        }
      }

        // Adjust fingers
      for (int i = 0; i < 5; i++) {
         // if there's a change in the value that is greater than the threshold value, turn the servo to 
         //   reflect it. Used to avoid quivering between similar values, like 93 and 94 %.
        if (abs(numbers[i+1] - prevNum[i+1]) > THRESHOLD) {
          // Adjust index to start from 1 and map so 100% on the fingers means 180 degrees 
          //    turned (but buffered to not break the servos)
          servos[i].write(map(numbers[i + 1], 0, 100, 10, 170)); 
        }
      }

      // Turn wrist (in the mirrored direction)
      wristServo.write(180 - numbers[6]);

      // Debugging: Print received data
      Serial.print("Received data: ");
      Serial.print("Active?: "); Serial.print(numbers[0]);
      Serial.print(", Thumb Flex: "); Serial.print(numbers[1]);
      Serial.print(", Point Flex: "); Serial.print(numbers[2]);
      Serial.print(", Mid Flex: ");Serial.print(numbers[3]); 
      Serial.print(", Ring Flex: "); Serial.print(numbers[4]); 
      Serial.print(", Pink Flex: "); Serial.print(numbers[5]);
      Serial.print(", Wrist Rotation: "); Serial.print(numbers[6]);
      Serial.println();

      // Send acknowledgment to The Glove
      Glove.write('A');  // You can choose any character to represent acknowledgment

      // Store the current positions of each servo for comparison against the threshold.
      for (int i = 0; i < 7; i++) {
        prevNum[i] = numbers[i];
        numbers[i] = 0;
      }

    } // end "active state"
  } // end "data fully recieved"
}