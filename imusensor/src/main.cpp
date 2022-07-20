#include <M5Unified.h>
#include <BluetoothSerial.h>
#include "imu/ImuReader.h"
#include "imu/AverageCalc.h"
#include "input/ButtonCheck.h"
#include "input/ButtonData.h"
#include "session/SessionData.h"
#include "prefs/Settings.h"
#include "device_name/DeviceName.h"
#include "beep/Speaker.h"

#define TASK_DEFAULT_CORE_ID 1
#define TASK_STACK_DEPTH 4096UL
#define TASK_NAME_IMU "IMUTask"
#define TASK_NAME_WRITE_SESSION "WriteSessionTask"
#define TASK_NAME_READ_SESSION "ReadSessionTask"
#define TASK_NAME_BUTTON "ButtonTask"
#define TASK_SLEEP_IMU 5           // = 1000[ms] / 200[Hz]
#define TASK_SLEEP_WRITE_SESSION 40  // = 1000[ms] / 25[Hz]
#define TASK_SLEEP_READ_SESSION 100 // = 1000[ms] / 10[Hz]
#define TASK_SLEEP_BUTTON 1         // = 1000[ms] / 1000[Hz]
#define MUTEX_DEFAULT_WAIT 1000UL

static void ImuLoop(void *arg);
static void WriteSessionLoop(void *arg);
static void ReadSessionLoop(void *arg);
static void ButtonLoop(void *arg);

imu::ImuReader *imuReader;
BluetoothSerial btSpp;
input::ButtonCheck button;

imu::ImuData imuData;
input::ButtonData btnData;
SPEAKER speaker;
bool hasButtonUpdate = false;
static SemaphoreHandle_t imuDataMutex = NULL;
static SemaphoreHandle_t btnDataMutex = NULL;

uint8_t readBuffer[session::data_length::max] = {0};
bool gyroOffsetInstalled = false;
bool restartGyroInstalled = true;
bool isDetected = false;
imu::AverageCalcXYZ gyroAve;
prefs::Settings settingPref;
device_name::DeviceName deviceName("els_demo");

void UpdateLcd()
{
  M5.Lcd.setCursor(40, 40);
  if (gyroOffsetInstalled == true)
  {
    /*speaker.setBeep(1000, 300);
    speaker.tone(1000);
    speaker.setVolume(0xFF);*/
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Start to record");

    /*speaker.mute();*/
  }
  else if (gyroOffsetInstalled == false && isDetected == false)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Stop to record");
  }
  else if (gyroOffsetInstalled == false && isDetected == true)
  {
    // speaker.setBeep(1000,1);
    speaker.tone(1000);
    M5.Lcd.println("Detect result:");
    M5.Lcd.println("speed too slow");
    speaker.mute();
  }
}

float gyroOffset[3] = {0.0F};
void setup()
{
  M5.begin();
  Serial.begin(115200);
  // read settings
  // float gyroOffset[3] = { 0.0F };
  settingPref.begin();
  // settingPref.clear(); // to reinstall gyro offset by only m5stickc remove commentout
  settingPref.readGyroOffset(gyroOffset);
  settingPref.finish();
  // lcd
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(1);
  UpdateLcd();
  // speaker
  speaker.begin();
  // imu
  imuReader = new imu::ImuReader(M5.Imu);
  imuReader->initialize();
  // if (gyroOffsetInstalled) {
  //   imuReader->writeGyroOffset(gyroOffset[0], gyroOffset[1], gyroOffset[2]);
  // }
  //  bluetooth serial
  btSpp.begin(deviceName.getName(M5.getBoard()));
  // task
  imuDataMutex = xSemaphoreCreateMutex();
  btnDataMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(ImuLoop, TASK_NAME_IMU, TASK_STACK_DEPTH,
                          NULL, 2, NULL, TASK_DEFAULT_CORE_ID);
  xTaskCreatePinnedToCore(WriteSessionLoop, TASK_NAME_WRITE_SESSION, TASK_STACK_DEPTH,
                          NULL, 1, NULL, TASK_DEFAULT_CORE_ID);
  xTaskCreatePinnedToCore(ReadSessionLoop, TASK_NAME_READ_SESSION, TASK_STACK_DEPTH,
                          NULL, 1, NULL, TASK_DEFAULT_CORE_ID);
  xTaskCreatePinnedToCore(ButtonLoop, TASK_NAME_BUTTON, TASK_STACK_DEPTH,
                          NULL, 1, NULL, TASK_DEFAULT_CORE_ID);
}

void loop()
{ /* Do Nothing */
}

static void ImuLoop(void *arg)
{
  while (1)
  {
    M5.update();

    if (M5.BtnA.wasPressed() && gyroOffsetInstalled == false)
    {
      isDetected = false;
      gyroOffsetInstalled = true;
      UpdateLcd();
    }
    else if (M5.BtnA.wasPressed() && gyroOffsetInstalled == true)
    {
      isDetected = false;
      gyroOffsetInstalled = false;
      UpdateLcd();
    }
    else if (gyroOffsetInstalled == false && isDetected == true)
    {
      UpdateLcd();
    }

    uint32_t entryTime = millis();
    if (xSemaphoreTake(imuDataMutex, MUTEX_DEFAULT_WAIT) == pdTRUE)
    {
      imuReader->update();
      imuReader->read(imuData);
      imuReader->acc_rotate(imuData);
      if (!gyroOffsetInstalled)
      {
        if (!gyroAve.push(imuData.gyro[0], imuData.gyro[1], imuData.gyro[2]))
        {
          float x = gyroAve.averageX();
          float y = gyroAve.averageY();
          float z = gyroAve.averageZ();
          // set offset
          imuReader->writeGyroOffset(x, y, z);
          // save offset
          float offset[] = {x, y, z};
          settingPref.begin();
          settingPref.writeGyroOffset(offset);
          settingPref.finish();
          // gyroOffsetInstalled = true;
          gyroAve.reset();
          // UpdateLcd();
        }
      }
    }
    xSemaphoreGive(imuDataMutex);
    // idle
    int32_t sleep = TASK_SLEEP_IMU - (millis() - entryTime);
    vTaskDelay((sleep > 0) ? sleep : 0);
  }
}

static void WriteSessionLoop(void *arg)
{
  static session::SessionData imuSessionData(session::DataDefineImu);
  static session::SessionData btnSessionData(session::DataDefineButton);
  while (1)
  {
    uint32_t entryTime = millis();
    // imu
    if (gyroOffsetInstalled)
    {
      if (xSemaphoreTake(imuDataMutex, MUTEX_DEFAULT_WAIT) == pdTRUE)
      {
        imuSessionData.write((uint8_t *)&imuData, imu::ImuDataLen);
        btSpp.write((uint8_t *)&imuSessionData, imuSessionData.length());
      }
      xSemaphoreGive(imuDataMutex);
    }
    // button
    if (xSemaphoreTake(btnDataMutex, MUTEX_DEFAULT_WAIT) == pdTRUE)
    {
      if (hasButtonUpdate)
      {
        btnSessionData.write((uint8_t *)&btnData, input::ButtonDataLen);
        btSpp.write((uint8_t *)&btnSessionData, btnSessionData.length());
        hasButtonUpdate = false;
      }
      xSemaphoreGive(btnDataMutex);
    }
    // idle
    int32_t sleep = TASK_SLEEP_WRITE_SESSION - (millis() - entryTime);
    vTaskDelay((sleep > 0) ? sleep : 0);
  }
}

static void ReadSessionLoop(void *arg)
{
  while (1)
  {
    uint32_t entryTime = millis();
    // read
    size_t len = btSpp.readBytes(readBuffer, session::data_length::header);
    if (len == (size_t)session::data_length::header)
    {
      uint16_t dataId = (uint16_t)((readBuffer[1] << 8) | readBuffer[0]);
      if (dataId == session::data_type::installGyroOffset && gyroOffsetInstalled)
      {
        gyroOffsetInstalled = false;
        imuReader->writeGyroOffset(0.0F, 0.0F, 0.0F);
        UpdateLcd();
      }
      else if (dataId == session::data_type::restartGyro && restartGyroInstalled)
      {
        gyroOffsetInstalled = true;
        imuReader->writeGyroOffset(0.0F, 0.0F, 0.0F);
        UpdateLcd();
      }
      else if (dataId == session::data_type::detectResult)
      {
        isDetected = true;
      }
    }
    // idle
    int32_t sleep = TASK_SLEEP_READ_SESSION - (millis() - entryTime);
    vTaskDelay((sleep > 0) ? sleep : 0);
  }
}

static void ButtonLoop(void *arg)
{
  uint8_t btnFlag = 0;
  while (1)
  {
    uint32_t entryTime = millis();
    M5.update();

    if (button.containsUpdate(M5, btnFlag))
    {
      for (int i = 0; i < INPUT_BTN_NUM; i++)
      {
        if (xSemaphoreTake(btnDataMutex, MUTEX_DEFAULT_WAIT) == pdTRUE)
        {
          btnData.timestamp = millis();
          btnData.btnBits = btnFlag;
          hasButtonUpdate = true;
        }
        xSemaphoreGive(btnDataMutex);
      }
    }

    // idle
    int32_t sleep = TASK_SLEEP_BUTTON - (millis() - entryTime);
    vTaskDelay((sleep > 0) ? sleep : 0);
  }
}
