#include "sensornet.h"
#include "sensors.h"

extern "C" {
#include <systick.h>
}

Sensornet::Sensornet(const RF24& radio) :
    m_radio(radio)
{
    /*const Node node1 = {
        {
            1,
            "test1",
            sensors::TYPE_SOIL,
            { 192 }
        },
        millis()
    };

    m_nodes.push_back(node1);

    const Node node2 = {
        {
            2,
            "test2",
            sensors::TYPE_TEMP,
            { 24 }
        },
        millis()
    };

    m_nodes.push_back(node2);*/
}

bool Sensornet::begin() {
    if(!m_radio.begin())
        return false;

    m_radio.setChannel(120);
    m_radio.setAutoAck(true);
    //m_radio.enableAckPayload();
    m_radio.setRetries(5, 15);
    m_radio.setPayloadSize(32);

    m_radio.setPALevel(RF24_PA_MAX);
    m_radio.setDataRate(RF24_1MBPS);

    // Open pipe 1 for reading on address 255
    m_radio.openReadingPipe(1, (uint64_t)255 | (1 << 10));

    m_radio.startListening();

    return true;
}

void Sensornet::Poll() {
    if(m_radio.available()) {
        SensorPacket pkt;
        m_radio.read(&pkt, 32);

        ProcessPacket(pkt);

        m_radio.flush_rx();
    }
}

void Sensornet::ProcessPacket(SensorPacket& pkt) {
    if(pkt.id == 0) {
        // Initial node id
        pkt.id = 1;

        for(auto& node : m_nodes) {
            // If a node with the same name exists, replace it
            if(strncmp(node.data.name, pkt.name, sizeof(SensorPacket::name)) == 0) {
                pkt.id = node.data.id;
                break;
            }

            // Find a new id for the new node
            if(node.data.id >= pkt.id) pkt.id = node.data.id + 1;
        }

        UpdateNode(pkt);

        // Send the packet back to set node's ID
        m_radio.stopListening();
        pkt.type = Sensors::TYPE_COMMAND;
        pkt.data[0] = Sensors::COMMAND_SET_ID;
        m_radio.openWritingPipe((uint64_t)0 | (1 << 10));
        m_radio.write(&pkt, 32);
        m_radio.startListening();
    } else {
        UpdateNode(pkt);

        for(size_t i = 0; i < m_cmd_queue.size(); i++) {
            auto& cmd = m_cmd_queue[i];
            if(cmd.id == pkt.id) {
                m_radio.stopListening();
                m_radio.openWritingPipe((uint64_t)cmd.id | (1 << 10));
                m_radio.write(&cmd, 32);
                m_radio.startListening();

                m_cmd_queue.erase(m_cmd_queue.begin() + i);
                i--;
            }
        }
    }
}

void Sensornet::UpdateNode(const SensorPacket& pkt) {
    for(auto& node : m_nodes) {
        if(node.data.id == pkt.id) {
            memcpy(&node.data, &pkt, sizeof(SensorPacket));
            node.last_seen = millis();
            return;
        }
    }

    m_nodes.push_back({ pkt, millis() });
}

void Sensornet::CMDSetName(GSM* gsm, const char* sender, const char* c_args) {
    char* id_str = NULL;
    char* name = NULL;
    char* args = NULL;

    if(c_args) {
        args = (char*)malloc(strlen(c_args));
        if(!args) {
            gsm->SendSMS(sender, "malloc failed");
            return;
        }
        strcpy(args, c_args);

        id_str = strtok(args, " ");
        name = strtok(NULL, " ");
    }

    if(id_str && name && atoi(id_str)) {
        SensorPacket cmd;
        cmd.id = atoi(id_str);
        cmd.type = Sensors::TYPE_COMMAND;
        cmd.data[0] = Sensors::COMMAND_SET_NAME;
        strncpy(cmd.name, name, sizeof(cmd.name));

        m_cmd_queue.push_back(cmd);

        gsm->SendSMS(sender, "Command queued");
    } else {
        gsm->SendSMS(sender, "Usage: setname <id> <new name>");
    }

    if(args) free(args);
}