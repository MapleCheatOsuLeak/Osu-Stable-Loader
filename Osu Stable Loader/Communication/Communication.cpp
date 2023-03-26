#include "Communication.h"

#include <random>

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

#include "../Utilities/Windows/WindowsUtilities.h"
#include "../Utilities/Windows/PdbUtilities.h"

#pragma optimize("", off)
void Communication::onReceive(const std::vector<unsigned char>& data)
{
	VM_SHARK_BLACK_START

	int codeIntegrityVar = 0x671863E2;
	CHECK_CODE_INTEGRITY(codeIntegrityVar, 0x40CD69D0)
	if (codeIntegrityVar != 0x40CD69D0)
	{
		integritySignature1 -= 0x1;
		integritySignature2 -= 0x1;
		integritySignature3 -= 0x1;
	}

	int debuggerVar = 0xD0A7E6;
	CHECK_DEBUGGER(debuggerVar, 0x3E839EE3)
	if (debuggerVar != 0x3E839EE3)
	{
		integritySignature1 -= 0x1;
		integritySignature2 -= 0x1;
		integritySignature3 -= 0x1;
	}

	const auto type = static_cast<PacketType>(data[0]);
	const std::vector payload(data.begin() + 1, data.end());

	VM_SHARK_BLACK_END

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

			const PdbInformation ntdllPdbInformation = PdbUtilities::GetPdbInformation(WindowsUtilities::GetNtdllPath(), WindowsUtilities::Is64BitOS());

			int imageBaseAddress = ImageMapper::AllocateMemoryForImage(imageStreamStageOneResponse.GetImageSize());
			std::vector<ImageResolvedImport> resolvedImports = ImageMapper::ResolveImports(imageStreamStageOneResponse.GetImports());

			ImageStreamStageTwoRequest imageStreamStageTwoRequest = ImageStreamStageTwoRequest(UserDataManager::GetSessionToken(), UserDataManager::GetCheatID(), imageBaseAddress, ntdllPdbInformation, resolvedImports);
			tcpClient.Send(imageStreamStageTwoRequest.Serialize());

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END
		}
			break;
		case PacketType::ImageStreamStageTwo:
		{
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			bool integrityViolated = integritySignature1 != 0xdeadbeef || integritySignature2 != 0xefbeadde || integritySignature3 != 0xbeefdead;

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> behaviorRNG(1, 3);
			std::uniform_int_distribution<> xorByteRNG(0, 255);
			std::uniform_int_distribution<> xorIntRNG(1, 1337);

			// 1 - fail, 2 - corrupt sections, 3 - corrupt callbacks
			const int behavior = integrityViolated ? behaviorRNG(gen) : 0;

			ImageStreamStageTwoResponse imageStreamStageTwoResponse = ImageStreamStageTwoResponse::Deserialize(payload);

			if (behavior == 1 || imageStreamStageTwoResponse.GetResult() != ImageStreamStageTwoResult::Success)
			{
				Disconnect();

				return;
			}
			
			int ldrpHandleTlsDataOffset = imageStreamStageTwoResponse.GetLdrpHandleTlsDataOffset();
			int entryPointOffset = imageStreamStageTwoResponse.GetEntryPointOffset();
			std::vector<unsigned char> headers = imageStreamStageTwoResponse.GetHeaders();

			std::vector<ImageSection> sections = imageStreamStageTwoResponse.GetSections();
			if (behavior == 2)
				for (auto& section : sections)
					for (unsigned int i = 0; i < section.Data.size(); i++)
						if (i % 5 == 0) // corrupt each 5th byte
							section.Data[i] = section.Data[i] ^ (section.Data[i - 1] ^ section.Data[i] ^ xorByteRNG(gen));

			std::vector<int> tlsCallbacks = imageStreamStageTwoResponse.GetTLSCallbacks();
			if (behavior == 3)
			{
				entryPointOffset = entryPointOffset ^ xorIntRNG(gen);

				for (int& callback : tlsCallbacks)
					callback = callback ^ xorIntRNG(gen);
			}

			ImageMapper::MapImage(ldrpHandleTlsDataOffset, entryPointOffset, headers, sections, tlsCallbacks);

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
	if (!tcpClient.Connect(xorstr_("198.251.89.179"), xorstr_("9999")))
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
#pragma optimize("", on)
