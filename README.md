# ESP32 LED Control Web Server

这个项目实现了一个ESP32 WiFi热点，可以通过网页控制连接到GPIO2 (D2引脚) 的LED灯。

## 硬件要求

* ESP32 DEVKIT_C 开发板
* LED灯连接到GPIO2 (D2引脚)
* 220Ω电阻（串联在LED和GPIO2之间）
* 面包板和跳线

## 电路连接

```
ESP32 GPIO2 (D2) --> 220Ω 电阻 --> LED 正极
LED 负极 --> ESP32 GND
```

## 功能特点

* **WiFi热点模式**: ESP32创建一个名为 "ESP32_LED_Control" 的WiFi热点
* **Web界面**: 提供简洁美观的网页界面控制LED
* **实时状态**: 网页实时显示LED的当前状态
* **响应式设计**: 支持手机和电脑浏览器访问

## 网络配置

* **WiFi名称**: ESP32_LED_Control
* **WiFi密码**: 12345678
* **ESP32 IP地址**: 192.168.4.1
* **Web服务器端口**: 80

## 如何使用

### 1. 构建和烧录

```bash
# 设置ESP-IDF环境
. $HOME/esp/esp-idf/export.sh

# 进入项目目录
cd d:\vs_test\esp32\led\simple

# 配置项目（可选）
idf.py menuconfig

# 构建项目
idf.py build

# 烧录到ESP32
idf.py -p COM3 flash monitor
```

### 2. 连接WiFi

1. 烧录完成后，ESP32会自动创建WiFi热点
2. 使用手机或电脑搜索并连接到 "ESP32_LED_Control" WiFi网络
3. 输入密码: 12345678

### 3. 控制LED

1. 打开浏览器
2. 访问: http://192.168.4.1
3. 使用网页上的 "Turn ON" 和 "Turn OFF" 按钮控制LED

## 网页功能

* **LED状态显示**: 绿色表示LED开启，红色表示LED关闭
* **控制按钮**: 
  - "Turn ON" 按钮: 打开LED（绿色按钮）
  - "Turn OFF" 按钮: 关闭LED（红色按钮）
* **自动刷新**: 点击按钮后页面自动刷新显示最新状态

## 代码结构

* `main/main.c`: 主程序文件
  - WiFi热点初始化
  - HTTP服务器设置
  - LED GPIO控制
  - 网页生成和处理

## 技术细节

* **GPIO配置**: GPIO2配置为输出模式
* **WiFi模式**: AP（Access Point）模式
* **HTTP服务器**: 使用ESP-IDF的HTTP服务器组件
* **网页技术**: HTML + CSS + JavaScript

## 故障排除

1. **无法连接WiFi**: 检查WiFi名称和密码是否正确
2. **网页无法访问**: 确认已连接到ESP32的WiFi，并访问 192.168.4.1
3. **LED不亮**: 检查电路连接和电阻值
4. **编译错误**: 确保ESP-IDF环境正确安装和配置

## 自定义配置

您可以在 `main.c` 文件中修改以下配置：

```c
#define LED_GPIO_PIN GPIO_NUM_2    // LED连接的GPIO引脚
#define WIFI_SSID "ESP32_LED_Control"  // WiFi名称
#define WIFI_PASS "12345678"           // WiFi密码
#define WIFI_CHANNEL 1                 // WiFi频道
#define MAX_STA_CONN 4                 // 最大连接数
```
```
I (9580) example_connect: - IPv4 address: 192.168.194.219
I (9580) example_connect: - IPv6 address: fe80:0000:0000:0000:266f:28ff:fe80:2c74, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (9590) example: Starting server on port: '80'
I (9600) example: Registering URI handlers
I (66450) example: Found header => Host: 192.168.194.219
I (66460) example: Request headers lost
```

## Troubleshooting
* If the server log shows "httpd_parse: parse_block: request URI/header too long", especially when handling POST requests, then you probably need to increase HTTPD_MAX_REQ_HDR_LEN, which you can find in the project configuration menu (`idf.py menuconfig`): Component config -> HTTP Server -> Max HTTP Request Header Length
