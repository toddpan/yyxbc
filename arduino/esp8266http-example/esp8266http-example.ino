#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 

#include <WiFiClientSecureBearSSL.h>  
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

/**
 * 
 * 请求类型
请求信息类型
— begin – 设置请求URL
语法
httpClient.begin(url)
httpClient.begin(url, port)

参数
url: HTTP请求网址
port: HTTP请求的网络服务器端口 (此参数为可选参数，默认为端口80)

返回值
无

— setReuse – 设置请求头中的keep-alive
如果设置 keep-alive为true，则服务端在返回 response 后不关闭 TCP 连接；同样的，在接收完响应报文后，客户端也不关闭连接，发送下一个 HTTP 请求时会重用该连接。

语法
httpClient.setReuse（true）
httpClient.setReuse（false）

参数
返回值类型为布尔型。返回值的简要说明如下：
true: 启用请求头中的keep-alive
false: 禁用请求头中的keep-alive

返回值
无


— setUserAgent – 设置请求头中的User-Agent
User Agent请求头的作用是让服务器能够识别客户端使用的操作系统及版本、客户端设备的CPU类型、浏览器名称和版本、浏览器所用的语言、浏览器所用的插件等。换句话说,就是告诉服务端现在是什么设备在访问服务器。

语法
httpClient.setUserAgent（device_info）

参数
– device_info : 用户自定义的客户端标识
– 参数类型: 字符串 或 const char *

返回值
无
— setAuthorization – 设置请求头中的Authorization
说明
此函数用于设置ESP8266开发板在通过HTTP发送请求时，设置请求头中的Authorization部分。

语法
httpClient.setAuthorization(user,password)

参数
user: 设备进行访问的用户名请求头信息

password: 设备进行访问的密码请求头信息

返回值
无


— addHeader – 请求头中添加自定义信息
说明
使用此函数，我们可以ESP8266开发板发起HTTP请求的时候,添加自定义的请求头信息。

语法
http.addHeader(name,value)

参数
name: 自定义请求头名称 (类型：String 或 const char *)
value: 自定义请求头参数值 (类型：String 或 const char *)

注：自定义请求头不能使用Connection、User-Agent、Host、Authorization等标准请求头名称。

返回值
无
— useHTTP10 – 设置http协议版本
说明
此函数用于设置ESP8266开发板在通过HTTP发送请求时使用的HTTP协议版本。

语法
http.useHTTP10(val)

参数
val: 控制HTTTP版本。默认情况下，设备时使用HTTTP 1.1 版本。当此参数为true时，则使用HTTTP 1.0版本。false时，使用HTTTP 1.1版本。（参数类型：bool）

返回值
无

请求方法类型
— GET
— POST
— PUT
— PATCH

发送请求类型
— sendRequest – 发送请求
  说明
  此函数用于ESP8266使用HTTP协议通过网络向服务器发送请求。本函数可以自定义请求方法类型，如 GET、POST等。
  
  语法
  sendRequest(type, payload)
  sendRequest(type, payload, size)
  sendRequest(type, stream, size)
  
  参数
  type: 发送请求的类型,可以是”GET”、”POST”等。参数类型: const char *
  
  payload: 请求所发送的数据信息（该信息置于请求体中被发送）。该参数可使用字符串类型。（使用GET作为请求方法时，此参数可以省略）。
  
  size: 请求所发送的数据字节数(此参数为可选参数，类型：size_t )。
  
  stream: 数据流对象
  
  返回值
  服务器状态码
  
  以下是常见的HTTP状态码：
  – 200 – 请求成功
  – 301 – 资源（网页等）被永久转移到其它URL
  – 404 – 请求的资源（网页等）不存在
  – 500 – 内部服务器错误

— end – 结束请求
  语法
  http.end()
  
  参数
  无
  
  返回值
  无

响应类型
响应头类型
— collectHeaders – 设置收集响应头内容
  语法
  httpClient.collectHeaders(headerKeys, headerKeysCount)
  
  参数
  – headerKeys: 响应头名数组。此数组中的元素即是需要ESP8266处理或收集的响应头信息。下面我们看一个headerKeys数组的例子：
  const char *headerKeys[] = {"Content-Length", "Date"};
  此数组共有两个元素，分别是”Content-Length”和”Date”。只有如此建立了数组后，接下来的程序里我们才可以使用其它ESP8266HTTPClient库函数来处理这两个响应头的信息内容。可以把这个数组看作是对ESP8266的提醒，即告诉ESP8266在后续的程序中，我们将要对数组中所包含的响应头信息进行处理。（具体操作方法，请见本函数的示例程序部分。）
  
  – headerKeysCount: 需要获取/处理的响应头个数（可选参数，类型：size_t）
  
  返回值
  无
  
— header – 获取指定响应头参数值
  语法
  httpClient.header(name)
  httpClient.header(index)
  
  参数
  name: 响应头中指定项的参数名(参数类型: const char*)
  index: 响应头中指定项的序号(参数类型: size_t)
  
  返回值
  响应头中指定项的参数值 (返回值类型:String)

  const char *headerKeys[] = {"Content-Length", "Date"};
  //通过headerKeys数组设置即将处理的服务器响应头内容
  httpClient.collectHeaders(headerKeys, 2);

   Serial.print("Content-Length = "); 
  Serial.println(httpClient.header("Content-Length"));
  
— headers – 获取响应头数量
int headerQuantity = httpClient.headers();
 
— headerName – 获取指定的响应头名
  String headerKey = httpClient.headerName(0);
  
— hasHeader – 确认是否存在指定响应头
  if(httpClient.hasHeader("Date")){
    Serial.println("==========================");
    Serial.println("Server Response has Date Header");      
    Serial.println("==========================");
  }

响应体类型
— getString – 获取响应体数据并且以字符串形式返回
  //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  //如果服务器不响应OK则将服务器响应状态码通过串口输出
  if (httpCode == HTTP_CODE_OK) {
    String responsePayload = httpClient.getString();
    Serial.println("Server Response Payload: ");
    Serial.println(responsePayload);
  } else {
    Serial.println("Server Respose Code：");
    Serial.println(httpCode);
  }
— getStream – 获取响应体数据并且以Stream形式返回
 //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  //如果服务器不响应OK则将服务器响应状态码通过串口输出
  if (httpCode == HTTP_CODE_OK) {
    
    // 获取服务器响应体Stream并传递给wifiClient
    wifiClient = httpClient.getStream();
    
    Serial.println("Server Response: ");
    // 以下部分使用了Stream类中的available函数以及readStringUntil函数
    // 对服务器响应体信息进行分析处理。并且通过串口监视器将服务器响应体信息
    // 输出到串口监视器中。
    while (wifiClient.available()){
      String resonseBodyLine = wifiClient.readStringUntil('\n');
      Serial.println(resonseBodyLine);
    }
  } else {
    Serial.println("Server Respose Code：");
    Serial.println(httpCode);
  }
 
— writeToStream – 获取响应体数据并且写入其它Stream对象
  int ret =  httpClient.writeToStream(stream)
   返回值
    返回值类型为int。
    如果返回值大于零，则返回值数值是写入对象的数据大小（单位：字节）。
    如果返回值小于零，说明函数执行出错。以下是错误代码说明：
  -1： HTTPC_ERROR_CONNECTION_REFUSED
  -2： HTTPC_ERROR_SEND_HEADER_FAILED
  -3： HTTPC_ERROR_SEND_PAYLOAD_FAILED
  -4： HTTPC_ERROR_NOT_CONNECTED
  -5： HTTPC_ERROR_CONNECTION_LOST
  -6： HTTPC_ERROR_NO_STREAM
  -7： HTTPC_ERROR_NO_HTTP_SERVER
  -8： HTTPC_ERROR_TOO_LESS_RAM
  -9： HTTPC_ERROR_ENCODING
  -10： HTTPC_ERROR_STREAM_WRITE
  -11： HTTPC_ERROR_READ_TIMEOUT
— getSize – 获取响应体数据字节数
  int nlen = httpClient.getSize()
  返回值
  返回值类型为int。
  服务器响应中的响应体大小（单位：字节）
— errorToString – 获取错误代码并且以字符串形式返回
  String errstr = httpClient.errorToString(httpCode)
  参数
  httpCode:服务器错误代码。
  注意，此错误代码非HTTP响应状态码，而是ESP8266库自定义的一系列错误代码。以下是错误代码数值以及该数值所对应的错误信息。
  
  -1： HTTPC_ERROR_CONNECTION_REFUSED
  -2： HTTPC_ERROR_SEND_HEADER_FAILED
  -3： HTTPC_ERROR_SEND_PAYLOAD_FAILED
  -4： HTTPC_ERROR_NOT_CONNECTED
  -5： HTTPC_ERROR_CONNECTION_LOST
  -6： HTTPC_ERROR_NO_STREAM
  -7： HTTPC_ERROR_NO_HTTP_SERVER
  -8： HTTPC_ERROR_TOO_LESS_RAM
  -9： HTTPC_ERROR_ENCODING
  -10： HTTPC_ERROR_STREAM_WRITE
  -11： HTTPC_ERROR_READ_TIMEOUT
 * 
 */ 

//http://www.taichi-maker.com/homepage/iot-development/iot-dev-reference/esp8266-c-plus-plus-reference/esp8266httpclient/
 
//WiFi连接信息（注意：需要自行修改以下内容否则ESP8266无法连接WiFi）
const char ssid[] = "panzujiMi10";  //WiFi名panzujiMi10
//const char ssid[] = "Xiaomi_10EE";    //WiFi名
const char password[] = "moto1984";   //WiFi密码
 
//测试HTTPS通讯的网站
const String HttpsGetURL = "https://www.iesdouyin.com/web/api/v2/user/info/?sec_uid=MS4wLjABAAAAonhiVxXz8CqcZ7UCo54jOPkKTNEGz5qVJufj5rfsfCo";
const String HttpsPostURL = "https://baidu.com";
const String HttpGetURL = "http://baidu.com";
const String HttpPostURL = "http://baidu.com";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);        //设置ESP8266为无线终端工作模式
  
  WiFi.begin(ssid, password); //连接WiFi
  Serial.println("");
 
  Serial.println("Connecting"); Serial.println("");
  
  // 等待连接
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  //成功连接后通过串口监视器显示WiFi名称以及ESP8266的IP地址。
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
}
 
void loop(){
    // 实现HTTPS通讯
  https_get();
  delay(30000);
//  httpCom();
}


void http_post(String jsonstr) {
 
  //创建 WiFiClient 实例化对象
  WiFiClient client;
 
  //创建http对象
  HTTPClient http;
 
  //配置请求地址
  http.begin(client, HttpPostURL); //HTTP请求
  Serial.print("[HTTP] begin...\n");
 
  //启动连接并发送HTTP报头和报文
  int httpCode = http.POST(jsonstr);
  Serial.print("[HTTP] POST...\n");
 
  //连接失败时 httpCode时为负
  if (httpCode > 0) {
 
    //将服务器响应头打印到串口
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
 
    //将从服务器获取的数据打印到串口
    if (httpCode == HTTP_CODE_OK) {
      const String& payload = http.getString();
      Serial.println("received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  //关闭http连接
  http.end();
}

/**
 * http_get()
 */
void http_get() {
 
  //创建 HTTPClient 对象
  HTTPClient httpClient;
 
  //配置请求地址。此处也可以不使用端口号和PATH而单纯的
  httpClient.begin(HttpGetURL); 
  Serial.print("HttpGetURL: "); 
  Serial.println(HttpGetURL);
 
  //启动连接并发送HTTP请求
  int httpCode = httpClient.GET();
  Serial.print("Send GET request to URL: ");
  Serial.println(HttpGetURL);
  
  //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  //如果服务器不响应OK则将服务器响应状态码通过串口输出
  if (httpCode == HTTP_CODE_OK) {
    String responsePayload = httpClient.getString();
    Serial.println("Server Response Payload: ");
    Serial.println(responsePayload);
  } else {
    Serial.println("Server Respose Code：");
    Serial.println(httpCode);
  }
 
  //关闭ESP8266与服务器连接
  httpClient.end();
}

void https_get() {
 
  //创建 HTTPClient 对象
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);   
  client->setInsecure();  
  
   //创建 HTTPClient 对象
  HTTPClient httpClient;
  //配置请求地址。此处也可以不使用端口号和PATH而单纯的
  httpClient.begin(*client,HttpsGetURL); 
  Serial.print("HttpGetURL: "); 
  Serial.println(HttpsGetURL);
 
  //启动连接并发送HTTP请求
  int httpCode = httpClient.GET();
  Serial.print("Send GET request to URL: ");
  Serial.println(HttpsGetURL);
  
  //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  //如果服务器不响应OK则将服务器响应状态码通过串口输出
  if (httpCode == HTTP_CODE_OK) {
    String responsePayload = httpClient.getString();
    Serial.println("Server Response Payload: ");
    Serial.println(responsePayload);
  } else {
    Serial.println("Server Respose Code：");
    Serial.println(httpCode);
  }
 
  //关闭ESP8266与服务器连接
  httpClient.end();
}

void https_post(String jsonstr) {
 
  //创建 HTTPClient 对象
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);   
  client->setInsecure();  
  
   //创建 HTTPClient 对象
  HTTPClient httpClient;
  //配置请求地址。此处也可以不使用端口号和PATH而单纯的
  httpClient.begin(*client,HttpsGetURL); 
  Serial.print("HttpGetURL: "); 
  Serial.println(HttpsGetURL);
 
  //启动连接并发送HTTP请求
  int httpCode = httpClient.POST(jsonstr);
  Serial.print("Send GET request to URL: ");
  Serial.println(HttpsGetURL);
  
  //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  //如果服务器不响应OK则将服务器响应状态码通过串口输出
  if (httpCode == HTTP_CODE_OK) {
    String responsePayload = httpClient.getString();
    Serial.println("Server Response Payload: ");
    Serial.println(responsePayload);
  } else {
    Serial.println("Server Respose Code：");
    Serial.println(httpCode);
  }
 
  //关闭ESP8266与服务器连接
  httpClient.end();
}
