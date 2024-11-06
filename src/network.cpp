#include <WiFi.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "game.h"
#include "network.h"

Network::Network(Game *game):
        channel(1),
        remoteTick(nullptr),
        isNewTick(false),
        remoteTickMutex(xSemaphoreCreateMutex()) {
    this->game = game;
    xSemaphoreGive(remoteTickMutex);
}

void Network::init() {
    char message[100];
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    esp_err_t result = esp_now_init();
    if (result != ESP_OK) {
        Serial.printf("Failed to initialize ESP-NOW: %x\n", result);
        snprintf(message, 100, "Failed to initialize ESP-NOW");
        game->getGraphics()->showMessage("Error", message);
    }
}

void Network::deinit() {
    esp_now_deinit();
}

void Network::resetDiscovered() {
    discoveredPeers.clear();
}

void Network::enableDiscovery() {
    esp_now_register_recv_cb(Network::discoveryRequestCallback);
}

void Network::discover() {
    uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t discovery[1] = {'D'};
    Serial.printf("Sending discovery message\n");
    Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", broadcastMac[0], broadcastMac[1], broadcastMac[2], broadcastMac[3], broadcastMac[4], broadcastMac[5]);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastMac, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);

    esp_now_register_recv_cb(Network::discoveryResponseCallback);

    esp_err_t result = esp_now_send(broadcastMac, discovery, 1);
    if (result == ESP_OK) {
        Serial.printf("Discovery message sent");
    } else {
        Serial.printf("Failed to send discovery message: %d\n", result);
    }
    esp_now_del_peer(broadcastMac);
}

std::vector<uint8_t*> Network::getDiscovered() {
    return discoveredPeers;
}

int Network::getChannel() {
    uint8_t pChannel;
    wifi_second_chan_t sChannel;
    esp_wifi_get_channel(&pChannel, &sChannel);
    return pChannel;
}

uint8_t* Network::getMac() {
    uint8_t* mac = new uint8_t[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    return mac;
}

String Network::getMacString() {
    return WiFi.macAddress();
}

String Network::stringFromMac(uint8_t* mac) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

uint8_t* Network::macFromString(String mac) {
    uint8_t* macAddr = new uint8_t[6];
    sscanf(mac.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &macAddr[0], &macAddr[1], &macAddr[2], &macAddr[3], &macAddr[4], &macAddr[5]);
    return macAddr;
}

SemaphoreHandle_t Network::getRemoteTickMutex() {
    return remoteTickMutex;
}

void Network::addDiscoveredPeer(uint8_t* mac) {
    if (std::find(discoveredPeers.begin(), discoveredPeers.end(), mac) != discoveredPeers.end()) {
        return;
    }
    discoveredPeers.push_back(mac);
}

void Network::setPeer(uint8_t* mac) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = false; // TODO: Add encryption
    esp_now_add_peer(&peerInfo);
}

void Network::sendTick(RemoteTick* tick) {
    uint8_t* mac = game->getPeer();
    if (!mac) {
        Serial.println("Peer MAC not set");
        return;
    }

    esp_err_t result = esp_now_send(mac, (uint8_t*)tick, sizeof(RemoteTick));
    if (result != ESP_OK) {
        Serial.printf("Failed to send remote tick: %d\n", result);
    }
}

RemoteTick* Network::receiveTick() {
    if (!isNewTick) {
        return nullptr;
    }

    isNewTick = false;
    return remoteTick.get();
}

void Network::setRemoteTick(std::unique_ptr<RemoteTick> tick) {
    if (xSemaphoreTake(remoteTickMutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
        return;
    }
    remoteTick = std::move(tick);
    xSemaphoreGive(remoteTickMutex);
    isNewTick = true;
}

void Network::waitJoinResponse() {
    esp_now_register_recv_cb(Network::joinResponseCallback);
}

void Network::requestJoin(uint8_t* mac) {
    Game* game = Game::instance;
    if (!game) {
        Serial.println("Game instance not found");
        return;
    }

    Menu* menu = game->getMenu();
    uint8_t join[1] = {'J'};
    Serial.printf("Sending join message to ");
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);

    esp_now_register_recv_cb(Network::joinRequestCallback);

    esp_err_t result = esp_now_send(mac, join, 1);
    if (result == ESP_OK) {
        Serial.println("Join message sent");
    } else {
        Serial.printf("Failed to send join message: %d\n", result);
    }
}

void Network::acceptJoin() {
    uint8_t* mac = game->getPeer();
    if (!mac) {
        Serial.println("Peer MAC not set");
        return;
    }

    uint8_t accept[3] = {'A', 'J', 'A'};
    esp_err_t result = esp_now_send(mac, accept, 3);
    if (result == ESP_OK) {
        Serial.println("Join accepted");
    } else {
        Serial.printf("Failed to accept join: %d\n", result);
    }
}

void Network::declineJoin() {
    uint8_t* mac = game->getPeer();
    if (!mac) {
        Serial.println("Peer MAC not set");
        return;
    }

    Serial.printf("Declining join from %s\n", stringFromMac(mac).c_str());
    Serial.printf("Host is added? %d\n", esp_now_is_peer_exist(mac));

    uint8_t decline[3] = {'A', 'J', 'D'};
    esp_err_t result = esp_now_send(mac, decline, 3);
    esp_now_del_peer(mac);
    if (result == ESP_OK) {
        Serial.println("Join declined");
    } else {
        Serial.printf("Failed to decline join: %d\n", result);
    }
}

void Network::setMultiplayerHandlers() {
    Serial.printf("Remote peer MAC: %s\n", stringFromMac(game->getPeer()).c_str());
    Serial.printf("ESP-NOW is peer exist? %d\n", esp_now_is_peer_exist(game->getPeer()));
    esp_now_register_recv_cb(Network::remoteTickCallback);
    Serial.println("Multiplayer handlers set");
}

void Network::discoveryRequestCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    Game* game = Game::instance;
    if (!game) {
        Serial.println("Game instance not found");
        return;
    }

    Serial.printf("Received discovery from %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.printf("Data: %s\n", std::string((char*)data, data_len).c_str());

    Menu* menu = game->getMenu();
    Network* network = game->getNetwork();
    uint8_t ack[2] = {'A', 'P'}; // Acknowledge Packet
    if (data_len == 1) {
        if (data[0] == 'D') {
            Serial.printf("Received discovery from %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            ack[1] = 'D'; // Acknowledge Discovery

            esp_now_peer_info_t peerInfo = {};
            memcpy(peerInfo.peer_addr, mac_addr, 6);
            peerInfo.channel = network->getChannel();
            esp_now_add_peer(&peerInfo);
            esp_now_send(mac_addr, ack, 2);
            esp_now_del_peer(mac_addr);
            return;
        } else if (data[0] == 'J') {
            Serial.printf("Received join request from %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            ack[1] = 'J'; // Acknowledge Join - This is not accept, it's just an acknowledgement

            menu->handleJoinRequestReceived((uint8_t*)mac_addr);
            esp_now_send(mac_addr, ack, 2);
            return;
        }
    }
    Serial.println("Unknown message");
}

void Network::discoveryResponseCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    Serial.printf("Received response from %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Game* game = Game::instance;
    if (!game) {
        Serial.println("Game instance not found");
        return;
    }

    Network* network = game->getNetwork();
    if (data_len == 2) {
        if (data[0] == 'A' && data[1] == 'D') {
            uint8_t newMac[6];
            for (int i = 0; i < 6; i++) {
                newMac[i] = mac_addr[i];
            }
            network->addDiscoveredPeer(newMac);
            return;
        }
    }
}

void Network::joinRequestCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    Game* game = Game::instance;
    if (!game) {
        Serial.println("Game instance not found");
        return;
    }

    Menu* menu = game->getMenu();
    Network* network = game->getNetwork();
    if (data_len == 2) {
        if (data[0] == 'A' && data[1] == 'J') {
            Serial.printf("Acknowledge join request from %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            // TODO: Maybe set a screen between join request and join request acknowledged
            esp_now_register_recv_cb(Network::joinResponseCallback);
            return;
        }
    }
}

void Network::joinResponseCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    Game* game = Game::instance;
    if (!game) {
        Serial.println("Game instance not found");
        return;
    }

    Menu* menu = game->getMenu();
    Network* network = game->getNetwork();
    if (data_len == 3) {
        if (data[0] == 'A' && data[1] == 'J') {
            if (data[2] == 'A') {
                Serial.printf("Received join accept from %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
                game->setPeer((uint8_t*)mac_addr);
                menu->handleJoinRequestAccepted();
                return;
            } else if (data[2] == 'D') {
                Serial.printf("Received join decline from %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
                menu->handleJoinRequestDeclined();
                return;
            }
        }
    }
}

void Network::remoteTickCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    Game* game = Game::instance;
    if (!game) {
        Serial.println("Game instance not found");
        return;
    }

    Network* network = game->getNetwork();
    if (data_len != sizeof(RemoteTick)) {
        Serial.printf("Invalid tick size: %d\n", data_len);
    }

    std::unique_ptr<RemoteTick> tick(new RemoteTick());
    memcpy(tick.get(), data, sizeof(RemoteTick));
    network->setRemoteTick(std::move(tick));
}
