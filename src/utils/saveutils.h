#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QObject>

enum SaveAction : unsigned int {
    SaveToDesktop,
    SaveToImage,
    AutoSave,
    SaveToSpecificDir,
    SaveToClipboard,
    SaveToAutoClipboard,
    PadDefaultPath //平板适配采用扩展的思维，尽量可能少的修改桌面代码
};

Q_DECLARE_METATYPE(SaveAction);

#endif // SAVEUTILS_H
