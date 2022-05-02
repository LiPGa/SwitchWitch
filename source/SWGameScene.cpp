//
//  SWGameScene.cpp
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
#include <cugl/scene2/actions/CUMoveAction.h>
#include <cugl/scene2/actions/CUScaleAction.h>
#include <cugl/scene2/actions/CUFadeAction.h>
#include <cugl/scene2/actions/CUAnimateAction.h>
#include <cugl/math/CUEasingBezier.h>
//#include <unistd.h>

#include "SWGameScene.h"

using namespace cugl;
using namespace std;
#define PADDING_SCALE = 1.2

///** Define the time settings for animation */
#define SWAP_DURATION 0.5f
#define WALKPACE 100
#define ACT_KEY  "current"
#define ALT_KEY  "slide"

/** How large the unit should be for tween*/
#define ENLARGE 1.25f
#define BACK2NORMAL 1.0f

/** Scene2 z-order definitions */
#define UNIT_Z 1
#define SQUARE_Z 2

/** Initial amount of time where no interactions do anything **/
#define TIME_BEFORE_INTERACTION 0.2

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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
{
    _time = 0;
    _debug = false;
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    if (assets == nullptr)
        return false;

    // GetJSONValuesFromAssets
    std::shared_ptr<cugl::JsonValue> constants = assets->get<JsonValue>("constants");
    std::shared_ptr<cugl::JsonValue> boardMembers = assets->get<JsonValue>("boardMember");

    // Initialize Constants
    auto sceneHeight = constants->getInt("scene-height");
    vector<int> boardSize = constants->get("board-size")->asIntArray();
    _maxBoardWidth = boardSize.at(0);
    _maxBoardHeight = boardSize.at(1);
    _defaultSquareSize = constants->getInt("square-size");


    // Initialize Scene
    dimen *= sceneHeight / dimen.height;
    if (!Scene2::init(dimen))
        return false;

    // Start up the input handler
    _input.init();

    // Initialize Variables
    _assets = assets;
    
    // Allocate the manager and the actions
    _actions = cugl::scene2::ActionManager::alloc();
    
    // Seed the random number generator to a new seed
    srand(static_cast<int>(time(NULL)));


    _didRestart = false;
    _didGoToLevelMap = false;
    _didPause = false;
    // Get Textures
    // Preload all the textures into a hashmap
    vector<string> textureVec = constants->get("textures")->asStringArray();
    for (string textureName : textureVec)
    {
        _textures.insert({textureName, _assets->get<Texture>(textureName)});
    }
    
    // Get Probablities
    // Preload all the probabilities into a hashmap
    vector<string> probabilityVec = constants->get("probability-respawn")->asStringArray();
    int probabilitySum = 0;
    for (string probabilityName : probabilityVec)
    {
        int respawnProb = boardMembers-> get("unit") -> get(probabilityName) -> get("probability-respawn") -> asInt();
        probabilitySum += respawnProb;
        _probability.insert({probabilityName, probabilitySum});
        _unitRespawnProbabilities.insert({probabilityName, static_cast<float>(respawnProb)});
    }
    
    // Set up GUI
    _guiNode = scene2::SceneNode::allocWithBounds(getSize());
    addChild(_guiNode);
    
    // Allocate Layout
    _layout = scene2::AnchoredLayout::alloc();

    // Set up top UI
    setTopUI(assets, constants);

    // Initialize state
    _currentState = SELECTING_UNIT;

    // Initialize Board
    _board = Board::alloc(_maxBoardWidth, _maxBoardHeight);

    // Create and layout the turn meter
    std::string turnMsg = strtool::format("%d/%d", _turns, 100);
    _turn_text = scene2::Label::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    _turn_text->setScale(0.5);
    _layout->addAbsolute("turn_text", cugl::scene2::Layout::Anchor::TOP_RIGHT, Vec2(-_topuibackgroundNode->getSize().width / 5, -0.43 * (_topuibackgroundNode->getSize().height)));
    _guiNode->addChildWithName(_turn_text, "turn_text");
//
//    // Create and layout unitsNeededToKill info
//    auto units_needed_to_kill = scene2::Label::allocWithText(turnMsg, assets->get<Font>("pixel32"));
    
    
//
//    // Create and layout the score meter
//
//    _scoreMeter = scene2::ProgressBar::allocWithCaps(_assets->get<Texture>("scoremeter_background"), _assets->get<Texture>("scoremeter_foreground"), _assets->get<Texture>("scoremeter_endcap_left"), _assets->get<Texture>("scoremeter_endcap_right"), Size(97, 15));
//    _scoreMeter->setProgress(0.0);
////    _scoreMeter->setPosition(Vec2(0, 0));
//    _layout->addAbsolute("scoreMeter", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(10 -_topuibackgroundNode->getSize().width / 6, -0.87 * (_topuibackgroundNode->getSize().height)));
////    _layout->addRelative("scoreMeter", cugl::scene2::Layout::Anchor::CENTER_FILL, Vec2(0, 0));
//    _guiNode->addChildWithName(_scoreMeter, "scoreMeter");
//    _scoreMeterStar1 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_empty"));
//    _scoreMeterStar1->setScale(0.15f);
//    _scoreMeterStar2 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_empty"));
//    _scoreMeterStar2->setScale(0.15f);
//    _scoreMeterStar3 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_empty"));
//    _scoreMeterStar3->setScale(0.15f);
//
//    _guiNode->addChildWithName(_scoreMeterStar1, "scoreMeterStar1");
//    _guiNode->addChildWithName(_scoreMeterStar2, "scoreMeterStar2");
//    _guiNode->addChildWithName(_scoreMeterStar3, "scoreMeterStar3");
//    std::string scoreMsg = strtool::format("%d", _score);
//    _score_text = scene2::Label::allocWithText(scoreMsg, assets->get<Font>("pixel32"));
//    _score_text->setScale(0.35);
//    _score_text->setHorizontalAlignment(HorizontalAlign::RIGHT);
//    _score_text->setAnchor(Vec2(1.0, 0.5));
//    _score_text->setForeground(Color4::WHITE);
//    _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7 + 12,
//                                                                                      _scoreMeter->getSize().height / 2 + -0.87 * (_topuibackgroundNode->getSize().height)));
////    _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_LEFT, Vec2(20, 20));
//    _guiNode->addChildWithName(_score_text, "score_text");
    

    // --------------- Set the view of the board ------------------------
    _squareSizeAdjustedForScale = _defaultSquareSize * min(_scale.width, _scale.height);
    _boardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, _maxBoardWidth * _squareSizeAdjustedForScale, _maxBoardHeight * _squareSizeAdjustedForScale));
    _orderedBoardChild = scene2::OrderedNode::allocWithOrder(cugl::scene2::OrderedNode::DESCEND);
    _layout->addRelative("boardNode", cugl::scene2::Layout::Anchor::CENTER, Vec2(0, -0.075));
    _boardNode->setTexture(_textures.at("transparent"));
    _boardNode->addChild(_orderedBoardChild);
    _board->setViewNode(_boardNode);
    _guiNode->addChildWithName(_boardNode, "boardNode");

    // --------------- Initialize units with different types --------------
    // Children will be types "basic", "three-way", etc.
    auto children = boardMembers->get("unit")->children();
    for (auto child : children)
    {
        // Get basic attack
        auto subtypeString = child->key();
        auto basicAttackJson = child->get("basic-attack")->children();
        vector<Vec2> basicAttackVec;
        for (auto basicAttack : basicAttackJson)
        {
            auto basicAttackArray = basicAttack->asFloatArray();
            basicAttackVec.push_back(Vec2(basicAttackArray.at(0), basicAttackArray.at(1)));
        }
        // Get special attack
        auto specialAttackJson = child->get("special-attack")->children();
        vector<Vec2> specialAttackVec;
        for (auto specialAttack : specialAttackJson)
        {
            auto specialAttackArray = specialAttack->asFloatArray();
            specialAttackVec.push_back(Vec2(specialAttackArray.at(0), specialAttackArray.at(1)));
        }

        // store the default color:red for this type of unit
        shared_ptr<Unit> unit = Unit::alloc(_textures, subtypeString, Unit::Color::RED, basicAttackVec, specialAttackVec, Vec2(0, -1), subtypeString != "king");
        _unitTypes.insert({child->key(), unit});
    }

    _resultLayout = assets->get<scene2::SceneNode>("result");
    _resultLayout->setContentSize(dimen);
    _resultLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_resultLayout);
    _resultLayout->setVisible(false);
    
    _settingsLayout = assets->get<scene2::SceneNode>("settings");
    _settingsLayout->setContentSize(dimen);
    _settingsLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_settingsLayout);
    
    _settingsMenuLayout = assets->get<scene2::SceneNode>("settings-menu");
    _settingsMenuLayout->setContentSize(dimen);
    _settingsMenuLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_settingsMenuLayout);
    _settingsMenuLayout->setVisible(false);
    
    _almanacLayout = assets->get<scene2::SceneNode>("almanac-menu");
//    _layout->addAbsolute("almanac-menu", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(1, 1));
    _almanacLayout->setContentSize(dimen);
    _almanacLayout->doLayout();
    _guiNode->addChild(_almanacLayout);
    _almanacLayout->setVisible(false);
    
    _score_number = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("result_board_number"));
    _level_info = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("result_board_level"));
    _star1 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star1"));
    _star2 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star2"));
    _star3 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star3"));
    _restartbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("result_board_restart"));
    _restartbutton->deactivate();
    _restartbutton->setDown(false);
    _restartbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didRestart = true;
        }
    });
    
    _settingsbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings_btn"));
    _settingsbutton->setVisible(true);
    _settingsbutton->activate();
    _settingsbutton->setDown(false);
    _settingsbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didPause = true;
        }
    });

    _scoreExplanationButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings_score-explanation-btn"));
    _scoreExplanationButton->setVisible(true);
    _scoreExplanationButton->activate();
    _scoreExplanationButton->setDown(false);
    _scoreExplanationButton->clearListeners();
    _scoreExplanationButton->addListener([this](const std::string& name, bool down) {
        if (down && _time >= TIME_BEFORE_INTERACTION) {
             _scoreExplanation->setVisible(!_scoreExplanation->isVisible());
        }
        });
    
    _scoreExplanation = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("settings_score-explanation"));
    _scoreExplanation->setScale(_scale);
    _scoreExplanation->setVisible(false);
    
    _almanacbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac"));
    //_almanacbutton->setScale(_scale);
    _almanacbutton->setVisible(true);
    _almanacbutton->setContentSize(dimen);
    _almanacbutton->doLayout();
    _almanacbutton->activate();
    _almanacbutton->setDown(false);
    _almanacbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            CULog("%u is pressed", 2);
            _didPreview = true;
        }
    });
    _guiNode->addChild(_almanacbutton);

    _settingsRestartBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_restart"));
    _settingsRestartBtn->setVisible(true);
    _settingsRestartBtn->deactivate();
    _settingsRestartBtn->setDown(false);
    _settingsRestartBtn->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didRestart = true;
            CULog("Pressed Settings restart button");
        }
    });
    
    _settingsCloseBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_close"));
    _settingsCloseBtn->setVisible(true);
    _settingsCloseBtn->deactivate();
    _settingsCloseBtn->setDown(false);
    _settingsCloseBtn->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didPause = false;
            _settingsMenuLayout->setVisible(false);
            CULog("Pressed Settings restart button");
        }
    });
    
    _almanacCloseBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_close"));
    _almanacCloseBtn->setVisible(true);
    _almanacCloseBtn->activate();
    _almanacCloseBtn->setDown(false);
    _almanacCloseBtn->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didPreview = false;
            _almanacLayout->setVisible(false);
            CULog("Pressed Settings restart button");
        }
    });
    
    viewAttackingPatterns();
    
    _settingsBackBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_exit"));
    _settingsBackBtn->setVisible(true);
    _settingsBackBtn->deactivate();
    _settingsBackBtn->setDown(false);
    _settingsBackBtn->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didGoToLevelMap = true;
            CULog("Pressed Settings exit button");
        }
    });

    _backbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("result_board_exit"));
    _backbutton->deactivate();
    _backbutton->setDown(false);
    _backbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _didGoToLevelMap = true;
        }
    });
    _kingsKilled = false;
    return true;
}

void GameScene::viewAttackingPatterns() {
    _unitPattern1 = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit1-pattern"));
    _unitPattern2 = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit2-pattern"));
    _unitPattern3 = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit3-pattern"));
    _unitPattern4 = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit4-pattern"));
    _unitPattern1->setVisible(false);
    _unitPattern2->setVisible(false);
    _unitPattern3->setVisible(false);
    _unitPattern4->setVisible(false);
    
    _unit1button = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit1"));
    _unit1button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit1-selected"));
    _unit2button = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit2"));
    _unit2button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit2-selected"));
    _unit3button = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit3"));
    _unit3button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit3-selected"));
    _unit4button = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit4"));
    _unit4button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit4-selected"));
    
    _unitButtons = {_unit1button, _unit2button, _unit3button, _unit4button};
    
    _unit1button->setVisible(true);
    _unit1button->activate();
    _unit1button->setDown(false);
    _unit1button->addListener([this](const std::string& name, bool down) {
        if (down) {
            CULog("Pressed unit1 button");
            unit1Selected = true;
            unit2Selected = false;
            unit3Selected = false;
            unit4Selected = false;
        }
    });
    
    _unit1button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit1-selected"));
    _unit1button_selected->deactivate();
    _unit1button_selected->setVisible(false);
    _unit1button_selected->setDown(false);
    _unit1button_selected->addListener([this](const std::string& name, bool down) {
        if (down) {
            CULog("Pressed unit1 button");
            unit1Selected = false;
        }
    });
 
 
    _unit2button->setVisible(true);
    _unit2button->activate();
    _unit2button->setDown(false);
    _unit2button->addListener([this](const std::string& name, bool down) {
        if (down) {
            unit2Selected = true;
            unit1Selected = false;
            unit3Selected = false;
            unit4Selected = false;
        }
    });
    
    _unit2button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit2-selected"));
    _unit2button_selected->setVisible(false);
    _unit2button_selected->deactivate();
    _unit2button_selected->setDown(false);
    _unit2button_selected->addListener([this](const std::string& name, bool down) {
        if (down) {
            CULog("Pressed unit2 button");
            unit2Selected = false;
        }
    });
    
    _unit3button->setVisible(true);
    _unit3button->activate();
    _unit3button->setDown(false);
    _unit3button->addListener([this](const std::string& name, bool down) {
        if (down) {
            CULog("Pressed unit3 button");
            unit3Selected = true;
            unit1Selected = false;
            unit2Selected = false;
            unit4Selected = false;
        }
    });
    
    _unit3button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit3-selected"));
    _unit3button_selected->setVisible(false);
    _unit3button_selected->deactivate();
    _unit3button_selected->setDown(false);
    _unit3button_selected->addListener([this](const std::string& name, bool down) {
            if (down) {
                CULog("Pressed unit3 button");
                unit3Selected = false;
            }
        });
    

    _unit4button->setVisible(true);
    _unit4button->activate();
    _unit4button->setDown(false);
    _unit4button->addListener([this](const std::string& name, bool down) {
        if (down) {
            CULog("Pressed unit4 button");
            unit4Selected = true;
            unit1Selected = false;
            unit2Selected = false;
            unit3Selected = false;
        }
    });
    
    _unit4button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit4-selected"));
        _unit4button_selected->setVisible(false);
        _unit4button_selected->deactivate();
        _unit4button_selected->setDown(false);
        _unit4button_selected->addListener([this](const std::string& name, bool down) {
            if (down) {
                unit4Selected = false;
            }
        });
}

/**
 * Set up the top UI
 *
 * UI includes: star and score system, number of swaps, setting menu, and unit icons
 */
void GameScene::setTopUI(const std::shared_ptr<cugl::AssetManager> &assets,std::shared_ptr<cugl::JsonValue> &constants) {
    // -------- Get the background image and constant values ----------
    _background = assets->get<Texture>("background-" + constants->get("themes")->asStringArray().at(0));
    _backgroundNode = scene2::PolygonNode::allocWithTexture(_background);
    _scale = getSize() / _background->getSize();
    _backgroundNode->setScale(_scale);
    
    _topuibackground = assets->get<Texture>("top-ui-background");
    _topuibackgroundNode = scene2::PolygonNode::allocWithTexture(_topuibackground);
    _scale.set(getSize().width / _topuibackground->getSize().width, getSize().width / _topuibackground->getSize().width);
    _topuibackgroundNode->setScale(_scale);

    _guiNode->addChild(_backgroundNode);
    _backgroundNode->setAnchor(Vec2::ZERO);
    _layout->addAbsolute("top_ui_background", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(0, -(_topuibackgroundNode->getSize().height/1.8)));
    _guiNode->addChildWithName(_topuibackgroundNode, "top_ui_background");
    
    // --------------- Create the star and score system -------------
    _oneStar = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("score_bar_oneStar"));
    _oneStar = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_full"));
    _twoStar1 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("score_bar_twoStar1"));
    _twoStar1 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_full"));
    _twoStar2 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("score_bar_twoStar2"));
    _twoStar2 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_full"));
    _threeStar1 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("score_bar_threeStar1"));
    _threeStar1 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_full"));
    _threeStar2 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("score_bar_threeStar2"));
    _threeStar2 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_full"));
    _threeStar3 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("score_bar_threeStar3"));
    _threeStar3 = scene2::PolygonNode::allocWithTexture(assets->get<Texture>("star_full"));
    // Resize the stars
    std::shared_ptr<cugl::scene2::PolygonNode> starSystem[6] = {_oneStar, _twoStar1, _twoStar2, _threeStar1, _threeStar2, _threeStar3};
    for (auto star : starSystem) {
        star->setScale(0.155f);
    };
    // Add stars to guiNode
    _guiNode->addChildWithName(_oneStar, "oneStar");
    _guiNode->addChildWithName(_twoStar1, "twoStar1");
    _guiNode->addChildWithName(_twoStar2, "twoStar2");
    _guiNode->addChildWithName(_threeStar1, "threeStar1");
    _guiNode->addChildWithName(_threeStar2, "threeStar2");
    _guiNode->addChildWithName(_threeStar3, "threeStar3");
    
    // Load scores
//    std::string oneStarMsg = strtool::format("%d", _level->oneStarThreshold);
    //    _score_text = scene2::Label::allocWithText(scoreMsg, assets->get<Font>("pixel32"));
    //    _score_text->setScale(0.35);
    //    _score_text->setHorizontalAlignment(HorizontalAlign::RIGHT);
    //    _score_text->setAnchor(Vec2(1.0, 0.5));
    //    _score_text->setForeground(Color4::WHITE);
    
    // --------------- Layout of the star and score system -------------
    // One star layout
    float xStartPoint = 10 -_topuibackgroundNode->getSize().width / 4.3;
    float xInterval = _topuibackgroundNode->getSize().width * 0.039;
    float yStartPoint = -0.4 * (_topuibackgroundNode->getSize().height);
    float yInterval = -0.11 * (_topuibackgroundNode->getSize().height);
    _layout->addAbsolute("oneStar", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint, yStartPoint));
//    _layout->addAbsolute("oneStar_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7, -1.275 * (_topuibackgroundNode->getSize().height)));
    // Two stars
    _layout->addAbsolute("twoStar1", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint, yStartPoint + yInterval));
    _layout->addAbsolute("twoStar2", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint + xInterval, yStartPoint + yInterval));
    // Three stars
    _layout->addAbsolute("threeStar1", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint, yStartPoint + yInterval * 2));
    _layout->addAbsolute("threeStar2", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint + xInterval, yStartPoint + yInterval * 2));
    _layout->addAbsolute("threeStar3", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint + xInterval * 2, yStartPoint + yInterval * 2));
};

/**
 * Get the pattern for a unit provided its type and color
 *
 */
std::string GameScene::getUnitType(std::string type, std::string color)
{
    return type + "-" + color;
}

std::string GameScene::generateRandomUnitType(std::map<std::string, float> probs) {
    float total = 0.0;
    for (auto const& type : probs) {
        total += type.second;
    }
    float randomChoice = total * ((double) rand() / (RAND_MAX));
    float i = 0.0;
    for (auto const& type : probs) {
        if (i + type.second >= randomChoice) return type.first;
        i += type.second;
    }
    return ""; // Unreachable
}

Unit::Color GameScene::generateRandomUnitColor(std::map<Unit::Color, float> probs) {
    float total = 0.0;
    for (auto const& type : probs) {
        total += type.second;
    }
    float randomChoice = total * ((double) rand() / (RAND_MAX));
    float i = 0.0;
    for (auto const& type : probs) {
        if (i + type.second >= randomChoice) return type.first;
        i += type.second;
    }
    return Unit::Color(0); // Unreachable
}

Vec2 GameScene::generateRandomDirection() {
    vector<cugl::Vec2> allDirs = Unit::getAllPossibleDirections();
    return allDirs[rand() % allDirs.size()];
}

/**
 * sets the correct square texture for a square based on the unit on it.
 * @param square         the square being set
 * @param textures       the texture map being used
 */
void GameScene::updateSquareTexture(shared_ptr<Square> square)
{
    Vec2 squarePos = square->getPosition();
    string sqTexture;
    shared_ptr<Unit> currentUnit = square->getUnit();
    string currentDirection = Unit::directionToString(currentUnit->getDirection());
    if (_currentReplacementDepth[_board->flattenPos(square->getPosition().x, square->getPosition().y)] + 1 >= _level->maxTurns) {
        if (currentUnit->isSpecial() && currentUnit->getSubType() != "king" && currentUnit->getSubType() != "empty") sqTexture = "special_" + currentDirection + "_square";
        else sqTexture = "square-" + _level->backgroundName;
        square->getViewNode()->setTexture(_textures.at(sqTexture));
        return;
    }
    shared_ptr<Unit> replacementUnit = _level->getBoard(_currentReplacementDepth[_board->flattenPos(square->getPosition().x, square->getPosition().y)] + 1)->getSquare(square->getPosition())->getUnit();
    std::string color = Unit::colorToString(replacementUnit->getColor());
    string replacementDirection = Unit::directionToString(replacementUnit->getDirection());
    
    if (currentUnit->isSpecial() && (replacementUnit->getSubType() == "basic" || replacementUnit->getSubType() == "random"))
    {
        sqTexture = "special_" + currentDirection + "_square";
    }
    else if (currentUnit->isSpecial() && replacementUnit->getSubType() != "basic" && replacementUnit->getSubType() != "random")
    {
        sqTexture = "arrow-" + currentDirection;
    }
    else if (!currentUnit->isSpecial() && replacementUnit->getSubType() != "basic" && replacementUnit->getSubType() != "random")
    {
        sqTexture = "square-" + replacementDirection + "-" + color;
        // sqTexture = "next-special-square";
    }
    else
    {
        sqTexture = "square-" + _level->backgroundName;
    }
    square->getViewNode()->setTexture(_textures.at(sqTexture));
}

void GameScene::refreshUnitAndSquareView(shared_ptr<Square> sq) {
    refreshUnitView(sq);
    updateSquareTexture(sq);
}

void GameScene::refreshUnitView(shared_ptr<Square> sq) {
    auto unit = sq->getUnit();
    auto unitNode = unit->getViewNode();
    std::string unitSubtype = unit->getSubType();
    if (unitSubtype != "basic") unit->setSpecial(true);
    else unit->setSpecial(false);
    sq->getViewNode()->removeAllChildren();
    if (unitNode->getParent() == NULL) sq->getViewNode()->addChild(unitNode);
    if (_debug) unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
}

/**
 * Generate a unit on the given square.
 *
 * @param sq    The given square pointer
 * @param unitType  The type of unit to generate
 * @param color  The color of the unit to generate
 * @param dir  The direction of the unit to generate
 */
void GameScene::generateUnit(shared_ptr<Square> sq, std::string unitType, Unit::Color color, Vec2 dir, int numberOfUnitsNeededToKill)
{
    // Set Unit Information
    auto unit = sq->getUnit();
    unit->setDirection(dir);
    unit->setColor(color);
    auto unitSelected = _unitTypes[unitType];
    unit->setSubType(unitSelected->getSubType());
    unit->setBasicAttack(unitSelected->getBasicAttack());
    unit->setSpecialAttack(unitSelected->getSpecialAttack());
    unit->setSpecial(unitType != "basic");
    unit->setUnitsNeededToKill(numberOfUnitsNeededToKill);
    unit->setState(Unit::State::RESPAWNING);
    unit->setHasBeenHit(false);
    refreshUnitView(sq);
}

std::map<Unit::Color, float> GameScene::generateColorProbabilities() {
    std::map<Unit::Color, float> probs = {
        { Unit::Color(0), 0.0 },
        { Unit::Color(1), 0.0 },
        { Unit::Color(2), 0.0 }
    };
    std::map<Unit::Color, int> colorCounts = {
        { Unit::Color(0), 0 },
        { Unit::Color(1), 0 },
        { Unit::Color(2), 0 }
    };
    int totalUnitCount = 0;
    vector<std::shared_ptr<Square>> allSquares = _board->getAllSquares();
    for (auto sq : allSquares) {
        std::shared_ptr<Unit> unit = sq->getUnit();
        Unit::Color unitColor = unit->getColor();
        colorCounts.at(unitColor) = colorCounts.at(unitColor) + 1;
        totalUnitCount++;
    }
    float cumulativeProb = 0.0;
    for (auto const& color : colorCounts) {
        float inverseProbability = static_cast<float>(totalUnitCount) / color.second;
        colorCounts.at(color.first) = inverseProbability;
        cumulativeProb += inverseProbability;
    }
    for (auto const& color : probs) {
        probs.at(color.first) = colorCounts.at(color.first) / cumulativeProb;
    }
    return probs;
}

void GameScene::deconfirmSwap() {
    _upcomingUnitNode->setVisible(false);
    _upcomingUnitNode->removeAllChildren();

    _currentState = SELECTING_SWAP;
    // If we are de-confirming a swap, we must undo the swap.
    _board->switchUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
    _selectedSquare->getUnit()->setDirection(_selectedSquareOriginalDirection);
    //_swappingSquare->getUnit()->setDirection(_swappingSquareOriginalDirection);
    for (shared_ptr<Square> square : _board->getAllSquares())
    {
        updateSquareTexture(square);
    }
    _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
}


/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose()
{
    if (_active)
    {
        removeAllChildren();
        _restartbutton = nullptr;
        _backbutton = nullptr;
        _active = false;
        _input.dispose();
        _moveup = nullptr;
        _movedn = nullptr;
        _moveleft = nullptr;
        _moveright = nullptr;
        Scene2::dispose();
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
void GameScene::update(float timestep)
{
    //if (_audioQueue->getState() == cugl::AudioEngine::State::INACTIVE) {
      //  _audioQueue->play(_assets->get<Sound>("track_1"), false, .3, false);
    //}
//    _audioQueue->setLoop(true);
    if (_time <= TIME_BEFORE_INTERACTION) {
        _time += timestep;
    }
    
    // Read the keyboard for each controller.
    // Read the input
    _input.update();
    

    if (_didRestart == true) reset(_levelJson);
    if (_didRestart == true && _didPause == true) reset(_levelJson);

    if ((_turns == 0 || _kingsKilled) && _currentState != ANIMATION )
    {
        // Show results screen
        showResultText(_kingsKilled && _level->getNumberOfStars(_score) >= 3, _guiNode);
        _resultLayout->setVisible(true);
        _restartbutton->activate();
        _backbutton->activate();
        _scoreExplanationButton->deactivate();
        _score_number->setText(to_string(_score));
        _level_info->setText("Level " + to_string(_levelJson->getInt("id")));
        _star1->setTexture(_textures.at("star_empty"));
        _star2->setTexture(_textures.at("star_empty"));
        _star3->setTexture(_textures.at("star_empty"));
        if (_level->getNumberOfStars(_score) >= 1) {
            _star1->setTexture(_textures.at("star_full"));
        }
        if (_level->getNumberOfStars(_score) >= 2) {
            _star2->setTexture(_textures.at("star_full"));
        }
        if (_level->getNumberOfStars(_score) >= 3) {
            _star3->setTexture(_textures.at("star_full"));
        }
        return;
    }
    
    if (_didPause == true) {
        // Show settings screen
        _settingsMenuLayout->setVisible(true);
        _settingsBackBtn->activate();
        _settingsRestartBtn->activate();
        _settingsCloseBtn->activate();
        return;
    }
    
    if (_didPreview == true) {
        // show attacking patterns preview
        _almanacLayout->setVisible(true);
    }
    
    if (unit1Selected == true) {
        _unit1button->setVisible(false);
        _unit1button->deactivate();
        _unit1button_selected->setVisible(true);
        _unit1button_selected->activate();
        _unitPattern1->setVisible(true);
        _unitPattern2->setVisible(false);
        _unitPattern3->setVisible(false);
        _unitPattern4->setVisible(false);
    }
    
    if (unit1Selected == false) {
        _unit1button_selected->setVisible(false);
        _unit1button_selected->deactivate();
        _unit1button->setVisible(true);
        _unit1button->activate();
    }
    
    if (unit2Selected == true) {
        _unit2button->setVisible(false);
        _unit2button->deactivate();
        _unit2button_selected->setVisible(true);
        _unit2button_selected->activate();
        _unitPattern2->setVisible(true);
        _unitPattern1->setVisible(false);
        _unitPattern3->setVisible(false);
        _unitPattern4->setVisible(false);
//        unit1Selected = false;
//        unit3Selected = false;
//        unit4Selected = false;
    }
    
    if (unit2Selected == false) {
        _unit2button_selected->setVisible(false);
        _unit2button_selected->deactivate();
        _unit2button->setVisible(true);
        _unit2button->activate();
    }
    
    if (unit3Selected == true) {
            _unit3button->setVisible(false);
            _unit3button->deactivate();
            _unit3button_selected->setVisible(true);
            _unit3button_selected->activate();
            _unitPattern3->setVisible(true);
            _unitPattern1->setVisible(false);
            _unitPattern2->setVisible(false);
            _unitPattern4->setVisible(false);
//            unit1Selected = false;
//            unit2Selected = false;
//            unit4Selected = false;
        }
    
    if (unit3Selected == false) {
            _unit3button_selected->setVisible(false);
            _unit3button_selected->deactivate();
            _unit3button->setVisible(true);
            _unit3button->activate();
    }
    
    if (unit4Selected == true) {
            _unit4button->setVisible(false);
            _unit4button->deactivate();
            _unit4button_selected->setVisible(true);
            _unit4button_selected->activate();
            _unitPattern4->setVisible(true);
            _unitPattern1->setVisible(false);
            _unitPattern2->setVisible(false);
            _unitPattern3->setVisible(false);
//            unit1Selected = false;
//            unit2Selected = false;
//            unit3Selected = false;
        }
    
    if (unit4Selected == false) {
            _unit4button_selected->setVisible(false);
            _unit4button_selected->deactivate();
            _unit4button->setVisible(true);
            _unit4button->activate();
        }
    

    Vec2 pos = _input.getPosition();
    Vec3 worldCoordinate = screenToWorldCoords(pos);
    Vec2 boardPos = _boardNode->worldToNodeCoords(worldCoordinate);
    Vec2 squarePos = Vec2(int(boardPos.x / (_squareSizeAdjustedForScale)), int(boardPos.y / (_squareSizeAdjustedForScale)));
    if (_input.isDebugDown())
    {
        _debug = !_debug;
    }

//    if (_currentState == State::ANIMATION) {
//        bool attackSequenceDone = true;
//        for (auto sq : _attackedSquares) {
//            if (sq->getUnit()->getState() != Unit::State::IDLE) {
//                attackSequenceDone = false;
//                break;
//            }
//        }
//        if (attackSequenceDone) _currentState = State::SELECTING_UNIT;
//    }
//    else if (_input.isDown() && _currentState != ANIMATION)

    if (_input.isDown() && _currentState != ANIMATION)
    {
        if (_board->doesSqaureExist(squarePos) && boardPos.x >= 0 && boardPos.y >= 0 && _board->getSquare(squarePos)->isInteractable())
        {
            auto squareOnMouse = _board->getSquare(squarePos);
            if (_currentState == SELECTING_UNIT && squareOnMouse->getUnit()->getSubType() != "king")
            {
                _selectedSquare = squareOnMouse;
                _selectedSquare->getViewNode()->setTexture(_textures.at("square-selected"));
                _currentState = SELECTING_SWAP;
                
                // Restore the size of the previous enlarged unitNode
                if (_enlargedUnitNode != NULL){
                    _enlargedUnitNode->setScale(BACK2NORMAL);
                }
                
                // Enlarge the new selected unit
                _enlargedUnitNode = _selectedSquare->getUnit()->getViewNode();
                _enlargedUnitNode->setScale(ENLARGE);
                
                std::shared_ptr<Square> replacementSquare = _selectedSquare == NULL ? NULL : _level->getBoard(_currentReplacementDepth[_board->flattenPos(_selectedSquare->getPosition().x, _selectedSquare->getPosition().y)] + 1)->getSquare(_selectedSquare->getPosition());
                auto upcomingUnitType = replacementSquare->getUnit()->getSubType();
                auto upcomingUnitColor = Unit::colorToString(replacementSquare->getUnit()->getColor());
                auto upcomingUnitDirection =Unit::directionToString(replacementSquare->getUnit()->getDirection());
                auto upcomingSquarePos = _selectedSquare->getViewNode()->getPosition();
                if (upcomingUnitType != "basic" && upcomingUnitType != "random" && upcomingUnitType != "empty") {
                    _upcomingUnitNode->setPosition(upcomingSquarePos + Vec2(0, _squareSizeAdjustedForScale));
                    _upcomingUnitNode->setScale((float)_squareSizeAdjustedForScale / (float)_defaultSquareSize);
                    auto upcomingDirectionNode = scene2::PolygonNode::allocWithTexture(_textures.at("special_"+upcomingUnitDirection+"_square"));
                    upcomingDirectionNode->setPosition(Vec2(81+_squareSizeAdjustedForScale/2,81+_squareSizeAdjustedForScale));
                    upcomingDirectionNode->setScale(0.8f);
                    _upcomingUnitNode->addChild(upcomingDirectionNode);
                    auto upcomingTextureNode = scene2::PolygonNode::allocWithTexture(_textures.at(upcomingUnitType + "-" + upcomingUnitColor));
                    upcomingTextureNode->setScale(0.8f);
                    upcomingTextureNode->setPosition(Vec2(81+_squareSizeAdjustedForScale/2,81+_squareSizeAdjustedForScale));
                    _upcomingUnitNode->addChild(upcomingTextureNode);
                    _upcomingUnitNode->setVisible(true);
                }
            }
            else if (_currentState == SELECTING_SWAP && squareOnMouse->getPosition().distance(_selectedSquare->getPosition()) == 1 && _selectedSquare->getUnit()->isMoveable())
            {
                if (!squareOnMouse->getUnit()->isMoveable()) return;
                _upcomingUnitNode->setVisible(false);
                _upcomingUnitNode->removeAllChildren();

                _currentState = CONFIRM_SWAP;
                _swappingSquare = squareOnMouse;
                
                // Rotation and Swapping of Model
                _selectedSquareOriginalDirection = _selectedSquare->getUnit()->getDirection();
                _swappingSquareOriginalDirection = _swappingSquare->getUnit()->getDirection();
                // We do this so that we can show the attack preview, without changing the actual positional view of units.
//                _board->switchUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                _board->switchAndRotateUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
                squareOnMouse->getViewNode()->setTexture(_textures.at("square-swap"));
                _swappingSquare->getUnit()->setDirection(_swappingSquare->getPosition() - _selectedSquare->getPosition());
                _attackedSquares = _board->getAttackedSquares(_swappingSquare->getPosition());
                _protectedSquares = _board->getProtectedSquares(_swappingSquare->getPosition());

                for (shared_ptr<Square> attackedSquare : _attackedSquares)
                {
                    attackedSquare->getViewNode()->setTexture(_textures.at("square-attacked"));
                }
                for (shared_ptr<Square> protectedSquare : _protectedSquares)
                {
                    protectedSquare->getViewNode()->setTexture(_textures.at("shield"));
                }
            }
            else if (_currentState == CONFIRM_SWAP && squareOnMouse != _swappingSquare)
            {
                deconfirmSwap();
            }
        } else if (_currentState == CONFIRM_SWAP) {
            deconfirmSwap();
        }
        
    }
    else if (_input.didRelease() && _currentState != ANIMATION)
    {
        _upcomingUnitNode->setVisible(false);
        _upcomingUnitNode->removeAllChildren();
        if (_enlargedUnitNode) _enlargedUnitNode->setScale(BACK2NORMAL);

        for (shared_ptr<Square> square : _board->getAllSquares())
        {
            updateSquareTexture(square);
        }

        if (_board->doesSqaureExist(squarePos) && boardPos.x >= 0 && boardPos.y >= 0 && _board->getSquare(squarePos)->isInteractable() && _currentState == CONFIRM_SWAP)
        {
            _currentState = ANIMATION;
            _midSwap = true;
            
            
            // Undo the swap for the animation
//            _board->switchAndRotateUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
            _board->switchUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
            
            // Animation
            auto animationNodeSWA = _swappingSquare->getUnit()->getViewNode();
            auto animationNodeSWB = _selectedSquare->getUnit()->getViewNode();
            string direction = moveDirection(_swappingSquare, _selectedSquare);
            if (direction == "up"){
                doMove("swapA", animationNodeSWA,_moveup);
                doMove("swapB", animationNodeSWB,_movedn);
            } else if (direction == "down"){
                doMove("swapA", animationNodeSWA, _movedn);
                doMove("swapB", animationNodeSWB,_moveup);
            } else if (direction == "left"){
                doMove("swapA", animationNodeSWA,_moveleft);
                doMove("swapB", animationNodeSWB,_moveright);
            } else {
                doMove("swapA", animationNodeSWA, _moveright);
                doMove("swapB", animationNodeSWB,_moveleft);
            }
            _initalAttackSquare = _swappingSquare;
            _turns--;
        } else {
            _currentState = SELECTING_UNIT;
        }
    }
    
    if (_currentState == ANIMATION) {
        bool completedAllAnimations = true;
        bool completedAttackSequence = true;
        bool respawning = false;
        bool completedSwap = true;
//        vector<std::shared_ptr<Square>> deadSquares;
        if (_actions->isActive("swapA")) completedSwap = false;
        if (_actions->isActive("swapB")) completedSwap = false;
        
        if (completedSwap) {
            if (_midSwap) updateModelPostSwap();
        }
        
        for (auto square : _board->getAllSquares()) {
            auto unit = square->getUnit();
            auto unitState = unit->getState();
            bool isKing = unit->getSubType() == "king";
            if (!isKing && unitState != Unit::State::IDLE) {
                completedAllAnimations = false;
                if (unitState != Unit::State::DEAD && unitState != Unit::State::RESPAWNING) completedAttackSequence = false;
                if (unitState == Unit::State::RESPAWNING) respawning = true;
//                else deadSquares.push_back(square);
            } else if (isKing && unitState != Unit::IDLE && unitState != Unit::DEAD) {
                completedAttackSequence = false;
            }
            updateSquareTexture(square);
        }
        
        if (completedSwap && completedAttackSequence) {
            if (completedAllAnimations && !_kingsKilled) _currentState = SELECTING_UNIT;
            else if (!respawning) respawnAttackedSquares();
        }
        
    }
    
    

    // Update the score meter
//    if (_score >= _level->oneStarThreshold) _scoreMeterStar1->setTexture(_assets->get<Texture>("star_full"));
//    if (_score >= _level->twoStarThreshold) _scoreMeterStar2->setTexture(_assets->get<Texture>("star_full"));
//    if (_score >= _level->threeStarThreshold) _scoreMeterStar3->setTexture(_assets->get<Texture>("star_full"));
//    if (_score < _level->threeStarThreshold) {
//        _scoreMeter->setProgress(static_cast<float>(_score) / _level->threeStarThreshold);
//        _layout->remove("score_text");
//        _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7 + 12 + (static_cast<float>(_score) / _level->threeStarThreshold) * (_scoreMeter->getWidth() - 14),
//                                                                                          _scoreMeter->getSize().height / 2 + -0.87 * (_topuibackgroundNode->getSize().height)));
//    } else {
//        _scoreMeter->setProgress(1.0f);
//        _layout->remove("score_text");
//        _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7 + (_scoreMeter->getWidth() - 2),
//                                                                                          _scoreMeter->getSize().height / 2 + -0.87 * (_topuibackgroundNode->getSize().height)));
//    }
//    _score_text->setText(strtool::format("%d", _score), true);
//    if ((_prev_score < 9 && _score > 9) || (_prev_score < 99 && _score > 99))
//    {
//        _layout->remove("score_text");
//        _layout->addAbsolute("score_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(-_topuibackgroundNode->getSize().width / 7, -0.85 * (_topuibackgroundNode->getSize().height)));
//    }
    // Update the remaining turns
    _turn_text->setText(strtool::format("%d/%d", _turns, _level->maxTurns));
    
    // Animate all units
    for (shared_ptr<Square> square : _board->getAllSquares())
    {
        auto unit = square->getUnit();
        auto unitState = unit->getState();
        std::string unitType = unit->getSubType();
        if (unit->completedAnimation) {
            switch (unitState) {
                case Unit::State::IDLE:
                    break;
                case Unit::State::DEAD:
//                    square->getViewNode()->setVisible(false);
                    break;
                case Unit::State::PROTECTED:
                    unit->setState(Unit::State::IDLE);
                    refreshUnitView(square);
                    break;
                case Unit::State::HIT:
                    if (unitType != "basic" && unitType != "empty" && unitType != "king") {
                        unit->setState(Unit::State::ATTACKING);
                        refreshUnitView(square);
                    } else {
                        unit->setState(Unit::State::DYING);
                        refreshUnitView(square);
                    }
                    break;
                case Unit::State::ATTACKING:
                    for (auto atkSquare : _board->getInitallyAttackedSquares(square->getPosition(), unit == _initalAttackSquare->getUnit())) {
                        if (atkSquare->getUnit()->getState() == Unit::State::IDLE) {
//                            if (_attackedSquares.size() >= atkSquare->getUnit()->getUnitsNeededToKill()) {
                                atkSquare->getUnit()->setState(Unit::State::HIT);
//                            }
                        }
                        refreshUnitView(atkSquare);
                    }
                    for (auto ptdSquare : _board->getInitiallyProtectedSquares(square->getPosition(), unit == _initalAttackSquare->getUnit())) {
                        if (ptdSquare->getUnit()->getState() == Unit::State::IDLE){
                            ptdSquare->getUnit()->setState(Unit::State::PROTECTED);
                        }
                        refreshUnitView(ptdSquare);
                    }
                    if (unit->hasBeenHit()) unit->setState(Unit::State::DYING);
                    else unit->setState(Unit::State::IDLE);
                    refreshUnitView(square);
                    break;
                case Unit::State::DYING:
                    unit->setState(Unit::State::DEAD);
                    if (unit->getSubType() == "king") _kingsKilled = true;
//                    unit->getViewNode()->setVisible(false);
                    refreshUnitView(square);
                    break;
                case Unit::State::RESPAWNING:
                    unit->setState(Unit::State::IDLE);
                    refreshUnitView(square);
                    break;
                default:
                    break;
            }
        }
//        refreshUnitAndSquareView(square);
//        refreshUnitView(square);
        square->getUnit()->update(timestep);
    }

    // Layout everything
    _layout->layout(_guiNode.get());
    
    // Animate
     _actions->update(timestep);
}

void GameScene::updateModelPostSwap() {
    _midSwap = false;
    
    _board->switchAndRotateUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
    //  Because the units in the model where already swapped.
    float inverseSquareFactor = 1 / _squareScaleFactor;
    auto swappedUnitNode = _selectedSquare->getUnit()->getViewNode();
    auto selectedUnitNode = _swappingSquare->getUnit()->getViewNode();
    swappedUnitNode->setPosition(Vec2::ONE * inverseSquareFactor * (_squareSizeAdjustedForScale / 2));
    selectedUnitNode->setPosition(Vec2::ONE * inverseSquareFactor * (_squareSizeAdjustedForScale / 2));
//    swappedUnitNode->setPosition(Vec2::ONE * (200));
//    selectedUnitNode->setPosition(Vec2::ONE * (_squareSizeAdjustedForScale / 2));
    // Updating View
    _selectedSquare->getViewNode()->removeChild(selectedUnitNode);
    _swappingSquare->getViewNode()->removeChild(swappedUnitNode);
    _selectedSquare->getViewNode()->addChild(swappedUnitNode);
    _swappingSquare->getViewNode()->addChild(selectedUnitNode);

    if (_attackedSquares.size() > 0) {
        _initalAttackSquare->getUnit()->setState(Unit::State::ATTACKING); // begin the attack sequence
        refreshUnitView(_initalAttackSquare);
    }
    else{ // need to show shield animation even if no unit is killed in this swap
        for (auto ptdSquare: _protectedSquares){
            ptdSquare->getUnit()->setState(Unit::State::PROTECTED);
            refreshUnitView(ptdSquare);
        }
    }

}

void GameScene::respawnAttackedSquares() {
//    _prevScore = _score;
    // remove the attacked squares
    int scoreNum = 0;
    bool plusScore = false;
    for (shared_ptr<Square> attackedSquare : _attackedSquares)
    {
        if (attackedSquare->getUnit()->getSubType() == "king" && _kingsKilled) {
////            _kingsKilled = true;
            plusScore = true;
        }
        auto attacked_unit = attackedSquare->getUnit();
        if (_attackedSquares.size() < attackedSquare->getUnit()->getUnitsNeededToKill()) {
            continue;
        }

//                if (attacked_unit->getSubType()=="king") {
//                    std::string unitsNeededToKill = strtool::format("%d/%d",_attackedSquares.size(),attacked_unit->getUnitsNeededToKill());
//                    _info_text->setText(unitsNeededToKill);
//                }

        // Replace Unit
        if (attackedSquare->getUnit()->getSubType() != "king") replaceUnitOnSquare(attackedSquare);
        scoreNum++;
    }
    if (plusScore) {
        _score += scoreNum;
    }
    if (_kingsKilled) _currentState = SELECTING_UNIT;
}

void GameScene::replaceUnitOnSquare(shared_ptr<Square> sq) {
    std::map<Unit::Color, float> colorProbabilities = generateColorProbabilities();
    Vec2 squarePos = sq->getPosition();
    _currentReplacementDepth[_board->flattenPos(squarePos.x, squarePos.y)]++;
    int currentReplacementDepth = _currentReplacementDepth[_board->flattenPos(squarePos.x, squarePos.y)];
    std::shared_ptr<Square> replacementSquare = _level->getBoard(currentReplacementDepth)->getSquare(squarePos);
    auto unitSubType = replacementSquare->getUnit()->getSubType();
    auto unitColor = unitSubType == "random" ? generateRandomUnitColor(colorProbabilities) : replacementSquare->getUnit()->getColor();
    auto unitDirection = unitSubType == "random" ? generateRandomDirection() : replacementSquare->getUnit()->getDirection();
    if (unitSubType == "random") unitSubType = generateRandomUnitType(_unitRespawnProbabilities);
    std:string unitPattern = getUnitType(unitSubType, unitColor);
    generateUnit(sq, unitSubType, unitColor, unitDirection, replacementSquare->getUnit()->getUnitsNeededToKill());

    // Set empty squares to be uninteractable.
    sq->setInteractable(unitSubType != "empty");
    sq->getViewNode()->setVisible(unitSubType != "empty");
//    refreshUnitAndSquareView(sq);
    if (unitSubType == "king") loadKingUI(replacementSquare->getUnit()->getUnitsNeededToKill(), replacementSquare->getUnit()->getUnitsNeededToKill(), squarePos, replacementSquare->getUnit()->getViewNode());
}

/**
 * Performs a move action
 *
 * @param action The move action
 */
void GameScene::doMove(std::string act_key, shared_ptr<cugl::scene2::PolygonNode> viewNode,const std::shared_ptr<cugl::scene2::MoveBy>& action) {
    if (!viewNode->isVisible()) {
        CULog("Not Visible");
        return;
    }
    if (_actions->isActive(act_key)) {
        CULog("You must wait for the animation to complete first");
    } else {
        auto fcn = EasingFunction::alloc(EasingFunction::Type::LINEAR);
        _actions->activate(act_key, action, viewNode, fcn);
        CULog("doMove is called");
    }
}

/**
 * Check the direction that the selectedSq is moving to
 *
 * @param the selected square
 * @param the square will be swapped with the selected square
 *
 * @return the direction the selected square is moving to
 */
string GameScene::moveDirection(shared_ptr<Square> selectedSq, shared_ptr<Square> swappingSq) {
    Vec2 selectedPos = selectedSq -> getPosition();
    int x1 = (int)selectedPos.x;
    int y1 = (int)selectedPos.y;
    
    Vec2 swappingPos = swappingSq -> getPosition();
    int x2 = (int)swappingPos.x;
    int y2 = (int)swappingPos.y;
    
    string direction;
    if (x1 == x2 && y1 != y2) {
        if (y1 - y2 == 1){
            direction = "down";
        } else if (y1 - y2 == -1) {
            direction = "up";
        }
    } else if (y1 == y2 && x1 != x2) {
        if (x1 - x2 == 1) {
            direction = "left";
        } else if (x1 - x2 == -1) {
            direction = "right";
        }
    }
    return direction;
}

#pragma mark -
#pragma mark Rendering
/**
 * Draws all this scene to the given SpriteBatch.
 *
 * The default implementation of this method simply draws the scene graph
 * to the sprite batch.  By overriding it, you can do custom drawing
 * in its place.
 *
 * @param batch     The SpriteBatch to draw with.
 */
void GameScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch)
{
    // For now we render 3152-style
    // DO NOT DO THIS IN YOUR FINAL GAME
    batch->begin(getCamera()->getCombined());
    batch->setColor(Color4::RED);
    //_info_text->render(batch);
    _guiNode->render(batch);

    if (_debug)
    {
        batch->setColor(Color4::RED);
        std::shared_ptr<Square> replacementSquare = _selectedSquare == NULL ? NULL : _level->getBoard(_currentReplacementDepth[_board->flattenPos(_selectedSquare->getPosition().x, _selectedSquare->getPosition().y)] + 1)->getSquare(_selectedSquare->getPosition());
        std::string unitType = _selectedSquare == NULL ? "Unit Type: " : "Unit Type: " + _selectedSquare->getUnit()->getSubType();
        std::string unitColor = _selectedSquare == NULL ? "Unit Color: " : "Unit Color: " + Unit::colorToString(_selectedSquare->getUnit()->getColor());
        std::string unitIsMoveable = _selectedSquare == NULL ? "Unit Moveable: " : (_selectedSquare->getUnit()->isMoveable() ? "true" : "false");
        std::string squareIsInteractable = _selectedSquare == NULL ? "Interactable: " : (_selectedSquare->isInteractable() ? "true" : "false");
        std::string replacementType = replacementSquare == NULL ? "" : replacementSquare->getUnit()->getSubType();
        std::string replacementColor = replacementSquare == NULL ? "" : Unit::colorToString(replacementSquare->getUnit()->getColor());
        auto direction = _selectedSquare == NULL ? Vec2::ZERO : _selectedSquare->getUnit()->getDirection();
        auto replacementDirection = replacementSquare == NULL ? Vec2::ZERO : replacementSquare->getUnit()->getDirection();
        std::ostringstream unitDirection;
        std::ostringstream replacementUnitDirection;
        unitDirection << "Unit Direction: (" << int(direction.x) << ", " << int(direction.y) << ")";
        replacementUnitDirection << "(" << int(replacementDirection.x) << ", " << int(replacementDirection.y) << ")";
        batch->drawText(unitType, _turn_text->getFont(), Vec2(50, getSize().height - 100));
        batch->drawText(unitColor, _turn_text->getFont(), Vec2(50, getSize().height - 150));
        batch->drawText(unitDirection.str(), _turn_text->getFont(), Vec2(50, getSize().height - 200));
//        batch->drawText(unitIsMoveable, _turn_text->getFont(), Vec2(50, getSize().height - 250));
        batch->drawText(squareIsInteractable, _turn_text->getFont(), Vec2(50, getSize().height - 300));
//        std::string spe = _selectedSquare == NULL ? "" : _selectedSquare->getUnit()->isSpecial() ? "isSpecial() = true"
//                                                                                                 : "isSpecial() = false";
        std::string spe = "";
        if (_selectedSquare != NULL) {
            switch (_selectedSquare->getUnit()->getState()){
                case Unit::IDLE:
                    spe = "Idle";
                    break;
                case Unit::HIT:
                    spe = "Hit";
                    break;
                case Unit::ATTACKING:
                    spe = "Attacking";
                    break;
                case Unit::DYING:
                    spe = "Dying";
                    break;
                default:
                    spe = "Unknown state";
                    break;
            }
        }
        
        batch->drawText(spe, _turn_text->getFont(), Vec2(50, getSize().height - 250));
//        batch->drawText(replacementType, _turn_text->getFont(), Vec2(50, getSize().height - 300));
//        batch->drawText(replacementColor, _turn_text->getFont(), Vec2(50, getSize().height - 350));
//        batch->drawText(replacementUnitDirection.str(), _turn_text->getFont(), Vec2(50, getSize().height - 400));
        
    }

    batch->end();
}
/**
 * Resets the status of the game so that we can play again.
 */
void GameScene::reset()
{
    reset(_levelJson);
}

/**
 * Resets the status of the game with the board JSON.
 */
void GameScene::reset(shared_ptr<cugl::JsonValue> boardJSON)
{
    //    _endgame_text->setForeground(Color4::CLEAR);
    //    _turns = _boardJson->getInt("total-swap-allowed");
    //    _score = 0;
    removeChild(_guiNode);
    _didGoToLevelMap = false;
    _didPause = false;
    init(_assets);
    setLevel(boardJSON);
}

/**
 * Sets whether the scene is currently active
 *
 * @param value whether the scene is currently active
 */
void GameScene::setActive(bool value)
{
    _active = value;
//        if (value && ! _restartbutton->isActive()) {
//            _restartbutton->activate();
//        } else if (!value && _restartbutton->isActive()) {
//            _restartbutton->deactivate();
//        }
}

void GameScene::setLevel(shared_ptr<cugl::JsonValue> levelJSON) {
    _levelJson = levelJSON;
    _level = Level::alloc(_textures, levelJSON);
    vector<int> vector(_level->getNumberOfRows() * _level->getNumberOfColumns(), 0);
    _currentReplacementDepth = vector;
    _score = 0;
    _turns = _level->maxTurns;

    // Change Background
    _backgroundNode->setTexture(_textures.at("background-" + _levelJson->getString("background")));

    map<Unit::Color, float> startingColorProbabilities = {
        { Unit::Color(0), 0.33 },
        { Unit::Color(1), 0.33 },
        { Unit::Color(2), 0.33 }
    };
    // Create the squares & units and put them in the map
    _board->getViewNode()->getChild(0)->removeAllChildren();
    _board = Board::alloc(_level->getNumberOfColumns(),_level->getNumberOfRows());
    _board->setViewNode(_boardNode);
    // Set the view of the board.
    _squareSizeAdjustedForScale = _defaultSquareSize * min(_scale.width, _scale.height) * (min((float)_maxBoardHeight / (float)_level->getNumberOfRows(), (float)_maxBoardWidth / (float)_level->getNumberOfColumns()));
    _squareScaleFactor = (float)_squareSizeAdjustedForScale / (float)_defaultSquareSize;
    _boardNode->setPolygon(Rect(0, 0, _level->getNumberOfColumns() * _squareSizeAdjustedForScale, _level->getNumberOfRows() * _squareSizeAdjustedForScale));
    _boardNode->setTexture(_textures.at("transparent"));
    for (int i = 0; i < _level->getNumberOfColumns(); i++) {
        for (int j = 0; j < _level->getNumberOfRows(); ++j) {
            auto test = "square-" + _level->backgroundName;
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square-" + _level->backgroundName));
            auto squarePosition = Vec2(i, j);
            squareNode->setPosition((Vec2(squarePosition.x, squarePosition.y) * _squareSizeAdjustedForScale) + Vec2::ONE * (_squareSizeAdjustedForScale / 2));
            squareNode->setScale(_squareScaleFactor);
            squareNode->setPriority(SQUARE_Z);
            shared_ptr<Square> sq = _board->getSquare(squarePosition);
            sq->setViewNode(squareNode);
            _board->getViewNode()->getChild(0)->addChild(squareNode);
            // Generate unit for this square
            auto unit = _level->getBoard(0)->getSquare(squarePosition)->getUnit();
            auto unitSubType = unit->getSubType();
            auto unitColor = Unit::colorToString(unit->getColor());
            Vec2 unitDirection = unit->getDirection();
            
            if (unitSubType == "random") {
                unitSubType = generateRandomUnitType(_unitRespawnProbabilities);
                unitColor = Unit::colorToString(generateRandomUnitColor(startingColorProbabilities));
                unitDirection = generateRandomDirection();
            }
            sq->setInteractable(unitSubType != "empty");
            sq->getViewNode()->setVisible(unitSubType != "empty");
            std::string unitPattern = getUnitType(unitSubType, unitColor);
            std::string unitIdleTextureName = unitSubType + "-idle-" + unitColor;
            auto unitTemplate = _unitTypes.at(unitSubType);
            Unit::Color c = Unit::stringToColor(unitColor);
            shared_ptr<Unit> newUnit = Unit::alloc(_textures, unitSubType, c, unitTemplate->getBasicAttack(), unitTemplate->getSpecialAttack(), unitDirection, unitSubType!= "king", unitSubType != "basic", unit->getUnitsNeededToKill());
            newUnit->setState(Unit::State::IDLE);
            sq->setUnit(newUnit);
            auto unitNode = newUnit->getViewNode();
//            auto unitNode = scene2::PolygonNode::allocWithTextzure(_textures.at(unitPattern));
            unitNode->setAnchor(Vec2::ANCHOR_CENTER);
            unitNode->setPriority(UNIT_Z);
//            newUnit->setViewNode(unitNode);

            newUnit->setSpecial(unitSubType != "basic");
            if (_debug) unitNode->setAngle(newUnit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);
            
            // load the ui for king unit
//            auto unit_node = unit->getViewNode();
            if (unitSubType == "king") {
                loadKingUI(unit->getUnitsNeededToKill(), unit->getUnitsNeededToKill(), squarePosition, unitNode);
            }
        }
    }
    // Create the upcoming special unit indicator
    _upcomingUnitNode = scene2::PolygonNode::allocWithTexture(_textures.at("bubble"));
    _boardNode->addChild(_upcomingUnitNode);
    _upcomingUnitNode->setVisible(false);
    _layout->layout(_guiNode.get());

    // Update UI Elements
//    _layout->addAbsolute("scoreMeterStar1", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(2 - _topuibackgroundNode->getSize().width / 6 + 18 + (_scoreMeter->getSize().width - 14) * (static_cast<float>(_level->oneStarThreshold) / _level->threeStarThreshold),
//        20 + -0.85 * (_topuibackgroundNode->getSize().height)));
//    _layout->addAbsolute("scoreMeterStar2", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(2 - _topuibackgroundNode->getSize().width / 6 + 18 + (_scoreMeter->getSize().width - 14) * (static_cast<float>(_level->twoStarThreshold) / _level->threeStarThreshold),
//        20 + -0.85 * (_topuibackgroundNode->getSize().height)));
//    _layout->addAbsolute("scoreMeterStar3", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(2 - _topuibackgroundNode->getSize().width / 6 + (_scoreMeter->getSize().width),
//        20 + -0.85 * (_topuibackgroundNode->getSize().height)));
    
    // Create the unit swap actions
    float inverseSquareFactor = 1 / _squareScaleFactor;
    _moveleft = cugl::scene2::MoveBy::alloc(Vec2(inverseSquareFactor * -_squareSizeAdjustedForScale,0),SWAP_DURATION);
    _moveright = cugl::scene2::MoveBy::alloc(Vec2(inverseSquareFactor * _squareSizeAdjustedForScale,0),SWAP_DURATION);
    _moveup = cugl::scene2::MoveBy::alloc(Vec2(0,inverseSquareFactor * _squareSizeAdjustedForScale),SWAP_DURATION);
    _movedn = cugl::scene2::MoveBy::alloc(Vec2(0,inverseSquareFactor * -_squareSizeAdjustedForScale),SWAP_DURATION);
}

void GameScene::loadKingUI(int unitsKilled, int goal, Vec2 sq_pos, std::shared_ptr<cugl::scene2::PolygonNode> unitNode) {
    std::string unitsNeededToKill = strtool::format("%d", goal);
    _info_text = scene2::Label::allocWithText(unitsNeededToKill, _assets->get<Font>("pixel32"));
    _info_text->setScale(3);
    _info_text->setColor(Color4::RED);
    _info_text->setPosition(Vec2(unitNode->getPosition().x+Vec2::ONE.x, unitNode->getPosition().y+Vec2::ONE.y));
    unitNode->addChildWithName(_info_text, "info");
//    CULog("text has priority of %f", _info_text->getPriority());
//    CULog("unit has priority of %f", unitNode->getPriority());
}

void GameScene::showResultText(bool success, std::shared_ptr<cugl::scene2::SceneNode> node) {
    std::string resultText;
    if (success) {
        resultText = strtool::format("You win!");
    } else {
        resultText = strtool::format("You lose!");
    }
    auto text = scene2::Label::allocWithText(resultText, _assets->get<Font>("pixel32"));
    text->setScale(1);
    text->setColor(Color4::RED);
    text->setPosition(Vec2(100, 400));
    node->addChildWithName(text, "info");
//    CULog("text has priority of %f", _info_text->getPriority());
//    CULog("unit has priority of %f", unitNode->getPriority());
}
