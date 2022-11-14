#pragma once

#include <vector>

#include "TCP/TCPClient.h"

class Communication
{
	static inline TCPClient tcpClient;
	static inline bool isConnected = false;

	static void onReceive(const std::vector<unsigned char>& data);
	static void onDisconnect();
public:
	static bool Connect();
	static void Disconnect();
	static bool GetIsConnected();
};
