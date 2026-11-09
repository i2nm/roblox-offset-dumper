#include "../globals.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <DbgHelp.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#pragma comment(lib, "Dbghelp.lib")

struct RttiCompleteObjectLocator {
    DWORD signature;
    DWORD offset;
    DWORD cdOffset;
    DWORD typeDescriptor;
    DWORD classDescriptor;
    DWORD baseOffset;
};

struct TypeDescriptor {
    void* vtable;
    uint64_t spare;
    char name[255];
};

struct ModuleInfo {
    uintptr_t baseAddress;
    uintptr_t endAddress;
};

static std::vector<ModuleInfo> storedModules;

static bool isAllZeros(const std::string& str) {
    return ranges::all_of(str,[](const char c) { return c == '\0'; });
}

static std::string demangleSymbol(const std::string& mangledName) {
    try {
        if (mangledName.empty() || mangledName.size() > 100) {
            return mangledName;
        }

        std::string name = mangledName;

        if (name.rfind(".?AV", 0) == 0) {
            name = "?" + name.substr(4);
        }

        char buffer[1024]{};

        const DWORD length = UnDecorateSymbolName(
            name.c_str(),
            buffer,
            static_cast<DWORD>(sizeof(buffer)),
            UNDNAME_COMPLETE
        );

        if (!length || length >= sizeof(buffer)) {
            return name;
        }

        std::string result(buffer, length);

        if (result.rfind(" ??", 0) == 0) {
            result.erase(0, 4);
        }

        return result.empty() ? name : result;
    }
    catch (...) {
        return mangledName;
    }
}

static std::vector<ModuleInfo> getModules(DWORD processId) {
    std::vector<ModuleInfo> modules;

    HANDLE snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
        processId
    );

    if (snapshot == INVALID_HANDLE_VALUE) {
        return modules;
    }

    MODULEENTRY32 moduleEntry{};
    moduleEntry.dwSize = sizeof(moduleEntry);

    if (Module32First(snapshot, &moduleEntry)) {
        do {
            const uintptr_t baseAddress =
                reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);

            const uintptr_t moduleSize =
                static_cast<uintptr_t>(moduleEntry.modBaseSize);

            if (!baseAddress ||
                !moduleSize ||
                baseAddress > UINTPTR_MAX - moduleSize) {
                continue;
            }

            modules.push_back({
                baseAddress,
                baseAddress + moduleSize
            });
        } while (Module32Next(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);
    return modules;
}

static uintptr_t getModuleContaining(
    DWORD processId,
    uintptr_t address
) {
    if (!address) {
        return 0;
    }

    if (storedModules.empty()) {
        storedModules = getModules(processId);
    }

    for (const auto& module : storedModules) {
        if (address >= module.baseAddress &&
            address < module.endAddress) {
            return module.baseAddress;
        }
    }

    return 0;
}

std::string rtti::get(uintptr_t objectAddress) {
    if (!objectAddress || !Mem::IsValid(objectAddress)) {
        return "???";
    }

    const uintptr_t vtableAddress =
        Mem::Read<uintptr_t>(objectAddress);

    if (!vtableAddress || !Mem::IsValid(vtableAddress)) {
        return "???";
    }

    const uintptr_t colAddress =
        Mem::Read<uintptr_t>(
            vtableAddress - sizeof(uintptr_t)
        );

    if (!colAddress || !Mem::IsValid(colAddress)) {
        return "???";
    }

    const RttiCompleteObjectLocator col =
        Mem::Read<RttiCompleteObjectLocator>(colAddress);

    uintptr_t typeInfoAddress =
        getModuleContaining(
            Mem::TargetProcessId,
            colAddress
        ) + col.typeDescriptor;

    if (!typeInfoAddress ||
        !Mem::IsValid(typeInfoAddress)) {
        return "???";
        }

    const TypeDescriptor typeInfo =
        Mem::Read<TypeDescriptor>(typeInfoAddress);

    if (isAllZeros(typeInfo.name)) {
        return "???";
    }

    return demangleSymbol(typeInfo.name);
}

std::unordered_map<std::string, uintptr_t> rtti::scan(
    const std::vector<std::string>& symbols
) {
    std::unordered_map<std::string, uintptr_t> results;

    if (symbols.empty() || !Globals::baseAddress) {
        return results;
    }

    for (const auto& symbol : symbols) {
        results.emplace(symbol, 0);
    }

    size_t remaining = symbols.size();

    for (uintptr_t offset = 0x8000000;
         offset < 0x9000000 && remaining > 0;
         offset += sizeof(uintptr_t)) {

        const uintptr_t address =
            Globals::baseAddress + offset;

        if (!Mem::IsValid(address)) {
            continue;
        }

        const uintptr_t objectAddress =
            Mem::Read<uintptr_t>(address);

        if (!Mem::IsValid(objectAddress)) {
            continue;
        }

        std::string rttiName;

        try {
            rttiName = get(objectAddress);
        }
        catch (...) {
            continue;
        }

        if (rttiName == "???") {
            continue;
        }

        auto result = results.find(rttiName);

        if (result != results.end() &&
            result->second == 0) {

            result->second = offset;
            --remaining;
            }
         }

    return results;
}
