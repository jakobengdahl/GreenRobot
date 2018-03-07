#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER ""  //you MQTT IP Address
const char* ssid = "";
const char* password = "";
char const* switchTopic1 = "/robot/heart";

int pinCS = D4; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
// int pinCLK=5;
// int pinDIN=7;
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);


boolean boolUpdate=false;
String tape = "GREEN";
int wait = 180; // In milliseconds

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

//SNAKE STUFF:
const int pinRandom = A0;

int length = 6;
int x[8], y[8];
int ptr, nextPtr;

//SpinStuff:
int inc = -2;

//Animations:
int aniFrames=3;
byte mBitmap[64];


int seconds=0;
boolean talking=false;
int command=0;

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {
  matrix.setIntensity(4); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 3);

  ArduinoOTA.setHostname("robotheart"); // A name given to your ESP8266 module when discovering it as a port in ARDUINO IDE
  ArduinoOTA.begin(); // OTA initialization

  //start the serial line for debugging
  Serial.begin(115200);
  delay(100);

  //start wifi subsystem
  WiFi.begin(ssid, password);
  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();

  //wait a bit before starting the main loop
  delay(2000);
}


void loop(){

  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {reconnect();}

  //maintain MQTT connection
  client.loop();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10); 
  ArduinoOTA.handle();
  if(tape=="snake"){
    Snake();
  } else if(tape=="spin"){
    Spin();
  } else if(tape=="boom"){
    Boom(); 
  } else if(tape=="none"){
    none(); 
  } else if(tape=="heart"){
    heart();
  } else if(tape=="music"){
    Music();
  } else if(tape=="right"){
     right(); 
  } else if(tape=="left"){
     left(); 
  } else if(tape=="pacman"){
    pacman();
  } else if(tape=="pinky"){
    pinky();
  } else if(tape=="smile"){
    smile();
  } else if(tape=="skull"){
    skull();
  } else{
    MsgBoard(); 
  }
  boolUpdate=false;

}

void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic;
  String payloadStr; 
  
  for (int i = 0; i < length; i++) {
    payloadStr+=((char)payload[i]);
  }

  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);

  tape=payloadStr;
  Serial.print("payload: ");
  Serial.println(payloadStr);

  if (topicStr == "/robot/heart"){ 
    
    boolUpdate=true;
    
     if(tape=="snake"){
        SnakeReset();
     }
     else if(tape=="spin"){
        SpinReset();
     }
     else{ //msg, boom, none, heart, music.....
       Clear(); 
    }

  }
}


void reconnect() {

  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
    //debug printing
    Serial.print("Connecting to ");
    Serial.println(ssid);

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    //print out some more debug once connected
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      //if connected, subscribe to the topic(s) we want to be notified about
      //EJ: Delete "mqtt_username", and "mqtt_password" here if you are not using any 
      if (client.connect((char*) clientName.c_str(),"", "")) {  //EJ: Update accordingly with your MQTT account 
        Serial.print("\tMQTT Connected");
        client.subscribe(switchTopic1);

        //EJ: Do not forget to replicate the above line if you will have more than the above number of relay switches
      }

      //otherwise print failed for debugging
      else{Serial.println("\tFailed."); abort();}
    }
  }
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}



void MsgBoard(){

 for ( int i = 0 ; i < width * tape.length() + matrix.width() - 1 - spacer; i++ ) {

    matrix.fillScreen(LOW);

    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while ( x + width - spacer >= 0 && letter >= 0 ) {
      
        if ( letter < tape.length() ) {
          if(boolUpdate!=true){
            matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
          }
        }
      letter--;
      x -= width;
    }

    matrix.write(); // Send bitmap to display
    if(boolUpdate!=true){
      delay(wait);
    }
  } 
}
void Boom(){
   for(int i=9;i--;i>=0){
       matrix.fillScreen(LOW);
       matrix.drawChar(1, 0, i+49, HIGH, LOW, 1);
       matrix.write();
      delay(1000);
   } 
   matrix.fillScreen(HIGH);
   matrix.drawChar(1, 0, 15, LOW, HIGH, 1);
   matrix.write();
   delay(1000);
   matrix.fillScreen(LOW);
   matrix.drawChar(1, 0, 15, HIGH, LOW, 1);
   matrix.write();
   delay(1000);
   matrix.fillScreen(HIGH);
   matrix.drawChar(1, 0, 15, LOW, HIGH, 1);
   matrix.write();
   delay(1000);
   matrix.fillScreen(LOW);
   matrix.drawChar(1, 0, 15, HIGH, LOW, 1);
   matrix.write();
   delay(1000);
   tape="BOOM!";
}
void none(){
   matrix.fillScreen(LOW);
   matrix.write();
   delay(1000);
}
void Music(){
   matrix.fillScreen(HIGH);
   matrix.drawChar(1, 0, 14, LOW, HIGH, 1);
   matrix.write();
   delay(1000);
   matrix.fillScreen(LOW);
   matrix.drawChar(1, 0, 14, HIGH, LOW, 1);
   matrix.write();
   delay(1000);
}
void Dir(int d){
   int x=0;
   switch(d){
      case 1:
      //left
         for(int16_t x=-8;x++;x>=8){
           matrix.fillScreen(LOW);
           matrix.drawChar(x, 0, 175, HIGH, LOW, 1);
           matrix.write();
           delay(100);
         }
      break;
      case 0:
      //right
         for(int16_t x=8;x--;x>=-8){
           matrix.fillScreen(LOW);
           matrix.drawChar(x, 0, 174, HIGH, LOW, 1);
           matrix.write();
           delay(100);
         }
      break;
   } 
}
void Clear(){
   wait = 180;
   matrix.fillScreen(LOW);
}

void Spin(){
  for ( int x = 0; x < matrix.width() - 1; x++ ) {
    matrix.fillScreen(LOW);
    matrix.drawLine(x, 0, matrix.width() - 1 - x, matrix.height() - 1, HIGH);
    matrix.write(); // Send bitmap to display
    delay(wait);
  }

  for ( int y = 0; y < matrix.height() - 1; y++ ) {
    matrix.fillScreen(LOW);
    matrix.drawLine(matrix.width() - 1, y, 0, matrix.height() - 1 - y, HIGH);
    matrix.write(); // Send bitmap to display
    delay(wait);
  }

  wait = wait + inc;
  if ( wait == 0 ) inc = 2;
  if ( wait == 50 ) inc = -2;
}
void Snake(){
  ptr = nextPtr;
  nextPtr = next(ptr);

  matrix.drawPixel(x[ptr], y[ptr], HIGH); // Draw the head of the snake
  matrix.write(); // Send bitmap to display

  delay(wait);

  if ( ! occupied(nextPtr) ) {
    matrix.drawPixel(x[nextPtr], y[nextPtr], LOW); // Remove the tail of the snake
  }

  for ( int attempt = 0; attempt < 10; attempt++ ) {

    // Jump at random one step up, down, left, or right
    switch ( random(4) ) {
    case 0: x[nextPtr] = constrain(x[ptr] + 1, 0, matrix.width() - 1); y[nextPtr] = y[ptr]; break;
    case 1: x[nextPtr] = constrain(x[ptr] - 1, 0, matrix.width() - 1); y[nextPtr] = y[ptr]; break;
    case 2: y[nextPtr] = constrain(y[ptr] + 1, 0, matrix.height() - 1); x[nextPtr] = x[ptr]; break;
    case 3: y[nextPtr] = constrain(y[ptr] - 1, 0, matrix.height() - 1); x[nextPtr] = x[ptr]; break;
    }

    if ( ! occupied(nextPtr) ) {
      break; // The spot is empty, break out the for loop
    }
  }
}
void SpinReset(){
   wait=50;
   inc=-2; 
}
void SnakeReset(){
  wait=100;
  Clear();
  for ( ptr = 0; ptr < length; ptr++ ) {
    x[ptr] = numberOfHorizontalDisplays * 8 / 2;
    y[ptr] = numberOfVerticalDisplays * 8 / 2;
  }
  nextPtr = 0;
  length = 6;
  randomSeed(analogRead(pinRandom)); // Initialize random generator
}
boolean occupied(int ptrA) {
  for ( int ptrB = 0 ; ptrB < length; ptrB++ ) {
    if ( ptrA != ptrB ) {
      if ( equal(ptrA, ptrB) ) {
        return true;
      }
    }
  }

  return false;
}

int next(int ptr) {
  return (ptr + 1) % length;
}

boolean equal(int ptrA, int ptrB) {
  return x[ptrA] == x[ptrB] && y[ptrA] == y[ptrB];
}


void pacman(){
 memset(mBitmap,0,sizeof(mBitmap));
 aniFrames=3;
 mBitmap[0]=B00111100;
 mBitmap[1]=B01111110;
 mBitmap[2]=B11101111;
 mBitmap[3]=B11111111;
 mBitmap[4]=B11111111;
 mBitmap[5]=B11111111;
 mBitmap[6]=B01111110;
 mBitmap[7]=B00111100;
 
 mBitmap[8]=B00111100;
 mBitmap[9]=B01111110;
 mBitmap[10]=B11101111;
 mBitmap[11]=B11111111;
 mBitmap[12]=B11110000;
 mBitmap[13]=B11111111;
 mBitmap[14]=B01111110;
 mBitmap[15]=B00111100;
 
 mBitmap[16]=B00111100;
 mBitmap[17]=B01111110;
 mBitmap[18]=B11101100;
 mBitmap[19]=B11111000;
 mBitmap[20]=B11110000;
 mBitmap[21]=B11111000;
 mBitmap[22]=B01111110;
 mBitmap[23]=B00111100;
 dBitmap();
}
void pinky(){
 memset(mBitmap,0,sizeof(mBitmap));
 aniFrames=2;
 mBitmap[0]=B00111000;
 mBitmap[1]=B01111100;
 mBitmap[2]=B10010010;
 mBitmap[3]=B11011010;
 mBitmap[4]=B11111111;
 mBitmap[5]=B11111111;
 mBitmap[6]=B11111111;
 mBitmap[7]=B10101010;
 
 mBitmap[8]=B00111000;
 mBitmap[9]=B01111100;
 mBitmap[10]=B10010010;
 mBitmap[11]=B10110110;
 mBitmap[12]=B11111111;
 mBitmap[13]=B11111111;
 mBitmap[14]=B11111111;
 mBitmap[15]=B01010101;

 dBitmap();
}
void smile(){
 memset(mBitmap,0,sizeof(mBitmap));
 aniFrames=1;
 mBitmap[0]=B00111100;
 mBitmap[1]=B01000010;
 mBitmap[2]=B10100101;
 mBitmap[3]=B10000001;
 mBitmap[4]=B10100101;
 mBitmap[5]=B10011001;
 mBitmap[6]=B01000010;
 mBitmap[7]=B00111100;

 dBitmap();
}

void skull(){
 memset(mBitmap,0,sizeof(mBitmap));
 aniFrames=6;
 mBitmap[0]=B00111110;
 mBitmap[1]=B01111110;
 mBitmap[2]=B10010011;
 mBitmap[3]=B11011011;
 mBitmap[4]=B11101111;
 mBitmap[5]=B01111111;
 mBitmap[6]=B01111100;
 mBitmap[7]=B01010100;

 mBitmap[8]=B00111110;
 mBitmap[9]=B01111110;
 mBitmap[10]=B11011011;
 mBitmap[11]=B10010011;
 mBitmap[12]=B11101111;
 mBitmap[13]=B01111111;
 mBitmap[14]=B01111100;
 mBitmap[15]=B01010100;

 mBitmap[16]=B00111110;
 mBitmap[17]=B01111110;
 mBitmap[18]=B10110111;
 mBitmap[19]=B10010011;
 mBitmap[20]=B11101111;
 mBitmap[21]=B01111111;
 mBitmap[22]=B01111100;
 mBitmap[23]=B01010100;

 mBitmap[24]=B00111110;
 mBitmap[25]=B01111110;
 mBitmap[26]=B10010011;
 mBitmap[27]=B10110111;
 mBitmap[28]=B11101111;
 mBitmap[29]=B01111111;
 mBitmap[30]=B01111100;
 mBitmap[31]=B01010100;

 mBitmap[32]=B00111110;
 mBitmap[33]=B01111110;
 mBitmap[34]=B10010011;
 mBitmap[35]=B11011011;
 mBitmap[36]=B11101111;
 mBitmap[37]=B01111111;
 mBitmap[38]=B01111100;
 mBitmap[39]=B01010100;
 
 mBitmap[40]=B00111110;
 mBitmap[41]=B01111110;
 mBitmap[42]=B10010011;
 mBitmap[43]=B11011011;
 mBitmap[44]=B11101111;
 mBitmap[45]=B01111111;
 mBitmap[46]=B01111100;
 mBitmap[47]=B01010100;
 
 dBitmap();
}

void heart(){
 memset(mBitmap,0,sizeof(mBitmap));
 aniFrames=3;
 mBitmap[0]=B00000000; 
 mBitmap[1]=B01100110;
 mBitmap[2]=B11111111;
 mBitmap[3]=B11111111;
 mBitmap[4]=B01111110;
 mBitmap[5]=B00111100;
 mBitmap[6]=B00011000;
 mBitmap[7]=B00000000;
 
 mBitmap[8]=B00000000; 
 mBitmap[9]=B00000000;
 mBitmap[10]=B00101000;
 mBitmap[11]=B01111100;
 mBitmap[12]=B00111000;
 mBitmap[13]=B00010000;
 mBitmap[14]=B00000000;
 mBitmap[15]=B00000000;

 mBitmap[16]=B00000000; 
 mBitmap[17]=B01100110;
 mBitmap[18]=B11111111;
 mBitmap[19]=B11111111;
 mBitmap[20]=B01111110;
 mBitmap[21]=B00111100;
 mBitmap[22]=B00011000;
 mBitmap[23]=B00000000;
 
 dBitmap();
}
void right(){
 memset(mBitmap,0,sizeof(mBitmap));
 aniFrames=8;
 mBitmap[0]=B11001000; 
 mBitmap[1]=B01100100;
 mBitmap[2]=B00110010;
 mBitmap[3]=B00011001;
 mBitmap[4]=B00011001 ;
 mBitmap[5]=B00110010;
 mBitmap[6]=B01100100;
 mBitmap[7]=B11001000;

 mBitmap[8]=B01100100; 
 mBitmap[9]=B00110010;
 mBitmap[10]=B00011001;
 mBitmap[11]=B10001100;
 mBitmap[12]=B10001100;
 mBitmap[13]=B00011001;
 mBitmap[14]=B00110010;
 mBitmap[15]=B01100100;

 mBitmap[16]=B00110010; 
 mBitmap[17]=B00011001;
 mBitmap[18]=B10001100;
 mBitmap[19]=B01000110;
 mBitmap[20]=B01000110;
 mBitmap[21]=B10001100;
 mBitmap[22]=B00011001;
 mBitmap[23]=B00110010;
 
 mBitmap[24]=B00011001; 
 mBitmap[25]=B10001100;
 mBitmap[26]=B01000110;
 mBitmap[27]=B00100011;
 mBitmap[28]=B00100011;
 mBitmap[29]=B01000110;
 mBitmap[30]=B10001100;
 mBitmap[31]=B00011001;
 
 mBitmap[32]=B10001100; 
 mBitmap[33]=B01000110;
 mBitmap[34]=B00100011;
 mBitmap[35]=B10010001;
 mBitmap[36]=B10010001;
 mBitmap[37]=B00100011;
 mBitmap[38]=B01000110;
 mBitmap[39]=B10001100;
 
 mBitmap[40]=B01000110; 
 mBitmap[41]=B00100011;
 mBitmap[42]=B10010001;
 mBitmap[43]=B11001000;
 mBitmap[44]=B11001000;
 mBitmap[45]=B10010001;
 mBitmap[46]=B00100011;
 mBitmap[47]=B01000110;
 
 mBitmap[48]=B00100011; 
 mBitmap[49]=B10010001;
 mBitmap[50]=B11001000;
 mBitmap[51]=B01100100;
 mBitmap[52]=B01100100;
 mBitmap[53]=B11001000;
 mBitmap[54]=B10010001;
 mBitmap[55]=B00100011;
  
 mBitmap[56]=B10010001; 
 mBitmap[57]=B11001000;
 mBitmap[58]=B01100100;
 mBitmap[59]=B00110010;
 mBitmap[60]=B00110010;
 mBitmap[61]=B01100100;
 mBitmap[62]=B11001000;
 mBitmap[63]=B10010001;
 
 dBitmap();
}

void left(){
 memset(mBitmap,0,sizeof(mBitmap));
 aniFrames=8;
 mBitmap[0]=B00010011;
 mBitmap[1]=B00100110;
 mBitmap[2]=B01001100;
 mBitmap[3]=B10011000;
 mBitmap[4]=B10011000;
 mBitmap[5]=B01001100;
 mBitmap[6]=B00100110;
 mBitmap[7]=B00010011;

 mBitmap[8]=B00100110;
 mBitmap[9]=B01001100;
 mBitmap[10]=B10011000;
 mBitmap[11]=B00110001;
 mBitmap[12]=B00110001;
 mBitmap[13]=B10011000;
 mBitmap[14]=B01001100;
 mBitmap[15]=B00100110;

 mBitmap[16]=B01001100;
 mBitmap[17]=B10011000;
 mBitmap[18]=B00110001;
 mBitmap[19]=B01100010;
 mBitmap[20]=B01100010;
 mBitmap[21]=B00110001;
 mBitmap[22]=B10011000;
 mBitmap[23]=B01001100;
 
 mBitmap[24]=B10011000;
 mBitmap[25]=B00110001;
 mBitmap[26]=B01100010;
 mBitmap[27]=B11000100;
 mBitmap[28]=B11000100;
 mBitmap[29]=B01100010;
 mBitmap[30]=B00110001;
 mBitmap[31]=B10011000;
 
 mBitmap[32]=B00110001;
 mBitmap[33]=B01100010;
 mBitmap[34]=B11000100;
 mBitmap[35]=B10001001;
 mBitmap[36]=B10001001;
 mBitmap[37]=B11000100;
 mBitmap[38]=B01100010;
 mBitmap[39]=B00110001;
 
 mBitmap[40]=B01100010;
 mBitmap[41]=B11000100;
 mBitmap[42]=B10001001;
 mBitmap[43]=B00010011;
 mBitmap[44]=B00010011;
 mBitmap[45]=B10001001;
 mBitmap[46]=B11000100;
 mBitmap[47]=B01100010;
 
 mBitmap[48]=B11000100;
 mBitmap[49]=B10001001;
 mBitmap[50]=B00010011;
 mBitmap[51]=B00100110;
 mBitmap[52]=B00100110;
 mBitmap[53]=B00010011;
 mBitmap[54]=B10001001;
 mBitmap[55]=B11000100;
  
 mBitmap[56]=B10001001;
 mBitmap[57]=B00010011;
 mBitmap[58]=B00100110;
 mBitmap[59]=B01001100;
 mBitmap[60]=B01001100;
 mBitmap[61]=B00100110;
 mBitmap[62]=B00010011;
 mBitmap[63]=B10001001;
 
 dBitmap();
}


void dBitmap(){
  int offset=0;
  for(offset=0;offset<aniFrames*8;offset+=8){
    for(int i=0;i<8;i++){
      byte mask = 1; //our bitmask
      int b=0;
      for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
        if (mBitmap[i+offset] & mask){ // if bitwise AND resolves to true
          matrix.drawPixel(b,i, HIGH);
        }
        else{ //if bitwise and resolves to false
           matrix.drawPixel(b,i, LOW);
        }
        b++;
      }
    }
    matrix.write();
    delay(wait);
  }
}
