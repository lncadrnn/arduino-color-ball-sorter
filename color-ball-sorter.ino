#include <Servo.h>

// Create objects for the servo motors
Servo gateServo;
Servo sorterServo;

// Define pin numbers
const int gateServoPin = 9;       // Gate servo signal pin connected to pin 9
const int sorterServoPin = 10;    // Sorter servo signal pin connected to pin 10
const int colorSensorS0 = 2;      // TCS3200 S0 pin connected to pin 2
const int colorSensorS1 = 3;      // TCS3200 S1 pin connected to pin 3
const int colorSensorS2 = 4;      // TCS3200 S2 pin connected to pin 4
const int colorSensorS3 = 7;      // TCS3200 S3 pin connected to pin 5
const int colorSensorOut = 8;     // TCS3200 OUT pin connected to pin 6

// Define angles for servo motors
const int gateClosedAngle = 0;
const int gateOpenAngle = 45; // Reduced gate opening angle
const int redPosition = 60;   // Adjusted sorter angle for red
const int bluePosition = 120; // Adjusted sorter angle for blue
const int neutralPosition = 90; // Neutral position for other colors

// Variables for non-blocking delays
unsigned long previousMillis = 0;
const long interval = 500;

// Threshold values for frequency comparison
int redThreshold = 10;
int blueThreshold = 15; // Increased threshold for blue detection

void setup() {
  Serial.begin(9600);

  // Attach servos
  gateServo.attach(gateServoPin);
  sorterServo.attach(sorterServoPin);

  // Set initial servo positions
  gateServo.write(gateClosedAngle);
  sorterServo.write(neutralPosition);

  // Initialize TCS3200 sensor pins
  pinMode(colorSensorS0, OUTPUT);
  pinMode(colorSensorS1, OUTPUT);
  pinMode(colorSensorS2, OUTPUT);
  pinMode(colorSensorS3, OUTPUT);
  pinMode(colorSensorOut, INPUT);

  // Set sensor frequency scaling to 20%
  digitalWrite(colorSensorS0, HIGH);
  digitalWrite(colorSensorS1, LOW);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Run the sorting process if enough time has passed (non-blocking delay)
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Get average frequency for each color
    int redFrequency = getAverageFrequency(LOW, LOW);
    int greenFrequency = getAverageFrequency(HIGH, HIGH);
    int blueFrequency = getAverageFrequency(LOW, HIGH);
    
    // Debug output for frequency readings
    Serial.print("Red Frequency: "); Serial.print(redFrequency);
    Serial.print(" Green Frequency: "); Serial.print(greenFrequency);
    Serial.print(" Blue Frequency: "); Serial.println(blueFrequency);
    
    // Determine the ball's color using the new logic
    if (isRed(redFrequency, greenFrequency, blueFrequency)) {
      sorterServo.write(redPosition);
      Serial.println("Ball is Red. Sorting Left.");
    } else if (isBlue(redFrequency, greenFrequency, blueFrequency)) {
      sorterServo.write(bluePosition);
      Serial.println("Ball is Blue. Sorting Right.");
    } else {
      sorterServo.write(neutralPosition);
      Serial.println("Ball color unrecognized. Setting to Neutral.");
    }

    // Open the gate to release the ball
    gateServo.write(gateOpenAngle);
    delay(1000); // Wait for the ball to pass through
    gateServo.write(gateClosedAngle);

    // Wait for the ball to settle into the bin
    delay(2000); 
  }
}

// Function to get the average frequency over multiple readings
int getAverageFrequency(int s2State, int s3State) {
  int totalFrequency = 0;
  for (int i = 0; i < 5; i++) {
    digitalWrite(colorSensorS2, s2State);
    digitalWrite(colorSensorS3, s3State);
    totalFrequency += pulseIn(colorSensorOut, LOW);
    delay(10); // Small delay between readings
  }
  return totalFrequency / 5;
}

// Function to check if the color is red
bool isRed(int r, int g, int b) {
  return (r < g && r < b && r < 200); // Adjust thresholds as needed
}

// Function to check if the color is blue
bool isBlue(int r, int g, int b) {
  return (b < r && b < g && b < 200); // Adjust thresholds as needed
}