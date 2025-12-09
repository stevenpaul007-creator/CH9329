======English ver.======

USB HID Keyboard using WCH's dual CH9329 chips.

🚧 Work in progress

=========中文版==========

在Arduino上使用两个或多个CH9329 USB转HID键盘芯片.

#### 定义个数
```c++
#define CH9329COUNT 2
```

#### 参数
```c++
struct CH9329CFG ch9329cfgs[CH9329COUNT] = {
{
    .led_pin = 2,
    .rx_pin = 8,
    .tx_pin = 9,
    .cfg1_pin = 10,
    .mode1_pin = 11,
    .addr = 0x30,
    .baud = 115200,
    .CFG1 = HIGH,
    .MODE1 = HIGH // LOW: 注：Linux/Android/macOS等操作系统下，建议使用该模式。软件设置后不生效
},
{
    .led_pin = 28,
    .rx_pin = 8,
    .tx_pin = 9,
    .cfg1_pin = 27,
    .mode1_pin = 26,
    .addr = 0x01,
    .baud = 115200,
    .CFG1 = HIGH,
    .MODE1 = HIGH // LOW: 注：Linux/Android/macOS等操作系统下，建议使用该模式。软件设置后不生效
}};
```

#### 初始化
```c++
CH9329 *CH9329Client;
CH9329Client = new CH9329(&Serial2, ch9329cfgs);
```

#### 按下按键或组合键
```c++
CH9329.pressASCII(uint8_t index, char key, uint8_t control = 0);
```
示例：
```c++
ch9329.pressASCII(0, 'h');   // 按下 h
ch9329.pressASCII(0, 'H');   // 按下 Shift + h

/*
 * 注意：此方法会识别传入字符是否需要Shift。
 * 注意：如果不需要长时间按住该按键，应在调用该方法后调用 CH9329.releaseAll() 。
 */

ch9329.pressASCII(0, 'c' , KEY_LEFT_CTRL );   // 按下 Ctrl + a
ch9329.pressASCII(0, 'c' , KEY_LEFT_CTRL | KEY_LEFT_SHIFT );   // 按下 Ctrl + Shift + a
```

控制键的可选值：

| 可选值          | 含义             |
| --------------- | ---------------- |
| KEY_LEFT_CTRL   | 左Ctrl键         |
| KEY_LEFT_SHIFT  | 左Shift键        |
| KEY_LEFT_ALT    | 左Alt键          |
| KEY_LEFT_GUI    | 同 KEY_LEFT_WIN  |
| KEY_LEFT_WIN    | 左Winodws键      |
| KEY_RIGHT_CTRL  | 右Ctrl键         |
| KEY_RIGHT_SHIFT | 右Shift键        |
| KEY_RIGHT_ALT   | 右Alt键          |
| KEY_RIGHT_GUI   | 同 KEY_RIGHT_WIN |
| KEY_RIGHT_WIN   | 右Winodws键      |

#### 松开所有按键
```c++
void releaseAll(uint8_t index);
```
应在 CH9329.pressASCII() 后调用此方法，防止连续输入。
#### 连续发送多个按键
```c++
void sendString(uint8_t index, char * string , uint8_t len );
```
连续向主机发送多个按键。


示例：
```c++
char * str = "Hello World\nHello World";
ch9329.sendString(0, str , 23 );
/*
 * 注意：此方法会将主机切换到小写模式，结束后还原。
 */
```


#### 鼠标点击
```c++
void mouseClick(uint8_t index, uint8_t ms_key = MOUSE_LEFT_BUTTON , uint8_t delay_ms = 10 );
```
默认点击鼠标左键，默认按下10毫秒松开。
示例：
```c++
ch9329.mouseClick(0); // 单击鼠标左键

ch9329.mouseClick(0, MOUSE_MIDDLE_BUTTON ); // 单击鼠标中键

ch9329.mouseClick(0, MOUSE_RIGHT_BUTTON , 1000 ); // 按住鼠标右键一秒后松开
```

#### 鼠标长按
```c++
void mousePress(uint8_t index, uint8_t ms_key = MOUSE_LEFT_BUTTON );
```
#### 松开鼠标
```c++
void mouseRelease(uint8_t index);
```

#### 鼠标移动（绝对移动）
```c++
/**
 * 芯片默认模拟的绝对鼠标分辨率为 4096 * 4096，外围串口设备下传 XY 绝对值时，需
 * 要先根据自身屏幕分辨率进行计算，再下传计算后的值。
 * 比如当前屏幕分辨率为：X_MAX(1280) * Y_MAX(768)，则需要移动到点(100，120)，需要进行
 *
 * 如下计算：
 * X_Cur = ( 4096 * 100 ) / X_MAX；
 * Y_Cur = ( 4096 * 120 ) / Y_MAX
 */
void mouseMoveAbs(uint8_t index, uint16_t x, uint16_t y, uint8_t ms_key = 0, uint8_t ms_wheel = 0);
```
 * @param x : 水平方向的绝对位置， -127 ~ -1 向左移动，0 不动，1~127 向右移动
 * @param y   : 垂直方向的绝对位置， -127 ~ -1 向上移动，0 不动，1~127 向下移动
 * @param ms_key     : 移动鼠标时按住鼠标按键。（使用此函数后需要调用 mouseRelease() 松开按键）
 * @param ms_wheel: 滚轮

#### 鼠标移动（相对移动）
```c++
void mouseMove(uint8_t index, uint8_t horizontal , uint8_t vertical , uint8_t ms_key = 0);
```
 * @param horizontal : 水平方向的相对位置， -127 ~ -1 向左移动，0 不动，1~127 向右移动
 * @param vertical   : 垂直方向的相对位置， -127 ~ -1 向上移动，0 不动，1~127 向下移动
 * @param ms_key     : 移动鼠标时按住鼠标按键。（使用此函数后需要调用 mouseRelease() 松开按键）

#### 鼠标滚轮
```c++
void mouseWheel(uint8_t index, uint8_t scale , uint8_t ms_key = 0);
```
 * @param scale     : 滚轮滚动齿数， -127 ~ -1 屏幕向下滚动，0 不动，1~127 屏幕向上滚动
 * @param ms_key    : 移动鼠标时按住鼠标按键。（使用此函数后需要调用 mouseRelease() 松开按键）

#### 获取主机或芯片的状态
```c++
uint8_t getChipVer(uint8_t index);  // 获取芯片版本号
bool isUSBConnected(uint8_t index);  // 判断USB连接状态
bool isCapsLock(uint8_t index); // CapsLock指示灯状态
bool isNumLock(uint8_t index); // NumLock指示灯状态
bool isScrollLock(uint8_t index); // ScrollLock指示灯状态
```


#### 自定义命令
```c++
void customizeCmd(uint8_t index, uint8_t cmd , uint8_t * data , uint8_t len);
```
对于未封装的命令，可使用此方法自行调用。


示例：
```c++
// 发送获取当前参数配置信息命令
ch9329.customizeCmd(0, CMD_GET_PARA_CFG , nullptr , 0 );

// 获取最后一次命令的串口响应
uart_fmt * data =  ch9329.getLastUartData();

// 验证校验和
if( data->SUM == ch9329.sum( data ) ){
//    
} else{
//    
}

```


#### 官方文档
- [WCH](https://www.wch.cn/products/CH9329.html?from=list)

#### 参考项目
- [beijixiaohu/CH9329_COMM](https://github.com/beijixiaohu/CH9329_COMM)
- [lxydiy/CH9328-Keyboard](https://github.com/lxydiy/CH9328-Keyboard)