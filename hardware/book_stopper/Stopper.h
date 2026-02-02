#ifndef STOPPER_H
#define STOPPER_H

#include <ArduinoJson.h>
#include <RotaryEncoder.h>
#include <LiquidCrystal_I2C.h>

#include "Book.h"
#include "BookManager.h"
#include "Hardware.h"
#include "BLEJson.h"

enum STATE { DISCONNECTING, CONNECTING, PREPARING, READING };


class RxCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    String line = String(pCharacteristic->getValue().c_str());

    if (line.length() == 0) return;

    Serial.print("RX <- ");
    Serial.println(line);

    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc, line);
    if (err) {
      Serial.print("JSON Error: ");
      Serial.println(err.c_str());
      return;
    }

    const char* type = doc["type"] | "unknown";

    if (strcmp(type, "REGISTER_BOOK") == 0) {
      int id       = doc["id"]    | 0;
      String title = doc["title"] | "";
      int page     = doc["page"]  | 0;

      if (id != 0 && title.length() != 0 && page >= 0) {
        BookManager::getInstance().registerBook(id, title, page);
      }
      /*
      StaticJsonDocument<128> res;
      res["type"] = "ack";
      String resStr;
      serializeJson(res, resStr);
      txChar->setValue((uint8_t*)resStr.c_str(), resStr.length());
      txChar->notify();
      */
    } else if (strcmp(type, "INIT_REGISTER_BOOK") == 0) {
      BookManager::getInstance().init();
    }
  }
};

extern RotaryEncoder encoder;
void IRAM_ATTR handleEncoder();

class Stopper {
private:
  Button clipBtn = Button(18, LOW);
  Button shortcutBtn = Button(17);
  Button collisionBtn = Button(5);

  LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

  STATE state = DISCONNECTING;

  long pos, prevPos = -999;

  Stopper() {}

  void sendCurrentBook(String type);

  void initLcd() {
    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
    lcd.clear();

    transitionTo(state);
  }

  void initRotaryEncoder() {
    pinMode(32, INPUT_PULLUP);
    pinMode(33, INPUT_PULLUP);

    encoder.setPosition(0);

    attachInterrupt(digitalPinToInterrupt(32), handleEncoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(33), handleEncoder, CHANGE);
  }

  void transitionTo(STATE _state) {
    state = _state;
    switch (state) {
      case DISCONNECTING: BookManager::getInstance().init(); break;
      case    CONNECTING: break;
      case     PREPARING: prevPos = pos; break;
      case       READING: prevPos = pos; break;
    }
    lcd.clear(); printState();
  }

  void printState() {
    lcd.setCursor(0, 0);
    switch (state) {
      case DISCONNECTING: lcd.clear(); lcd.print(" [disconnecting]"); break;
      case    CONNECTING: lcd.clear(); lcd.print("    [connecting]"); break;
      case     PREPARING:              lcd.print("     [preparing]"); break;
      case       READING:              lcd.print("       [reading]"); break;
    }
  }

  void printBook() {
    Book *book = BookManager::getInstance().getCurrentBook();

    String text = "";
    text += (book->id / 1000) > 0 ? "" : "0";
    text += (book->id /  100) > 0 ? "" : "0";
    text += (book->id /   10) > 0 ? "" : "0";
    text += String(book->id);
    // text += "| " + book->title + "            ";

    lcd.setCursor(0, 1); lcd.print(text);
  }

  void printPage() {
    Book *book = BookManager::getInstance().getCurrentBook();

    String text = "";
    text += (book->id / 1000) > 0 ? "" : "0";
    text += (book->id /  100) > 0 ? "" : "0";
    text += (book->id /   10) > 0 ? "" : "0";
    text += String(book->id);
    text += "| page: ";
    text += (book->page / 100) > 0 ? "" : "0";
    text += (book->page /  10) > 0 ? "" : "0";
    text += String(book->page) + "  ";

    lcd.setCursor(0, 1); lcd.print(text);
  }

  void handleShortcut();
  void handleTurnPage();
  void handleSelectBook();
  void handleEditPage();

  void handleAdvertiseBLE();
  void handleConnBLE();
  void handleDisconnBLE();
  void handleOpen();
  void handleClose();

public:
  static Stopper& getInstance() {
    static Stopper instance;
    return instance;
  }

  Stopper(const Stopper&) = delete;
  Stopper& operator=(const Stopper&) = delete;

  void init() {
    clipBtn.init();
    shortcutBtn.init();
    collisionBtn.init();
    initLcd();
    initRotaryEncoder();
    
    BookManager::getInstance().init();

    BLEJson::getInstance().init(new RxCallback());
    BLEJson::getInstance().start();
  }

  void run() {
    pos = encoder.getPosition();
    
    switch (state) {
      case DISCONNECTING:
        handleAdvertiseBLE();
        break;

      case CONNECTING:
        handleConnBLE();
        break;

      case PREPARING:
        handleSelectBook();

        handleDisconnBLE();
        handleOpen();

        printBook();
        break;

      case READING:
        handleTurnPage();
        handleEditPage();
        handleShortcut();

        handleDisconnBLE();
        handleClose();

        printPage();
        break;
    }
  }
};

#endif