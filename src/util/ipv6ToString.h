#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ws2def.h>
#else
#include <netinet/in.h>
#endif

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <string>

#ifdef _WIN32
std::string ipv6ToString(sockaddr_in6* address) {
	return fmt::format(
		"{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{:x}{:x}:{}",
		address->sin6_addr.u.Byte[0],
		address->sin6_addr.u.Byte[1],
		address->sin6_addr.u.Byte[2],
		address->sin6_addr.u.Byte[3],
		address->sin6_addr.u.Byte[4],
		address->sin6_addr.u.Byte[5],
		address->sin6_addr.u.Byte[6],
		address->sin6_addr.u.Byte[7],
		address->sin6_addr.u.Byte[8],
		address->sin6_addr.u.Byte[9],
		address->sin6_addr.u.Byte[10],
		address->sin6_addr.u.Byte[11],
		address->sin6_addr.u.Byte[12],
		address->sin6_addr.u.Byte[13],
		address->sin6_addr.u.Byte[14],
		address->sin6_addr.u.Byte[15],
		address->sin6_port
	);
}

std::string ipv6ToString(sockaddr_in6 address) {
	return ipv6ToString(&address);
}
#else
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
#endif
