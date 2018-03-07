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
char const* switchTopic1 = "/robot/mouth";

int pinCS = D4; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
// int pinCLK=5;
// int pinDIN=7;
int numberOfHorizontalDisplays = 3;
int numberOfVerticalDisplays = 1;



Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

static const uint8_t PROGMEM // Bitmaps are stored in program memory
  mouthImg[][24] = {                 // Mouth animation frames
  { B00000000, B00000000, B00000000, // Mouth position A
    B00000000, B00000000, B00000000,
    B01111111, B11111111, B11111110,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position B
    B00000000, B00000000, B00000000,
    B00111111, B11111111, B11111100,
    B00000111, B00000000, B11100000,
    B00000000, B11111111, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position C
    B00000000, B00000000, B00000000,
    B00111111, B11111111, B11111100,
    B00001000, B00000000, B00010000,
    B00000110, B00000000, B01100000,
    B00000001, B11000011, B10000000,
    B00000000, B00111100, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position D
    B00000000, B00000000, B00000000,
    B00111111, B11111111, B11111100,
    B00100000, B00000000, B00000100,
    B00010000, B00000000, B00001000,
    B00001100, B00000000, B00110000,
    B00000011, B10000001, B11000000,
    B00000000, B01111110, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position E
    B00000000, B00111100, B00000000,
    B00011111, B11000011, B11111000,
    B00000011, B10000001, B11000000,
    B00000000, B01111110, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00111100, B00000000, // Mouth position F
    B00000000, B11000011, B00000000,
    B00001111, B00000000, B11110000,
    B00000001, B00000000, B10000000,
    B00000000, B11000011, B00000000,
    B00000000, B00111100, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 } };

int seconds=0;
boolean talking=false;
int command=0;

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {
  matrix.setIntensity(0);
  matrix.setRotation(0, 2);
  matrix.setRotation(1, 2);
  matrix.setRotation(2, 2);
  //randomSeed(analogRead(A0));

  drawMouth(mouthImg[2]);

  ArduinoOTA.setHostname("robotmouth"); // A name given to your ESP8266 module when discovering it as a port in ARDUINO IDE
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
  
  if (talking==true){
    drawMouth(mouthImg[random(6)]);
    delay(200);
  }

}

void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic; 
  //EJ: Note:  the "topic" value gets overwritten everytime it receives confirmation (callback) message from MQTT

  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);

   if (topicStr == "/robot/mouth"){
     if(payload[0] == '1'){
       drawMouth(mouthImg[2]);
       talking=false;
       Serial.println("normal");     
       //client.publish("/house/switchConfirm1/", "1");
     }
     else if (payload[0] == '2'){
       Serial.println("talking");
       talking=true;
       //client.publish("/house/switchConfirm1/", "0");
     }else if(payload[0] == '0'){
        Serial.println("quiet");
        matrix.fillScreen(LOW);
        matrix.write();
        talking=false;
        //client.publish("/house/switchConfirm1/", "0");
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


void drawMouth(const uint8_t *img) {
  matrix.fillScreen(LOW);  
  matrix.drawBitmap(0, 0, img, 24, 8, HIGH);
  matrix.write(); // Send bitmap to display
}
