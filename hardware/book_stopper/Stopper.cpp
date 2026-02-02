#include "Stopper.h"

RotaryEncoder encoder(32, 33, RotaryEncoder::LatchMode::TWO03);

void IRAM_ATTR handleEncoder() {
  encoder.tick();
}

void Stopper::sendCurrentBook(String type) {
  Book *book = BookManager::getInstance().getCurrentBook();
  if (book == nullptr) return;

  StaticJsonDocument<128> doc;
  doc["type"] = type;
  doc["id"] = book->id;
  doc["title"] = book->title;
  doc["page"] = book->page;

  BLEJson::getInstance().sendJson(doc);
}

void Stopper::handleShortcut() {
  if (shortcutBtn.isPressed()) {
    sendCurrentBook("SHORTCUT");
  }
}

void Stopper::handleTurnPage() {
  if (collisionBtn.isPressed()) {
    BookManager::getInstance().turnPageForward();
    BookManager::getInstance().turnPageForward();
    sendCurrentBook("TURN_PAGE");
  }
}

void Stopper::handleSelectBook() {
  if (pos != prevPos) {
    if (pos < prevPos) BookManager::getInstance().rotateLeft();
    else               BookManager::getInstance().rotateRight();
    
    sendCurrentBook("SELECT_BOOK");
    prevPos = pos;
  }
}

void Stopper::handleEditPage() {
  if (pos != prevPos) {
    if (pos < prevPos) BookManager::getInstance().turnPageForward();
    else               BookManager::getInstance().turnPageBackward();
    
    sendCurrentBook("EDIT_PAGE");
    prevPos = pos;
  }
}

void Stopper::handleAdvertiseBLE() {
  BLEJson::getInstance().advertise();
  transitionTo(CONNECTING);
}

void Stopper::handleConnBLE() {
  if (BLEJson::getInstance().isConnected()) {
    transitionTo(PREPARING);
  }
}

void Stopper::handleDisconnBLE() {
  if (!BLEJson::getInstance().isConnected()) {
    transitionTo(DISCONNECTING);
  }
}

void Stopper::handleOpen() {
  if (!clipBtn.isHeld() && BookManager::getInstance().getCurrentBook()->id) {
    sendCurrentBook("START");
    delay(100);
    transitionTo(READING);
  }
}

void Stopper::handleClose() {
  if (clipBtn.isHeld()) {
    sendCurrentBook("END");
    delay(100);
    transitionTo(PREPARING);
  }
}