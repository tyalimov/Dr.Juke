/*
 
    Author – Timur Yalimov [https://github.com/tyalimov/]
    Last Modify – 02.08.2019
    Description:
    
    Файл содержит описание класса исключения.

*/

#pragma once

#include <exception>
#include <string>
#include <curl/curl.h>

namespace drjuke::netlib
{
    class CurlException final
        : public std::exception
    {
    private:
        mutable std::string  m_message;
        CURLcode             m_error_code;

    public:
        /// <summary> Сгенерировать исключение со специфичным кодом ошибки </summary>
        /// <param name="message"> Словестное описание ошибки </param>
        /// <param name="error_code"> Код ошибки </param>
        explicit CurlException(CURLcode error_code);

        /// <summary> Вывести описание ошибки </summary>
        [[nodiscard]] const char *what() const noexcept override final;
        
        ~CurlException() override = default;
    };
}
