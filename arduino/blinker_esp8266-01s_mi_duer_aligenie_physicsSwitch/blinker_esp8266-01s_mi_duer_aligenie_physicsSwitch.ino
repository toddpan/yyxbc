/*
NodeMcu或esp-01/01s控制1路继电器，集成blinker平台，
用手机app控制1路继电器开关，添加一路物理开关控制，继电器用常开（NO）模式
 
v1.0
Created 2021
by 阳阳学编程

说明：
1，继电器高电平触发时，YYXBC_HIGH = 1，YYXBC_LOW  = 0
继电器低电平触发时，YYXBC_HIGH = 0，YYXBC_LOW  = 1
const int YYXBC_HIGH = 0 ;
const int YYXBC_LOW  = 1 ;

2，用esp-01时，物理开关接在vcc 和gpio2上，继电器接在gpio0上
用nodeMcu时，物理开关接在vcc 和D4上，继电器接在D3上，下面的宏
定义只能同时有一个生效

//NodeMCU 继电器接D3,物理开关接D4
#define LED_BUILTIN_LIGHT 0
#define LED_BUILTIN_K2 2

//Esp-01/01s，继电器接GPIO0,物理开关接GPIO2
//#define LED_BUILTIN_LIGHT D3
//#define LED_BUILTIN_K2 D4

3，YYXBC_BUTTON_TYPE = 1时表示物理开关为点动模式，0时表示自锁模式
const int YYXBC_BUTTON_TYPE = 1;

获得具体连接电路图和更多资源，请参阅阳阳学编程网站 www.sooncore.com。
   
This example code is in the public domain.
*/

#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#define BLINKER_MIOT_OUTLET              // 设置小爱灯类库
#define BLINKER_DUEROS_OUTLET            // 设置小度灯类库
#define BLINKER_ALIGENIE_OUTLET          // 设置天猫灯类库

#define BLINKER_WITHOUT_SSL              //使用这个宏，表示不用SSL加密，可以得到更多内存


//
////如果要使用apconfig配网模式，打开注释掉，加让这行代码生效
//#define BLINKER_APCONFIG
////
////如果要使用smartconfig配网模式，打开注释掉，加让这行代码生效
//#define BLINKER_ESP_SMARTCONFIG

//支持阳阳学编程的web wifi配网，暂时不开源
//#define YYXBC_WEBCONFIG


//支持433自锁开关功能，需要用蜂鸟无线公司出的远-R1 433模块，
//将R1的VCC和GND分别接在esp-01s的VCC和GND上，两个DAT引脚的随意一个接在esp-01s的RX上
#define YYXBC_WITH_433

#include <Blinker.h>
#include <ESP8266WebServer.h>

//依赖库 LinkedList
//https://github.com/ivanseidel/LinkedList
#include <LinkedList.h>

#if (defined(YYXBC_WEBCONFIG))
  #include "wificfg.h"
#endif


#if (defined(YYXBC_WITH_433))
  #include <LittleFS.h>
  #include <FS.h>
  #include <RCSwitch.h>
  RCSwitch mySwitch = RCSwitch();
  //433指令数据格式
  struct RawData{
    uint16_t bits;
    uint16_t protocol;
    long unsigned data;
    RawData():protocol(0),data(0){}
    
  };
//433指令列表
LinkedList<RawData*> mySwitchList;
#define MYFS LittleFS
#endif

char auth[] = "a7a437131912";
char ssid[] = "panzujiMi10";
char pswd[] = "moto1984";

String version  = "1.0.6";

//Esp-01/01s，继电器接GPIO0,物理开关接GPIO2
#define LED_BUILTIN_LIGHT 0
#define LED_BUILTIN_K2 2
#define LED_BUILTIN_K3 3
#define LED_BUILTIN_KED 2

////NodeMCU 继电器接D3,物理开关接D4
//#define LED_BUILTIN_LIGHT D3
//#define LED_BUILTIN_K2 D4

/***
 * 继电器高电平触发时，YYXBC_HIGH = 1，YYXBC_LOW  = 0
 * 继电器低电平触发时，YYXBC_HIGH = 0，YYXBC_LOW  = 1
 */
const int YYXBC_HIGH = 0 ;
const int YYXBC_LOW  = 1 ;

/***
 * 物理开关点动模式1，自锁模式0
 */
const int YYXBC_BUTTON_TYPE = 1;

//http接口请求密码
String httppswd = "123456";

bool oDuerState = YYXBC_LOW;
bool oMioState = YYXBC_LOW;
bool oAligenieState = YYXBC_LOW;


// 新建组件对象
BlinkerButton Button1("btn-abc");//注意：要和APP组件’数据键名’一致
BlinkerText Text1("tex-abc");

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

//webserver for siri
ESP8266WebServer* esp8266_server = NULL;

//是否处理联网状态
bool isNetConnected(){return (WiFi.status() == WL_CONNECTED);}

//心跳回调
void heartbeat()
{
   BLINKER_LOG("heartbeat,state: ", digitalRead(LED_BUILTIN));

    //较正app的按钮状态
    if(YYXBC_HIGH == digitalRead(LED_BUILTIN_LIGHT) ){
        Button1.print("on");
        BUILTIN_SWITCH.print("on");
        Text1.color(oDuerState == YYXBC_HIGH  ? "Blue" : "black");
        Text1.print(oDuerState == YYXBC_HIGH  ? "灯状态: 开启" : "灯状态: 关闭");
    }else{
       Button1.print("off");
       BUILTIN_SWITCH.print("off");
       Text1.color(oDuerState == YYXBC_HIGH  ? "Blue" : "black");
       Text1.print(oDuerState == YYXBC_HIGH  ? "灯状态: 开启" : "灯状态: 关闭");
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 按下BlinkerAPP按键即会执行该函数
void switch_callback(const String & state)
{
    BLINKER_LOG("get switch state: ", state);

    //检查配网模式
    check_if_config(state);
    
    if (state == BLINKER_CMD_ON) {
        BLINKER_LOG("Toggle on!");
        if(isNetConnected())Button1.print("on");
        digitalWrite(LED_BUILTIN_LIGHT, YYXBC_HIGH);
        oDuerState = YYXBC_HIGH;
        oMioState =YYXBC_HIGH;
        oAligenieState = YYXBC_HIGH;
    }
    else if (state == BLINKER_CMD_OFF) {
        BLINKER_LOG("Toggle off!");
        if(isNetConnected())Button1.print("off");
        digitalWrite(LED_BUILTIN_LIGHT, YYXBC_LOW);
        oDuerState = YYXBC_LOW;
        oMioState = YYXBC_LOW;
        oAligenieState = YYXBC_LOW;
    }

    if (state == BLINKER_CMD_ON)
    {
      BUILTIN_SWITCH.print("on"); //发送开关当前的状态(多用于反馈开关状态)到APP
  
      Text1.color(oDuerState == YYXBC_HIGH  ? "Blue" : "black");
      Text1.print(oDuerState == YYXBC_HIGH  ? "灯状态: 开启" : "灯状态: 关闭");
    }
    else
    {
      BUILTIN_SWITCH.print("off");
      
      Text1.color(oDuerState == YYXBC_HIGH  ? "Blue" : "black");
      Text1.print(oDuerState == YYXBC_HIGH  ? "灯状态: 开启" : "灯状态: 关闭");
    }

    if(isNetConnected()){
      BlinkerDuerOS.powerState(oDuerState == YYXBC_HIGH ? "on" : "off");
      BlinkerDuerOS.report();
      BlinkerMIOT.powerState(oMioState == YYXBC_HIGH ? "on" : "off");
      BlinkerMIOT.print();
      BlinkerAliGenie.powerState(oAligenieState == YYXBC_HIGH ? "on" : "off");
      BlinkerAliGenie.print(); 
    }
}

//*******如果是手机app 有对设备进行操作就执行下面

void button1_callback(const String & state)
{
    BLINKER_LOG("button1_callback get button state: ", state);

    //检查配网模式
    check_if_config(state);

    if (state == BLINKER_CMD_ON) {
        BLINKER_LOG("Toggle on!");
        if(isNetConnected())Button1.print("on");
        digitalWrite(LED_BUILTIN_LIGHT, YYXBC_HIGH);
        oDuerState = YYXBC_HIGH;
        oMioState =YYXBC_HIGH;
        oAligenieState = YYXBC_HIGH;
    }
    else if (state == BLINKER_CMD_OFF) {
        BLINKER_LOG("Toggle off!");
        if(isNetConnected())Button1.print("off");
        digitalWrite(LED_BUILTIN_LIGHT, YYXBC_LOW);
        oDuerState = YYXBC_LOW;
        oMioState = YYXBC_LOW;
        oAligenieState = YYXBC_LOW;
    }

     if (state == BLINKER_CMD_ON)
    {
      BUILTIN_SWITCH.print("on"); //发送开关当前的状态(多用于反馈开关状态)到APP
  
      Text1.color(oDuerState == YYXBC_HIGH  ? "Blue" : "black");
      Text1.print(oDuerState == YYXBC_HIGH  ? "灯状态: 开启" : "灯状态: 关闭");
    }
    else
    {
      BUILTIN_SWITCH.print("off");
      
      Text1.color(oDuerState == YYXBC_HIGH  ? "Blue" : "black");
      Text1.print(oDuerState == YYXBC_HIGH  ? "灯状态: 开启" : "灯状态: 关闭");
    }

    if(isNetConnected()){
      BlinkerDuerOS.powerState(oDuerState == YYXBC_HIGH ? "on" : "off");
      BlinkerDuerOS.report();
      BlinkerMIOT.powerState(oMioState == YYXBC_HIGH ? "on" : "off");
      BlinkerMIOT.print();
      BlinkerAliGenie.powerState(oAligenieState == YYXBC_HIGH ? "on" : "off");
      BlinkerAliGenie.print(); 
    }
}

void check_if_config(const String & state)
{
    BLINKER_LOG("check_if_config get button state: ", state);

    static long lastpresstime =millis();
    if (state == BLINKER_CMD_BUTTON_PRESSED){
        BLINKER_LOG("Button pressed!");
        lastpresstime = millis();
    }
    else if (state == BLINKER_CMD_BUTTON_PRESSUP){
      BLINKER_LOG("Button pressup,lastpresstime(%d)!",lastpresstime);
      //判断是否是长按5秒
      if(millis() - lastpresstime > 5000){
        lastpresstime =  millis();
          Serial.println("正在重启esp8266，启动后进入配网模式...");
          #if (defined(YYXBC_WEBCONFIG))
              WIFI_RestartToCfg();
          #else
             ESP.restart();
          #endif
          
      }
    }

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果小度 有对设备进行操作就执行下面
void duerPowerState(const String & state)
{
    BLINKER_LOG("duerPowerState need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
       button1_callback(BLINKER_CMD_ON);
    }
    else if (state == BLINKER_CMD_OFF) {
      button1_callback(BLINKER_CMD_OFF);
    }
}

void duerQuery(int32_t queryCode)
{
    BLINKER_LOG("DuerOS Query codes: ", queryCode);

    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :        
            BLINKER_LOG("DuerOS Query power state");
            BlinkerDuerOS.powerState(oDuerState == YYXBC_HIGH? "on" : "off");
            BlinkerDuerOS.print();
            break;
        case BLINKER_CMD_QUERY_TIME_NUMBER :
            BLINKER_LOG("DuerOS Query time");
            BlinkerDuerOS.time(millis());
            BlinkerDuerOS.print();
            break;
        default :
            BlinkerDuerOS.powerState(oDuerState == YYXBC_HIGH? "on" : "off");
            BlinkerDuerOS.print();
            break;
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果小爱有对设备进行操作就执行下面
void miotPowerState(const String & state)
{
    BLINKER_LOG("miotPowerState need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
       button1_callback(BLINKER_CMD_ON);
    }
    else if (state == BLINKER_CMD_OFF) {
      button1_callback(BLINKER_CMD_OFF);
    }
}

void miotQuery(int32_t queryCode)
{
    BLINKER_LOG("MIOT Query codes: ", queryCode);

    switch (queryCode)
    {
      case BLINKER_CMD_QUERY_ALL_NUMBER :
          BLINKER_LOG("MIOT Query All");
          BlinkerMIOT.powerState(oMioState == YYXBC_HIGH? "on" : "off");
          BlinkerMIOT.print();
          break;
      case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
          BLINKER_LOG("MIOT Query Power State");
          BlinkerMIOT.powerState(oMioState ==YYXBC_HIGH ? "on" : "off");
          BlinkerMIOT.print();
          break;
      default :
          BlinkerMIOT.powerState(oMioState ==YYXBC_HIGH ? "on" : "off");
          BlinkerMIOT.print();
          break;
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果天猫精灵有对设备进行操作就执行下面
void aligeniePowerState(const String & state)
{
    BLINKER_LOG("aligeniePowerState need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
       button1_callback(BLINKER_CMD_ON);
    }
    else if (state == BLINKER_CMD_OFF) {
      button1_callback(BLINKER_CMD_OFF);
    }
}

void aligenieQuery(int32_t queryCode)
{
    BLINKER_LOG("AliGenie Query codes: ", queryCode);

    switch (queryCode)
    {
      case BLINKER_CMD_QUERY_ALL_NUMBER :
          BLINKER_LOG("AliGenie Query All");
          BlinkerAliGenie.powerState(oAligenieState == YYXBC_HIGH? "on" : "off");
          BlinkerAliGenie.print();
          break;
      case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
          BLINKER_LOG("AliGenie Query Power State");
          BlinkerAliGenie.powerState(oAligenieState == YYXBC_HIGH? "on" : "off");
          BlinkerAliGenie.print();
          break;
      default :
          BlinkerAliGenie.powerState(oAligenieState == YYXBC_HIGH ? "on" : "off");
          BlinkerAliGenie.print();
          break;
    }
}

void dataRead(const String & data){
  BLINKER_LOG("Blinker readString: ", data);

//  Blinker.vibrate();
//  
//  uint32_t BlinkerTime = millis();
//  
//  Blinker.print("millis", BlinkerTime);

}

/////////////////////////////////////main/////////////////////////////////


/*
*程序入口函数
**/
void setup() {

//   
    // 初始化串口
    Serial.begin(115200);

    #if defined(BLINKER_PRINT)
        BLINKER_DEBUG.stream(BLINKER_PRINT);
    #endif

//        //debug 命令
//    BLINKER_DEBUG.stream(BLINKER_PRINT);
//    BLINKER_DEBUG.debugAll();

    // 初始化有LED的IO
    pinMode(LED_BUILTIN_LIGHT, OUTPUT);
    digitalWrite(LED_BUILTIN_LIGHT, YYXBC_LOW);
    
    pinMode(LED_BUILTIN_K2, OUTPUT);
//    digitalWrite(LED_BUILTIN_K2, LOW);

  #if(defined(YYXBC_WITH_433))
      //load 433 configure
    loadConfig(&MYFS);
    pinMode(LED_BUILTIN_K3, FUNCTION_3); 
    mySwitch.enableReceive(LED_BUILTIN_K3);
    Serial.println("started RCSwitch");
  #endif  

 #if (defined(YYXBC_WEBCONFIG))
    //wifi 配网
    attach(loop_callback);
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
    BlinkerDuerOS.attachPowerState(duerPowerState); //小度语音操作注册函数
    BlinkerDuerOS.attachQuery(duerQuery);

    BlinkerMIOT.attachPowerState(miotPowerState);//小爱语音操作注册函数
    BlinkerMIOT.attachQuery(miotQuery);

    BlinkerAliGenie.attachPowerState(aligeniePowerState);//天猫语音操作注册函数
    BlinkerAliGenie.attachQuery(aligenieQuery);     
  
    Button1.attach(button1_callback);
    BUILTIN_SWITCH.attach(switch_callback);
    
    //注册回调函数
    Blinker.attachHeartbeat(heartbeat);
    
    //启动webserver ,提供接口给siri用
    esp8266_server = new ESP8266WebServer(80);
    esp8266_server->on("/", handleRoot); 
    esp8266_server->on("/post", handleSetConfig);           
    esp8266_server->onNotFound(handleNotFound);  
    esp8266_server->begin();                  
    Serial.println("HTTP esp8266_server started");
    

}

void loop() {

    static int lastms = millis();
    if (millis()-lastms > 30000) {
      lastms = millis();
      Serial.printf(PSTR("Running (%s),state(%s),version %s for %d Free mem=%d\n"),
          WiFi.localIP().toString().c_str(),
          oAligenieState == YYXBC_HIGH  ? "YYXBC_HIGH" : "CLOES",
          version.c_str(), lastms/1000, ESP.getFreeHeap());
    }
    
    Blinker.run();//运行Blinker
    Blinker.autoRun();
    esp8266_server->handleClient();// 处理http服务器访问
    
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
    //物理开关开、关次数，1秒内计入
    static int physics_count  = 0;
    static int lastphysicsms = 0;
    if(bret ){
      Serial.println("物理开关被触发");
      Serial.println(millis()-lastphysicsms);
      if (millis()-lastphysicsms < 2000) {
        Serial.printf("%u/5物理开关触发配网模式...",physics_count);
        physics_count++;
      }else{
        physics_count = 0;  
      }
      lastphysicsms = millis();
    }
    //进入配网模式，重启esp8266
    if(physics_count >= 4){
        Serial.println("正在重启esp8266，启动后进入配网模式...");
       #if (defined(YYXBC_WEBCONFIG))
        WIFI_RestartToCfg();
       #else
       ESP.restart();
      #endif
    } 

   #if(defined(YYXBC_WITH_433))
   if (mySwitch.available()) {
      uint16_t bit = mySwitch.getReceivedBitlength();
      uint16_t protocol = mySwitch.getReceivedProtocol();
      long unsigned switchKey = mySwitch.getReceivedValue();
      Serial.printf("Received :%u,/bit %u,Protocol %u \r\n",switchKey,bit,protocol);
      if(bit > 0 ){
        if( findRawData( bit, protocol,switchKey )){
          mySwitchHandler();
        }else{
          Serial.println("不执行");
        }
      } 
      mySwitch.resetAvailable();
    }
  #endif  
    
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
      data += "</body>";
      data += "</html>";  
      
    esp8266_server->send(200, "text/html; charset=utf-8", data);  
} 


void handleSetConfig() {   //处理来自siri的访问请求 
  if (esp8266_server->method() != HTTP_POST) {
    esp8266_server->send(405, "text/plain", "Method Not Allowed");
  } else {
//    String message = "POST form was:\n";
    String btnName,btnState,btnPswd;
    for (uint8_t i = 0; i < esp8266_server->args(); i++) {
       String name = esp8266_server->argName(i);
       if(name == "btn") {
           btnName = esp8266_server->arg(i);
       }
       else if (name == "state"){
           btnState = esp8266_server->arg(i);
       } 
       else if (name == "passwd"){
           btnPswd = esp8266_server->arg(i);
       }     
    }
    if(btnPswd != httppswd){
      String message = "{\"errcode\":0,\"msg\":\"password error\"}";  
      esp8266_server->send(200, "text/html; charset=utf-8", message);  
      return;
    }
    if(btnName.length() >0 && btnState.length() >0) {
        //开关btn-abc
        if(btnName == "btn-1"){
          if(btnState == "on"){
             button1_callback( BLINKER_CMD_ON);
          }else if (btnState == "off"){
             button1_callback( BLINKER_CMD_OFF);
          }
        }
    
    }
    String message = "{\"errcode\":0,\"msg\":\" ok\"}";  
    esp8266_server->send(200, "text/html; charset=utf-8", message); 
  }
}

// 设置处理404情况的函数'handleNotFound'
void handleNotFound(){   
//  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += esp8266_server->uri();
  message += "\nMethod: ";
  message += (esp8266_server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += esp8266_server->args();
  message += "\n";
  for (uint8_t i = 0; i < esp8266_server->args(); i++) {
    message += " " + esp8266_server->argName(i) + ": " + esp8266_server->arg(i) + "\n";
  }
  esp8266_server->send(404, "text/plain", message);

}

//点动模式按钮，监听按钮状态，执行相应处理
bool btnHandler1()
{
  static bool oButtonState = false;
  int state1 =  digitalRead(LED_BUILTIN_K2); //按钮状态
  int state2 =  digitalRead(LED_BUILTIN_LIGHT); //灯的状态
  if(state1 == HIGH )
  {
    if(oButtonState){
      if(state2 == YYXBC_HIGH )
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
  static bool is_btn = digitalRead(LED_BUILTIN_K2);//按钮的标志位，用来逻辑处理对比，判断按钮有没有改变状态
  bool is = digitalRead(LED_BUILTIN_K2);   //按钮状态
  if ( is != is_btn)
  {
    bool is_led = digitalRead(LED_BUILTIN_LIGHT);
    digitalWrite(LED_BUILTIN_LIGHT, !is_led);
    if (is_led == YYXBC_HIGH)
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

//自锁模式按钮，监听433按钮状态，执行相应处理
bool btnHandler3()
{
  static bool is_btn = digitalRead(LED_BUILTIN_K3);//按钮的标志位，用来逻辑处理对比，判断按钮有没有改变状态
  bool is = digitalRead(LED_BUILTIN_K3);   //按钮状态
  if ( is != is_btn)
  {
    bool is_led = digitalRead(LED_BUILTIN_LIGHT);
    digitalWrite(LED_BUILTIN_LIGHT, !is_led);
    if (is_led == YYXBC_HIGH)
    {
      button1_callback(BLINKER_CMD_OFF);
      Serial.println("按钮对灯已执行关闭");
    }
    else
    {
      button1_callback(BLINKER_CMD_ON);
      Serial.println("按钮对灯已执行打开");
    }
    is_btn = digitalRead(LED_BUILTIN_K3);  //更新按钮状态
    return true;
  }
 return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool findRawData(uint16_t bit,uint16_t protocol,long unsigned data )
{
 int nSize = mySwitchList.size();
  for(int i = 0;i< nSize ;i++){
    RawData* pRawData = mySwitchList.get(i);
    if(pRawData){
      if(bit == pRawData->bits &&
      protocol == pRawData->protocol &&
      data == pRawData->data){
        return true;
      }
    }
  }
  return false;
}

void addRawData(uint16_t bit,uint16_t protocol,long unsigned data ){
  if(findRawData(bit,protocol,data)){
    Serial.println("433指令已经存在，不作处理");
    return;
  }
  RawData* tempRawData_   =  new RawData();
  tempRawData_->bits = bit;
  tempRawData_->protocol = protocol;
  tempRawData_->data = data;
  mySwitchList.add(tempRawData_);
  Serial.println("指令对码完成");
  
}

bool saveConfig(FS *fs){
  if (!fs->begin()) {
    Serial.println("Unable to begin(), aborting");
    return false;
  }
  Serial.println("Save file, may take a while...");
  long start = millis();
  File f = fs->open("/433config.bin", "w");
  if (!f) {
    Serial.println("Unable to open file for writing, aborting");
    return false;
  }
  int nSize = mySwitchList.size();
  int nwrite = f.write((char*)&nSize, sizeof(uint16_t));
  // assert(nwrite != sizeof(uint16_t));
  for(int i = 0;i< nSize ;i++){
    RawData* pRawData = mySwitchList.get(i);
    if(pRawData){
      f.write((char*)&pRawData->bits, sizeof(uint16_t));
      f.write((char*)&pRawData->protocol, sizeof(uint16_t));
      f.write((char*)&pRawData->data, sizeof(long unsigned));
    }
  }
  f.close();
  long stop = millis();
  Serial.printf("==> Time to write  chunks = %ld milliseconds\n", stop - start);
  return true;
}

bool loadConfig(FS *fs){
if (!fs->begin()) {
    Serial.println("Unable to begin(), aborting");
    return false;
  }
  Serial.println("Reading file, may take a while...");
  long start = millis();
  File f = fs->open("/433config.bin", "r");
  if (!f) {
    Serial.println("Unable to open file for reading, aborting");
    return false;
  }
  int nSize = 0;
  int nread = f.read((uint8_t*)&nSize, sizeof(uint16_t));
  // assert(nread != sizeof(uint16_t));
   Serial.println(nread);
  for(int i = 0;i< nSize ;i++){
    RawData* pRawData = new RawData();
    if(pRawData){
      f.read((uint8_t*)&pRawData->bits, sizeof(uint16_t));
      f.read((uint8_t*)&pRawData->protocol, sizeof(uint16_t));
      f.read((uint8_t*)&pRawData->data, sizeof(long unsigned));
      Serial.printf("bit:%u,protocol:%u,data:%u \r\n",pRawData->bits,pRawData->protocol,pRawData->data);
      mySwitchList.add(pRawData);
    }
  }
  f.close();
  long stop = millis();
  Serial.printf("==> Time to write  chunks = %ld milliseconds\n", stop - start);
  return true;
}


/**
*配网模式下，loop在这里执行
*/
int loop_callback (){
  
//  static int lastms = millis();
//  if (millis()-lastms > 3000) {
//    lastms = millis();
//    Serial.printf(PSTR("配网模式 version %s for %d Free mem=%d\n"),version.c_str(), lastms/1000, ESP.getFreeHeap());
//  }

  #if(defined(YYXBC_WITH_433))
   if (mySwitch.available()) {

      uint16_t bit = mySwitch.getReceivedBitlength();
      uint16_t protocol = mySwitch.getReceivedProtocol();
      long unsigned switchKey = mySwitch.getReceivedValue();
      Serial.printf("Received :%u,bit %u,Protocol %u \r\n",switchKey,bit,protocol);
  
      if(bit > 0 ){
        if(findRawData( bit, protocol,switchKey )){
          mySwitchHandler();
        }else{
          addRawData( bit, protocol,switchKey);
          saveConfig(&MYFS);
        }
      }

      //LED灯常亮5秒
      digitalWrite(LED_BUILTIN_KED, HIGH);
      delay(5000);
      digitalWrite(LED_BUILTIN_KED, LOW);

      mySwitch.resetAvailable();
    }
  #endif  
  
  return 0;
}

//自锁模式按钮，监听433按钮状态，执行相应处理
bool mySwitchHandler()
{
  static long lastphysicsms = 0;
  if (millis()-lastphysicsms > 800) {
      bool is_led = digitalRead(LED_BUILTIN_LIGHT);
      digitalWrite(LED_BUILTIN_LIGHT, !is_led);
      if (is_led == YYXBC_HIGH)
      {
        button1_callback(BLINKER_CMD_OFF);
        Serial.println("按钮对灯已执行关闭");
      }
      else
      {
        button1_callback(BLINKER_CMD_ON);
        Serial.println("按钮对灯已执行打开");
      }
      is_led = digitalRead(LED_BUILTIN_LIGHT);  //更新按钮状态
      lastphysicsms = millis();
      
      return true;
  }

 return false;
}
