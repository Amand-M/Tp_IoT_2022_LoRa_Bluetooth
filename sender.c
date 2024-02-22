#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SPI.h> // include libraries
#include <LoRa.h>
#include <U8x8lib.h>

#define SCK 5 // GPIO5 -- SX127x's SCK
#define MISO 19 // GPIO19 -- SX127x's MISO
#define MOSI 27 // GPIO27 -- SX127x's MOSI
#define SS 18 // GPIO18 -- SX127x's CS
#define RST 14 // GPIO14 -- SX127x's RESET
#define DI0 26 // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define freq 868E6
#define sf 7
#define sb 125E3

//Display

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15,4,16); // clock, data, reset

// WiFi
const char *ssid = "raspi-webgui"; // Enter your Wi-Fi name
const char *password = "RASPI_TP";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "10.3.141.205";
const char *topic = "srt/jvas";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

//constantes MQTT
const char *groupe = "jvas";
// const char *freq = "868E6";
// const char *sf = "7";
// const char *sb = "125";
char *param = "";

//Constantes LoRa
float d1 = 0;
float d2 = 0;
float rssi = 0;

WiFiClient espClient;
PubSubClient client(espClient);

union pack
  {
    uint8_t frame[16]; // trames avec octets
    float data[4]; // 4 valeurs en virgule flottante
  } rdp;

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(9600);

  //Display
  u8x8.begin(); // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  // Connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connection au WiFi...");
  }
  Serial.println("\n\nConnecté au Wi-Fi.");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "erwanamand";
      Serial.printf("\nConnection du client %s en cours...\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Broker MQTT connecté.");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // Publish and subscribe
  client.subscribe(topic);

  //setup LoRa

  pinMode(DI0, INPUT); // signal interrupt
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);

  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth (sb);
  //LoRa.onReceive(onReceive); // pour indiquer la fonction ISR
  //LoRa.receive(); // pour activer l'interruption (une fois)

}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrivé dans le topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

void mqtt5sec(){
  int time;
  time = millis() + 5000;
  param = "868000000;7;125000";

  while(1)
  {  
    if(millis()>time){
    u8x8.clear();

    client.publish(topic, param);
    Serial.println("Message MQTT envoyé");
    u8x8.drawString(0,1,"Message MQTT :");
    u8x8.drawString(0,2,param);

    time = millis() + 5000;
    }  
  };

}

void loop() {
 
  //émetteur LoRa

  // Serial.printf("\nNouveau Paquet LoRa envoyé : %f\n",d1) ;
  // u8x8.drawString(0,3,"Message LoRa :");
  // // u8x8.drawString(0,4,d1);
  // LoRa.beginPacket(); // start packet
  //   sdp.data=d1;
  //   LoRa.write(sdp.frame,16);
  // LoRa.endPacket();
  // d1++;

  int packetLen;
  packetLen=LoRa.parsePacket();

  if(packetLen>0) {
    int i=0;
    while (LoRa.available()) {
      rdp.frame[i]=LoRa.read();i++;
    }
    d1=rdp.data[0];
    d2=rdp.data[1];

    rssi=LoRa.packetRssi(); // force du signal en réception en dB
    Serial.println(d1); Serial.println(d2);
    Serial.printf("RSSI : ");
    Serial.println(rssi);
  }

  client.loop();
}
