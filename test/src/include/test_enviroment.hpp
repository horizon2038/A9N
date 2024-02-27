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
    hal::interface::serial *hal_test_serial;
    kernel::utility::logger *hal_test_logger;

    void setup_hal_test();
    void setup_logger();
};

testing::Environment *const env
    = testing::AddGlobalTestEnvironment(new enviroment);
