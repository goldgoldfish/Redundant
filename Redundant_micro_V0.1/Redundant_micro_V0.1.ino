//declaring pin variables
int masterLED = 8;
int redundant = 9;
int LED = 13;

//declaring global variables
int is_Master = 1;
unsigned long curr_time_micro = 0;
unsigned long prev_time_micro = 0;
unsigned long last_heartbeat_rec = 0;

//declaring functions
void startup();
void heartbeat();

void setup() { //begin setup
  //setup pins
  pinMode(masterLED, OUTPUT); //set master LED pin to output
  pinMode(redundant, OUTPUT); //set redundant LED pin to output
  pinMode(LED, OUTPUT); //set LED pin to output

  Serial.begin(9600); //start up serial communication on pins 0 and 1.
  startup();
} //end setup

void loop() { //begin loop
  curr_time_micro = micros(); //get the current time
  int inByte = 0; ////create temp for serial in
    while (Serial.available() > 0) {
    inByte = Serial.read(); //read the data on the serial port
    if (inByte == 170) {
      is_Master = 0; //set this micro to slave mode
      last_heartbeat_rec = micros(); //get the current time
      digitalWrite(masterLED, is_Master); //update the LED
    } //end if
  } //end if
  if ((is_Master) && (curr_time_micro >= prev_time_micro + 1000)) { //if the micro is the master and 1ms has elapsed
    digitalWrite(LED, HIGH);   // turn the LED on
    heartbeat(); //perform heartbeat
    prev_time_micro = curr_time_micro;
  } //end if
  else if (!(is_Master) && (last_heartbeat_rec + 100000 < curr_time_micro)) { //change mirco to master
    is_Master = 1; //set this micro to master mode
    digitalWrite(masterLED, is_Master); //update LED
    heartbeat(); //send heartbeat
    prev_time_micro = curr_time_micro;
  } //end else if
  else { //do calculations here
    
  } //end else
} //end loop

void startup() { //begin startup
  unsigned long time_begin = millis(); //time that startup started
  unsigned long time_end = millis(); //current time in millis
  int no_cmmd = 1; //flag for if the cmmd has been received
  int inByte = 0; //create temp for serial in
  while ((time_end < (time_begin + 2000)) && (no_cmmd)) { //check if 2 secs have passed
    while (Serial.available() > 0) { //check if there is something on the serial port to read
      inByte = Serial.read(); //read the data on the serial port
      if (inByte == 170) {
        is_Master = 0; //set the master status to be false
        last_heartbeat_rec = micros(); //get the current time
        no_cmmd = 0;
      } //end if
    } //end while
    time_end = millis(); //get current time.
  } //end while
  //if another controller has not assumed command this one will
  digitalWrite(masterLED, is_Master); //update status LED
  if (is_Master) heartbeat(); //if controller is still master heartbeat
} //end startup

void heartbeat() {
  Serial.write(170); //send a value of 170 to indicate this controller is the master
  Serial.flush(); //wait for the transfer to be complete
} //end heartbeat
