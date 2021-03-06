#include "instruction/Instruction.h"

#include "CPU.h"
#include "memory/WordAddressable.h"

namespace GameBoy {

auto Instruction::execute(CPU& cpu) -> void
{
    perform_operation(cpu);
    for (auto& postOp : m_postOperationActions)
        postOp();
    move_program_counter(cpu);
    tick_clock(cpu);
}

auto Instruction::with_cycles(uint8_t numCycles) -> Instruction&
{
    m_cycles = numCycles;
    return *this;
}

auto Instruction::with_instruction_length(uint16_t numBytes) -> Instruction&
{
    m_numBytes = numBytes;
    return *this;
}

auto Instruction::then(std::function<void()> action) -> Instruction&
{
    m_postOperationActions.push_back(action);
    return *this;
}

auto Instruction::move_program_counter(CPU& cpu) -> void
{
    auto pcRef = cpu.get_program_counter();
    auto pcValue = pcRef->read16();
    pcValue += m_numBytes;
    pcRef->write16(pcValue);
}

auto Instruction::tick_clock(CPU& cpu) -> void
{
    auto cyclesTicked = 0;
    while (cyclesTicked++ <= m_cycles) {
        cpu.tick();
    }
}

}
