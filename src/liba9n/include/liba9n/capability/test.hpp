#ifndef TEST_HPP
#define TEST_HPP

#include "liba9n/result/__result/result_common.hpp"
#include <liba9n/result/result.hpp>

namespace a9n::kernel
{
    enum class my_error
    {
        A,
        B,
    };

    auto some_func(int x) -> liba9n::result<int, my_error>
    {
        if (x == 0)
        {
            return 128;
        }

        return my_error::B;
    }

    int my_main()
    {
        auto res
            = some_func(2)
                  .and_then(
                      [=](int x) -> liba9n::result<int, my_error>
                      {
                          if (x == 0)
                          {
                              return 128;
                          }

                          return my_error::A;
                      }
                  )
                  .or_else(
                      [=](my_error e) -> liba9n::result<int, my_error>
                      {
                          return 0;
                      }
                  );
    }
}

#endif
