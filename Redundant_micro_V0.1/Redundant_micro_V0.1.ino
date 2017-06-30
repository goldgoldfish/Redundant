//declaring pin variables
int masterLED = 8;
int redundant = 9;
int LED = 11;

//declaring global variables
int is_Master = 1;
unsigned long curr_time_micro = 0;
unsigned long last_heartbeat_sent = 0;
unsigned long last_heartbeat_rec = 0;

//declaring void functions
void startup();
void heartbeat(char cmmd, char data);

//declaring returning functions
int get_Data();

void setup() {
  //setup pins
  pinMode(masterLED, OUTPUT); //set master LED pin to output
  pinMode(redundant, OUTPUT); //set redundant LED pin to output
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
        is_Master = 0; //set this micro to slave mode
        last_heartbeat_rec = micros(); //get the current time
        digitalWrite(masterLED, is_Master); //update the LED
      } //end if
    } //end while
    curr_time_micro = micros(); //get the current time
    if ((is_Master) && (curr_time_micro >= last_heartbeat_sent + 1000)) { //if the micro is the master and 5ms has elapsed
      prev_LED_data = curr_LED_data; //sync curr and prev data
      curr_LED_data++; //incriment data 
      heartbeat(170, curr_LED_data); //send heartbeat
      last_heartbeat_sent = curr_time_micro;
      analogWrite(LED, curr_LED_data); //update the LED
    } //end if
    else if (!(is_Master) && (curr_time_micro > last_heartbeat_rec + 10000)) { //change mirco to master if no recent heartbeat
      is_Master = 1; //set this micro to master mode
      prev_LED_data = curr_LED_data; //sync curr and prev data
      curr_LED_data++; //incriment data 
      heartbeat(170, curr_LED_data); //send heartbeat
      last_heartbeat_sent = curr_time_micro;
      analogWrite(LED, curr_LED_data); //update the LED
    } //end else if
    else { //do other stuff
      if(is_Master);
      else digitalWrite(LED, LOW); //turn LED off
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

void heartbeat(char cmmd, char data) {
  Serial.write(cmmd); //send command byte
  Serial.flush(); //wait for the transfer to be complete
  Serial.write(data); //send data byte
  Serial.flush(); //wait for the transfer to be complete
} //end heartbeat

int get_Data(){
  int iter = 0; //start iter at 0
  while(!(Serial.available()) || iter > 1000){ //wait for data to come in
    iter++;
  } //end while
  return Serial.read(); //read byte into data_in
} //end get_Data

