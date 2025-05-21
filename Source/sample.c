/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "TouchPanel/TouchPanel.h"
#include "timer/timer.h"
#include <stdio.h>
#include <stdlib.h>
#include "joystick/joystick.h"
#include "../sample.h"

#include "Can/CAN.h"    


#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif
		
int maze[31][28] = {		
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
				{1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
				{1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1},
				{1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1},
				{1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1},
				{1, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, FPACMAN, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
				{1, 2, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1},
				{1, 2, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1},
				{1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 1},
				{1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1},
				{0, 0, 0, 0, 0, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0},
				{1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 9, 9, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1},
				{1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1},
				{0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0},
				{1, 1, 1, 1, 1, 1, 0, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 0, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1},
        {1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1},
				{1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1},
        {1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 0, 0, 1},
				{1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1},
        {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
				};


// VARIABILI GLOBALI
int pacman_curr_x = 12, pacman_curr_y = 5; 
int ghost_curr_x = 14, ghost_curr_y = 14, ghost_prev_x, ghost_prev_y;

char pacman_dir;	// u, d, r, l, n
char ghost_dir = 'u';
uint8_t ghost_mode = CHASE_MODE;
				
int score = 0;
uint8_t time = 60;
char score_char[5] = "", time_char[5] = "";
				
uint8_t pause = 1;
uint8_t lives = 1;
uint8_t time_points[6];			
				
extern uint8_t timeCAN;
extern uint8_t livesCAN;
extern uint8_t scoreCAN1;
extern uint8_t scoreCAN2;

extern uint16_t scoreCAN;
			
void print_maze(){
	volatile int i, j;
	for(i=0; i<31; i++){
		for(j=0; j<28; j++){
			LCD_DrawBlock(maze[i][j], 8*j, 8*i);
		}
	}
}

void print_score(){
	GUI_Text(0,0, (uint8_t *) "SCORE: ", White, Black);
	//sprintf(score_char,"%4d",score);
	sprintf(score_char,"%4d",scoreCAN);
	GUI_Text(50, 0, (uint8_t *) score_char, White, Black);
}

void print_time(){
	GUI_Text(100,0, (uint8_t *) "TIME: ", White, Black);
	//sprintf(time_char,"%2d",time);
	sprintf(time_char,"%2d",timeCAN);
	GUI_Text(140, 0, (uint8_t *) time_char, White, Black);
}

void print_lives(){
	
	GUI_Text(0,275, (uint8_t *) "LIVES: ", White, Black);
	volatile int i;
	//for(i=0; i<lives;i++){
	for(i=0; i<livesCAN;i++){
		LCD_DrawBlock(FPACMAN, 8*i + 50, 253);
		LCD_DrawBlock(BLACK, 8*(i+1) + 50, 253);
	}
}

int main(void)
{
	// INIZIALIZZAZIONI
	
  SystemInit();  												/* System Initialization (i.e., PLL)  */
  LCD_Initialization();
	LCD_Clear(Black);
	joystick_init();
	CAN_Init();
	init_RIT(0x004C4B40);	/* 50ms */
	init_timer(0, 0x003C12D0); 	/* Timer for visual updates */
	init_timer(1, 0x017D7840);	/* Timer for game seconds (1 sec)*/

	GUI_Text(8, 135, (uint8_t *) "PRESS INT0 TO START THE GAME", White, Green);

	NVIC_EnableIRQ(EINT0_IRQn);
	
	LPC_PINCON->PINSEL4 |= (1 << 20);
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);				

	LPC_PINCON->PINSEL1 |= (1<<21);
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);
	
  while (1)	
  {
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/