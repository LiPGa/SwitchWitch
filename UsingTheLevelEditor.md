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
Press the Info button to access the level variables. Here, you can change the id and score thresholds.

**Note: If a level with the same id already exists in the save directory, it will automatically replace the level.** 

## Editing replacement.
Press the **next button** to load up a new board. This board represents the next set of units that will replace the units in the previous board if they are killed. Pressing the next button will automatically increase the turn counter.

Press the **back button** to go to the previous board configuration.

Press the **delete button** to remove the current board. This also automatically reduces the number of turns by 1. By deleting the current board, all board configurations will shift to the left by 1. For example, if the board configuration that is used for the 1st unit replacemment is deleted, then the 2nd unit replacement board will be used for the 1st unit replacement and so on and so forth.

Which board represents which point in unit replacement can be seen in the bottom left corner. 0/0 is the initial state. This means that only the initial configuration of the board exists. Therefore, there is no turns that the player can do. Once the next button is pressed the counter increases by 1/1. This means that you are currently looking at the 1st unit replacement board, meaning that for all units on the initial board, when they are killed they will be replaced by the unit on the square in the 1st unit replcaement board.

**In order to add random uints** place the R token on the square.

## Saving
Saving a level is done by clicking on the save button or pressing ctrl+S.

### For Mac
The path the Json files are located is specific to the user. You might need to read the print statement to see where it is.
e.g. 2022-03-23 16:32:39.118067-0400 SwitchWitch (Mac)[30277:10179121] INFO: The file path is: /Users/zhangyihan/Library/Application Support/GDIAC/Switch Witch//board6.json
When I tested it it seems like I could not find the folder from my computer UI. So if that happens, You can access the folder by opening the terminal (for a mac user) and then do the command: "cd /Users/zhangyihan/Library/Application Support/GDIAC/Switch Witch" and then do "open ." which will open the folder. 
One thing that need to be careful is that if there is a pre-existing json file then saving the current json file will not replace the previous one and thus has no effect. You might need to manually delete it yourself and then click the 'save' button.

### For Windows
To retrieve the json file. Open Run by going to the search bar and searching run on your computer. Once opened, type in "appdata". Then AppData/Roaming/GDIAC/Switch Witch. The saved file should be there.

## Playing
Play a level by pressing ENTER or pressing the play button. This will automatically take you to the game scene. Return to the level editor by pressing ESCAPE.

## Changing Board Size
The board size is linked to the size of individual squares. All squares will have the same size. In order to change the size of squares, press the info button and change the Sq Size variable.
The default size of squares can be found in the constants.json file.

# Changing JSON Values directly in JSON
Sometimes, it is faster to change the JSON files of levels already loaded into the game directly than using the level editor. However, some variables should not be changed or else they might corrupt the level files themselves.
**The following variables can be changed in JSON without corrupting the file**
- id (make sure you change the name of the file to board{id}). This way you can change the order of levels without using the level editor).
- one-star-condition
- two-star-condition
- three-star-condition
- background (as long as that type of background exists, you can find the backgrounds that exist in constants.json).
- square-size

Any other variables should not be manipulated unless if you create a backup of the file.

# Creating New Units
Follow these steps to create a new unit
1. Add a unit in boardMember.json. All units must have the exact following format for it to work:
```json
    "[unitType]": {
        "texture-red": "[unitType]-red",
        "texture-blue": "[unitType]-blue",
        "texture-green": "[unitType]-green",
        "basic-attack": [[1,0]],
        "special-attack":[],
        "probability-respawn": 90
    },
```
For attacks, all attacks must be in the form of an array with the elements of the array representing vectors. 
The square that is attacked is the sum of the position of the square the unit is on plus the vector. 
For example, a basic unit has a basic-attack of [1,0]. If this basic unit is at square [3,2] it will attack square [4,2].
**Note:** The attack information should be inputted assuming that the unit is facing the right.
The game will rotate the vectors depending on the direction automatically.
**Example:** 
```json
    "three-way": {
        "texture-red": "three-way-red",
        "texture-blue": "three-way-blue",
        "texture-green": "three-way-green",
        "basic-attack": [[1,0]],
        "special-attack": [[1,1], [1,0], [1,-1]],
        "probability-respawn": 4
    },
```
2. Add the unit textures into the assets folder. Every unit should have three textures depending on the color. All textures should use "_" instead of space.
3. Add the textures into assets.json. All textures should have this format under the "textures" tag.
```json
"[texture-Name]": {
    "file": "textures/[texture_Name].png"
},
```
The tag for the texture should use "-" instead of space. While the actual file name should use "_".
**Example:** 
```json
"basic-red": {
    "file": "textures/red_unit.png"
},
"basic-blue": {
    "file": "textures/blue_unit.png"
},
"basic-green": {
    "file": "textures/orange_unit.png"
},
```
4. Add the texture references in constants.json. It should be under the "textures" tag. 
The name of the texture must be the same as the tag for the texture in assets.
