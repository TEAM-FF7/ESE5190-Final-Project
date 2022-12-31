#include <stdio.h>
#include "pico/stdlib.h"
#include <tusb.h>
#include "pico/multicore.h"
#include "arducam/arducam.h"
#include "hardware/pio.h"
#include "lib/lsm6ds3.h"
#include "lib/pio_i2c.h"
#include "lib/st7735.h"
#include "lib/fonts.h"
#include "lib/draw.h"
#include "time.h"
#include "stdlib.h"

uint8_t image_buf[324*324];
uint8_t displayBuf[80*160*2];
uint8_t header[2] = {0x55,0xAA};

#define FLAG_VALUE 123
#define RESTART_PIN 

bool restart = true;

PIO pio = pio1;
uint sm = 1;


void gameOver(){
	char over[] = "Game Over";
 	ST7735_WriteString(8, 50, over, Font_16x26, ST7735_RED, ST7735_WHITE);
	bool loop = true;
	int16_t data[3];
	while (loop){
		readGyro(pio,sm,data);
		if (data[2] < -2000)
			loop = false;
		sleep_ms(50);
	}
	restart = true;
}

void core1_entry() {
	multicore_fifo_push_blocking(FLAG_VALUE);

	uint32_t g = multicore_fifo_pop_blocking();

	if (g != FLAG_VALUE)
		printf("Hmm, that's not right on core 1!\n");
	else
		printf("It's all gone well on core 1!\n");

	gpio_init(PIN_LED);
	gpio_set_dir(PIN_LED, GPIO_OUT);

	ST7735_Init();
	ST7735_DrawImage(0, 0, 80, 160, arducam_logo);
	uint offset = pio_add_program(pio, &i2c_program);
    i2c_program_init(pio, sm, offset, PIN_SDA, PIN_SCL);

	LSM6DS3_init(pio, sm);

	struct arducam_config config;
	config.sccb = i2c0;
	config.sccb_mode = I2C_MODE_16_8;
	config.sensor_address = 0x24;
	config.pin_sioc = PIN_CAM_SIOC;
	config.pin_siod = PIN_CAM_SIOD;
	config.pin_resetb = PIN_CAM_RESETB;
	config.pin_xclk = PIN_CAM_XCLK;
	config.pin_vsync = PIN_CAM_VSYNC;
	config.pin_y2_pio_base = PIN_CAM_Y2_PIO_BASE;

	config.pio = pio0;
	config.pio_sm = 0;

	config.dma_channel = 0;
	config.image_buf = image_buf;
	config.image_buf_size = sizeof(image_buf);

	arducam_init(&config);

	
	int16_t data[3];

	srand(time(NULL));

	// lane initialize
	int16_t lane_y_int[5];// = {-40,0,40,80,120};
	float lane_y[5];// = {-40,0,40,80,120};
	float mov_speed;// = 2;
	
	// block initialize
	int16_t block_y_int[3];
	float time; 
	//generate length factor 90-150 
	float block_y[3];// = {-40,-40 - (rand()%60 + 90),-40 -(2 * rand()%60 + 90)};
	uint16_t block_x[3]; 
	uint16_t x_position[3] = {16,40,64}; //three possible spawn location of block
	uint16_t block_w[3];
	uint16_t block_h[3];

	//car spawn position 
	uint16_t car_x;
	int16_t car_y;
	uint16_t car_w;
	uint16_t car_h;
	uint32_t control;

	bool flag = false;

	uint integtime = 3;
	bool integflag = false;
	float gyrosum[2] = {0,0};
	
	while (true) {
		gpio_put(PIN_LED, !gpio_get(PIN_LED)); 
		arducam_capture_frame(&config);

		uint16_t index = 0;
		for (int y = 0; y < 160; y++) {
			for (int x = 0; x < 80; x++) {
				uint8_t c = image_buf[(2+320-2*y)*324+(2+40+2*x)];
				uint16_t imageRGB   = ST7735_COLOR565(c, c, c);
				displayBuf[index++] = (uint8_t)(imageRGB >> 8) & 0xFF;
				displayBuf[index++] = (uint8_t)(imageRGB)&0xFF;
				}
		}
		//all parameter restart
		if (restart){
			car_x = 32;
			car_y = 120;
			car_w = 16;
			car_h = 30;
			for (int i = 0; i < 5; i++){
				lane_y[i] = (i - 1) * 40;
			}
			mov_speed = 2;
			for (int i = 0; i < 3; i++){
				block_y[i] = -40 - i*(rand()%40+100);
				int r = rand() % 3;
				block_x[i] = x_position[r];
				r = rand() % 4;
				block_w[i] = 15;	//fixed width of car block
				block_h[i] = rand() % 6 + 25;	//random length of car block 
				block_x[i] = block_x[i] - block_w[i] / 2;
			}
			time = 0;
			restart = false;
		}

		//draw grass sides; boundary of the game for easy divide of blocks 
		//left 4 right 4, 80 - 12 = 68 left
		draw_rec_onbuf(0,0,4,160,displayBuf,ST7735_DARK_GREEN);
		draw_rec_onbuf(76,0,4,160,displayBuf,ST7735_DARK_GREEN);
		
		//draw lanes 
		for(int i = 0; i < 5; i++) {
			lane_y[i] = lane_y[i] + mov_speed;
			lane_y_int[i] = (int16_t)lane_y[i];
			draw_rec_onbuf(26,lane_y_int[i],3,28,displayBuf,ST7735_WHITE);
			draw_rec_onbuf(52,lane_y_int[i],3,28,displayBuf,ST7735_WHITE);
			if (lane_y[i] >= 160) lane_y[i] = -40;
		}

		//draw block 
		for(int i = 0; i < 3; i++){
			block_y[i] = block_y[i] + mov_speed;
			block_y_int[i] = (int16_t)block_y[i];
			draw_rec_onbuf(block_x[i],block_y_int[i],block_w[i],block_h[i],displayBuf,ST7735_BLUE);
			if (block_y[i] >= 160){
				int r = rand() % 3;
				block_x[i] = x_position[r];
				block_w[i] = 15;	//fixed width of car block
				block_h[i] = rand() % 6 + 25;	//random length of car block 
				block_x[i] = block_x[i] - block_w[i] / 2;
				block_y[i] = block_y[(i+2)%3] -(rand()%30 + 100);
			}
		}


		//repl control
		control = getchar_timeout_us(0);
		if ((control != PICO_ERROR_TIMEOUT) && (!flag)){
			switch(control){
            case 'a': // change r/w mode
                car_x = car_x - 5;
				car_x = car_x_lim(car_x,car_w);
				flag = true;
                break;
			case 'd': // change r/w mode
				car_x = car_x + 5;
				car_x = car_x_lim(car_x,car_w);
				flag = true;
				break;
			case 'w':
				car_y = car_y - mov_speed;
				car_y = car_y_lim(car_y);
				flag = true;
				break;
			case 's':
				car_y = car_y + mov_speed;
				car_y = car_y_lim(car_y);
				flag = true;
				break;
			}
		}
		if ((control = PICO_ERROR_TIMEOUT) && (flag))
			flag = false;

		// sensor control

		// rapid change response
		readGyro(pio, sm, data);
		for (int i = 0; i < 2; i++){
			if ((integflag == false) && (data[i] > 5) || (data[i] < -5)){
				integflag = true;
			}	
		}
		if (integflag){
			for (int i = 0; i < 2; i++){
				gyrosum[i] += data[i];
			}
			integtime -= 1;
		}
		if (integtime == 0){
			integflag = false;
			car_y = car_y + mov_speed + 0.2*gyrosum[1];
			car_y = car_y_lim(car_y);
			car_x = car_x + 0.2*gyrosum[0];
			car_x = car_x_lim(car_x,car_w);
		}
			
		//Fixed tilt response
		readAcceleration(pio, sm, data);
        printf("x: %d, y: %d, z: %d.\n",data[0] ,data[1] ,data[2]);
		if (data[1] > -1000) {
			car_y = car_y + mov_speed + (data[1] + 1000) / 600;
			car_y = car_y_lim(car_y);
		}
		else if (data[1] < -5500){
			car_y = car_y - mov_speed + (data[1] + 5500) / 1000;
			car_y = car_y_lim(car_y);
		}
		if (data[0] > 1700) {
			car_x = car_x - 2 - (data[0] - 1700) / 725;
			car_x = car_x_lim(car_x,car_w);
		}
		else if (data[0] < -1700){
			car_x = car_x + 2 - (data[0] + 1700) / 725;
			car_x = car_x_lim(car_x,car_w);
		}

		car_y = (int16_t)(car_y + mov_speed/2);
		car_y = car_y_lim(car_y);
		
		bool collFlag = 0;
		for ( int i = 0; i < 3; i++){
			bool collisionX = block_x[i] + block_w[i] >= car_x && car_x + car_w >= block_x[i];
			bool collisionY = block_y_int[i] + block_h[i] >= car_y && car_y + car_h >= block_y_int[i];
			collFlag = collisionX && collisionY;
			if(collFlag){
				break;
			}
		}
		
		draw_car(car_x,car_y,car_w, car_h,displayBuf,ST7735_RED);
		
	  	ST7735_DrawImage(0, 0, 80, 160, displayBuf);

		if (mov_speed <= 7.5){
			mov_speed += (rand() % 2) * 0.012;
		}
		

		if (collFlag){
			gameOver();
		}

	  
	}
}



#include "hardware/vreg.h"

int main() {
	int loops=20;
	stdio_init_all();
	while (!tud_cdc_connected()) { sleep_ms(100); if (--loops==0) break;  }

	printf("tud_cdc_connected(%d)\n", tud_cdc_connected()?1:0);

	vreg_set_voltage(VREG_VOLTAGE_1_30);
	sleep_ms(1000);
	set_sys_clock_khz(250000, true);

	multicore_launch_core1(core1_entry);

	uint32_t g = multicore_fifo_pop_blocking();

	if (g != FLAG_VALUE)
		printf("Hmm, that's not right on core 0!\n");
	else {
		multicore_fifo_push_blocking(FLAG_VALUE);
		printf("It's all gone well on core 0!\n");
	}

	while (1)
	tight_loop_contents();
}
