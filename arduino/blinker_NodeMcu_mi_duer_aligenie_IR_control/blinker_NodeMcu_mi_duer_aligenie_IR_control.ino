/*
NodeMcu控制1路继电器,配合红外接收和红外发射模块，实现红外学习和遥控功能，集成blinker平台，
用手机app控制1路继电器开关，添加一路物理开关控制，继电器用常开（NO）模式
 
v1.0
Created 2021
by 阳阳学编程
www.sooncore.com

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

获得具体连接电路图和更多资源，请参阅阳阳学编程网站 www.sooncore.com
   
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

//支持阳阳学编程的web wifi配网
#define YYXBC_WEBCONFIG

#include <Blinker.h>
#include <ESP8266WebServer.h>
//依赖库 LinkedList
//https://github.com/ivanseidel/LinkedList
#include <LinkedList.h>

#if (defined(YYXBC_WEBCONFIG))
  #include "wificfg.h"
#endif
//IRremoteESP8266
//
#include <TimeLib.h>
#include <IRsend.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <LittleFS.h>
#include <FS.h>

char auth[] = "c3329a614cbd";
char ssid[] = "panzujiMi10";
char pswd[] = "moto1984";

String version  = "1.0.4";

// uint16_t rawData[279] = {8998, 4478,  666, 540,  664, 540,  666, 540,  666, 540,  664, 540,  666, 540,  666, 540,  664, 540,  666, 1644,  662, 540,  664, 540,  664, 1642,  666, 540,  666, 540,  666, 542,  664, 542,  664, 540,  664, 540,  664, 542,  664, 540,  666, 540,  664, 1642,  668, 540,  664, 540,  664, 540,  666, 540,  666, 540,  666, 540,  664, 1644,  662, 542,  664, 1642,  666, 540,  666, 540,  666, 1642,  666, 540,  664, 19972,  664, 540,  666, 542,  664, 540,  666, 538,  666, 540,  664, 540,  666, 540,  666, 540,  664, 540,  664, 540,  666, 540,  666, 540,  666, 542,  664, 540,  664, 1640,  668, 542,  662, 540,  664, 540,  666, 540,  664, 540,  664, 540,  666, 540,  664, 540,  666, 540,  666, 540,  666, 540,  664, 542,  664, 540,  664, 1644,  664, 1644,  664, 1644,  664, 540,  666, 39982,  8996, 4482,  664, 540,  664, 540,  664, 542,  664, 540,  666, 540,  666, 542,  664, 540,  664, 542,  664, 1644,  664, 538,  666, 542,  664, 1644,  662, 540,  666, 542,  668, 536,  666, 540,  666, 540,  664, 540,  666, 542,  664, 540,  664, 540,  678, 1632,  664, 540,  664, 540,  666, 542,  664, 540,  664, 542,  664, 542,  662, 1644,  664, 1644,  664, 1642,  666, 540,  664, 542,  664, 1642,  666, 540,  666, 19974,  666, 538,  666, 540,  666, 540,  664, 540,  664, 540,  666, 538,  666, 540,  664, 540,  666, 540,  666, 538,  666, 540,  666, 540,  664, 540,  666, 540,  666, 540,  664, 540,  664, 540,  666, 538,  668, 540,  664, 542,  664, 542,  666, 540,  666, 540,  664, 542,  664, 540,  664, 540,  664, 540,  664, 540,  666, 1646,  662, 1642,  664, 542,  664, 540,  666};  // UNKNOWN 900
// uint16_t rawData[279] = {9024, 4478,  666, 540,  666, 538,  666, 542,  664, 542,  662, 540,  664, 540,  666, 540,  664, 542,  664, 1644,  664, 540,  666, 540,  664, 1644,  664, 540,  664, 542,  664, 540,  666, 540,  666, 538,  666, 540,  664, 542,  664, 542,  664, 540,  666, 1644,  666, 540,  666, 540,  664, 542,  664, 540,  664, 540,  664, 542,  664, 1644,  662, 540,  664, 1642,  672, 532,  666, 538,  666, 1642,  664, 540,  664, 19974,  668, 536,  666, 538,  666, 540,  666, 540,  664, 540,  666, 542,  664, 540,  666, 540,  666, 540,  664, 542,  662, 540,  666, 540,  664, 542,  664, 540,  666, 1642,  664, 540,  666, 538,  666, 540,  666, 544,  660, 542,  664, 540,  666, 540,  666, 540,  664, 540,  666, 542,  664, 542,  664, 540,  664, 542,  664, 1642,  666, 1642,  666, 1642,  664, 540,  666, 39982,  8998, 4478,  666, 540,  666, 540,  664, 540,  666, 540,  664, 542,  664, 540,  664, 540,  666, 540,  666, 1644,  664, 540,  666, 540,  666, 1642,  664, 540,  666, 540,  664, 542,  664, 542,  664, 540,  664, 540,  666, 540,  664, 540,  666, 538,  666, 1644,  664, 540,  666, 540,  666, 540,  664, 540,  664, 540,  664, 542,  664, 1642,  664, 1644,  666, 1640,  666, 538,  666, 540,  666, 1644,  664, 540,  666, 19974,  664, 540,  664, 540,  666, 540,  666, 540,  666, 540,  666, 540,  664, 540,  666, 540,  664, 540,  666, 540,  666, 540,  664, 540,  664, 540,  666, 540,  664, 540,  666, 540,  666, 540,  664, 540,  666, 540,  666, 540,  666, 540,  666, 540,  666, 542,  664, 540,  666, 540,  664, 542,  664, 540,  666, 540,  666, 1642,  666, 1642,  670, 536,  666, 538,  666};  // PANASONIC 900
//Esp-01/01s，继电器接GPIO0,物理开关接GPIO2
//
//#define LED_BUILTIN_LIGHT 0
//#define LED_BUILTIN_K2 2

//NodeMCU 继电器接D3,物理开关接D4
#define LED_BUILTIN_LIGHT D3
#define LED_BUILTIN_K2 D4

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

//红外指令数据格式
struct RawData{
  uint16_t bits;
  uint16_t nIndex;
  uint16_t nSize;
  uint16_t* data;
  RawData():nIndex(0),nSize(0),
  data(NULL){}
};
RawData* pCurrentRawData = NULL;
//上次按钮被长按下去时的时间，判断长按
LinkedList<int> lastpresstime;
//当前语音助手控制的按钮
int control_by_call_btn = 0;
//debug 模式
bool is_debug = false;
//当前调式的红外指令对应的按钮
int index_debug_btn = 0;
//红外指令列表
LinkedList<RawData*> myLinkedList;
//上一次发送指令的时间
long lastSendIR =  millis();
//接收和发送红外指令个数
long sendIRCount = 0;
long recvIRCount = 0;
//433
bool stu = 0;
char code[30];
// Blinker.vibrate()
// 发送手机振动指令, 震动时间, 单位ms 毫秒, 数值范围0-1000, 默认为500
// Blinker.vibrate();
// Blinker.vibrate(255);
#define TESTFS LittleFS
// ==================== start of TUNEABLE PARAMETERS ====================
//IR send  pin,esp8266-01/01s use 2,nodemcu use 14(D5);
const uint16_t kRecvPin = 14;
//IR send  pin,esp8266-01/01s use 0,nodemcu use 4(D2);
const uint16_t kSendPin = 4;
// The Serial connection baud rate.
const uint32_t kBaudRate = 115200;
// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
const uint16_t kCaptureBufferSize = 1024;

#if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t kTimeout = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif  // DECODE_AC
// Alternatives:
const uint16_t kMinUnknownSize = 12;
// Legacy (No longer supported!)
// Change to `true` if you miss/need the old "Raw Timing[]" display.
#define LEGACY_TIMING_INFO false
// ==================== end of TUNEABLE PARAMETERS ====================

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);

// 新建组件对象
#define BlinkerButton_Callback_Def(index,name)             \
  BlinkerButton Button##index(name);         \
  void button_callback##index(const String & state) { buttonx_callback(index,state);}

#define BlinkerButton_Attach_Def(index)             \
    Button##index.attach(button_callback##index);

BlinkerButton_Callback_Def(0,"btn-0");
BlinkerButton_Callback_Def(1,"btn-1");
BlinkerButton_Callback_Def(2,"btn-2");
BlinkerButton_Callback_Def(3,"btn-3");
BlinkerButton_Callback_Def(4,"btn-4");
BlinkerButton_Callback_Def(5,"btn-5");
BlinkerButton_Callback_Def(6,"btn-6");
BlinkerButton_Callback_Def(7,"btn-7");
BlinkerButton_Callback_Def(8,"btn-8");
BlinkerButton_Callback_Def(9,"btn-9");
BlinkerButton_Callback_Def(10,"btn-10");

BlinkerNumber NUMR("num-irr");
BlinkerNumber NUMS("num-irs");
//webserver for siri
static ESP8266WebServer esp8266_server(80);

//是否处理联网状态
bool isNetConnected(){return (WiFi.status() == WL_CONNECTED);}

//心跳回调
void heartbeat()
{
   BLINKER_LOG("heartbeat,state: ", digitalRead(LED_BUILTIN));

    //较正app的按钮状态
    if(YYXBC_HIGH == digitalRead(LED_BUILTIN_LIGHT) ){
      getBlinkerButton(0).print("on");
    }else{
      getBlinkerButton(0).print("off");
    }
}

void resetBlinkerButton(){
  for(int  i = 1;i<11;i++){
    getBlinkerButton(i).print("off");
  }
}

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
    case 5:
      return Button5;
    case 6:
      return Button6;
    case 7:
      return Button7;
    case 8:
      return Button8;
    case 9:
      return Button9;
    case 10:
      return Button10;
  }
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void freeRawData(RawData * pRawData){
  if(pRawData){
    if(pRawData->data){
      delete[] pRawData->data;
    }
    delete pRawData;
    pRawData = NULL;
  }

}

//绑定当前指令到指令按钮上
void bindRICodeToBlinkerButton(const int& index,const BlinkerButton& btn){
  Serial.println("红外指令和按钮绑定 index:"+ index );
  if(pCurrentRawData && pCurrentRawData->nSize > 0){
      RawData *tempRawData_ = myLinkedList.get(index);
      if(tempRawData_){
        Serial.println("红外指令已经存在，准备替换");
        myLinkedList.remove(index);
        //先释放原来指令的内存
        freeRawData(tempRawData_);
      }
      //保存新的指令
      tempRawData_   =  new RawData();
      tempRawData_->bits = pCurrentRawData->bits;
      tempRawData_->nIndex = index;
      tempRawData_->nSize = pCurrentRawData->nSize;
      tempRawData_->data = new uint16_t[pCurrentRawData->nSize];
      memcpy((void*)tempRawData_->data,(void*)pCurrentRawData->data,pCurrentRawData->nSize * sizeof(uint16_t));
      myLinkedList.add(index, tempRawData_);
      Serial.println("红外指令绑定完成");

  }else{
     BLINKER_LOG("invalid code, bindCurrentRICode: ", index);
  }
}

bool saveConfig(FS *fs){
  if (!fs->begin()) {
    Serial.println("Unable to begin(), aborting");
    return false;
  }
  Serial.println("Creating file, may take a while...");
  long start = millis();
  File f = fs->open("/irconfig.bin", "w");
  if (!f) {
    Serial.println("Unable to open file for writing, aborting");
    return false;
  }
  int nSize = myLinkedList.size();
  int nwrite = f.write((char*)&nSize, sizeof(uint16_t));
  // assert(nwrite != sizeof(uint16_t));
  for(int i = 0;i< nSize ;i++){
    RawData* pRawData = myLinkedList.get(i);
    if(pRawData){
      f.write((char*)&pRawData->bits, sizeof(uint16_t));
      f.write((char*)&pRawData->nIndex, sizeof(uint16_t));
      f.write((char*)&pRawData->nSize, sizeof(uint16_t));
      f.write((char*)pRawData->data, pRawData->nSize * sizeof(uint16_t));
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
  File f = fs->open("/irconfig.bin", "r");
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
      f.read((uint8_t*)&pRawData->nIndex, sizeof(uint16_t));
      f.read((uint8_t*)&pRawData->nSize, sizeof(uint16_t));
      Serial.println(pRawData->bits);
      Serial.println(pRawData->nIndex);
      Serial.println(pRawData->nSize);
      pRawData->data = new uint16_t[pRawData->nSize];
      if(pRawData->data){
        f.read((uint8_t*)pRawData->data, pRawData->nSize * sizeof(uint16_t));
        myLinkedList.add(pRawData->nIndex,pRawData);
      }
    }
  }
  f.close();
  long stop = millis();
  Serial.printf("==> Time to write  chunks = %ld milliseconds\n", stop - start);
  return true;
}

void button0_callback(const String & state) { 
      if (state == BLINKER_CMD_ON) {
        BLINKER_LOG("Toggle on!");
        if(isNetConnected())getBlinkerButton(0).print("on");
        digitalWrite(LED_BUILTIN_LIGHT, YYXBC_HIGH);
    }
    else if (state == BLINKER_CMD_OFF) {
        BLINKER_LOG("Toggle off!");
        if(isNetConnected())getBlinkerButton(0).print("off");
        digitalWrite(LED_BUILTIN_LIGHT, YYXBC_LOW);
    }
}

void buttont_callback(const String & state) { 
      if (state == BLINKER_CMD_ON) {
        BLINKER_LOG("Toggle on!");
        RawData*  pRawData = myLinkedList.get(1);
        if(pRawData){
          //rawData
          SendIrMsg(pRawData->data,pRawData->nSize);
          dataStorage(pRawData->bits);
        }else{
          BLINKER_LOG("还没有绑定指令,快去绑定指令吧！");
       }

        oDuerState = YYXBC_HIGH;
        oMioState =YYXBC_HIGH;
        oAligenieState = YYXBC_HIGH;
    }
    else if (state == BLINKER_CMD_OFF) {
        BLINKER_LOG("Toggle off!");
        RawData*  pRawData = myLinkedList.get(2);
        if(pRawData){
          //rawData
          SendIrMsg(pRawData->data,pRawData->nSize);
          dataStorage(pRawData->bits);
        }else{
          BLINKER_LOG("还没有绑定指令,快去绑定指令吧！");
       }
        oDuerState = YYXBC_LOW;
        oMioState = YYXBC_LOW;
        oAligenieState = YYXBC_LOW;
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

void buttonx_callback( const uint16_t index, const String & state) {
    BLINKER_LOG("get button state: ", state);
    BLINKER_LOG("index： ", index);


    if (state == BLINKER_CMD_BUTTON_PRESSED){
        BLINKER_LOG("Button pressed!");
        lastpresstime.set(index ,millis());
    }
    else if (state == BLINKER_CMD_BUTTON_TAP){
        BLINKER_LOG("Toggle tap!");
        if(is_debug  && index_debug_btn == index ){
            BLINKER_LOG("IR test is_debug = true");
            SendIrMsg((uint16_t*)pCurrentRawData->data,pCurrentRawData->nSize);
            // printRawData(pCurrentRawData);
        }else{
            BLINKER_LOG("IR test is_debug = false");
            RawData*  pRawData = myLinkedList.get(index);
            if(pRawData){
              //rawData
              SendIrMsg(pRawData->data,pRawData->nSize);
              dataStorage(pRawData->bits);
              // printRawData(pRawData);
            }else{
              BLINKER_LOG("还没有绑定指令,快去绑定指令吧！");
            }
        }
    }
    else if (state == BLINKER_CMD_BUTTON_PRESSUP){
      int last = lastpresstime.get(index);
      BLINKER_LOG("Button pressup,lastpresstime(%d)!",last);
      //判断是否是长按5秒
      if(millis() - lastpresstime.get(index) > 5000){
        index_debug_btn  = index;
        is_debug = !is_debug;
        lastpresstime.set(index , millis());
        if(is_debug){
          resetBlinkerButton();
          getBlinkerButton(index).icon("fal fa-siren-on");
          getBlinkerButton(index).color("#FF4500");
          getBlinkerButton(index).text("学习模式");
          getBlinkerButton(index).print();
          Blinker.vibrate();
        }else{
          //红外指令和blinke 的按钮绑定
          bindRICodeToBlinkerButton(index,getBlinkerButton(index));
          saveConfig(&TESTFS);
          getBlinkerButton(index).print("off");
          Blinker.vibrate();
        }
      }
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果小度 有对设备进行操作就执行下面
void duerPowerState(const String & state)
{
    BLINKER_LOG("duerPowerState need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
       button_callback1(BLINKER_CMD_ON);
    }
    else if (state == BLINKER_CMD_OFF) {
      button_callback1(BLINKER_CMD_OFF);
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
       button_callback1(BLINKER_CMD_ON);
    }
    else if (state == BLINKER_CMD_OFF) {
      button_callback1(BLINKER_CMD_OFF);
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
       button_callback1(BLINKER_CMD_ON);
    }
    else if (state == BLINKER_CMD_OFF) {
      button_callback1(BLINKER_CMD_OFF);
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

void setup() {
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
    BlinkerDuerOS.attachPowerState(duerPowerState); //小度语音操作注册函数
    BlinkerDuerOS.attachQuery(duerQuery);

    BlinkerMIOT.attachPowerState(miotPowerState);//小爱语音操作注册函数
    BlinkerMIOT.attachQuery(miotQuery);

    BlinkerAliGenie.attachPowerState(aligeniePowerState);//天猫语音操作注册函数
    BlinkerAliGenie.attachQuery(aligenieQuery);     
  
    BlinkerButton_Attach_Def(0);
    BlinkerButton_Attach_Def(1);
    BlinkerButton_Attach_Def(2);
    BlinkerButton_Attach_Def(3);
    BlinkerButton_Attach_Def(4);
    BlinkerButton_Attach_Def(5);
    BlinkerButton_Attach_Def(6);
    BlinkerButton_Attach_Def(7);
    BlinkerButton_Attach_Def(8);
    BlinkerButton_Attach_Def(9);
    BlinkerButton_Attach_Def(10);

    //注册回调函数
    Blinker.attachHeartbeat(heartbeat);
    
    //启动webserver ,提供接口给siri用
    esp8266_server.on("/", handleRoot); 
    esp8266_server.on("/post", handleSetConfig);           
    esp8266_server.onNotFound(handleNotFound);  
    esp8266_server.begin();                  
      
    Serial.println("HTTP esp8266_server started");

    //load IR configure
    loadConfig(&TESTFS);
    int nSize = myLinkedList.size();
    for(int i = 0;i< nSize ;i++){
      decode_results results;
      RawData* pRawData = myLinkedList.get(i);
      if(pRawData){
        results.bits = pRawData->bits;
        results.rawlen = pRawData->nSize;
        results.rawbuf = pRawData->data;
        print_decode_results(results);
      }
    }

  #if DECODE_HASH
    // Ignore messages with less than minimum on or off pulses.
    irrecv.setUnknownThreshold(kMinUnknownSize);
  #endif  // DECODE_HASH
    irrecv.enableIRIn();  // Start the receiver

//   //443
//   init443();

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
    esp8266_server.handleClient();// 处理http服务器访问
    
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

    //接收IR指令
    recvIR();


}
//
//void init443(){
//   mySwitch.enableTransmit(D4);   //定义433发射模块的接口位D4
//   mySwitch.enableReceive(D3);    //定义433接受模块的接口位D2
//    // pinMode(D0, OUTPUT);
//    // digitalWrite(D0, HIGH);
//}
//
//void recv443(){
//    if (stu == 1){
//      if (mySwitch.available()) {
//        Serial.print("Received ");
//        Serial.print( mySwitch.getReceivedValue() );
//        Serial.print(" / ");
//        Serial.print( mySwitch.getReceivedBitlength() );
//        // Textcode.print(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength());
//        Serial.print("bit ");
//        Serial.print("Protocol: ");
//        Serial.println( mySwitch.getReceivedProtocol() );
//        mySwitch.resetAvailable();
//
//      }
//    }
//}

void recvIR(){
  // Check if the IR code has been received.
  decode_results results;  // Somewhere to store the results
  if (irrecv.decode(&results)) {
    Serial.println("接收指令:");
    print_decode_results(results);
    long nowtime =  millis();
    if(results.decode_type != UNKNOWN && nowtime - lastSendIR > 2000)
    {
       if(pCurrentRawData != NULL) {
          //先释放原来指令的内存
          freeRawData(pCurrentRawData);
       }

      // SendIrMsg((uint16_t*)results.rawbuf,results.rawlen);
      // Serial.print("{");
  
      // for(int i = 0;i< results.rawlen;i++){
      //   Serial.print(results.rawbuf[i]);
      //   if(i != results.rawlen - 1 )
      //      Serial.print(",");
      // }
      // Serial.print("}");

      // Serial.println(resultToSourceCode1(&results));

      LinkedList<uint16_t> sourceCode;
      resultToRawData(&results,sourceCode);

      // Serial.print("{");
      // for(int i = 0;i< sourceCode.size();i++){
      //   Serial.print(sourceCode.get(i));
      //   if(i != sourceCode.size() - 1 )
      //      Serial.print(",");
      // }
      // Serial.print("}");

      //保存新的指令
      pCurrentRawData   =  new RawData();
      pCurrentRawData->bits = results.value;
      pCurrentRawData->nIndex = 0;
      pCurrentRawData->nSize = sourceCode.size();
      pCurrentRawData->data = new uint16_t[sourceCode.size()];
      for(int i = 0;i< sourceCode.size();i++){
         pCurrentRawData->data[i] = sourceCode.get(i);
      }
      // memcpy((void*)pCurrentRawData->data,(void*)results.rawbuf,results.rawlen * sizeof(uint16_t));
      recvIRCount++;
      NUMR.print(recvIRCount);
      dataStorage(results.value);
      Serial.println("已经保存为测试指令");
    }
  }
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
        //开关btn-abc
        if(btnName == "btn-1"){
          if(btnState == "on"){
             button_callback1( BLINKER_CMD_ON);
          }else if (btnState == "off"){
             button_callback1( BLINKER_CMD_OFF);
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
        button_callback1(BLINKER_CMD_OFF);
        Serial.println("按钮对灯已执行关闭");
      }else{
        button_callback1(BLINKER_CMD_ON);
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
    if (is_led == YYXBC_HIGH)
    {
      button_callback1(BLINKER_CMD_OFF);
      Serial.println("按钮对灯已执行关闭");
    }
    else
    {
      button_callback1(BLINKER_CMD_ON);
      Serial.println("按钮对灯已执行打开");
    }
    is_btn = digitalRead(LED_BUILTIN_K2);  //更新按钮状态
    return true;
  }
 return false;
}

// /// Return a String containing the key values of a decode_results structure
//  String resultToSourceCode1(const decode_results * const results) {
//   String output = "";
//   // Reserve some space for the string to reduce heap fragmentation.
//   output.reserve(1536);  // 1.5KB should cover most cases.
//   // Start declaration
//   output += F("uint16_t ");  // variable type
//   output += F("rawData[");   // array name
//   output += uint64ToString(getCorrectedRawLength(results), 10);
//   // array size
//   output += F("] = {");  // Start declaration

//   // Dump data
//   for (uint16_t i = 1; i < results->rawlen; i++) {
//     uint32_t usecs;
//     for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX; usecs -= UINT16_MAX) {
//       output += uint64ToString(UINT16_MAX);
//       if (i % 2)
//         output += F(", 0,  ");
//       else
//         output += F(",  0, ");
//     }
//     output += uint64ToString(usecs, 10);
//     if (i < results->rawlen - 1)
//       output += kCommaSpaceStr;            // ',' not needed on the last one
//     if (i % 2 == 0) output += ' ';  // Extra if it was even.
//   }

//   // End declaration
//   output += F("};");
//   return output;
// }

void print_decode_results(const decode_results& results){
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_ "\n");
    // Display the basic output of what we found.
    String basic =  resultToHumanReadableBasic(&results);
    basic.replace(" ","");
    Serial.print(basic.c_str());
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    description.replace(" ","");
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
#if LEGACY_TIMING_INFO
    // Output legacy RAW timing info of the result.
    String results = resultToTimingInfo(&results);
    Serial.println(results.c_str());
    yield();  // Feed the WDT (again)
#endif  // LEGACY_TIMING_INFO
    // Output the results as source code
    String sourcecode = resultToSourceCode(&results);
    Serial.println(sourcecode.c_str());
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
}

void printRawData(RawData* pRawData){
  if(pRawData){
    decode_results results;
    results.rawlen = pRawData->nSize;
    results.value = pRawData->bits;
    results.rawbuf = (uint16_t *)pRawData->data;
    Serial.println("发送指令:");
    print_decode_results(results);
  }

}

void SendIrMsg( const uint16_t* rawbuf, uint16_t nSize){
  if(rawbuf && nSize >0 ){
    IRsend irsend(kSendPin);  // Set the GPIO （D2）to be used to sending the message.
    irsend.begin();
    Serial.printf("RawData length: %u",nSize);
    Serial.println("#");
    irsend.sendRaw(rawbuf, nSize, 38);
   
    lastSendIR =  millis();
    sendIRCount++;
    NUMS.print(sendIRCount);
  }else{
    Serial.print("illegal RawData");
  }
}

//统计数据
void dataStorage(const uint16_t& bits){
  int nSize = myLinkedList.size();
  // assert(nwrite != sizeof(uint16_t));
  for(int i = 0;i< nSize ;i++){
    RawData* pRawData = myLinkedList.get(i);
    if(pRawData){
       if(bits == pRawData->bits){
          Blinker.dataStorage("sensor", pRawData->nIndex);
          Serial.println("Code:");
          Serial.println(pRawData->bits);
          Serial.print("保存操作历史：btn-");
          Serial.println(pRawData->nIndex);
          break;
       }
    }
  }

}


/// Return a String containing the key values of a decode_results structure
 void resultToRawData(const decode_results * const results,LinkedList<uint16_t>& sourceCode) {
  // int nLength = getCorrectedRawLength(results);
  // pRawData->nSize = nLength;
  // Dump data
  for (uint16_t i = 1; i < results->rawlen; i++) {
    uint32_t usecs;
    for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX; usecs -= UINT16_MAX) {
      sourceCode.add(UINT16_MAX);
    }
     sourceCode.add(usecs);
  }
}
