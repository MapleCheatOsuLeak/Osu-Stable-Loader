#pragma once

enum class PacketType : unsigned char
{
    Handshake = 0xA0,
    ImageStreamStageOne = 0xD0,
    ImageStreamStageTwo = 0xD1
};