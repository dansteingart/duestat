#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

// #include <ArduinoJson.h>
// #include <ArduinoJson.hpp>
#include <AutoPID.h>

StaticJsonDocument<1000> doc;

//pid settings and gains
#define OUTPUT_MIN 0
#define OUTPUT_MAX 3.3

String mode = "ocv";

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
int last;

int sample_period = 5000; //send sample at 10000 us
int a0 = 0; //set for dacc 
long a1 = 1986; //set for dac
long a2 = 0; //running sum
long a3 = 0; //running sum
long a4 = 0; //running sum
long a5 = 0; //running sum
int samps = 0; //OverSample counter
float rate = 1000;
int amp = 1000;
double res1 = 1000.0;

//pstat
double output = 1;
double target = 1;
double VCell;
int rold = 0;

double KP = 8;
double KI = 5;
double KD = 5;


AutoPID myPID(&VCell, &target, &output, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);


void setup() {
  Serial.begin(1000000); //fast right?
  analogReadResolution(14); //set read to 14 bits
  analogWriteResolution(12); //set write to 12 bits
  last = micros();
  inputString.reserve(1000);
  myPID.setBangBang(.1);
  myPID.setTimeStep(50);


}



void loop() {

  //check serial
  serialEvent();
  // print the string when a newline arrives:
  if (stringComplete) {
    DeserializationError error = deserializeJson(doc, inputString);
    //Serial.println(doc.as<String>());

    //a1 = inputString.toInt();
    if (doc.containsKey("a1"))      a1 = doc["a1"];
    if (doc.containsKey("mode"))    mode = doc["mode"].as<String>();
    if (doc.containsKey("rate"))    rate = doc["rate"];
    if (doc.containsKey("amp"))     amp = doc["amp"];
    if (doc.containsKey("target"))  target = doc["target"];
    if (doc.containsKey("res1"))    res1 = doc["res1"];

    if (doc.containsKey("setpid"))
    {
      myPID.stop();
      KP = doc["kp"];
      KI = doc["ki"];
      KD = doc["kd"];
      long tts = doc["tts"];

      myPID.setTimeStep(tts);
      myPID.setGains(KP, KI, KD);
    }


    inputString = "";
    stringComplete = false;
  }



  a2 += analogRead(A2); //read
  a3 += analogRead(A3); //read
  a4 += analogRead(A4); //read
  a5 += analogRead(A5); //read
  samps++; //increment OS
  int last_marker = 0;

  if (( micros() - last) > sample_period)
  {
    last_marker = micros() - last; //mark time since last send started
    last = micros();

    a2 = a2 / samps; //average readingsi
    a3 = a3 / samps; //average readings
    a4 = a4 / samps; //average readings
    a5 = a5 / samps; //average readings


    //send packet

    Serial.print(last_marker);
    //Serial.print(0);
    Serial.print("\t");
    Serial.print(3.3 * (a3 - 0) / pow(2, 14), 6);
    Serial.print("\t");
    Serial.print(3.3 * (a4 - 0) / pow(2, 14), 6);
    Serial.print("\t");
    Serial.print(3.3 * a5 / pow(2, 14), 6);
    Serial.print("\t");
    Serial.print(3.3 * a2 / pow(2, 14), 6); //GND reading
    Serial.print("\t");
    Serial.print(output);
    Serial.print("\t");
    Serial.print(target);
    Serial.print("\t");
    Serial.print(VCell);
    //Serial.print(0);

    Serial.print("\t");
    Serial.println(micros() - last); //time it took to send the package
    //Serial.println(0);


    if (mode == "pots")
    {
      int temp = map(a5, 0, pow(2, 14), 0, pow(2, 12));
      a0 = constrain(temp, 0, pow(2, 12) - 100);
      analogWrite(A0, a0);
    }

    //reset counters
    a2 = 0;
    a3 = 0;
    a4 = 0;
    samps = 0;
    a5 = 0;

  }

  if (mode == "sine") analogWrite(A0, 4095 / 2 + amp * sin(micros()*.000000001 * rate));

  if (mode == "ocv")
  {
    int aa2 = analogRead(A2);
    int aa4 = analogRead(A4);
    VCell = 3.3 * (aa4 - aa2) / pow(2, 14);
    pinMode(A0, INPUT);
  }

  if (mode == "pstat")
  {
    int aa2 = analogRead(A2);
    int aa4 = analogRead(A4);
    VCell = 3.3 * (aa4 - aa2) / pow(2, 14);
    myPID.run(); //call every loop, updates automatically at certain time interval
    a0 = (int)(4095 * output / 3.3);
    if (a0 < 0) a0 = 0;
    else if (a0 > 4090) a0 = 4090;
    analogWrite(A0, a0);
  }

  if (mode == "gstat")
  {
    int aa4 = analogRead(A4);
    int aa3 = analogRead(A3);
    VCell = 3.3 * (aa3 - aa4) / pow(2, 14);
    myPID.run(); //call every loop, updates automatically at certain time interval
    a0 = (int)(4095 * output / 3.3);
    if (a0 < 0) a0 = 0;
    else if (a0 > 4090) a0 = 4090;
    analogWrite(A0, a0);
  }

  

  analogWrite(A1, a1); //write xxx V to refAD620


}


void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
      //      Serial.println(inputString);
      //      delay(1000);
    }
  }
}
