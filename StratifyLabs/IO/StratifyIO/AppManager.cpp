/*
Copyright 2016 Tyler Gilbert

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "AppManager.h"

#include <QDebug>
#include <QFile>

using namespace StratifyIO;

AppManager::AppManager(Link & link) : DeviceManager(link){}
AppManager::AppManager(Link & link, Link::update_callback_t update, void * context) : DeviceManager(link, update, context){}


int AppManager::kill(const QString & name){
    int pid;

    pid = mLink.get_pid(name.toStdString());
    if( pid < 0 ){
        //not currently running
        return 0;
    }

    mLink.kill_pid(pid, LINK_SIGKILL);
    return 0;
}


int AppManager::prepareBinary(const QString & sourcePath, const QString & name, bool startup, bool ram, int ramSize){
    if( mLink.update_binary_install_options(sourcePath.toStdString().c_str(), name.toStdString().c_str(), startup, ram, ramSize) < 0 ){
        mError = mLink.error_message().c_str();
        qDebug() << "Failed to prepare binary";
        return -1;
    }
    return 0;
}


int AppManager::installApp(const QString & sourcePath, const QString & installPath, const QString & name){

    QString unlinkPath;
    //delete the app if it currently is in the app system
    unlinkPath = "/app/flash/" + name;

    qDebug() << "unlink" << unlinkPath;

    while( mLink.unlink(unlinkPath.toStdString()) >= 0 ){
        //delete all versions
        qDebug() << "unlinked" << unlinkPath;
    }

    unlinkPath = "/app/ram/" + name;
    while( mLink.unlink(unlinkPath.toStdString()) >= 0 ){
        //delete all versions
        qDebug() << "unlinked" << unlinkPath;
    }

    qDebug() << "Install" << sourcePath << "at" << installPath << "as" << name;
    if( mLink.install_app(sourcePath.toStdString(),
                           installPath.toStdString(),
                           name.toStdString(),
                           mUpdate, mContext) < 0 ){

        qDebug() << "Failed to install App" << name << "from" << sourcePath << "in" << installPath;
        mError = mLink.error_message().c_str();
        return -1;
    }

    return 0;
}

int AppManager::runApp(const QString & name){
    QString appPath;
    //look for the full path (flash or RAM)

    if( mLink.get_is_connected() == false ){
        mError = "Can't run (not connected)";
        return -1;
    }


    if( mLink.get_pid(name.toStdString()) >= 0 ){
        mError = name + " is already running";
        return 1;
    }


    appPath = "/app/flash/" + name;
    if( mLink.run_app(appPath.toStdString()) < 0 ){
        appPath = "/app/ram/" + name;
        if( mLink.run_app(appPath.toStdString()) < 0 ){
            mError = "Failed to run app " + QString(mLink.error_message().c_str());
            return -1;
        }
    }
    return 0;
}


int AppManager::uninstallApp(const QString & path, const QString & name){

    qDebug() << "Uninstall App" << path << name;

    if( mLink.unlink("/app/flash/" + name.toStdString()) == 0 ){
        return 0;
    }

    if( mLink.unlink("/app/ram/" + name.toStdString()) == 0 ){
        return 0;
    }

    if( mLink.unlink(QString(path + "/" + name).toStdString()) == 0 ){
        return 0;
    }

    mError = "Failed to uninstall " + name;
    return -1;

}

int AppManager::installFiles(const QString & settingsPath){

    qDebug() << "Settings Path" << settingsPath;
    //settingsPath is a JSON file which says which files are to be installed

    return 0;
}