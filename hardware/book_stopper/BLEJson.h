#ifndef BLE_JSON_H
#define BLE_JSON_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // Write (Flutter -> ESP32)
#define CHAR_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // Notify (ESP32 -> Flutter)

// Forward declaration so we can reference BLEJson in ServerCallback definitions later
class BLEJson;

// ---------------------- ServerCallback ---------------------- //
class ServerCallback : public BLEServerCallbacks {
public:
  void onConnect(BLEServer* pServer) override;
  void onDisconnect(BLEServer* pServer) override;
};

// ------------------------ BLEJson --------------------------- //
class BLEJson {
private:
  BLECharacteristic* txChar = nullptr;
  bool deviceConnected;

  BLEServer*  pServer  = nullptr;
  BLEService* pService = nullptr;

  BLEJson() : deviceConnected(false) {}

public:
  static BLEJson& getInstance() {
    static BLEJson instance;
    return instance;
  }

  BLEJson(const BLEJson&) = delete;
  BLEJson& operator=(const BLEJson&) = delete;

  void connect()    { deviceConnected = true;  }
  void disconnect() { deviceConnected = false; }
  bool isConnected() const { return deviceConnected; }

  void init(BLECharacteristicCallbacks* rxCallback) {
    // init device
    BLEDevice::init("ESP32_JSON_BLE");

    // init server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallback());

    // init service
    pService = pServer->createService(SERVICE_UUID);

    // init tx characteristic
    txChar = pService->createCharacteristic(
      CHAR_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY |
      BLECharacteristic::PROPERTY_READ
    );
    txChar->addDescriptor(new BLE2902());

    // init rx characteristic
    BLECharacteristic* rxChar = pService->createCharacteristic(
      CHAR_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE
    );
    rxChar->setCallbacks(rxCallback);

    // start service
    pService->start();

    // init advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
  }

  void start() {
    if (pService != nullptr) {
      pService->start();
    }
  }

  void advertise() {
    BLEDevice::startAdvertising();
    Serial.println("BLE UART JSON Service started");
  }

  template<size_t N>
  void sendJson(const StaticJsonDocument<N>& doc) {
    if (!txChar) return;

    String jsonStr;
    serializeJson(doc, jsonStr);
    jsonStr += "\n";

    txChar->setValue((uint8_t*)jsonStr.c_str(), jsonStr.length());
    txChar->notify();

    // txChar->setValue((uint8_t*)"", 0);  // clear

    Serial.print("TX -> ");
    Serial.println(jsonStr);
  }
};

// ---------------- ServerCallback method defs ---------------- //

inline void ServerCallback::onConnect(BLEServer* pServer) {
  BLEJson::getInstance().connect();
  Serial.println("Device connected");
}

inline void ServerCallback::onDisconnect(BLEServer* pServer) {
  BLEJson::getInstance().disconnect();
  Serial.println("Device disconnected");
}

#endif
