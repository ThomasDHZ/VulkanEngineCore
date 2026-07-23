#pragma once
#include "Platform.h"

struct MemoryLeakPtr {
    void*  PtrAddress = nullptr;
    int    Line = 0;
    bool   IsArray = false;
    size_t ElementCount = 0;
    String File;
    String TypeName;
    String Function;
    String Notes;
};

class MemorySystem
{
    public:
        static MemorySystem& Get();

    private:
        MemorySystem() = default;
        ~MemorySystem() = default;
        MemorySystem(const MemorySystem&) = delete;
        MemorySystem& operator=(const MemorySystem&) = delete;
        MemorySystem(MemorySystem&&) = delete;
        MemorySystem& operator=(MemorySystem&&) = delete;

        std::mutex                               m_mutex;
        std::unordered_map<void*, MemoryLeakPtr> m_ptrAddressMap;

        void TrackAllocation(void* ptr, size_t count, bool isArray, const char* file, int line, const char* typeName, const char* func, const char* notes)
        {
            if (!ptr) return;

            MemoryLeakPtr memoryLeakPtr{
                .PtrAddress = ptr,
                .Line = line,
                .IsArray = isArray,
                .ElementCount = count,
                .File = file ? file : "unknown",
                .TypeName = typeName ? typeName : "unknown",
                .Function = func ? func : "unknown",
                .Notes = notes ? notes : ""
            };

            m_ptrAddressMap[ptr] = std::move(memoryLeakPtr);
        }

    public:

        template<typename T>
        T* AddPtrBuffer(const char* file, int line, const char* func, const char* notes = "")
        {
            return AddPtrBuffer<T>(1, file, line, func, notes);
        }

        template <typename T>
        T* AddPtrBuffer(size_t count, const char* file, int line, const char* func, const char* notes = "")
        {
            if (count == 0) return nullptr;
            std::lock_guard<std::mutex> lock(m_mutex);

            size_t size = sizeof(T) * count;
            void* rawPtr = ::operator new(size, std::nothrow);
            if (!rawPtr) return nullptr;

            T* ptr = static_cast<T*>(rawPtr);
            for (size_t x = 0; x < count; ++x)
            {
                new (&ptr[x]) T();
            }

            auto destructor = [](void* p, size_t cnt)
                {
                    T* typed = static_cast<T*>(p);
                    for (size_t x = 0; x < cnt; ++x)
                    {
                        typed[x].~T();
                    }
                };

            TrackAllocation(rawPtr, count, count > 1, file, line, typeid(T).name(), func, notes);
            return ptr;
        }

        template <typename T>
        T* AddPtrBuffer(T* src, size_t count, const char* file, int line, const char* func, const char* notes = "")
        {
            T* ptr = AddPtrBuffer<T>(count, file, line, func, notes);
            if (ptr && src)
            {
                for (size_t x = 0; x < count; ++x)
                {
                    ptr[x] = src[x];
                }
            }
            return ptr;
        }

        template <typename T>
        T* AddPtrBuffer(const T* src, size_t count, const char* file, int line, const char* func, const char* notes = "")
        {
            if (count == 0) return nullptr;
            T* ptr = AddPtrBuffer<T>(count, file, line, func, notes);
            if (ptr && src)  memcpy(ptr, src, count * sizeof(T));
            return ptr;
        }

        DLL_EXPORT void DeletePtr(void* ptr);
        DLL_EXPORT void ReportLeaks();
};
extern DLL_EXPORT MemorySystem& memorySystem;
inline MemorySystem& MemorySystem::Get()
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    static MemorySystem instance;
    return instance;
}

