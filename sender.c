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
#define sf 8
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

WiFiClient espClient;
PubSubClient client(espClient);

union pack
  {
    uint8_t frame[16]; // trames avec octets
    float data; // 4 valeurs en virgule flottante
  } sdp ; // paquet d’émission

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

}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrivé dans le topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}


void loop() {
  //Partie MQTT
  param = "param;jvas;868E6;7;125E3";
  // param = "param;" + freq + ";" + sf + ";" + sb;
  // snprintf(param, sizeof(param), "param;%s;%s;%s;%s", groupe, freq, sf, sb );

  client.publish(topic, param);
  Serial.println("Message envoyé");
 
  //émetteur LoRa

  Serial.printf("\nNouveau Paquet : %f\n",d1) ;
  LoRa.beginPacket(); // start packet
    sdp.data=d1;
    LoRa.write(sdp.frame,16);
  LoRa.endPacket();
  d1++;
  

  client.loop();
  delay(5000);
}
