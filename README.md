# Arduino-Tetris
Tetris for a 16x32 LED matrix, coded using C.

Excuse us in advance for the un-cleanliness of the code, alot of readability was sacrificed in exchange to make the game fit on the limited 32k flash memory of the LED controler Arduino.

The game is played on 2 Arduino UNOs. One of the Arduino is used for all the I/O tasks (including the joystick and the LCD panel) while the other UNO is used to control all the LED lights of the Matrix. The tasks are shared between the two Arduinos using the IC2 communication protocol. Number generation (and consequently piece generation) is distributed using a digital tempature sensor.

==Physical hardware needed======

<ul>
<li>1x 16x32 LED matrix [http://www.adafruit.com/product/420]</li>
<li>2x Arduino (We used 2 Arduino UNO)</li>
<li>1x Analog Joystick [http://www.adafruit.com/product/512]</li>
<li>1x Digital Tempature sensor [https://www.sparkfun.com/products/245]</li>
<li>+30 Male/Male Jumper cables [http://www.adafruit.com/products/1956]</li>
<li>1x LCD panel </li>
</ul>

==Useful resources==============

<ul>
<li>LCD pin setup : http://www.seeedstudio.com/wiki/images/a/a9/Electronic_Bricks_Starter_Kit_Cookbook_v1.3.pdf</li>
<li>LED wiring: https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/</li>
</ul>

==External Librairies===========

<ul>
<li>Adafruit_GFX.h :  https://github.com/adafruit/Adafruit-GFX-Library</li>
<li>RGBmatrixPanel.h : https://github.com/adafruit/RGB-matrix-Panel</li>
<li>OneWire.h : http://playground.arduino.cc/Learning/OneWire</li>
</ul>
