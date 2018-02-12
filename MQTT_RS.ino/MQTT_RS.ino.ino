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
const char* mqtt_server = "broker.mqtt-dashboard.com";

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
  if (client.connect("arduinoClient")) 
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
void setup()
{
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  ETin.begin(details(rxdata), &Serial);
  ETout.begin(details(txdata), &Serial);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
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


void loop()
{
  if(!WiFiConnected())
  {  
    if (millis() - lastWIFIReconnectAttempt > 5000)
    {
      if(setup_wifi())
      {
        RSpisz(RS_CONN_INFO,"WiFi=ok");
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
     
      if (millis() - lastMQTTReconnectAttempt > 5000)
      {
        lastMQTTReconnectAttempt = millis();
        if (reconnectMQTT())
        {
           RSpisz(RS_CONN_INFO,"MQTT=ok");
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
          readRS();
    }
  }
}
