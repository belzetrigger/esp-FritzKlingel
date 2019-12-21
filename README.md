# Fritz Klingel / Fritz Doorbell
Just turn an ESP into a dummy phone and let it forward all rings from the door to your phone. So you get notices also outside in the yard. 

## Summary
Small project to make your Wemos D1 or ESP interact as "virtual phone" and calls the Fritz!Box, if someone is ringing the bell. In this situation ESP wakes up from sleep and does its work. Afterwards going back to sleep again.
Calling works only with AVM Fritz!Box Router. But you can also turn off this and just use MQTT.

## Features
|Feature  | |
|---------|-------------------------------------------------------------------|
|Tr064    | Using protocol tr064 to make a phone call from esp via Fritz!Box. Works good wiht **9 or all internal numbers. For outgoing calls, have a longer hangup period. |
|MQTT     | Sending simple text message via mqtt                              |
|Domoticz | sending status of Wifi and battery to Domoticz               |


## Background
A few years ago I was reading the article 'Überallklingel' ( c't 17/2017) about using a Fritz!Box with a Raspberry Pi to forward the doorbell ring also to your phones. Cool idea - being in the garden and hear the doorbell, but the Pi looked a bit too much for doing this. So I tried similar with a ESP and came across github from [tIsGoud](https://github.com/tIsGoud/Doorbell-via-Wemos-and-optocoupler). So I took some information from here and there and started my own project.
For the sake of completeness the c't has now also a version of this article directly for Wemos / ESP  'ESP-Überallklingel' (c't 17/2018).


## Similar projects and links
* https://github.com/tIsGoud/Doorbell-via-Wemos-and-optocoupler
* Überallklingel c't magazine 2017/17 https://www.heise.de/select/ct/2017/17/1502995489716437
* ESP-Überallklingel c't magazine 2018/17 https://www.heise.de/select/ct/2018/17/1534215254552977 


## Install and Setup

### Prepare Fritzbox
- setup your Fritz!Box for TR064
  - enable TR064: Heimnetz > Netzwerk > Netzwerkeinstellungen: enable Heimnetzfreigabe: Zugriff für Anwendung zulassen
  - create a user: System > Fritz!Box Benutzer > Benutzer: Benutzer hinzufügen
    - set password
    - assign rights to this user: 
  - more under: https://avm.de/service/schnittstellen/ 
- enable Wemos as dummy phone
  -  Telefonie > Telefoniegeräte: Neues Gerät einrichten
     -  1. Type: Telefon mit AB
     -  2. choose a available connection and 3. type a name
     -  4. just accept the number for outgoing calls
     -  5. just accept MSN handling
     -  say yes, that Test was successfully and phone was ringing
     -  get the internal phone numnber eg **23
-  enable calling assistance / Wahlhilfe
### Mobile Phone
Just install Fritz!Box Phone App on your smartphone. Check the **xxx number this device gets from your Fritz!Box and you can just forward the door bells to this phone.

### Soldering
#### Simple version
* just use an pc817 or similar photo-coupler  / opto-coupler to separate Wemos and bell. But be able to get Wemos back from sleep 
* use 1N 4148 Switching Diodes between 1 and 2 of the PC817  
* adjust R1 based on bell transformer 
  * 12v 560Ω 
  * 8v: 330Ω
  

```
           _/          ___
       .-o/  o--o-----|R1_|----.      .-------.
       |        |              |      |PC 817 |
       |        |              o---o----1   4----o RST
  -. ,-         |  .---|       |      |       |
   )|(          '--|   |       -      |       |
   )|(           .-|   |       ^1N4148|       |
  -' '-.         | '---|       o---o----2   3----o---.
       '---------o-------------'      |       |      |
                                      '-------'      |
                                                    ===
                                                    GND
```
(created by AACircuit v1.28.7 beta 10/23/16 www.tech-chat.de)

This one is easy to solder and test. Unfortunately Wemos only wakes up on release of the button.

#### a bit more
to wake up directly on button press, you could try it with a capacitor like the following. 
```

                                                          .-----------o RST
                                                          |
                                                          |
                                                          '-----------.
                                                          |           |
                                                          |          .-.
                                                         ---C1       |R|
           _/          ___                               ---         |3|
       .-o/  o--o-----|R1_|----.      .-------.           |          '-'
       |        |              |      |PC 817 |           |      ___  |
       |        |              o---o----1   4----o--------o-----|R2_|-o  3.3V
  -. ,-         |  .---|       |      |       |
   )|(          '--|   |       -      |       |
   )|(           .-|   |       ^1N4148|       |
  -' '-.         | '---|       o---o----2   3----o----------.
       '---------o-------------'      |       |             |
                                      '-------'             |
                                                           ===
                                                           GND

```
(created by AACircuit v1.28.7 beta 10/23/16 www.tech-chat.de)
* R1 = see above
* R2 = 10k Ω
* R3 = 10k Ω
* C1 = 1µF


### Install, Build and Upload
I work with VS Code and PlatformIO, so if you use other tools adapt this according to your tool chain. 
As I was working lately with ESPEasy and Tasmota I took some of their ideas. E.g. extra config headers and build parameters to keep the source code free from private or sensitive data. 

* get sources from github
* adapt platformio.ini
  * set build flags starting form line 18.
    ```
    -DUSE_CONFIG_OVERRIDE 
    -DMY_IP="\"{IP Address of the device itself }"\"
    -DMY_GW="\"{IP Address of your gateway}"\"
    -DMY_DNS="\"{IP Address of your DNS}"\"
    -DMY_FB_NR="\"{number that should be called **9 -> all, or just an individual number}"\"
    -DMY_DOM_IDX={if using Domoticz, here u can add the Index of domoticz device}
    ```
* copy user_config_override_sample.h to  user_config_override.h
  * adapt to your needs
  * at least
    * set wifi data
    * add TR064 data

* if running on another kind of battery adapt it
  * if you use a TP4056 or similar take the under voltage protection into account 
  * VMIN: min value from the battery, do not go under 2.4V.  ESP want work anymore, so 2.4 V is like dead
  * VMAX: max voltage of the battery.

## Bugs and To-Do
* clean up and documentation
* play with internal LED to show status, easier for debugging 
* better drawing of wiring
* turn door bell off and just use tr064 - kind of silent bell
* maybe version 2: check how long Tasmota / EspEasy would need to come back from sleep and simple plugin to do the calling stuff


## State 
Works, but will be add some more functions.
No Warranty or anything like that. Use on own risk.
