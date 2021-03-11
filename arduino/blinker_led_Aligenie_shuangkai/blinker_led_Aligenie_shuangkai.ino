#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#define BLINKER_ALIGENIE_OUTLET          // 设置天猫灯类库

#define BLINKER_WITHOUT_SSL

//如果要使用apconfig配网模式，打开注释掉，加让这行代码生效
//#define BLINKER_APCONFIG
//
//如果要使用smartconfig配网模式，打开注释掉，加让这行代码生效
//#define BLINKER_ESP_SMARTCONFIG


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

#define LED_BUILTIN D1
#define LED_BUILTIN_OPEN  D2
#define LED_BUILTIN_CLOSE D3
#define LED_BUILTIN_LIGHT D4

#include <Blinker.h>

char auth[] = "0041057c5088";
char ssid[] = "Xiaomi_10EE";
char pswd[] = "moto1984";

String version  = "1.0.1";


// 新建组件对象
BlinkerButton Button1("btn-abc");
BlinkerNumber Number1("num-abc");

int counter = 0;
int oAligenieState = LOW;
//心跳回调
void heartbeat()
{
   BLINKER_LOG("heartbeat,state: ", digitalRead(LED_BUILTIN));

    //较正app的按钮状态
    int state =  digitalRead(LED_BUILTIN);
    if(state != oAligenieState){
      if(HIGH == state ){
          Button1.print("on");
      }else{
         Button1.print("off");
      }
    }
}


void button1_callback(const String & state)
{
    BLINKER_LOG("get button state: ", state);

    if (state == BLINKER_CMD_ON) {
        BLINKER_LOG("Toggle on!");
        Button1.print("on");
        digitalWrite(LED_BUILTIN, HIGH);
        
        analogWrite(LED_BUILTIN_LIGHT, 255);
        
        oAligenieState = true;
    }
    else if (state == BLINKER_CMD_OFF) {
        BLINKER_LOG("Toggle off!");
        Button1.print("off");
        digitalWrite(LED_BUILTIN, LOW);
        oAligenieState = false;

        analogWrite(LED_BUILTIN_LIGHT, 0);
    }
    BlinkerAliGenie.powerState(oAligenieState ? "on" : "off");
    BlinkerAliGenie.print(); 
}


void button2_callback(const String & state)
{
    BLINKER_LOG("get button state: ", state);

    if (state == BLINKER_CMD_ON) {
        BLINKER_LOG("Toggle on!");
        Button1.print("on");
        oAligenieState = true;
    }
    else if (state == BLINKER_CMD_OFF) {
        BLINKER_LOG("Toggle off!");
        Button1.print("off");
        oAligenieState = false;
    }
    BlinkerAliGenie.powerState(oAligenieState ? "on" : "off");
    BlinkerAliGenie.print(); 
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果天猫精灵有对设备进行操作就执行下面
void aligeniePowerState(const String & state)
{
    BLINKER_LOG("need set power state: ", state);

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
          BlinkerAliGenie.powerState(oAligenieState ? "on" : "off");
          BlinkerAliGenie.print();
          break;
      case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
          BLINKER_LOG("AliGenie Query Power State");
          BlinkerAliGenie.powerState(oAligenieState ? "on" : "off");
          BlinkerAliGenie.print();
          break;
      default :
          BlinkerAliGenie.powerState(oAligenieState ? "on" : "off");
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

    // 初始化有LED的IO
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    pinMode(LED_BUILTIN_OPEN, OUTPUT);
    digitalWrite(LED_BUILTIN_OPEN, LOW);
    
    pinMode(LED_BUILTIN_CLOSE, OUTPUT);
    digitalWrite(LED_BUILTIN_CLOSE, LOW);

    pinMode(LED_BUILTIN_LIGHT, OUTPUT); 
    analogWrite(LED_BUILTIN_LIGHT, 0);  
    
 #if (defined(BLINKER_APCONFIG)) || (defined(BLINKER_ESP_SMARTCONFIG))
     //启动配网模式用这行代码
    Blinker.begin(auth);
 #else 
    Blinker.begin(auth, ssid, pswd);
 #endif
    
    Blinker.attachData(dataRead);
    BlinkerAliGenie.attachPowerState(aligeniePowerState);//天猫语音操作注册函数
    BlinkerAliGenie.attachQuery(aligenieQuery);     
  
    Button1.attach(button1_callback);
    //注册回调函数
    Blinker.attachHeartbeat(heartbeat);
    
   //串口打印连接成功的IP地址
   Serial.println("连接成功");  
   Serial.print("IP:");
   Serial.println(WiFi.localIP());
}

void loop() {
    Blinker.run();//运行Blinker
    
    int state1 =  digitalRead(LED_BUILTIN_OPEN);
    Serial.println("state1:"); 
    Serial.println(state1);
    if(/*oAligenieState  == LOW && */state1 == HIGH){
       button1_callback(BLINKER_CMD_ON);
    }

    int state2 =  digitalRead(LED_BUILTIN_CLOSE);
    Serial.println("state2:"); 
    Serial.println(state2);
    if(/*oAligenieState == HIGH &&*/ state2 == HIGH){ 
       button1_callback(BLINKER_CMD_OFF);
    }

    static int lastms = 0;
    if (millis()-lastms > 30000) {
      lastms = millis();
      Serial.printf(PSTR("Running version %s for %d Free mem=%d\n"),version.c_str(), lastms/1000, ESP.getFreeHeap());
    }
}
