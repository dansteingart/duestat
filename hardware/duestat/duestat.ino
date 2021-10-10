#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <AutoPID.h>
// #include <QuickPID.h>

StaticJsonDocument<1000> doc;

//pid settings and gains
#define OUTPUT_MIN 0
#define OUTPUT_MAX 3.3

String mode = "ocv";
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
int last;

int sample_period = 10000; //send sample at 10000 us
int a0 = 0; //set for dacc 
long a1 = 1986; //set for dac
long a2 = 0; //running sum
long a3 = 0; //running sum
long a4 = 0; //running sum
long a5 = 0; //running sum
int samps = 0; //OverSample counter
float rate = 1.0; //sine rate
float amp = 0.5;
double res1 = 1000.0;


//manual mode
double dac0 = 0; //dac0 potential
double dac1 = 0; //dac1 potential

//pstat
double output = 1;
double target = 1;
double VCell;
int rold = 0;

double KP = .1;
double KI = 0;
double KD = 0;

//forgot what this was about
AutoPID myPID(&VCell, &target, &output, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);

//
//QuickPID myQuickPID(&VCell, &target, &output, KP, KI, KD, QuickPID::DIRECT);


void setup() {
  Serial.begin(1000000); //fast right?
  analogReadResolution(14); //set read to 14 bits
  analogWriteResolution(12); //set write to 12 bits
  last = micros();
  inputString.reserve(1000);
  myPID.setBangBang(.0001);
  myPID.setTimeStep(1000);
  //myQuickPID.SetMode(QuickPID::AUTOMATIC);


}


void loop() {

  //check serial
  serialEvent();
  // print the string when a newline arrives:
  if (stringComplete) {
    DeserializationError error = deserializeJson(doc, inputString);
    //Serial.println(doc.as<String>());

    //a1 = inputString.toInt();
    if (doc.containsKey("dac0"))    dac0 = doc["dac0"];
    if (doc.containsKey("dac1"))    dac1 = doc["dac1"];
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
    Serial.print(3.3 * (a3 - 0) / pow(2, 14), 4);
    Serial.print("\t");
    Serial.print(3.3 * (a4 - 0) / pow(2, 14), 4);
    Serial.print("\t");
    Serial.print(3.3 * a5 / pow(2, 14), 4);
    Serial.print("\t");
    Serial.print(3.3 * a2 / pow(2, 14), 4); //GND reading
    Serial.print("\t");
    Serial.print(output,4);
    Serial.print("\t");
    Serial.print(target,12); //because we're expressing V and uA
    Serial.print("\t");
    Serial.print(VCell,4);
    Serial.print("\t");
    Serial.print(res1);
    Serial.print("\t");
    Serial.print(mode);
    Serial.print("\t");
    Serial.print(KP);
    Serial.print("\t");
    Serial.print(KI);
    Serial.print("\t");
    Serial.print(KD);
    Serial.print("\t");
    Serial.println(micros() - last); //time it took to send the package

    //reset counters
    a2 = 0;
    a3 = 0;
    a4 = 0;
    samps = 0;
    a5 = 0;

  }

  if (mode == "sine") 
  { 
      analogWrite(A0, (4095/2) + (4095/3.3)*(amp)* sin(micros()*0.000001 * rate));
  }

  if (mode == "ocv")
  {
    int aa2 = analogRead(A2);
    int aa4 = analogRead(A4);
    VCell = 3.3 * (aa4 - aa2) / pow(2, 14);

    //leaving here for my future self:
    //for whatever reason the SAMD51 doesn't like pinMode at all.
    //to OCV, just go to analogRead 
    //pinMode(A0, INPUT);
    analogRead(A0);

  }

   if (mode == "pstat")
   {
     pstat(target);
   }

   if (mode == "pstat_3")
   {
    double oldoutput = output;
    int aa4 = analogRead(A4);
    int aa5 = analogRead(A5);

    VCell = 3.3 * (aa4 - aa5) / pow(2, 14);
    double diff = VCell - target;
    output = oldoutput+diff*-1*KP;
    if (output > 3.3) output = 3.3;
    if (output < 0) output = 0;

    //AUTOPID NOT BEHAVING FOR SOME REASON 2021-10-04, pulling out
    //myPID.run(); //call every loop, updates automatically at certain time interval
    
    a0 = (int)(4095 * output / 3.3);
    if (a0 < 0) a0 = 0;
    else if (a0 > 4090) a0 = 4090;
    analogWrite(A0, a0);
    }

  if (mode == "gstat")
  {
    int aa4 = analogRead(A4); //
    int aa3 = analogRead(A3);
    VCell = 3.3 * (aa3 - aa4) / pow(2, 14);

    //AUTOPID NOT BEHAVING FOR SOME REASON 2021-10-04, pulling out
    //myPID.run(); //call every loop, updates automatically at certain time interval

    double oldoutput = output;
    double diff = VCell - target*res1;
    output = oldoutput+diff*-1*KP;
    if (output > 3.3) output = 3.3;
    if (output < 0) output = 0;

    a0 = (int)(4095 * output / 3.3);
    if (a0 < 0) a0 = 0;
    else if (a0 > 4090) a0 = 4090;
    analogWrite(A0, a0);
  }
  
  if (mode == "manual")
  {
    int aa2 = analogRead(A2);
    int aa4 = analogRead(A4);
    VCell = 3.3 * (aa4 - aa2) / pow(2, 14);
    a0 = (int)(4095 * dac0 / 3.3);
    a1 = (int)(4095 * dac1 / 3.3);
    if (a0 < 0) a0 = 0;
    else if (a0 > 4090) a0 = 4090;
    if (a1 < 0) a1 = 0;
    else if (a1 > 4090) a1 = 4090;
    
    analogWrite(A0, a0);
  }

  analogWrite(A1, dac1*4095/3.3); //write xxx V to refAD620
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte and add it to the inputString:

    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {stringComplete = true;}
  }
}

void pstat(double targ)
{
    double oldoutput = output;
    int aa2 = analogRead(A2);
    int aa4 = analogRead(A4);

    VCell = 3.3 * (aa4 - aa2) / pow(2, 14);
    double diff = VCell - targ;
    output = oldoutput+diff*-1*KP;
    if (output > 3.3) output = 3.3;
    if (output < 0) output = 0;

    a0 = (int)(4095 * output / 3.3);
    if (a0 < 0) a0 = 0;
    else if (a0 > 4090) a0 = 4090;
    analogWrite(A0, a0);
}