#pragma once

#include <windows.h>
#include <wintrust.h>
#include <Softpub.h>

#include <filesystem>

#include "base_analyzer.h"

/*
    Анализатор файлов, проверяющий ЭЦП исполняемых файлов.
    Формат отчета от анализатора:
    {
        "status" : (non-signed|signed|non-trusted|disallowded|error) ----- Результат проверки
    }
*/

namespace drjuke::scanlib
{
    class SignatureReport final
        : public BaseReport
    {
        std::string m_status;

    private:

        void initializeJson();
        void fillJsonWithStatus();

    public:

        explicit SignatureReport(const std::string &status)
            : m_status(status)
        {
            initializeJson();
            fillJsonWithStatus();
        }

        Json makeJson() override;
    };

    class DigitalSignatureAnalyzer final 
        : public BaseAnalyzer
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

    public:

        BaseReportPtr getReport(const Path& path) override;
        void loadResources()                      override;
        std::string getName()                     override;
    };
}