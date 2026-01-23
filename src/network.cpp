#include "network.h"
#include "config_autogen.h"
#include "receiver.h"
#include <QNEthernet.h>

using namespace qindesign::network;

static EthernetUDP udp_sockets[RUN_COUNT > 0 ? RUN_COUNT : 1];
static EthernetUDP status_socket;

static IPAddress static_ip(STATIC_IP_0, STATIC_IP_1, STATIC_IP_2, STATIC_IP_3);
static IPAddress netmask(STATIC_NETMASK_0, STATIC_NETMASK_1, STATIC_NETMASK_2, STATIC_NETMASK_3);
static IPAddress gateway(STATIC_GATEWAY_0, STATIC_GATEWAY_1, STATIC_GATEWAY_2, STATIC_GATEWAY_3);
static IPAddress sender_ip(SENDER_IP_0, SENDER_IP_1, SENDER_IP_2, SENDER_IP_3);

static char ip_string[16];
static uint8_t packet_buffer[2048];

void network_init() {
    // Configure static IP
    Ethernet.begin(static_ip, netmask, gateway);

    // Format IP string for heartbeat
    snprintf(ip_string, sizeof(ip_string), "%d.%d.%d.%d",
             STATIC_IP_0, STATIC_IP_1, STATIC_IP_2, STATIC_IP_3);

    // Bind UDP socket for each run
    for (int i = 0; i < RUN_COUNT; i++) {
        udp_sockets[i].begin(PORT_BASE + i);
    }

    // Status socket for sending heartbeats (no need to bind to specific port)
    status_socket.begin(0);
}

void network_poll() {
    // Check each run's UDP socket for incoming packets
    for (int run_index = 0; run_index < RUN_COUNT; run_index++) {
        int packet_size = udp_sockets[run_index].parsePacket();

        while (packet_size > 0) {
            // Read packet data
            int len = udp_sockets[run_index].read(packet_buffer, sizeof(packet_buffer));

            if (len > 0) {
                // Dispatch to receiver
                receiver_handle_packet(run_index, packet_buffer, len);
            }

            // Check for more packets on this socket
            packet_size = udp_sockets[run_index].parsePacket();
        }
    }
}

void network_send_status(const char* json, size_t len) {
    status_socket.beginPacket(sender_ip, STATUS_PORT);
    status_socket.write((const uint8_t*)json, len);
    status_socket.endPacket();
}

bool network_link_up() {
    return Ethernet.linkState();
}

const char* network_get_ip_string() {
    return ip_string;
}
