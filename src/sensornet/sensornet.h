#pragma once

#include <stdint.h>
#include <vector.h>
#include <RF24.h>
#include <GSM.h>

struct SensorPacket {
	int8_t id; // 1 byte
	char name[8]; // 8 bytes
	uint8_t type; // 1 byte
	uint8_t data[22]; // total 32 bytes (max nrf24l01 packet size)
};

class Sensornet {
private:
    RF24 m_radio;

    Vector<SensorPacket> m_cmd_queue;

    void ProcessPacket(SensorPacket& pkt);
    void UpdateNode(const SensorPacket& pkt);

public:
    struct Node {
        SensorPacket data;
        uint32_t last_seen;
    };

    Vector<Node> m_nodes;
    
    Sensornet(const RF24& radio);

    bool begin();

    void Poll();
    void CMDSetName(GSM* gsm, const char* sender, const char* c_args);
};