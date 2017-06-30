/*
 * Redundant_micro
 * Ben Wedemire
 * 
 * Description: This code is meant for two arduino like controllers connected
 * together by a UART interface running at 115200 baud. The serial interface
 * is a uses small, single byte, messages to update the other controller on their
 * statues and critical data. The system is designed so that if one controller
 * fails the other one will immdiately take over and begin controller the device
 * that the controller group is controlling.
 */

//declaring pin variables
int masterLED = 8;
int LED = 11;
int red_ok = 12;

//declaring global variables
int is_Master = 1;
const unsigned long incr_time = 10; //time in milliseconds between LED incriment
unsigned long curr_time_micro = 0;
unsigned long last_heartbeat_sent = 0;
unsigned long last_heartbeat_rec = 0;
unsigned long last_LED_up = 0;

//declaring void functions
void startup();
void heartbeat(char out);

//declaring returning functions
int get_Data();

void setup() {
  //setup pins
  pinMode(masterLED, OUTPUT); //set master LED pin to output
  pinMode(red_ok, OUTPUT); //set red_ok LED pin to output
  pinMode(LED, OUTPUT); //set LED pin to output
  //start serial
  Serial.begin(115200); //start up serial communication on pins 0 and 1.
  //do startup
  startup();
} //end setup

void loop() {
  int inByte = 0; //create temp for serial in
  char cmmd = 0; //cmmd byte
  char curr_LED_data = 0; //most recent LED data
  char prev_LED_data = 0; //previous LED data
  curr_time_micro = micros(); //get the current time
  last_heartbeat_sent = curr_time_micro;
  while(1) {
    while (Serial.available()) {
      inByte = Serial.read(); //read the data on the serial port
      if (inByte == 170) {
        curr_LED_data = get_Data();
        heartbeat(100);
        is_Master = 0; //set this micro to slave mode
        last_heartbeat_rec = micros(); //get the current time
        digitalWrite(masterLED, is_Master); //update the LED
        digitalWrite(LED, LOW); //turn LED off
      } //end if
    } //end while
    curr_time_micro = micros(); //get the current time
    if ((is_Master) && (curr_time_micro >= last_heartbeat_sent + 500)) { //if the micro is the master and 5ms has elapsed
      if ((curr_time_micro) >= (last_LED_up + (incr_time*1000))){ //check if enough time has elasped since last led val update
        prev_LED_data = curr_LED_data; //sync curr and prev data
        curr_LED_data++; //incriment data
        last_LED_up = curr_time_micro;
      } //end if
      heartbeat(170); //send heartbeat cmmd to other controller
      heartbeat(curr_LED_data); //send data to other controller
      if(get_Data() == 100) digitalWrite(red_ok, HIGH); //since the controller rec ack redundant op is good
      else digitalWrite(red_ok, LOW); //since no ack, turn off red_ok
      last_heartbeat_sent = curr_time_micro;
      analogWrite(LED, curr_LED_data); //update the LED
    } //end if
    else if (!(is_Master) && (curr_time_micro > last_heartbeat_rec + 1000)) { //change mirco to master if no recent heartbeat
      is_Master = 1; //set this micro to master mode
      prev_LED_data = curr_LED_data; //sync curr and prev data
      curr_LED_data++; //incriment data since the controller could've missed data
      last_LED_up = curr_time_micro;
      last_heartbeat_sent = curr_time_micro;
      analogWrite(LED, curr_LED_data); //update the LED
      heartbeat(170); //send heartbeat cmmd to other controller
      heartbeat(curr_LED_data); //send data to other controller
      if(get_Data() == 100) digitalWrite(red_ok, HIGH); //since the controller rec ack redundant op is good
      else digitalWrite(red_ok, LOW); //since no ack, turn off red_ok
    } //end else if
    else if (!(is_Master)){
      
    } //end else if
    else { //do other stuff
      
    } //end else
    digitalWrite(masterLED, is_Master); //update LED
  } //end while
} //end loop

void startup() {
  unsigned long time_begin = millis(); //time that startup started
  unsigned long time_curr = millis(); //current time in millis
  int no_cmmd = 1; //flag for if a cmmd has not been received
  int inByte = 0; //create temp for serial in
  while ((time_curr < (time_begin + 2000)) && (no_cmmd)) { //check if 2 secs have passed
    while (Serial.available()) { //check if there is something on the serial port to read
      inByte = Serial.read(); //read the data on the serial port
      if (inByte == 170) {
        is_Master = 0; //set the master status to be false
        last_heartbeat_rec = micros(); //get the current time in uS
        no_cmmd = 0; //a command has been receiced
      } //end if
    } //end while
    time_curr = millis(); //get current time.
  } //end while
  //if another controller has not assumed command this one will
  digitalWrite(masterLED, is_Master); //update status LED
} //end startup

void heartbeat(char out) {
  Serial.write(out); //send output byte
  Serial.flush(); //wait for the transfer to be complete
} //end heartbeat

int get_Data(){
  int iter = 0; //start iter at 0
  while(!(Serial.available()) && (iter < 100)){ //wait for data to come in
    iter++;
  } //end while
  int temp = 0;
  while(Serial.available()){
    temp = Serial.read(); //get data
    if (temp < 0) return 0; //return 0 if bad values
    return temp; //return data from serial port
  } //end while
  return 0; //return 0 if no data received
} //end get_Data

