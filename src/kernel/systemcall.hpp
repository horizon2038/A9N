#ifndef SYSCALL_HPP
#define SYSCALL_HPP

namespace kernel
{
    class syscall_manager
    {
        public:
            void init();

        private:
            void init_system_call_table();
    };
}

#endif
