#include "ChatSystem.h"
#include "NetworkSystem.h"
#include "ImGuiSystem.h"

ChatSystem& chatSystem = ChatSystem::Get();

void ChatSystem::DrawChatWindow()
{
    imGuiSystem.Begin("Chat");
    const float footerHeight = imGuiSystem.GetFrameHeightWithSpacing() + 10.0f;
    imGuiSystem.BeginChild("ChatScroll", ImVec2(0, -footerHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& line : m_chatLines)
    {
        imGuiSystem.TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "[%s]", line.sender.c_str());
        imGuiSystem.SameLine();
        imGuiSystem.TextWrapped("%s", line.text.c_str());
    }
    if (imGuiSystem.GetScrollY() >= imGuiSystem.GetScrollMaxY() - 10.0f) imGuiSystem.SetScrollHereY(1.0f);

    imGuiSystem.EndChild();
    static char inputBuffer[128] = "";
    bool reclaimFocus = false;

    imGuiSystem.PushItemWidth(-1);
    if (imGuiSystem.InputText("##ChatInput", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (inputBuffer[0] != '\0')
        {
            AddChatMessage("You", inputBuffer);
            networkSystem.SendChatMessage(inputBuffer);

            inputBuffer[0] = '\0';
            reclaimFocus = true;
        }
    }
    imGuiSystem.PopItemWidth();

    if (reclaimFocus || imGuiSystem.IsWindowAppearing()) imGuiSystem.SetKeyboardFocusHere(-1);
    imGuiSystem.End();
}



void ChatSystem::AddChatMessage(const char* sender, const char* text)
{
    m_chatLines.push_back(ChatLine
        { 
            .sender = sender,
            .text = text 
        });
    if (m_chatLines.size() > m_maxChatLines) m_chatLines.erase(m_chatLines.begin());
}