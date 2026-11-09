#include "../globals.h"

inline static const uint8_t expectedBytecode[] = { 0x22, 0xEE, 0x32, 0xE2, 0x41, 0x61, 0x14, 0x77, 0x90, 0xB0, 0x97, 0xE6, 0x7C, 0xFC, 0x11, 0xBD, 0x00, 0x4F, 0x45, 0x73, 0xBD, 0x71, 0xF1, 0xEE, 0xDB, 0xB4, 0x55, 0xD9, 0x7A, 0x2C, 0x81, 0x0A, 0x03, 0x15, 0xC3, 0x9C, 0x97, 0x15, 0xA0, 0xCF, 0x18, 0xD0, 0x3C, 0xD4, 0x51, 0x9A, 0xD7, 0x20, 0x9B, 0xBC, 0xDB, 0x62, 0x3E, 0x92, 0xE6, 0xF9, 0xCD, 0x6B, 0xBB, 0x9C, 0xBA, 0xA7, 0x77, 0x8F, 0xB7, 0xF5, 0xBF, 0xEE, 0x1F, 0x9B, 0xE6, 0xD3, 0x9B, 0x94, 0x39, 0x69, 0x85, 0x6C, 0x1C, 0x63, 0xE6, 0xB0, 0xAA, 0x13, 0xEC, 0x30, 0xE4, 0xCF, 0x88, 0x77 };

void Dump::Scripts() {
	uintptr_t workspace = Roblox::FindFirstChild(Globals::dataModel, "Workspace");
	uintptr_t localScript = Roblox::FindFirstChild(workspace, "LocalScript");

	uintptr_t bytecodeSize = 0x28;
	uintptr_t bytecodePointer = 0x18;

	string lsGuid = "{704E7537-4EFC-4E9E-BB5A-60CDCA238EAD}";
	string lsHash = "d41d8cd98f00b204e9800998ecf8427e";

	Helper::DoBasicScan(BasicScanConfig<string>(
		"GUID",
		"LocalScript",
		{ localScript },
		{ lsGuid }
	));

	Helper::DoBasicScan(BasicScanConfig<string>(
		"Hash",
		"LocalScript",
		{ localScript },
		{ lsHash }
	));

	uintptr_t localScriptBytecodeOffset = 0;
	for (int i = 0x100; i < 0x1000; i++) {
		try {
			uintptr_t embedded = Mem::Read<uintptr_t>(localScript + i);
			uintptr_t size = Mem::Read<uintptr_t>(embedded + bytecodeSize);

			if (size == sizeof(expectedBytecode))
			{
				localScriptBytecodeOffset = i;
				break;
			}
		}
		catch (const exception&) {}
	}

	Globals::offsets.add("ByteCode", "LocalScript", localScriptBytecodeOffset, "unsigned __int64");

	Globals::offsets.add("Size", "ByteCode", bytecodeSize, "unsigned __int64");
	Globals::offsets.add("Pointer", "ByteCode", bytecodePointer, "unsigned __int64");

	uintptr_t moduleScript = Roblox::FindFirstChild(workspace, "ModuleScript");
	string msGuid = "{4330BBB6-38D5-43C8-B302-C2DC97068AF6}";

	Helper::DoBasicScan(BasicScanConfig<string>(
		"GUID",
		"ModuleScript",
		{ moduleScript },
		{ msGuid }
	));

	Helper::DoBasicScan(BasicScanConfig<string>(
		"Hash",
		"ModuleScript",
		{ moduleScript },
		{ lsHash }
	));

	uintptr_t moduleScriptBytecodeOffset = 0;
	for (int i = 0x100; i < 0x1000; i++) {
		try {
			uintptr_t embedded = Mem::Read<uintptr_t>(moduleScript + i);
			uintptr_t size = Mem::Read<uintptr_t>(embedded + bytecodeSize);

			if (size == sizeof(expectedBytecode))
			{
				moduleScriptBytecodeOffset = i;
				break;
			}
		}
		catch (const exception&) {}
	}
	Globals::offsets.add("ByteCode", "ModuleScript", moduleScriptBytecodeOffset, "unsigned __int64");

	uintptr_t script = Roblox::FindFirstChild(workspace, "Script");
	string scriptGuid = "{7C692C7B-5ABE-400F-AA26-035C6BC2A36A}";

	Helper::DoBasicScan(BasicScanConfig<string>(
		"GUID",
		"Script",
		{ script },
		{ scriptGuid }
	));

	Helper::DoBasicScan(BasicScanConfig<string>(
		"Hash",
		"Script",
		{ script },
		{ lsHash }
	));

	uintptr_t scriptBytecodeOffset = 0;
	for (int i = 0x100; i < 0x1000; i++) {
		try {
			uintptr_t embedded = Mem::Read<uintptr_t>(script + i);
			uintptr_t size = Mem::Read<uintptr_t>(embedded + bytecodeSize);

			if (size == sizeof(expectedBytecode))
			{
				scriptBytecodeOffset = i;
				break;
			}
		}
		catch (const exception&) {}
	}
	Globals::offsets.add("ByteCode", "Script", scriptBytecodeOffset, "unsigned __int64");
}
