#include <test_enviroment.hpp>

#include <gtest/gtest.h>

#include <hal/test/serial.hpp>

#include <stdio.h>

enviroment::~enviroment()
{
}

void enviroment::SetUp()
{
    setup_hal_test();
}

void enviroment::setup_hal_test()
{
    printf("\e[32m[ %-8s ]\e[0m hal::test\n", "SETUP");
    setup_logger();
}

void enviroment::setup_logger()
{
    hal_test_serial = new std_serial();
    hal_test_logger = new kernel::utility::logger(*hal_test_serial);
}

void enviroment::TearDown()
{
}
