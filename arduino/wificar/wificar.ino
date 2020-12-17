//电机控制引脚
#define LEFT_ENA     14 //Enable/speed motors Right GPIO14(D5)
#define RIGHT_ENB    12 //Enable/speed motors Left  GPIO12(D6)
#define RIGHT_MOTOR1 15 //L298N in1 motors Right    GPIO15(D8)
#define RIGHT_MOTOR2 13 //L298N in2 motors Right    GPIO13(D7)
#define LEFT_MOTOR1  2  //L298N in3 motors Left     GPIO2(D4)
#define LEFT_MOTOR2  0  //L298N in4 motors Left     GPIO0(D3)

//控制命令
#define        STOP            0
#define        FORWARD         1
#define        BACKWARD        2
#define        TURNLEFT        3
#define        TURNRIGHT       4



#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// 在这里填入WiFi名称
const String ssid = "NodeMCU Car";
// 在这里填入WiFi密码
const String passwd = "123456";

int highSpeed = 800;// 400 -1023
int lowSpeed = 400;

// 初始化WebSocketsServer对象
WebSocketsServer server(81);

//https://www.arduino.cn/thread-93593-1-1.html
IPAddress myIP(192,168,4,1);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
   
  pinMode(LEFT_MOTOR1, OUTPUT);
  pinMode(LEFT_MOTOR2, OUTPUT);
  pinMode(RIGHT_MOTOR1, OUTPUT);
  pinMode(RIGHT_MOTOR2, OUTPUT);
  pinMode(LEFT_ENA,OUTPUT);
  pinMode(RIGHT_ENB,OUTPUT);
   
  motor_run(STOP);
  
//  // 设置WiFi运行模式为无线终端模式
//  WiFi.mode(WIFI_STA);
//  // 为当前设备配置固定IP
//  if (!WiFi.config(host, gateway, netmask)) {
//    Serial.println("Can't config wifi.");
//  }
//  Serial.println("Connecting to " + ssid);
//  // 连接WiFi
//  WiFi.begin(ssid, passwd);
//  // 判断是否连接成功
//  while (WiFi.status() != WL_CONNECTED) {
//    Serial.print('.');
//    delay(100);
//  }
//  Serial.println("");
//  Serial.println("Connected to " + ssid);
//  Serial.print("Current ip address is ");
//  Serial.println(WiFi.localIP());

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid,passwd);
  WiFi.softAPConfig(myIP,myIP,IPAddress(255,255,255,0));
//  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(myIP); 


  // 指定事件处理函数
  server.onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t length){
    if (type == WStype_CONNECTED) {
      // 若为客户端连接事件，显示提示信息
      Serial.println("New connection!");
    } else if (type == WStype_DISCONNECTED) {
      // 若为连接断开事件，显示提示信息
      Serial.println("Close the connection.");
    } else if (type == WStype_TEXT) {
      // 接收来自客户端的信息（客户端FLASH按键状态），并控制LED的工作
//      String data((char*)payload,length);
      String data = (char*)payload;
      Serial.println(data); 
      jsonDataHandler(data);
    }
  });
  
  // 启动WebSocket服务器
  server.begin();
}

// 通过IP地址获取客户端编号
int8_t get_num_from_ip(IPAddress ip_needed) {
    for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        IPAddress ip = server.remoteIP(i);
        if(ip == ip_needed) {
           // ip found
           return i;
        }
    }
    return -1;
}

void loop() {
  // put your main code here, to run repeatedly:
  server.loop();
  //heartbeat message
  static int lastms = 0;
    if (millis()-lastms > 30000) {
      lastms = millis();
      char szdata[256];
      sprintf(szdata,PSTR("Running  for %d Free mem=%d\n"), lastms/1000, ESP.getFreeHeap());
      for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        IPAddress ip = server.remoteIP(i);
        server.sendTXT(i, szdata);
      }
    }

}

void jsonDataHandler(String jsonstr){
  DynamicJsonDocument doc(1024);
  auto error = deserializeJson(doc, jsonstr);
  if (error) {
    Serial.println("Failed to parse cmd from app");
    return ;
  }
  JsonObject obj = doc.as<JsonObject>();
  String cmd = obj["cmd"];
  if(cmd == "forward"){
     motor_run(FORWARD);
  }else if (cmd == "back"){
     motor_run(BACKWARD);
  }else if (cmd == "left"){
     motor_run(TURNLEFT);
  }else if (cmd == "right"){
    motor_run(TURNRIGHT);
  }else if (cmd == "stop"){
    motor_run(STOP);
  }else{
    Serial.println("not surport this cmd from app");
  }
}


void motor_run(int cmd)
{
  switch(cmd)
  {
  case FORWARD:
    Serial.println("FORWARD"); //输出状态
    analogWrite(LEFT_ENA,highSpeed);
    digitalWrite(LEFT_MOTOR1, LOW);
    digitalWrite(LEFT_MOTOR2, HIGH);
    analogWrite(RIGHT_ENB,highSpeed);
    digitalWrite(RIGHT_MOTOR1, LOW);
    digitalWrite(RIGHT_MOTOR2, HIGH);
  break;
  case BACKWARD:
    Serial.println("BACKWARD"); //输出状态
    digitalWrite(LEFT_MOTOR1,  HIGH);
    digitalWrite(LEFT_MOTOR2, LOW);
    digitalWrite(RIGHT_MOTOR1, HIGH);
    digitalWrite(RIGHT_MOTOR2, LOW);
  break;
  case TURNLEFT:
    Serial.println("TURN  LEFT"); //输出状态
    analogWrite(LEFT_ENA,lowSpeed);
    analogWrite(RIGHT_ENB,highSpeed);
    digitalWrite(LEFT_MOTOR1, LOW);
    digitalWrite(LEFT_MOTOR2, HIGH);//C
    digitalWrite(RIGHT_MOTOR1, LOW);
    digitalWrite(RIGHT_MOTOR2, HIGH);
  break;
  case TURNRIGHT:
    Serial.println("TURN  RIGHT1"); //输出状态
    analogWrite(LEFT_ENA,highSpeed);
    analogWrite(RIGHT_ENB,lowSpeed);
    digitalWrite(LEFT_MOTOR1, LOW);
    digitalWrite(LEFT_MOTOR2, HIGH);
    digitalWrite(RIGHT_MOTOR1, LOW);//C
    digitalWrite(RIGHT_MOTOR2, HIGH);
  break;
  case STOP:
  default:
    Serial.println("STOP"); //输出状态
    digitalWrite(LEFT_ENA,1);
    digitalWrite(RIGHT_ENB,1);
    digitalWrite(LEFT_MOTOR1, LOW);
    digitalWrite(LEFT_MOTOR2, LOW);
    digitalWrite(RIGHT_MOTOR1, LOW);
    digitalWrite(RIGHT_MOTOR2, LOW);
  break;
  }
}
