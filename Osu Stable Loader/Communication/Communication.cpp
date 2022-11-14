#include "Communication.h"

#include "ThemidaSDK.h"

#include "../Utilities/Security/xorstr.hpp"
#include "Packets/PacketType.h"
#include "Packets/Requests/HandshakeRequest.h"
#include "Packets/Responses/HandshakeResponse.h"
#include "Crypto/CryptoProvider.h"
#include "../Mapping/ImageMapper.h"
#include "Packets/Requests/ImageStreamStageOneRequest.h"
#include "../UserData/UserDataManager.h"
#include "Packets/Responses/ImageStreamStageOneResponse.h"
#include "Packets/Requests/ImageStreamStageTwoRequest.h"
#include "Packets/Responses/ImageStreamStageTwoResponse.h"

void Communication::onReceive(const std::vector<unsigned char>& data)
{
	const auto type = static_cast<PacketType>(data[0]);
	const std::vector payload(data.begin() + 1, data.end());

	switch (type)
	{
		case PacketType::Handshake:
		{
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			HandshakeResponse handshakeResponse = HandshakeResponse::Deserialize(payload);

			CryptoProvider::GetInstance()->InitializeAES(handshakeResponse.GetKey(), handshakeResponse.GetIV());

			ImageStreamStageOneRequest imageStreamStageOneRequest = ImageStreamStageOneRequest(UserDataManager::GetSessionToken(), UserDataManager::GetCheatID(), UserDataManager::GetReleaseStream());
			tcpClient.Send(imageStreamStageOneRequest.Serialize());

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END
		}
			break;
		case PacketType::ImageStreamStageOne:
		{
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			ImageStreamStageOneResponse imageStreamStageOneResponse = ImageStreamStageOneResponse::Deserialize(payload);

			if (imageStreamStageOneResponse.GetResult() != ImageStreamStageOneResult::Success)
			{
				Disconnect();

				return;
			}

			int imageBaseAddress = ImageMapper::AllocateMemoryForImage(imageStreamStageOneResponse.GetImageSize());
			std::vector<ImageResolvedImport> resolvedImports = ImageMapper::ResolveImports(imageStreamStageOneResponse.GetImports());

			ImageStreamStageTwoRequest imageStreamStageTwoRequest = ImageStreamStageTwoRequest(UserDataManager::GetSessionToken(), UserDataManager::GetCheatID(), imageBaseAddress, resolvedImports);
			tcpClient.Send(imageStreamStageTwoRequest.Serialize());

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END
		}
			break;
		case PacketType::ImageStreamStageTwo:
		{
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			ImageStreamStageTwoResponse imageStreamStageTwoResponse = ImageStreamStageTwoResponse::Deserialize(payload);

			if (imageStreamStageTwoResponse.GetResult() != ImageStreamStageTwoResult::Success)
			{
				Disconnect();

				return;
			}

			ImageMapper::MapImage(imageStreamStageTwoResponse.GetSections(), imageStreamStageTwoResponse.GetCallbacks());

			Disconnect();

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END
		}
			break;
	}
}

void Communication::onDisconnect()
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	isConnected = false;

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END
}

bool Communication::Connect()
{
	VM_FISH_RED_START
	STR_ENCRYPT_START

	tcpClient = TCPClient(&onReceive, &onDisconnect);
	if (!tcpClient.Connect(xorstr_("127.0.0.1"), xorstr_("9999")))
		return false;

	HandshakeRequest handshakeRequest = HandshakeRequest();
	tcpClient.Send(handshakeRequest.Serialize());

	isConnected = true;

	STR_ENCRYPT_END
	VM_FISH_RED_END

	return true;
}

void Communication::Disconnect()
{
	VM_SHARK_BLACK_START

	tcpClient.Disconnect();

	isConnected = false;

	VM_SHARK_BLACK_END
}

bool Communication::GetIsConnected()
{
	return isConnected;
}
