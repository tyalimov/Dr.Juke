#pragma once

#include <string>
#include <vector>
#include "raii.h"

#pragma comment ( lib, "Dbghelp.lib" )

namespace drjuke::winlib
{
    struct WindowsStackFrame
    {
        /// <summary> Конструктор для базовой инициализации </summary>
        WindowsStackFrame()
            : m_name("")
            , m_module("")
            , m_address(0)
        {}

        /// <summary> Конструктор стекового кадра. </summary>
        /// <param name="name"> Имя функции </param>
        /// <param name="module"> Имя модуля (в нашем случае это исполняемые файлы и библиотеки) </param>
        /// <param name="address"> Адрес функции</param>
        WindowsStackFrame(const std::string& name,
                          const std::string& module,
                          std::uint64_t      address)
            : m_name(name)
            , m_module(module)
            , m_address(address)
        {}

        std::string   m_name;
        std::string   m_module;
        std::uint64_t m_address;
    };

    class StackTraceBuilder
    {
    private:

        enum : uint32_t
        {
            kFramesToCapture   = 64,
            kMaxModuleNameSize = 512
        };

        UniqueSymHandle m_process;
        WORD m_captured_frames_count;
        void *m_captured_frames[kFramesToCapture];
        std::vector<WindowsStackFrame> m_trace;

        void initializeProcess();
        void initializeCapturedFrames();
        void initializeTrace();

    public:

        StackTraceBuilder();

        [[nodiscard]] std::string format();
    };
}