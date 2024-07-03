# A Game Bundle: Two Game Package

**Daniel Ledesma and Eitan Sahnovich**

*EEC172 SQ24*

## Description

The game bundle is a device that contains two pre-loaded games and a menu interface. The CC3200 is used as the general processor of this device to interpret the multiple forms of inputs from the user. AWS IoT was used to create a device shadow to store the high scores. The menu is interfaced using an IR remote to make decisions across the various menu screens. Pressing two is used to move the cursor up with five to move the cursor down. Pressing mute is the equivalent of an enter key and is used to make the decision once the cursor was properly navigated. The maze game if selected will give the open to choose an easy, medium, or hard maze. After the user has selected their difficulty the maze is loaded and the user can attempt to navigate the maze by tilting the board in the appropriate direction. The score is determined by subtracting the number of collisions from 800. If the user reaches the end zone they receive a win screen with their score. If this is a highscore then the shadow is updated with this score. The user loses if the amount of collisions surpasses 800 and is given a losing screen. The user is then navigated back to the menu screen. If navigating to the high score screen then the user will be shown the high score for each game. If navigating to the hangman game, the hangman game is loaded with the choice of 5 different words labeled as numbers for the user to guess in the game. The max word is 5 letters. The score is determined by counting the length of the word and assigning a max score for the longest word of 5 letters at 800 and 100 points less for each letter shorter than 5. If the stickman is drawn then the user loses with a score of 0 and is shown the lose screen. If the user wins they are shown the win screen with their score and if it is a highscore, the shadow is updated.

Our source code can be found [here](https://github.com/dledesma2000/GameBundle_EEC172Final.git).

Base Menu interfaces and the maze game implemented by Daniel Ledesma

Hangman implemented by Eitan Sahnovich
## Game Screens

![Main Menu](./media/main_menu.jpg)
*Main Menu*

![Maze Difficulty](./media/maze_difficulty.jpg)
*Maze Difficulty*

![Easy Maze](./media/easyMaze.jpg)
*Easy Maze*

![Medium Maze](./media/mediumMaze.jpg)
*Medium Maze*

![Hard Maze](./media/hardMaze.jpg)
*Hard Maze*

![Win/Lose Screen](./media/WinLoseScreen.jpg)
*Win/Lose Screen*

![Hangman](./media/hangman.jpg)
*Hangman*

## Video Demo

[![Alt text](https://img.youtube.com/vi/_6uB2nkWJQg/0.jpg)](https://www.youtube.com/watch?v=_6uB2nkWJQg)

  ## Table of Contents
  - [Similar Products](#similar-products)
  - [Design](#design)
    - [System Architecture](#system-architecture)
    - [Functional Specification](#functional-specification)
  - [Implementation](#implementation)
    - [IR Interrupt Handling](#ir-interrupt-handling)
    - [Maze](#maze)
    - [State Machine](#state-machine)
    - [AWS](#aws)
    - [Hangman](#hangman)
  - [Challenges](#challenges)
  - [Future Work](#future-work)
  - [Finalized BOM](#finalized-bom)

## Similar Products

### [VCS All-In Bundle](https://atari.com/products/atari-vcs-onyx-all-in)

![VCS All-In Bundle Image](./media/vcs.jfif)

- 100 Atari Games preloaded
- Controlled by controller or joystick

### [Sega Genesis Classic Game Console](https://www.amazon.com/Sega-Genesis-Classic-Game-Console-Gear/dp/B06XWBY3YP)

![Sega Genesis Classic Game Console Image](./media/sega.jpg)

- 81 classic Sega games preloaded
- Controlled by controllers

## Design

### System Architecture

![Hardware Diagram](./media/hardware_diagram.jpg)
*Hardware Diagram*

![Game Schematic](./media/GameSchematic.jpg)
*Game Schematic*

The CC3200 serves as the main controller for the implementation of this code. There are two sensors involved in this implementation: the accelerometer and the IR circuit. The accelerometer is communicated with using the I2C protocol. The IR circuit communicates with the NEC protocol, and the OLED is communicated with using the SPI protocol. The accelerometer is used in the maze game. The output from the accelerometer is then fed into the CC3200 to determine the orientation of the board and the new location of the next pixel.

The IR Circuit is the second sensor to detect input from the user’s IR remote. This circuit will receive the IR wave and cause an interrupt in the CC3200 to allow for the data stream to be captured by the CC3200. Depending on the state and the data stream, the CC3200 then decides the outcome of the current state or game. The CC3200 then uses the information of the current state and the input from the IR or the accelerometer to then update the OLED with the current state information. AWS is used upon initialization to get the high scores and is also called whenever there is a new high score or the high score screen is selected.

### Functional Specification

![State Diagram](./media/state_diagram.jpg)
*State Diagram*

The device has the states main_menu, maze_difficulty_screen, highscore_state, maze_state, hangman_screen, hangman_state, win_lose_screen, and cursor_state. Upon loading the game, the main_menu state is the beginning state. In this state, the menu screen is printed and the cursor variables are initialized. Then the state switches to the cursor state. This state will update the cursor location and switch the states depending on the input from the IR remote. If the maze selection is made on the first menu screen, then the state switches to maze_difficulty_screen. This state will display the selection screen and update the cursor variables, then switch to the cursor state. This state will update the cursor location and switch the states depending on the input from the IR remote.

From this state, once the difficulty is selected, the state switches to maze_state. In this state, depending on the difficulty selected, the appropriate maze is created and loaded into the maze game. The device will stay in this state until the user has navigated to the end location of the map or has pressed 1 on the IR remote. Reaching the end point will load the winning screen. Pressing 1 will quit the game in a loss and load the losing screen. Both of these screens are in the win_lose_state and once they are printed, then the device enters the cursor state and waits for the user's input. Once the user presses mute, the screen is exited and the main screen is loaded again, beginning the process.

If from the main menu screen the high score screen is selected, then the state will switch to highscore_state and use AWS to grab the high score and display it along with the headers. After this is displayed, it then enters the cursor state and waits for the user's input. Once the user clicks mute, then the state switches to the main menu state and begins


## Implementation

### IR Interrupt Handling
This function was initialized to be triggered on falling edges and was tied to pin 50. This function uses NEC protocol to decipher the IR wave. This uses the systick module to determine the wavelength of each pulse. The checks are if the wavelength is 13.5ms which indicates the beginning of the data stream. This sets our data stream to 0 and our bit counter to 0. Then it checks if the wavelength is less than 1.2ms to indicate a 0 being transmitted or if it is larger than 1.2ms and less than 2.7ms then a 1 was transmitted. Both of these checks increment the bit counter variable. This continues until the bit counter is 32 which sets the checkFlag to 1 indicating that this data stream is completed and ready to be checked.

### Maze
The implementation of the maze involved maze creation and maze navigation. Maze creation was done by creating a function called mazeLine that uses Bresenham's algorithm to draw in a straight line given a start and end set of coordinates. This was done to place x’s in an array to represent walls in the maze. To create each maze, it was drawn by hand then the coordinates of the drawing were translated into the drawLine function to assemble buildXmaze() function where x represents the difficulty. A generic maze is passed into the chosen function and the appropriate difficulty maze is populated into the array. The maze is then drawn by iterating through the array and drawing a white pixel wherever in the array an x is found.

Maze navigation involved use of the function calculateShift from Lab 2. This function intakes an angle reading and a coordinate of 1 axis, checks that it is within -90<x<90 and then uses a linear relationship of .2 pixels per angle to calculate the shift in the position of the given pixel. The main functionality of the maze game is in a function called mazeGame(). The way this function works is that it is given the starting and end coordinates of the given maze, as well as the array that contains the selected maze difficulty. Then when entering the function the start and end points are drawn in green and red, respectively. Next collisions are initialized as zero and the user circle is drawn at the starting location. With initialization done, the function then enters a forever while loop. Inside the loop the function first gets the orientation of the accelerometer in the x and y direction and stores those values into roll and pitch, respectively. This is done using a function called getAccleration which reads from the corresponding axis register and converts output into an integer before returning the orientation of the tilt for each axis. The current x and y coordinates are fed into variables oldX and oldY. The acceleration is then passed into calculateShift with the current coordinates of the ball. The output from this function is then fed into variables called newX and newY. Then a check is run to ensure that the new position is a valid location. A thorough explanation of this check can be found in the challenges section under maze movement. If the position is a valid position then the newX and newY values are placed in the current x and y variables. If it is not a valid position then the collisions variable is incremented. Next the old location is drawn over in black and the new location is drawn in the ball color. Finally, a check of our new location is done to check whether our current location is close enough to the end location of the maze. This is done by calculating the hypotenuse that is created between the new location and the end location. If this distance is shorter than our ball radius plus the goal radius then we are within the goal and should be awarded with the win. Otherwise the loop repeats again. In a win, the score is calculated by subtracting the collisions away from the max score, the game result is set to a win, the state is set to the win lose screen, and finally the loop is broken. At the beginning of the loop it is also checking if a one is pressed on the IR remote which indicates the user wishes to quit. If this happens the game is set to a loss, score is set to 0, the state is changed to the win lose screen state and the loop is broken. This is the implementation of the maze game.

### State Machine
The state machine can be broken into two main components necessary to function. The first is the state machine itself built using switch statements and the other is a control block used to switch state variables. The state machine is in the main function and is in an infinite loop after the initialization of all protocols used in the device as well as the state variables initialized. The two variables used to switch states are currentState and previousState. To give an example of how the two functions work together would be from the main menu to the maze difficulty select screen. So upon loading the code currentState and previousState are set to be the main menu state. Then when entering the while loop it first checks if here is an interrupt from the IR remote which activates the control block. If the current state is the cursor state then this indicates a decision can be made with the IR remote. Upon first loading the control block will not be able to be activated as we are currently in the main menu state. This state will load and print the main menu screen and then switch the current state to cursor state and the previous state is set to the main menu state. Then if the IR remote is activated the control block will activate and check for the previous state to know the current action for the cursor. If the control block is activated and has a valid previous state the block will then begin checking for input from the user. In general two and five are used to change the cursor location and the mute check is used to change states from the cursor function to draw the appropriate screen. In this example the previous state is main menu state so the cursor will change using the main menu cursor locations and the cursor will be printed to the screen continuously waiting for user input and only switch if mute is pressed. If mute is pressed then using the current cursor location the correct screen state is called and the process is repeated. In general the functionality of the state machine can be described as check if the control block is active, if it is active check our previous state and then compare the user input for the appropriate action to be taken if there is not user input, then continuously print the current state on the screen which is the cursor state, except in the maze and hangman states. These states are switched differently and are discussed in their respective implementations.

### AWS
AWS was implemented using the http_post and http_get functions given to us in lab 4 with slight modifications. Http_post was not changed from lab 4, as this function intakes the JSON data string to be inserted into the AWS format and then sends this message to the AWS shadow. Http_get communicates with the AWS server to get our shadow statements, but was modified to return the integer value from the read message. The functionality remained the same from lab 4, except at the end of the function getScore function is called to extract the integer value from the AWS message. getScore uses a key value to parse through the function to find the start value. This value is then used to find the endpoint of the data by searching for the end value. Once this length is determined then we use strcpy to copy the entire length of the string from our start point. This is returned as a string value, then in http_get, using atoi this string value is converted to an integer and returned to the caller. generateJSON is used to generate the JSON string in AWS format. It intakes the given high score and places this in the JSON string and then when this string is passed into http_post it updates the shadow with this new value. Http_get was called when loading the game because in the win lose screen there is a check if the currentHighscore is less than the score just achieved in the game to update the high score for the game. If there was a high score then this score is converted to a string then passed into generateJSON and this string is finally passed into http_post to update the shadow with the new high score. Http_get was also used in the high score screen. When this screen is loaded it calls http_get to get the high score to be printed on the string. This is how AWS was implemented for high scores in the device.

### Hangman
The hangman game we tried to implement for this project consisted of two different modes, user vs user, where one person inputs the up to five letter word using the IR remote controller, and the other person would try to guess it using the same IR controller, and also another mode where the words are already predefined by us and are stored within the program, to be omitted randomly for the user to guess using the letters submitted through the IR controller. For the implementation, the hanging stand and the noose are first drawn onto the OLED, along with up to 5 horizontal lines which represent the slots where the letters in the word go, and a rectangle in the corner to store the incorrect letters. As the game progresses, parts of the hangman figure which are stored within the function are called upon, if it is determined that a letter submitted is not one of the letters in the word. For the guessed words, they are to be stored in an array which stores currently guessed letters and placeholders for letters that have not yet been guessed. For the state of the hangman, a counter or set of flags is used to track which parts of the hangman have been drawn, corresponding to the number of incorrect guesses, As mentioned earlier, these parts are readily available and are called in the sequential order of penalties (head, body, right arm, left arm, right leg, leg leg). The cursor that we used prior for other games will not be shown within the hangman game. The game loops through state transitions, between different states such as guessing letters, waiting on input from the user, and the end game (win/loss). The intended implementation was to have managed these transitions through a state machine where each state is corresponding to a part of the game logic. The input processing was to receive input for IR remote and to decode it in order of finding out which letter is pressed. For memory allocation within the game, most of the game's data structures are statically allocated to avoid the overhead and complexities associated with dynamic memory management.

## Challenges

### Menu Selection

The biggest issue encountered in this section was an efficient way to display our selection screens without having to either redraw the screens with new cursor position or delete the previously drawn screen with only our current cursor position. This was implemented with the creation of a cursor state and change cursor function. The cursor state calls a function called blinkcursor that will use the current cursor location to access an array with the appropriate screen cursor location. The change cursor function takes the appropriate array for the current screen and increments/decrements or resets the current cursor location variable and then uses this update to update the cursor variables. So in general while there is no interrupt from the remote the screen will print and immediately move into the cursor state. In the cursor state it will blink the cursor at the current location. If there is an interrupt from the remote the input is evaluated and depending on the input from the remote it will either change screens or call the change cursor function to move the cursor up or down so when blinkcursor is called again the cursor is in the new location.

There is also an issue that occurred during the recording of our demonstration. The issue being the cursor moving in a cyclic pattern across the current menu selection screen whenever attempting to video the project. The first step in investigation to come to the conclusion it must come from the phone was clicking non-recognised inputs such as the number 8 on the IR remote. As expected this input had no effect on the selection screens. Following this, holding the phone next to the IR circuit and pressing the power button on the Iphone caused the cursor to consistently move down to the next position.  Upon further research the Iphone has a proximity sensor that is always active that is [for features such as the raise-to-wake feature](https://www.moropo.com/post/list-of-all-sensors-available-in-iphone-15#:~:text=Proximity%20Sensors,visibility%20in%20any%20lighting%20condition). This proximity sensor utilizes IR to measure proximity and it seems this interferes with the IR circuit as it is being interpreted as an input. There is currently no fix for the problem but the problem was deduced.

### Maze Movement

Another Issue encountered during the development of the maze was crafting a collision system. The original collision system implemented, checked if the new location was at a banned location in the maze. A banned location is a location outside the range of the screen or where a maze wall exists. This allowed for the ball to phase through walls. The next implementation checked the 8 positions around the ball and determined banned positions then if the shifted position was one of these positions it would remain at the current location. This would still allow for the ball the phase through walls. The next attempt was to attempt to determine the respect of the ball with the walls around it and place x’s around every location outside the walls so the ball could not move to that position. Then we would update the position then clear all the added x’s. This implementation was never able to work as the x’s were unreliably placed. The final implementation checks every location of the ball that a pixel exists at and makes sure the location of every pixel did not contain an x which indicates a maze or was out of bounds. This was able to prevent the ball from passing through walls in most cases but in tight places or corners it may still occur.

### AWS Connection

When attempting to utilize the given functions for connecting to the internet, errors would occur due to wrong pin connections. In our pin configuration Pin 2 was used for the data line in the I2C communication but in the given functions, pin 2 was used for an LED during the initialization process. This was fixed by locating the macros for the LEDs in “gpio_if.h” and changing the macro from pin 2 to pin 64. This change allowed for communication between AWS and the board.

### Hangman

When attempting to implement the hangman game in the way mentioned earlier in the implementation section, we ran into a few issues with the game logic. Our main issue was that when the game was loading onto the screen, we kept getting “garbage” data written randomly onto our game, somewhere along where the “incorrectly guessed” letters would have gone. Additionally, somewhere in the game logic there were faults, as at times when debugging, our entire sequence of states for the hangman body parts building would just run in a loop without the press of any buttons. Another challenge was decoding the IR signals, similarly to lab 3, and getting the letter values from the various different button presses, as long as situations when the same button was pressed multiple times in a row. Interpreting the data, and finding a proper way to use the decoded data for our game to receive the now converted to letter input, and then throwing that into the game logic, was perhaps the most challenging obstacle in the way of completing the hangman game. This challenge brought on other issues on top of it, as not being able to convert the IR input into a letter, was probably one of the biggest steps to getting the game logic properly running. With more time to debug, this challenge would have been overcome.

## Future Work

### Save State Implementation

For further improvements to the device, a save state of the current state and relevant variables would be implemented. The relevant variables required would be to save the current x and y location of the ball, the current maze array, and the current score. AWS would be used to save the state of all these variables using http_post and passing in the variables into the JSON message. Then once saving is done the states then navigate back to the main menu. Then when choosing the maze game it will ask to load the save or play a new game. Loading the game will call http_get() to get the JSON message, This message will then be parsed to obtain the relevant variables and then passing these variables into the mazeGame function. The input that would be mapped to this would be the digit 3 on the remote to keep all the inputs in a similar location on the remote.
 
### Random Maze Generation

Another improvement made could be from implementing random maze generation for random mazes. The first step would be to implement another cursor location on the maze difficulty selection screen for “Random.” The next step would be to generate an array with a random maze to be passed into the game function. This would be done using an algorithm to generate minimum spanning trees. Some popular algorithms are Kruskals, Prim’s, and Aldous-Broder. With further research to select the best algorithm this could be used to generate a random maze for the user to navigate.

### Random Word Selection in Hangman

One final improvement (aside from finishing hangman), would come from the implementation of a random word selection for solo play in hangman. This would be done by adding another selection for hangman rather than directly loading the game when selected. If the random word selection was made then an API would be used to query a database for a random word to be used in hangman. API ninja has a random word generator that can be interfaced with. This would most likely be used to generate a random word for the user.
  

## Finalized BOM

| No. | PART NAME                           | DESCRIPTION                                           | Qty | SUPPLIER / MANUFACTURER | UNIT COST | TOTAL PART COST | Purpose                                  |
|-----|-------------------------------------|-------------------------------------------------------|-----|--------------------------|------------|-----------------|------------------------------------------|
| 1   | CC3200-LAUNCHXL                     | MCU Board                                             | 1   | University of California, Davis | $66.00     | $66.00          | Interpret inputs from the IR circuit, read and interpret information from the onboard accelerometer and to send output to the OLED |
| 2   | Adafruit OLED SSD1351 Display       | 128x128 RGB Display, SPI Protocol                    | 1   | University of California, Davis | $39.95     | $39.95          | Display the state of the game            |
| 3   | AT&T S10-S3 General Remote          | General-purpose TV remote. IR NEC protocol           | 1   | University of California, Davis | $9.99      | $9.99           | Take user input                         |
| 4   | Vishay TSOP381xx/383xx/385xx IR Receiver | 30-56kHz Carrier Frequency depending on model    | 1   | University of California, Davis | $1.41      | $1.41           | Receive input from the IR remote        |
| 5   | 100Ω resistor                       | 0.5 W, 5% tolerance                                   | 1   | University of California, Davis | $0.57      | $0.57           | Limit current for the IR receiver        |
| 6   | 100μF capacitor                     | 16V Electrolytic Capacitor                            | 1   | University of California, Davis | $0.50      | $0.50           | DC filtering for the IR circuit        |

