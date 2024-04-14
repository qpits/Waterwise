#ifndef SIMULATION_FUNCTIONS_H
#define SIMULATION_FUNCTIONS_H

#include <TM1637Display.h>

//helpful #define statements
#define BUTTON 18
#define DAC 25

#define LED_LEAK_1 16
#define LED_LEAK_2 17
#define LED_LEAK_3 13
#define LED_LEAK_4 14
#define LED_LEAK_5 27

#define LEAK_1 21
#define LEAK_2 3
#define LEAK_3 1
#define LEAK_4 22
#define LEAK_5 23

#define ROT_DT 39
#define ROT_CLK 36

#define LED_OK 26

#define DISP_CLK 32
#define DISP_DIO 33

//global variables
extern int pipe_damage;
extern TM1637Display display;
//functions
extern void setupPins();
extern void attachInterrupts();
extern void noiseGenerator(void *parameter);
#endif //SIMULATION_FUNCTIONS_H
