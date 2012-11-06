
int pin = 0;

int val0 = 0;
int val1 = 0;
int val2 = 0;
int val3 = 0;
int val4 = 0;
int val5 = 0;

long startTime = 0;
long elapsedTime = 0;     // number of seconds that have passed since starting
int caught = 0;           // boolean indicating we have hit an 'event'
int dump = 0;             // boolean indicating we have dumped EEPROM
int count = 0;

void setup() {
 Serial.begin(9600); 
 //pinMode(12, OUTPUT);
 //digitalWrite(12, HIGH);
 startTime = millis();
}


void loop() {
  
  count++;
  val0 = analogRead(pin);
  val1 = analogRead(1);
  //val2 = analogRead(2);
  val3 = analogRead(3);
  
  //val3 = analogRead(2);
  if(count == 90){
       Serial.print(val0);
       Serial.print(".");
       Serial.print(val1);
       Serial.print(".");       
       Serial.print(val2);
       Serial.print(".");       
       Serial.print(val3);
       Serial.print(".");       
       Serial.print(val4);
       Serial.print(".");
       Serial.print(val5);
       Serial.println("#");
  count = 0;
  } 
    
    

 }

