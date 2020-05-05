// main.cpp
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10 in C++

// Last Modified: 1/17/2020 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2017

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include <vector>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "SlidePot.h"
#include "Images.h"
#include "UART.h"
#include "Timer0.h"
#include "Timer1.h"
#include <stdlib.h>
#include "Sound.h"

#define enemiesperRow 5
#define bulletSpeed 7
#define enemymovespeed 5	//interrupts per move (bigger is slower);
#define gap 5

SlidePot my(163,20);
int move;
int currentDirection = 1;	//right is 1, left is -1
int totalscore;
int row = -1;

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);
extern "C" void GPIOF_Handler(void);


typedef enum {dead,alive} status_t;
struct sprite{
  int16_t x;      // x coordinate
  int16_t y;      // y coordinate
  const unsigned short *image; // ptr->image
  status_t life;            // dead/alive
	uint8_t points;
	uint8_t w;
	uint8_t h;
};

struct sprite enemies[20];
int numberofEnemies = 20;

struct sprite bullet = {0,0,bulletimg,dead};

int flag = 0;

SlidePot player(163,20);

void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;
  NVIC_ST_RELOAD_R = 2666666;
  NVIC_ST_CURRENT_R = 0;      
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1
  NVIC_ST_CTRL_R = 0x07;
}

void switchModels(int index){
	if(enemies[index].image == SmallEnemy10pointA){
		enemies[index].image = SmallEnemy10pointB;
	}else if(enemies[index].image == SmallEnemy10pointB){
		enemies[index].image = SmallEnemy10pointA;
	}else if(enemies[index].image == SmallEnemy20pointA){
		enemies[index].image = SmallEnemy20pointB;
	}else if(enemies[index].image == SmallEnemy20pointB){
		enemies[index].image = SmallEnemy20pointA;
	}else if(enemies[index].image == SmallEnemy30pointA){
		enemies[index].image = SmallEnemy30pointB;
	}else if(enemies[index].image == SmallEnemy30pointB){
		enemies[index].image = SmallEnemy30pointA;
	}
}

void SysTick_Handler(void){
	
	int playerPos = ADC_In();
	player.Save(playerPos); 
	
	bullet.y -= bulletSpeed;
	
	if(move % enemymovespeed == 0){
		if(move < (enemymovespeed)*(128-((16+gap)*enemiesperRow))){		//moving only until the edge of screen
			for(int i = 0; i < numberofEnemies; i++){
				enemies[i].x += currentDirection;			//only moves the enemies every "enemymovespeed" frames
			}
		}else if(move < (enemymovespeed)*((128-((16+gap)*enemiesperRow))+15)){
			for(int i = 0; i < numberofEnemies; i++){
				enemies[i].y ++;
			}
		}
	}
	if(move % 20 == 0){
		for(int i = 0; i < numberofEnemies; i++){
			switchModels(i);
		}
	}
	
	if(move >= (enemymovespeed) * ((128-((16+gap)*enemiesperRow))+10)){
		move = 0;														//reset move if we have "touched" the edge and moved down
		currentDirection *= -1;
	}
	
	move++;
	flag = 1;
}

void enemiesLvl1(void){
	for(int i = 0; i < numberofEnemies; i++){
		enemies[i].w = 16;
		enemies[i].h = 10;
		enemies[i].x = (i%enemiesperRow)*(enemies[i].w+gap);
		enemies[i].y = ((i/enemiesperRow)+1)*15 + 10;
		enemies[i].life = alive;
		if(i/enemiesperRow == 0){
			enemies[i].image = SmallEnemy30pointB;
			enemies[i].points = 30;
		}else if(i/enemiesperRow == 1){
			enemies[i].image = SmallEnemy20pointB;
			enemies[i].points = 20;
		}else{
			enemies[i].image = SmallEnemy10pointB;
			enemies[i].points = 10;
		}
	}
}

void EdgeTrigger_Init(){
	volatile int delay = 0;
	SYSCTL_RCGCGPIO_R |= 0x20;
	delay = 0;
	GPIO_PORTF_DIR_R &= ~0x10;
	GPIO_PORTF_AFSEL_R &= ~0x10;
	GPIO_PORTF_DEN_R |= 0x10;
	GPIO_PORTF_PCTL_R &=0x000F0000;
	GPIO_PORTF_AMSEL_R &= ~0x10;
	GPIO_PORTF_PUR_R |= 0x10;
	GPIO_PORTF_IS_R &= ~0x10;
	GPIO_PORTF_IBE_R &= ~0x10;
	GPIO_PORTF_IEV_R &= ~0x10;
	GPIO_PORTF_ICR_R = 0x10;
	GPIO_PORTF_IM_R |= 0x10;
	NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000;
	NVIC_EN0_R = 0x40000000;
}

void GPIOF_Handler(void){
	GPIO_PORTF_ICR_R = 0x10;
	if(bullet.life == dead){
		bullet.life = alive;
		bullet.x = player.getPixel() + 9;
		bullet.y = 150;
		bullet.w = 2;
		bullet.h = 7;
		Sound_Shoot();
	}
}

void PortF (void){
	SYSCTL_RCGCGPIO_R |= 0x20;
	volatile int delay = 0;
	delay = 0;
	delay = 0;
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
	GPIO_PORTF_CR_R |= 0xFF;
	GPIO_PORTF_DIR_R &= ~0x11;
	GPIO_PORTF_DEN_R |= 0x11;
	GPIO_PORTF_PUR_R |= 0x11;


}

const char Hello_English[] ="Welcome Friend!";
const char Welcome_English[] = "to the World of";
const char Welcome_English2[] = "Space Invaders!";

const char Hello_Spanish[] ="Hola Amigos!";
const char Welcome_Spanish[] = "al mundo de";
const char Welcome_Spanish2[] = "Invasores Espaciales";

const char GameOver_English[] = "Game Over";
const char GameOver_Spanish[] = "Juego Terminado";

const char YouWon_English[] = "You Won";
const char YouWon_Spanish[] = "Tu Ganaste";

//const char YouLost_English[] = "You Lost";
//const char YouLost_Spanish[] = "Tu Perdiste";

const char YouLost_English[] = "Nice Try";
const char YouLost_Spanish[] = "Buen Intento";

const char Earthling_English[] = "Earthling!";
const char Earthling_Spanish[] = "Terricola!";
	
const char Score_English[] = "Score:";
//const char Score_Spanish[] = "Puntacion del Juego";
const char Score_Spanish[] = "Puntuacion:";


const char* sentences[2][8] = {
	{Hello_English, Welcome_English, Welcome_English2, GameOver_English, YouWon_English, YouLost_English, Score_English,Earthling_English},
	{Hello_Spanish, Welcome_Spanish, Welcome_Spanish2, GameOver_Spanish, YouWon_Spanish, YouLost_Spanish, Score_Spanish,Earthling_Spanish}
};

void LanguageSelection(void){
	ST7735_SetCursor(1,0);
	ST7735_OutString((char*)"Choose a language");
	ST7735_SetCursor(2,4);
	ST7735_OutString((char*)"English (Sw1)");
	ST7735_SetCursor(2,5);
	ST7735_OutString((char*)"Espa\xA4ol (Sw2)");
	
	int x = 0;
	while (x == 0){
			if ((GPIO_PORTF_DATA_R & 0x01) == 0x00){ //Spanish
				x = 1;
				row = 1;
			}
			if ((GPIO_PORTF_DATA_R & 0x10) == 0x00){ //English
				x = 1;
				row = 0;
			}
	}
}

void Delayms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 72724*2/91;  // 1msec, tuned at 80 MHz
    while(time){
      time--;
    }
    n--;
  }
}

void Welcome(void){
	ST7735_FillScreen(0);
	
	ST7735_SetCursor(4,5);
	ST7735_OutString((char*)sentences[row][0]);
	ST7735_SetCursor(4,6);
	ST7735_OutString((char*)sentences[row][1]);
	if(row == 0){
		ST7735_SetCursor(4,7);
	}else{
		ST7735_SetCursor(0,7);
	}
	ST7735_OutString((char*)sentences[row][2]);
	
	Delayms(5000);
	ST7735_FillScreen(0);
	
	
}

void Instructions(void){}//Tell the instructions

int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Random_Init(1);
  Output_Init();
	PortF();
	//LanguageSelection();
	ADC_Init();
  //Timer0_Init(&background,1600000); // 50 Hz
  //Timer1_Init(&clock,80000000); // 1 Hz
	SysTick_Init();
	EdgeTrigger_Init();
	enemiesLvl1();
	EnableInterrupts();
	//Welcome();
	//Instructions();		//tell the player the instructions
	bool gameOver = false;
	uint8_t deadEnemies = 0;
	ST7735_DrawBitmap(0,160,beach,128,160);
	while(!gameOver && deadEnemies != numberofEnemies){
		while(flag == 0){}
		flag = 0;

		if(bullet.life == alive){
			if((bullet.y - 7) <= 0){			//remove the bullet if its top has gone out of bounds
				bullet.life = dead;
			}else{
				ST7735_DrawBitmap(bullet.x,bullet.y,bullet.image,bullet.w,bullet.h);
			}
			ST7735_FillRect(bullet.x,bullet.y,2,bulletSpeed,0);									//cover up the old position of the bullet
		}
			

		for(int i = 0; i < numberofEnemies; i++){
			if((bullet.life == alive) && (enemies[i].life == alive) && (bullet.x >= enemies[i].x) && bullet.x <= (enemies[i].x + 16)
				&& ((bullet.y - 7) <= enemies[i].y) && ((bullet.y - 7) > enemies[i].y - 10)){
				bullet.life = dead;
				enemies[i].life = dead;
				ST7735_FillRect(enemies[i].x,enemies[i].y - enemies[i].h,enemies[i].w,enemies[i].h,0);		//remove the enemy sprite
				ST7735_FillRect(bullet.x,bullet.y - bullet.h,bullet.w,bullet.h,0);												//remove the bullet sprite
				totalscore += enemies[i].points;
				
				deadEnemies++;
				Sound_Killed();
				//add explosion sound.
			}
				
			if(enemies[i].life == alive){
				if(enemies[i].y >= 150){
					gameOver = true;
					break;
				}
 				ST7735_DrawBitmap(enemies[i].x,enemies[i].y,enemies[i].image,enemies[i].w,enemies[i].h);
			}	
		}
		ST7735_SetCursor(0,0);
		ST7735_OutString((char*)sentences[row][6]);			//replace this with the language phrase;
		ST7735_OutUDec(totalscore);
		ST7735_DrawBitmap(player.getPixel(),159,PlayerShip0,22,8);
	}
	Delayms(5000);
	if(deadEnemies == numberofEnemies){
		ST7735_SetCursor(1, 1);
		//ST7735_OutString((char*)"GAME OVER");
		//ST7735_SetCursor(1, 2);
		ST7735_SetTextColor(ST7735_WHITE);
		ST7735_OutString((char*)sentences[row][4]);
		ST7735_SetCursor(1, 3);
		ST7735_OutString((char*)sentences[row][7]);
		ST7735_SetCursor(2, 4);
		ST7735_SetTextColor(ST7735_WHITE);
	}else{
		ST7735_FillScreen(0x0000);            // set screen to black
		ST7735_SetCursor(2, 1);
		ST7735_OutString((char*)sentences[row][6]);
		ST7735_OutUDec(totalscore);
		ST7735_SetCursor(1, 2);
		ST7735_OutString((char*)sentences[row][3]);
		ST7735_SetCursor(1, 3);
		ST7735_SetTextColor(ST7735_WHITE);
		ST7735_OutString((char*)sentences[row][5]);
		ST7735_SetCursor(1, 4);
		ST7735_OutString((char*)sentences[row][7]);
		ST7735_SetCursor(2, 5);
		ST7735_SetTextColor(ST7735_WHITE);
	}
}


















































//sprite_t bill={60,9,SmallEnemy20pointB,alive};

//uint32_t time = 0;
//volatile uint32_t flag;
//void background(void){
//  flag = 1; // semaphore
//  if(bill.life == alive){
//    bill.y++;
//  }
//  if(bill.y>155){
//    bill.life = dead;
//  }
//}
//void clock(void){
//  time++;
//}

//int main(void){
//  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
//  Random_Init(1);
//  Output_Init();
//  Timer0_Init(&background,1600000); // 50 Hz
//  Timer1_Init(&clock,80000000); // 1 Hz
//  EnableInterrupts();
//  ST7735_DrawBitmap(52, 159, PlayerShip0, 18,8); // player ship middle bottom
//  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
//  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
//  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
//  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
//  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
//  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);
//  while(bill.life == alive){
//    while(flag==0){};
//    flag = 0;
//    ST7735_DrawBitmap(bill.x,bill.y,bill.image,16,10);
//  }

//  ST7735_FillScreen(0x0000);            // set screen to black
//  ST7735_SetCursor(1, 1);
//  ST7735_OutString((char*)"GAME OVER");
//  ST7735_SetCursor(1, 2);
//  ST7735_SetTextColor(ST7735_WHITE);
//  ST7735_OutString((char*)"Nice try,");
//  ST7735_SetCursor(1, 3);
//  ST7735_OutString((char*)"Earthling!");
//  ST7735_SetCursor(2, 4);
//  ST7735_SetTextColor(ST7735_WHITE);
//  while(1){
//    while(flag==0){};
//    flag = 0;
//    ST7735_SetCursor(2, 4);
//    ST7735_OutUDec(time);
//  }

//}



