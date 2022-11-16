#pragma once

#include <vector>

#include "TCP/TCPClient.h"

class Communication
{
	static inline TCPClient tcpClient;
	static inline bool isConnected = false;

	static inline unsigned int integritySignature1 = 0xdeadbeef;
	static inline unsigned int integritySignature2 = 0xefbeadde;
	static inline unsigned int integritySignature3 = 0xbeefdead;

	static void onReceive(const std::vector<unsigned char>& data);
	static void onDisconnect();
public:
	static bool Connect();
	static void Disconnect();
	static bool GetIsConnected();
};
