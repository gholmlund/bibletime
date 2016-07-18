/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2016 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "btinstallbackend.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <string>
#include <swordxx/filemgr.h>
#include <swordxx/swconfig.h>
#include "../util/btassert.h"
#include "../util/directory.h"
#include "managers/cswordbackend.h"
#include "btinstallmgr.h"


using namespace swordxx;

namespace BtInstallBackend {

/** Adds the source described by Source to the backend. */
bool addSource(swordxx::InstallSource& source) {
    SWConfig config(configFilename().toLatin1());
    if (isRemote(source)) {
        if (source.directory[ source.directory.length()-1 ] == '/') {
            source.directory.pop_back();
        }
    if (!strcmp(source.type.c_str(), "FTP")) {
            config["Sources"].insert( std::make_pair(std::string("FTPSource"), source.getConfEnt()) );
        }
        else if (!strcmp(source.type.c_str(), "SFTP")) {
            config["Sources"].insert( std::make_pair(std::string("SFTPSource"), source.getConfEnt()) );
        }
        else if (!strcmp(source.type.c_str(), "HTTP")) {
            config["Sources"].insert( std::make_pair(std::string("HTTPSource"), source.getConfEnt()) );
        }
        else if (!strcmp(source.type.c_str(), "HTTPS")) {
            config["Sources"].insert( std::make_pair(std::string("HTTPSSource"), source.getConfEnt()) );
        }
    }
    else if (!strcmp(source.type.c_str(), "DIR")) {
        config["Sources"].insert( std::make_pair(std::string("DIRSource"), source.getConfEnt()) );
    }
    config.Save();
    return true;
}

/** Returns the Source struct. */
swordxx::InstallSource source(const QString &name) {
    BtInstallMgr mgr;
    InstallSourceMap::iterator source = mgr.sources.find(name.toLatin1().data());
    if (source != mgr.sources.end()) {
        return *(source->second);
    }
    else { //not found in Sword++, may be a local DIR source
        SWConfig config(configFilename().toLatin1());
        SectionMap::iterator sourcesSection = config.Sections.find("Sources");
        if (sourcesSection != config.Sections.end()) {
            ConfigEntMap::iterator sourceBegin =
                sourcesSection->second.lower_bound("DIRSource");
            ConfigEntMap::iterator sourceEnd =
                sourcesSection->second.upper_bound("DIRSource");

            while (sourceBegin != sourceEnd) {
                InstallSource is("DIR", sourceBegin->second.c_str());
                if (!strcmp(is.caption.c_str(), name.toLatin1())) { //found local dir source
                    return is;
                }

                ++sourceBegin; //next source
            }
        }
    }

    InstallSource is("EMPTY");   //default return value
    is.caption = "unknown caption";
    is.source = "unknown source";
    is.directory = "unknown dir";
    return is;
}

/** Deletes the source. */
bool deleteSource(const QString &name) {
    swordxx::InstallSource is = source(name );

    SWConfig config(configFilename().toLatin1());

    //this code can probably be shortened by using the stl remove_if functionality
    std::string sourceConfigEntry = is.getConfEnt();
    bool notFound = true;
    ConfigEntMap::iterator it = config["Sources"].begin();
    while (it != config["Sources"].end()) {
        //SWORD lib gave us a "nice" surprise: getConfEnt() adds uid, so old sources added by BT are not recognized here
        if (it->second == sourceConfigEntry) {
            config["Sources"].erase(it);
            notFound = false;
            break;
        }
        ++it;
    }
    if (notFound) {
        qDebug() << "source was not found, trying without uid";
        //try again without uid
        QString sce(sourceConfigEntry.c_str());
        QStringList l = sce.split('|');
        l.removeLast();
        sce = l.join("|").append("|");
        it = config["Sources"].begin();
        while (it != config["Sources"].end()) {
            if (QString::fromStdString(it->second) == sce) {
                config["Sources"].erase(it);
                break;
            }
            ++it;
        }
    }

    config.Save();
    return true; /// \todo dummy
}

/** Returns the moduleinfo list for the source. Delete the pointer after using. IS THIS POSSIBLE?*/
QList<CSwordModuleInfo*> moduleList(QString name) {
    /// \todo dummy
    Q_UNUSED(name);
    BT_ASSERT(false && "not implemented");
    return QList<CSwordModuleInfo*>();
}

bool isRemote(const swordxx::InstallSource& source) {
    return !strcmp(source.type.c_str(), "FTP") ||
            !strcmp(source.type.c_str(), "SFTP") ||
            !strcmp(source.type.c_str(), "HTTP") ||
            !strcmp(source.type.c_str(), "HTTPS");
}

QString configPath() {
    return util::directory::getUserHomeSwordDir().absolutePath().append("/InstallMgr");
}

QString configFilename() {
    return configPath().append("/InstallMgr.conf");
}

QStringList targetList() {
    QStringList names = CSwordBackend::instance()->swordDirList();
    return names;
}

bool setTargetList( const QStringList& targets ) {
    namespace DU = util::directory;

    //saves a new Sword++ config using the provided target list
    //QString filename = KGlobal::dirs()->saveLocation("data", "bibletime/") + "swordxx.conf"; //default is to assume the real location isn't writable
    //QString filename = util::DirectoryUtil::getUserBaseDir().canonicalPath().append("/.swordxx/swordxx.conf");
    //bool directAccess = false;
    QString filename = swordConfigFilename();
    {
        QFile f(filename);
        if (!f.exists()) {
            if (!f.open(QIODevice::ReadWrite)) {
                qWarning() << "The Sword++ config file can't be created!";
                return false;
            }
            f.close();
            qDebug() << "The Sword++ config file \"" << filename
                     << "\" had to be (re)created!";
        }
    }

    filename = util::directory::convertDirSeparators(filename);
    SWConfig conf(filename.toLocal8Bit());
    conf.Sections.clear();

#ifdef Q_OS_WIN
    // On Windows, add the Sword++ directory to the config file.
    QString swordPath = DU::convertDirSeparators( DU::getApplicationSwordDir().absolutePath());
    conf["Install"].insert(
        std::make_pair(std::string("LocalePath"), swordPath.toLocal8Bit().data())
    );
#endif

    bool setDataPath = false;
    for (QStringList::const_iterator it = targets.begin(); it != targets.end(); ++it) {
        QString t = DU::convertDirSeparators(*it);
#ifdef Q_OS_WIN
        if (t.contains(DU::convertDirSeparators(DU::getUserHomeDir().canonicalPath().append("\\Swordxx")))) {
#else
        if (t.contains(DU::getUserHomeDir().canonicalPath().append("/.swordxx"))) {
#endif
            //we don't want $HOME/.swordxx in the config
            continue;
        }
        else {
            qDebug() << "Add path to the conf file" << filename << ":" << t;
            conf["Install"].insert(
                        std::make_pair(std::string(!setDataPath
                                                   ? "DataPath"
                                                   : "AugmentPath"),
                                       t.toLocal8Bit().data()));
            setDataPath = true;
        }
    }
    qDebug() << "Saving Sword++ configuration ...";
    conf.Save();
    CSwordBackend::instance()->reloadModules(CSwordBackend::PathChanged);
    return true;
}

QStringList sourceNameList() {
    BtInstallMgr mgr;
    BT_ASSERT(mgr.installConf);

    QStringList names;

    //add Sword++ remote sources
    for (InstallSourceMap::iterator it = mgr.sources.begin(); it != mgr.sources.end(); ++it) {
        names << QString::fromLocal8Bit(it->second->caption.c_str());
    }

    // Add local directory sources
    SWConfig config(configFilename().toLatin1());
    swordxx::SectionMap::iterator sourcesSection = config.Sections.find("Sources");
    if (sourcesSection != config.Sections.end()) {
        swordxx::ConfigEntMap::iterator sourceBegin = sourcesSection->second.lower_bound("DIRSource");
        swordxx::ConfigEntMap::iterator sourceEnd = sourcesSection->second.upper_bound("DIRSource");

        while (sourceBegin != sourceEnd) {
            InstallSource is("DIR", sourceBegin->second.c_str());
            names << QString::fromLatin1(is.caption.c_str());

            ++sourceBegin;
        }
    }

    return names;
}


void initPassiveFtpMode() {
    SWConfig config(configFilename().toLatin1());
    config["General"]["PassiveFTP"] = "true";
    config.Save();
}
QString swordConfigFilename() {
    namespace DU = util::directory;

    qDebug() << "Sword++ config:"
#ifdef Q_OS_WIN
             << DU::getUserHomeDir().absolutePath().append("/Swordxx/swordxx.conf");
    return DU::getUserHomeDir().absolutePath().append("/Swordxx/swordxx.conf");
//    return DU::getApplicationDir().absolutePath().append("/swordxx.conf");
#else
             << DU::getUserHomeDir().absolutePath().append("/.swordxx/swordxx.conf");
    return DU::getUserHomeDir().absolutePath().append("/.swordxx/swordxx.conf");
#endif
}

QDir swordDir() {
    namespace DU = util::directory;

#ifdef Q_OS_WIN
    return QDir(DU::getUserHomeDir().absolutePath().append("/Swordxx/"));
#else
    return QDir(DU::getUserHomeDir().absolutePath().append("/.swordxx/"));
#endif
}

CSwordBackend * backend(const swordxx::InstallSource & is) {
    /// \anchor BackendNotSingleton
    CSwordBackend * const ret = new CSwordBackend(isRemote(is)
                                                  ? is.localShadow.c_str()
                                                  : is.directory.c_str(),
                                                  false);
    ret->initModules(CSwordBackend::OtherChange);
    return ret;
}

} // namespace BtInstallBackend
