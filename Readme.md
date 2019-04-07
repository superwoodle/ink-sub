# Ink Sub

> Built at HatterHacks2019

YouTube Demo:
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/jSxVXv6bG_E/0.jpg)](https://www.youtube.com/watch?v=jSxVXv6bG_E)

It's an E-Ink Youtube stats counter display, running an Adafruit Feather Huzzah.

The ESP8826 makes an HTTPS request to google apis to pull youtube channel meta
How I built it

Adafruit Feather Huzzah + Adafruit 2.13" Tri-Color eInk / ePaper Display FeatherWing + Lipo battery 

# References

### Youtube API
https://stackoverflow.com/questions/30723866/youtube-subscriber-count-with-youtube-data-api-v3
https://developers.google.com/youtube/v3/getting-started

#### API Calls:
Get channel id from username:
https://www.googleapis.com/youtube/v3/channels?key=<API_KEY>&forUsername=<CHANNEL_USERNAME>&part=id

Get channel stats by id:
https://www.googleapis.com/youtube/v3/channels?part=statistics&id=<CHANNEL_ID>&key=<API_KEY>


### ESP8826 TLS
https://www.youtube.com/watch?v=Wm1xKj4bKsY
https://github.com/SensorsIot/HTTPS-for-Makers/blob/master/CertToESP8266.py

### Adafruit

EPD Samples

# Tools

https://arduinojson.org/v6/example/parser/
https://arduinojson.org/v6/assistant/
