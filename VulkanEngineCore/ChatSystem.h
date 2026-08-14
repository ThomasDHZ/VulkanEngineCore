#pragma once

#include "Platform.h"
#include "ImGuiSystem.h"

struct ChatLine
{
	String sender;
	String text;
};

class ChatSystem
{
	friend class NetworkSystem;

public:
	static ChatSystem& Get();

private:
	ChatSystem() = default;
	~ChatSystem() = default;
	ChatSystem(const ChatSystem&) = delete;
	ChatSystem& operator=(const ChatSystem&) = delete;
	ChatSystem(ChatSystem&&) = delete;
	ChatSystem& operator=(ChatSystem&&) = delete;

	const uint		 m_maxChatLines = 255;
    Vector<ChatLine> m_chatLines;

	void AddChatMessage(const char* sender, const char* text);

public:
	void DrawChatWindow();
};
extern ChatSystem& chatSystem;
inline ChatSystem& ChatSystem::Get()
{
	static ChatSystem instance;
	return instance;
}

