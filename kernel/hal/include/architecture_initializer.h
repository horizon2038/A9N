class architecture_initializer
{
    public:
        virtual ~architecture_initializer() {};
        // architecture-dependent processing is done in the constructor
        virtual void init_interrupt() = 0;
        virtual void init_memory() = 0;
        virtual void init_timer() = 0;
        virtual void init_serial_device() = 0;
        // init_serial_device is for debugging. it will be removed in release.
};