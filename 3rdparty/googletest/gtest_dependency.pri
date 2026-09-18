# GTest/GMock dependency for Qt6
# Links against system-installed gtest and gmock libraries

LIBS += -lgtest -lgmock -lgtest_main -lpthread
DEFINES += GTEST_HAS_PTHREAD=1
