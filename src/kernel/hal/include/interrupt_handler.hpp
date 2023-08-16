#ifndef INTERRUPT_HANDLER_HPP
#define INTERRUPT_HANDLER_HPP

namespace hal
{
    class interrupt_handler 
    {
        public:
            virtual void handle() = 0;
    };

}
#endif
