# Using the Level Editor

**Note:** The level editor only currently works on Desktop. 

## Placing Units
The level editor has a selection board of units to choose bellow the board.
Select a unit type by clicking on it. Once the square is greeen, it is selected.
Now, clicking on any square on the board will automatically change the unit type to the one selected.

## Changing Color
Colors are changed by selecting a square on the board and pressing one of the buttons.
- 1 or R for Red
- 2 or G for Green
- 3 or B for Blue

## Changing Direction
The direction the unit is facing is changed by selecting a square on the board and pressing one of the directional buttons or WSAD.

## Editing Level Variables
Editing the number of turns, score, or level id is done by clicking on one of the text input's (black boxes) next to the attribute you want to change.

**Note: If a level with the same id already exists in the save directory, it will automatically replace the level.** 

## Saving
Saving a level is done by clicking on the save button or pressing ctrl+S. The board will be saved in the assets folder.
The path the Json files are located is specific to the user. You might need to read the print statement to see where it is.
e.g. 2022-03-23 16:32:39.118067-0400 SwitchWitch (Mac)[30277:10179121] INFO: The file path is: /Users/zhangyihan/Library/Application Support/GDIAC/Switch Witch//board6.json
When I tested it it seems like I could not find the folder from my computer UI. So if that happens, You can access the folder by opening the terminal (for a mac user) and then do the command: "cd /Users/zhangyihan/Library/Application Support/GDIAC/Switch Witch" and then do "open ." which will open the folder. 
One thing that need to be careful is that if there is a pre-existing json file then saving the current json file will not replace the previous one and thus has no effect. You might need to manually delete it yourself and then click the 'save' button.

## Playing
Play a level by pressing ENTER or pressing the play button. This will automatically take you to the game scene. Return to the level editor by pressing ESCAPE.
