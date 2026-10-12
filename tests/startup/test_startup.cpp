// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include <QDebug>
#include "ximagefixture.h"
#ifndef STARTUP_TEST_LEGACY
#include "../../src/utils/ximageconverter.h"
#endif

static int failures = 0;
static unsigned long calls = 0;
static unsigned long (*originalGetPixel)(XImage *, int, int);
static unsigned long countingGetPixel(XImage *image, int x, int y)
{
    ++calls;
    return originalGetPixel(image, x, y);
}
static void expect(bool condition, const char *message)
{
    if (!condition) {
        qCritical() << "FAIL:" << message;
        ++failures;
    }
}
static QImage convert(XImage *image)
{
#ifdef STARTUP_TEST_LEGACY
    return legacyXImageConversion(image);
#else
    return XImageConverter::toImage(image);
#endif
}
static void checkLayout(int bits, bool native, int offset, bool fast)
{
    XImageFixture fixture(7, 5, bits, native, offset);
    const QImage expected = legacyXImageConversion(&fixture.image);
    // Poison the unused high byte: RGB32 must still become opaque.
    if (bits == 32 && native) {
        for (int y = 0; y < fixture.image.height; ++y) {
            for (int x = 0; x < fixture.image.width; ++x) {
                const int highByte = QSysInfo::ByteOrder == QSysInfo::LittleEndian ? 3 : 0;
                fixture.image.data[y * fixture.image.bytes_per_line + x * 4 + highByte] = 0x42;
            }
        }
    }
    originalGetPixel = fixture.image.f.get_pixel;
    fixture.image.f.get_pixel = countingGetPixel;
    calls = 0;
    const QImage result = convert(&fixture.image);
    expect(result == expected, "All RGB pixels and alpha must match the original conversion");
    expect(fast ? calls == 0 : calls > 0, "Native aligned RGB32 must avoid per-pixel XGetPixel; other layouts must fall back");
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x)
            expect(qAlpha(result.pixel(x, y)) == 255, "Captured pixels must be opaque");
    }
#ifndef STARTUP_TEST_LEGACY
    expect(fast ? fixture.image.data == nullptr : fixture.image.data != nullptr,
           "Native RGB32 buffers must be adopted; other layouts must stay with the XImage");
#endif
    // The returned image must not depend on storage the XImage still owns.
    fixture.release();
    expect(result == expected, "Converted image must survive destruction of XImage storage");
}
int main()
{
    checkLayout(32, true, 0, true);
    checkLayout(32, false, 0, false);
    checkLayout(24, true, 0, false);
    checkLayout(32, true, 1, false);
    expect(convert(nullptr).isNull(), "Null XImage must return a null image");
    XImage invalid{};
    expect(convert(&invalid).isNull(), "Empty XImage must return a null image");
    XImageFixture shallow(7, 5);
    shallow.image.depth = 16;
    XInitImage(&shallow.image);
    expect(convert(&shallow.image) == legacyXImageConversion(&shallow.image),
           "Nonstandard depth must preserve the legacy pixel mask");
    XImageFixture nonstandard(7, 5);
    nonstandard.image.red_mask = 0xf800;
    originalGetPixel = nonstandard.image.f.get_pixel;
    nonstandard.image.f.get_pixel = countingGetPixel;
    const QImage expected = legacyXImageConversion(&nonstandard.image);
    calls = 0;
    const QImage fallback = convert(&nonstandard.image);
    expect(calls > 0 && fallback == expected, "Nonstandard masks must preserve the legacy fallback");
    XImageFixture shortStride(7, 5);
    shortStride.image.bytes_per_line = 4;
    originalGetPixel = shortStride.image.f.get_pixel;
    shortStride.image.f.get_pixel = countingGetPixel;
    const QImage shortExpected = legacyXImageConversion(&shortStride.image);
    calls = 0;
    const QImage shortResult = convert(&shortStride.image);
    expect(calls > 0 && shortResult == shortExpected, "Short source stride must use the legacy fallback");
    XImageFixture missingData(7, 5);
    char *const missingStorage = missingData.image.data;
    missingData.image.data = nullptr;
    expect(convert(&missingData.image).isNull(), "Missing image data must be rejected");
    missingData.image.data = missingStorage;
    qInfo() << (failures ? "FAILED" : "PASSED") << failures << "failures";
    return failures ? 1 : 0;
}
