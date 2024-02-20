#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "OneButton.h"

#define TFT_WIDTH 170
#define TFT_HEIGHT 320

#define UP_BUTTON 0
#define DOWN_BUTTON 14

OneButton menuButton(DOWN_BUTTON, true);

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite timer1_sprite = TFT_eSprite(&tft);
TFT_eSprite goal_sprite = TFT_eSprite(&tft);
TFT_eSprite info_sprite = TFT_eSprite(&tft);
TFT_eSprite logo_sprite = TFT_eSprite(&tft);
TFT_eSprite menu0 = TFT_eSprite(&tft);
TFT_eSprite menu1 = TFT_eSprite(&tft);
TFT_eSprite menu2 = TFT_eSprite(&tft);
TFT_eSprite menu_logo = TFT_eSprite(&tft);

uint8_t gameMode = 0;  // 0简单 1中等 2困难
uint8_t gameState = 0; // 0菜单 1初始 2预备 3开始 4结束

int timer1 = 0; // 计时器值

bool drawFlag = true;

unsigned long prevMillis = 0;
unsigned long currMillis = 0;

int goal = 111; // 目标数值
int goal2 = 11; // 目标数值2，用于高级难度

int upBtnState = 0;     // current state of the button
int lastUpBtnState = 0; // previous state of the button

/***** 游戏进程相关 *****/
// 更新目标数值
void updateGoal()
{
  goal_sprite.setTextDatum(TL_DATUM);
  goal_sprite.setTextSize(2);
  goal_sprite.fillSprite(TFT_BROWN);
  goal_sprite.setTextColor(TFT_LIGHTGREY, TFT_BROWN);
  goal_sprite.drawString("Target", 3, 3);
  goal_sprite.setTextDatum(TC_DATUM);
  goal_sprite.setTextSize(4);
  goal_sprite.setTextColor(TFT_WHITE, TFT_BROWN);
  Serial.println("Goal: " + String(goal));

  if (goal == -1)
  {
    goal_sprite.pushSprite(0, 0);
    return;
  }

  if (gameMode == 0)
  {
    goal_sprite.drawString(String(goal / 10), goal_sprite.width() / 2, 35);
  }
  else if (gameMode == 1)
  {
    goal_sprite.drawString(String(goal / 10) + "." + String(goal % 10), goal_sprite.width() / 2, 35);
  }
  else if (gameMode == 2)
  {
    goal2 = random(1, 20);
    goal_sprite.setTextSize(3);
    goal_sprite.drawString(String(goal / 10) + "." + String(goal % 10), goal_sprite.width() / 2, 22);
    goal_sprite.drawString("+" + String(goal2 / 10) + "." + String(goal2 % 10), goal_sprite.width() / 2, 48);
  }
  goal_sprite.pushSprite(0, 0);
}

// 更新信息区域
void updateInfo(String msg)
{
  info_sprite.fillSprite(TFT_DARKGREY);
  info_sprite.drawString(msg, info_sprite.width() / 2, info_sprite.height() / 2);
  info_sprite.pushSprite(0, 74);
}

// 更新计时器区域的信息
void updateTimerMessage(String msg)
{
  timer1_sprite.setTextSize(3);
  timer1_sprite.fillSprite(TFT_LIGHTGREY);
  timer1_sprite.drawString(msg, timer1_sprite.width() / 2, timer1_sprite.height() / 2);
  timer1_sprite.pushSprite(0, 100);
}

// 更新计时器
void updateTimer()
{
  timer1++;

  if (timer1 == 200) // 超过20秒，直接结束游戏
  {
    gameState = 4;
    drawFlag = true;
    return;
  }

  timer1_sprite.fillSprite(TFT_LIGHTGREY);
  timer1_sprite.setTextColor(TFT_BLACK);
  timer1_sprite.setTextSize(7);
  if (gameMode == 0)
  {
    timer1_sprite.drawString(String(timer1 / 10), timer1_sprite.width() / 2, timer1_sprite.height() / 2);
  }
  else
  {
    timer1_sprite.drawString(String(timer1 / 10) + "." + String(timer1 % 10), timer1_sprite.width() / 2, timer1_sprite.height() / 2);
  }
  timer1_sprite.pushSprite(0, 100);
}

// 读取按键状态
void readButton()
{
  upBtnState = digitalRead(UP_BUTTON);

  // compare the buttonState to its previous state
  if (upBtnState != lastUpBtnState)
  {
    // if the state has changed, increment the counter
    if (upBtnState == HIGH)
    {
      if (gameState == 2) // 预备->开始
      {
        gameState = 3;
        drawFlag = true;
        if (gameMode == 2)
        {
          goal = random(50, 100);
          updateGoal();
        }
      }

      Serial.println("hand released");
    }
    else
    {
      if (gameState == 1) // 初始->预备
      {
        gameState = 2;
        drawFlag = true;
        if (gameMode == 0 || gameMode == 1)
        {
          goal = random(70, 140);
          updateGoal();
        }
      }
      else if (gameState == 3) // 开始->结束
      {
        gameState = 4;
        drawFlag = true;
      }
      else if (gameState == 4) // 结束->初始
      {
        gameState = 1;
        timer1 = 0;
        drawFlag = true;
        goal = -1;
        updateGoal();
      }

      Serial.println("hand pressed");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  lastUpBtnState = upBtnState;
}

void createGameSprite()
{
  timer1_sprite.createSprite(320, 70);
  timer1_sprite.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  timer1_sprite.setTextDatum(MC_DATUM);

  goal_sprite.createSprite(100, 74);
  // updateGoal();

  info_sprite.createSprite(320, 26);
  info_sprite.setTextColor(TFT_BLACK, TFT_DARKGREY);
  info_sprite.setTextDatum(MC_DATUM);
  info_sprite.setTextSize(2);
  updateInfo("PRESS & HOLD");

  logo_sprite.createSprite(220, 74);
  logo_sprite.fillSprite(TFT_BLACK);
  logo_sprite.setTextColor(TFT_WHITE);
  logo_sprite.setTextDatum(TC_DATUM);
  logo_sprite.setTextSize(2);
  logo_sprite.drawString("Created by", logo_sprite.width() / 2, 5);
  logo_sprite.setTextSize(4);
  logo_sprite.drawString("XSTER", logo_sprite.width() / 2, 30);
  logo_sprite.pushSprite(100, 0);
}

void removeGameSprite()
{
  timer1_sprite.deleteSprite();
  goal_sprite.deleteSprite();
  info_sprite.deleteSprite();
  logo_sprite.deleteSprite();
}

// 画获胜界面
void drawWinScreen(String msg)
{
  timer1_sprite.fillSprite(TFT_BROWN);
  timer1_sprite.setTextColor(TFT_BLACK);
  timer1_sprite.setTextSize(7);
  timer1_sprite.drawString(msg, timer1_sprite.width() / 2, timer1_sprite.height() / 2);
  timer1_sprite.pushSprite(0, 100);
  updateInfo("YOU WIN!!!");
}

void processGameState()
{
  if (gameState == 0) // 菜单
    /*** 菜单阶段 ***/
    delay(10);
  else
  {
    /*** 游戏阶段 ***/
    if (gameState == 3) // 开始
    {
      currMillis = millis();
      if (currMillis - prevMillis >= 100)
      {
        updateTimer();
        prevMillis = currMillis;
      }

      if (drawFlag)
      {
        drawFlag = false;
        updateInfo("PRESS TO STOP");
      }
    }
    else if (gameState == 1 && drawFlag) // 加载完毕
    {
      drawFlag = false;
      updateTimerMessage("Game Ready");
      updateInfo("PRESS & HOLD");
    }
    else if (gameState == 2 && drawFlag) // 预备
    {
      drawFlag = false;
      updateTimerMessage("RELEASE TO START");
    }
    else if (gameState == 4 && drawFlag) // 结束
    {
      drawFlag = false;
      if (gameMode == 0) // 简单模式
      {
        if (timer1 / 10 == goal / 10) // WIN
          drawWinScreen(String(timer1 / 10));
        else
          updateInfo("GAMEOVER");
      }
      else if (gameMode == 1) // 中等模式
      {
        if (timer1 == goal)
          drawWinScreen(String(timer1 / 10) + "." + String(timer1 % 10));
        else
          updateInfo("GAMEOVER");
      }
      else if (gameMode == 2) // 困难模式
      {
        if (timer1 == goal + goal2)
          drawWinScreen(String(timer1 / 10) + "." + String(timer1 % 10));
        else
          updateInfo("GAMEOVER");
      }
    }

    // read button status
    readButton();
  }
}

/***** 游戏进程相关（end） *****/

/***** 菜单相关 *****/

void _createMenu(TFT_eSprite *menu, String text, int mode)
{
  menu->setTextSize(4);
  menu->setTextDatum(TC_DATUM);
  if (gameMode == mode)
  {
    menu->fillSprite(TFT_BROWN);
    menu->setTextColor(TFT_BLACK, TFT_BROWN);
  }
  else
  {
    menu->fillSprite(TFT_BLACK);
    menu->setTextColor(TFT_WHITE, TFT_BLACK);
  }
  menu->drawString(text, menu->width() / 2, 10);
}

// 更新菜单项
void updateMenu()
{
  _createMenu(&menu0, "NOOOOB", 0);
  menu0.pushSprite(0, 0);

  _createMenu(&menu1, "OKAY~", 1);
  menu1.pushSprite(0, 50);

  _createMenu(&menu2, "HELL!!!", 2);
  menu2.pushSprite(0, 100);
}

void createMenuSprite()
{
  menu0.createSprite(320, 50);
  menu1.createSprite(320, 50);
  menu2.createSprite(320, 50);
  updateMenu();

  menu_logo.createSprite(320, 20);
  menu_logo.fillSprite(TFT_BLACK);
  menu_logo.setTextColor(TFT_WHITE);
  menu_logo.setTextDatum(TR_DATUM);
  menu_logo.setTextSize(1);
  menu_logo.drawString("Created by XSTER", menu_logo.width() - 5, 2);
  menu_logo.pushSprite(0, menu0.height() * 3);
}

void removeMenuSprite()
{
  menu0.deleteSprite();
  menu1.deleteSprite();
  menu2.deleteSprite();
  menu_logo.deleteSprite();
}

// 下一个难度
void nextGameMode()
{
  if (gameState != 0)
    return;

  Serial.println("Next");
  gameMode++;
  if (gameMode == 3)
    gameMode = 0;

  updateMenu();
}

// 进入游戏
void enterGame()
{
  if (gameState != 0)
    return;

  Serial.println("Enter");
  tft.fillScreen(TFT_BLACK);
  removeMenuSprite();
  createGameSprite();
  drawFlag = true;
  gameState = 1;
  goal = -1;
  timer1 = 0;
  updateGoal();
}

// 返回菜单
void backToMenu()
{
  if (gameState == 0)
    return;

  Serial.println("Back");
  tft.fillScreen(TFT_BLACK);
  removeGameSprite();
  createMenuSprite();
  gameState = 0;
}

/***** 菜单相关（end） *****/

void setup()
{
  Serial.begin(115200);
  randomSeed(analogRead(1));

  pinMode(UP_BUTTON, INPUT_PULLUP);

  menuButton.attachClick(nextGameMode);
  menuButton.attachDoubleClick(enterGame);
  menuButton.attachLongPressStart(backToMenu);
  menuButton.setLongPressIntervalMs(1000);

  // 初始化屏幕
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  createMenuSprite();
}

void loop()
{
  menuButton.tick();

  // 处理游戏进程相关
  processGameState();
}
