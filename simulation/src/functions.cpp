#include "functions.h"
#include <Arduino.h>
#include <TM1637Display.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//useful variables for the rotary encoder
int currentStateCLK;
int lastStateCLK;
TM1637Display display(DISP_CLK, DISP_DIO);
//from 0 to 100, where 0 is best and 100 is worst
int pipe_damage;
int volatile debouncerMillis = millis();
int buttonState = 0; //0 --> ok, 1..5 --> leak
bool volatile noiseOn = false;
uint64_t micro = micros();


void setupPins(){
    //display pins
    pinMode(DISP_CLK, OUTPUT);
    pinMode(DISP_DIO,OUTPUT);

    //rotary encoder pins
    pinMode(ROT_CLK, INPUT);
    pinMode(ROT_DT,INPUT);

    //led pins
    pinMode(LED_LEAK_1, OUTPUT);
    pinMode(LED_LEAK_2, OUTPUT);
    pinMode(LED_LEAK_3, OUTPUT);
    pinMode(LED_LEAK_4, OUTPUT);
    pinMode(LED_LEAK_5, OUTPUT);
    pinMode(LED_OK,OUTPUT);

    //dac pin
    pinMode(DAC, OUTPUT);

    //BJT base pins
    pinMode(LEAK_1, OUTPUT);
    pinMode(LEAK_2, OUTPUT);
    pinMode(LEAK_3, OUTPUT);
    pinMode(LEAK_4, OUTPUT);
    pinMode(LEAK_5, OUTPUT);
}

void allPinsOff(){
    digitalWrite(LED_LEAK_1, LOW);
    digitalWrite(LED_LEAK_2, LOW);
    digitalWrite(LED_LEAK_3, LOW);
    digitalWrite(LED_LEAK_4, LOW);
    digitalWrite(LED_LEAK_5, LOW);
    digitalWrite(LED_OK,LOW);

    digitalWrite(LEAK_1, LOW);
    digitalWrite(LEAK_2, LOW);
    digitalWrite(LEAK_3, LOW);
    digitalWrite(LEAK_4, LOW);
    digitalWrite(LEAK_5, LOW);
}

void buttonHandler(){
    if(millis()-debouncerMillis<300){
        return;
    }
    debouncerMillis = millis();
    allPinsOff();
    if(buttonState==5){
        buttonState = 0;
    }
    else{
        buttonState++;
    };
    if(buttonState == 0){
        digitalWrite(LED_OK,HIGH);
        digitalWrite(LEAK_1,HIGH);
        noiseOn = false;
        return;
    }
    if(buttonState == 1){
        digitalWrite(LED_LEAK_1, HIGH);
        digitalWrite(LEAK_1,HIGH);
        noiseOn = true;
        return;
    }
    if(buttonState == 2){
        digitalWrite(LED_LEAK_2, HIGH);
        digitalWrite(LEAK_2,HIGH);
        noiseOn = true;
        return;
    }
    if(buttonState == 3){
        digitalWrite(LED_LEAK_3, HIGH);
        digitalWrite(LEAK_3,HIGH);
        noiseOn = true;
        return;
    }
    if(buttonState == 4){
        digitalWrite(LED_LEAK_4, HIGH);
        digitalWrite(LEAK_4,HIGH);
        noiseOn = true;
        return;
    }
    if(buttonState == 5){
        digitalWrite(LED_LEAK_5, HIGH);
        digitalWrite(LEAK_5,HIGH);
        noiseOn = true;
        return;
    }
}

void updateEncoder(){

    currentStateCLK = digitalRead(ROT_CLK);

    // If last and current state of CLK are different, then pulse occurred
    // React to only 1 state change to avoid double count
    if (digitalRead(ROT_DT) != currentStateCLK) {
        if(pipe_damage>0){
            pipe_damage = pipe_damage - 5;
            display.showNumberDec(pipe_damage);
        }
    } else {
        // Encoder is rotating CW so increment
        if(pipe_damage<100){
            pipe_damage = pipe_damage + 5;
            display.showNumberDec(pipe_damage);
        }
    }
    // Remember last CLK state
    lastStateCLK = currentStateCLK;


}

void attachInterrupts(){
    attachInterrupt(ROT_DT, updateEncoder, CHANGE);
    attachInterrupt(BUTTON,buttonHandler,RISING);
}

int map(int value){
    if(value < 20 && value >= 0){
        return value * 255 / 100;
    }
    if(value < 70 && value >= 20){
        return (20 * 255 / 100) + (value - 20);
    }
    if(value < 100 && value >= 70){
        return (20 * 255 / 100) + (70 - 20) + (value - 70)*1.5;
    }
    if(value == 100){
        return 255;
    }
    else{
        return 255;
    }

}

void noiseGenerator(void *parameter){
    randomSeed(analogRead(0));
    while(true){
        if(micros()-micro > 500||micros()-micro < 0){
            micro = micros();
            if(noiseOn){
                int randomNumber = random(0,2);
                if(randomNumber == 0){
                    dacWrite(DAC,0);
                }
                if(randomNumber == 1){
                    dacWrite(DAC,map(pipe_damage));
                }
            }
            else{
                dacWrite(DAC,0);
            }

        }
    }
}