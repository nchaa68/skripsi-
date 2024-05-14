const int hallPin1 = 3; // Hall sensor pertama
const int hallPin2 = 2; // Hall sensor kedua
const int hallPin3 = 18; // Hall sensor ketiga
const int hallPin4 = 19; // Hall sensor keempat

volatile bool sensor1Detected = false;
volatile bool sensor2Detected = false;
volatile bool sensor3Detected = false;
volatile bool sensor4Detected = false;

void setup() {
  Serial.begin(115200);

  attachInterrupt(digitalPinToInterrupt(hallPin1), hallSensor1, RISING);
  attachInterrupt(digitalPinToInterrupt(hallPin2), hallSensor2, RISING);
  attachInterrupt(digitalPinToInterrupt(hallPin3), hallSensor3, RISING);
  attachInterrupt(digitalPinToInterrupt(hallPin4), hallSensor4, RISING);
}

void loop() {
  if (sensor1Detected) {
    Serial.println("compartement home detected");
    sensor1Detected = false;
  }

  if (sensor2Detected) {
    Serial.println("compartement 1 detected");
    sensor2Detected = false;
  }

  if (sensor3Detected) {
    Serial.println("compartement 2 detected");
    sensor3Detected = false;
  }

  if (sensor4Detected) {
    Serial.println("compartement 3 detected");
    sensor4Detected = false;
  }
}

void hallSensor1() {
  sensor1Detected = true;
}

void hallSensor2() {
  sensor2Detected = true;
}

void hallSensor3() {
  sensor3Detected = true;
}

void hallSensor4() {
  sensor4Detected = true;
}
