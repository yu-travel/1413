初始化iic1的gpio管脚
初始化spi的gpio，功能配置
初始化ad5542
初始化ad7606
初始化外设使能管脚
初始化默认限流配置
初始化串口1
初始化定时器2
初始化软件定时器，0.5s闪烁喂狗



初始化串口1和串口3
初始化led灯
初始化vpx的GPIO地址管脚
初始化ipmi en ,ready管脚为高电平，

初始化xc388_en管脚
初始化XCA4001_RESET_Pin和XCA4001_Alert_Pin
初始化ADC
初始化TIM2
初始化iic1,iic2,iic3的gpio管脚，初始化iic接收buff

获取vpx从机地址

初始化软件定时器
1、0.5s闪烁
2、2s更新板载1.2V 1.5V 1.8V 1.0V 3.3V电压值，温度
3、3s循环获取温度，设置风扇占空比


初始化ipmi协议，填充_protocol_t ipmi_i2ca
typedef struct {
    _i2c_interrpt_t *i2cbus;
    _protocol_resp_t response;
    do_cmd_func do_cmd;
}_protocol_t;
填充其中的*i2cbus;和do_cmd;

初始化SPI协议栈


中断处理函数中
在上述iic中读取ipmi数据到环形队列protocol->i2cbus->rb_handler中，根据数据内容进行处理

while循环中
ipmi_recv_deal函数中验证下标0为8，验证下标1为数据长度，验证环形队列中是否接收完全，转到do_cmd_dispatch函数中处理
do_cmd_dispatch函数中，验证下标len-1为crc校验结果，根据下标2来确定是IICA还是IICB，取下标2为发送目的地址，下标3为请求会话序列号，
下标4为命令序列号，根据命令序列号来调用对应do_cmdxx函数，_do_data_t do_cmd[]来控制哪些命令序列号有效：

do_cmd19返回给主控板，正常、过压、欠压等异常状态
do_cmd20获取当前板卡传感器总数量
do_cmd2d查询电流、电压、以及开关状态



spi中
do_respcmd11 
板载多子卡监控上报接口。
上位机通过 SPI 下发CMD11命令（入参len == 0代表读子卡监控数据），
本函数把整机所有子卡（ZJB、XHCL、JKKZ、ZKB、PWR、HTM、PWR2）的温度、电压、电流、槽位地址、风扇转速组装成应答报文，
调用底层 SPI 发送接口回传给主机。

do_respcmd12 
CMD12 SPI 应答处理函数，不现场实时组装业务报文，
直接把全局缓冲区respcmd12里已经准备好的数据复制到 SPI 应答缓冲区，通过 SPI 发送给上位机



