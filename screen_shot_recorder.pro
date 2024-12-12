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

# 打印 lupdate 和 lrelease 的路径
message("lupdate path11: $$system(which lupdate)")
message("lrelease path11: $$system(which lrelease)")

CONFIG(update_translations) {
    isEmpty(lupdate):lupdate=/usr/lib/qt6/bin/lupdate
    system($$lupdate -no-obsolete -locations none $$_PRO_FILE_)
}
CONFIG(release_translations) {
    isEmpty(lrelease):lrelease=/usr/lib/qt6/bin/lrelease
    system($$lrelease $$_PRO_FILE_)
}

# 打印 lupdate 和 lrelease 的路径
message("lupdate path22: $$system(which lupdate)")
message("lrelease path22: $$system(which lrelease)")

TRANSLATIONS_COMPILED = $$TRANSLATIONS
TRANSLATIONS_COMPILED ~= s/\.ts/.qm/g
translations.files = $$TRANSLATIONS_COMPILED

INSTALLS += translations
