/*
 PubSubClient -mqtt
 Easy-Transfer https://github.com/madsci1016/Arduino-EasyTransfer
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EasyTransfer.h>

//create two objects
EasyTransfer ETin, ETout; 

#define RS_CONN_INFO 0  // wifi / mqtt status
#define RS_RECEIVE_MQTT 1 // msg from mqtt serwer
#define RS_PUBLISH_MQTT 2 // msg to send
#define RS_SUBSCRIBE_MQTT 3 //setup subsribe topic
#define RS_SETUP_INFO 4 //
#define RS_DEBUG_INFO 5 //debug info

#define LED BUILTIN_LED
#define OFF HIGH
#define ON LOW

struct RS_DATA_STRUCTURE
{
  uint8_t type; //RS_xx
  String topic;
  String msg;
};

//give a name to the group of data
RS_DATA_STRUCTURE rxdata;
RS_DATA_STRUCTURE txdata;

const char* ssid = "instalujWirusa";
const char* password = "blablabla123";
const char* mqtt_server = "m10.cloudmqtt.com";
const char* mqtt_user="sduiuylx";
const char* mqtt_pass="x1PkAMgboEro";
const char* mqtt_port="14707";

#define MAX_TOPIC_CNT 50;
char id[5]="";
int topic_cnt=0;
String topicList[50];

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

unsigned long lastMQTTReconnectAttempt = 0;
unsigned long lastWIFIReconnectAttempt = 0;

void callback(char* topic, byte* payload, unsigned int length) {
    // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.

  // Allocate the correct amount of memory for the payload copy
  byte* p = (byte*)malloc(length);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  //client.publish("outTopic", p, length);
  
  txdata.type=RS_RECEIVE_MQTT;
  txdata.topic=topic;
  txdata.msg=p;
  ETout.sendData();
  
  // Free the memory
  free(p);
}


void RSpisz(int t,String s)
{
   txdata.type=t;
   txdata.msg=s;
   ETout.sendData();
}


boolean reconnectMQTT() {
  if (client.connect("MQTT_RS",mqtt_user,mqtt_pass)) 
  {
    for(int i=0;i<topic_cnt;i++)
    {
      client.subscribe(topicList[i]);
    }
  }
  return client.connected();
}

bool setup_wifi() 
{ 
  
  RSpisz(RS_DEBUG_INFO,"Restart WiFi ");
 RSpisz(RS_DEBUG_INFO,ssid);
  WiFi.begin(ssid, password);
  if (!WiFiConnected()) 
  {
    RSpisz(RS_DEBUG_INFO,"Wifi Connection Error.");
    return false;
  }else
  {
   
   RSpisz(RS_DEBUG_INFO,"WiFi connected");
    RSpisz(RS_DEBUG_INFO,"IP address: ");
    RSpisz(RS_DEBUG_INFO,WiFi.localIP());
    return true;
  }
}

//WiFiConnected
//true - connected
//false -not connected
bool WiFiConnected() 
{
  return (WiFi.status() == WL_CONNECTED)
}

/////////////////////////SETUP///////////////////////////
void setup()
{
  pinMode(LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED,OFF);
  Serial.begin(115200);
  ETin.begin(details(rxdata), &Serial);
  ETout.begin(details(txdata), &Serial);
  
 // setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  lastReconnectAttempt = 0;
}


void readRS()
{
    if(!ETin.receiveData()) return;

    switch(ETin.type)
    {
      case RS_CONN_INFO:   // wifi / mqtt status
      //nie pojawi się
           break;
      case RS_RECEIVE_MQTT  // msg from mqtt serwer
      //nie pojawi się
           break;
      case RS_PUBLISH_MQTT  // msg to send
            client.publish(ETin.topic,ETin.msg);
           break;
      case RS_SUBSCRIBE_MQTT  //setup subsribe topic
            if(topic_cnt<MAX_TOPIC_CNT)
            {
              for(int i=0;i<topic_cnt;i++)
              {
                if(ETin.topic==topicList[i] return;
              }
              topicList[topic_cnt]=ETin.topic;
              topic_cnt++;
              client.subscribe(ETin.topic);
            }
           break;
      case RS_SETUP_INFO  //
           break;
      case RS_DEBUG_INFO  //debug info
      //nie pojawi się
           break;
      }
}

#define CONN_STAT_NO 0
#define CONN_STAT_WIFI_CONNECTING 1
#define CONN_STAT_WIFI_OK 2
#define CONN_STAT_WIFIMQTT_CONNECTING 3
#define CONN_STAT_WIFIMQTT_OK 4

int conStat=CONN_STAT_NO;
unsigned long sLEDmillis=0;

void loop()
{
  if(!WiFiConnected())
  { 
    conStat=CONN_STAT_WIFI_CONNECTING;
    if (millis() - lastWIFIReconnectAttempt > 5000)
    {
      if(setup_wifi())
      {
        RSpisz(RS_CONN_INFO,"WiFi=ok");
        conStat=CONN_STAT_WIFI_OK;
      }
      else
      {
        RSpisz(RS_CONN_INFO,"WiFi=Err");
      }
      lastWIFIReconnectAttempt = millis();
    }
  }else
  {
    if (!client.connected()) 
    {
     conStat=CONN_STAT_WIFIMQTT_CONNECTING;
      if (millis() - lastMQTTReconnectAttempt > 5000)
      {
        lastMQTTReconnectAttempt = millis();
        if (reconnectMQTT())
        {
           RSpisz(RS_CONN_INFO,"MQTT=ok");
           conStat=CONN_STAT_WIFIMQTT_OK;
          lastMQTTReconnectAttempt = 0;
        }
        else
        {
             RSpisz(RS_CONN_INFO,"MQTT=Err");
        }
      }
    } else
    {
          client.loop();                
    }
  }
          readRS();


          ///// LED status blink
          unsigned long d=millis()-sLEDmillis;
          if(d>3000)
          {
            sLEDmillis=millis();
          }
            switch(conStat)
            {
              case CONN_STAT_NO: ///----------__------------__
                if(d<2800)digitalWrite(LED,ON)else digitalWrite(LED,OFF);
                    break;
              case CONN_STAT_WIFI_CONNECTING: // ------_-_---------_-_----
                if(d>=0&&d<1000)digitalWrite(LED,ON);
                if(d>=1000&&d<1300)digitalWrite(LED,OFF);
                if(d>=1300&&d<1600)digitalWrite(LED,ON);
                if(d>=1600&&d<1900)digitalWrite(LED,OFF);
                if(d>=1900)digitalWrite(LED,ON);
                    break;
              case CONN_STAT_WIFI_OK: // ---___---___---___
                if(d>=0&&d<700)digitalWrite(LED,ON);
                if(d>=700&&d<1400)digitalWrite(LED,OFF);
                if(d>=1400&&d<2100)digitalWrite(LED,ON);
                if(d>=2100)digitalWrite(LED,OFF);
                    break;
              case CONN_STAT_WIFIMQTT_CONNECTING:// ____-_-_-_______-_-_-_____
                if(d>=0&&d<1000)digitalWrite(LED,OFF);
                if(d>=1000&&d<1300)digitalWrite(LED,ON);
                if(d>=1300&&d<1600)digitalWrite(LED,OFF);
                if(d>=1600&&d<1900)digitalWrite(LED,ON);
                if(d>=1900&&d<2100)digitalWrite(LED,OFF);
                if(d>=2300&&d<2500)digitalWrite(LED,ON);
                if(d>=2500)digitalWrite(LED,OFF);
                    break;
              case CONN_STAT_WIFIMQTT_OK:
               if(d<300)digitalWrite(LED,ON) else digitalWrite(LED,OFF);
                    break;
              }
}
