//Created By dansteingart
//Speed Up By peterkinget
// 2020-03-31

int last;
void setup() {
  Serial.begin(1000000); //fast right?
  analogReadResolution(14); //set read to 14 bits
  analogWriteResolution(12); //set write to 12 bits
  analogWrite(A0, pow(2, 12) * 1.6 / 3.3); //write 1.6 V to ref pin on AD620
  last = micros();
}

int sample_period = 10000; //send sample at 10000 us
long a2 = 0;  //running sum for a2
long a3 = 0; //running sum for a3
int samps = 0; //OverSample counter

void loop() {

  a2 += analogRead(A2); //read
  a3 += analogRead(A3); //read
  samps++; //increment OS
  int last_marker = 0;

  if (( micros() - last) > sample_period)
  {
    last_marker = micros() - last; //mark time since last send started
    last = micros();

    a2 = a2/samps; //average readings
    a3 = a3/samps; //average readings

    //send packet
    Serial.print(last_marker);
    Serial.print("\t");
    Serial.print(3.3 * a2 / pow(2, 14), 6);
    Serial.print("\t");
    Serial.print(3.3 * a3 / pow(2, 14), 6);
    Serial.print("\t");
    Serial.print(samps);
    Serial.print("\t");
    Serial.println(micros() - last);

    //reset counters
    a2 = 0;
    a3 = 0;
    samps = 0;
  }
}
