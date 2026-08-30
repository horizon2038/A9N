extern "C" void __cxa_pure_virtual()
{
    for (;;)
    {
        asm volatile("wfe");
    }
}
