#include <FS.h>
#include "Free_Fonts.h"

#include <TFT_eSPI.h>
#include <TFT_eWidget.h>

TFT_eSPI tft = TFT_eSPI();

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

ButtonWidget btnL = ButtonWidget(&tft);
ButtonWidget btnR = ButtonWidget(&tft);
ButtonWidget btnDispense = ButtonWidget(&tft);

ButtonWidget btnClose = ButtonWidget(&tft);

#define BUTTON_W 200
#define BUTTON_H 100

ButtonWidget* btn[] = {&btnL, &btnR, &btnDispense, &btnClose};
uint8_t buttonCount = sizeof(btn) / sizeof(btn[0]);

bool medstanceState = false;
bool yourMedAlarmState = false;
bool medEmergencyState = false;

void btnL_pressAction(void);
void btnR_pressAction(void);
void btnL_releaseAction(void);
void btnR_releaseAction(void);
void btnDispense_pressAction(void);
void btnDispense_releaseAction(void);
void btnClose_pressAction(void);

void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF18);

  touch_calibrate();
  initButtons();
}

void loop()
{
  static uint32_t scanTime = millis();
  uint16_t t_x = 9999, t_y = 9999;

  if (millis() - scanTime >= 50)
  {
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    for (uint8_t b = 0; b < buttonCount; b++)
    {
      if (pressed)
      {
        if (btn[b]->contains(t_x, t_y))
        {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else
      {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  if (!LittleFS.begin())
  {
    Serial.println("Formatting file system");
    LittleFS.format();
    LittleFS.begin();
  }

  if (LittleFS.exists(CALIBRATION_FILE))
  {
    if (REPEAT_CAL)
    {
      LittleFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f)
      {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL)
  {
    tft.setTouch(calData);
  }
  else
  {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL)
    {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void btnL_pressAction(void)
{
  if (btnL.justPressed())
  {
    Serial.println("New Alarm button just pressed");
    medstanceState = true;
    yourMedAlarmState = false;
    setButtonVisibility(false);
    drawMedstanceText();
  }
}

void btnR_pressAction(void)
{
    if (btnR.justPressed())
    {
        Serial.println("My Alarm button just pressed");
        yourMedAlarmState = true;
        medstanceState = false;
        setButtonVisibility(false);
        drawYourMedAlarmText();
    }
}

void btnDispense_pressAction(void)
{
  if (btnDispense.justPressed())
  {
    Serial.println("Dispense Now button just pressed");
    medEmergencyState = true;
    medstanceState = false;
    yourMedAlarmState = false;
    setButtonVisibility(false);
    drawMedEmergencyText();
  }
}

void btnClose_pressAction(void)
{
  if (btnClose.justPressed())
  {
    Serial.println("Close button just pressed");
    medstanceState = false;
    yourMedAlarmState = false;
    medEmergencyState = false;
    clearMedstanceText();
    clearYourMedAlarmText();
    clearMedEmergencyText();
    setButtonVisibility(true);
  }
}

void btnL_releaseAction(void)
{
  // Handle release action for btnL
}

void btnR_releaseAction(void)
{
  // Handle release action for btnR
}

void btnDispense_releaseAction(void)
{
  // Handle release action for btnDispense
}


void setButtonVisibility(bool visible)
{
  int8_t visibility = visible ? 1 : 0;

  for (uint8_t b = 0; b < buttonCount; b++)
  {
    btn[b]->setPivot(-100, -100);  // Move the button off-screen
  }

  if (visible)
  {
    // Set the initial button positions when visible
    initButtons();
  }
}

void initButtons()
{
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 4;

  btnL.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_CYAN, "New Alarm", 1);
  btnL.setPressAction(btnL_pressAction);
  btnL.setReleaseAction(btnL_releaseAction);
  btnL.drawSmoothButton(false, 3, TFT_BLACK);

  y += BUTTON_H + 10;
  btnR.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_GREEN, "My Alarm", 1);
  btnR.setPressAction(btnR_pressAction);
  btnR.setReleaseAction(btnR_releaseAction);
  btnR.drawSmoothButton(false, 3, TFT_BLACK);

  y += BUTTON_H + 10;
  btnDispense.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_YELLOW, "Take Med Now", 1);
  btnDispense.setPressAction(btnDispense_pressAction);
  btnDispense.setReleaseAction(btnDispense_releaseAction);
  btnDispense.drawSmoothButton(false, 3, TFT_BLACK);

  // Initialize close button for MEDSTANCE and Your Med Alarm state
  x = tft.width() - 50;  // Adjust x position
  btnClose.initButtonUL(x, 4 , 40, 40, TFT_RED, TFT_WHITE, TFT_RED, "X", 1);
  btnClose.setPressAction(btnClose_pressAction);
}

void drawMedstanceText()
{
  tft.fillScreen(TFT_BLACK);

  int textWidth = 200;  // Lebar teks "MEDSTANCE"
  int textHeight = 50;  // Tinggi teks "MEDSTANCE"

  int x = (tft.width() - textWidth) / 5;  // Posisi x di tengah horizontal
  int y = 35;  // Posisi y di atas layar, sesuaikan sesuai kebutuhan

  tft.setCursor(x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);  // Sesuaikan ukuran teks sesuai kebutuhan
  tft.println("Set Med-Alarm Here");
  btnClose.drawButton();  // Gambar tombol close untuk keadaan MEDSTANCE
}

void drawYourMedAlarmText()
{
  tft.fillScreen(TFT_BLACK);

  int textWidth = 300;  // Lebar teks "Your Med Alarm"
  int textHeight = 50;  // Tinggi teks "Your Med Alarm"

  int x = (tft.width() - textWidth) / 2;  // Posisi x di tengah horizontal
  int y = 35;  // Posisi y di atas layar, sesuaikan sesuai kebutuhan

  tft.setCursor(x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);  // Sesuaikan ukuran teks sesuai kebutuhan
  tft.println("Your Med-Alarm is Here");
  btnClose.drawButton();  // Gambar tombol close untuk keadaan Your Med Alarm
}

void drawMedEmergencyText()
{
  tft.fillScreen(TFT_BLACK);

  int textWidth = 300;  // Lebar teks "Med-Emergency Only"
  int textHeight = 50;  // Tinggi teks "Med-Emergency Only"

  int x = (tft.width() - textWidth) / 2;  // Posisi x di tengah horizontal
  int y = 35;  // Posisi y di atas layar, sesuaikan sesuai kebutuhan

  tft.setCursor(x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);  // Sesuaikan ukuran teks sesuai kebutuhan
  tft.println("Med-Emergency Only");
  btnClose.drawButton();  // Gambar tombol close untuk keadaan Med-Emergency
}


void clearMedstanceText()
{
  tft.fillScreen(TFT_BLACK);
}

void clearYourMedAlarmText()
{
  tft.fillScreen(TFT_BLACK);
}

void clearMedEmergencyText()
{
  tft.fillScreen(TFT_BLACK);
}