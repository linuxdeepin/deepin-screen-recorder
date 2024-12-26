TEMPLATE  = subdirs

isEmpty(PREFIX){
    PREFIX = /usr
}

SUBDIRS  += \
    src \
    src/dde-dock-plugins \
    src/pin_screenshots


translations.path = $$PREFIX/share/deepin-screen-recorder/translations

isEmpty(TRANSLATIONS) {
     include(./translations.pri)
}
CONFIG += update_translations release_translations

QT6_LUPDATE = /usr/lib/qt6/bin/lupdate
QT6_LRELEASE = /usr/lib/qt6/bin/lrelease
DEFAULT_LUPDATE = lupdate
DEFAULT_LRELEASE = lrelease

equals(QT_VERSION_MAJOR, 6) {
    CONFIG(update_translations) {
        isEmpty(lupdate):lupdate=$$QT6_LUPDATE
        system($$lupdate -no-obsolete -locations none $$_PRO_FILE_)
    }
    CONFIG(release_translations) {
        isEmpty(lrelease):lrelease=$$QT6_LRELEASE
        system($$lrelease $$_PRO_FILE_)
    }
} else {
    CONFIG(update_translations) {
        isEmpty(lupdate):lupdate=$$DEFAULT_LUPDATE
        system($$lupdate -no-obsolete -locations none $$_PRO_FILE_)
    }
    CONFIG(release_translations) {
        isEmpty(lrelease):lrelease=$$DEFAULT_LRELEASE
        system($$lrelease $$_PRO_FILE_)
    }
}

TRANSLATIONS_COMPILED = $$TRANSLATIONS
TRANSLATIONS_COMPILED ~= s/\.ts/.qm/g
translations.files = $$TRANSLATIONS_COMPILED

INSTALLS += translations
