/*********************************************************************
 * to make life easier and share with other projects some common functions
 * - a few functions are from tasmota
 * - a few own one
 * 
 *********************************************************************/

unsigned int analogRaw=0;                         // raw value on anlog pin
float volt=0.0;                                   // analog value converted to volt
#define VMIN 2.6                                  // used to calculate battery level: Vmin = 2.6 LiIon empty
#define VMAX 4.2                                  // used to calculate battery level : Vmax = 4.2 volt LiIon full

#if MQTT_USE
// IPAddress addr;
WiFiClient espClient;
PubSubClient mqttclient(espClient);
#endif
/**
 * @brief  log levels
 *
 */
enum LoggingLevels {LOG_LEVEL_NONE, LOG_LEVEL_ERROR, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, LOG_LEVEL_DEBUG_MORE};


char mqtt_data[893]; // stores data for mqtt

/**
 * @brief  
 * @note   from tasmota
 * @param  format: 
 * @retval 
 */
int Response_P(const char* format, ...)        // Content send snprintf_P char data
{
  // This uses char strings. Be aware of sending %% if % is needed
  va_list args;
  va_start(args, format);
  int len = vsnprintf_P(mqtt_data, sizeof(mqtt_data), format, args);
  va_end(args);
  return len;
}


/**
 * @brief  takes rssi value and convert it into level
 * @note   from tasmota
 * @param  rssi: 
 * @retval 
 */
int WifiGetRssiAsQuality(int rssi)
{
  int quality = 0;

  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }
  return quality;
}

/**
 * @brief  
 * @note   
 * @param  rssi:  WiFi.RSSI()
 * @retval 
 */
int DomoticzRssiQuality(int32 rssi)
{
  // RSSI range: 0% to 10% (12 means disable RSSI in Domoticz)

  return WifiGetRssiAsQuality(rssi) / 10;
}

/**
 * @brief  calculate battery level based on volt 
 * @note   
 * @retval 
 */
int DomoticzBatteryQuality(void)
{
  // Battery 0%: ESP 2.6V (minimum operating voltage is 2.5)
  // Battery 100%: ESP 3.6V (maximum operating voltage is 3.6)
  // Battery 101% to 200%: ESP over 3.6V (means over maximum operating voltage)

  int quality = 100;	// Voltage range from 2,6V > 0%  to 3,6V > 100%

   quality = static_cast<int>(((volt-VMIN)/(VMAX-VMIN))*100.);
   if(quality > 100){
     quality = 100;
   }else if (quality < 1)
   {
     quality = 0;
   }
   
   Serial.print("Battery percent: "); Serial.print(quality); Serial.println(" %");  
  return quality;
}

/**
 * @brief  is using mqtt_data
 * @note   
 * @param  topic: used to indicate where the msg belongs too
 * @param  retained: 
 * @retval 
 */
bool MqttPublish(const char* topic, bool retained)
{
  bool result = false;
#if MQTT_USE == 1
  result = mqttclient.publish(topic, mqtt_data, retained);
  yield();  // #3313
#endif  
  return result;
}

/**
 * @brief  update a domoticz sensor 
 * @note   (from tasmota)
 * @param  idx: the device index in domoticz
 * @param  *data: data to send
 * @param  battery: battery level!
 * @param  wifiRssi: status of rssi
 * @retval None
 */
void DomoticzSensor(uint8_t idx, char *data, int battery, int wifiRssi)
{
  char dmess[128]; // {"idx":26700,"nvalue":0,"svalue":"22330.1;10234.4;22000.5;10243.4;1006;3000","Battery":100,"RSSI":10}
  Response_P(PSTR("{\"idx\":%d,\"nvalue\":%d,\"svalue\":\"%s\",\"Battery\":%d,\"RSSI\":%d}"),
             idx,0, data, battery, wifiRssi);

  MqttPublish(DOMOTICZ_IN_TOPIC,false);
  //memcpy(mqtt_data, dmess, sizeof(dmess));
}