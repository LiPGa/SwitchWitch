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
#define ACT_KEY "current"
#define ALT_KEY "slide"

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
    _defaultUnitSize = constants->getInt("unit-size");
    _unitSpritePaddingFactor = constants->getFloat("unit-sprite-padding-factor");
    _topUI_scores_color = constants->getString("topUI-scores-color");
    _topUI_maxTurn_color = constants->getString("topUI-maxTurn-color");

    // Initialize Scene
    dimen *= sceneHeight / dimen.height;
    _dimen = dimen;
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
    _didGoToNextLevel = false;
    _didPause = false;
    unit1Selected = true;
    unit2Selected = false;
    unit3Selected = false;
    unit4Selected = false;
    unitMissing = {false, false, false};
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
        int respawnProb = boardMembers->get("unit")->get(probabilityName)->get("probability-respawn")->asInt();
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
    
    // --------------------- tutorial -----------------------
    for (int i=1; i<=7; i++) {
        _tutorialTextures.insert({"tutorial_"+to_string(i), assets->get<Texture>("tutorial_"+to_string(i))});
    }
    _tutorialTextures.insert({"tutorial_16", assets->get<Texture>("tutorial_16")});
    _tutorialTextures.insert({"tutorial_20", assets->get<Texture>("tutorial_20")});
    _tutorialTextures.insert({"tutorial_25", assets->get<Texture>("tutorial_25")});
    // default tutorial is level 1
    _tutorialLayout2 = assets->get<scene2::SceneNode>("tutorialLayout");
    _tutorialLayout2->setContentSize(dimen);
    _tutorialLayout2->doLayout();
    _guiNode->addChild(_tutorialLayout2);
    _tutorialLayout2->setVisible(true);
    _tutorialNode = scene2::SpriteNode::alloc(_tutorialTextures.at("tutorial_1"), 1, animationFrameCounts.at(ANIMATION_TYPE::TUTORIAL));

    _tutorialNode->setPosition(0.5*dimen.width, 0.45*dimen.height);
    _tutorialNode->setScale(0.3f);
    _tutorialLayout2->addChildWithName(_tutorialNode, "tutorialNode");
    
    // Initialize Board
    _board = Board::alloc(_maxBoardWidth, _maxBoardHeight);

    // -------- Set the view of the board --------------
    _squareSizeAdjustedForScale = _defaultSquareSize * min(_scale.width, _scale.height);
    _boardNode = scene2::PolygonNode::allocWithPoly(Rect(0, 0, _maxBoardWidth * _squareSizeAdjustedForScale, _maxBoardHeight * _squareSizeAdjustedForScale));
    _orderedBoardChild = scene2::OrderedNode::allocWithOrder(cugl::scene2::OrderedNode::DESCEND);
    _layout->addRelative("boardNode", cugl::scene2::Layout::Anchor::CENTER, Vec2(0, -0.075));
    _boardNode->setTexture(_textures.at("transparent"));
    _boardNode->addChild(_orderedBoardChild);
    _board->setViewNode(_boardNode);
    _guiNode->addChildWithName(_boardNode, "boardNode");

    // ------ Initialize units with different types ----------
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

    _helpMenu = std::dynamic_pointer_cast<scene2::SceneNode>(_assets->get<scene2::SceneNode>("help_menu"));
    _helpMenu->setContentSize(dimen);
    _helpMenu->doLayout();
    _guiNode->addChild(_helpMenu);
    _helpMenu->setVisible(false);
    _isScrolling = false;

    _tutorialLayout = assets->get<scene2::SceneNode>("tutorial");
    _tutorialLayout->setContentSize(dimen);
    _tutorialLayout->doLayout();
    _guiNode->addChild(_tutorialLayout);
    _tutorialLayout->setVisible(false);
    _tutorialActive = false;

    // sucess score summary screen
    _resultLayout = assets->get<scene2::SceneNode>("result");
    _resultLayout->setContentSize(dimen);
    _resultLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_resultLayout);
    _resultLayout->setVisible(false);

    // failure score summary screen
    _failResultLayout = assets->get<scene2::SceneNode>("failresult");
    _failResultLayout->setContentSize(dimen);
    _failResultLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_failResultLayout);
    _failResultLayout->setVisible(false);

    _settingsLayout = assets->get<scene2::SceneNode>("settings");
    _settingsLayout->setContentSize(dimen);
    _settingsLayout->doLayout(); // Repositions the HUD
    _guiNode->addChild(_settingsLayout);

    _settingsMenuLayout = assets->get<scene2::SceneNode>("settings-menu");
    _settingsMenuLayout->setContentSize(dimen);
    _settingsMenuLayout->doLayout(); // Repositions the HUD
    _settingsMenuLayout->getChild(0)->getChild(2)->setScale(0.725);
    _settingsMenuLayout->getChild(0)->getChild(3)->setScale(0.725);
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
    _fail_level_info = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("failresult_board_level"));

    _score_number = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("result_board_number"));
    _fail_score_number = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("failresult_board_number"));

    _star1 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star1"));
    _star2 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star2"));
    _star3 = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("result_board_star3"));

    _restartbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("result_board_restart"));
    _restartbutton->deactivate();
    _restartbutton->setDown(false);
    _restartbutton->clearListeners();
    _restartbutton->addListener([this](const std::string &name, bool down)
                                {
        if (down) {
            _didRestart = true;
        } });

    _failRestartButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("failresult_board_restart"));
    _failRestartButton->deactivate();
    _failRestartButton->setDown(false);
    _failRestartButton->addListener([this](const std::string &name, bool down)
                                    {
        if (down) {
            _didRestart = true;
        } });

    _backbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("result_board_exit"));
    _backbutton->deactivate();
    _backbutton->setDown(false);
    _backbutton->clearListeners();
    _backbutton->addListener([this](const std::string &name, bool down)
                             {
        if (down) {
            _didGoToLevelMap = true;
        } });

    _failBackButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("failresult_board_exit"));
    _failBackButton->deactivate();
    _failBackButton->setDown(false);
    _failBackButton->addListener([this](const std::string &name, bool down)
                                 {
        if (down) {
            _didGoToLevelMap = true;
        } });

    _nextbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("result_board_next"));
    _nextbutton->deactivate();
    _nextbutton->setDown(false);
    _nextbutton->addListener([this](const std::string &name, bool down)
                             {
        if (down) {
            _didGoToNextLevel = true;
        } });

    _settingsbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings_btn"));
    _settingsbutton->setVisible(true);
    _settingsbutton->activate();
    _settingsbutton->setDown(false);
    _settingsbutton->clearListeners();
    _settingsbutton->addListener([this](const std::string &name, bool down)
                                 {
        if (down) {
            _didPause = true;
        } });

    _scoreExplanationButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings_score-explanation-btn"));
    _scoreExplanationButton->setVisible(true);
    _scoreExplanationButton->activate();
    _scoreExplanationButton->setDown(false);
    _scoreExplanationButton->clearListeners();
    _scoreExplanationButton->addListener([this](const std::string &name, bool down)
                                         {
        if (down && _time >= TIME_BEFORE_INTERACTION) {
             _scoreExplanation->setVisible(!_scoreExplanation->isVisible());
        } });

    //    _unit_types = _level->unitTypes;

    _scoreExplanation = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("settings_score-explanation"));
    _scoreExplanation->setScale(_scale);
    _scoreExplanation->setVisible(false);

    _almanac = assets->get<scene2::SceneNode>("almanac");
    _almanac->setContentSize(dimen);
    _almanac->doLayout(); // Repositions the HUD
    _guiNode->addChild(_almanac);

    _almanacbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac_btn"));
    //_almanacbutton->setScale(_scale);
    _almanacbutton->setVisible(true);
    //    _almanacbutton->setContentSize(dimen);
    //    _almanacbutton->doLayout();
    _almanacbutton->activate();
    _almanacbutton->setDown(false);
    _almanacbutton->addListener([this](const std::string &name, bool down)
                                {
        if (down) {
            CULog("%u is pressed", 2);
            _didPreview = true;
        } });
    //    _guiNode->addChild(_almanacbutton);

    _settingsRestartBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_restart"));
    _settingsRestartBtn->setVisible(true);
    _settingsRestartBtn->deactivate();
    _settingsRestartBtn->setDown(false);
    _settingsRestartBtn->clearListeners();
    _settingsRestartBtn->addListener([this](const std::string &name, bool down)
                                     {
        if (down) {
            _didRestart = true;
            CULog("Pressed Settings restart button");
        } });

    _settingsCloseBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_close"));
    _settingsCloseBtn->setVisible(true);
    _settingsCloseBtn->deactivate();
    _settingsCloseBtn->setDown(false);
    _settingsCloseBtn->clearListeners();
    _settingsCloseBtn->addListener([this](const std::string &name, bool down)
                                   {
        if (down) {
            _didPause = false;
            _settingsMenuLayout->setVisible(false);
            CULog("Pressed Settings restart button");
        } });

    _settingsHelpBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_help"));
    _settingsHelpBtn->setVisible(true);
    _settingsHelpBtn->deactivate();
    _settingsHelpBtn->setDown(false);
    _settingsHelpBtn->clearListeners();
    _settingsHelpBtn->addListener([this](const std::string &name, bool down)
                                  {
        if (down) {
            // help menu
            _isHelpMenuOpen = true;
            _helpMenu->setVisible(true);
            _helpBackBtn->activate();
            _helpMenu->setPositionY(-1182+720);
            // almanac
            _almanacbutton->setVisible(false);
            _almanacbutton->deactivate();
            _scoreExplanationButton->deactivate();
            // tutorial
            _tutorialLayout->setVisible(false);
            _tutorialActive = false;
            _tutorialLeftBtn->deactivate();
            _tutorialRightBtn->deactivate();
            _tutorialCloseBtn->deactivate();
            // set setting buttons inactive
            _didPause = false;
            _settingsMenuLayout->setVisible(false);
            _settingsLayout->setVisible(false);
            _settingsbutton->deactivate();
            _settingsBackBtn->deactivate();
            _settingsRestartBtn->deactivate();
            _settingsCloseBtn->deactivate();
            _soundSlider->deactivate();
            _musicSlider->deactivate();
            CULog("Pressed Settings help button");
        } });

    _helpBackBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("help_menu_background_back"));
    _helpBackBtn->setVisible(true);
    _helpBackBtn->deactivate();
    _helpBackBtn->setDown(false);
    _helpBackBtn->clearListeners();
    _helpBackBtn->addListener([this](const std::string &name, bool down)
                              {
        if (down) {
            // help
            _isHelpMenuOpen = false;
            _helpMenu->setVisible(false);
            // settings
            _settingsLayout->setVisible(true);
            _settingsbutton->activate();
            // almanac
            _almanacbutton->setVisible(true);
            _almanacbutton->activate();
            _scoreExplanationButton->activate();
            CULog("Pressed help menu close button");
        } });
    
//    // --------------------- tutorial -----------------------
//    _tutorialTextures.insert({"tutorial_1", assets->get<Texture>("tutorial_1")});
//    _tutorialTextures.insert({"tutorial_2", assets->get<Texture>("tutorial_2")});
//    _tutorialTextures.insert({"tutorial_4", assets->get<Texture>("tutorial_4")});
//    _tutorialTextures.insert({"tutorial_5", assets->get<Texture>("tutorial_5")});
//    _tutorialTextures.insert({"tutorial_6", assets->get<Texture>("tutorial_6")});
//    // default tutorial is level 1
//    _tutorialLayout2 = assets->get<scene2::SceneNode>("tutorialLayout");
//    _tutorialLayout2->setContentSize(dimen);
//    _tutorialLayout2->doLayout();
//    _guiNode->addChild(_tutorialLayout2);
//    _tutorialLayout->setVisible(false);
//    _tutorialNode = scene2::SpriteNode::alloc(_tutorialTextures.at("tutorial_1"), 1, animationFrameCounts.at(ANIMATION_TYPE::TUTORIAL));
//
//    _tutorialNode->setPosition(0.5*dimen.width, 0.45*dimen.height);
//    _tutorialNode->setScale(0.3f);
//    _tutorialLayout2->addChildWithName(_tutorialNode, "tutorialNode");
    
    _tutorial_page = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("tutorial_board_page"));

    _tutorialCloseBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("tutorial_board_close"));
    _tutorialCloseBtn->setVisible(true);
    _tutorialCloseBtn->deactivate();
    _tutorialCloseBtn->setDown(false);
    _tutorialCloseBtn->clearListeners();
    _tutorialCloseBtn->addListener([=](const std::string &name, bool down)
                                   {
        if (down) {
            _tutorialLayout->setVisible(false);
            _tutorialActive = false;
            CULog("Pressed tutorial close button");
        } });

    _tutorialLeftBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("tutorial_board_left"));
    _tutorialLeftBtn->setVisible(true);
    _tutorialLeftBtn->deactivate();
    _tutorialLeftBtn->setDown(false);
    _tutorialLeftBtn->clearListeners();
    _tutorialLeftBtn->addListener([=](const std::string &name, bool down)
                                  {
        if (down) {
            flipTutorialPage("left");
            CULog("Pressed tutorial left button");
        } });

    _tutorialRightBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("tutorial_board_right"));
    _tutorialRightBtn->setVisible(true);
    _tutorialRightBtn->deactivate();
    _tutorialRightBtn->setDown(false);
    _tutorialRightBtn->clearListeners();
    _tutorialRightBtn->addListener([=](const std::string &name, bool down)
                                   {
        if (down) {
            flipTutorialPage("right");
            CULog("Pressed tutorial right button");
        } });

    _almanacCloseBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_close"));
    _almanacCloseBtn->setVisible(true);
    _almanacCloseBtn->activate();
    _almanacCloseBtn->setDown(false);
    _almanacCloseBtn->addListener([this](const std::string &name, bool down)
                                  {
        if (down) {
            _didPreview = false;
            _almanacLayout->setVisible(false);
            CULog("Pressed Settings restart button");
        } });

    viewAttackingPatterns();

    _settingsBackBtn = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("settings-menu_board_exit"));
    _settingsBackBtn->setVisible(true);
    _settingsBackBtn->deactivate();
    _settingsBackBtn->setDown(false);
    _settingsBackBtn->clearListeners();
    _settingsBackBtn->addListener([this](const std::string &name, bool down)
                                  {
        if (down) {
            _didGoToLevelMap = true;
            CULog("Pressed Settings exit button");
        } });

    _audioQueue = AudioEngine::get()->getMusicQueue();
    _audioQueue->setVolume(_musicVolume);

    _musicSlider = std::dynamic_pointer_cast<scene2::Slider>(assets->get<scene2::SceneNode>("settings-menu_board_musicSlider"));
    _musicSlider->setVisible(true);
    _musicSlider->deactivate();
    _musicSlider->setValue(_musicVolume);
    _musicSlider->addListener([=](const std::string &name, float value)
                              {
        if (_musicVolume != value ) {
            _musicVolume = value;} });

    _soundSlider = std::dynamic_pointer_cast<scene2::Slider>(assets->get<scene2::SceneNode>("settings-menu_board_soundSlider"));
    _soundSlider->setVisible(true);
    _soundSlider->deactivate();
    _soundSlider->setValue(_soundVolume);
    std::shared_ptr<cugl::scene2::Button> knob = std::dynamic_pointer_cast<scene2::Button>(_soundSlider->getKnob());
    knob->addListener([=](const std::string &name, bool down)
                      {
                          if (!down)
                          {
                              if (AudioEngine::get()->getState("attacksound") != AudioEngine::State::PLAYING)
                              {
                                  AudioEngine::get()->play("attacksound", _assets->get<Sound>("attacksound"), false, _soundVolume, false);
                              }
                          } });
    _soundSlider->addListener([=](const std::string &name, float value)
                              {
        if (_soundVolume != value) {
            _soundVolume = value;} });

    _kingsKilled = false;
    return true;
}

void GameScene::viewAttackingPatterns()
{
    _unitPattern1 = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit1-pattern"));
    _unitPattern2 = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit2-pattern"));
    _unitPattern3 = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit3-pattern"));
    _unitPattern4 = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit4-pattern"));
    _unitPattern1->setVisible(true);
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
    _unitButtons_selected = {_unit1button_selected, _unit2button_selected, _unit3button_selected, _unit4button_selected};
    _unitPatterns = {_unitPattern1, _unitPattern2, _unitPattern3, _unitPattern4};

    _unit1button->setVisible(true);
    _unit1button->activate();
    _unit1button->setDown(false);
    _unit1button->addListener([this](const std::string &name, bool down)
                              {
        if (down) {
            CULog("Pressed unit1 button");
            unit1Selected = true;
            unit2Selected = false;
            unit3Selected = false;
            unit4Selected = false;
        } });

    _unit1button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit1-selected"));
    _unit1button_selected->deactivate();
    _unit1button_selected->setVisible(false);
    _unit1button_selected->setDown(false);
    _unit1button_selected->addListener([this](const std::string &name, bool down)
                                       {
        if (down) {
            CULog("Pressed unit1 button");
            unit1Selected = false;
        } });

    _unit2button->setVisible(true);
    _unit2button->activate();
    _unit2button->setDown(false);
    _unit2button->addListener([this](const std::string &name, bool down)
                              {
        if (down) {
            unit2Selected = true;
            unit1Selected = false;
            unit3Selected = false;
            unit4Selected = false;
        } });

    _unit2button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit2-selected"));
    _unit2button_selected->setVisible(false);
    _unit2button_selected->deactivate();
    _unit2button_selected->setDown(false);
    _unit2button_selected->addListener([this](const std::string &name, bool down)
                                       {
        if (down) {
            CULog("Pressed unit2 button");
            unit2Selected = false;
        } });

    _unit3button->setVisible(true);
    _unit3button->activate();
    _unit3button->setDown(false);
    _unit3button->addListener([this](const std::string &name, bool down)
                              {
        if (down) {
            CULog("Pressed unit3 button");
            unit3Selected = true;
            unit1Selected = false;
            unit2Selected = false;
            unit4Selected = false;
        } });

    _unit3button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit3-selected"));
    _unit3button_selected->setVisible(false);
    _unit3button_selected->deactivate();
    _unit3button_selected->setDown(false);
    _unit3button_selected->addListener([this](const std::string &name, bool down)
                                       {
            if (down) {
                CULog("Pressed unit3 button");
                unit3Selected = false;
            } });

    _unit4button->setVisible(true);
    _unit4button->activate();
    _unit4button->setDown(false);
    _unit4button->addListener([this](const std::string &name, bool down)
                              {
        if (down) {
            CULog("Pressed unit4 button");
            unit4Selected = true;
            unit1Selected = false;
            unit2Selected = false;
            unit3Selected = false;
        } });

    _unit4button_selected = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("almanac-menu_board_unit4-selected"));
    _unit4button_selected->setVisible(false);
    _unit4button_selected->deactivate();
    _unit4button_selected->setDown(false);
    _unit4button_selected->addListener([this](const std::string &name, bool down)
                                       {
            if (down) {
                unit4Selected = false;
            } });
}

/**
 * Set up the top UI
 *
 * UI includes: star and score system, number of swaps, setting menu, and unit icons
 */
void GameScene::setTopUI(const std::shared_ptr<cugl::AssetManager> &assets, std::shared_ptr<cugl::JsonValue> &constants)
{
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
    _layout->addAbsolute("top_ui_background", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(0, -(_topuibackgroundNode->getSize().height / 1.8)));
    _guiNode->addChildWithName(_topuibackgroundNode, "top_ui_background");

    // ------------- Create the star system -------------
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
    for (auto star : starSystem)
    {
        star->setScale(0.155f);
    };
    // Add stars to guiNode
    _guiNode->addChildWithName(_oneStar, "oneStar");
    _guiNode->addChildWithName(_twoStar1, "twoStar1");
    _guiNode->addChildWithName(_twoStar2, "twoStar2");
    _guiNode->addChildWithName(_threeStar1, "threeStar1");
    _guiNode->addChildWithName(_threeStar2, "threeStar2");
    _guiNode->addChildWithName(_threeStar3, "threeStar3");

    // ------ Layout of the star system ----------
    // One star
    float xStartPoint = 10 - _topuibackgroundNode->getSize().width / 4.3;
    float xInterval = _topuibackgroundNode->getSize().width * 0.039;
    float yStartPoint = -0.4 * (_topuibackgroundNode->getSize().height);
    float yInterval = -0.11 * (_topuibackgroundNode->getSize().height);
    _layout->addAbsolute("oneStar", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint, yStartPoint));
    // Two stars
    _layout->addAbsolute("twoStar1", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint, yStartPoint + yInterval));
    _layout->addAbsolute("twoStar2", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint + xInterval, yStartPoint + yInterval));
    // Three stars
    _layout->addAbsolute("threeStar1", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint, yStartPoint + yInterval * 2));
    _layout->addAbsolute("threeStar2", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint + xInterval, yStartPoint + yInterval * 2));
    _layout->addAbsolute("threeStar3", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint + xInterval * 2, yStartPoint + yInterval * 2));

    // Load scores for the star system
    _oneStar_text = scene2::Label::allocWithText("xxx", _assets->get<Font>("pixel32"));
    _guiNode->addChildWithName(_oneStar_text, "oneStar_text");

    _twoStar_text = scene2::Label::allocWithText("xxx", _assets->get<Font>("pixel32"));
    _guiNode->addChildWithName(_twoStar_text, "twoStar_text");

    _threeStar_text = scene2::Label::allocWithText("xxx", _assets->get<Font>("pixel32"));
    _guiNode->addChildWithName(_threeStar_text, "threeStar_text");

    std::shared_ptr<cugl::scene2::Label> starText[3] = {_oneStar_text, _twoStar_text, _threeStar_text};
    for (auto text : starText)
    {
        text->setScale(0.45);
        text->setHorizontalAlignment(HorizontalAlign::RIGHT);
        text->setAnchor(Vec2(1.0, 0.5));
        text->setForeground(Color4(_topUI_scores_color));
    };

    // Layout of the scores for the star system
    _layout->addAbsolute("oneStar_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint + 4 * xInterval, yStartPoint));
    _layout->addAbsolute("twoStar_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint + 4 * xInterval, yStartPoint + yInterval));
    _layout->addAbsolute("threeStar_text", cugl::scene2::Layout::Anchor::TOP_CENTER, Vec2(xStartPoint + 4 * xInterval, yStartPoint + 2 * yInterval));

    // Load the current number of turns left for the player
    //    std::string turnMsg = to_string(_turns);
    _turn_text = scene2::Label::allocWithText("xxx", assets->get<Font>("pixel32"));
    _turn_text->setScale(0.5);
    _turn_text->setForeground(Color4::WHITE);
    _guiNode->addChildWithName(_turn_text, "turn_text");
    _layout->addAbsolute("turn_text", cugl::scene2::Layout::Anchor::TOP_RIGHT, Vec2(-_topuibackgroundNode->getSize().width / 4.5, -0.43 * (_topuibackgroundNode->getSize().height)));

    // Load the maximum number of turns for this level
    //    std::string maxTurnMsg = "/ " + to_string(_maxturns);
    _maxTurn_text = scene2::Label::allocWithText("xxx", assets->get<Font>("pixel32"));
    _maxTurn_text->setScale(0.5);
    _maxTurn_text->setForeground(Color4(_topUI_maxTurn_color));
    _guiNode->addChildWithName(_maxTurn_text, "maxTurn_text");
    _layout->addAbsolute("maxTurn_text", cugl::scene2::Layout::Anchor::TOP_RIGHT, Vec2(-_topuibackgroundNode->getSize().width / 5.25, -0.43 * (_topuibackgroundNode->getSize().height)));
};

/**
 * Get the pattern for a unit provided its type and color
 *
 */
std::string GameScene::getUnitType(std::string type, std::string color)
{
    return type + "-" + color;
}

std::string GameScene::generateRandomUnitType(std::map<std::string, float> probs)
{
    float total = 0.0;
    for (auto const &type : probs)
    {
        total += type.second;
    }
    float randomChoice = total * ((double)rand() / (RAND_MAX));
    float i = 0.0;
    for (auto const &type : probs)
    {
        if (i + type.second >= randomChoice)
            return type.first;
        i += type.second;
    }
    return ""; // Unreachable
}

Unit::Color GameScene::generateRandomUnitColor(std::map<Unit::Color, float> probs)
{
    float total = 0.0;
    for (auto const &type : probs)
    {
        total += type.second;
    }
    float randomChoice = total * ((double)rand() / (RAND_MAX));
    float i = 0.0;
    for (auto const &type : probs)
    {
        if (i + type.second >= randomChoice)
            return type.first;
        i += type.second;
    }
    return Unit::Color(0); // Unreachable
}

Vec2 GameScene::generateRandomDirection()
{
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
    if (_currentReplacementDepth[_board->flattenPos(square->getPosition().x, square->getPosition().y)] + 1 >= _level->maxTurns)
    {
        if (currentUnit->isSpecial() && currentUnit->getSubType() != "king" && currentUnit->getSubType() != "empty")
            sqTexture = "special_" + currentDirection + "_square";
        else
            sqTexture = "square-" + _level->backgroundName;
        CULog("set square texture??");
        square->getViewNode()->setTexture(_textures.at(sqTexture));
//        if (find(_attackedSquares.begin(), _attackedSquares.end(), square)!=_attackedSquares.end() && square->getUnit()->getState() == Unit::State::TARGETED)
//            square->getUnit()->setState(Unit::State::IDLE);
//            refreshUnitView(square);
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
    CULog("set texutre square !!");
    square->getViewNode()->setTexture(_textures.at(sqTexture));
}

void GameScene::refreshUnitAndSquareView(shared_ptr<Square> sq)
{
    refreshUnitView(sq);
    updateSquareTexture(sq);
}

void GameScene::refreshUnitView(shared_ptr<Square> sq)
{
    auto unit = sq->getUnit();
    auto unitNode = unit->getViewNode();

    std::string unitSubtype = unit->getSubType();
    if (unitSubtype != "basic")
        unit->setSpecial(true);
    else
        unit->setSpecial(false);
    sq->getViewNode()->removeAllChildren();
    if (unitNode->getParent() == NULL)
        sq->getViewNode()->addChild(unitNode);
    if (_debug)
        unitNode->setAngle(unit->getAngleBetweenDirectionAndDefault());
    float inverseSquareFactor = 1 / _squareScaleFactor;
    unitNode->setPosition(Vec2::ONE * inverseSquareFactor * (_squareSizeAdjustedForScale / 2));
    unitNode->setScale(_unitScaleFactor);
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

std::map<Unit::Color, float> GameScene::generateColorProbabilities()
{
    std::map<Unit::Color, float> probs = {
        {Unit::Color(0), 0.0},
        {Unit::Color(1), 0.0},
        {Unit::Color(2), 0.0}};
    std::map<Unit::Color, int> colorCounts = {
        {Unit::Color(0), 0},
        {Unit::Color(1), 0},
        {Unit::Color(2), 0}};
    int totalUnitCount = 0;
    vector<std::shared_ptr<Square>> allSquares = _board->getAllSquares();
    for (auto sq : allSquares)
    {
        std::shared_ptr<Unit> unit = sq->getUnit();
        Unit::Color unitColor = unit->getColor();
        colorCounts.at(unitColor) = colorCounts.at(unitColor) + 1;
        totalUnitCount++;
    }
    float cumulativeProb = 0.0;
    for (auto const &color : colorCounts)
    {
        float inverseProbability = static_cast<float>(totalUnitCount) / color.second;
        colorCounts.at(color.first) = inverseProbability;
        cumulativeProb += inverseProbability;
    }
    for (auto const &color : probs)
    {
        probs.at(color.first) = colorCounts.at(color.first) / cumulativeProb;
    }
    return probs;
}

void GameScene::deconfirmSwap()
{
    _upcomingUnitNode->setVisible(false);
    _upcomingUnitNode->removeAllChildren();

    _currentState = SELECTING_SWAP;
    // If we are de-confirming a swap, we must undo the swap.
    _board->switchUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
    _selectedSquare->getUnit()->setDirection(_selectedSquareOriginalDirection);
    //_swappingSquare->getUnit()->setDirection(_swappingSquareOriginalDirection);
    for (shared_ptr<Square> protectedSquare : _protectedSquares)
    {
        protectedSquare->getUnit()->getViewNode()->removeAllChildren();
    }
    for (shared_ptr<Square> square : _board->getAllSquares())
    {
        square->getViewNode()->removeChildByName("shield");
        updateSquareTexture(square);
    }
    CULog("set square-selected");
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
        _nextbutton = nullptr;
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
    // if (_audioQueue->getState() == cugl::AudioEngine::State::INACTIVE) {
    //   _audioQueue->play(_assets->get<Sound>("track_1"), false, .3, false);
    //}
    //    _audioQueue->setLoop(true);
    if (_time <= TIME_BEFORE_INTERACTION)
    {
        _time += timestep;
    }

    // Read the keyboard for each controller.
    // Read the input
    _input.update();

    // show tutorial when first enter the level
//    if (_enterLevel && !_tutorialActive)
//    {
//        showTutorial(_guiNode);
//        _enterLevel = false;
//    }
//    else if (!_tutorialActive)
//    {
//        _tutorialLeftBtn->deactivate();
//        _tutorialRightBtn->deactivate();
//        _tutorialCloseBtn->deactivate();
//    }

    // help menu
    if (_isHelpMenuOpen)
    {
        _settingsHelpBtn->deactivate();
        // Process the toggled key commands
        Vec2 pos = _input.getPosition();
        if (_isScrolling)
        {
            Vec2 prevPos = _input.getPrevious();
            Vec2 moveDist = Vec2(pos.x - prevPos.x, prevPos.y - pos.y);
            Vec2 nodePos = _helpMenu->getPosition();
            moveDist.x = 0;
            if (moveDist.y + nodePos.y < -1182 + 720)
            {
                moveDist.y = -1182 + 720 - nodePos.y;
            }
            else if (moveDist.y + nodePos.y > 0)
            {
                moveDist.y = 0 - nodePos.y;
            }
            _helpMenu->setPosition(nodePos + moveDist);
        }
        if (_input.isDown())
        {
            _isScrolling = true;
            //            CULog("help back btn position: %f, %f", _helpBackBtn->nodeToScreenCoords(Vec2(0, 0)).x, _helpBackBtn->nodeToScreenCoords(Vec2(0, 0)).y);
            //            CULog("input position: %f, %f", pos.x, pos.y);
        }
        else if (_input.didRelease())
        {
            _isScrolling = false;
        }
        return;
    }
    else
    {
        _helpBackBtn->deactivate();
        _isScrolling = false;
    }

    if (_didRestart == true)
        reset(_levelJson);
    if (_didRestart == true && _didPause == true)
        reset(_levelJson);

    if (_didGoToNextLevel == true)
    {
        int newLevelNum = _levelJson->getInt("id") + 1;
        auto newBoardJson = _assets->get<JsonValue>("level" + std::to_string(newLevelNum));
        _currLevel = newLevelNum;
        reset(newBoardJson);
    }

    // ------------- tutorial animation ----------------
    _time_since_last_frame += timestep;
    _time_since_start_animation += timestep;
    if (_time_since_last_frame > _time_per_frame) {
        _time_since_last_frame = 0.0f;
        int frame = _tutorialNode->getFrame() + 1;
        if (frame >= _tutorialNode->getSize()) {
//            if (animationShouldLoop(_state)) frame = 0;
//            else frame = _tutorialNode->getSize() - 1;
            frame = 0;
//            completedAnimation = true;
        }
        _tutorialNode->setFrame(frame);
    }

    if ((_turns == 0 || _kingsKilled) && _currentState != ANIMATION)
    {
        // Show results screen
        // showResultText(_kingsKilled && _level->getNumberOfStars(_score) >= 3, _guiNode);
        _scoreExplanationButton->deactivate();

        if (_kingsKilled && _level->getNumberOfStars(_score) >= 1)
        {
            _resultLayout->setVisible(true);
            _restartbutton->activate();
            _backbutton->activate();
            _nextbutton->activate();
            _score_number->setText(to_string(_score));
            _level_info->setText("Level " + to_string(_levelJson->getInt("id")));
            _star1->setTexture(_textures.at("star_empty"));
            _star2->setTexture(_textures.at("star_empty"));
            _star3->setTexture(_textures.at("star_empty"));
            if (_level->getNumberOfStars(_score) >= 1)
            {
                _star1->setTexture(_textures.at("star_full"));
            }
            if (_level->getNumberOfStars(_score) >= 2)
            {
                _star2->setTexture(_textures.at("star_full"));
            }
            if (_level->getNumberOfStars(_score) >= 3)
            {
                _star3->setTexture(_textures.at("star_full"));
            }
        }

        else
        {
            _failResultLayout->setVisible(true);
            _failRestartButton->activate();
            _failBackButton->activate();
            _fail_level_info->setText("Level " + to_string(_levelJson->getInt("id")));
            _fail_score_number->setText(to_string(_score));
        }

        return;
    }
    _audioQueue->setVolume(_musicVolume);

    if (_didPause == true)
    {
        // Show settings screen
        _settingsMenuLayout->setVisible(true);
        _settingsBackBtn->activate();
        _settingsRestartBtn->activate();
        _settingsCloseBtn->activate();
        _settingsHelpBtn->activate();
        _musicSlider->activate();
        _soundSlider->activate();

        return;
    }

    if (_didPreview == true)
    {
        // show attacking patterns preview
        _almanacLayout->setVisible(true);
    }

    if (unit1Selected == true)
    {
        _unit1button->setVisible(false);
        _unit1button->deactivate();
        _unit1button_selected->setVisible(true);
        _unit1button_selected->activate();
        _unitPattern1->setVisible(true);
        _unitPattern2->setVisible(false);
        _unitPattern3->setVisible(false);
        _unitPattern4->setVisible(false);
    }

    if (unit1Selected == false)
    {
        _unit1button_selected->setVisible(false);
        _unit1button_selected->deactivate();
        _unit1button->setVisible(true);
        _unit1button->activate();
        //        _unitPattern1->setVisible(false);
    }

    if (unit2Selected == true && unitMissing.at(0) != true)
    {
        _unit2button->setVisible(false);
        _unit2button->deactivate();
        _unit2button_selected->setVisible(true);
        _unit2button_selected->activate();
        _unitPattern2->setVisible(true);
        _unitPattern1->setVisible(false);
        _unitPattern3->setVisible(false);
        _unitPattern4->setVisible(false);
    }

    if (unit2Selected == false && unitMissing.at(0) != true)
    {
        _unit2button_selected->setVisible(false);
        _unit2button_selected->deactivate();
        _unit2button->setVisible(true);
        _unit2button->activate();
        //        _unitPattern2->setVisible(false);
    }

    if (unit3Selected == true && unitMissing.at(1) != true)
    {
        _unit3button->setVisible(false);
        _unit3button->deactivate();
        _unit3button_selected->setVisible(true);
        _unit3button_selected->activate();
        _unitPattern3->setVisible(true);
        _unitPattern1->setVisible(false);
        _unitPattern2->setVisible(false);
        _unitPattern4->setVisible(false);
    }

    if (unit3Selected == false && unitMissing.at(1) != true)
    {
        _unit3button_selected->setVisible(false);
        _unit3button_selected->deactivate();
        _unit3button->setVisible(true);
        _unit3button->activate();
        //        _unitPattern3->setVisible(false);
    }

    if (unit4Selected == true && unitMissing.at(2) != true)
    {
        _unit4button->setVisible(false);
        _unit4button->deactivate();
        _unit4button_selected->setVisible(true);
        _unit4button_selected->activate();
        _unitPattern4->setVisible(true);
        _unitPattern1->setVisible(false);
        _unitPattern2->setVisible(false);
        _unitPattern3->setVisible(false);
    }

    if (unit4Selected == false && unitMissing.at(2) != true)
    {
        _unit4button_selected->setVisible(false);
        _unit4button_selected->deactivate();
        _unit4button->setVisible(true);
        _unit4button->activate();
        //        _unitPattern4->setVisible(false);
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
                if (_enlargedUnitNode != NULL)
                {
                    _enlargedUnitNode->setScale(_unitScaleFactor * BACK2NORMAL);
                }

                // Enlarge the new selected unit
                _enlargedUnitNode = _selectedSquare->getUnit()->getViewNode();
                _enlargedUnitNode->setScale(_unitScaleFactor * ENLARGE);

                std::shared_ptr<Square> replacementSquare = _selectedSquare == NULL ? NULL : _level->getBoard(_currentReplacementDepth[_board->flattenPos(_selectedSquare->getPosition().x, _selectedSquare->getPosition().y)] + 1)->getSquare(_selectedSquare->getPosition());
                auto upcomingUnitType = replacementSquare->getUnit()->getSubType();
                auto upcomingUnitColor = Unit::colorToString(replacementSquare->getUnit()->getColor());
                auto upcomingUnitDirection = Unit::directionToString(replacementSquare->getUnit()->getDirection());
                auto upcomingSquarePos = _selectedSquare->getViewNode()->getPosition();
                if (upcomingUnitType != "basic" && upcomingUnitType != "random" && upcomingUnitType != "empty")
                {
                    _upcomingUnitNode->setPosition(upcomingSquarePos + Vec2(0, _squareSizeAdjustedForScale));
                    _upcomingUnitNode->setAnchor(Vec2::ANCHOR_CENTER);
                    float squareSizeFactor = (float)_squareSizeAdjustedForScale / (float)_defaultSquareSize;
                    _upcomingUnitNode->setScale(squareSizeFactor);
                    auto upcomingDirectionNode = scene2::PolygonNode::allocWithTexture(_textures.at("special_" + upcomingUnitDirection + "_square"));
                    upcomingDirectionNode->setAnchor(Vec2::ANCHOR_CENTER);
                    upcomingDirectionNode->setPosition(Vec2(_upcomingUnitNode->getWidth() / 2, _upcomingUnitNode->getHeight() / 2.1) / squareSizeFactor);
                    upcomingDirectionNode->setScale(0.8f);
                    _upcomingUnitNode->addChild(upcomingDirectionNode);
                    auto upcomingTextureNode = scene2::PolygonNode::allocWithTexture(_textures.at(upcomingUnitType + "-" + upcomingUnitColor));
                    upcomingTextureNode->setScale(0.8f * _unitScaleFactor);
                    upcomingTextureNode->setAnchor(Vec2::ANCHOR_CENTER);
                    upcomingTextureNode->setPosition(Vec2(_upcomingUnitNode->getWidth() / 2, _upcomingUnitNode->getHeight() / 1.7) / squareSizeFactor);
                    _upcomingUnitNode->addChild(upcomingTextureNode);
                    _upcomingUnitNode->setVisible(true);
                }
            }
            else if (_currentState == SELECTING_SWAP && squareOnMouse->getPosition().distance(_selectedSquare->getPosition()) == 1 && _selectedSquare->getUnit()->isMoveable())
            {
                if (!squareOnMouse->getUnit()->isMoveable())
                    return;
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
//                    CULog("attacking square has unit type: %s", attackedSquare->getUnit()->stateToString(attackedSquare->getUnit()->getState()).c_str());
                    // show targeted animation
                    attackedSquare->getUnit()->setState(Unit::State::TARGETED);
                    refreshUnitView(attackedSquare);
                }
                for (shared_ptr<Square> protectedSquare : _protectedSquares)
                {
                    if (find(_attackedSquares.begin(), _attackedSquares.end(), protectedSquare)!=_attackedSquares.end()) continue;
                    auto protectedUnit = protectedSquare->getUnit();
                    _shieldNode = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>("shield"));
                    _shieldNode->setScale(1 / protectedUnit->getViewNode()->getScale());
                    _shieldNode->setPosition(Vec2(protectedUnit->getViewNode()->getWidth(), protectedUnit->getViewNode()->getHeight()));
                    protectedUnit->getViewNode()->addChildWithName(_shieldNode, "shield");
                }
            }
            else if (_currentState == CONFIRM_SWAP && squareOnMouse != _swappingSquare)
            {
                deconfirmSwap();
            }
        }
        else if (_currentState == CONFIRM_SWAP)
        {
            deconfirmSwap();
        }
    }
    else if (_input.didRelease() && _currentState != ANIMATION)
    {
        _upcomingUnitNode->setVisible(false);
        _upcomingUnitNode->removeAllChildren();
        if (_enlargedUnitNode)
            _enlargedUnitNode->setScale(_unitScaleFactor * BACK2NORMAL);

        for (shared_ptr<Square> square : _board->getAllSquares())
        {
            updateSquareTexture(square);
        }

        if (_board->doesSqaureExist(squarePos) && boardPos.x >= 0 && boardPos.y >= 0 && _board->getSquare(squarePos)->isInteractable() && _currentState == CONFIRM_SWAP)
        {
            _currentState = ANIMATION;
            _midSwap = true;

            _board->switchUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
            AudioEngine::get()->play("swap", _assets->get<Sound>("swap"), false, _soundVolume, false);
            // Animation
            // <Hedy>
            _selectedSquare->getUnit()->setState(Unit::State::SELECTED_START);
            refreshUnitView(_selectedSquare);
            // <Hedy/>
            _initalAttackSquare = _swappingSquare;
            _turns--;
        }
        else
        {
            _currentState = SELECTING_UNIT;
        }
    }

    if (_currentState == ANIMATION)
    {
        bool completedAllAnimations = true;
        bool completedAttackSequence = true;
        bool respawning = false;
        bool completedSwap = true;

        //        vector<std::shared_ptr<Square>> deadSquares;
        // <Hedy>
        if (_actions->isActive("swapA") || _actions->isActive("swapB") || _selectedSquare->getUnit()->getState() == Unit::State::SELECTED_START || _selectedSquare->getUnit()->getState() == Unit::State::SELECTED_MOVING)
        {
            completedSwap = false;
        }
        
        if (_selectedSquare->getUnit()->getState() == Unit::State::SELECTED_MOVING) {
        // <Hedy/>
            auto animationNodeSWA = _swappingSquare->getUnit()->getViewNode();
            auto animationNodeSWB = _selectedSquare->getUnit()->getViewNode();
            string direction = moveDirection(_swappingSquare, _selectedSquare);
            if (direction == "up")
            {
                doMove("swapA", animationNodeSWA, _moveup);
                doMove("swapB", animationNodeSWB, _movedn);
            }
            else if (direction == "down")
            {
                doMove("swapA", animationNodeSWA, _movedn);
                doMove("swapB", animationNodeSWB, _moveup);
            }
            else if (direction == "left")
            {
                doMove("swapA", animationNodeSWA, _moveleft);
                doMove("swapB", animationNodeSWB, _moveright);
            }
            else
            {
                doMove("swapA", animationNodeSWA, _moveright);
                doMove("swapB", animationNodeSWB, _moveleft);
            }
        // <Hedy>
            if (!_actions->isActive("swapA") && !_actions->isActive("swapB"))
            {
                _selectedSquare->getUnit()->completedAnimation = true;
            }
        }
        // <Hedy/>

        if (completedSwap)
        {
            if (_midSwap)
                updateModelPostSwap();
        }

        for (auto square : _board->getAllSquares())
        {
            auto unit = square->getUnit();
            auto unitState = unit->getState();
            bool isKing = unit->getSubType() == "king";
            if (!isKing && unitState != Unit::State::IDLE)
            {
                completedAllAnimations = false;
                if (unitState != Unit::State::DEAD && unitState != Unit::State::RESPAWNING)
                    completedAttackSequence = false;
                if (unitState == Unit::State::RESPAWNING)
                    respawning = true;
                //                else deadSquares.push_back(square);
            }
            else if (isKing && unitState != Unit::IDLE && unitState != Unit::DEAD)
            {
                completedAttackSequence = false;
            }
            updateSquareTexture(square);
        }

        if (completedSwap && completedAttackSequence)
        {
            if (completedAllAnimations && !_kingsKilled)
                _currentState = SELECTING_UNIT;
            else if (!respawning)
                respawnAttackedSquares();
        }
    }

    // Update the remaining turns
    _turn_text->setText(to_string(_turns));
    //    _maxTurn_text->setText("/ " + to_string(_maxturns));

    // Animate all units
    for (shared_ptr<Square> square : _board->getAllSquares())
    {
        auto unit = square->getUnit();
        auto unitState = unit->getState();
        std::string unitType = unit->getSubType();
        if (unit->completedAnimation)
        {
            switch (unitState)
            {
            case Unit::State::IDLE:
                break;
            case Unit::State::DEAD:
                //                    square->getViewNode()->setVisible(false);
                break;
            //<Hedy>
            case Unit::State::SELECTED_START:
                unit->setState(Unit::State::SELECTED_MOVING);
                break;
            case Unit::State::SELECTED_MOVING:
                unit->setState(Unit::State::SELECTED_NONE);
                break;
            case Unit::State::SELECTED_END:
                unit->setState(Unit::State::ATTACKING);
                refreshUnitView(square);
                break;
            //<Hedy/>
            case Unit::State::TARGETED:
                unit->setState(Unit::State::IDLE);
                refreshUnitView(square);
                break;
            case Unit::State::PROTECTED:
                unit->setState(Unit::State::IDLE);
                refreshUnitView(square);
                break;
            case Unit::State::HIT:
                if (unitType != "basic" && unitType != "empty" && unitType != "king")
                {
                    unit->setState(Unit::State::ATTACKING);
                    refreshUnitView(square);
                }
                else
                {
                    unit->setState(Unit::State::DYING);
                    refreshUnitView(square);
                }
                break;
            case Unit::State::ATTACKING:
                if (AudioEngine::get()->getState("attacksound") != AudioEngine::State::PLAYING) {
                    AudioEngine::get()->play("attacksound", _assets->get<Sound>("attacksound"), false, _soundVolume, false);
                }
                for (auto atkSquare : _board->getInitallyAttackedSquares(square->getPosition(), unit == _initalAttackSquare->getUnit()))
                {
                    if (atkSquare->getUnit()->getState() == Unit::State::IDLE)
                    {
                        //                            if (_attackedSquares.size() >= atkSquare->getUnit()->getUnitsNeededToKill()) {
                        atkSquare->getUnit()->setState(Unit::State::HIT);
                        //                            }
                    }
                    refreshUnitView(atkSquare);
                }
                for (auto ptdSquare : _board->getInitiallyProtectedSquares(square->getPosition(), unit == _initalAttackSquare->getUnit()))
                {
                    if (ptdSquare->getUnit()->getState() == Unit::State::IDLE)
                    {
                        ptdSquare->getUnit()->setState(Unit::State::PROTECTED);
                    }
                    refreshUnitView(ptdSquare);
                }
                if (unit->hasBeenHit())
                    unit->setState(Unit::State::DYING);
                else
                    unit->setState(Unit::State::IDLE);
                refreshUnitView(square);
                break;
            case Unit::State::DYING:
                unit->setState(Unit::State::DEAD);
                if (unit->getSubType() == "king")
                {
                    _kingsKilled = true;
                    AudioEngine::get()->play("deathsound", _assets->get<Sound>("deathsound"), false, _soundVolume, false);
                }
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

void GameScene::updateModelPostSwap()
{
    _midSwap = false;
    // <Hedy>
    // Save the selected unit testure before switch and rotate units
    string texture = (_selectedSquare->getUnit()->getSubType()) + "-" + Unit::colorToString(_selectedSquare->getUnit()->getColor()) + "-selected-end";
   // <Hedy/>
    _board->switchAndRotateUnits(_selectedSquare->getPosition(), _swappingSquare->getPosition());
    //  Because the units in the model where already swapped.
    float inverseSquareFactor = 1 / _squareScaleFactor;
    auto swappedUnitNode = _selectedSquare->getUnit()->getViewNode();
    auto selectedUnitNode = _swappingSquare->getUnit()->getViewNode();
    swappedUnitNode->setPosition(Vec2::ONE * inverseSquareFactor * (_squareSizeAdjustedForScale / 2));
    selectedUnitNode->setPosition(Vec2::ONE * inverseSquareFactor * (_squareSizeAdjustedForScale / 2));
    // <Hedy>
    // Prepare for Animation
    swappedUnitNode->setVisible(false);
    // <Hedy/>
    
    // Updating View
    _selectedSquare->getViewNode()->removeChild(selectedUnitNode);
    _swappingSquare->getViewNode()->removeChild(swappedUnitNode);
    _selectedSquare->getViewNode()->addChild(swappedUnitNode);
    _swappingSquare->getViewNode()->addChild(selectedUnitNode);
    
    // <Hedy>
    // Animation
    _swappingSquare->getUnit()->setSelectedEnd(_textures.at(texture));
    refreshUnitView(_swappingSquare);
    swappedUnitNode->setVisible(true);
    refreshUnitView(_selectedSquare);
    if ( _swappingSquare->getUnit()->completedAnimation) {
    // <Hedy/>
        if (_attackedSquares.size() > 0)
        {
            _initalAttackSquare->getUnit()->setState(Unit::State::ATTACKING); // begin the attack sequence
            refreshUnitView(_initalAttackSquare);
        }
        else
        { // need to show shield animation even if no unit is killed in this swap
            for (auto ptdSquare : _protectedSquares)
            {
                ptdSquare->getUnit()->setState(Unit::State::PROTECTED);
                refreshUnitView(ptdSquare);
            }
        }
    }
}

void GameScene::respawnAttackedSquares()
{
    //    _prevScore = _score;
    // remove the attacked squares
    int scoreNum = 0;
    bool plusScore = false;
    for (shared_ptr<Square> attackedSquare : _attackedSquares)
    {
        if (attackedSquare->getUnit()->getSubType() == "king" && _kingsKilled)
        {
            ////            _kingsKilled = true;
            plusScore = true;
        }
        auto attacked_unit = attackedSquare->getUnit();
        if (_attackedSquares.size() < attackedSquare->getUnit()->getUnitsNeededToKill())
        {
            continue;
        }

        //                if (attacked_unit->getSubType()=="king") {
        //                    std::string unitsNeededToKill = strtool::format("%d/%d",_attackedSquares.size(),attacked_unit->getUnitsNeededToKill());
        //                    _info_text->setText(unitsNeededToKill);
        //                }

        // Replace Unit
        if (attackedSquare->getUnit()->getSubType() != "king")
            replaceUnitOnSquare(attackedSquare);
        scoreNum++;
    }
    if (plusScore)
    {
        _score += scoreNum;
    }
    if (_kingsKilled)
        _currentState = SELECTING_UNIT;
}

void GameScene::replaceUnitOnSquare(shared_ptr<Square> sq)
{
    std::map<Unit::Color, float> colorProbabilities = generateColorProbabilities();
    Vec2 squarePos = sq->getPosition();
    _currentReplacementDepth[_board->flattenPos(squarePos.x, squarePos.y)]++;
    int currentReplacementDepth = _currentReplacementDepth[_board->flattenPos(squarePos.x, squarePos.y)];
    std::shared_ptr<Square> replacementSquare = _level->getBoard(currentReplacementDepth)->getSquare(squarePos);
    auto unitSubType = replacementSquare->getUnit()->getSubType();
    auto unitColor = unitSubType == "random" ? generateRandomUnitColor(colorProbabilities) : replacementSquare->getUnit()->getColor();
    auto unitDirection = unitSubType == "random" ? generateRandomDirection() : replacementSquare->getUnit()->getDirection();
    if (unitSubType == "random")
        unitSubType = generateRandomUnitType(_unitRespawnProbabilities);
std:
    string unitPattern = getUnitType(unitSubType, unitColor);
    generateUnit(sq, unitSubType, unitColor, unitDirection, replacementSquare->getUnit()->getUnitsNeededToKill());

    // Set empty squares to be uninteractable.
    sq->setInteractable(unitSubType != "empty");
    sq->getViewNode()->setVisible(unitSubType != "empty");
    //    refreshUnitAndSquareView(sq);
    if (unitSubType == "king")
        loadKingUI(replacementSquare->getUnit()->getUnitsNeededToKill(), replacementSquare->getUnit()->getUnitsNeededToKill(), squarePos, replacementSquare->getViewNode());
}

/**
 * Performs a move action
 *
 * @param action The move action
 */
void GameScene::doMove(std::string act_key, shared_ptr<cugl::scene2::PolygonNode> viewNode, const std::shared_ptr<cugl::scene2::MoveBy> &action)
{
    if (!viewNode->isVisible())
    {
        CULog("Not Visible");
        return;
    }
    if (_actions->isActive(act_key))
    {
        CULog("You must wait for the animation to complete first");
    }
    else
    {
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
string GameScene::moveDirection(shared_ptr<Square> selectedSq, shared_ptr<Square> swappingSq)
{
    Vec2 selectedPos = selectedSq->getPosition();
    int x1 = (int)selectedPos.x;
    int y1 = (int)selectedPos.y;

    Vec2 swappingPos = swappingSq->getPosition();
    int x2 = (int)swappingPos.x;
    int y2 = (int)swappingPos.y;

    string direction;
    if (x1 == x2 && y1 != y2)
    {
        if (y1 - y2 == 1)
        {
            direction = "down";
        }
        else if (y1 - y2 == -1)
        {
            direction = "up";
        }
    }
    else if (y1 == y2 && x1 != x2)
    {
        if (x1 - x2 == 1)
        {
            direction = "left";
        }
        else if (x1 - x2 == -1)
        {
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
        if (_selectedSquare != NULL)
        {
            switch (_selectedSquare->getUnit()->getState())
            {
            case Unit::IDLE:
                spe = "Idle";
                break;
            case Unit::HIT:
                spe = "Hit";
                break;
            // <Hedy>
            case Unit::SELECTED_START:
                spe = "Selected Start";
                break;
            case Unit::SELECTED_END:
                spe = "Selected End";
                break;
            // <Hedy/>
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
    removeChild(_guiNode);
    _didGoToLevelMap = false;
    _didPause = false;
    _isScrolling = false;
    _isHelpMenuOpen = false;
    _tutorialActive = false;
    init(_assets);
    setLevel(boardJSON);
    setTutorial();
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

void GameScene::updateAttackingPatterns()
{
    for (size_t i = 0; i < 3; i++)
    {
        string unit_type = _unit_types.at(i);
        if (unit_type == "empty")
        {
            unitMissing.at(i) = true;
        }
        else
        {
            unitMissing.at(i) = false;
        }
    }
    for (size_t j = 0; j < 3; j++)
    {
        int num = static_cast<int>(j) + 2;
        string nodeName = "almanac-menu_board_unit" + to_string(num) + "-missing";
        string iconName = "almanac_btn_unit" + to_string(num);
        string missingIconName = "almanac_btn_unit" + to_string(num) + "-missing";
        auto img = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>(nodeName));
        auto icon = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>(iconName));
        auto missingIcon = std::dynamic_pointer_cast<scene2::PolygonNode>(_assets->get<scene2::SceneNode>(missingIconName));
        if (unitMissing.at(j) == true)
        {
            img->setVisible(true);
            icon->setVisible(false);
            missingIcon->setVisible(true);
            _unitButtons.at(j + 1)->setVisible(false);
            _unitButtons.at(j + 1)->deactivate();
            _unitButtons_selected.at(j + 1)->setVisible(false);
            _unitButtons_selected.at(j + 1)->deactivate();
            _unitPatterns.at(j + 1)->setVisible(false);
        }
        else
        {
            img->setVisible(false);
            icon->setVisible(true);
            missingIcon->setVisible(false);
        }
    }
}

void GameScene::setLevel(shared_ptr<cugl::JsonValue> levelJSON)
{
    _levelJson = levelJSON;
    _level = Level::alloc(_textures, levelJSON);
    vector<int> vector(_level->getNumberOfRows() * _level->getNumberOfColumns(), 0);
    _currentReplacementDepth = vector;
    _score = 0;
    _turns = _level->maxTurns;
    _maxturns = _level->maxTurns;
    _unit_types = _level->unitTypes;
    updateAttackingPatterns();
    for (auto unit : _unit_types)
    {
        CULog("unit type is %s", unit.c_str());
    }

    // Change Background
    _backgroundNode->setTexture(_textures.at("background-" + _levelJson->getString("background")));

    map<Unit::Color, float> startingColorProbabilities = {
        {Unit::Color(0), 0.33},
        {Unit::Color(1), 0.33},
        {Unit::Color(2), 0.33}};

    // Load the turn texts
    std::string turnMsg = to_string(_turns);
    _turn_text->setText(turnMsg);
    std::string maxTurnMsg = "/ " + to_string(_maxturns);
    _maxTurn_text->setText(maxTurnMsg);

    // Load star thresholds
    std::string oneStarMsg = strtool::format("%d", _level->oneStarThreshold);
    _oneStar_text->setText(oneStarMsg, true);

    std::string twoStarMsg = strtool::format("%d", _level->twoStarThreshold);
    _twoStar_text->setText(twoStarMsg, true);

    std::string threeStarMsg = strtool::format("%d", _level->threeStarThreshold);
    _threeStar_text->setText(threeStarMsg, true);

    // Create the squares & units and put them in the map
    _board->getViewNode()->getChild(0)->removeAllChildren();
    _board = Board::alloc(_level->getNumberOfColumns(), _level->getNumberOfRows());
    _board->setViewNode(_boardNode);
    // Set the view of the board.
    _squareSizeAdjustedForScale = _defaultSquareSize * min(_scale.width, _scale.height) * (min((float)_maxBoardHeight / (float)_level->getNumberOfRows(), (float)_maxBoardWidth / (float)_level->getNumberOfColumns())) * 1.25;
    _squareScaleFactor = (float)_squareSizeAdjustedForScale / (float)_defaultSquareSize;
    _unitScaleFactor = (float)_squareSizeAdjustedForScale / (float)_defaultUnitSize / _squareScaleFactor / _unitSpritePaddingFactor;
    _boardNode->setPolygon(Rect(0, 0, _level->getNumberOfColumns() * _squareSizeAdjustedForScale, _level->getNumberOfRows() * _squareSizeAdjustedForScale));
    _boardNode->setTexture(_textures.at("transparent"));
    for (int i = 0; i < _level->getNumberOfColumns(); i++)
    {
        for (int j = 0; j < _level->getNumberOfRows(); ++j)
        {
            auto test = "square-" + _level->backgroundName;
            shared_ptr<scene2::PolygonNode> squareNode = scene2::PolygonNode::allocWithTexture(_textures.at("square-" + _level->backgroundName));
            auto squarePosition = Vec2(i, j);
            squareNode->setAnchor(Vec2::ANCHOR_CENTER);
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

            if (unitSubType == "random")
            {
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
            shared_ptr<Unit> newUnit = Unit::alloc(_textures, unitSubType, c, unitTemplate->getBasicAttack(), unitTemplate->getSpecialAttack(), unitDirection, unitSubType != "king", unitSubType != "basic", unit->getUnitsNeededToKill());
            newUnit->setState(Unit::State::IDLE);
            sq->setUnit(newUnit);
            auto unitNode = newUnit->getViewNode();
            //            auto unitNode = scene2::PolygonNode::allocWithTextzure(_textures.at(unitPattern));
            unitNode->setAnchor(Vec2::ANCHOR_CENTER + Vec2(0, -0.2));
            float inverseSquareFactor = 1.0f / _squareScaleFactor;
            unitNode->setPosition(Vec2::ONE * inverseSquareFactor * (_squareSizeAdjustedForScale / 2));
            unitNode->setScale(_unitScaleFactor);
            unitNode->setPriority(UNIT_Z);
            //            newUnit->setViewNode(unitNode);

            newUnit->setSpecial(unitSubType != "basic");
            if (_debug)
                unitNode->setAngle(newUnit->getAngleBetweenDirectionAndDefault());
            squareNode->addChild(unitNode);

            // load the ui for king unit
            //            auto unit_node = unit->getViewNode();
            if (unitSubType == "king")
            {
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
    _moveleft = cugl::scene2::MoveBy::alloc(Vec2(inverseSquareFactor * -_squareSizeAdjustedForScale, 0), SWAP_DURATION);
    _moveright = cugl::scene2::MoveBy::alloc(Vec2(inverseSquareFactor * _squareSizeAdjustedForScale, 0), SWAP_DURATION);
    _moveup = cugl::scene2::MoveBy::alloc(Vec2(0, inverseSquareFactor * _squareSizeAdjustedForScale), SWAP_DURATION);
    _movedn = cugl::scene2::MoveBy::alloc(Vec2(0, inverseSquareFactor * -_squareSizeAdjustedForScale), SWAP_DURATION);
}

void GameScene::loadKingUI(int unitsKilled, int goal, Vec2 sq_pos, std::shared_ptr<cugl::scene2::PolygonNode> squareNode)
{

    std::string unitsNeededToKill = strtool::format("%d", goal);
    _info_text = scene2::Label::allocWithText(unitsNeededToKill, _assets->get<Font>("pixel32"));
    _info_text->setScale(5);
    _info_text->setColor(Color4::RED);
    _info_text->setPriority(0);
    _info_text->setAnchor(Vec2::ANCHOR_TOP_RIGHT);
    float inverseSquareFactor = 1 / _squareScaleFactor;
    _info_text->setPosition(Vec2(squareNode->getSize().width, squareNode->getSize().height) / squareNode->getScale() / 1.1);
    //    _info_text->setLayout(Vec2::)
    //    _info_text->setPosition(Vec2::ONE);
    //    _info_text->setPosition(Vec2(unitNode->getPosition().x+Vec2::ONE.x, unitNode->getPosition().y+Vec2::ONE.y));
    squareNode->addChildWithName(_info_text, "info");
    //    CULog("text has priority of %f", _info_text->getPriority());
    //    CULog("unit has priority of %f", unitNode->getPriority());
}

void GameScene::showResultText(bool success, std::shared_ptr<cugl::scene2::SceneNode> node)
{
    std::string resultText;
    if (success)
    {
        resultText = strtool::format("You win!");
    }
    else
    {
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

void GameScene::showTutorial(std::shared_ptr<cugl::scene2::SceneNode> node)
{
    _tutorialLayout->getChild(0)->getChildByName("level1_rule1")->setVisible(false);
    _tutorialLayout->getChild(0)->getChildByName("level1_rule2")->setVisible(false);
    _tutorialLayout->getChild(0)->getChildByName("level3_rule1")->setVisible(false);
    _tutorialLayout->getChild(0)->getChildByName("level3_rule2")->setVisible(false);
    _tutorialLayout->getChild(0)->getChildByName("level5_rule1")->setVisible(false);
    _tutorialLayout->getChild(0)->getChildByName("level6_rule1")->setVisible(false);
    _tutorialLayout->getChild(0)->getChildByName("level7_rule1")->setVisible(false);
    switch (_currLevel)
    {
    case 1:
    {
        _tutorialLayout->setVisible(true);
//        _tutorialCloseBtn->activate();
//        _tutorialLeftBtn->activate();
//        _tutorialRightBtn->activate();
        _tutorialActive = true;
        _tutorialLayout->getChild(0)->getChildByName("level1_rule1")->setVisible(true);
        _tutorial_page->setText("1/2");
        break;
    }
    case 3:
    {
        _tutorialLayout->setVisible(true);
//        _tutorialCloseBtn->activate();
//        _tutorialLeftBtn->activate();
//        _tutorialRightBtn->activate();
        _tutorialActive = true;
        _tutorialLayout->getChild(0)->getChildByName("level3_rule1")->setVisible(true);
        _tutorial_page->setText("1/2");
        break;
    }
    case 5:
    {
        _tutorialLayout->setVisible(true);
//        _tutorialCloseBtn->activate();
        _tutorialLeftBtn->setVisible(false);
        _tutorialRightBtn->setVisible(false);
        _tutorialActive = true;
        _tutorialLayout->getChild(0)->getChildByName("level5_rule1")->setVisible(true);
        _tutorial_page->setText("1/1");
        break;
    }
    case 6:
    {
        _tutorialLayout->setVisible(true);
//        _tutorialCloseBtn->activate();
        _tutorialLeftBtn->setVisible(false);
        _tutorialRightBtn->setVisible(false);
        _tutorialActive = true;
        _tutorialLayout->getChild(0)->getChildByName("level6_rule1")->setVisible(true);
        _tutorial_page->setText("1/1");
        break;
    }
    case 7:
    {
        _tutorialLayout->setVisible(true);
//        _tutorialCloseBtn->activate();
        _tutorialLeftBtn->setVisible(false);
        _tutorialRightBtn->setVisible(false);
        _tutorialActive = true;
        _tutorialLayout->getChild(0)->getChildByName("level7_rule1")->setVisible(true);
        _tutorial_page->setText("1/1");
        break;
    }
    default:
    {
        _tutorialLayout->setVisible(false);
        _tutorialCloseBtn->deactivate();
        _tutorialLeftBtn->deactivate();
        _tutorialRightBtn->deactivate();
        _tutorialActive = false;
    }
    }
}

void GameScene::flipTutorialPage(std::string dir)
{
    switch (_currLevel)
    {
    case 1:
    {
        auto level1_rule1 = _tutorialLayout->getChild(0)->getChildByName("level1_rule1");
        auto level1_rule2 = _tutorialLayout->getChild(0)->getChildByName("level1_rule2");
        _tutorialLayout->getChild(0)->getChildByName("level1_rule1")->setVisible(!level1_rule1->isVisible());
        _tutorialLayout->getChild(0)->getChildByName("level1_rule2")->setVisible(!level1_rule2->isVisible());
        if (level1_rule1->isVisible())
            _tutorial_page->setText("1/2");
        else
            _tutorial_page->setText("2/2");
        break;
    }
    case 3:
    {
        auto level3_rule1 = _tutorialLayout->getChild(0)->getChildByName("level3_rule1");
        auto level3_rule2 = _tutorialLayout->getChild(0)->getChildByName("level3_rule2");
        _tutorialLayout->getChild(0)->getChildByName("level3_rule1")->setVisible(!level3_rule1->isVisible());
        _tutorialLayout->getChild(0)->getChildByName("level3_rule2")->setVisible(!level3_rule2->isVisible());
        if (level3_rule1->isVisible())
            _tutorial_page->setText("1/2");
        else
            _tutorial_page->setText("2/2");
        break;
    }
//    case 5:
//    {
//        auto level4_rule1 = _tutorialLayout->getChild(0)->getChildByName("level5_rule1");
//    }
//        auto level4_rule2 = _tutorialLayout->getChild(0)->getChildByName("level4_rule2");
//        auto level4_rule3 = _tutorialLayout->getChild(0)->getChildByName("level4_rule3");
//        if ((dir == "left" && level4_rule1->isVisible()) || (dir == "right" && level4_rule2->isVisible()))
//        {
//            _tutorialLayout->getChild(0)->getChildByName("level4_rule1")->setVisible(false);
//            _tutorialLayout->getChild(0)->getChildByName("level4_rule2")->setVisible(false);
//            _tutorialLayout->getChild(0)->getChildByName("level4_rule3")->setVisible(true);
//        }
//        else if ((dir == "left" && level4_rule2->isVisible()) || (dir == "right" && level4_rule3->isVisible()))
//        {
//            _tutorialLayout->getChild(0)->getChildByName("level4_rule1")->setVisible(true);
//            _tutorialLayout->getChild(0)->getChildByName("level4_rule2")->setVisible(false);
//            _tutorialLayout->getChild(0)->getChildByName("level4_rule3")->setVisible(false);
//        }
//        else
//        {
//            _tutorialLayout->getChild(0)->getChildByName("level4_rule1")->setVisible(false);
//            _tutorialLayout->getChild(0)->getChildByName("level4_rule2")->setVisible(true);
//            _tutorialLayout->getChild(0)->getChildByName("level4_rule3")->setVisible(false);
//        }
//        if (level4_rule1->isVisible())
//            _tutorial_page->setText("1/3");
//        else if (level4_rule2->isVisible())
//            _tutorial_page->setText("2/3");
//        else
//            _tutorial_page->setText("3/3");
//        break;
//    }
    }
}

void GameScene::setTutorial() {
//    if ((_currLevel >= 1 && _currLevel <= 7) || _currLevel == 16 || _currLevel == 20 || _currLevel == 25) {
    if (_currLevel >= 1 && _currLevel <= 7) {
        _tutorialNode = scene2::SpriteNode::alloc(_tutorialTextures.at("tutorial_"+std::to_string(_currLevel)), 1, animationFrameCounts.at(ANIMATION_TYPE::TUTORIAL));
        _tutorialNode->setPosition(0.5*_dimen.width, 0.5*_dimen.height);
        _tutorialNode->setScale(0.27f);
        _tutorialLayout2->removeAllChildren();
        _tutorialLayout2->addChildWithName(_tutorialNode, "tutorialNode");
        _tutorialLayout2->setVisible(true);
    } else {
        _tutorialLayout2->setVisible(false);
    }
}
