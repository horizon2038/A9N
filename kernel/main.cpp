extern "C" void kernel_main()
{
    while(true)
    {
        __asm__ ("hlt");
    }
}