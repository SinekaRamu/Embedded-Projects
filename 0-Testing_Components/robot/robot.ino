#define ENA 16  // Speed control for Left motors
#define ENB 17  // Speed control for Right motors
#define IN1 5   // Left motor forward
#define IN2 18  // Left motor backward
#define IN3 19  // Right motor forward
#define IN4 21  // Right motor backward

void setup() {
    Serial.begin(115200);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    
}

void loop() {
    Serial.println("Forward");
    moveForward();
    delay(2000);
    Serial.println("Backward");
    moveBackward();
    delay(5000);
    Serial.println("left");
    turnLeft();
    delay(3000);
    Serial.println("right");
    turnRight();
    delay(4000);
    Serial.println("stop");
    stopMotors();
    delay(5000);
}

void moveForward() {
    analogWrite(ENA, 250);  // Set motor speed (0-255)
    analogWrite(ENB, 250);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void moveBackward() {
    analogWrite(ENB, 100);
    analogWrite(ENB, 100);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void turnLeft() {
    analogWrite(ENB, 150);
    analogWrite(ENB, 150);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight() {
    analogWrite(ENB, 150);
    analogWrite(ENB, 150);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void stopMotors() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}