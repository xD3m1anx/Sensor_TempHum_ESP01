//#define BLYNK_PRINT Debug
#define OTA_PRINT   Debug

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>
#include <DHTesp.h>
#include "mydebug.h"
#include "esp8266gpio.h"

/* --------------------------------------------------------------------------------------------------------------- */

#define WIFI_CONNECT_HOME
#define OTA_PGM
#define BUILTIN_LED_ON
#define TELNET_DEBUG

#define OTA_HOSTNAME        "Temp and Hum.temp"
#define HOST_NAME           "temphumsens"
#define BLYNK_SERVER_IP     "192.168.0.50"
#define BLYNK_AUTH_TOKEN    "x6dy_P0g6iT9GNlRs1xRwSTYaHFDILu8"      //192.168.0.61

#define DEBUG_MSG_INTERVAL  (15000)                                 //in ms 

#define DHT_DATA_PIN        GPIO2

#define BPIN_UPTIME         V0
#define BPIN_HUMIDITY       V10
#define BPIN_TEMPERATURE    V11

/* --------------------------------------------------------------------------------------------------------------- */

void ledBuiltinBlink(uint16 d);
void localUptime(void);

/* --------------------------------------------------------------------------------------------------------------- */

#ifdef BUILTIN_LED_ON
  #define BLINK_BUILTIN(__delay__)    ledBuiltinBlink(__delay__)
#else
  #define BLINK_BUILTIN(any)     ;
  #pragma "Builtin led is off. To led on define 'BUILTIN_LED_ON'"  
#endif

/* --------------------------------------------------------------------------------------------------------------- */

const char* ssid = "IrMa";  
const char* password = "4045041990";

DHTesp sensorDht;
RemoteDebug Debug;
BlynkTimer timer_Uptime;

/* --------------------------------------------------------------------------------------------------------------- */

void setup() {
  /* --- Wifi support --- */
  #ifdef WIFI_CONNECT_HOME
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        BLINK_BUILTIN(250);
        WiFi.begin(ssid, password);
        //Serial.println("Retrying connection...");
    }
    //IPAddress localIp = WiFi.localIP();
    //Serial.print("Start network with IP: ");
    //Serial.println(localIp);
  #endif

  /* --- OTA programming (working with WiFi support) --- */
  #ifdef OTA_PGM
    //OTA_PRINT.print("Starting OTA with HOSTNAME: ");
    //OTA_PRINT.print(OTA_HOSTNAME);
    //OTA_PRINT.print("...");
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.onStart([]() {
          //OTA_PRINT.println("Start updating ");
          BLINK_BUILTIN(0);
        }
    );
    ArduinoOTA.onEnd([]() {
          //OTA_PRINT.println("\nEnd");          
        }
    );
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        //OTA_PRINT.printf("Progress: %u%%\r", (progress / (total / 100)));
        BLINK_BUILTIN(0);
        }
    );
    ArduinoOTA.onError([](ota_error_t error) {
          //OTA_PRINT.printf("Error[%u]: ", error);
          if (error == OTA_AUTH_ERROR) {
            //OTA_PRINT.println("Auth Failed");
          } else if (error == OTA_BEGIN_ERROR) {
            //OTA_PRINT.println("Begin Failed");
          } else if (error == OTA_CONNECT_ERROR) {
            //OTA_PRINT.println("Connect Failed");
          } else if (error == OTA_RECEIVE_ERROR) {
            //OTA_PRINT.println("Receive Failed");
          } else if (error == OTA_END_ERROR) {
            //OTA_PRINT.println("End Failed");
          }
        }
    );
    ArduinoOTA.begin();
    //Debug.println("done.");
    BLINK_BUILTIN(1000);
  #endif

  	/* --- RemoteDebug --- */
  #ifdef TELNET_DEBUG
    Debug.begin(HOST_NAME); // Initialize the WiFi server
    Debug.setResetCmdEnabled(true); // Enable the reset command
    Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
    Debug.showColors(true); // Colors

    String hostNameWifi = HOST_NAME;
  hostNameWifi.concat(".local");

  #ifdef ESP8266 // Only for it
    WiFi.hostname(hostNameWifi);
  #endif

  #ifdef USE_MDNS  // Use the MDNS ?
    /*if (MDNS.begin(HOST_NAME)) {
        Debug.print("* MDNS responder started. Hostname -> ");
        Debug.println(HOST_NAME);
    }*/
    MDNS.begin(HOST_NAME);
    MDNS.addService("telnet", "tcp", 23);

  #endif
  #endif


  sensorDht.setup(DHT_DATA_PIN, DHTesp::DHT11);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password, BLYNK_SERVER_IP, 8080);
  timer_Uptime.setInterval(1000UL, localUptime);
}

/* --------------------------------------------------------------------------------------------------------------- */
float humidity, temperature;
uint32_t mLastTime;

void loop() {
  if(millis() - mLastTime >= DEBUG_MSG_INTERVAL) {
    humidity = sensorDht.getHumidity();
    temperature = sensorDht.getTemperature();
    Debug.print("Local uptime: ");
    Debug.println(millis() / 1000);
    Debug.println("DHT:");
    Debug.print("Temp.: ");
    Debug.println(temperature);
    Debug.print("Hum.: ");
    Debug.println(humidity);

    mLastTime = millis();
  }

  ArduinoOTA.handle();
  Blynk.run();
  timer_Uptime.run();
  Debug.handle();
}

/* --------------------------------------------------------------------------------------------------------------- */

BLYNK_READ(V10) {
    Blynk.virtualWrite(BPIN_HUMIDITY, humidity);
}


BLYNK_READ(V11) {
    Blynk.virtualWrite(BPIN_TEMPERATURE, temperature);
}

void localUptime(void) {
  Blynk.virtualWrite(BPIN_UPTIME, millis() / 1000);
}

void ledBuiltinBlink(uint16 d) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(d);
  digitalWrite(LED_BUILTIN, HIGH);
}