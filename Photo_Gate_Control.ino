int photoGate;

void setup(){
  Serial.begin(9600);
  pinMode(6, INPUT);
}

void loop(){
  photoGate = digitalRead(6);
  Serial.print("photoGate: ");
  Serial.println(photoGate);
}
