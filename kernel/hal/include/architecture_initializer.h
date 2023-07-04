#ifndef ARCHITECTURE_INITIALIZER
#define ARCHITECTURE_INITIALIZER

class architecture_initializer
{
    public:
        virtual ~architecture_initializer() {};
        // architecture-dependent processing is done in the constructor
        // Possibility to make the Code cleaner by introducing a kernel module
        virtual void init_interrupt() = 0;
        virtual void init_memory() = 0;
        virtual void init_timer() = 0;
        virtual void init_serial_device() = 0;
        // init_serial_device is for debugging. it will be removed in release.
};

#endif