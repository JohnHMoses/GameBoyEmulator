#pragma once

#include <memory>

namespace GameBoy {

class Memory;
class WordAddressable;

// Executes instructions
class CPU {
public:
    CPU(Memory&);

    auto tick() -> void;
    auto get_program_counter() -> std::unique_ptr<WordAddressable>;
    auto get_stack_pointer() -> std::unique_ptr<WordAddressable>;

private:
    Memory& m_memory;
};

}
