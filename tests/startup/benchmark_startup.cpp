// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <algorithm>
#include <vector>
#include "ximagefixture.h"
#include "../../src/utils/ximageconverter.h"
#include "../../src/utils/cpuvendor.h"

static double median(std::vector<double> samples)
{
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}
static void benchmarkImage(XImage *image, const QString &label)
{
    QTextStream output(stdout);
    const QImage expected = legacyXImageConversion(image);
    XImageClone check(*image);
    const QImage actual = XImageConverter::toImage(&check.image);
    if (expected != actual)
        qFatal("Pixel mismatch during benchmark");
    std::vector<double> before, after;
    QElapsedTimer timer;
    // Alternate order to reduce cache/temperature bias.
    for (int i = 0; i < 31; ++i) {
        for (int step = 0; step < 2; ++step) {
            const bool fast = (i + step) % 2 == 0;
            // The optimized conversion consumes its input, so each sample gets a fresh copy.
            XImageClone input(*image);
            timer.start();
            QImage result = fast ? XImageConverter::toImage(&input.image) : legacyXImageConversion(&input.image);
            const double elapsed = timer.nsecsElapsed() / 1e6;
            if (result.isNull())
                qFatal("Conversion failed during benchmark");
            (fast ? after : before).push_back(elapsed);
        }
    }
    output << label << ": legacy median " << median(before) << " ms, optimized median "
           << median(after) << " ms; pixels identical\n";
}
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    for (const QSize size : {QSize(2880, 1800), QSize(3840, 2160)}) {
        XImageFixture fixture(size.width(), size.height());
        benchmarkImage(&fixture.image, QString("synthetic %1x%2").arg(size.width()).arg(size.height()));
    }
    if (app.arguments().contains("--desktop")) {
        Display *display = XOpenDisplay(nullptr);
        if (!display)
            qFatal("Cannot open X display for real desktop benchmark");
        XWindowAttributes attributes{};
        XGetWindowAttributes(display, DefaultRootWindow(display), &attributes);
        XImage *image = XGetImage(display, DefaultRootWindow(display), 0, 0,
                                 attributes.width, attributes.height, AllPlanes, ZPixmap);
        if (!image)
            qFatal("Cannot capture desktop for benchmark");
        benchmarkImage(image, QStringLiteral("actual desktop"));
        XDestroyImage(image);
        XCloseDisplay(display);
    }
    std::vector<double> before, after;
    QElapsedTimer timer;
    for (int i = 0; i < 31; ++i) {
        timer.start();
        QProcess process;
        process.start("lscpu");
        if (!process.waitForFinished(5000) || process.exitCode() != 0)
            qFatal("lscpu failed during benchmark");
        const bool oldValue = process.readAllStandardOutput().toLower().contains("centaurhauls");
        before.push_back(timer.nsecsElapsed() / 1e6);
        timer.start();
        QFile cpuInfo(QStringLiteral("/proc/cpuinfo"));
        if (!cpuInfo.open(QIODevice::ReadOnly))
            qFatal("Cannot read cpuinfo during benchmark");
        const bool newValue = CpuVendor::isZhaoxin(cpuInfo);
        after.push_back(timer.nsecsElapsed() / 1e6);
        if (oldValue != newValue)
            qFatal("CPU detection mismatch on this machine");
    }
    QTextStream(stdout) << "CPU detection: lscpu median " << median(before)
                        << " ms, procfs median " << median(after) << " ms\n";
}
