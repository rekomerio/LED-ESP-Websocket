# LED-ESP-Websocket
Control RGB LED strip from your browser with fast response time. Currently websocket sends new color update every 50ms,
if the color changes, but this value can be decreased even more, if necessary. 

You will need to build your own circuit to drive the strip.
Parts needed: 
* Wemos Mini D1 or alternative
* 3x Transistor or MOSFET's to suit your needs (I use MPS 13, but I dont recommend them as I already burnt one set)
* 3x 220ohm resistor (depends on the transistors you use and the power consumption of your strip)
* RGB LED 
### UI
![](https://raw.githubusercontent.com/K9260/LED-ESP-Websocket/master/interface.jpg)
### YouTube
[![](http://img.youtube.com/vi/FgJDixEFlrs/0.jpg)](http://www.youtube.com/watch?v=FgJDixEFlrs "Youtube")
