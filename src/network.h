#pragma once

#include <iostream>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string>
#include <vector>
#include "esp_now.h"
#include "esp_wifi.h"
#include "game.h"

class Game;
typedef struct RemoteTick RemoteTick;

class Network {
public:
    Network(Game* game);
    void init();
    void deinit();
    void resetDiscovered();
    void enableDiscovery();
    void discover();
    std::vector<uint8_t*> getDiscovered();
    int getChannel();
    uint8_t* getMac();
    String getMacString();
    String stringFromMac(uint8_t* mac);
    uint8_t* macFromString(String mac);
    SemaphoreHandle_t getRemoteTickMutex();
    void addDiscoveredPeer(uint8_t* mac);
    void setPeer(uint8_t* mac);
    void sendTick(RemoteTick* tick);
    RemoteTick* receiveTick();
    void setRemoteTick(std::unique_ptr<RemoteTick> tick);
    void waitJoinResponse();
    void requestJoin(uint8_t* mac);
    void acceptJoin();
    void declineJoin();
    void setMultiplayerHandlers();
private:
    Game* game;
    uint8_t channel;
    std::vector<uint8_t*> discoveredPeers;
    std::unique_ptr<RemoteTick> remoteTick;
    SemaphoreHandle_t remoteTickMutex;
    bool isNewTick;

    static void discoveryRequestCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len);
    static void discoveryResponseCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len);
    static void joinRequestCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len);
    static void joinResponseCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len);
    static void remoteTickCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len);
};