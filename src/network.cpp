#include <WiFi.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "game.h"
#include "network.h"

Network::Network(Game *game):
        channel(1) {
    this->game = game;
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

void Network::addDiscoveredPeer(uint8_t* mac) {
    if (std::find(discoveredPeers.begin(), discoveredPeers.end(), mac) != discoveredPeers.end()) {
        return;
    }
    discoveredPeers.push_back(mac);
}

void Network::discoveryRequestCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    Game* game = Game::instance;
    if (!game) {
        Serial.println("Game instance not found");
        return;
    }

    Network* network = game->getNetwork();
    if (data_len == 1) {
        if (data[0] == 'D') {
            Serial.printf("Received discovery from %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            
            uint8_t ack[2] = {'A', 'D'}; // Acknowledge Discovery
            
            esp_now_peer_info_t peerInfo = {};
            memcpy(peerInfo.peer_addr, mac_addr, 6);
            peerInfo.channel = network->getChannel();
            esp_now_add_peer(&peerInfo);
            esp_now_send(mac_addr, ack, 2);
            esp_now_del_peer(mac_addr);
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