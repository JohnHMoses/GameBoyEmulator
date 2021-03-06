#include "instruction/InstructionInterpreter.h"

#include "Registers.h"
#include "instruction/Instruction.h"
#include "instruction/LoadByteInstruction.h"
#include "instruction/LoadWordInstruction.h"
#include "instruction/PopInstruction.h"
#include "instruction/PushInstruction.h"
#include "memory/ByteAddressable.h"
#include "memory/Memory.h"

namespace GameBoy::InstructionInterpreter {

using namespace std;

auto get_ref_with_signed_offset(Memory& mem, ByteAddressable& offsetRef) -> unique_ptr<WordAddressable>
{
    const auto offsetValue = int8_t(offsetRef.read8());
    const auto addr = 0xFF00 + offsetValue;
    return mem.get_word_ref(addr);
}

auto interpret_next_instruction(Memory& memory) -> unique_ptr<Instruction>
{
    // Interpret the bytes the program counter currently points to as an instruction
    auto programCounterRef = memory.get_word_register(WordRegister::PC);
    const auto nextByteValue = memory.deref(*programCounterRef)->read8();

    // grab some commonly used values so we don't have to redefine them for every instruction
    auto regA = memory.get_register(Register::A);
    auto regB = memory.get_register(Register::B);
    auto regC = memory.get_register(Register::C);
    auto regD = memory.get_register(Register::D);
    auto regE = memory.get_register(Register::E);
    auto regH = memory.get_register(Register::H);
    auto regL = memory.get_register(Register::L);

    auto regAF = memory.get_word_register(WordRegister::AF);
    auto regBC = memory.get_word_register(WordRegister::BC);
    auto regDE = memory.get_word_register(WordRegister::DE);
    auto regHL = memory.get_word_register(WordRegister::HL);

    auto stackPointerRef = memory.get_word_register(WordRegister::SP);

    auto immediateByteRef = memory.deref(*programCounterRef, 1);
    auto immediateWordRef = memory.deref_word(*programCounterRef, 1);

    switch (nextByteValue) {
    case 0x00:
    case 0x01: // LD BC,d16
    {
        auto instr = make_unique<LoadWordInstruction>(
            move(immediateWordRef),
            move(regBC));
        (*instr).with_cycles(12).with_instruction_length(3);
        return instr;
    }
    case 0x02: // LD (BC),A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(memory.deref(*regBC)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06: // LD B,n
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(immediateByteRef));
        (*instr).with_cycles(8).with_instruction_length(2);
        return instr;
    }
    case 0x07:
    case 0x08: // LD (a16),SP
    {
        auto instr = make_unique<LoadWordInstruction>(
            move(stackPointerRef),
            move(memory.deref_word(*immediateWordRef)));
        (*instr).with_cycles(20).with_instruction_length(3);
    }
    case 0x09:
    case 0x0A: // LD A,(BC)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regBC)),
            move(regA));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E: // LD C,n
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regC),
            move(immediateByteRef));
        (*instr).with_cycles(8).with_instruction_length(2);
        return instr;
    }
    case 0x0F:
    case 0x10:
    case 0x11: // LD DE,d16
    {
        auto instr = make_unique<LoadWordInstruction>(
            move(immediateWordRef),
            move(regDE));
        (*instr).with_cycles(12).with_instruction_length(3);
        return instr;
    }
    case 0x12: // LD (DE),A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(memory.deref(*regDE)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16: // LD D,n
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regD),
            move(immediateByteRef));
        (*instr).with_cycles(8).with_instruction_length(2);
        return instr;
    }
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A: // LD A,(DE)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regDE)),
            move(regA));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E: // LD E,n
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regE),
            move(immediateByteRef));
        (*instr).with_cycles(8).with_instruction_length(2);
        return instr;
    }
    case 0x1F:
    case 0x20:
    case 0x21: // LD HL,d16
    {
        auto instr = make_unique<LoadWordInstruction>(
            move(immediateWordRef),
            move(regHL));
        (*instr).with_cycles(12).with_instruction_length(3);
        return instr;
    }
    case 0x22: // LD (HL+),A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(8).with_instruction_length(1).then([&memory]() {
            auto regHL = memory.get_word_register(WordRegister::HL);
            regHL->write16(regHL->read16() + 1);
        });
    }
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26: // LD H,n
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regH),
            move(immediateByteRef));
        (*instr).with_cycles(8).with_instruction_length(2);
        return instr;
    }
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2A: // LD A,(HL+)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regHL)),
            move(regA));
        (*instr).with_cycles(8).with_instruction_length(1).then([&memory]() {
            auto regHL = memory.get_word_register(WordRegister::HL);
            regHL->write16(regHL->read16() + 1);
        });
    }
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2E: // LD L,n
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regL),
            move(immediateByteRef));
        (*instr).with_cycles(8).with_instruction_length(2);
        return instr;
    }
    case 0x2F:
    case 0x30:
    case 0x31: // LD SP,d16
    {
        auto instr = make_unique<LoadWordInstruction>(
            move(immediateWordRef),
            move(stackPointerRef));
        (*instr).with_cycles(12).with_instruction_length(3);
        return instr;
    }
    case 0x32: // LD (HL-),A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(8).with_instruction_length(1).then([&memory]() {
            auto regHL = memory.get_word_register(WordRegister::HL);
            regHL->write16(regHL->read16() - 1);
        });
    }
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36: // LD (HL),n
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(immediateByteRef),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(12).with_instruction_length(2);
        return instr;
    }
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A: // LD A,(HL-)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regHL)),
            move(regA));
        (*instr).with_cycles(8).with_instruction_length(1).then([&memory]() {
            auto regHL = memory.get_word_register(WordRegister::HL);
            regHL->write16(regHL->read16() - 1);
        });
    }
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E: // LD A,d8
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(immediateByteRef),
            move(regA));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x3F:
    case 0x40: // LD B,B
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(regB));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x41: // LD B,C
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regC),
            move(regB));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x42: // LD B,D
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regD),
            move(regB));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x43: // LD B,E
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regE),
            move(regB));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x44: // LD B,H
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regH),
            move(regB));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x45: // LD B,L
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regL),
            move(regB));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x46: // LD B,(HL)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regHL)),
            move(regB));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x47: // LD B,A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(regB));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x48: // LD C,B
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(regC));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x49: // LD C,C
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regC),
            move(regC));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x4A: // LD C,D
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regD),
            move(regC));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x4B: // LD C,E
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regE),
            move(regC));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x4C: // LD C,H
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regH),
            move(regC));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x4D: // LD C,L
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regL),
            move(regC));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x4E: // LD C,(HL)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regHL)),
            move(regC));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x4F: // LD C,A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(regC));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x50: // LD D,B
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(regD));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x51: // LD D,C
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regC),
            move(regD));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x52: // LD D,D
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regD),
            move(regD));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x53: // LD D,E
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regE),
            move(regD));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x54: // LD D,H
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regH),
            move(regD));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x55: // LD D,L
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regL),
            move(regD));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x56: // LD D,(HL)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regHL)),
            move(regD));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x57: // LD D,A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(regD));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x58: // LD E,B
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(regE));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x59: // LD E,C
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regC),
            move(regE));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x5A: // LD E,D
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regD),
            move(regE));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x5B: // LD E,E
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regE),
            move(regE));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x5C: // LD E,H
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regH),
            move(regE));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x5D: // LD E,L
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regL),
            move(regE));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x5E: // LD E,(HL)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regHL)),
            move(regE));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x5F: // LD E,A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(regE));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x60: // LD H,B
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(regH));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x61: // LD H,C
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regC),
            move(regH));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x62: // LD H,D
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regD),
            move(regH));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x63: // LD H,E
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regE),
            move(regH));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x64: // LD H,H
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regH),
            move(regH));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x65: // LD H,L
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regL),
            move(regH));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x66: // LD H,(HL)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regHL)),
            move(regH));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x67: // LD H,A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(regH));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x68: // LD L,B
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(regL));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x69: // LD L,C
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regC),
            move(regL));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x6A: // LD L,D
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regD),
            move(regL));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x6B: // LD L,E
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regE),
            move(regL));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x6C: // LD L,H
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regH),
            move(regL));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x6D: // LD L,L
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regL),
            move(regL));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x6E: // LD L,(HL)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regHL)),
            move(regL));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x6F: // LD L,A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(regL));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x70: // LD (HL),B
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x71: // LD (HL),C
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regC),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x72: // LD (HL),D
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regD),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x73: // LD (HL),E
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regE),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x74: // LD (HL),H
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regH),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x75: // LD (HL),L
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x76:
    case 0x77: // LD (HL),A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(memory.deref(*regHL)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x78: // LD A,B
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regB),
            move(regA));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x79: // LD A,C
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regC),
            move(regA));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x7A: // LD A,D
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regD),
            move(regA));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x7B: // LD A,E
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regE),
            move(regA));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x7C: // LD A,H
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regH),
            move(regA));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x7D: // LD A,L
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regL),
            move(regA));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x7E: // LD A,(HL)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*regHL)),
            move(regA));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0x7F: // LD A,A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(regA));
        (*instr).with_cycles(4).with_instruction_length(1);
        return instr;
    }
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x87:
    case 0x88:
    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0x8D:
    case 0x8E:
    case 0x8F:
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
    case 0x98:
    case 0x99:
    case 0x9A:
    case 0x9B:
    case 0x9C:
    case 0x9D:
    case 0x9E:
    case 0x9F:
    case 0xA0:
    case 0xA1:
    case 0xA2:
    case 0xA3:
    case 0xA4:
    case 0xA5:
    case 0xA6:
    case 0xA7:
    case 0xA8:
    case 0xA9:
    case 0xAA:
    case 0xAB:
    case 0xAC:
    case 0xAD:
    case 0xAE:
    case 0xAF:
    case 0xB0:
    case 0xB1:
    case 0xB2:
    case 0xB3:
    case 0xB4:
    case 0xB5:
    case 0xB6:
    case 0xB7:
    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBE:
    case 0xBF:
    case 0xC0:
    case 0xC1: // POP BC
    {
        auto instr = make_unique<PopInstruction>(
            move(regBC));
        (*instr).with_cycles(12).with_instruction_length(1);
        return instr;
    }
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5: // PUSH BC
    {
        auto instr = make_unique<PushInstruction>(
            move(regBC));
        (*instr).with_cycles(16).with_instruction_length(1);
        return instr;
    }
    case 0xC6:
    case 0xC7:
    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xD0:
    case 0xD1: // POP DE
    {
        auto instr = make_unique<PopInstruction>(
            move(regDE));
        (*instr).with_cycles(12).with_instruction_length(1);
        return instr;
    }
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5: // PUSH DE
    {
        auto instr = make_unique<PushInstruction>(
            move(regDE));
        (*instr).with_cycles(16).with_instruction_length(1);
        return instr;
    }
    case 0xD6:
    case 0xD7:
    case 0xD8:
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
    case 0xE0: // LDH ($FF00+a8),A
    {
        auto derefWith = get_ref_with_signed_offset(memory, *immediateByteRef);
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(memory.deref(*derefWith)));
        (*instr).with_cycles(12).with_instruction_length(2);
        return instr;
    }
    case 0xE1: // POP HL
    {
        auto instr = make_unique<PopInstruction>(
            move(regHL));
        (*instr).with_cycles(12).with_instruction_length(1);
        return instr;
    }
    case 0xE2: // LD ($FF00+C),A
    { // FIXME: Is this the wrong order?
        auto derefWith = get_ref_with_signed_offset(memory, *regC);
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*derefWith)),
            move(regA));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0xE3:
    case 0xE4:
    case 0xE5: // PUSH HL
    {
        auto instr = make_unique<PushInstruction>(
            move(regHL));
        (*instr).with_cycles(16).with_instruction_length(1);
        return instr;
    }
    case 0xE6:
    case 0xE7:
    case 0xE8:
    case 0xE9:
    case 0xEA: // LD (a16),A
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(memory.deref(*immediateWordRef)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
    case 0xEF:
    case 0xF0: // LDH A,($FF00+a8)
    {
        auto derefWith = get_ref_with_signed_offset(memory, *immediateByteRef);
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*derefWith)),
            move(regA));
        (*instr).with_cycles(12).with_instruction_length(2);
        return instr;
    }
    case 0xF1: // POP AF
    {
        auto instr = make_unique<PopInstruction>(
            move(regAF));
        (*instr).with_cycles(12).with_instruction_length(1);
        return instr;
    }
    case 0xF2: // LD A,($FF00+C)
    {

        auto derefWith = get_ref_with_signed_offset(memory, *regC);
        auto instr = make_unique<LoadByteInstruction>(
            move(regA),
            move(memory.deref(*derefWith)));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0xF3:
    case 0xF4:
    case 0xF5: // PUSH AF
    {
        auto instr = make_unique<PushInstruction>(
            move(regAF));
        (*instr).with_cycles(16).with_instruction_length(1);
        return instr;
    }
    case 0xF6:
    case 0xF7:
    case 0xF8: // LS HL,SP+r8
    { // TODO: Flags?
        uint16_t effectiveAddress = stackPointerRef->read16() + int8_t(immediateByteRef->read8());
        auto instr = make_unique<LoadWordInstruction>(
            move(memory.get_word_ref(effectiveAddress)),
            move(regHL));
        (*instr).with_cycles(12).with_instruction_length(2);
        return instr;
    }
    case 0xF9: // LD SP,HL
    {
        auto instr = make_unique<LoadWordInstruction>(
            move(regHL),
            move(stackPointerRef));
        (*instr).with_cycles(8).with_instruction_length(1);
        return instr;
    }
    case 0xFA: // LD A,(nn)
    {
        auto instr = make_unique<LoadByteInstruction>(
            move(memory.deref(*immediateWordRef)),
            move(regA));
        (*instr).with_cycles(16).with_instruction_length(3);
        return instr;
    }
    case 0xFB:
    case 0xFC:
    case 0xFD:
    case 0xFE:
    case 0xFF:
        abort();
    }

    abort();
    return nullptr;
}

}
