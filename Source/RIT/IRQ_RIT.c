/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
#include "GLCD/GLCD.h" 
#include <stdio.h>

#include "../sample.h"
#include "music.h"


/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

#define UPTICKS 1

/*NOTE song[] = 
{
   {g3, time_semicroma}, {a3, time_semicroma}, {b3, time_semicroma}, {c4, time_semicroma}, 
   {d4, time_biscroma}, {e4, time_croma}, {f4, time_croma}, {g4, time_semicroma}, 
   {a4, time_semicroma}, {b4, time_semicroma}, {c5, time_semicroma}, {g4, time_biscroma}, 
   {f4, time_croma}, {e4, time_croma},

    // 2
    {g3, time_semicroma}, {a3, time_semicroma}, {b3, time_semicroma}, {c4, time_semicroma}, 
    {d4, time_biscroma}, {e4, time_croma}, {f4, time_croma}, {g4, time_biscroma}, 
    {f4, time_biscroma}, {e4, time_biscroma}, {d4, time_biscroma}, {c4, time_biscroma}, 
    {b3, time_biscroma}, {a3, time_biscroma}, {g3, time_croma}, {f3, time_croma}
};*/

NOTE song[] = 
{
    {d3, time_semicroma}, 
    {d3, time_semicroma}, 
    {d3, time_semicroma},
    {g3, time_semiminima},
    {d4, time_semiminima}, 
    {c4, time_semicroma},
    {b3, time_semicroma}, 
    {a3, time_semicroma}, 
    {g4, time_semiminima}, 
    {d4, time_croma},
    {c4, time_semicroma}, 
    {b3, time_semicroma}, 
    {a3, time_semicroma}, 
    {g4, time_semiminima}, 
    {d4, time_croma}, 
    {c4, time_semicroma}, 
    {b3, time_semicroma},
    {c4, time_semicroma}, 
    {a3, time_semiminima}, 

    {d3, time_semicroma}, 
    {d3, time_semicroma}, 
    {d3, time_semicroma},
    {g3, time_semiminima},
    {d4, time_semiminima}, 
    {c4, time_semicroma},
    {b3, time_semicroma}, 
    {a3, time_semicroma}, 
    {g4, time_semiminima}, 
    {d4, time_croma},
    {c4, time_semicroma}, 
    {b3, time_semicroma}, 
    {a3, time_semicroma}, 
    {g4, time_semiminima}, 
    {d4, time_croma}, 
    {c4, time_semicroma}, 
    {b3, time_semicroma},
    {c4, time_semicroma}, 
    {a3, time_semiminima}, 

    {d3, time_semicroma},
    {d3, time_biscroma}, 
    {e3, time_croma+time_semicroma}, 
    {e3, time_semicroma},
    {c4, time_semicroma}, 
    {b3, time_semicroma},  
    {a3, time_semicroma},
    {g3, time_semicroma}, 
    {g3, time_semicroma}, 
    {a3, time_semicroma}, 
    {b3, time_semicroma}, 
    {a3, time_croma}, 
    {e3, time_semicroma}, 
    {f3b, time_croma},
    {d3, time_semicroma},
    {d3, time_biscroma}, 
    {e3, time_semicroma+time_croma}, 
    {e3, time_semicroma},
    {c4, time_semicroma}, 
    {b3, time_semicroma},  
    {a3, time_semicroma},
    {g3, time_semicroma},  
    {d4, time_croma},
    {a3, time_semiminima},
    {d3, time_semicroma},
    {d3, time_biscroma}, 
    {e3, time_semicroma+time_croma}, 
    {e3, time_semicroma},
    {c4, time_semicroma}, 
    {b3, time_semicroma},  
    {a3, time_semicroma},
    {g3, time_semicroma}, 
    {g3, time_semicroma}, 
    {a3, time_semicroma}, 
    {b3, time_semicroma}, 
    {a3, time_semicroma}, 
    {e3, time_semicroma}, 
    {f3b, time_croma}, 
    {d3, time_semicroma},
    {d3, time_biscroma}, 
    {a4, time_croma}, 
    {g4, time_semicroma},
    {f4b, time_croma}, 
    {e4, time_semicroma},  
    {d4, time_croma},
    {c4, time_semicroma}, 
    {b3, time_croma}, 
    {a3, time_semicroma}, 
    {d4, time_minima}, 
    {pause_music, time_croma},
    {pause_music, time_semicroma},    

};

extern int maze[31][28];
extern int curr_x, curr_y, time, score;
extern char pacman_dir;	// u, d, r, l, n
extern char time_char[5], score_char[5];
extern uint8_t pause;

extern char ghost_dir;
extern uint8_t ghost_speedup;
extern void move_ghost();
extern int ghost_curr_x, ghost_curr_y;
uint8_t rit_count = 0;

extern uint8_t ghost_eaten;

void RIT_IRQHandler (void){
	
	/* Joystick handling */
	if(pause == 0){
			if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){	
				/* Joytick UP pressed */
				pacman_dir = 'u';
			}
			else if ((LPC_GPIO1->FIOPIN & (1<<28)) == 0){
					/* Joytick RIGHT pressed */
				pacman_dir = 'r';
			}
			else if ((LPC_GPIO1->FIOPIN & (1<<27)) == 0){
					/* Joytick LEFT pressed */
				pacman_dir = 'l';
			}
			else if ((LPC_GPIO1->FIOPIN & (1<<26)) == 0){
					/* Joytick DOWN pressed */
				pacman_dir = 'd';
			}
			
			
			/* Ghost movement */
			if(ghost_eaten==0){
				if (rit_count >= ghost_speedup){
					rit_count = 0;
					ghost_dir = compute_next_ghost_dir();
					move_ghost(ghost_dir, &ghost_curr_x, &ghost_curr_y);
				}		
				else{
					rit_count++;
				}
			}	
		}
	
		
	/* Button EINT0 */
		if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){//int0
						reset_RIT();
			}else{	// button released
						NVIC_EnableIRQ(EINT0_IRQn);
						LPC_PINCON->PINSEL4 |= (1 << 20);		//ext int selection
			}
	
	
	/* Background music handling */
	static int currentNote = 0;
  static int ticks = 0;
	if(!isNotePlaying())
	{
		++ticks;
		if(ticks == UPTICKS)
		{
			ticks = 0;
			playNote(song[currentNote++]);
		}
	}
	if(currentNote == (sizeof(song) / sizeof(song[0]))){//|| (LPC_TIM2->TC > 6000) ){		(sizeof(song) / sizeof(song[0])))
		currentNote=0;
	}
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
  return;
}



/******************************************************************************
**                            End Of File
******************************************************************************/
