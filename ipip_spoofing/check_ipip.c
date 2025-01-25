#include <arpa/inet.h>
#include <memory.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

unsigned short net_sum(void *buffer, size_t size)
{
	register int nleft = size;
	register u_short *w = (u_short *)buffer;
	register int sum = 0;
	u_short answer = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16); // add carry
	answer = ~sum;
	return answer;
}

int main(void)
{
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sock == -1) {
		perror("Could not open socket");
		_exit(1);
	}

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);

	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("Could not bind socket");
		_exit(1);
	}

	struct iphdr ip_header = {0};
	ip_header.version = 4;
	ip_header.ihl = 5;
	ip_header.tos = 0;
	ip_header.tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
	ip_header.id = 0;
	ip_header.frag_off = 0;
	ip_header.ttl = 2;
	ip_header.protocol = IPPROTO_TCP;
	ip_header.saddr = inet_addr("127.0.0.1");
	ip_header.daddr = inet_addr("127.0.0.1");

	ip_header.check = htons(net_sum(&ip_header, ip_header.ihl * 4));

	struct tcphdr tcp_header = {0};
	tcp_header.source = htons(1234);
	tcp_header.dest = htons(9999);
	tcp_header.seq = htonl(1);
	tcp_header.ack_seq = htonl(0);
	tcp_header.res1 =0;
	tcp_header.doff = 5;
	tcp_header.fin = 0;
	tcp_header.syn = 1;
	tcp_header.rst = 0;
	tcp_header.psh = 0;
	tcp_header.ack = 0;
	tcp_header.urg = 0;
	tcp_header.res2 = 0;
	tcp_header.window = htons(20);
	tcp_header.urg_ptr = 0;

	struct {
		unsigned int source;
		unsigned int dest;
		unsigned char zeros;
		unsigned char protocol;
		unsigned short tot_len;
	} pseudoip_header = {
		.source = ip_header.saddr,
		.dest = ip_header.daddr,
		.protocol = ip_header.protocol,
		.tot_len = htons(sizeof(tcp_header))
	};

	size_t checksum_buffer_len = sizeof(pseudoip_header) + sizeof(struct tcphdr);
	unsigned char checksum_buffer[checksum_buffer_len];
	memcpy(checksum_buffer, &pseudoip_header, sizeof(pseudoip_header));
	memcpy(checksum_buffer + sizeof(pseudoip_header), &tcp_header, sizeof(tcp_header));

	tcp_header.check = net_sum(&checksum_buffer, checksum_buffer_len);

	unsigned char buffer[sizeof(struct iphdr) + sizeof(struct tcphdr)];
	memcpy(buffer, &ip_header, sizeof(ip_header));
	memcpy(buffer + sizeof(ip_header), &tcp_header, sizeof(tcp_header));

	sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, sizeof(addr));

	close(sock);
}
