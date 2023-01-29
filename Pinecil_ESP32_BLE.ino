#include "BLEDevice.h"

//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "Pinecil"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID bmeServiceUUID("9eae1adb-9d0d-48c5-a6e7-ae93f0ea37b0");

// BLE Characteristics
//Temperature Celsius Characteristic
static BLEUUID temperatureCharacteristicUUID("00000001-0000-1000-8000-00805f9b34fb");

//Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

//Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;

//Characteristicd that we want to read
static BLERemoteCharacteristic* temperatureCharacteristic;

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient();

  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(bmeServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(bmeServiceUUID.toString().c_str());
    return (false);
  }

  // Obtain a reference to the characteristics in the service of the remote BLE server.
  temperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharacteristicUUID);

  if (temperatureCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");

  //Assign callback functions for the Characteristics
  return true;
}

//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
        advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
        pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
        doConnect = true; //Set indicator, stating that we are ready to connect
        Serial.println("Device found. Connecting!");
      }
    }
};

void setup() {

  //Start serial communication
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

  //Init BLE device
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connected = true;
      uint32_t value = temperatureCharacteristic->readUInt32();
      Serial.println(value);
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  } else if (connected) {
    uint32_t value = temperatureCharacteristic->readUInt32();
    Serial.println(value);
  }
  //delay(50); // Delay a second between loops.
}
