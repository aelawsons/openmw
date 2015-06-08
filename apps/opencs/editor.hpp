#ifndef CS_EDITOR_H
#define CS_EDITOR_H

#include <memory>

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem/fstream.hpp>

#include <QObject>
#include <QString>
#include <QLocalServer>
#include <QLocalSocket>

#ifndef Q_MOC_RUN
#include <components/files/configurationmanager.hpp>
#endif

#include <components/files/multidircollection.hpp>

#include "model/settings/usersettings.hpp"
#include "model/doc/documentmanager.hpp"

#include "view/doc/viewmanager.hpp"
#include "view/doc/startup.hpp"
#include "view/doc/filedialog.hpp"
#include "view/doc/newgame.hpp"

#include "view/settings/dialog.hpp"

namespace VFS
{
    class Manager;
}

namespace CS
{
    class Editor : public QObject
    {
            Q_OBJECT

            // FIXME: should be moved to document, so we can have different resources for each opened project
            std::auto_ptr<VFS::Manager> mVFS;

            Files::ConfigurationManager mCfgMgr;
            CSMSettings::UserSettings mUserSettings;
            CSMDoc::DocumentManager mDocumentManager;
            CSVDoc::ViewManager mViewManager;
            CSVDoc::StartupDialogue mStartup;
            CSVDoc::NewGameDialogue mNewGame;
            CSVSettings::Dialog mSettings;
            CSVDoc::FileDialog mFileDialog;
            boost::filesystem::path mLocal;
            boost::filesystem::path mResources;
            boost::filesystem::path mPid;
            boost::interprocess::file_lock mLock;
            boost::filesystem::ofstream mPidFile;
            bool mFsStrict;

            void setupDataFiles (const Files::PathContainer& dataDirs);

            std::pair<Files::PathContainer, std::vector<std::string> > readConfig();
            ///< \return data paths

            // not implemented
            Editor (const Editor&);
            Editor& operator= (const Editor&);

        public:

            Editor ();
            ~Editor ();

            bool makeIPCServer();
            void connectToIPCServer();

            int run();
            ///< \return error status

        private slots:

            void createGame();
            void createAddon();
            void cancelCreateGame();
            void cancelFileDialog();

            void loadDocument();
            void openFiles (const boost::filesystem::path &path);
            void createNewFile (const boost::filesystem::path& path);
            void createNewGame (const boost::filesystem::path& file);

            void showStartup();

            void showSettings();

            void documentAdded (CSMDoc::Document *document);

            void lastDocumentDeleted();

        private:

            QString mIpcServerName;
            QLocalServer *mServer;
            QLocalSocket *mClientSocket;
    };
}

#endif
