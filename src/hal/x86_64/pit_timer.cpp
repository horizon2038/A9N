#include "pit_timer.hpp"

namespace hal::x86_64
{
    constexpr uint32_t CLOCK_RATE = 1193182;
    constexpr uint16_t PIT_CHANNEL_0 = 0x40;
    constexpr uint16_t PIT_CHANNEL_1 = 0x41;
    constexpr uint16_t PIT_CHANNEL_2 = 0x42;
    constexpr uint16_t PIT_COMMAND_REGISTER = 0x43;

    pit_timer *pit_timer::this_timer = nullptr;

    pit_timer::pit_timer(hal::interface::port_io &injected_port_io, hal::interface::serial &injected_serial)
        : _port_io(injected_port_io),
        _serial(injected_serial)
    {
        this_timer = this;
        _serial.write_string_serial("init pit_timer in constructor!\n");
    }

    pit_timer::~pit_timer()
    {
    }

    void pit_timer::init_timer()
    {
        init_pic();
        configure_timer(100);
    }

    void pit_timer::init_pic()
    {
        _port_io.write(0x20, 0x11); // マスターピックに初期化コマンド（0x11）を送信
        _port_io.write(0xa0, 0x11); // スレーブピックに初期化コマンド（0x11）を送信

        _port_io.write(0x21, 0x20); // マスターピックのIRQ 0-7を割り当て（0x20-0x27）
        _port_io.write(0xa1, 0x28); // スレーブピックのIRQ 8-15を割り当て（0x28-0x2F）

        _port_io.write(0x21, 0x04); // マスターピックにスレーブピックの接続を通知（ビット2をセット）
        _port_io.write(0xa1, 0x02); // スレーブピックにマスターピックへの接続を通知（スレーブピックのIR2ラインを使用）

        _port_io.write(0x21, 0x01); // マスターピックを正常モードに設定
        _port_io.write(0xa1, 0x01); // スレーブピックを正常モードに設定

        _port_io.write(0x21, 0x00);
        _port_io.write(0xa1, 0x00);
    }

    void pit_timer::eoi_pic()
    {
        _port_io.write(0x20, 0x20);
    }


    void pit_timer::configure_timer(uint16_t hz)
    {
        uint32_t pit_rate = CLOCK_RATE / hz;
        _port_io.write(PIT_COMMAND_REGISTER, 0x36);
        _port_io.write(PIT_CHANNEL_0, pit_rate & 0xff);
        _port_io.write(PIT_CHANNEL_0, pit_rate >> 8);
    }

    void pit_timer::handle(void *data)
    {
        this_timer->_serial.write_string_serial("test_handle\n");
        this_timer->clock();
        this_timer->eoi_pic();
        // volatile auto end_of_interrupt = reinterpret_cast<uint32_t*>(0xfee000b0);
        // *end_of_interrupt = 0;
    }

    void pit_timer::clock()
    {
        ticks++;
    }

    uint32_t pit_timer::get_tick()
    {
        return ticks;
    }

}
