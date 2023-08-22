#ifndef INTERRUPT_HANDLER_HPP
#define INTERRUPT_HANDLER_HPP

namespace hal::interface
{
    class interrupt_handler 
    {
        public:
            virtual void handle() = 0;
    };

}
#endif
