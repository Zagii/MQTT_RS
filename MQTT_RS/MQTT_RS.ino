/*
 PubSubClient -mqtt
 Easy-Transfer https://github.com/madsci1016/Arduino-EasyTransfer
*/
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <EasyTransfer.h>
#include <SoftwareSerial.h>



#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif



//create two objects
EasyTransfer ETin, ETout; 

SoftwareSerial swSer(14, 12, false, 256);



#define RS_CONN_INFO 'c'  // wifi / mqtt status
#define RS_RECEIVE_MQTT 'r' // msg from mqtt serwer
#define RS_PUBLISH_MQTT 'p' // msg to send
#define RS_SETUP_INFO 's' //
#define RS_DEBUG_INFO 'd' //debug info

#define LED 2

#define OFF HIGH
#define ON LOW

#define MAX_TOPIC_LENGHT 50
#define MAX_MSG_LENGHT 20
#define MAX_STR MAX_TOPIC_LENGHT+MAX_MSG_LENGHT

struct RS_DATA_STRUCTURE
{
  char msg[MAX_MSG_LENGHT];
  char topic[MAX_TOPIC_LENGHT];
  char typ;
};


//give a name to the group of data
RS_DATA_STRUCTURE rxdata;
RS_DATA_STRUCTURE txdata;

const char* nodeMCUid="Reku";
const char* debugTopic="DebugTopic/Reku/nodeMCU";
const char* mqtt_server ="broker.hivemq.com"; //"m23.cloudmqtt.com";
const char* mqtt_user="";//"aigejtoh";
const char* mqtt_pass="";//"ZFlzjMm4T-XH";
const uint16_t mqtt_port=1883;


ESP8266WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient client(espClient);


unsigned long lastMQTTReconnectAttempt = 0;
unsigned long lastWIFIReconnectAttempt = 0;

bool isNumber(char * tmp)
{
   int j=0;
   while(j<strlen(tmp))
  {
    if(tmp[j] > '9' || tmp[j] < '0')
    {
      return false;
    }     
    j++;
  }
 return true; 
}
unsigned long WDmillis=0;
void callback(char* topic, byte* payload, unsigned int length) 
{
  if(length==0)return;
  char* p = (char*)malloc(length);
  memcpy(p,payload,length);
  p[length]='\0';
  txdata.typ=RS_RECEIVE_MQTT;
  strcpy(txdata.topic,topic);
  strcpy(txdata.msg,p);
  DPRINT(__func__);DPRINT(", typ=");  DPRINT(txdata.typ);  DPRINT(" topic=");  DPRINT(txdata.topic); DPRINT(" msg=");  DPRINTLN(txdata.msg);
  
  if(strstr(topic,"watchdog"))
  {
    DPRINT("Watchdog msg=");
    DPRINT(txdata.msg);
    DPRINT(" teraz=");
   
    if(isNumber(p))
      WDmillis=strtoul (p, NULL, 0);
    DPRINTLN(WDmillis);
    

  }else
  {
    ETout.sendData();
  }
  
  free(p);
}


void RSpisz(char typ,const char* topic,char* msg)
{
   txdata.typ=typ;
   strcpy(txdata.topic,topic);
   strcpy(txdata.msg,msg);
   ETout.sendData();
   DPRINT("Debug RSpisz typ=");  DPRINT(txdata.typ);  DPRINT(" topic=");  DPRINT(txdata.topic); DPRINT(" msg=");  DPRINTLN(txdata.msg);
}




bool setup_wifi() 
{ 
  RSpisz(RS_DEBUG_INFO,debugTopic,"Restart WiFi ");
  WiFi.mode(WIFI_STA);
  if(wifiMulti.run() == WL_CONNECTED)
  {
    IPAddress ip=WiFi.localIP();
    char ss[30];
    WiFi.SSID().toCharArray(ss,WiFi.SSID().length()+1);
    char b[100];
    sprintf(b,"WiFi connected: %s ,%d.%d.%d.%d\n", ss, ip[0],ip[1],ip[2],ip[3]);
    RSpisz(RS_DEBUG_INFO,debugTopic,b);
    
    return false;
  }else
  {
    RSpisz(RS_DEBUG_INFO,debugTopic,"Wifi Connection Error."); 
    return true;
  }
}

//WiFiConnected
//true - connected
//false -not connected
bool WiFiConnected() 
{
  return (WiFi.status() == WL_CONNECTED);
}

boolean reconnectMQTT()
{
  if (client.connect(debugTopic,mqtt_user,mqtt_pass)) 
  {
    char s[MAX_TOPIC_LENGHT];
    strcpy(s,nodeMCUid);
    strcat(s,"\/#");  
    client.subscribe(s);
    DPRINT("reconnectMQTT, subscribe to: ");
    DPRINTLN(s);
   // loguj((String)"reconnectMQTT, subscribe to: "+s);
   
  }
  return client.connected();
}


/////////////////////////SETUP///////////////////////////
void setup()
{
 
  Serial.begin(115200);
 swSer.begin(115200);
  //swSer.begin(9600);
 
  DPRINT("");
  DPRINTLN("Setup Serial");
  pinMode(LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED,ON);
    delay(1500);
  wifiMulti.addAP("DOrangeFreeDom", "KZagaw01_ruter_key");
  wifiMulti.addAP("open.t-mobile.pl", "");
  wifiMulti.addAP("instalujWirusa", "blablabla123");
  
  ETin.begin(details(rxdata), &swSer);
  ETout.begin(details(txdata), &swSer);
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loguj(char* t)
{
  char m[MAX_STR];
  sprintf(m,"%s_%s",nodeMCUid,t);
  client.publish(debugTopic,t);//m);
}
void loguj(String s)
{
  char m[MAX_STR];
  s.toCharArray(m,s.length());
  loguj(m);
}

void readRS()
{ 
  
    if(!ETin.receiveData()) return;
   
    DPRINT(__func__);DPRINT(" typ=");DPRINT(rxdata.typ);DPRINT(", topic=");DPRINT(rxdata.topic);DPRINT(", msg=");DPRINTLN(rxdata.msg);
   
    switch(rxdata.typ)
    {
      case RS_CONN_INFO:   // wifi / mqtt status
    
           break;
      case RS_RECEIVE_MQTT:  // msg from mqtt serwer
          
           break;
      case RS_PUBLISH_MQTT:  // msg to send, nie pojawi sie
        
            client.publish(rxdata.topic,rxdata.msg);
           break;
      case RS_SETUP_INFO:  //
           break;
      case RS_DEBUG_INFO:  //debug info
          client.publish(rxdata.topic,rxdata.msg);
           break;
      }
     
}


// t is time in seconds = millis()/1000;
char * TimeToString(unsigned long t)
{
 static char str[12];
 long d =t/(3600*24);
 t=t%(3600*24);
 long h = t / 3600;
 t = t % 3600;
 int m = t / 60;
 int s = t % 60;
 sprintf(str, "%03ldd%02ldh%02dm",d, h, m);
 return str;
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
        RSpisz(RS_CONN_INFO,debugTopic,"WiFi=ok");
        conStat=CONN_STAT_WIFI_OK;
      }
      else
      {
        RSpisz(RS_CONN_INFO,debugTopic,"WiFi=Err");
      
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
           RSpisz(RS_CONN_INFO,(char*)debugTopic,"MQTT=ok");
           conStat=CONN_STAT_WIFIMQTT_OK;
          lastMQTTReconnectAttempt = 0;
          loguj((String)"Połączono ssid="+WiFi.SSID()+" ip="+WiFi.localIP()[0]+"."+WiFi.localIP()[1]+"."+WiFi.localIP()[2]+"."+WiFi.localIP()[3]+"\0");
        }
        else
        {
             RSpisz(RS_CONN_INFO,debugTopic,"MQTT=Err");
             DPRINT("Err MQTTstat= ");DPRINTLN(client.state());
             DPRINT("WIFI ip= ");DPRINTLN(WiFi.localIP());
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
          if(millis()%600000==0) //10 min
          {
            char m[MAX_MSG_LENGHT];
            sprintf(m,"%ld",millis());
            char m2[MAX_TOPIC_LENGHT];
            sprintf(m2,"%s/watchdog",debugTopic);//nodeMCUid);
            client.publish(m2,m);
          }
          if(d>3000)// max 3 sek
          {
           
           sLEDmillis=millis();
 char mst[50];
     sprintf(mst,"t=%02d min %02d s",sLEDmillis/60000,(sLEDmillis%60000)/1000);
  RSpisz(RS_DEBUG_INFO,"Z nodeMCU",mst);
   //  Serial.println(mst);
          }
           
          if(millis()%600000==0)//10 min
          {
            unsigned long mmm=millis();
   
            String str="czas od restartu= "+(String) TimeToString(mmm/1000);
//loguj(str);
            
            DPRINTLN(str);
            DPRINT("Watchdog czas ");
            DPRINTLN(mmm-WDmillis);
            if(mmm-WDmillis>600000)
            {
              DPRINTLN("Watchdog restart");
              loguj("Watchdog restart");
              delay(3000);
              ESP.restart();
            }
          }
            switch(conStat)
            {
              case CONN_STAT_NO: ///----------__------------__  <-- ten stan praktycznie nie występuje
                if(d<2800)digitalWrite(LED,ON); else digitalWrite(LED,OFF);
                    break;
              case CONN_STAT_WIFI_CONNECTING: // ------_-_---------_-_----
                if(d>=0&&d<1000)digitalWrite(LED,ON);
                if(d>=1000&&d<1300)digitalWrite(LED,OFF);
                if(d>=1300&&d<1600)digitalWrite(LED,ON);
                if(d>=1600&&d<1900)digitalWrite(LED,OFF);
                if(d>=1900)digitalWrite(LED,ON);
                    break;
              case CONN_STAT_WIFI_OK: // ---___---___---___ <-- ten stan praktycznie nie występuje
                if(d>=0&&d<700)digitalWrite(LED,ON);
                if(d>=700&&d<1400)digitalWrite(LED,OFF);
                if(d>=1400&&d<2100)digitalWrite(LED,ON);
                if(d>=2100)digitalWrite(LED,OFF);
                    break;
              case CONN_STAT_WIFIMQTT_CONNECTING:// ---___---___---___
                  digitalWrite(LED, !digitalRead(LED));
                    break;
              case CONN_STAT_WIFIMQTT_OK: // --___________--_________
               if(d<300)digitalWrite(LED,ON); else digitalWrite(LED,OFF);
                    break;
              }
}
