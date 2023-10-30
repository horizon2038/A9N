// test user program

void main();

void _start()
{
    main();
}

void main()
{
    while(0)
    {
        asm volatile ("nop");
    }
}
