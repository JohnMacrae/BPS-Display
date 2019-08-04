#include <WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>


/* change it with your ssid-password */
const char* ssid = "agboat";
const char* password = "jram7757";

#define CELL1    "bps/cell1"
#define CELL2    "bps/cell2"
#define CELL3    "bps/cell3"
#define CELL4    "bps/cell4"
#define BANK     "bps/bank"
#define DELTA    "bps/delta"
#define HICELL   "bps/hicell"
#define LOCELL   "bps/locell"
#define CELLSUM  "bps/cellsum"

#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

int normal = 0;
int chirp = 2;
int lowchirp = 1;
int highchirp = 3;

const int highcut = 30;
const int lowcut = 20;
const int highon = 31;
const int lowon = 21;


// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 5, 4);
// SH1106 display(0x3c, D3, D5);

/* this is the IP of PC/raspberry where you installed MQTT Server 
on Wins use "ipconfig" 
on Linux use "ifconfig" to get its IP address */
const char* mqtt_server = "192.168.2.10";

WiFiClient espClient;
PubSubClient client(espClient);

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);
  char str[20]{0};
  char msg[6]{0};
 
    for (int i = 0; i < length; i++) {
      msg[i]=(char)payload[i];
//     Serial.print((char)payload[i]);
//     Serial.print((char)msg[i]);
    }
//  Serial.println();
//  Serial.println(msg);
  float voltage = atof(msg)/1000;

  //display.setFont(ArialMT_Plain_10);
  display.setFont(Lato_Regular_12);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  Serial.print("payload: ");
  
  if(strcmp(topic,CELL1)==0){
    sprintf(str, "Cell1: %7.3f\n",voltage);
    Serial.print(str);
    display.clear();
    display.normalDisplay();
    display.drawString(0, 12, str);
  }
  //display.display();
  if(strcmp(topic,CELL2)==0){
    //sprintf(str, "Cell2: %6.3f\n",voltage);
    sprintf(str, "2: %6.3f\n",voltage);
    Serial.print(str);
    display.drawString(80, 12, str);
  }
  //display.display();
  if(strcmp(topic,CELL3)==0){
    sprintf(str, "Cell3: %7.3f\n",voltage);
    Serial.print(str);
    display.drawString(0, 24, str);
  }
  //display.display();
    if(strcmp(topic,CELL4)==0){
    sprintf(str, "4: %6.3f\n",voltage);
    Serial.print(str);
    //display.drawString(0, 36, str);
    display.drawString(80, 24, str);

  }
  //display.display();
  if(strcmp(topic,BANK)==0){
    sprintf(str, "Bank: %6.3f\n",voltage);
    Serial.print(str);
    display.drawString(0, 0, str);
  }
  //display.display();
  if(strcmp(topic,DELTA)==0){
    sprintf(str, "MD: %s",msg);
    Serial.print(str);
    display.drawString(80, 0, str);
  }  
  if(strcmp(topic,"alarmstatus")>=0){
     /*
   * alarmstate = 0 do nothing
   * alarmstate = 1 chirp
   * alarmstate = 10 high alarm and disconnect
   * alarmstate = 20 low alarm and disconnect
   */
   int status = atoi(msg);
    sprintf(str, "ALARM: %1.0f\n",status);
    Serial.print(str);
    display.drawString(0, 36, str);
  }
  if(strcmp(topic,"alarmstatus")==0){
   if(voltage !=0){
      display.invertDisplay();
     
    }
  }
  display.display();
  str[0]=0;
}

void mqttconnect() {
  /* Loop until reconnected */
  int count=0;
  while (!client.connected()) {
    Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "ESP32Display";
    /* connect now */
    if (client.connect(clientId.c_str(), "mosquitto", "jram7757")) {
      Serial.println("connected");
      /* subscribe topic with default QoS 0*/
      client.subscribe("bps/bank");
      client.subscribe(CELL1);
      client.subscribe(CELL2);
      client.subscribe(CELL3);
      client.subscribe(CELL4);
      client.subscribe(DELTA);
      client.subscribe("alarmstatus");
      client.publish("outTopic", "display");
      Serial.print("Subscribed");
    } else {
//      Serial.print("failed, status code =");
//      Serial.print(client.state());
//      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      count++;
    if(count > 20){
      ESP.restart();
    }
      delay(100);
    }
  }
}

void pause(int duration){
  for(int i=1;i < duration; i++){
    yield();
    delay(1000);
  }
}

void setup(void) 
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
    //WiFi.disconnect();
    
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  Serial.print(WiFi.status());
    delay(500);
  Serial.print("+");
  }
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  /* configure the MQTT server with IPaddress and port */
  client.setServer(mqtt_server, 1883);
  /* this receivedCallback function will be invoked 
  when client received subscribed topic */
  client.setCallback(receivedCallback);
  
  mqttconnect();
  
  display.init();
  display.flipScreenVertically();
  display.setContrast(255);

  
    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("BPSensorDisplay");

    ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
 //    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
//    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
//    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
//  Serial.println("Ready");
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());

}

void loop(void) 
{
//  display.clear();
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("+");
    WiFi.begin(ssid, password);
    delay(500);
    count++;
    if(count > 20){
      ESP.restart();
    }
  } 
   ArduinoOTA.handle();

    if (!client.connected()) {
    mqttconnect();
  }
  client.loop();
  
}
