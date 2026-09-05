#include <gtest/gtest.h>

#include <kernel/process/process.hpp>
#include <kernel/process/scheduler.hpp>

namespace a9n::kernel
{
    namespace
    {
        process ready_process(a9n::sword priority)
        {
            process target {};
            target.status   = process_status::READY;
            target.priority = priority;
            return target;
        }
    }

    TEST(scheduler_test, remove_only_process)
    {
        scheduler scheduler_core {};
        auto      target = ready_process(4);

        ASSERT_TRUE(scheduler_core.add_process(&target));
        ASSERT_TRUE(scheduler_core.remove_process(&target));
        EXPECT_EQ(target.next, nullptr);
        EXPECT_EQ(target.preview, nullptr);

        auto result = scheduler_core.schedule();
        ASSERT_FALSE(result);
        EXPECT_EQ(result.unwrap_error(), scheduler_error::PROCESS_NOT_IN_QUEUE);
    }

    TEST(scheduler_test, remove_middle_process)
    {
        scheduler scheduler_core {};
        auto      first  = ready_process(4);
        auto      middle = ready_process(4);
        auto      last   = ready_process(4);

        ASSERT_TRUE(scheduler_core.add_process(&first));
        ASSERT_TRUE(scheduler_core.add_process(&middle));
        ASSERT_TRUE(scheduler_core.add_process(&last));
        ASSERT_TRUE(scheduler_core.remove_process(&middle));

        auto first_result = scheduler_core.schedule();
        ASSERT_TRUE(first_result);
        EXPECT_EQ(first_result.unwrap(), &first);

        auto last_result = scheduler_core.schedule();
        ASSERT_TRUE(last_result);
        EXPECT_EQ(last_result.unwrap(), &last);

        auto empty_result = scheduler_core.schedule();
        ASSERT_FALSE(empty_result);
        EXPECT_EQ(empty_result.unwrap_error(), scheduler_error::PROCESS_NOT_IN_QUEUE);
    }

    TEST(scheduler_test, remove_process_not_in_queue)
    {
        scheduler scheduler_core {};
        auto      target = ready_process(4);

        auto result = scheduler_core.remove_process(&target);
        ASSERT_FALSE(result);
        EXPECT_EQ(result.unwrap_error(), scheduler_error::PROCESS_NOT_IN_QUEUE);
    }

    TEST(scheduler_test, reject_duplicate_only_process)
    {
        scheduler scheduler_core {};
        auto      target = ready_process(4);

        ASSERT_TRUE(scheduler_core.add_process(&target));
        auto duplicate = scheduler_core.add_process(&target);
        ASSERT_FALSE(duplicate);
        EXPECT_EQ(duplicate.unwrap_error(), scheduler_error::PROCESS_ALREADY_EXISTS_IN_QUEUE);

        auto result = scheduler_core.schedule();
        ASSERT_TRUE(result);
        EXPECT_EQ(result.unwrap(), &target);
        EXPECT_FALSE(target.is_in_ready_queue);

        auto empty_result = scheduler_core.schedule();
        ASSERT_FALSE(empty_result);
        EXPECT_EQ(empty_result.unwrap_error(), scheduler_error::PROCESS_NOT_IN_QUEUE);
    }
}
