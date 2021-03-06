#pragma once

#include "instruction/Instruction.h"
#include "memory/WordAddressable.h"

#include <memory>

namespace GameBoy {

class PushInstruction : public Instruction {
public:
    PushInstruction(
        std::unique_ptr<WordAddressable>&& fromRef);

    ~PushInstruction() override;

private:
    auto perform_operation(CPU&) -> void override;

    std::unique_ptr<WordAddressable> m_fromRef;
};

}
