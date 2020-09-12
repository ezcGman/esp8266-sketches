# Ritto Twinbus Doorbell Sensor

## Disclaimer
I don't take credits for the idea of this solution, as I just followed the findings and solutions posted by following people, so they deservere the credits. I just wrote the code myself, based on their posts (all in German):
- Robin Henniges: https://robinhenniges.com/diy-smart-door-fuer-5-euro-door-bell-part-2/
- Peter Fickinger: https://www.nicht-trivial.de/index.php/2018/02/14/ritto-zu-mqtt/
- Thomas Abraham: http://www.deh0511.de/twinbus/

## How it works
The Ritto Twinbus doorbell has a connector for supporting external doorbells using their extension module "Ritto 1764600": https://eref.se.com/de/de/scope/product-pdf/1764600

We don't need this module, but we can use the connector pins and grab the signal whenever somebody rings the doorbell by simply soldering two wires to EXT and GND.
Whenever somebodz rings, EXT will go high to ~5V (I actually measured 4,937V@0,023A) for ~5 seconds and drop to 0V again after. 5V would be too much for any GPIO pin of the ESP8266 to handle and we would roast it. So we can either:
1. Lower the 5V to ~3V using a few resistors put in a row, or
2. Simply use a transistor to bridge any GPIO (which we will set PULLUP in code before) to LOW / GND, so we don't have any hassle with caring about the voltage
I've also went with #2, like the people I linked in the disclaimer above. See the circuit diagram on how to wire it.

So whenever EXT goes to 5V, the transistor will switch and pull GPIO4 (note that GPIO4 is labelled "D2" on the NodeMCU boards) to GND. I react on this on code which will then send a MQTT message to my Home Assistant (HA). The the "sensor.yaml" and "automations.yaml" for the HA configuration.
*Note: You will need the Mosquitto broker add-on in HA to make this work (or have any other MQTT broker in your home network)*

## Known issues & versions
### Version 1
I call this implementation "version 1", because I have a version 2 and 3 in mind which I couldn't quite solve yet; I can't get the circuit to work. Because this version has a known issue and it is called:

#### Known issue: Power consumption
In this implementation, the ESP is awake all the time and waits for somebody to ring the door. So 99,9% of the time it is consuming energy for doing nothing. And the ESP can be a hungry beast. I'm using a small powerbank currently with around 1500mA and it drains it in ~13 hours.

So the solutions can be version (or better ideas) 2, 3, 4 and/or 5:

### Version 2
The ESP can save power by sending it into various sleep modes. In deep sleep it will consume only around 20µa, so a battery back would last for months! But if we send it to deep sleep, we have two issues to solve:
1. We need to wake it up. We can wake it up by either:
    1. Pulling the RST pin LOW / to GND and release it again (the ESP will only wake up after releasing the pin!)
    2. Pulling the EN pin HIGH and release it again (the ESP will only wake up after releasing the pin!)
2. We need to dramatically increase the setup() routine time, as connecting to Wi-Fi takes around 3 seconds, then MQTT, then sending the message to HA, then have HA triggering automations... All of this takes time and for opening the door we shouldn't get the notification 5+ seconds after somebody rang

While I have solved #2 and could lower the setup() routine to ~500-800ms, I could (not yet) solve #1. I am still working on it...

### Version 3
Basically what's described in Peter Fickinger's post I linked above:

The Ritto Twinbus also provides a constant 24V on a PIN we could grab and use anz step down converter to lower the 24V to 3V3 - 5V and connect it to the ESPs VIN. However, when I did this, I had pretty bad distortion noises in the phone, so I dropped this solution again.
I want to try it again with a (Schottky) diode connected to the 24V wire, as I think what happens is that the step down converter pushes some distortion over the 24V back to the Ritto. Will post my results when I try it

### Version 4
Grabbing a 230V and step it down to 5V.

There are also small and nice step down converters available that can give you 5V out of a usual 230V power socket. I have not tried that yet, as I have no power socket nearby. But it's a solution.

### Version 5
Using a reed switch / window/door sensor!

These little window/door sensors are awesome for "hacking" and use them for other usecases where you have a binary signal / state. E.g., I have built a rain sensor out of a Xiaomi door/window sensor based on this guide: https://community.home-assistant.io/t/portable-rain-drop-sensor-made-with-xiaomi-door-sensor/74090
And I think it should be possible to do the same with the doorbell. But I just had the idea a few hours ago and didn't yet think it through on how to do it.
