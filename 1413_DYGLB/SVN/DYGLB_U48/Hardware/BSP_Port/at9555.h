/*
***********************************************************************************************************************
    @brief          : 板载gpio扩展操作
    @author         : xiongjinqi
    @date           : 2024/07/21
***********************************************************************************************************************
*/
#ifndef __AT9555_H_
#define __AT9555_H_



#define AT9555_DEV_1_ID     0x46
#define AT9555_DEV_2_ID     0x4a


#define AT9555_REG00        0x00        //Input port register pair
#define AT9555_REG01        0x01        //Input port register pair
#define AT9555_REG02        0x02        //Output port register pair
#define AT9555_REG03        0x03        //Output port register pair
#define AT9555_REG06        0x06        //Configuration port register pair
#define AT9555_REG07        0x07        //Configuration port register pair

typedef struct {
    _myiic_t *i2cbus;
    u8       devid[2];
    u16       input[2];    // 高8位port1, 低8位port0
    u16       output[2];   // 高8位port1, 低8位port0
    u16       config[2];   // 高8位port1, 低8位port0
}_at9555_t;

/*
    @brief      : Gpio扩展AT9555初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void at9555_init(void);

/*
    @brief      : Gpio扩展AT9555初始化
    @param[in]  : none
    @param[out] : none
    @retval     : none
*/
void at9555_output_config(u16 dev1_data, u16 dev2_data);

#if TEST_ENABLE
void at9555_test(void);

#endif

#endif /* __AT9555_H_ */

