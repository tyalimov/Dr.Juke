#pragma once

#include "i_analyzer.h"

#include <windows.h>
#include <wintrust.h>
#include <Softpub.h>

#include <filesystem>

/*
    Анализатор файлов, проверяющий ЭЦП исполняемых файлов.
    Формат отчета от анализатора:
    {
        "resolution"  : (non-signed|signed|non-trusted|disallowded|error) ----- Результат проверки
        "signer"      : <строка> -- Подписывающий, при non-signed или error - "undefined"
        "application" : <строка> -- Название приложения, при non-signed или error - "undefined"
        "datetime"    : <строка> -- Дата и время подписывания, при non-signed или error - "undefined"
        "url"         : <строка>
    }
*/

namespace drjuke::scanlib
{
    class SignatureReport : public IReport
    {
    public:
        SignatureReport
        (
        //    bool is_valid,
        //    std::string signer
        );
    };

    class SignatureAnalyzer : public IAnalyzer
    {
    private:
        WINTRUST_FILE_INFO m_file_info;
        WINTRUST_DATA      m_win_trust_data;
        GUID               m_wvt_policy_guid;

    public:

        SignatureAnalyzer()
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

        IReportPtr getReport(const Path& path) override;
        void loadResources()                   override;
        std::string getName()                  override;
    };
}