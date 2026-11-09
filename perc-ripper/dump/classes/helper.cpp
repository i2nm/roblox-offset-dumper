#include "../globals.h"

namespace Helper {
    RbxDumper::Offset DoRTTIScan(RTTIScanConfig config)
    {
        for (uintptr_t i = config.start; i < config.end; i += config.step) {
            bool allMatch = true;

            for (size_t j = 0; j < config.targetInstances.size(); j++) {
                uintptr_t addressToCheck = config.targetInstances[j] + i;

                uintptr_t val = Mem::Read<uintptr_t>(addressToCheck);
				string valueAtAddress = rtti::get(val);

                if (valueAtAddress != config.expectedValues[j]) {
                    allMatch = false;
                    break;
                }
            }

            if (allMatch) {
                RbxDumper::Offset offset(config.name, config.catagory, i, config.type);
                Globals::offsets.add(offset);
                return offset;
            }
        }

        Logger::Log(Logger::Level::Warning, "No offset found for " + config.name + "::" + config.catagory);

        RbxDumper::Offset offset(config.name, config.catagory, 0, config.type);
        Globals::offsets.add(offset);
        return offset;
    }


    std::vector<uintptr_t> ScanStringRuntime(const std::string& target, uintptr_t maxScanSize)
    {
        std::vector<uintptr_t> matches;
        if (target.empty() || !Globals::baseAddress)
            return matches;

        HANDLE handle = Mem::GetHandle();
        if (!handle)
            return matches;

        MEMORY_BASIC_INFORMATION mbi{};
        uintptr_t current = Globals::baseAddress;
        const uintptr_t scanEnd = Globals::baseAddress + maxScanSize;

        while (current < scanEnd && VirtualQueryEx(handle, reinterpret_cast<LPCVOID>(current), &mbi, sizeof(mbi)) == sizeof(mbi)) {
            const uintptr_t regionStart = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
            const uintptr_t regionEnd = min(regionStart + mbi.RegionSize, scanEnd);

            if (mbi.State == MEM_COMMIT && !(mbi.Protect & PAGE_GUARD) && !(mbi.Protect & PAGE_NOACCESS)) {
                auto buffer = Mem::ReadBytesSafe(regionStart, regionEnd - regionStart);

                if (buffer.size() >= target.size()) {
                    for (size_t offset = 0; offset <= buffer.size() - target.size(); ++offset) {
                        if (std::memcmp(buffer.data() + offset, target.data(), target.size()) == 0) {
                            matches.push_back(regionStart + offset);
                        }
                    }
                }
            }

            if (regionEnd <= current) break;
            current = regionEnd;
        }

        return matches;
    }

    std::vector<uintptr_t> ScanXrefsRuntime(uintptr_t targetAddress, uintptr_t maxScanSize)
    {
        std::vector<uintptr_t> xrefs;
        if (!targetAddress || !Globals::baseAddress)
            return xrefs;

        HANDLE handle = Mem::GetHandle();
        if (!handle)
            return xrefs;

        ZydisDecoder decoderEngine;
        ZydisDecoderInit(&decoderEngine, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

        MEMORY_BASIC_INFORMATION mbi{};
        uintptr_t current = Globals::baseAddress;
        const uintptr_t scanEnd = Globals::baseAddress + maxScanSize;

        while (current < scanEnd && VirtualQueryEx(handle, reinterpret_cast<LPCVOID>(current), &mbi, sizeof(mbi)) == sizeof(mbi)) {
            const uintptr_t regionStart = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
            const uintptr_t regionEnd = min(regionStart + mbi.RegionSize, scanEnd);

            if (mbi.State == MEM_COMMIT && !(mbi.Protect & PAGE_GUARD) && !(mbi.Protect & PAGE_NOACCESS) &&
                (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) {

                auto buffer = Mem::ReadBytesSafe(regionStart, regionEnd - regionStart);

                if (!buffer.empty()) {
                    uintptr_t offset = 0;
                    while (offset < buffer.size()) {
                        ZydisDecodedInstruction instruction;
                        ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

                        ZyanStatus status = ZydisDecoderDecodeFull(
                            &decoderEngine,
                            buffer.data() + offset,
                            buffer.size() - offset,
                            &instruction,
                            operands
                        );

                        if (!ZYAN_SUCCESS(status) || instruction.length == 0 || instruction.length > (buffer.size() - offset)) {
                            offset++;
                            continue;
                        }

                        const uintptr_t instruction_addr = regionStart + offset;

                        for (int i = 0; i < instruction.operand_count_visible; i++) {
                            const auto& operand = operands[i];
                            if (operand.type == ZYDIS_OPERAND_TYPE_MEMORY &&
                                operand.mem.base == ZYDIS_REGISTER_RIP &&
                                operand.mem.disp.has_displacement) {

                                const uintptr_t rip_target = instruction_addr + instruction.length + operand.mem.disp.value;
                                if (rip_target == targetAddress) {
                                    xrefs.push_back(instruction_addr);
                                }
                            }
                        }

                        offset += instruction.length;
                    }
                }
            }

            if (regionEnd <= current) break;
            current = regionEnd;
        }

        return xrefs;
    }

    std::vector<Dissasembler::Instruction> DisassembleRuntime(uintptr_t address, size_t size, size_t bytesAbove)
    {
        const uintptr_t startAddress = (address > bytesAbove) ? address - bytesAbove : address;
        const size_t totalSize = size + (address - startAddress);

        auto buffer = Mem::ReadBytesSafe(startAddress, totalSize);
        if (buffer.empty())
            return {};

        Dissasembler::Decoder dec(buffer);
        return dec.Disassemble(0, buffer.size(), startAddress - Globals::baseAddress);
    }

    std::vector<Dissasembler::Instruction> FindStringRuntime(const std::string& targetString, size_t bytesAbove, size_t bytesTotal)
    {
        auto matches = ScanStringRuntime(targetString);
        if (matches.empty()) {
            return {};
        }

        for (uintptr_t strAddr : matches) {
            auto xrefs = ScanXrefsRuntime(strAddr);
            for (uintptr_t xref : xrefs) {
                auto insts = DisassembleRuntime(xref, bytesTotal, bytesAbove);
                if (!insts.empty()) {
                    return insts;
                }
            }
        }

        return {};
    }
}
