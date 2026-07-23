#include "MemorySystem.h"
MemorySystem& memorySystem = MemorySystem::Get();

void MemorySystem::DeletePtr(void* ptr)
{
    if (!ptr) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_ptrAddressMap.find(ptr);
    if (it == m_ptrAddressMap.end())
    {
        std::cerr << "[MemorySystem] Double delete or unknown pointer: " << ptr << std::endl;
        return;
    }

    MemoryLeakPtr& info = it->second;
    ::operator delete(info.PtrAddress);
    m_ptrAddressMap.erase(it);
}

void MemorySystem::ReportLeaks()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_ptrAddressMap.empty()) {
        std::cout << "No memory leaks detected.\n";
        return;
    }

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD originalAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    if (hConsole != INVALID_HANDLE_VALUE) {
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        originalAttributes = consoleInfo.wAttributes;
    }
#endif

    size_t maxAddr = 6;
    size_t maxSize = 4;
    size_t maxArray = 5;
    size_t maxFile = 4;
    size_t maxLine = 4;
    size_t maxFunc = 4;
    size_t maxNotes = 5;

    for (const auto& [addr, info] : m_ptrAddressMap) {
        std::ostringstream oss;
        oss << "0x" << std::hex << std::uppercase << addr;
        maxAddr = std::max(maxAddr, oss.str().length());

        std::string sizeStr = std::to_string(info.ElementCount);
        maxSize = std::max(maxSize, sizeStr.length());

        std::string arrayStr = info.IsArray ? "Yes" : "No";
        maxArray = std::max(maxArray, arrayStr.length());

        std::string fileBase = info.File.substr(info.File.find_last_of("\\/") + 1);
        maxFile = std::max(maxFile, fileBase.length());

        maxLine = std::max(maxLine, static_cast<size_t>(info.Line));
        maxFunc = std::max(maxFunc, info.Function.length());
        maxNotes = std::max(maxNotes, info.Notes.length());
    }

    std::cout << "\nMEMORY LEAKS DETECTED: " << m_ptrAddressMap.size() << "\n";
    std::cout << String(119, '-') << "\n";

    for (const auto& [addr, info] : m_ptrAddressMap) 
    {
        std::ostringstream addrStream;
        addrStream << "0x" << std::hex << std::uppercase << addr;
        String addrStr = addrStream.str();

        String sizeStr = std::to_string(info.ElementCount);
        String arrayStr = info.IsArray ? "Yes" : "No";
        String fileBase = info.File.substr(info.File.find_last_of("\\/") + 1);
        String lineStr = std::to_string(info.Line);
        String funcStr = info.Function;
        String noteStr = info.Notes.empty() ? "None" : info.Notes;

#ifdef _WIN32
        if (hConsole != INVALID_HANDLE_VALUE)
        {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            std::cout << std::left << std::setw(7) << "Error: ";
            SetConsoleTextAttribute(hConsole, originalAttributes);
            std::cout << "Memory Leak at: " << std::setw(maxAddr) << addrStr;

            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << " Size: ";
            SetConsoleTextAttribute(hConsole, originalAttributes);
            std::cout << std::setw(maxSize) << sizeStr;

            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << " IsArray: ";
            SetConsoleTextAttribute(hConsole, originalAttributes);
            std::cout << std::setw(maxArray) << arrayStr;

            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << " File: ";
            SetConsoleTextAttribute(hConsole, originalAttributes);
            std::cout << std::setw(maxFile) << fileBase;

            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << " Line: ";
            SetConsoleTextAttribute(hConsole, originalAttributes);
            std::cout << std::setw(maxLine) << lineStr;

            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << " Function: ";
            SetConsoleTextAttribute(hConsole, originalAttributes);
            std::cout << std::setw(maxFunc) << funcStr;

            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << " Notes: ";
            SetConsoleTextAttribute(hConsole, originalAttributes);
            std::cout << noteStr << "\n";
        }
        else
#endif
        {
            std::cout << "Error: Memory Leak at: " << addrStr
                << " Size: " << sizeStr
                << " IsArray: " << arrayStr
                << " File: " << fileBase
                << " Line: " << lineStr
                << " Function: " << funcStr
                << " Notes: " << noteStr << "\n";
        }
    }

    std::cout << std::string(119, '-') << "\n";
}