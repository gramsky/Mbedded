int pin = 9;
int brightness = 255;
int count = 0;

/*
created 2006
 by David A. Mellis
 modified 30 Aug 2011
 by Tom Igoe and Scott Fitzgerald
 */

void setup(){
 Serial.begin(9600);
 pinMode(pin, OUTPUT);
}

void loop(){
  if(count == 10){
    brightness--;
    count = 0;
  }
  count++;
  Serial.print(brightness);
  Serial.print(".");
  Serial.println(count);
  analogWrite(pin, brightness);
}
