#include "CH9329.h"

extern const uint8_t _asciimap[128] PROGMEM;

#define SHIFT 0x80
const uint8_t _asciimap[128] PROGMEM =
    {
        0x00, // NUL
        0x00, // SOH
        0x00, // STX
        0x00, // ETX
        0x00, // EOT
        0x00, // ENQ
        0x00, // ACK
        0x00, // BEL
        0x2a, // BS	Backspace
        0x2b, // TAB	Tab
        0x28, // LF	Enter
        0x00, // VT
        0x00, // FF
        0x00, // CR
        0x00, // SO
        0x00, // SI
        0x00, // DEL
        0x00, // DC1
        0x00, // DC2
        0x00, // DC3
        0x00, // DC4
        0x00, // NAK
        0x00, // SYN
        0x00, // ETB
        0x00, // CAN
        0x00, // EM
        0x00, // SUB
        0x00, // ESC
        0x00, // FS
        0x00, // GS
        0x00, // RS
        0x00, // US

        0x2c,         //  ' '
        0x1e | SHIFT, // !
        0x34 | SHIFT, // "
        0x20 | SHIFT, // #
        0x21 | SHIFT, // $
        0x22 | SHIFT, // %
        0x24 | SHIFT, // &
        0x34,         // '
        0x26 | SHIFT, // (
        0x27 | SHIFT, // )
        0x25 | SHIFT, // *
        0x2e | SHIFT, // +
        0x36,         // ,
        0x2d,         // -
        0x37,         // .
        0x38,         // /
        0x27,         // 0
        0x1e,         // 1
        0x1f,         // 2
        0x20,         // 3
        0x21,         // 4
        0x22,         // 5
        0x23,         // 6
        0x24,         // 7
        0x25,         // 8
        0x26,         // 9
        0x33 | SHIFT, // :
        0x33,         // ;
        0x36 | SHIFT, // <
        0x2e,         // =
        0x37 | SHIFT, // >
        0x38 | SHIFT, // ?
        0x1f | SHIFT, // @
        0x04 | SHIFT, // A
        0x05 | SHIFT, // B
        0x06 | SHIFT, // C
        0x07 | SHIFT, // D
        0x08 | SHIFT, // E
        0x09 | SHIFT, // F
        0x0a | SHIFT, // G
        0x0b | SHIFT, // H
        0x0c | SHIFT, // I
        0x0d | SHIFT, // J
        0x0e | SHIFT, // K
        0x0f | SHIFT, // L
        0x10 | SHIFT, // M
        0x11 | SHIFT, // N
        0x12 | SHIFT, // O
        0x13 | SHIFT, // P
        0x14 | SHIFT, // Q
        0x15 | SHIFT, // R
        0x16 | SHIFT, // S
        0x17 | SHIFT, // T
        0x18 | SHIFT, // U
        0x19 | SHIFT, // V
        0x1a | SHIFT, // W
        0x1b | SHIFT, // X
        0x1c | SHIFT, // Y
        0x1d | SHIFT, // Z
        0x2f,         // [
        0x31,         // bslash
        0x30,         // ]
        0x23 | SHIFT, // ^
        0x2d | SHIFT, // _
        0x35,         // `
        0x04,         // a
        0x05,         // b
        0x06,         // c
        0x07,         // d
        0x08,         // e
        0x09,         // f
        0x0a,         // g
        0x0b,         // h
        0x0c,         // i
        0x0d,         // j
        0x0e,         // k
        0x0f,         // l
        0x10,         // m
        0x11,         // n
        0x12,         // o
        0x13,         // p
        0x14,         // q
        0x15,         // r
        0x16,         // s
        0x17,         // t
        0x18,         // u
        0x19,         // v
        0x1a,         // w
        0x1b,         // x
        0x1c,         // y
        0x1d,         // z
        0x2f | SHIFT, // {
        0x31 | SHIFT, // |
        0x30 | SHIFT, // }
        0x35 | SHIFT, // ~
        0             // DEL
};

SemaphoreHandle_t ch9329_mutex; // 声明一个互斥锁句柄
__attribute__((weak)) void kvm_change_keyboard_leds_cb(uint8_t index, uint8_t leds)
{
    (void)index;
    (void)leds;
}
CH9329::CH9329(SerialUART *serial, struct CH9329CFG *ch9329cfgs)
{
    // 创建一个互斥信号量
    ch9329_mutex = xSemaphoreCreateMutex();
    if (ch9329_mutex == NULL)
    {
        // 互斥锁创建失败，可能内存不足或系统错误
        DBG_println("Error: Failed to create CH9329 mutex!");
        while (1)
            ; // 停止程序
    }
    this->_serial = serial;
    _ch9329cfgs = ch9329cfgs;
    _serial->setPinout(_ch9329cfgs[0].rx_pin, _ch9329cfgs[0].tx_pin);
    _serial->begin(_ch9329cfgs[0].baud);
    for (size_t i = 0; i < CH9329COUNT; i++)
    {
        pinMode(_ch9329cfgs[i].cfg1_pin, OUTPUT);
        pinMode(_ch9329cfgs[i].mode1_pin, OUTPUT);
        pinMode(_ch9329cfgs[i].led_pin, OUTPUT);
        digitalWrite(_ch9329cfgs[i].cfg1_pin, _ch9329cfgs[i].CFG1);
        digitalWrite(_ch9329cfgs[i].mode1_pin, _ch9329cfgs[i].MODE1);
        digitalWrite(_ch9329cfgs[i].led_pin, HIGH);

        memset(keyboardStatus[i].DATA, 0, sizeof(keyboardStatus[i].DATA));
    }
}

void CH9329::setSerialPort(SerialUART *serial)
{
    this->_serial = serial;
}

void CH9329::cmdGetInfo(uint8_t index)
{
    uart_fmt data{};
    data.CMD = CMD_GET_INFO;
    data.LEN = 0x00;
    this->writeUart(index, &data);
    // return this->readUart(info);
}

/**
 *
 * @param uint8_t key[8]： Length must be 8
 * @return
 */
void CH9329::cmdSendKbGeneralData(uint8_t index, uint8_t *key)
{
    uart_fmt data{};
    data.CMD = CMD_SEND_KB_GENERAL_DATA;
    data.LEN = 0x08;
    for (int i = 0; i < data.LEN; ++i)
    {
        data.DATA[i] = key[i];
    }
    this->writeUart(index, &data);
    // this->readUart(&this->_lastUartData);
    return;
}

void CH9329::cmdSendKbMediaData(uint8_t index, uint8_t *key)
{
    uart_fmt data{};
    data.CMD = CMD_SEND_KB_MEDIA_DATA;
    data.LEN = 0x02;
    for (int i = 0; i < data.LEN; ++i)
    {
        data.DATA[i] = key[i];
    }
    this->writeUart(index, &data);
    // this->readUart(&this->_lastUartData);
    return;
}

void CH9329::cmdSendMsAbsData(uint8_t index, uint8_t *data)
{
    uart_fmt uartData{};
    uartData.CMD = CMD_SEND_MS_ABS_DATA;
    uartData.LEN = 0x07;
    for (int i = 0; i < uartData.LEN; ++i)
    {
        uartData.DATA[i] = data[i];
    }

    this->writeUart(index, &uartData);
}

/**
 *
 * @param uint8_t data[5]： Length must be 5
 * @return
 */
void CH9329::cmdSendMsRelData(uint8_t index, uint8_t *data)
{
    uart_fmt uartData{};
    uartData.CMD = CMD_SEND_MS_REL_DATA;
    uartData.LEN = 0x05;
    for (int i = 0; i < uartData.LEN; ++i)
    {
        uartData.DATA[i] = data[i];
    }
    this->writeUart(index, &uartData);
    // this->readUart(&this->_lastUartData);
    return;
}

void CH9329::writeUart(uint8_t index, uart_fmt *data)
{
    if (xSemaphoreTake(ch9329_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        data->HEAD[0] = 0x57;
        data->HEAD[1] = 0xAB;
        data->ADDR = _ch9329cfgs[index].addr;
        // printUartFormat(data, true);

        _serial->write(data->HEAD[0]);
        _serial->write(data->HEAD[1]);
        _serial->write(data->ADDR);
        _serial->write(data->CMD);
        _serial->write(data->LEN);

        for (int i = 0; i < data->LEN; ++i)
        {
            _serial->write(data->DATA[i]);
        }

        _serial->write(this->sum(data));
        xSemaphoreGive(ch9329_mutex);
    }
}

uint8_t CH9329::sum(uart_fmt *data)
{
    uint8_t sum_ = data->HEAD[0] + data->HEAD[1] + data->ADDR + data->CMD + data->LEN;

    for (int i = 0; i < data->LEN; ++i)
    {
        sum_ += data->DATA[i];
    }
    return sum_ & 0xFF;
}

void CH9329::cmdReset(uint8_t index)
{
    uart_fmt uartData{};
    uartData.CMD = CMD_RESET;
    uartData.LEN = 0x00;
    this->writeUart(index, &uartData);
    // this->readUart(&this->_lastUartData);
    return;
}

// 辅助函数：安全地从 Serial 读取指定字节数到缓冲区
// 返回实际读取到的字节数
size_t CH9329::readBytesToBuffer(byte *buffer, size_t length)
{
    size_t count = 0;
    while (count < length)
    {
        if (this->_serial->available())
        {
            *buffer++ = (byte)this->_serial->read();
            count++;
        }
        else
        {
            // 如果需要，可以在此处添加超时逻辑
            // delay(1); // 可以根据需要调整或删除
        }
    }
    return count;
}

void CH9329::readUart()
{

    if (xSemaphoreTake(ch9329_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        // 只要串口有数据可用，就持续读取并处理
        while (this->_serial->available())
        {
            uint8_t incomingByte = this->_serial->read();
            // DBG_printf("%02X ", incomingByte);
            processByte(incomingByte);
        }
        xSemaphoreGive(ch9329_mutex);
    }
}

void CH9329::processByte(uint8_t byte)
{
    // 在计算校验和时，需要包含 HEAD、ADDR、CMD、LEN 和 DATA
    // SUM 本身不参与校验和计算
    if (_parseState >= STATE_GET_ADDR && _parseState <= STATE_GET_DATA)
    {
        _calculatedSum += byte;
    }

    switch (_parseState)
    {
    case STATE_WAIT_HEAD1:
        if (byte == 0x57)
        {
            _receiveBuffer[0] = byte;
            _bytesReceived = 1;
            _parseState = STATE_WAIT_HEAD2;
        }
        break;

    case STATE_WAIT_HEAD2:
        if (byte == 0xAB)
        {
            _receiveBuffer[1] = byte;
            _bytesReceived = 2;
            _parseState = STATE_GET_ADDR;
        }
        else
        {
            // 如果 HEAD2 不匹配，回到等待 HEAD1 状态（帧同步）
            _parseState = STATE_WAIT_HEAD1;
        }
        break;

    case STATE_GET_ADDR:
        _receiveBuffer[_bytesReceived++] = byte;
        _parseState = STATE_GET_CMD;
        break;

    case STATE_GET_CMD:
        _receiveBuffer[_bytesReceived++] = byte;
        _parseState = STATE_GET_LEN;
        break;

    case STATE_GET_LEN:
        _receiveBuffer[_bytesReceived++] = byte;
        _dataLength = byte; // 记录预期的数据长度 N

        // 边界检查！防止溢出
        if (_dataLength > sizeof(_lastUartData.DATA))
        {
            // 长度无效，丢弃此包
            _parseState = STATE_WAIT_HEAD1;
            break;
        }

        if (_dataLength == 0)
        {
            // 如果没有 DATA 部分，直接跳到获取 SUM 状态
            _parseState = STATE_GET_SUM;
        }
        else
        {
            _parseState = STATE_GET_DATA;
        }
        break;

    case STATE_GET_DATA:
        _receiveBuffer[_bytesReceived++] = byte;

        // 检查是否接收完所有 DATA 字节
        if (_bytesReceived == (5 + _dataLength))
        { // 5是HEAD+ADDR+CMD+LEN的长度
            _parseState = STATE_GET_SUM;
        }
        break;

    case STATE_GET_SUM:
        // 接收到最后的 SUM 字节
        uint8_t receivedSum = byte;
        // DBG_printf("receivedSum= %02X, _calculatedSum= %02X, %02X", receivedSum, _calculatedSum, (_calculatedSum & 0xFF));
        // 校验和验证
        // if (receivedSum == (_calculatedSum & 0xFF))
        // {
        // 校验通过！将临时缓冲区的数据安全地复制到 _lastUartData 结构体
        memcpy(&_lastUartData, _receiveBuffer, 5 + _dataLength + 1); // +1 是 SUM 字节

        // 现在 _lastUartData 是一个完整的有效数据包，可以进行后续处理
        processValidPacket(); // 调用你的处理函数
        // }
        // else
        // {
        //     // 校验失败，丢弃此包
        // }

        // 无论成功与否，重置状态机以等待下一个包
        _parseState = STATE_WAIT_HEAD1;
        _bytesReceived = 0;
        _calculatedSum = 0;
        break;
    }
}
void CH9329::processValidPacket()
{
    // 现在我们可以安全地使用 _lastUartData 了

    if (_lastUartData.CMD == RET_GETINFO)
    {
        printUartFormat(&_lastUartData, false); // 假设这个函数存在
        uint8_t index = getIndexByAddr(_lastUartData.ADDR);
        // memcpy 是安全的，因为我们知道数据是完整的
        memcpy(&keyboardStatus[index], &_lastUartData, sizeof(uart_fmt));
        kvm_change_keyboard_leds_cb(index, keyboardStatus[index].DATA[2]);
    }
}

uint8_t CH9329::getIndexByAddr(uint8_t addr)
{
    for (size_t i = 0; i < CH9329COUNT; i++)
    {
        if (addr == _ch9329cfgs[i].addr)
        {
            return i;
        }
    }

    return 0;
}

void CH9329::turnOnLed(uint8_t index)
{
    digitalWrite(_ch9329cfgs[index].led_pin, LOW);
}

void CH9329::turnOffLed(uint8_t index)
{
    digitalWrite(_ch9329cfgs[index].led_pin, HIGH);
}

void CH9329::press(uint8_t index, uint8_t control,
                   uint8_t hid_code0,
                   uint8_t hid_code1,
                   uint8_t hid_code2,
                   uint8_t hid_code3,
                   uint8_t hid_code4,
                   uint8_t hid_code5)
{
    uint8_t data[8] = {control, 0, hid_code0, hid_code1, hid_code2, hid_code3, hid_code4, hid_code5};
    this->cmdSendKbGeneralData(index, data);
    return;
}

void CH9329::pressASCII(uint8_t index, char k, uint8_t control)
{
    uint8_t data[8] = {0};
    data[0] = control;
    if (k >= 136)
    { // it's a non-printing key (not a modifier)
        return;
    }

    if (k >= 128)
    { // it's a modifier key
        data[0] &= ~(1 << (k - 128));
        k = 0;
    }
    else
    { // it's a printing key
        k = pgm_read_byte(_asciimap + k);
        if (!k)
        {
            return;
        }
        if (k & SHIFT)
        {                    // it's a capital letter or other character reached with shift
            data[0] ^= 0x02; // the left shift modifier
            k &= 0x7F;
        }
    }

    data[2] = k;
    this->cmdSendKbGeneralData(index, data);
    return;
}

void CH9329::releaseAll(uint8_t index)
{
    uint8_t data[8] = {0};
    this->cmdSendKbGeneralData(index, data);
}

void CH9329::sendString(uint8_t index, char *string, uint8_t len)
{

    bool hostCapsLock = this->isCapsLock(index);
    uint8_t switchCapsLock[] = {0x00, 0x00, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (hostCapsLock)
    {
        this->cmdSendKbGeneralData(index, switchCapsLock);
        this->releaseAll(index);
    }

    for (int i = 0; i < len; ++i)
    {
        this->pressASCII(index, string[i]);
        this->releaseAll(index);
    }

    if (hostCapsLock)
    {
        this->cmdSendKbGeneralData(index, switchCapsLock);
        this->releaseAll(index);
    }
}

uint8_t CH9329::getChipVer(uint8_t index)
{
    return keyboardStatus[index].DATA[0];
}

bool CH9329::isUSBConnected(uint8_t index)
{
    return keyboardStatus[index].DATA[1];
}

bool CH9329::isCapsLock(uint8_t index)
{
    return keyboardStatus[index].DATA[2] & 0x02;
}

bool CH9329::isNumLock(uint8_t index)
{
    return keyboardStatus[index].DATA[2] & 0x01;
}

bool CH9329::isScrollLock(uint8_t index)
{
    return keyboardStatus[index].DATA[2] & 0x04;
}

void CH9329::mouseRelease(uint8_t index)
{
    uint8_t data[5] = {1, 0, 0, 0, 0};
    this->cmdSendMsRelData(index, data);
}

void CH9329::mouseMoveAbs(uint8_t index, uint16_t x, uint16_t y, uint8_t ms_key, uint8_t ms_wheel)
{
    uint8_t data[7] = {2, ms_key,
                       (uint8_t)(x & 0xFF),        // 低 8 位
                       (uint8_t)((x >> 8) & 0xFF), // 高 8 位
                       (uint8_t)(y & 0xFF),        // 低 8 位
                       (uint8_t)((y >> 8) & 0xFF), // 高 8 位
                       ms_wheel};
    DBG_print("ms_key = ");
    DBG_print(ms_key, BIN);
    DBG_print("ms_wheel = ");
    DBG_print(ms_wheel, BIN);
    DBG_println();
    this->cmdSendMsAbsData(index, data);
}

void CH9329::mouseMove(uint8_t index, uint8_t horizontal, uint8_t vertical, uint8_t ms_key, uint8_t ms_wheel)
{
    uint8_t data[5] = {1, ms_key, horizontal, vertical, ms_wheel};
    this->cmdSendMsRelData(index, data);
}

void CH9329::mouseMove(uint8_t index, uint8_t horizontal, uint8_t vertical, uint8_t ms_key)
{
    uint8_t data[5] = {1, ms_key, horizontal, vertical, 0};
    this->cmdSendMsRelData(index, data);
}

void CH9329::mouseWheel(uint8_t index, uint8_t scale, uint8_t ms_key)
{
    uint8_t data[5] = {1, ms_key, 0, 0, scale};
    this->cmdSendMsRelData(index, data);
}

void CH9329::mousePress(uint8_t index, uint8_t ms_key)
{
    uint8_t data[5] = {1, ms_key, 0, 0, 0};
    this->cmdSendMsRelData(index, data);
}

void CH9329::mouseClick(uint8_t index, uint8_t ms_key, uint8_t delay_ms)
{
    uint8_t data[5] = {1, ms_key, 0, 0, 0};
    this->cmdSendMsRelData(index, data);
    data[1] = 0;
    delay(delay_ms);
    this->cmdSendMsRelData(index, data);
}

void CH9329::customizeCmd(uint8_t index, uint8_t cmd, uint8_t *data, uint8_t len)
{
    uart_fmt uartData{};
    uartData.CMD = cmd;
    uartData.LEN = len;
    for (int i = 0; i < uartData.LEN; ++i)
    {
        uartData.DATA[i] = data[i];
    }
    this->writeUart(index, &uartData);
    // this->readUart(&this->_lastUartData);
    return;
}

uart_fmt *CH9329::getLastUartData()
{
    return &this->_lastUartData;
}

/**
 * @brief 打印 UART_FMT 结构体的内容到 USB 串口
 * @param info 指向要打印的结构体的指针
 */
void CH9329::printUartFormat(uart_fmt *info, bool isWrite)
{
#ifdef USB_DEBUG
    if (isWrite)
    {
        DBG_print(">>>>> ");
    }
    else
    {
        DBG_print("<<<<< ");
    }

    // 打印帧头 (HEAD) - 使用十六进制格式
    DBG_printf("HEAD: %02X, %02X ", info->HEAD[0], info->HEAD[1]);

    // 打印地址码 (ADDR)
    DBG_printf("ADDR: %02X ", info->ADDR);

    // 打印命令码 (CMD)
    DBG_printf("CMD:  %02X ", info->CMD);

    // 打印数据长度 (LEN)
    DBG_printf("LEN: %02X ", info->LEN);

    // 打印后续数据 (DATA)
    DBG_print("DATA: ");
    // 循环打印 N 个字节的数据
    for (int i = 0; i < (uint8_t)info->LEN; i++)
    {
        DBG_printf("%02X ", info->DATA[i]);
    }

    // 打印累加和 (SUM)
    DBG_printf("SUM:  %02X\n", info->SUM);
    Serial.flush();
#endif
}
