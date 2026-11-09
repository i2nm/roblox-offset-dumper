#include "../globals.h"

bool isValidViewMatrix(const ViewMatrix_t& mat)
{
	if (std::abs(mat.arrData[2][3] - 0.1f) > 0.01f)
		return false;

	if (std::abs(mat.arrData[3][2] + 1.0f) < 0.01f &&
		std::abs(mat.arrData[3][3]) < 0.01f)
	{
		return false;
	}

	if (std::abs(mat.arrData[3][3]) < 10.0f ||
		std::abs(mat.arrData[3][3]) > 10000.0f)
	{
		return false;
	}

	for (int r = 0; r < 4; r++)
	{
		for (int c = 0; c < 4; c++)
		{
			const float value = mat.arrData[r][c];

			if (std::isnan(value) || std::isinf(value))
				return false;
		}
	}

	return true;
};

void Dump::StaticDump()
{
	Logger::Log(Logger::Level::Info, "Scanning for TaskScheduler::Pointer...");
	uintptr_t taskSchedulerPointer = 0;


	auto stringMatches = Helper::ScanStringRuntime("Default job arbiter must always be valid");
	for (uintptr_t strAddr : stringMatches)
	{
		auto xrefs = Helper::ScanXrefsRuntime(strAddr);
		for (uintptr_t xref : xrefs)
		{
			auto taskInstructions = Helper::DisassembleRuntime(xref, 200, 50);

			for (const auto& instruction : taskInstructions)
			{
				if (instruction.info.mnemonic == ZYDIS_MNEMONIC_MOV) {
					uintptr_t disp = Dissasembler::Decoder::GetRipDisplacement(instruction);
					if (disp == 0) continue;

					uintptr_t addr = instruction.address + instruction.info.length + disp;
					uintptr_t possibleSched = Mem::Read<uintptr_t>(Globals::baseAddress + addr);
					if (!Mem::IsValid(possibleSched)) continue;

					taskSchedulerPointer = addr;
					break;
				}
			}

			if (taskSchedulerPointer != 0) break;
		}

		if (taskSchedulerPointer != 0) break;
	}

	if (!taskSchedulerPointer) {
		Logger::Log(Logger::Level::Error, "!!!!! TaskScheduler::Pointer failed to dump !!!!!");
		system("pause");
	}

	Logger::Log(Logger::Level::Info, "Scanning for VisualEngine::Pointer && FakeDataModel::Pointer...");

	const std::vector<std::string> rttiSymbols = {
		"RBX::Graphics::VisualEngine",
		"RBX::DataModel"
	};

	const auto results = rtti::scan(rttiSymbols);

	const auto visualEngineIt = results.find("RBX::Graphics::VisualEngine");
	const auto dataModelIt = results.find("RBX::DataModel");

	const uintptr_t visualEnginePointer = visualEngineIt != results.end() ? visualEngineIt->second : 0;
	const uintptr_t fakeDataModelPointer = dataModelIt != results.end() ? dataModelIt->second : 0;

	if (!visualEnginePointer) {
		Logger::Log(Logger::Level::Error, "!!!!! VisualEngine::Pointer failed to dump !!!!!");
		system("pause");
	}

	if (!fakeDataModelPointer) {
		Logger::Log(Logger::Level::Error, "!!!!! FakeDataModel::Pointer failed to dump !!!!!");
		system("pause");
	}

	Logger::Log(Logger::Level::Success, "Finished main pointer dump!");

	Globals::offsets.add("Pointer", "TaskScheduler", taskSchedulerPointer, "unsigned __int64");
	Globals::offsets.add("Pointer", "VisualEngine", visualEnginePointer, "unsigned __int64");
	Globals::offsets.add("Pointer", "FakeDataModel", fakeDataModelPointer, "unsigned __int64");

	uintptr_t taskScheduler = Mem::Read<uintptr_t>(Globals::baseAddress + taskSchedulerPointer);

	uintptr_t jobStartOff = 0;
	uintptr_t jobEndOff = 0;
	uintptr_t jobNameOff = 0;
	for (int i = 0x50; i < 0x250; i++)
	{
		uintptr_t jobStart = Mem::Read<uintptr_t>(taskScheduler + i);
		if (!Mem::IsValid(jobStart)) continue;

		for (int j = 0x50; j < 0x250; j++)
		{
			uintptr_t jobEnd = Mem::Read<uintptr_t>(taskScheduler + j);
			if (!Mem::IsValid(jobEnd)) continue;
			if (jobEnd - jobStart >= 0x1000) continue;

			for (uintptr_t jobPtr = jobStart; jobPtr < jobEnd; jobPtr += 0x10) {
				uintptr_t job = Mem::Read<uintptr_t>(jobPtr);
				if (!Mem::IsValid(job)) continue;

				for (int r = 0x10; r < 0x30; r++)
				{
					string jobName = Mem::ReadString(job + r);
					if (jobName == "RenderJob") {
						jobStartOff = i;
						jobEndOff = j;
						jobNameOff = r;
						goto foundjob;
					}
				}
			}
		}
	}

foundjob: {
	Globals::offsets.add("JobStart", "TaskScheduler", jobStartOff, "unsigned __int64");
	Globals::offsets.add("JobEnd", "TaskScheduler", jobEndOff, "unsigned __int64");
	Globals::offsets.add("JobName", "TaskScheduler", jobNameOff, "string");
}

	uintptr_t fakeDataModel = Mem::Read<uintptr_t>(Globals::baseAddress + fakeDataModelPointer);
	RbxDumper::Offset realDataModelOffset = Helper::DoRTTIScan(RTTIScanConfig(
		"RealDataModel",
		"FakeDataModel",
		{ fakeDataModel },
		{ "RBX::DataModel" },
		"unsigned __int64"
	));

	Globals::dataModel = Mem::Read<uintptr_t>(fakeDataModel + realDataModelOffset.value);
	Logger::Log(Logger::Level::Success, "DataModel found at: " + to_string(Globals::dataModel));

	uintptr_t visualEngine = Mem::Read<uintptr_t>(Globals::baseAddress + visualEnginePointer);
	Helper::DoBasicScan(BasicScanConfig<Vector2>(
		"Dimensions",
		"VisualEngine",
		{ visualEngine },
		{ Globals::windowResolution }
	));

	for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x10) {
		const ViewMatrix_t candidateMatrix = Mem::Read<ViewMatrix_t>(visualEngine + offset);

		if (isValidViewMatrix(candidateMatrix))
		{
			Globals::offsets.add("ViewMatrix", "VisualEngine", offset, "ViewMatrix_t");
			break;
		}
	}

	// compatibility, dont dump these anymore
	Globals::offsets.add("LightingValid", "RenderView", 0, "bool");
	Globals::offsets.add("SkyValid", "RenderView", 0, "bool");
	Globals::offsets.add("VisualEngine", "RenderView", 0, "unsigned __int64");
	Globals::offsets.add("DeviceD3D11", "RenderView", 0, "unsigned __int64");
	Globals::offsets.add("RequireBypass", "ScriptContext", 0);
	Globals::offsets.add("IsCoreScript", "ModuleScript", 0);
	Globals::offsets.add("SensitivityPointer", "MouseService", 0, "float");
	Globals::offsets.add("Pointer", "PlayerConfigurer", 0, "unsigned __int64");

	Helper::DoRTTIScan(RTTIScanConfig(
		"RenderView",
		"VisualEngine",
		{ visualEngine },
		{ "RBX::Graphics::RenderView" }
	));
}
