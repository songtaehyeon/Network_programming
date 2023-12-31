#include <stdio.h>
#include <stdint.h>//구조체 선언을 위해 uint8같은 단위를 사용하기 위해 추가한 해더
#include <pcap.h>
#include <arpa/inet.h>// IP 주소를 조작하고 네트워크 관련 작업을 수행하는 데 사용되는 함수 및 데이터 유형을 제공하는 해더

struct ethernet {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type; //2 바이트
} __attribute__((__packed__)); // 패딩 없이 구조체를 채우기 위해 사용

struct ipheader {
    uint8_t headerLength : 4; //1 바이트를 주는데 그중에서 4bit 만 할당
    uint8_t version : 4;
    uint8_t service_type;
    uint16_t total_Length;
    uint16_t identification;
    uint16_t flags_Fragment; //2바이트
    uint8_t TTL;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
} __attribute__((__packed__));

struct tcpheader {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t sequenceNumber;
    uint32_t acknowledgmentNumber;
    uint8_t dataOffset;
    uint8_t reserved_flags;
    uint16_t windowSize;
    uint16_t checksum;
    uint16_t urgentPointer;
} __attribute__((__packed__));

void packet_handler(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    struct ethernet *eth = (struct ethernet *)packet; //패킷 데이터를 내가 선언한 ethernet 구조체 같은 형태로 인식 하겠다는 의미

    if (ntohs(eth->type) != 0x0800) {  // 0x0800 == IP
        return;
    }

    struct ipheader *ip = (struct ipheader *)(packet + sizeof(struct ethernet)); // packet이 가리키는 위치에서 ethernet의 크기만큼 이동 해서 가리키겠다.
    if (ip->protocol != IPPROTO_TCP) {
        return;
    }

    struct tcpheader *tcp = (struct tcpheader *)(packet + sizeof(struct ethernet) + (ip->headerLength * 4)); // 위의 주석 + ip 헤더의 크기를 바이트 단위로 바꿔서 가리키겠다

    printf("Src Mac : %02X:%02X:%02X:%02X:%02X:%02X\n", eth->src[0], eth->src[1], eth->src[2], eth->src[3], eth->src[4], eth->src[5]);
    printf("Dst Mac : %02X:%02X:%02X:%02X:%02X:%02X\n", eth->dst[0], eth->dst[1], eth->dst[2], eth->dst[3], eth->dst[4], eth->dst[5]);
    printf("Src IP : %s\n", inet_ntoa(*(struct in_addr*)&(ip->src_ip))); //받은 데이터를  dotted-decimal(점있는 10진수) 로 바꾸겠다
    printf("Dst IP : %s\n", inet_ntoa(*(struct in_addr*)&(ip->dst_ip)));
    printf("Src Port : %u\n", ntohs(tcp->src_port));
    printf("Dst Port : %u\n", ntohs(tcp->dst_port));

    int totalBytes = pkthdr->len;
    printf("Total Bytes : %d\n", totalBytes);

    int header_length = sizeof(struct ethernet) + (ip->headerLength * 4) + (tcp->dataOffset >> 4) * 4;
    if (totalBytes >= header_length) {
        printf("TCP Payload : ");
        for (int i = header_length; i < header_length + 16 && i < totalBytes; i++) { //16바이트를 출력해주는데 패킷범위를 벗어나지 않게 한다.
            printf("%02X ", packet[i]);
        }
        printf("\n\n");
    }
}

int main(int argc, char *argv[]) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    handle = pcap_open_live(argv[1], BUFSIZ, 1, 1000, errbuf);//내가 사용하는 네트워크로 설정하고 캡쳐 하겠다
    if (handle == NULL) {
        fprintf(stderr, "NO: %s\n", errbuf);
        return 1;
    }

    pcap_loop(handle, 0, packet_handler, NULL);
    pcap_close(handle);
    return 0;
}
