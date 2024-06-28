#ifndef X86_64_HAL_HPP
#define X86_64_HAL_HPP

#include <hal/interface/hal_factory.hpp>
#include <hal/interface/hal.hpp>

// core services
#include <hal/x86_64/memory/memory_manager.hpp>
#include <hal/x86_64/process/process_manager.hpp>
#include <hal/x86_64/interrupt/interrupt.hpp>

// platform services
#include <hal/x86_64/arch/arch_initializer.hpp>

// peripheral drivers
#include <hal/x86_64/time/pit_timer.hpp>
#include <hal/x86_64/io/port_io.hpp>
#include <hal/x86_64/io/serial.hpp>

namespace a9n::hal::x86_64
{
    class hal_factory final : public a9n::hal::hal_factory
    {
      public:
        a9n::hal::hal      *make() override;
        constexpr static uint64_t hal_size = sizeof(a9n::hal::hal);
        alignas(a9n::hal::hal) static char hal_buffer[];

        constexpr static uint64_t memory_manager_size
            = sizeof(a9n::hal::x86_64::memory_manager);
        alignas(a9n::hal::x86_64::memory_manager
        ) static char memory_manager_buffer[];

        constexpr static uint64_t process_manager_size
            = sizeof(a9n::hal::x86_64::process_manager);
        alignas(a9n::hal::x86_64::process_manager
        ) static char process_manager_buffer[];

        constexpr static uint64_t interrupt_size
            = sizeof(a9n::hal::x86_64::interrupt);
        alignas(a9n::hal::x86_64::interrupt) static char interrupt_buffer[];

        constexpr static uint64_t arch_initializer_size
            = sizeof(a9n::hal::x86_64::arch_initializer);
        alignas(a9n::hal::x86_64::arch_initializer
        ) static char arch_initializer_buffer[];

        constexpr static uint64_t port_io_size = sizeof(a9n::hal::x86_64::port_io);
        alignas(a9n::hal::x86_64::port_io) static char port_io_buffer[];

        constexpr static uint64_t serial_size = sizeof(a9n::hal::x86_64::serial);
        alignas(a9n::hal::x86_64::serial) static char serial_buffer[];

        constexpr static uint64_t timer_size = sizeof(a9n::hal::x86_64::pit_timer);
        alignas(a9n::hal::x86_64::pit_timer) static char timer_buffer[];

      private:
    };
}

#endif
