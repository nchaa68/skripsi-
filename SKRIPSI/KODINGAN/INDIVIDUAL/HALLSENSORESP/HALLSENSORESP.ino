const int hallPin0 = 14; // Hall sensor home
const int hallPin1 = 27; // Hall sensor home
const int hallPin2 = 26;  // Hall sensor satu
const int hallPin3 = 25;  // Hall sensor kedua
const int hallPin4 = 33;  // Hall sensor ketiga
volatile bool sensor0Detected = false;
volatile bool sensor1Detected = false;
volatile bool sensor2Detected = false;
volatile bool sensor3Detected = false;
volatile bool sensor4Detected = false;

void setup() {
  Serial.begin(115200); 
  pinMode(hallPin0, INPUT);
  pinMode(hallPin1, INPUT);
  pinMode(hallPin2, INPUT);
  pinMode(hallPin3, INPUT);
  pinMode(hallPin4, INPUT);
  attachInterrupt(hallPin0, hallSensor0, RISING);
  attachInterrupt(hallPin1, hallSensor1, RISING);
  attachInterrupt(hallPin2, hallSensor2, RISING);
  attachInterrupt(hallPin3, hallSensor3, RISING);
  attachInterrupt(hallPin4, hallSensor4, RISING);
}

void loop() {
    if (sensor0Detected) {
    Serial.println("compartement home0 detected");
    sensor0Detected = false;
  }

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


void hallSensor0() {
  sensor0Detected = true;
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
