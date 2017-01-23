#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <SimpleDHT.h>

 
//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "192.168.0.106"
#define MQTT_SERVER_WAN "idirect.dlinkddns.com"

int pinDHT11 = 14;
SimpleDHT11 dht11;


byte temperature = 0;
byte humidity = 0;

//LED on ESP8266 GPIO2
const int light1= 0;
const int light2= 2;
const int int1= 12; // interruptor 1
//const int int2= 14;  // interruptor 2


// WIFI CASA
const char* ssid = "Red Virtual 2";
const char* password = "2410meridian";

/// WIFI ETB

const char* ssid_1 = "Consola";
const char* password_1 = "tyrrenal";



char* lightTopic = "prueba/light1";
char* lightTopic2 = "prueba/light2";

/////////// antirebote /////////////
volatile int contador = 0;   // Somos de lo mas obedientes
int n = contador ;
long T0 = 0 ;  // Variable global para tiempo

void ServicioBoton()
   {
       if ( millis() > T0  + 250)
          {   contador++ ;
              T0 = millis();
          }
    }

///////////////// fin antirebote //////////////////


void toggle1() {
 // detachInterrupt(int1);
  static int state = 0;
  state = !state;
  digitalWrite(light1, state);
//  attachInterrupt(int1, toggle1, CHANGE);
}

void toggle2() {
 // detachInterrupt(int2);
  static int state = 0;
  state = !state;
  digitalWrite(light2, state);
 // attachInterrupt(int2, toggle2, CHANGE);
}



WiFiClient wifiClient;
WiFiClientSecure wifiClientSecure;

PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {
 

  
  //initialize the light as an output and set to LOW (off)
  pinMode(light1, OUTPUT);
  pinMode(light2, OUTPUT);
  
  pinMode(int1, INPUT);
//  pinMode(int2, INPUT);
  
  digitalWrite(light1, HIGH);
  digitalWrite(light2, LOW);
  
  delay(200);
  //start the serial line for debugging
  Serial.begin(115200);
   //start wifi subsystem
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();

  
  attachInterrupt(int1, toggle1, CHANGE);
 // attachInterrupt(int2, ServicioBoton, CHANGE);
  //wait a bit before starting the main loop
  delay(250);
 
 if (dht11.read(pinDHT11, &temperature, &humidity, NULL)) {
    Serial.print("Read DHT11 failed.");
  }
  
  Serial.print((int)temperature); Serial.println(" *C, "); 
  Serial.print((int)humidity); Serial.println(" %");

  client.publish("prueba/light1/confirm", "Light1 On");

  
}


void loop(){

  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {reconnect();}

  //maintain MQTT connection
  client.loop();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10); 
}


void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic; 

  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);

  

  //turn the light on if the payload is '1' and publish to the MQTT server a confirmation message
  if(payload[0] == '1'){
    digitalWrite(light1, HIGH);
    client.publish("prueba/light1/confirm", "Light1 On");

  }

  //turn the light off if the payload is '0' and publish to the MQTT server a confirmation message
  else if (payload[0] == '0'){
    digitalWrite(light1, LOW);
    client.publish("prueba/light1/confirm", "Light1 Off");
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
      Serial.println("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
       Serial.println(clientName);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str(),"diego","24305314")){
        Serial.println("MTQQ Connected");
        client.subscribe(lightTopic);
        client.subscribe(lightTopic2);
      }

      //otherwise print failed for debugging
      else{Serial.println("Failed."); abort();}
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
