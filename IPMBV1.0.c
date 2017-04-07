
/*
 * IPMBBMCV1.0.c
 *
 * Created: 2017/2/8 15:22:04
 * Author : liyx
 */ 

#include "asf.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "conf_board.h"
#include "conf_twi_slave_example.h"

#define SLAVE_ADDRESS               0x70
#define MASTER_ADDRESS              0x58
#define MEMORY_SIZE                 512
#define APP_REQ                     0x18
#define APP_RESP                    0x1C
#define GET_SENSOR_READING          0x2D
#define GET_DEVICE_ID               0x01
#define RTC_V                       0x71
#define SENSOR_EVENT_REQUEST        0x10
#define SENSOR_EVENT_RESPONSE       0x14


uint8_t TEMP_V;                      
uint8_t num=0;
uint8_t data[MEMORY_SIZE];
uint8_t trans_data_id[22];
uint8_t trans_data_sensor[11];
uint8_t slave_addr[3] = {0x48,0x49,0x4A};



#if 0
void ADC1_init(void)
{
	pmc_enable_periph_clk(ID_ADC);//enable adc clk;
	adc_init(ADC, sysclk_get_cpu_hz(), 6400000, ADC_MR_STARTUP_SUT64);//speed set;
	adc_enable_channel(ADC,15);//enable adc channels;
	adc_enable_channel(ADC, ADC_CHANNEL_0);
	adc_enable_channel(ADC, ADC_CHANNEL_4);
	adc_enable_channel(ADC, ADC_CHANNEL_5);
	adc_enable_channel(ADC, ADC_CHANNEL_8);
	adc_enable_channel(ADC, ADC_CHANNEL_9);
	adc_enable_channel(ADC, ADC_CHANNEL_10);
	adc_enable_channel(ADC, ADC_CHANNEL_11);
	adc_enable_channel(ADC, ADC_CHANNEL_12);
	adc_enable_ts(ADC);//enable temp sensor;
	adc_configure_trigger(ADC, ADC_TRIG_SW, 1);//disable hardware trig;enable software and free run mode;

}
#endif

/*****************************************************************************************************

函数名称:   cal_temp

函数功能:   获取cpu附近传感器温度值

参数名称            类型            输入/输出           含义

slav_addr           uint8_t           输入              传感器iic总线地址
*TEMP_V             uint8_t           输出              所获取的温度计算值

返回值  :   无

函数说明:   使用中断方式接受iic信号，并处理接收信号发送响应信号。


*****************************************************************************************************/
static void cal_temp(uint8_t slav_addr,uint8_t *TEMP_V)
{
#if 0
    puts("start get temp!\n\r");
#endif
	uint8_t b[2] = {0,0};
	twi_packet_t p = {
		.addr[0] = 0x00,
		.addr_length = 1,
		.buffer = b,
		.length = 2,
		.chip = slav_addr
	};
#if 0
	int8_t us_h;
#endif
	twi_master_read(TWI1,&p);
#if 0
	if (us_h != TWI_SUCCESS )
	{
	    puts("we cant get temp!\n\r");
		printf("erro num is %d\n\r" ,us_h);
	}
	printf("adc sem is 0x%02x\n\r" ,b[0]);
	printf("adc sem is 0x%02x\n\r" ,b[1]);
#endif
    int8_t x = b[0];
	uint16_t regVal = ((b[0]<<8)|b[1])>>4;
	int8_t sign = 1;
	if (regVal&0x800) 
	{
		sign = -1;
		regVal&=0x7FF;
		regVal =((4096-regVal)*0x19)/0x14C;
	}
	else
	{
		regVal =regVal/0xD;
	}
	*TEMP_V = (int8_t)sign * regVal ;
	if (b[0]&0x80)
	{

		x &= 0x7f;
		x = (256 - x);
	}


	printf("current temp is %d\n" ,x);

}



/*****************************************************************************************************

函数名称:   TWI0_Handler

函数功能:   中断服务函数

参数名称            类型            输入/输出           含义


返回值  :   无

函数说明:   使用中断方式接受iic信号，并处理接收信号发送响应信号。


*****************************************************************************************************/
void TWI0_Handler(void)
{
	uint32_t status;
	status = twi_get_interrupt_status(TWI0);
	/* 设置中断标志RXRDY、GACC、NACK、EOSACC、WS */
	if (((status & TWI_SR_SVACC) == TWI_SR_SVACC))
	{
		twi_disable_interrupt(TWI0,TWI_IDR_SVACC);
		twi_enable_interrupt(TWI0,TWI_IER_RXRDY | TWI_IER_GACC
		| TWI_IER_NACK | TWI_IER_EOSACC | TWI_IER_SCL_WS);
	}
	if((status & TWI_SR_GACC) == TWI_SR_GACC)
	{
		puts("General Call Treatment\n\r");
	}
	/* SVACC为高 RXRDY为高 GACC为低 则收数 ，如果TXCOMP为高 EOSACC为高 则停止收数，初始化中断 */
	if (((status & TWI_SR_SVACC) == TWI_SR_SVACC) && ((status & TWI_SR_GACC) == 0)
	&& ((status & TWI_SR_RXRDY) == TWI_SR_RXRDY)) {
		data[num]=(twi_read_byte(TWI0) & 0xFF);
		num++;
		}else {
		if (((status & TWI_SR_TXCOMP) == TWI_SR_TXCOMP)
		&& ((status & TWI_SR_EOSACC) == TWI_SR_EOSACC))
		{
			twi_enable_interrupt (TWI0, TWI_SR_SVACC);
			twi_disable_interrupt(TWI0, TWI_IDR_RXRDY |TWI_IDR_GACC
			| TWI_IDR_EOSACC | TWI_IDR_NACK | TWI_IDR_SCL_WS);

#if 0
			printf("%d\n\r",num);
			for (i=0;i<num;i++)
			{
				printf("%x\n\r ",data[i]);
			}
#endif
			twi_options_t opts = {
				.master_clk = sysclk_get_cpu_hz(),
				.speed = 100000,
				.smbus = 0
			};

			if(twi_master_init(TWI0, &opts)!= TWI_SUCCESS)
			{
#if 0
				puts("iic0masterinit failed!\n\r");
#endif
			}
#if 0
    		puts("steptwo ok\n\r");
#endif

			if((data[0] & APP_REQ) == APP_REQ)
			{
#if 0
			    puts("id_message is detected!");
#endif
			    trans_data_id[0] = APP_RESP;
				trans_data_id[1] = 0x100 - (((MASTER_ADDRESS<<1) +APP_RESP)%0X100);
				trans_data_id[2] = SLAVE_ADDRESS<<1;
				trans_data_id[3] = data[3];
				trans_data_id[4] = GET_DEVICE_ID;
				trans_data_id[5] = 0x00;
				trans_data_id[6] = 0x4A;
				trans_data_id[7] = 0x01;
				trans_data_id[8] = 0x00;
				trans_data_id[9] = 0x00;
				trans_data_id[10] = 0x02;
				trans_data_id[11] = 0x21;
				trans_data_id[12] = 0x08;
				trans_data_id[13] = 0x29;
				trans_data_id[14] = 0x46;
				trans_data_id[15] = 0x88;
				trans_data_id[16] = 0x10;
				trans_data_id[17] = 0x00;
				trans_data_id[18] = 0x00;
				trans_data_id[19] = 0x00;
				trans_data_id[20] = 0x00;
				trans_data_id[21] = 0x100 - ((0x18A + data[3] +(SLAVE_ADDRESS<<1))%0x100);


#if 0
                for (uint8_t a = 0; a<12;a++)
				{
					printf("tempmessage is %x\n\r" ,trans_data_sensor[a]);
				}
				puts("ipmb message is done\n\r");
#endif

				twi_packet_t twi_packet = {
					.addr_length  = 0,
					.chip         = MASTER_ADDRESS, /* TWI slave bus address */
					.buffer       = (void *)trans_data_id,        /* transfer data source buffer */
					.length       = sizeof(trans_data_id)              /* transfer data size (bytes) */
					
				};
				int8_t s1;
				s1 = twi_master_write(TWI0, &twi_packet);
				if(s1 != TWI_SUCCESS)
				{
#if 0
					printf("%d is erro\n\r" ,s1);
#endif
				}
#if 0
				puts("data has transferd!\n\r");
#endif
				twi_slave_init(TWI0, SLAVE_ADDRESS);
				twi_read_byte(TWI0);
				NVIC_DisableIRQ(TWI0_IRQn);
				NVIC_ClearPendingIRQ(TWI0_IRQn);
				NVIC_SetPriority(TWI0_IRQn, 0);
				NVIC_EnableIRQ(TWI0_IRQn);
				twi_enable_interrupt(TWI0, TWI_SR_SVACC);
#if 0
				puts("iic has init slavemode!\n\r");
#endif
				num = 0;
				for(int8_t h = 0; h<22; h++)
				{
					data[h] = 0x00;
					trans_data_id[h] = 0x00;
				}
#if 0
				puts("clear messages!\n\r");
#endif
			}
			else
			{
			    if(data[5] == 0x13)
				{
#if 0
				    puts("temp_message is detected!");
#endif
				    trans_data_sensor[0] = SENSOR_EVENT_RESPONSE;
					trans_data_sensor[1] = 0x100 - (((MASTER_ADDRESS<<1) + SENSOR_EVENT_RESPONSE)%0x100);
					trans_data_sensor[2] = SLAVE_ADDRESS<<1;
					trans_data_sensor[3] = data[3];
					trans_data_sensor[4] = GET_SENSOR_READING;
					trans_data_sensor[5] = 0x00;
					trans_data_sensor[6] = TEMP_V;
					trans_data_sensor[7] = 0xC0;
					trans_data_sensor[8] = 0x00;
					trans_data_sensor[9] = 0x00;
					trans_data_sensor[10] = 0x100 - ((0xED + (SLAVE_ADDRESS<<1) + TEMP_V + data[3])%0x100);
#if 0
					for (uint8_t b = 0; b<12;b++)
					{
					    printf("tempmessage is %x\n\r" ,trans_data_sensor[b]);
					}
#endif

				}
				else
				{
#if 0
				    puts("rtc_message is detected!");
#endif
				    trans_data_sensor[0] = SENSOR_EVENT_RESPONSE;
					trans_data_sensor[1] = 0x100 - (((MASTER_ADDRESS<<1) + SENSOR_EVENT_RESPONSE)%0x100);
					trans_data_sensor[2] = SLAVE_ADDRESS<<1;
					trans_data_sensor[3] = data[3];
					trans_data_sensor[4] = GET_SENSOR_READING;
					trans_data_sensor[5] = 0x00;
					trans_data_sensor[6] = RTC_V;
					trans_data_sensor[7] = 0xC0;
					trans_data_sensor[8] = 0x00;
					trans_data_sensor[9] = 0x00;
					trans_data_sensor[10] = 0x100 - ((0xED + (SLAVE_ADDRESS<<1) + RTC_V + data[3])%0x100);

#if 0
					for (uint8_t c = 0; c<12;c++)
					{
						printf("tempmessage is %x\n\r" ,trans_data_sensor[c]);
					}
#endif

				}

#if 0
				puts("ipmb message is done\n\r");
#endif
				twi_packet_t twi_packet = {
					.addr_length  = 0,
					.chip         = MASTER_ADDRESS, /* TWI slave bus address */
					.buffer       = (void *)trans_data_sensor,        /* transfer data source buffer */
					.length       = sizeof(trans_data_sensor)              /* transfer data size (bytes) */
					
				};

				
				twi_master_write(TWI0, &twi_packet);

				twi_slave_init(TWI0, SLAVE_ADDRESS);
				twi_read_byte(TWI0);
				NVIC_DisableIRQ(TWI0_IRQn);
				NVIC_ClearPendingIRQ(TWI0_IRQn);
				NVIC_SetPriority(TWI0_IRQn, 0);
				NVIC_EnableIRQ(TWI0_IRQn);
				twi_enable_interrupt(TWI0, TWI_SR_SVACC);

#if 0
				puts("iic has init slavemode!\n\r");
#endif

				num = 0;
				for(int8_t j = 0; j<12; j++)
				{
					data[j] = 0x00;
					trans_data_sensor[j] = 0x00;
				}
#if 0
				puts("messages cleard\n\r");
#endif
			}
		}
	}
}
/*****************************************************************************************************

函数名称:   configure_console

函数功能:   串口初始化函数

参数名称            类型            输入/输出           含义


返回值  :   无

函数说明:   调用asf库函数初始化串口完成串口打印信息功能。


*****************************************************************************************************/
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY,

	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

/*****************************************************************************************************

函数名称:   main

函数功能:   主函数

参数名称            类型            输入/输出           含义


返回值  :   无

函数说明:   初始化板卡并设置TWI0(IIC0)为从模式并设置中断标识触发点，TWI1(IIC1)为主模式。完成初始化后在循环获取板载传感器温度数据。


*****************************************************************************************************/
int main(void)
{
    /* 设置系统时钟源并初始化系统时钟 */
    sysclk_init();
	
	/* 配置sam4sPB4PB5两引脚功能，屏蔽JTAG功能 */
	REG_CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;
	REG_CCFG_SYSIO |= CCFG_SYSIO_SYSIO5;

	/* 初始化板卡及pio引脚 */
	board_init();
	/* 初始化串口 */
	configure_console();

	uint8_t c_choice;
	/* 初始化iic0时钟并配置iic0为从模式 */
	pmc_enable_periph_clk(ID_TWI0);
	twi_slave_init(TWI0, SLAVE_ADDRESS);
	twi_read_byte(TWI0);
	/* 初始化iic0中断并设置iic0中断标识触发条件 */
	NVIC_DisableIRQ(TWI0_IRQn);
	NVIC_ClearPendingIRQ(TWI0_IRQn);
	NVIC_SetPriority(TWI0_IRQn, 0);
	NVIC_EnableIRQ(TWI0_IRQn);
	twi_enable_interrupt(TWI0, TWI_SR_SVACC);
	/* 初始化iic1时钟并配置iic1为主模式 */
	pmc_enable_periph_clk(ID_TWI1);
	twi_options_t opts = {
		.master_clk = sysclk_get_cpu_hz(),
		.speed = 10000,
		.smbus = 0,
        .chip = 0x40
	};
    twi_master_init(TWI1,&opts);
	printf("-I- Configuring IPMB mode\n\r");


    /* 校验iic1是否正常 */
    if(twi_probe(TWI1,slave_addr[0]) != TWI_SUCCESS)
	{
	    puts("twi1 transfer failed!\n\r");
	}

    /* Replace with your application code */
	/* 循环采集cpu附近传感器温度值 */
    while (1) 
    {
	    while(uart_read(UART0,&c_choice));
	    printf("%d\r", c_choice);
#if 0
		printf("geting DDR Temperature");
        cal_temp(slave_addr[0],&TEMP_V);
		printf("TEMP_V is %02x" ,TEMP_V);
#endif
		printf("getting CPU Temperature");
		cal_temp(slave_addr[1],&TEMP_V);
#if 0
		printf("geting PEX Temperature");
		cal_temp(slave_addr[2],&TEMP_V);
#endif





    }
}
