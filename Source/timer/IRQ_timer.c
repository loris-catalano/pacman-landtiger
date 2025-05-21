
/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include "LPC17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"
#include <stdio.h> /*for sprintf*/

#include "../sample.h"
#include "music.h"
#include "Can/CAN.h"

extern int maze[31][28];
extern int pacman_curr_x, pacman_curr_y, ghost_curr_x, ghost_curr_y, ghost_prev_x, ghost_prev_y, score;
extern uint8_t time;
extern char pacman_dir;	// u, d, r, l, n
extern char ghost_dir;
extern uint8_t ghost_mode;
extern char time_char[5], score_char[5];
extern uint8_t pause;
extern uint8_t lives;
extern uint8_t time_points[6];
extern uint8_t rit_count;

uint8_t respawn_counter=0;
uint8_t ghost_eaten=0;

uint8_t frightened_counter=0;

uint8_t arr_pos = 5;

void update_score(int score){
	sprintf(score_char,"%4d",score);
	GUI_Text(50, 0, (uint8_t *) score_char, White, Black);
}

void update_time(int time){
	sprintf(time_char,"%2d",time);
	GUI_Text(140, 0, (uint8_t *) time_char, White, Black);
}

void add_life(){
	lives++;
	print_lives();
}

void game_over(){
		disable_timer(0);
		disable_timer(1);
		disable_timer(2);
		disable_timer(3);
		disable_RIT();
		NVIC_DisableIRQ(EINT0_IRQn);
		GUI_Text(90, 135, (uint8_t *) "GAME OVER", White, Red);
}

void respawn_ghost(){
		ghost_mode = CHASE_MODE;
		respawn_counter = 0;
		ghost_eaten = 0;
	
		ghost_curr_x = 14;
		ghost_curr_y = 14;
}

void enter_frightened_mode(){
		ghost_mode = FRIGHTENED_MODE;
}
void enter_chase_mode(){
		ghost_mode = CHASE_MODE;
		frightened_counter = 0;
}

void print_power_pill(){
	int seed = LPC_TIM0->TC;
	srand(seed);
	
	uint8_t x,y;
	
	x = rand() % 28;
	y = rand() % 31;
	
/* While the spawned pill is spawned on forbidden blocks (!= from STDPILL && BLACK), OR on not reachable blocks */
	while( (maze[y][x] != STDPILL && maze[y][x] != BLACK)   ||  ((y==10||y==11||y==17||y==18) && x<=4)  ||  ((y==10||y==11||y==17||y==18) && x>=23) ||  ((y>=13 && y<=15) && (x>=11 && x<=16))){
		/*Generate new coordinates (spawn the pill in a different location)*/
		x = rand() % 28;
		y = rand() % 31;	
	}
	
	maze[y][x] = PWRPILL;
	LCD_DrawBlock(PWRPILL, x * 8, y * 8);
}

int manhattan_distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

char compute_next_ghost_dir(){
		char directions[] = {'u', 'd', 'l', 'r'};
    int dx[] = {0, 0, -1, 1};   
    int dy[] = {-1, 1, 0, 0};  
	
		char best_dir = 'n';
		
		if(ghost_mode == CHASE_MODE){
			int min_distance = 99999;
			int i;

			for (i = 0; i < 4; i++) {
					int next_x = ghost_curr_x + dx[i];
					int next_y = ghost_curr_y + dy[i];
					
					if (next_y >= 0 && next_y < 31 && next_x >= 0 && next_x < 28 && maze[next_y][next_x] != WALL	&& (next_x!=ghost_prev_x || next_y!=ghost_prev_y)  ) {
						int distance = manhattan_distance(next_x, next_y, pacman_curr_x, pacman_curr_y);

							if (distance < min_distance ) {	
									min_distance = distance;
									best_dir = directions[i];
							}
					}
			}
		}else{
			int max_distance = 0;
			int i;     

			for (i = 0; i < 4; i++) {
					int next_x = ghost_curr_x + dx[i];
					int next_y = ghost_curr_y + dy[i];
					
					if (next_y >= 0 && next_y < 31 && next_x >= 0 && next_x < 28 && maze[next_y][next_x] != WALL &&  (next_x!=ghost_prev_x || next_y!=ghost_prev_y)) {
						int distance = manhattan_distance(next_x, next_y, pacman_curr_x, pacman_curr_y);

							if (distance > max_distance) {
									max_distance = distance;
									best_dir = directions[i];
							}
					}
			}
		}
		return best_dir;
}


void move_pacman(char dir, int *curr_x, int *curr_y) {
    int next_x, next_y;

    switch(dir) {
        case 'u':
            next_x = *curr_x;
            next_y = *curr_y - 1;
            break;
        case 'd':
            next_x = *curr_x;
            next_y = *curr_y + 1;
            break;
        case 'r':
            next_x = *curr_x + 1;
            next_y = *curr_y;
            break;
        case 'l':
            next_x = *curr_x - 1;
            next_y = *curr_y;
            break;
        default: // if joystick is not moved
            next_x = *curr_x;
            next_y = *curr_y;
            break;
    }

		//Teleport
    if (*curr_x == 0 && *curr_y == 14 && dir == 'l') {
        next_x = 27;
    } else if (*curr_x == 27 && *curr_y == 14 && dir == 'r') {
        next_x = 0;
    }

    uint8_t next_block = maze[next_y][next_x];

		switch(next_block) {
					case(WALL):
							dir = 'n'; // STOP
							return;
					case(MAGENTALINE):
							dir = 'n'; // STOP
							return;
					case(STDPILL):
							score += 10;
							update_score(score);
							if (score % 1000 == 0) {
									add_life();
							}

							/*NOTE nota;
							nota.freq = c4;
							nota.duration = time_biscroma;
							playNote(nota);		*/			
							
							break;
					case(PWRPILL):
							score += 50;
							update_score(score);
							enter_frightened_mode();
							if (score % 1000 == 0) {
									add_life();
							}
							
							/* POWER PILL SOUND EFFECT */
							NOTE nota;
							nota.freq = c5;
							nota.duration = time_biscroma;
							playNote(nota);
							
							break;
		}

    if (*curr_x != next_x || *curr_y != next_y) {
					maze[*curr_y][*curr_x] = BLACK;
					LCD_DrawBlock(BLACK, *curr_x * 8, *curr_y * 8);
				
					maze[next_y][next_x] = FPACMAN;
					LCD_DrawBlock(FPACMAN, next_x * 8, next_y * 8);
			}

        *curr_x = next_x;
        *curr_y = next_y;
}


void move_ghost(char dir, int *curr_x, int *curr_y) {
    int next_x, next_y;

    switch(dir) {
        case 'u':			
            next_x = *curr_x;
            next_y = *curr_y - 1;
            break;
        case 'd':
            next_x = *curr_x;
            next_y = *curr_y + 1;
            break;
        case 'r':
            next_x = *curr_x + 1;
            next_y = *curr_y;
            break;
        case 'l':
            next_x = *curr_x - 1;
            next_y = *curr_y;
            break;
        default: // if joystick is not moved
            next_x = *curr_x;
            next_y = *curr_y;
            break;
    }

		//Teleport
    if (*curr_x == 0 && *curr_y == 14 && dir == 'l') {
        next_x = 27;
    } else if (*curr_x == 27 && *curr_y == 14 && dir == 'r') {
        next_x = 0;
    }

    if(*curr_x != next_x || *curr_y != next_y) {
					LCD_DrawBlock(maze[*curr_y][*curr_x], *curr_x * 8, *curr_y * 8);
					LCD_DrawBlock(GHOST, next_x * 8, next_y * 8);
		}	
	
		ghost_prev_x = *curr_x;
		ghost_prev_y = *curr_y;
		*curr_x = next_x;
		*curr_y = next_y;
}


/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler: handles pacman movements and CAN
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER0_IRQHandler (void)
{
	/* CAN */
	uint8_t halfScore1 = (uint8_t)score;
	uint8_t halfScore2 = ((uint8_t)(score >> 8));
	
	CAN_TxMsg.data[0] = time;
	CAN_TxMsg.data[1] = lives;
	CAN_TxMsg.data[2] = halfScore1;
	CAN_TxMsg.data[3] = halfScore2;
	CAN_TxMsg.len = 4;
	CAN_TxMsg.id = 2;
	CAN_TxMsg.format = STANDARD_FORMAT;
	CAN_TxMsg.type = DATA_FRAME;
	CAN_wrMsg (1, &CAN_TxMsg);        
	
	
	move_pacman(pacman_dir, &pacman_curr_x, &pacman_curr_y); 
		
	if(ghost_curr_y == pacman_curr_y && ghost_curr_x == pacman_curr_x){
		if(ghost_mode == CHASE_MODE){
			if(lives>1){
				lives--;
			}
			else {
				lives=0;
				print_lives();
				game_over();
			}
		}else{ //if ghost_mode == FRIGHTENED_MODE
			score += 100;
			ghost_eaten = 1;
			//The rest is handled in timer 1 (waiting of 3 seconds)
		}
	}
	
  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler: For the timer of the game (1 second)
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
uint8_t ghost_speedup = 7;	// The higher the slower 
void TIMER1_IRQHandler (void)
{
	time--;
	update_time(time);
	
	/* GHOST SPEEDUP */
	if(time%13 == 0){
		ghost_speedup--;
	}
	
	/* GAME OVER */
	if(time == 0){
		game_over();
	}
	
	/* VICTORY */
	if(score == 240*10 + 6*50){
		GUI_Text(90, 135, (uint8_t *) "VICTORY!", White, Green);
		disable_timer(0);
		disable_timer(1);
		disable_timer(2);
		disable_timer(3);
		disable_RIT();
	}
	
	/* POWER PILLS RANDOM SPAWN TIME */
	while(time == time_points[arr_pos] && time >0){		/* using while statement we also consider the case when there are multiple pills spawned at the same time */
		print_power_pill();
		arr_pos--;
	}
	
	/* FRIGHTENED MODE (10 sec) */
	if(ghost_mode == FRIGHTENED_MODE && frightened_counter<10){
		frightened_counter++;
	}
	else if(frightened_counter == 10){
		enter_chase_mode();
	}
	/* GHOST RESPAWNING AFTER BEING EATEN */
	if(ghost_eaten == 1){
		
		if(respawn_counter<3) respawn_counter++;
		else{
			respawn_ghost();
		}
	}
	
	
	
	print_lives();
	
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}





uint16_t SinTable[45] =                                    
{
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

/******************************************************************************
** Function name:		Timer2_IRQHandler
**
** Descriptions:		Timer/Counter 2 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER2_IRQHandler (void)
{	
	static int sineticks=0;
	/* DAC management */	
	static int currentValue; 
	currentValue = SinTable[sineticks];
	currentValue -= 410;
	currentValue /= 1;
	currentValue += 410;
	LPC_DAC->DACR = currentValue <<6;
	sineticks++;
	if(sineticks==45) sineticks=0;
	
  LPC_TIM2->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
** Function name:		Timer3_IRQHandler
**
** Descriptions:		Timer/Counter 3 interrupt handler: handles ghost movements
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER3_IRQHandler (void)
{
	disable_timer(2);
	LPC_TIM2->TC =0;	//	Reset Timer 2

  LPC_TIM3->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
**                            End Of File
******************************************************************************/
