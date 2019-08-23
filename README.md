# HITCON Badge 2019
## Overview
HITCON Badge 2019 is based on `M2351ZIAAE` MCU. 
## Usage
There are four pages, you can use `LEFT` and `RIGHT` button on the badge to switch them.
### Page 0: LED Level Display
* Badge will show the status of 24 LEDs.
    * Locked: white color
    * Unlocked: Bright and colorful
* Press `UP` + `A`/`B` to decrease/increase the brightness.
* You can unlock each LED by playing the game or completing the mission with sponsors.
### Page 1: Pattern Display
* Use `UP` and `DOWN` button to select patterns.
* Pattern 0 will be unlocked by unlocking led 0 1 2.
    * Pattern 1: led 3 4 5.
    * Pattern 2: led 6 7 8.
    * ..
* If the pattern is still locked, it will render the error pattern (all LED are red).
* There are 11 patterns in total.
### Page 2: Paint Mode
* In this page, you can customize the color of LEDs!
* Select the LEDs by pressing `UP` `DOWN` `LEFT` `RIGHT`.
* Change the color by pressing `A` `B`.
* Press `AB` (at the same time) and then press `LEFT`/`RIGHT` to leave paint mode.
### Page 3: Snake
```
################################
#                              #
#                              #                                   
#                              #                                   
#                              #                                   
#                              #                                   
#                              #                                   
#                              #                                   
#      @@@@@@@          o      #                                   
#                              #                                   
#                              #                                   
#                              #                                   
#                              #                                   
#                              #
#                              #
#                              # 
#                              #
################################
[Score] 6 pt
```
* Let badge connect to the computor with Micro USB cable.
* Use any client you like to connect the serial com port.
    * Linux/macOS: You can use `screen` command.
        * example: `screen /dev/tty.usbmodemxxx` (replace `/dev/tty.usbmodemxxx` with correct path)
    * Windows: You can use `PuTTY` (Connection type: Serial) to connect the COM port.
* Control the snake by prssing `UP` `DOWN` `LEFT` `RIGHT`.
* Press `AB` at the same time to pause the game.
    * Press `AB` again to continue the game, `LEFT` to exit.
* Score
    * score >= 50, snake pattern (pattern 8) will unlocked!
    * score == 2147483647: Well done, hacker :)
## Badge Command Line
```
  _  _ ___ _____ ___ ___  _  _   ___ __  _ ___
 | || |_ _|_   _/ __/ _ \| \| | |_  )  \/ / _ \
 | __ || |  | || (_| (_) | .` |  / / () | \_, /
 |_||_|___| |_| \___\___/|_|\_| /___\__/|_|/_/


HitconBadge2019 >>
```
* There is a simple command line interface running on the badge.
* You can use it by connecting the badge to computer with micro usb.
### Commands
#### help
Type help for all available commands.
```
HitconBadge2019 >> help
show
info
unlock
setname
clear
hello
angelboy
yuawn
ping
ls
id
cat
echo
alias
whoami
help
```
#### show
Show command will display the status of all LEDs and patterns.
```
HitconBadge2019 >> show
Pattern 0: Lock
  led 00: Lock
  led 01: Lock
  led 02: Lock
Pattern 1: Lock
  led 03: Lock
  led 04: Lock
  led 05: Lock
Pattern 2: Lock
  led 06: Lock
  led 07: Lock
  led 08: Lock
Pattern 3: Lock
  led 09: Lock
  led 10: Lock
  led 11: Lock
Pattern 4: Lock
  led 12: Lock
  led 13: Lock
  led 14: Lock
Pattern 5: Lock
  led 15: Lock
  led 16: Lock
  led 17: Lock
Pattern 6: Lock
  led 18: Lock
  led 19: Lock
  led 20: Lock
Pattern 7: Lock
  led 21: Lock
  led 22: Lock
  led 23: Lock

Badge challenge:
[Stage 1] Snake pattern: Lock
[Stage 2] Pwned NS pattern: Lock
[Stage 3] Pwned the whole badge pattern: Lock
```
## HITCON Badge Challenges
> Badge source code, solution and exploits will be released within the talk `HITCON Badge 2019 秘辛: MCU ARM TrustZone challenges` at R0 (Day2 14:40 - 15:30).

There are 11 pattern in total, but three of them are special, so you need to get them in special way :)
### Warm Up
* Before pwning the badge, why not play some game first.
* Get the score higher than 50, you can unlock the snake pattern.
### Pwn the Badge
* Try to pwn the badge for all patterns!
