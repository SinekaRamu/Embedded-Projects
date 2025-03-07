void setup()
{
  Serial.begin(115200);

  pinMode(D2, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  Serial.println("begin");
}

void loop()
{

  // digitalWrite(D2, HIGH);
  // Serial.println("HIGH");
  // delay(2000);
  // digitalWrite(D2, LOW);
  // Serial.println("LOW");
  // delay(5000);

  while (Serial.available())
  {
    char value = Serial.read();
    Serial.println(value);
    if (value == '1')
    {
      digitalWrite(D2, LOW);
    }
    else if (value == '2')
    {
      digitalWrite(D5, LOW);
    }
    else if (value == '3')
    {
      digitalWrite(D6, LOW);
    }
    delay(2000);
    digitalWrite(D2, HIGH);
    digitalWrite(D5, HIGH);
    digitalWrite(D6, HIGH);
  }
}
