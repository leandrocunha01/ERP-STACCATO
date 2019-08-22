#
#  This file is part of QSimpleUpdater
#
#  Copyright (c) 2014 Alex Spataru <alex_spataru@outlook.com>
#
#  Please check the license.txt file for more information.
#

QT *= gui widgets network

HEADERS += $$PWD/src/qsimpleupdater.h \
           $$PWD/src/dialogs/download_dialog.h \
           $$PWD/src/dialogs/progress_dialog.h

SOURCES += $$PWD/src/qsimpleupdater.cpp \
           $$PWD/src/dialogs/download_dialog.cpp \
           $$PWD/src/dialogs/progress_dialog.cpp

OTHER_FILES += $$PWD/src/QSimpleUpdater

INCLUDEPATH += $$PWD/src

#unix:!android {
#    LIBS += -lcrypto -lssl
#}

#mac{
# LIBS += -L/opt/local/lib -lcrypto -lssl
#}

win32* {
    gcc:LIBS += -L$$_PRO_FILE_PWD_/OpenSSL-Win32 -llibeay32
    msvc:LIBS += -L$$_PRO_FILE_PWD_/OpenSSL-Win64/lib -llibeay32
}

RESOURCES += $$PWD/res/qsu_resources.qrc

FORMS += $$PWD/src/dialogs/download_dialog.ui \
         $$PWD/src/dialogs/progress_dialog.ui
