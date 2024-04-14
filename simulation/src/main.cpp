#include <Arduino.h>
#include <TM1637Display.h>
#include "functions.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//setup of the 4x7 segments display


void setup() {

    //setup of all the pins
    setupPins();

    //attaching all the interrupts
    attachInterrupts();

    //setup of some important elements
    pipe_damage=100;
    display.setBrightness(10);
    display.showNumberDec(pipe_damage);
    digitalWrite(LED_OK,HIGH);


    xTaskCreatePinnedToCore(
            noiseGenerator,  // Function to be called
            "IdleTask",              // Name of the task (optional)
            2048,                    // Stack size (bytes)
            NULL,                    // Parameter to pass to the function (in this case, none)
            1,                       // Task priority (1 is the lowest priority)
            NULL,                    // Task handle (optional)
            1                        // Core to run the task (0 or 1, 0 is the default core)
    );


}

void loop() {

}

