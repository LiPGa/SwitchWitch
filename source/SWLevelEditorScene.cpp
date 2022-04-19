//
//  SWLevelEditorScene.cpp
//  Switch Witch
//
//  This is the primary class file for running the game.  You should study this file
//  for ideas on how to structure your own root class. This class is a reimagining of
//  the first game lab from 3152 in CUGL.
//
//  Based on Ship Lab
//  Author: Walker White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 1/20/22
//
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <algorithm>

#include "SWLevelEditorScene.h"

using namespace cugl;
using namespace std;

#define SELECTION_BOARD_WIDTH 6
#define SELECTION_BOARD_HEIGHT 2

/** How to reduce the scale of textures for select*/
#define LEVEL_EDITOR_SQUARE_SCALE 0.8

#define SELECTION_BOARD_Y_OFFSET -0.35

#pragma mark Constructors
/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool LevelEditorScene::init(const std::shared_ptr<cugl::AssetManager>& assets)
{
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    if (assets == nullptr) return false;

    // GetJSONValuesFromAssets
    std::shared_ptr<cugl::JsonValue> constants = assets->get<JsonValue>("constants");
    std::shared_ptr<cugl::JsonValue> boardMembers = assets->get<JsonValue>("boardMember");

    // Initialize Constants
    int sceneHeight = constants->getInt("scene-height");
    vector<int> boardSize = constants->get("board-size")->asIntArray();
    _maxBoardWidth = boardSize.at(0);
    _maxBoardHeight = boardSize.at(1);
    _pressedBoardSize = false;
    _rows = _maxBoardHeight;
    _columns = _maxBoardWidth;
    auto defaultSquareSize = constants->getInt("square-size");
    _currentBoardTurn = 0;

    // Initialize Scene
    dimen *= sceneHeight / dimen.height;
    if (!Scene2::init(dimen)) return false;

    // Start up the input handler
    _input.init();

    // Initialize States and Play Pressed
    _currentState = State::NOTHING;
    _playPressed = false;

    // Get the background image and constant values
    auto background = assets->get<Texture>("background");
    _backgroundNode = scene2::PolygonNode::allocWithTexture(background);
    _scale = getSize() / background->getSize();
    _backgroundNode->setScale(_scale);

    // Get Textures
    vector<string> textureVec = constants->get("textures")->asStringArray();
    for (string textureName : textureVec) {
        _textures.insert({ textureName, assets->get<Texture>(textureName) });
    }

    // Allocate Layout
    _rootNode = scene2::SceneNode::allocWithBounds(getSize());
    _layout = scene2::AnchoredLayout::alloc();
    addChild(_backgroundNode);
    _backgroundNode->setAnchor(Vec2::ZERO);

    // set up text fields
    _buttonsNode = assets->get<scene2::SceneNode>("level-editor-board");
    _buttonsNode->setContentSize(dimen);
    _buttonsNode->doLayout();
    _rootNode->addChild(_buttonsNode);

    // Initialize Buttons
    _playButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level-editor-board_play"));
    _saveButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level-editor-board_save"));
    _nextButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level-editor-board_next"));
    _backButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level-editor-board_back"));
    _deleteButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level-editor-board_delete"));
    _infoButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level-editor-board_info"));
    _saveButton->addListener([this](const std::string& name, bool down) {
        if (down) {
            saveBoardAsJSON();
        }
        });
    _playButton->addListener([this](const std::string& name, bool down) {
        if (down && _level->maxTurns > 0) {
            _playPressed = true;
        }
        });
    _nextButton->addListener([this](const std::string& name, bool down) {
        if (down) {
            if (_currentBoardTurn == _level->maxTurns) {
                _level->maxTurns++;
                _level->addBoard(allocBasicBoard(_level->getNumberOfColumns(), _level->getNumberOfRows()));
                // Make sure that empty squares are kept.
                for (auto square : _level->getBoard(_currentBoardTurn)->getAllSquares()) {
                    auto squarePosition = square->getPosition();
                    if (_level->getBoard(_currentBoardTurn)->getSquare(squarePosition)->getUnit()->getSubType() == "empty") {
                        _level->getBoard(_currentBoardTurn + 1)->getSquare(squarePosition)->getUnit()->setSubType("empty");
                    }
                }
            } 
            _currentBoardTurn++;
            _turnTextLabel->setText(strtool::format("Board: %d/%d", _currentBoardTurn, _level->maxTurns));
            updateBoardNode();
        }
        });
    _backButton->addListener([this](const std::string& name, bool down) {
        if (down) {
            if (_currentBoardTurn > 0) _currentBoardTurn--;
            _turnTextLabel->setText(strtool::format("Board: %d/%d", _currentBoardTurn, _level->maxTurns));
            updateBoardNode();
        }
        });
    _deleteButton->addListener([this](const std::string& name, bool down) {
        if (down) {
            if (_level->maxTurns > 0) {
                _level->maxTurns--;
                _level->removeBoard(_currentBoardTurn);
                _currentBoardTurn--;
            }
            _turnTextLabel->setText(strtool::format("Board: %d/%d", _currentBoardTurn, _level->maxTurns));
            updateBoardNode();
        }
        });

    _infoButton->addListener([this](const std::string& name, bool down) {
        if (down) {
            showInfo();
        }
        });

    // Initialize Text Inputs
    _infoNode = assets->get<scene2::SceneNode>("level-editor-info");
    _infoNode->setContentSize(dimen);
    _infoNode->doLayout();
    _rootNode->addChild(_infoNode);
    _levelIDText = std::dynamic_pointer_cast<scene2::TextField>(assets->get<scene2::SceneNode>("level-editor-info_id-field"));
    _oneStarScoreText = std::dynamic_pointer_cast<scene2::TextField>(assets->get<scene2::SceneNode>("level-editor-info_one-star-score-field"));
    _twoStarScoreText = std::dynamic_pointer_cast<scene2::TextField>(assets->get<scene2::SceneNode>("level-editor-info_two-star-score-field"));
    _threeStarScoreText = std::dynamic_pointer_cast<scene2::TextField>(assets->get<scene2::SceneNode>("level-editor-info_three-star-score-field"));
    _changeBoardSizeButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level-editor-info_board-size"));

    _levelIDText->addTypeListener([this](const std::string& name, const std::string& value) {
        if (value.empty()) return;
        if (!isInteger(value)) _levelIDText->setText(to_string(_level->levelID));
     });
    _levelIDText->addExitListener([this](const std::string& name, const std::string& value) {
        if (isInteger(value)) _level->levelID = stoi(value);
        else _levelIDText->setText(to_string(_level->levelID));
        });
    _oneStarScoreText->addTypeListener([this](const std::string& name, const std::string& value) {
        if (value.empty()) return;
        if (!isInteger(value)) _oneStarScoreText->setText(to_string(_level->oneStarThreshold));
    });
    _oneStarScoreText->addExitListener([this](const std::string& name, const std::string& value) {
        if (isInteger(value)) _level->oneStarThreshold = stoi(value);
        else _oneStarScoreText->setText(to_string(_level->oneStarThreshold));
        });
    _twoStarScoreText->addTypeListener([this](const std::string& name, const std::string& value) {
        if (value.empty()) return;
        if (!isInteger(value)) _twoStarScoreText->setText(to_string(_level->twoStarThreshold));
        });
    _twoStarScoreText->addExitListener([this](const std::string& name, const std::string& value) {
        if (isInteger(value)) _level->twoStarThreshold = stoi(value);
        else _twoStarScoreText->setText(to_string(_level->twoStarThreshold));
        });
    _threeStarScoreText->addTypeListener([this](const std::string& name, const std::string& value) {
        if (value.empty()) return;
        if (!isInteger(value)) _threeStarScoreText->setText(to_string(_level->threeStarThreshold));
        });
    _threeStarScoreText->addExitListener([this](const std::string& name, const std::string& value) {
        if (isInteger(value)) _level->threeStarThreshold = stoi(value);
        else _threeStarScoreText->setText(to_string(_level->threeStarThreshold));
        });
    _boardButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level-editor-info_board"));
    _boardButton->addListener([this](const std::string& name, bool down) {
        if (down) {
            showBoard();
        }
        });
    _changeBoardSizeButton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _pressedBoardSize = !_pressedBoardSize && _currentState == CHANGING_INFO;
            if (_pressedBoardSize) {
                _changeBoardSizeButton->setColor(Color4::RED);
            }
            else {
                _changeBoardSizeButton->setColor(Color4::BLUE);
            }
        }
        });


    // Initialize units with different types
    // Children will be types "basic", "three-way", etc.
    auto children = boardMembers->get("unit")->children();
    for (auto child : children) {
        // Get basic attack
        auto subtypeString = child->key();
        auto basicAttackJson = child->get("basic-attack")->children();
        vector<Vec2> basicAttackVec;
        for (auto basicAttack : basicAttackJson) {
            auto basicAttackArray = basicAttack->asFloatArray();
            basicAttackVec.push_back(Vec2(basicAttackArray.at(0), basicAttackArray.at(1)));
        }
        // Get special attack
        auto specialAttackJson = child->get("special-attack")->children();
        vector<Vec2> specialAttackVec;
        for (auto specialAttack : specialAttackJson) {
            auto specialAttackArray = specialAttack->asFloatArray();
            specialAttackVec.push_back(Vec2(specialAttackArray.at(0), specialAttackArray.at(1)));
        }

        // store the default color:red for this type of unit
        shared_ptr<Unit> unit = Unit::alloc(subtypeString, Unit::Color::RED, basicAttackVec, specialAttackVec, Vec2(0, -1));
        _unitTypes.insert({ child->key(), unit });
    }

    // Initialize Board
    _board = allocBasicBoard(_maxBoardWidth, _maxBoardHeight);

    /// Initialize Selection Board
    _selectionBoard = Board::alloc(SELECTION_BOARD_WIDTH, SELECTION_BOARD_HEIGHT);

    // Set the view of the board.
    _squareSizeAdjustedForScale = int(defaultSquareSize * min(_scale.width, _scale.height) * LEVEL_EDITOR_SQUARE_SCALE);
    _boardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, _maxBoardWidth * _squareSizeAdjustedForScale, _maxBoardHeight * _squareSizeAdjustedForScale));
    _boardNode->setName("boardNode");
    _layout->addRelative("boardNode", cugl::scene2::Layout::Anchor::CENTER, Vec2(0, 0));
    _boardNode->setTexture(_textures.at("transparent"));
    _board->setViewNode(_boardNode);
    _rootNode->addChildWithName(_boardNode, "boardNode");

    for (int i = 0; i < _maxBoardWidth; i++) {
        for (int j = 0; j < _maxBoardHeight; ++j) {
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square"));
            auto squarePosition = Vec2(i, j);
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * _squareSizeAdjustedForScale) + Vec2::ONE * (_squareSizeAdjustedForScale / 2));
            squareNode->setScale((float)_squareSizeAdjustedForScale / (float)defaultSquareSize);
            shared_ptr<Square> sq = _board->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            _board->getViewNode()->addChild(squareNode);
            // Generate unit for this square
            auto unit = sq->getUnit();
            std:string unitPattern = getUnitType(unit->getSubType(), Unit::colorToString(unit->getColor()));
            auto unitNode = scene2::PolygonNode::allocWithTexture(_textures.at(unitPattern));
            unit->setViewNode(unitNode);
            unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);
        }
    }

    // Set the view of the select board.
    _selectionBoardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, _maxBoardWidth * _squareSizeAdjustedForScale, _squareSizeAdjustedForScale));
    _layout->addRelative("selectionBoardNode", cugl::scene2::Layout::Anchor::CENTER, Vec2(0, SELECTION_BOARD_Y_OFFSET));
    _selectionBoardNode->setTexture(_textures.at("transparent"));
    _selectionBoard->setViewNode(_selectionBoardNode);
    _rootNode->addChildWithName(_selectionBoardNode, "selectionBoardNode");

    for (int i = 0; i < SELECTION_BOARD_WIDTH; i++) {
        for (int j = 0; j < SELECTION_BOARD_HEIGHT; ++j) {
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square"));
            auto squarePosition = (Vec2(i, j));
            squareNode->setScale(((float)_squareSizeAdjustedForScale / (float)defaultSquareSize));
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * _squareSizeAdjustedForScale) + Vec2::ONE * (_squareSizeAdjustedForScale / 2));
            shared_ptr<Square> sq = _selectionBoard->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            _selectionBoard->getViewNode()->addChild(squareNode);
        }
    }
    auto i = 0;
    for (auto element : _unitTypes) {
        auto square = _selectionBoard->getAllSquares()[i];
        auto unitType = element.second;        
        square->setUnit(unitType);
        auto test = getUnitType(unitType->getSubType(), Unit::colorToString(unitType->getColor()));
        auto unitNode = scene2::PolygonNode::allocWithTexture(_textures.at(getUnitType(unitType->getSubType(), Unit::colorToString(unitType->getColor()))));
        square->getUnit()->setViewNode(unitNode);
        square->getViewNode()->addChild(unitNode);
        i++;
    }

    // Initialize Level
    _level = Level::alloc(_maxBoardWidth, _maxBoardHeight);
    _level->addBoard(allocBasicBoard(_maxBoardWidth, _maxBoardHeight));

    // Initialize Turn Counter
    std::string turnMsg = strtool::format("Board: %d/%d", _currentBoardTurn, _level->maxTurns);
    _turnTextLabel = scene2::Label::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _turnTextLabel->setContentSize(Vec2(getSize().width, _turnTextLabel->getContentHeight()));
    _rootNode->addChild(_turnTextLabel);

    // Initialize Units Needed To Kill Counter
    std::string unitsNeededToKillMsg = strtool::format("Units Needed To Kill: 00");
    _unitsNeededToKillLabel = scene2::Label::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _unitsNeededToKillLabel->setContentSize(Vec2(getSize().width, _unitsNeededToKillLabel->getContentHeight()));
    _layout->addRelative("unitsNeededToKillLabel", cugl::scene2::Layout::Anchor::BOTTOM_LEFT, Vec2(0, 0.075));
    _rootNode->addChildWithName(_unitsNeededToKillLabel, "unitsNeededToKillLabel");
    _unitsNeededToKillLabel->setVisible(false);

    // Initialize Text For Board Size
    std::string boardSizeMessage = strtool::format("Size: %dx%d", _columns, _rows);
    _boardSizeLabel = scene2::Label::allocWithText(boardSizeMessage, assets->get<Font>("pixel32"));
    _boardSizeLabel->setContentSize(Vec2(getSize().width, _unitsNeededToKillLabel->getContentHeight()));
    _layout->addRelative("boardSizeLabel", cugl::scene2::Layout::Anchor::BOTTOM_RIGHT, Vec2(-0.5, 0));
    _rootNode->addChildWithName(_boardSizeLabel, "boardSizeLabel");
    
    
    addChild(_rootNode);

    reset();
    return true;
}

/**
 * Get the pattern for a unit provided its type and color
 *
 */
std::string LevelEditorScene::getUnitType(std::string type, std::string color) {
return type + "-" + color;
}

bool LevelEditorScene::isInteger(const std::string& s) {
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;
    char* p;
    strtol(s.c_str(), &p, 10);
    return (*p == 0);
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LevelEditorScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
    }
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LevelEditorScene::update(float timestep)
{
    // Read the keyboard for each controller.
    // Read the input
    _input.update();
    Vec2 pos = _input.getPosition();
    Vec3 worldCoordinate = screenToWorldCoords(pos);
    Vec2 boardPos = _boardNode->worldToNodeCoords(worldCoordinate);
    Vec2 selectionBoardPos = _selectionBoardNode->worldToNodeCoords(worldCoordinate);
    // Gets the current board that should be displayed
    auto currentBoard = _level->getBoard(_currentBoardTurn);
    if (_input.didPress() && _currentState == CHANGING_BOARD)
    {
        Vec2 squarePos = Vec2(int(boardPos.x / (_squareSizeAdjustedForScale)), int(boardPos.y / (_squareSizeAdjustedForScale)));
        Vec2 selectionSquarePos = Vec2(int(selectionBoardPos.x / (_squareSizeAdjustedForScale)), int(selectionBoardPos.y / (_squareSizeAdjustedForScale)));
        if (_board->doesSqaureExist(squarePos) && boardPos.x >= 0 && boardPos.y >= 0 && _board->getSquare(squarePos)->isInteractable())
        {
            auto squareOnMouse = _board->getSquare(squarePos);
            if (_selectedSquare != squareOnMouse) {
                if (_selectedSquare != NULL) _selectedSquare->getViewNode()->setTexture(_textures.at("square"));
                _selectedSquare = squareOnMouse;
                _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
                if (_selectedUnitFromSelectionBoard != NULL && _selectedUnitFromSelectionBoard->getUnit() != NULL) {
                    auto unit = currentBoard->getSquare(squarePos)->getUnit();
                    auto unitThatWillReplace = _selectedUnitFromSelectionBoard->getUnit();
                    unit->setBasicAttack(unitThatWillReplace->getBasicAttack());
                    unit->setSpecialAttack(unitThatWillReplace->getSpecialAttack());
                    unit->setSubType(unitThatWillReplace->getSubType());
                    updateBoardNode();
                }
            }
            else {
                if (_selectedSquare != NULL) _selectedSquare->getViewNode()->setTexture(_textures.at("square"));
                _selectedSquare = NULL;
            }
        }
        else {
            if (_selectedSquare != NULL) _selectedSquare->getViewNode()->setTexture(_textures.at("square"));
            _selectedSquare = NULL;
        }
        if (_selectionBoard->doesSqaureExist(selectionSquarePos) && selectionBoardPos.x >= 0 && selectionBoardPos.y >= 0) {
            auto squareOnMouse = _selectionBoard->getSquare(selectionSquarePos);
            if (_selectedUnitFromSelectionBoard != squareOnMouse) {
                if (_selectedUnitFromSelectionBoard != NULL) _selectedUnitFromSelectionBoard->getViewNode()->setTexture(_textures.at("square"));
                _selectedUnitFromSelectionBoard = squareOnMouse;
                _selectedUnitFromSelectionBoard->getViewNode()->setTexture(_textures.at("square-selected"));
            }
            else {
                _selectedUnitFromSelectionBoard->getViewNode()->setTexture(_textures.at("square"));
                _selectedUnitFromSelectionBoard = NULL;
            }
        }
    }
    if (_selectedSquare != NULL && _currentState == State::CHANGING_BOARD) {
        auto unit = currentBoard->getSquare(_selectedSquare->getPosition())->getUnit();
        bool change = false;
        if (unit->getSubType() != "king") {
            if (_input.isDirectionKeyDown()) {
                unit->setDirection(_input.directionPressed());
                updateBoardNode();
            }
            if (_input.isRedDown()) {
                unit->setColor(Unit::RED);
                updateBoardNode();
            }
            else if (_input.isGreenDown()) {
                unit->setColor(Unit::GREEN);
                updateBoardNode();
            }
            else if (_input.isBlueDown()) {
                unit->setColor(Unit::BLUE);
                updateBoardNode();
            }
            _unitsNeededToKillLabel->setVisible(false);
        }
        else {
            if (_input.isDirectionKeyDown()) {
                auto neededToKill = unit->getUnitsNeededToKill();
                if (_input.directionPressed() == Vec2(0, 1)) {
                    neededToKill++;
                }
                else if (_input.directionPressed() == Vec2(0, -1) && neededToKill > 0) {
                    neededToKill--;
                }
                unit->setUnitsNeededToKill(neededToKill);
                auto test = unit->getUnitsNeededToKill();
                auto test2 = unit->getUnitsNeededToKill();
                updateBoardNode();
            }
            _unitsNeededToKillLabel->setVisible(true);
            std::string unitsNeededToKillMsg = strtool::format("Units Needed To Kill: %d", unit->getUnitsNeededToKill());
            _unitsNeededToKillLabel->setText(unitsNeededToKillMsg);
        }
        
    }
    else {
        _unitsNeededToKillLabel->setVisible(false);
    }
    if (_input.isSaveDown()) {
        saveBoardAsJSON();
    }
    if (_pressedBoardSize) {
        if (_input.directionPressed() == Vec2(0, 1) && _rows < _maxBoardHeight) {
            _rows++;
        }
        else if (_input.directionPressed() == Vec2(0, -1) && _rows > 1) {
            _rows--;
        }
        else if (_input.directionPressed() == Vec2(1, 0) && _columns < _maxBoardWidth) {
            _columns++;
        }
        else if (_input.directionPressed() == Vec2(-1, 0) && _columns > 1) {
            _columns--;
        }
        _boardSizeLabel->setText(strtool::format("Size: %dx%d", _columns, _rows));
        if (_columns != _level->getNumberOfColumns() || _rows != _level->getNumberOfRows()) {
            _boardSizeLabel->setColor(Color4::RED);
        }
        else {
            _boardSizeLabel->setColor(Color4::BLACK);
        }
           
    }
    _layout->layout(_rootNode.get());
}

#pragma mark -
#pragma mark Rendering


/**
 * Sets whether the scene is currently active
 *
 * @param value whether the scene is currently active
 */
void LevelEditorScene::setActive(bool value) {
    _active = value;
    _playPressed = false;
    if (value) {
        Input::activate<TextInput>();
        showBoard();
    }
    else {
        _playButton->deactivate();
        _saveButton->deactivate();
        _nextButton->deactivate();
        _backButton->deactivate();
        _deleteButton->deactivate();
        _infoButton->deactivate();
        // If any were pressed, reset them
        _playButton->setDown(false);
        _saveButton->setDown(false);
        _nextButton->setDown(false);
        _backButton->setDown(false);
        _deleteButton->setDown(false);
        _infoButton->setDown(false);
    }
    if (!value) {
        Input::deactivate<TextInput>();
    }
}

shared_ptr<Board> LevelEditorScene::allocBasicBoard(int width, int height) {
    shared_ptr<Board> result = Board::alloc(width, height);
    // Create the squares & units and put them in the map
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; ++j) {
            auto squarePosition = Vec2(i, j);
            shared_ptr<Square> sq = result->getSquare(squarePosition);
            // Generate unit for this square
            std::string unitSubType = "random";
            auto unitColor = "red";
            std:string unitPattern = getUnitType(unitSubType, unitColor);
            Vec2 unitDirection = Unit::getDefaultDirection();
            auto unitTemplate = _unitTypes.at(unitSubType);
            Unit::Color c = Unit::stringToColor(unitColor);
            shared_ptr<Unit> unit = Unit::alloc(unitSubType, c, unitTemplate->getBasicAttack(), unitTemplate->getSpecialAttack(), unitDirection);
            sq->setUnit(unit);
        }
    }
    return result;
}

void LevelEditorScene::updateBoardNode() {
    auto currentBoard = _level->getBoard(_currentBoardTurn);
    for (int i = 0; i < _maxBoardWidth; i++) {
        for (int j = 0; j < _maxBoardHeight; ++j) {
            auto squarePosition = Vec2(i, j);
            if (!currentBoard->doesSqaureExist(squarePosition)) {
                _board->getSquare(squarePosition)->setInteractable(false);
                _board->getSquare(squarePosition)->getViewNode()->setVisible(false);
                continue;
            }
            auto unit = currentBoard->getSquare(squarePosition)->getUnit();
            auto unitNode = _board->getSquare(squarePosition)->getUnit()->getViewNode();
            unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
            unitNode->setTexture(_textures.at(getUnitType(unit->getSubType(), Unit::colorToString(unit->getColor()))));
            _board->getSquare(squarePosition)->setInteractable(true);
            _board->getSquare(squarePosition)->getViewNode()->setVisible(true);
        }
    }
}

/**
 * Saves the JSON value in the application save directory
 */
void LevelEditorScene::saveBoardAsJSON() {
    std::ostringstream fileName;
    fileName << "board" << _level->levelID << ".json";
    vector<string> path = {Application::get()->getSaveDirectory(), fileName.str()};
    CULog("The file path is: %s", filetool::join_path(path).c_str());
    shared_ptr<cugl::JsonWriter> saveDirectory = cugl::JsonWriter::alloc(filetool::join_path(path));
    saveDirectory->writeJson(_level->convertToJSON());
}

void LevelEditorScene::showBoard() {
    Input::activate<TextInput>();

    _playButton->activate();
    _saveButton->activate();
    _nextButton->activate();
    _backButton->activate();
    _deleteButton->activate();
    _infoButton->activate();
    _changeBoardSizeButton->deactivate();
    
    _changeBoardSizeButton->setDown(false);

    _levelIDText->deactivate();
    _oneStarScoreText->deactivate();
    _twoStarScoreText->deactivate();
    _threeStarScoreText->deactivate();

    _buttonsNode->setVisible(true);
    _boardNode->setVisible(true);
    _selectionBoardNode->setVisible(true);
    _infoNode->setVisible(false);
    _currentState = CHANGING_BOARD;
    _pressedBoardSize = false;
    _changeBoardSizeButton->setColor(Color4::BLUE);
    if (_rows != _level->getNumberOfRows() || _columns != _level->getNumberOfColumns()) {
        // Initialize Level
        auto newLevel = Level::alloc(_columns, _rows);
        newLevel->addBoard(allocBasicBoard(_columns, _rows));
        setLevel(newLevel);
    }
}

void LevelEditorScene::showInfo() {
    Input::activate<TextInput>();
    _levelIDText->activate();
    _oneStarScoreText->activate();
    _twoStarScoreText->activate();
    _threeStarScoreText->activate();
    _changeBoardSizeButton->activate();

    _playButton->deactivate();
    _saveButton->deactivate();
    _nextButton->deactivate();
    _backButton->deactivate();
    _deleteButton->deactivate();
    

    // If any were pressed, reset them
    _playButton->setDown(false);
    _saveButton->setDown(false);
    _nextButton->setDown(false);
    _backButton->setDown(false);
    _deleteButton->setDown(false);
    _infoButton->setDown(false);
    _changeBoardSizeButton->setDown(false);

    _buttonsNode->setVisible(false);
    _boardNode->setVisible(false);
    _selectionBoardNode->setVisible(false);
    _infoNode->setVisible(true);
    _boardButton->activate();
    _currentState = CHANGING_INFO;
}

void LevelEditorScene::setLevel(shared_ptr<Level> level) {
    _level = level;
    _currentBoardTurn = 0;
    _turnTextLabel->setText(strtool::format("Board: %d/%d", _currentBoardTurn, _level->maxTurns));
    
    _levelIDText->setText(to_string(_level->levelID));
    _oneStarScoreText->setText(to_string(_level->oneStarThreshold));
    _twoStarScoreText->setText(to_string(_level->twoStarThreshold));
    _threeStarScoreText->setText(to_string(_level->threeStarThreshold));
    updateBoardNode();
}
