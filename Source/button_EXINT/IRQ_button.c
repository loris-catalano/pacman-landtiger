#include "button.h"
#include "LPC17xx.h"
#include "../GLCD/GLCD.h" 
#include "../sample.h"

#include <stdio.h>

extern uint8_t pause;
extern uint8_t time;
extern uint8_t time_points[6];

int compare(const void *a, const void *b) {
    return (*(uint8_t *)a - *(uint8_t *)b);
}

void generate_time_points(){
	int seed = LPC_RIT->RICOUNTER;
	srand(seed);
				
	uint8_t i;
			
				/* Generate 6 random timestamps between 20 and 60 */
	for(i=0; i<6; i++){
			time_points[i] = 20 + rand() % 41;
	}
	
	qsort(time_points, 6, sizeof(uint8_t), compare);
}




void EINT0_IRQHandler (void)	  	/* INT0	handler */
{		
		NVIC_DisableIRQ(EINT0_IRQn);
	
		print_score();
		print_time();
		print_lives();
	
		LPC_PINCON->PINSEL4 &= ~(1 <<20 );
		
		if(pause == 1){
			if(time == 60){ //If it's the first time
				generate_time_points();
			}
			enable_RIT();
			enable_timer(0);
			enable_timer(1);
			pause = 0;
			print_maze();
		}else{
			disable_timer(0);
			disable_timer(1);
			pause = 1;
			GUI_Text(100, 130, (uint8_t *) "PAUSE", White, Red);
		}
	
	LPC_SC->EXTINT &= (1 << 0);    /* clear pending interrupt         */
}
