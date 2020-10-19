# Capacitive Soil Moisture Sensor
First, for everybody who's also playing around with these soil moisture sensors and is also quite frustrated by results not being really reliable: I feel you. And I tried to address a few of them here. So maybe this repo is for you and will help you a little bit.

## Why is my solution so cool?
Because in my little circuit I'm using a transistir and an GPI of the ESP to turn on the sensor, when the ESP turns on. So wenn the ESP is sleeping, the sensor is off and doesn't consume any battery ;)

## Which kind of sensor exist / should I use?
### 1. Electrical conduction sensors (Don't use: Boooo!)
When I was first playing with these soil moisture sensors, I've started with these sensors, which have a (visible) copper/some metal layer and two legs: They measure the soil moisture based on the electrical conduction of the soil / between the two legs.
The downside of these is that they can corrode and actually harm the plants, instead of surveiling them ;) And I don't know what upsides they actually have compared to:

### 2. Capacitive Soil Moisture Sensor (Use: Coooool!)
You guessed it: These sensors work by measuring the capacitance of the sensor (or it's layers (the electrodes)), where the soil and water function as the dielectric. These do not have copper layers being exposed which could corrode, so make them safe to use in any plant.
If you **really** want to deep dive into how these sensors work and how to properly calibrate them, lead this publication: https://www.researchgate.net/publication/342751186_Capacitive_Soil_Moisture_Sensor_Theory_Calibration_and_Testing

## Components I used
* Capacitive Soil Moisture Sensor
* Bare / raw ESP-12F chip
* One LiFePO4 battery, providing nice 3.2V
* 2n2222a transistor to switch on the soil moisture sensor when the ESP turns on
* Some resistors

## Why a bare ESP-12F, why not using a devboard like D1 or NodeMCU?
Easy answer: Power consumption + deep sleep. All these dev boards have an USB UART that is **always on**, even in deep sleep. And this eats a lot of energy. There is a really good post about why you should use ESP-12F over these devboards: https://github.com/z2amiller/sensorboard/blob/master/PowerSaving.md
Also: There is a reason why they are called **dev**boards ;) They are not really meant to be used "in production."

## The circuit
https://github.com/gman-php/esp8266-sketches/blob/master/soil-moisture-sensor/circuit-diagram.png
As I know that obvious things don't neccessarily need to be obvious to everybody, I gonna walk you through what I'm doing here:
* The power source: The ESP-12 can only work with 3.3V, not more, maybe a little, little less. And a 3.3V power source is not easy to find, so most people will use step down converters to power an ESP-12. But the holy grail are LiFePO4 batteries: They come in AA size and provide a straight and good 3.2V to easily power an ESP-12.
  * From the batteries plus side, we will need to draw one line to the sensors VCC and two to the ESP:
    * One to VCC and
    * One to EN pin, as this is like the "on switch" of the ESP: It needs to be pulled high to turn on the ESP
  * GND needs to go to a few more places:
    * ESP GND, obviously
    * GPIO15 also needs to pulled low / to GND, as without it, the ESP would work in "SD card boot mode". "Nobody" wants/needs that, so pull this pin low.
    * To the transistors emitter
    * And from the transistors collector to the soil moisture sensor.
* From GPIO4 (I took GPIO4, you can choose whatever GPIO you want) through a 1k resistor to the transistors base.
* To be able to use timed(!) deep sleep, you need to connect RST (the reset pin) to GPIO16. If you do a timed deep sleep in code, the ESP will pull the GPIO16 to LOW for a brief moment whenever this time has passed. And this will then pull RST low, which will make the ESP restart.

Last piece is the sensors analog out:

### ESP-12 and analog signals
As you've already seen above, we need to wire a few more things when using a bare ESP-12, compared to just a NodeMCU or D1.
Another example is the ESPs analog pin, A0.
First: an analog signal is nothing else than a lower or higher voltage: Higher voltage, "higher" analog signal, lower voltage, "lower" analog signal.
The ESP can only work with / consume 0-1V on the analog pin. No worries: You won't roast the ESP with a signal higher than 1V (as long as it's lower than 3.3), but everything  above will result in "1024" when reading from this pin.
So I've measured the maximum voltage of the soil moisture sensor using a multimeter and it was around 2.2 - 2.3V (I actually forgot the exact number, sorry). So we need to "convert" it down and that's what I'm doing with the resistirs conencted to the analog out of the sensor: It's a simple voltage divider. This may not be the best solution. but it fulfills my needs: The sensors output is "percentually" stepped down, where 2.3V would result in 0.9xxV, so perfect :)

## The code
This is now the simple part: If you're familiar with ESP coding, this should all be straight forward, but some key things in there:
* The whole code is in setup() as the ESP goes through it when doing timed deep sleep using the GPIO16-to-RST method.
* I'm using the sensor in Home Assistant through MQTT, so the code has tome MQTT logic
* Both my Wi-Fi and MQTT connect routines have some reconnect logic.
* GPIO4 ("D2" in Arduino IDE) is used to switch on the sensor, so this is one of the very first things the code does. This is done by pulling it HIGH, which will switch the transistor which then make the full circuit of the sensor complete, by letting the electrones flow from GND to the sensor.
* I then just read from the A0 pin, convert it in C-super-complicated-style (Python developer speaking here!) to a char and dropping it into an MQTT topic.
* It also has some percentage calculation, which is based on my own measurements of the sensor, which is the "air value", so sensor just laying around and the "water value", which is the sensor put into a glas of clear water.
* Another thing is a "dry/wet threshold", which is based on my girlfriend: She said "Andy, this is now dry!". And I took this as the threshold (minus a little bit for earlier alerts) for consodering a plant being dry. Note that this may very much differentiate by plant and flower soil used.
* Ultimately not needed, but for easier checking if the battery is empty (=no signal is sent anymore): A "last updated" timestamp also dropped into the MQTT topic

## Possible improvements
### More reliable analog readings
To get a more reliable and less fluctuating signal, you can draw a 100nF capacitor between the sensors analog output and GND. I have not tried this yet, but will soon do and report my results 
