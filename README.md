# Circle Pong
Game implemented on two Arduino Megas

### Gameplay Description
Much like the classic game pong, the player hits a moving ball with a paddle to prevent it from leaving the screen. However, this game has one player, and the paddle can wrap around the entire square borders. It's a one player endurance game, where you get score every time you make a save and keep the ball within the screen.

### Design & Implementation

#### Components:
* A start/reset pushbutton button is available to begin the game for the first time, restart the game when the game has finished, and end it while the game is still running.
* A sound mode pushbutton changes between different sound volumes and modes.
* A joystick is used to control the paddle's position, setting the paddle position to the joystick's angle. Whenever the joystick is in a neutral position (determined by a square margin on both the x & y axis), the most recent angle it formed is what gets used. As well, the joystick switch can restart the game, but only when a game is not currently active.
* An LCD display tells the player their highscore and their current score while playing, or prompts them to press the pushbutton to begin when they're not.
* A passive buzzer and an MP3 player play sound effects. The MP3 player plays longer sound effects when time is not a problem (due to the long delays it requires), while the passive buzzer plays short tones for sound effects that require as minimal delaying as possible (such as when the ball hits the paddle).
* The 8x8 LED grid displays the current state of the game. While the game is being played, it displays the paddle's position and the ball's current position.

#### Arduinos:
  The display Arduino handles displaying the game state to the 8x8 LED grid. By having this on its own arduino, a more consistent lighting can be achieved (because displaying a paddle + ball requires rapidly alternating between 3 different configurations -- the horizontal border, the vertical borders, and the ball position). Whenever the data pin turns high, this arduino reads the interrupt which triggers it to receive the position of the paddle (expressed as a binary number 0-27, requiring 5 pins) and the x and y coordinates of the ball (each expressed as a binary number 0-8, requiring 3 pins each).
  
  If debugging is enabled within game.ino, the serial monitor can emulate the display arduino.
  
  The game arduino handles driving the game, as well as receiving input from the start/reset button and joystick, and outputting to the LCD display, passive buzzer, and MP3 player. When the game is running, it will move the ball in one of 8 directions. When the ball hits the border, it will check if it's adjacent to the paddle. If so, the ball will move in a randomized direction away from where it hit, and if not the game is lost.


### Arduino setup instructions

#### Parts Required
1. 2 Arduino Megas
2. 1 8x8 LED Grid
3. 1 16x2 LCD Display
4. 1 Joystick
5. 2 Pushbuttons
6. 1 Passive Buzzer
7. 1 MP3 Player with SD card support

#### Display Arduino

* Pins to game arduino: 5 (interrupt pin), 22 - 31, 33
* Pins to 8x8: 40 - 53

#### Game Arduino

* Pins to display arduino: 5 (interrupt pin), 22 - 31, 33
* Pins to reset button: 3
* Pins to LCD: 20, 21
* Pins to joystick: A1, A2
* Pins to MP3 player: 18, 19
* Pins to Buzzer: 9

### Build instructions

* Copy folders in /audio/ to an SD card to use with mp3 player
* Wire as per diagrams
* Compile and send display.ino to the display Arduino
* Compile and send game.ino to the game Arduino
