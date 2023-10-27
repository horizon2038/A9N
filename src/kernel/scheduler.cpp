#include "scheduler.hpp"
#include "common.hpp"

#include <library/logger.hpp>

namespace kernel
{
    scheduler::scheduler(process *process_list)
    : _process_list(process_list)
    , _current_process(nullptr)
    {
        current_process_index = 0;
    }
    
    scheduler::~scheduler()
    {
    }

    process *scheduler::schedule_next_process()
    {
        /*
        utility::logger::printk("schedule_next_process\n");

        process *target_process = &_process_list[current_process_index];

        while(target_process->status != process_status::READY)
        {
            utility::logger::printk("loop: %d\n", current_process_index);
            target_process = &_process_list[current_process_index];

            utility::logger::printk("process_name: %s, process_id: %llu, process_entry: %llx\n", target_process->name, target_process->id, reinterpret_cast<virtual_address>(target_process->stack_pointer));

            if (current_process_index > 512)
            {
                current_process_index = 0;
                break;
            }
            current_process_index++;
        }

        if (target_process->status != process_status::READY)
        {
            utility::logger::printk("can't find schedulable process\n");
            return nullptr;
        }

        target_process->status = process_status::RUNNING;
        utility::logger::printk("set running\n");
        _current_process = target_process;
        utility::logger::printk("_current_process_index : %d\n", current_process_index);
        _current_process->status = process_status::READY;
        utility::logger::printk("set ready\n");
        return target_process;
        */
        utility::logger::printk("schedule_next_process\n");

        process *target_process = nullptr;
        uint32_t start_index = current_process_index;

        do
        {
            // インデックスをインクリメントする前に512を超えているかチェック
            if (current_process_index >= 12)
            {
                current_process_index = 0;
            }

            target_process = &_process_list[current_process_index];
            // utility::logger::printk("loop: %d\n", current_process_index);
            // utility::logger::printk("process_name: %s, process_id: %llu, process_entry: %llx\n", target_process->name, target_process->id, reinterpret_cast<virtual_address>(target_process->stack_pointer));

            // インデックスをインクリメント
            current_process_index++;

        }
        while(target_process->status != process_status::READY && current_process_index != start_index);  // 全プロセスをチェックするか、READY状態のプロセスを見つけるまでループ

        if (target_process->status != process_status::READY)
        {
            utility::logger::printk("can't find schedulable process\n");
            return nullptr;
        }

        // 新しい_current_processを設定する前に前の_current_processのstatusをREADYに設定
        if (_current_process != nullptr)
        {
            _current_process->status = process_status::READY;
        }

        target_process->status = process_status::RUNNING;
        utility::logger::printk("set running\n");
        _current_process = target_process;
        utility::logger::printk("_current_process_index : %d\n", current_process_index);
        utility::logger::printk("target_process_address : 0x%llx\n", reinterpret_cast<kernel::virtual_address>(target_process));

        return target_process;
    }
}
