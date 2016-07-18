/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2016 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "btinstallmgr.h"

#include "../util/btassert.h"
#include "btinstallbackend.h"


using namespace swordxx;

BtInstallMgr::BtInstallMgr(QObject * parent)
        : QObject(parent)
        , InstallMgr(BtInstallBackend::configPath().toLatin1(), this)
        , m_totalBytes(1)
        , m_completedBytes(0)
        , m_firstCallOfPreStatus(true)
{ // Use this class also as status reporter:
    this->setFTPPassive(true);
}

BtInstallMgr::~BtInstallMgr() {
    //doesn't really help because it only sets a flag
    this->terminate(); // make sure to close the connection
}

bool BtInstallMgr::isUserDisclaimerConfirmed() const {
    //// \todo Check from config if it's been confirmed with "don't show this anymore" checked.
    // Create a dialog with the message, checkbox and Continue/Cancel, Cancel as default.
    return true;
}

void BtInstallMgr::update(std::size_t dltotal, std::size_t dlnow) noexcept {
    static auto const calculateIntPercentage =
            [](std::size_t done, std::size_t total) noexcept -> int {
                if (done > total)
                    done = total;
                if (total == 0)
                    return 100;
                int r = double(done) * 100.0 / total;
                if (r < 0)
                    return 0;
                return (r > 100) ? 100 : r;
            };

    const int totalPercent = calculateIntPercentage(dlnow + m_completedBytes,
                                                    m_totalBytes);
    const int filePercent  = calculateIntPercentage(dlnow, dltotal);

    //qApp->processEvents();
    emit percentCompleted(totalPercent, filePercent);
}


void BtInstallMgr::preStatus(std::size_t totalBytes,
                             std::size_t completedBytes,
                             const char * message) noexcept
{
    Q_UNUSED(message);
    BT_ASSERT(completedBytes <= totalBytes);
    if (m_firstCallOfPreStatus) {
        m_firstCallOfPreStatus = false;
        emit downloadStarted();
    }
    m_completedBytes = completedBytes;
    m_totalBytes = totalBytes;
}
