#pragma once

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <netinet/in.h>
#include <string>

std::string ipv6ToString(sockaddr_in6* address) {
	return fmt::format(
		"{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{}",
		address->sin6_addr.s6_addr[0],
		address->sin6_addr.s6_addr[1],
		address->sin6_addr.s6_addr[2],
		address->sin6_addr.s6_addr[3],
		address->sin6_addr.s6_addr[4],
		address->sin6_addr.s6_addr[5],
		address->sin6_addr.s6_addr[6],
		address->sin6_addr.s6_addr[7],
		address->sin6_addr.s6_addr[8],
		address->sin6_addr.s6_addr[9],
		address->sin6_addr.s6_addr[10],
		address->sin6_addr.s6_addr[11],
		address->sin6_addr.s6_addr[12],
		address->sin6_addr.s6_addr[13],
		address->sin6_addr.s6_addr[14],
		address->sin6_addr.s6_addr[15],
		address->sin6_port
	);
}

std::string ipv6ToString(sockaddr_in6 address) {
	return ipv6ToString(&address);
}
