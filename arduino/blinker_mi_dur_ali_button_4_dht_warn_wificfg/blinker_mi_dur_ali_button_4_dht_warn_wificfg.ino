/*
NodeMcu控制四路继电器，集成blinker平台，
用手机app控制四路继电器开关，添加一路物理
开关控制，添加温湿度，添加人体感应控制继电器，继电器用常开（NO）模式
 
v1.0
Created 2021
by 阳阳学编程
www.sooncore.com
抖音号：yyxbc2010
微信号：yyxbc2010
 
说明：
本程序旨在演示如何将NodeMcu控制四路继电器，可以用点灯的App来控制四路继电器，
其间也可以通过物理开关、人体感应来触发控制开关，你还可以通过温湿度条件来触发
控制开关（还没有实现），点灯app中可以展现温湿度和人体感应触发的历史数据。
阳阳学编程共享代码不可商用
转发需取得阳哥同意并备注出处
获得具体连接电路图，请参阅阳阳学编程网站。
*/

//如果要使用apconfig配网模式，打开注释掉，加让这行代码生效
//#define BLINKER_APCONFIG
//
//如果要使用smartconfig配网模式，打开注释掉，加让这行代码生效
//#define BLINKER_ESP_SMARTCONFIG

//支持阳阳学编程的web wifi配网
#define YYXBC_WEBCONFIG


#define BLINKER_PRINT Serial
#define BLINKER_WIFI

#define BLINKER_MIOT_MULTI_OUTLET             // 设置小爱灯类库
#define BLINKER_DUEROS_MULTI_OUTLET           // 设置小度灯类库
#define BLINKER_ALIGENIE_MULTI_OUTLET         // 设置天猫灯类库
#define BLINKER_WITHOUT_SSL

#include <Blinker.h>
#include <ESP8266WebServer.h>

#include <DHT.h>


#if (defined(YYXBC_WEBCONFIG))
  #include "wificfg.h"
#endif

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)


#define D0  16
#define D1  5
#define D2  4
#define D3  0
#define D4  2
#define D5  14
#define D6  12
#define D7  13
#define D8  15
#define D9  3
#define D10 1

//define button GPIO pin
// 0 D0
 const uint16_t  kButton0Pin = D0;
//第一个继电器接D1引脚
const uint16_t  kButton1Pin = D1;
//第二个继电器接D2引脚
const uint16_t  kButton2Pin = D2;
//第三个继电器接D3引脚
const uint16_t  kButton3Pin = D3;
//第四个继电器接D4引脚
const uint16_t  kButton4Pin = D4;
// 物理开关控制的继电器接D1引脚
const uint16_t  LED_BUILTIN_LIGHT = D1;

//物理开关接D6引脚
const uint16_t  LED_BUILTIN_K2 = D6;
//温湿度传感器接在D7引脚
const uint16_t  DHT_DETEC_PIN = D7;

//人体感应传感器的信号线接在D5引脚
const uint16_t  IR_DETEC_PIN = D5;
//人体感应控制的继电器接在D1引脚
const uint16_t  LED_BUILTIN_SENSOR = D2;

/***
 * 物理开关点动模式1，自锁模式0
 */
const int YYXBC_BUTTON_TYPE = 1;

#define IR_DETECTOR_SUPPORT



DHT dht(DHT_DETEC_PIN, DHTTYPE);

int humi_read = 0, temp_read = 0;

char auth[] = "e45526061125";
char ssid[] = "TP-LINK_A407";
char pswd[] = "pp841105";

String version  = "1.0.1";
String httppswd = "123456";


// 新建组件对象
BlinkerButton Button0("btn-0");
BlinkerButton Button1("btn-1");
BlinkerButton Button2("btn-2");
BlinkerButton Button3("btn-3");
BlinkerButton Button4("btn-4");
BlinkerNumber HUMI("humi");
BlinkerNumber TEMP("temp");


//是否处理联网状态
bool isNetConnected(){return (WiFi.status() == WL_CONNECTED);}

//webserver for siri
static ESP8266WebServer esp8266_server(80);

int counter = 0;

int oDuerState[5] = { LOW,LOW,LOW,LOW,LOW };
int oMiState[5] = { LOW,LOW,LOW,LOW,LOW };
int oAligenieState[5] = { LOW,LOW,LOW,LOW,LOW };

BlinkerButton& getBlinkerButton(const uint16_t index){
  switch(index){
    case 0:
      return Button0;
    case 1:
      return Button1;
    case 2:
      return Button2;
    case 3:
      return Button3;
    case 4:
      return Button4;
  }
}

bool getBtn_0_Status(){
  if(HIGH == digitalRead(kButton1Pin) ||
     HIGH == digitalRead(kButton2Pin) ||
     HIGH == digitalRead(kButton3Pin) ||
     HIGH == digitalRead(kButton4Pin) ){
      return true;
    }
    return false;
}

void buttonx_callback(const uint16_t kButtonPin, const uint16_t index, const String & state) {
    BLINKER_LOG("get button state: ", state);
    BLINKER_LOG("kButtonPin： ", kButtonPin);
    BLINKER_LOG("index： ", index);
 
    if (state == BLINKER_CMD_ON) {
        digitalWrite(kButtonPin, HIGH);
        Serial.println("BLINKER_CMD_ON");
        if(HIGH == digitalRead(kButtonPin) ){
            getBlinkerButton(index).print("on");
            oDuerState[index] = HIGH;
            oAligenieState[index] = HIGH;
            oMiState[index]  = HIGH;
        }


    }
    else if (state == BLINKER_CMD_OFF) {
        digitalWrite(kButtonPin, LOW);
        Serial.println("BLINKER_CMD_OFF");
        if(LOW == digitalRead(kButtonPin) ){
            getBlinkerButton(index).print("off");
            oDuerState[index] = LOW;
            oAligenieState[index] = LOW;
            oMiState[index]  = LOW;
        }

  }
 /***
 *计算总开关的状态 
 */
  if(getBtn_0_Status()){
     Button0.print("on");
  }else{
    Button0.print("off");
  }
}
/**
 * 更新天猫精灵，小度，小爱的状态
 */
void updateStatus(int index){
  BlinkerAliGenie.powerState(oAligenieState[index] == HIGH ? "on" : "off", index);
  BlinkerAliGenie.print();
  BlinkerDuerOS.powerState(oDuerState[index] == HIGH ? "on" : "off", index);
  BlinkerDuerOS.print();
  BlinkerMIOT.powerState(oMiState[index] == HIGH ? "on" : "off", index);
  BlinkerMIOT.print();
  

}

void button1_callback(const String & state) {
    buttonx_callback(kButton1Pin,1,state);
}
void button2_callback(const String & state) {
    buttonx_callback(kButton2Pin,2,state);
}

void button3_callback(const String & state) {
    buttonx_callback(kButton3Pin,3,state);
}

void button4_callback(const String & state) {
    buttonx_callback(kButton4Pin,4,state);
}


// 按下按键即会执行该函数
void button0_callback(const String & state) {

    if (state == BLINKER_CMD_ON) {
      button1_callback( BLINKER_CMD_ON);
       delay(200);
      button2_callback( BLINKER_CMD_ON);
       delay(200);
      button3_callback( BLINKER_CMD_ON);
       delay(200);
      button4_callback( BLINKER_CMD_ON);
       delay(200);
      getBlinkerButton(0).print("on");
    }else if(state == BLINKER_CMD_OFF){
      button1_callback( BLINKER_CMD_OFF);
       delay(200);
      button2_callback( BLINKER_CMD_OFF);
       delay(200);
      button3_callback( BLINKER_CMD_OFF);
       delay(200);
      button4_callback( BLINKER_CMD_OFF);
       delay(200);
      getBlinkerButton(0).print("off");
    }
    
}


//心跳回调
void heartbeat()
{
   BLINKER_LOG("heartbeat,state: ", getBtn_0_Status());

    //较正app的按钮状态
  if(getBtn_0_Status()){
     if(isNetConnected()) Button0.print("on");
  }else{
    if(isNetConnected()) Button0.print("off");
  }
  if(HIGH == digitalRead(kButton1Pin) ){
    if(isNetConnected()) Button1.print("on");
  }else{
   if(isNetConnected()) Button1.print("off");
  }
  if(HIGH == digitalRead(kButton2Pin) ){
    if(isNetConnected()) Button2.print("on");
  }else{
   if(isNetConnected()) Button2.print("off");
  }
  if(HIGH == digitalRead(kButton3Pin) ){
    if(isNetConnected()) Button3.print("on");
  }else{
   if(isNetConnected()) Button3.print("off");
  }
  if(HIGH == digitalRead(kButton4Pin) ){
    if(isNetConnected()) Button4.print("on");
  }else{
   if(isNetConnected()) Button4.print("off");
  }

  //更新手机端温湿度数据

  if(isNetConnected()) HUMI.print(humi_read);
  if(isNetConnected()) TEMP.print(temp_read);

}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果小度 有对设备进行操作就执行下面
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void duerPowerState(const String & state, uint8_t num)
{
    BLINKER_LOG("DuerOS need set outlet: ", num, ", power state: ", state);

    //总开关处理
    if(num == 0){
      button1_callback( state);
      button2_callback( state);
      button3_callback( state);
      button4_callback( state);
    }else{
      if(num == 1){
        button1_callback( state);
      }else if(num == 2){
        button2_callback( state);
      }else if(num == 3){
        button3_callback( state);
      }else if(num == 4){
        button4_callback( state);
      }
  } 
  //更新状态
  updateStatus(num);
  
}

void duerQuery(int32_t queryCode, uint8_t num)
{
    BLINKER_LOG("DuerOS Query outlet: ", num,", codes: ", queryCode);

    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_ALL_NUMBER :
            BLINKER_LOG("DuerOS Query All");
            BlinkerMIOT.temp(temp_read);
            BlinkerMIOT.humi(humi_read);
            BlinkerMIOT.powerState(oDuerState[num] == HIGH ? "on" : "off", num);
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
            BLINKER_LOG("DuerOS Query Power State");
            BlinkerMIOT.powerState(oDuerState[num] == HIGH ? "on" : "off", num);
            BlinkerMIOT.print();
            break;
        default :
            BlinkerMIOT.powerState(oDuerState[num] == HIGH ? "on" : "off", num);
            BlinkerMIOT.print();
            break;

    }
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果小爱 有对设备进行操作就执行下面
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//*******如果小爱有对设备进行操作就执行下面
void miotPowerState(const String & state, uint8_t num)
{
    BLINKER_LOG("need set outlet: ", num, ", power state: ", state);

   //总开关处理
    if(num == 0){
      button1_callback( state);
      button2_callback( state);
      button3_callback( state);
      button4_callback( state);
    }else{
      if(num == 1){
        button1_callback( state);
      }else if(num == 2){
        button2_callback( state);
      }else if(num == 3){
        button3_callback( state);
      }else if(num == 4){
        button4_callback( state);
      }
       
  } 
  //更新状态
  updateStatus(num);
}

void miotQuery(int32_t queryCode, uint8_t num)
{
    BLINKER_LOG("AliGenie Query outlet: ", num,", codes: ", queryCode);

    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_ALL_NUMBER :
            BLINKER_LOG("MIOT Query All");
            BlinkerMIOT.temp(temp_read);
            BlinkerMIOT.humi(humi_read);
            BlinkerMIOT.powerState(oMiState[num] == HIGH ? "on" : "off", num);
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
            BLINKER_LOG("MIOT Query Power State");
            BlinkerMIOT.powerState(oMiState[num] == HIGH ? "on" : "off", num);
            BlinkerMIOT.print();
            break;
        default :
            BlinkerMIOT.powerState(oMiState[num] == HIGH ? "on" : "off", num);
            BlinkerMIOT.print();
            break;
    }
}
//

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果天猫精灵有对设备进行操作就执行下面

void aligeniePowerState(const String & state, uint8_t num)
{
    BLINKER_LOG("AliGenie need set outlet: ", num, ", power state: ", state);
   //总开关处理
    if(num == 0){
      button1_callback( state);
      button2_callback( state);
      button3_callback( state);
      button4_callback( state);
    }else{
      if(num == 1){
        button1_callback( state);
      }else if(num == 2){
        button2_callback( state);
      }else if(num == 3){
        button3_callback( state);
      }else if(num == 4){
        button4_callback( state);
      }
  } 
  //更新状态
  updateStatus(num);
}

void aligenieQuery(int32_t queryCode, uint8_t num)
{
    BLINKER_LOG("AliGenie Query outlet: ", num,", codes: ", queryCode);

    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_ALL_NUMBER :
            BLINKER_LOG("AliGenie Query All");
            BlinkerAliGenie.temp(temp_read);
            BlinkerAliGenie.humi(humi_read);
            BlinkerAliGenie.powerState(oAligenieState[num] == HIGH ? "on" : "off", num);
            BlinkerAliGenie.print();
            break;
        case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
            BLINKER_LOG("AliGenie Query Power State");
            BlinkerAliGenie.powerState(oAligenieState[num] == HIGH ? "on" : "off", num);
            BlinkerAliGenie.print();
            break;
        default :
            BlinkerAliGenie.powerState(oAligenieState[num] == HIGH ? "on" : "off", num);
            BlinkerAliGenie.print();
            break;
    }
}


void dataRead(const String & data){
  BLINKER_LOG("Blinker readString: ", data);

  Blinker.vibrate();
  
  uint32_t BlinkerTime = millis();
  
  Blinker.print("millis", BlinkerTime);
}



void setup() {
    // 初始化串口
    Serial.begin(115200);

//    #if defined(BLINKER_PRINT)
//        BLINKER_DEBUG.stream(BLINKER_PRINT);
//    #endif

    //debug 命令
    BLINKER_DEBUG.stream(BLINKER_PRINT);
    BLINKER_DEBUG.debugAll();

     // 初始化有BUTTON的IO
     
    pinMode(kButton1Pin, OUTPUT);
    digitalWrite(kButton1Pin, LOW);
    
    pinMode(kButton2Pin, OUTPUT);
    digitalWrite(kButton2Pin, LOW);
    
    pinMode(kButton3Pin, OUTPUT);
    digitalWrite(kButton3Pin, LOW);
    
    pinMode(kButton4Pin, OUTPUT);
    digitalWrite(kButton4Pin, LOW);

    pinMode(LED_BUILTIN_K2, OUTPUT);

   
 #if (defined(YYXBC_WEBCONFIG))
    //wifi 配网
    WIFI_Init();
    Settings& cfg = getSettings();
    Blinker.begin(cfg.auth.c_str(),WiFi.SSID().c_str(),WiFi.psk().c_str());
 #else
    #if (defined(BLINKER_APCONFIG)) || (defined(BLINKER_ESP_SMARTCONFIG))
       //启动配网模式用这行代码
      Blinker.begin(auth);
   #else
      Blinker.begin(auth, ssid, pswd);
   #endif
 #endif  

 
    Blinker.attachData(dataRead);

    BlinkerDuerOS.attachPowerState(duerPowerState);            //小度语音操作注册函数
    BlinkerDuerOS.attachQuery(duerQuery);
    BlinkerMIOT.attachPowerState(miotPowerState);              //小爱语音操作注册函数
    BlinkerMIOT.attachQuery(miotQuery);
    BlinkerAliGenie.attachPowerState(aligeniePowerState);      //天猫语音操作注册函数
    BlinkerAliGenie.attachQuery(aligenieQuery);

    Button0.attach(button0_callback);
    Button1.attach(button1_callback);
    Button2.attach(button2_callback);
    Button3.attach(button3_callback);
    Button4.attach(button4_callback);

    //人体感应器检测
    #ifdef IR_DETECTOR_SUPPORT
      //初始化sensor
      sensor_init(IR_DETEC_PIN);
    #endif

    //初始化温湿度
    Blinker.attachDataStorage(dataStorage);
    dht.begin();

    
    //注册回调函数
    Blinker.attachHeartbeat(heartbeat);

   //启动webserver ,提供接口给siri用
    esp8266_server.on("/", handleRoot); 
    esp8266_server.on("/post", handleSetConfig);           
    esp8266_server.onNotFound(handleNotFound);  
    esp8266_server.begin();                  
      
    Serial.println("HTTP esp8266_server started");
    Serial.println(WiFi.localIP()); 
}

void loop() {
    Blinker.run();
    esp8266_server.handleClient();// 处理http服务器访问
    static int lastms = 0;
    if (millis()-lastms > 3000) {
      lastms = millis();
      Serial.printf(PSTR("Running version %s for %d Free mem=%d\n"),version.c_str(), lastms/1000, ESP.getFreeHeap());
    }

    //人体感应器检测
    #ifdef IR_DETECTOR_SUPPORT
      sensor_detect(IR_DETEC_PIN);
    #endif

    //检查物理开关状态
    bool bret = false;
    if(YYXBC_BUTTON_TYPE == 1){
       bret = btnHandler1();
    }else{
       bret = btnHandler2();
    }
    
   /*
    *连续5次开关，时间5秒内，认为是要进入配网模式，重启进入配网模式
    */
    #if (defined(YYXBC_WEBCONFIG))
      //物理开关开、关次数，1秒内计入
      static int physics_count  = 0;
      static int lastphysicsms = 0;
      if(bret ){
        Serial.println("物理开关被触发");
        Serial.println(millis()-lastphysicsms);
        if (millis()-lastphysicsms < 2000) {
          
          Serial.println(physics_count);
          Serial.print("/5物理开关触发配网模式...");
          physics_count++;
        }
        lastphysicsms = millis();
      }
      //进入配网模式，重启esp8266
      if(physics_count >= 4){
          Serial.println("正在重启esp8266，启动后进入配网模式...");
          WIFI_RestartToCfg();
      } 
    #endif
    
    //获取温湿度
    getDHT();
  
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果iOS的siri有对设备进行操作就执行下面
                                                                  
void handleRoot() {   //处理网站根目录“/”的访问请求 
   String data = "<html>";
      data += "<head><title>设置</title></head>";
      data += "<body>";
      data +=" <p>ESP8266 智能开关，技术支持 微信:yyxbc2010</p><hr>";
      data +=" <p>SSID:"+ WiFi.SSID() + "</p><hr>";
      data +=" <p>PSW:"+ WiFi.psk() + "</p><hr>";
//      data +=" <form action=\"changeir\" method=\"POST\">";
//      data += "开关序号: <input type=\"text\" name=\"btn\"><br>";
//      data += "on/off: <input type=\"text\"  name=\"state\">";
//      data += "<input type=\"submit\" value=\"btn\"></form>";
      data += "</body>";
      data += "</html>";  
      
    esp8266_server.send(200, "text/html; charset=utf-8", data);  
} 


void handleSetConfig() {   //处理来自siri的访问请求 
  if (esp8266_server.method() != HTTP_POST) {
    esp8266_server.send(405, "text/plain", "Method Not Allowed");
  } else {
//    String message = "POST form was:\n";
    String btnName,btnState,btnPswd;
    for (uint8_t i = 0; i < esp8266_server.args(); i++) {
       String name = esp8266_server.argName(i);
       if(name == "btn") {
           btnName = esp8266_server.arg(i);
       }
       else if (name == "state"){
           btnState = esp8266_server.arg(i);
       } 
       else if (name == "passwd"){
           btnPswd = esp8266_server.arg(i);
       }     
    }
    if(btnPswd != httppswd){
      String message = "{\"errcode\":0,\"msg\":\"password error\"}";  
      esp8266_server.send(200, "text/html; charset=utf-8", message);  
      return;
    }
    if(btnName.length() >0 && btnState.length() >0) {
        //开关btn-0
        if(btnName == "btn-0"){
          if(btnState == "on"){
             button0_callback( BLINKER_CMD_ON);
          }else if (btnState == "off"){
             button0_callback( BLINKER_CMD_OFF);
          }
        }else if(btnName == "btn-1"){
          if(btnState == "on"){
             button1_callback( BLINKER_CMD_ON);
          }else if (btnState == "off"){
             button1_callback( BLINKER_CMD_OFF);
          }
          
        }else if (btnName == "btn-2"){
          if(btnState == "on"){
             button2_callback( BLINKER_CMD_ON);
          }else if (btnState == "off"){
             button2_callback( BLINKER_CMD_OFF);
          }
          
        }else if(btnName == "btn-3"){
          if(btnState == "on"){
             button3_callback( BLINKER_CMD_ON);
          }else if (btnState == "off"){
             button3_callback( BLINKER_CMD_OFF);
          }
          
        }else if(btnName == "btn-4"){
          if(btnState == "on"){
             button4_callback( BLINKER_CMD_ON);
          }else if (btnState == "off"){
             button4_callback( BLINKER_CMD_OFF);
          }
        }
    
    }
    String message = "{\"errcode\":0,\"msg\":\" ok\"}";  
    esp8266_server.send(200, "text/html; charset=utf-8", message); 
  }
}

// 设置处理404情况的函数'handleNotFound'
void handleNotFound(){   
//  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += esp8266_server.uri();
  message += "\nMethod: ";
  message += (esp8266_server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += esp8266_server.args();
  message += "\n";
  for (uint8_t i = 0; i < esp8266_server.args(); i++) {
    message += " " + esp8266_server.argName(i) + ": " + esp8266_server.arg(i) + "\n";
  }
  esp8266_server.send(404, "text/plain", message);

}

/////////////////////Warn sensor///////////////
void sensor_init(int IR_DETEC_PIN)
{
    // pinMode(IR_DETEC_PIN, INPUT_PULLUP);
    pinMode(IR_DETEC_PIN, INPUT);
}


void sensor_detect(int IR_DETEC_PIN)
{
  static bool is_btn = false;//按钮的标志位，用来逻辑处理对比，判断按钮有没有改变状态
  bool is = digitalRead(IR_DETEC_PIN);   //按钮状态
  if ( is != is_btn)
  {
    if (!is)
    {
      Blinker.dataStorage("sensor", false);
      button2_callback(BLINKER_CMD_OFF);
      Serial.println("人体感应对灯已执行关闭");
      Blinker.wechat("人体感应对灯已执行关闭!");
    }
    else
    {
      Blinker.dataStorage("sensor", true);
      button2_callback(BLINKER_CMD_ON);
      Serial.println("人体感应对灯已执行打开");
      Blinker.wechat("人体感应对灯已执行打开!");
    }
    is_btn = digitalRead(IR_DETEC_PIN);  //更新按钮状态
  }
  
}

///////////////////////////////物理开关//////////////////////////////////////////

//点动模式按钮，监听按钮状态，执行相应处理
bool btnHandler1()
{
  static bool oButtonState = false;
  int state1 =  digitalRead(LED_BUILTIN_K2); //按钮状态
  int state2 =  digitalRead(LED_BUILTIN_LIGHT); //灯的状态
  if(state1 == HIGH )
  {
    if(oButtonState){
      if(state2 == HIGH )
      { 
        button1_callback(BLINKER_CMD_OFF);
        Serial.println("按钮对灯已执行关闭");
      }else{
        button1_callback(BLINKER_CMD_ON);
        Serial.println("按钮对灯已执行打开");
      }
      oButtonState = false;
      return true;
    }
    
  }else{
      oButtonState = true;
  }
  

  return false;
}

//自锁模式按钮，监听按钮状态，执行相应处理
bool btnHandler2()
{
 static bool is_btn = false;//按钮的标志位，用来逻辑处理对比，判断按钮有没有改变状态
  bool is = digitalRead(LED_BUILTIN_K2);   //按钮状态
  if ( is != is_btn)
  {
    bool is_led = digitalRead(LED_BUILTIN_LIGHT);
    digitalWrite(LED_BUILTIN_LIGHT, !is_led);
    if (is_led == HIGH)
    {
      button1_callback(BLINKER_CMD_OFF);
      Serial.println("按钮对灯已执行关闭");
    }
    else
    {
      button1_callback(BLINKER_CMD_ON);
      Serial.println("按钮对灯已执行打开");
    }
    is_btn = digitalRead(LED_BUILTIN_K2);  //更新按钮状态
    return true;
  }
 return false;
}
///////////////////////////////温湿度//////////////////////////////////////////

void dataStorage()
{
    Blinker.dataStorage("temp", temp_read);
    Blinker.dataStorage("humi", humi_read);
}

void getDHT(){
  int h = dht.readHumidity();
  int t = dht.readTemperature();
  
  if (isnan(h) || isnan(t))
  {
    BLINKER_LOG("Failed to read from DHT sensor!");
  }
  else
  {
    humi_read = h;
    temp_read = t;
  }
}
