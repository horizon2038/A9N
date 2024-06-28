#include <gtest/gtest.h>

#include <hal/interface/serial.hpp>

#include <kernel/utility/logger.hpp>

class enviroment : public ::testing::Environment
{
  public:
    ~enviroment() override;
    void SetUp() override;
    void TearDown() override;

  private:
    a9n::hal::serial             *hal_test_serial;
    a9n::kernel::utility::logger *hal_test_logger;

    void setup_hal_test();
    void setup_logger();
};

testing::Environment *const env
    = testing::AddGlobalTestEnvironment(new enviroment);
