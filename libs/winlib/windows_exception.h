/*
 
    Author – Timur Yalimov [https://github.com/tyalimov/]
    Last Modify – 02.08.2019
    Description:
    
    Файл содержит описание класса исключения.

*/

#pragma once

#include <exception>
#include <string>
#include <vector>

namespace drjuke::winlib
{
    class WindowsException final
        : public std::exception
    {
    private:
        mutable std::string  m_message;
        std::string          m_error_message;
        uint32_t             m_error_code;

    public:
        /// <summary> Сгенерировать исключение. Код ошибки притянется сам через GetLastError() </summary>
        /// <param name="message"> Словестное описание ошибки </param>
        explicit WindowsException(const char *message);

        /// <summary> Сгенерировать исключение со специфичным кодом ошибки </summary>
        /// <param name="message"> Словестное описание ошибки </param>
        /// <param name="error_code"> Код ошибки, полученный не из GetLastError() (например WSAGetLastError) </param>
        explicit WindowsException(const char *message, uint32_t error_code);

        /// <summary> Вывести описание ошибки </summary>
        [[nodiscard]] const char *what() const noexcept override final;

        /// <summary> Вывести описание ошибки + стек вызовов, который к ней привел </summary>
        [[nodiscard]] std::string dumpStackTrace() const;
        
        ~WindowsException() override = default;
    };
}
