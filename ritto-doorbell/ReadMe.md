# Ritto Twinbus Doorbell Sensor

## Disclaimer
I don't take credits for the idea of this solution, as I just followed the findings and solutions posted by following people, so they deservere the credits. I just wrote the code myself, based on their posts:
- Robin Henniges: https://robinhenniges.com/diy-smart-door-fuer-5-euro-door-bell-part-2/
- Peter Fickinger: https://www.nicht-trivial.de/index.php/2018/02/14/ritto-zu-mqtt/
- Thomas Abraham: http://www.deh0511.de/twinbus/

## How it works
The Ritto Twinbus doorbell has a connector for supporting external doorbells using their extension module "Ritto 1764600": https://eref.se.com/de/de/scope/product-pdf/1764600

We don't need this module, but we can use the connector and grab the signal whenever somebody rings the doorbell. If that happens, EXT will go high to ~5V for ~5 seconds and drop to 0V again after.
5V would be too much for any GPIO pin of the ESP8266 to handle and we would roast it. So we can either
1. Lower the 5V to ~3V using a few resistors put in a row, or
2. Simply use a transistor to bridge any GPIO (which we will set PULLUP in code before) to LOW / GND, so we don't have any hassle with caring about the voltage
I've also went with #2, like the people I linked in the disclaimer above. See the circuit diagram on how to wire it.

## Known issues & versions
### Version 1
I call this implementation "version 1", because I have a version 2 and 3 in mind which I couldn't quite solve yet; I can't get the circuit to work. Because this version has a known issue and it is called:

#### Known issue: Power consumption
In this implementation, the ESP is awake all the time and waits for somebody to ring the door. So 99,9% of the time it is consuming energy for doing nothing. And the ESP can be a hungry beast. I'm using a small powerbank currently with around 1500mA and it drains it in ~13 hours.

So the solutions can be version 2 & 3:
