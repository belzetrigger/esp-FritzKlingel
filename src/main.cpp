/*********************************************************************
 * 
 * Note:
 * Wemos D1 Mini has already build in divider R1 220k/ R2 100k for pin A0.
 * 
 * Make sure you have set up 
 *    user_config.h
 *    user_config_override.h
 * to your needs.
 * 
 * Features:
 *  * call tr064
 *  * mqtt send simple mqtt msg on bell
 *  * domoticz
 *    * idx: 
 *    * battery
 *    * rssi
 * 
 *
 **********************************************************************/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "user_config.h"

#if FB_TR64_USE == 1
#include <tr064.h>
#endif

#if MQTT_USE == 1
#include <PubSubClient.h>
#endif

#include "common.h"

/* wifi settings */
const char WIFI_SSID[] = STA_SSID1;
const char WIFI_PASSWORD[] = STA_PASS1;

/* fritz box settings */
const char USER[] = FB_TR64_USER;
const char PASSWORD[] = FB_TR64_PASSWORD;

const char FRITZBOX_IP[] = FB_IP;
const int FRITZBOX_PORT = FB_TR64_PORT;

/* mqtt settigns
see in user_config_override
*/

#if FB_TR64_USE == 1
TR064 tr064_connection(FRITZBOX_PORT, FRITZBOX_IP, USER, PASSWORD);
const String tr064_service = "urn:dslforum-org:service:X_VoIP:1";

#endif



bool calling = false;

const char DEVICE_NAME[] = ESP_DEVICE_NAME;

/**
 * @brief  just write to stdout with same log level checkings
 * @note   
 * @param  prefix[]:  prefix
 * @param  msg[]:  the text
 * @param  level:  level, when to print out
 * @retval None
 */
void logTo ( const char prefix[], const char msg[], LoggingLevels level )
{
  //TODO use F() / PSTR()
  if(SERIAL_LOG_LEVEL >= level){
    Serial.print("BLZ#");
    Serial.print(prefix);
    Serial.print(":");
    Serial.println(msg);
  }
}
/**
 * @brief  just write to stdout to debug
 * @note   
 * @param  prefix[]:  prefix
 * @param  msg[]:  the text
 * @retval None
 */
void logDebug(const char prefix[], const char msg[])
{
  //TODO add debug prefix. 
  //char * b = "Debug:";
  //char *c = malloc(strlen(prefix)+strlen(b)+1);
  //strcpy(c,b);
  //strcat(c,prefix);
  logTo(prefix, msg, LOG_LEVEL_DEBUG );
  /*
#if SERIAL_LOG_LEVEL >= LOG_LEVEL_INFO
  Serial.print("BLZ#Debug#");
  Serial.print(prefix);
  Serial.print(":");
  Serial.println(msg);
#endif*/
}

void setupMqtt()
{
#if MQTT_USE == 1
  mqttclient.setServer(MQTT_HOST, MQTT_PORT);
  logDebug("mqttServer", MQTT_HOST);
#endif
}

/**
 * @brief  initiate wifi connection 
 * @retval None
 */
void setupWifi()
{

 // just using static IP, to save time usually used for DHCP-Handling 
  IPAddress STATIC_IP;
  STATIC_IP.fromString(WIFI_IP_ADDRESS);
  //(192, 168, *, 230);
  IPAddress GATEWAY;
  GATEWAY.fromString(WIFI_GATEWAY); //(192, 168, *, 1);
  IPAddress SUBNET;
  SUBNET.fromString(WIFI_SUBNETMASK); //(255, 255, 255, 0);
  IPAddress DNS;
  DNS.fromString(WIFI_DNS);

  WiFi.hostname(DEVICE_NAME);
  WiFi.config(STATIC_IP, SUBNET, GATEWAY, DNS);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);

  logDebug("wifi:ip", STATIC_IP.toString().c_str());
}

/**
 * @brief  inits connection to fritz box and starts calling. 
 * if calling again, call will be canceld
 * @note   
 * @retval true -> call is running, false -> call is stopped
 */
bool tr064Call()
{
#if FB_TR64_USE == 1

  if (calling == false)
  {
    logDebug("tr064:call", "start");
    tr064_connection.init();
    // Die Telefonnummer **9 ist der Fritzbox-Rundruf.
    String call_params[][2] = {{"NewX_AVM-DE_PhoneNumber", FB_TR64_NR}}; //FB_TR64_NR
    tr064_connection.action(tr064_service, "X_AVM-DE_DialNumber", call_params, 1);
    calling = true;
  }
  else
  {
    calling = false;
    tr064_connection.action(tr064_service, "X_AVM-DE_DialHangup");
    logDebug("tr064:call", "end");
  }
#endif
  return calling;
}

/**
 * @brief  send mqtt if enabled, also sends domoticz if enabled as well.
 * @note   
 * @retval None
 */
void sendMqtt()
{
  if (MQTT_USE == 1)
  {
      mqttclient.connect(DEVICE_NAME, MQTT_USER, MQTT_PASS);
  if (USE_DOMOTICZ == 1)
  {
    
    int rssi = DomoticzRssiQuality(WiFi.RSSI());
    int battery = DomoticzBatteryQuality();
    // * get battery
    // * get meaning full content
    // DomoticzSensor(1,"Ring Ring", 2, 2);
    String v = String(volt); // change float into string
    mqttclient.publish(DOMOTICZ_IN_TOPIC, v.c_str());
    mqttclient.publish(DOMOTICZ_IN_TOPIC, String(analogRaw).c_str());
    logDebug(":SendDomoticz", v.c_str());
    DomoticzSensor(DOMOTICZ_FB_BELL_IDX, DOMOTICZ_FB_BELL_MSG, battery, rssi);
  }
  else
  {
    logDebug(":mqtt", DOMOTICZ_FB_BELL_MSG);
    mqttclient.publish(MQTT_FULLTOPIC, DOMOTICZ_FB_BELL_MSG);
  }
  }else{
    logDebug("mqtt", "turned off");
  }
}

/**
 * @brief  read in analog value and try to calculate voltage
 * @note   
 * @retval None
 */
float getEnergyA0()
{

  pinMode(A0, INPUT);
  analogRaw = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 4.2V):
  volt = analogRaw / 1023.0;
  volt = volt * 4.2;
  logDebug("AnalogRaw", String(analogRaw).c_str());
  logDebug("Voltage", String(volt).c_str());
  volt;
}

void setup()
{

  Serial.begin(74880);

#if SERIAL_LOG_LEVEL >= LOG_LEVEL_INFO
  //  Serial.begin(115200);
  Serial.begin(74880);
  
  logDebug("Debugmode", "is ON");
  char cstr[1];
  itoa(MQTT_USE, cstr, 10);
  logDebug("mqttEnabled", cstr);
#endif
  setupMqtt();

  setupWifi();

  getEnergyA0();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(50);
  }
  // init and call
  tr064Call();

  // mqtt
  sendMqtt();

  // wait a bit ...
  if(FB_TR64_CALL_DURATION >= 1000)
  {
    delay(FB_TR64_CALL_DURATION);
  }else{
    logTo("Wrong time for call - must be >= 1000, was ", String(FB_TR64_CALL_DURATION).c_str(), LOG_LEVEL_INFO );
    delay(4000);
  }
  // .. and hang up
  tr064Call();
  //tr064_connection.action(tr064_service, "X_AVM-DE_DialHangup");

#if SERIAL_LOG_LEVEL >= LOG_LEVEL_INFO
  Serial.println("BLZ#go sleeping ....");
#endif
  ESP.deepSleep(0);
}

void loop()
{
  // do nothing ...
  // ... as esp should sleep not loop :)
}
