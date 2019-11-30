#pragma once

#include <windows.h>
#include <wintrust.h>
#include <Softpub.h>

#include <filesystem>

#include "base_analyzer.h"
#include "cert_viewer.h"

/*
    Анализатор файлов, проверяющий ЭЦП исполняемых файлов.
    Формат отчета от анализатора:
    {
        "status"          : (non-signed|signed|non-trusted|disallowded|error) ----- Результат проверки
        "signer"          : <строка> -- Подписывающий, при non-signed или error - "undefined"
        "application"     : <строка> -- Название приложения, при non-signed или error - "undefined"
        "url"             : <строка>
        "datetime"        : Дата подписания, структура ниже.
        {
            "year"        : <число> - год подписания          ( от 1601 до 30827                )
            "month"       : <число> - месяц подписания        ( от 1<январь> до 12<декабрь>     )
            "day"         : <число> - день подписания         ( от 1 до 31                      )
            "day of week" : <число> - день недели             ( от 0<воскресенье> до 6<суббота> )
            "hour"        : <число> - час подписания          ( от 0 до 23                      )
            "minute"      : <число> - минута подписания       ( от 0 до 59                      )
            "second"      : <число> - секунда подписания      ( от 0 до 59                      )
            "millisecond" : <число> - миллисекунда подписания ( от 0 до 999                     )
        }
    }
*/

namespace drjuke::scanlib
{
    class SignatureReport : public BaseReport
    {
        CertificateDetails m_details;
        std::string        m_status;

    private:
        void initializeJson();

    public:
        SignatureReport(const std::string &status, const CertificateDetails &details)
            : m_details(details)
            , m_status(status)
        {
            // TODO: Первично инициализировать json
            // TODO: Заполнить поля из details
        }

         SignatureReport(const std::string &status)
            : m_status(status)
        {
            // TODO: Первично инициализировать json
            // TODO: Заполнить поля из details как "незаполненные"
        }

        Json makeJson() override;
    };

    class DigitalSignatureAnalyzer : public BaseAnalyzer
    {
    private:
        WINTRUST_FILE_INFO m_file_info;
        WINTRUST_DATA      m_win_trust_data;
        GUID               m_wvt_policy_guid;

    public:

        DigitalSignatureAnalyzer()
            : m_file_info()
            , m_win_trust_data()
            , m_wvt_policy_guid(WINTRUST_ACTION_GENERIC_VERIFY_V2)
        {}

    private:
        void constructWinTrustFileInfo(const wchar_t *filename);
        void constructWinTrustData();
        void destroyWinTrustData();

        void processSigned(const wchar_t      *filename);
        void processNotSigned(const wchar_t   *filename);
        void processDisallowded(const wchar_t *filename);
        void processNonTrusted(const wchar_t  *filename);
        void processError(LONG status);

        void getSertificateDetails(const wchar_t *filename);

    public:

        BaseReportPtr getReport(const Path& path) override;
        void loadResources()                      override;
        std::string getName()                     override;
    };
}