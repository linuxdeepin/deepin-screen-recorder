//#include <QApplication>
#include <gtest/gtest.h>
#include <QDebug>
#include <sanitizer/asan_interface.h>
#include "test_all_interfaces.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qDebug() << "start test cases ..............";
    //testing::GTEST_FLAG(output) = "xml:./report/report.xml";
    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("./asan.log");
#endif
    //system("export ASAN_OPTIONS=halt_on_error=0");
    qDebug() << "end test cases ..............";
    //return app.exec();
    return ret;
}
