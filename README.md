# Circle Pong
Game implemented on two Arduino Megas

### Gameplay description
Much like the classic game pong, the player hits a moving ball with a paddle to prevent it from leaving the screen. However, this game has one player, and the paddle can wrap around the entire square borders. It's a one player endurance game, where you get score every time you make a save and keep the ball within the screen.

### Arduino setup instructions

#### Display Arduino

* Pins to game arduino: 5, 22 - 31, 33
* Pins to 8x8: 40 - 53

#### Game Arduino

* Pins to display arduino: 5, 22 - 31, 33
* Pins to reset button: 3
* Pins to LCD: 20, 21
* Pins to joystick: A1, A2
* Pins to MP3 player: 18, 19
* Pins to Buzzer: 9

### Build instructions

* Copy folders in /audio/ to an SD card to use with mp3 player
* Compile and send display.ino to the display Arduino
* Compile and send game.ino to the game Arduino
