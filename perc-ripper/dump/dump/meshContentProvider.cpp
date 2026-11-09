#include "../globals.h"

// just deal with my different code style in this file
// - kitodoescode

void Dump::MeshContentProvider() {
	const auto mcp = Roblox::FindFirstChild(Globals::dataModel, "MeshContentProvider");

	if (!mcp) {
		Logger::Log(Logger::Level::Warning, "Failed to find MeshContentProvider");
		return;
	}

    uintptr_t lrucache; // MemEnforcedLRUCache pointer, not offset
    bool found = false;
    for (auto i = 0; i < 0x300; i += 0x8) {
        auto lruholder = Mem::Read<uintptr_t>(mcp + i);
        if (!Mem::IsValid(lruholder)) continue;
        for (auto j = 0; j < 0x100; j += 0x8) {
            auto candidate = Mem::Read<uintptr_t>(lruholder + j);
            if (!Mem::IsValid(candidate)) continue;
            auto rtti = rtti::get(candidate);
            if (!rtti.empty() && rtti.find("MemEnforcedLRUCache") != std::string::npos) {
                Globals::offsets.add("LRUHolder", "MeshContentProvider", i);
                Globals::offsets.add("MemEnforcedLRUCache", "LRUHolder", j);
                lrucache = candidate;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (!Globals::offsets.get("LRUHolder", "MeshContentProvider").value ||
        !Globals::offsets.get("MemEnforcedLRUCache", "LRUHolder").value) {
        Logger::Log(Logger::Level::Warning,
            "Failed to get MeshContentProvider::LRUHolder or LRUHolder::MemEnforcedLRUCache");
        return;
    }

    for (auto i = 0x8; i < 0x100; i += 0x8) {
        auto ptr = Mem::Read<uintptr_t>(lrucache + i);
        if (!Mem::IsValid(ptr)) continue;
        auto next = Mem::Read<uintptr_t>(ptr);
        auto prev = Mem::Read<uintptr_t>(ptr + 0x8);
        if (Mem::IsValid(next) && Mem::IsValid(prev)) {
            Globals::offsets.add("Head", "MemEnforcedLRUCache", i);
            break;
        }
    }

    auto head = Globals::offsets.get("Head", "MemEnforcedLRUCache").value; // offset
    if (!head) {
        Logger::Log(Logger::Level::Warning, "Failed to get MemEnforcedLRUCache::Head");
        return;
    }

    auto sentinel = Mem::Read<uintptr_t>(lrucache + head);
    auto first_node = Mem::Read<uintptr_t>(sentinel);
    if (!Mem::IsValid(sentinel) || !Mem::IsValid(first_node) || sentinel == first_node) {
        Logger::Log(Logger::Level::Warning, "MemEnforcedLRUCache is empty");
        return;
    }

    for (auto i = 0; i < 0x100; i += 0x8) {
        auto str = Mem::ReadString(first_node + i);
        if (!str.empty() && (str.find("rbxasset") != std::string::npos || str.find("http") != std::string::npos)) {
            Globals::offsets.add("Next", "LRUNode", 0x0);
            Globals::offsets.add("AssetID", "LRUNode", i);
            break;
        }
    }

    auto assetid = Globals::offsets.get("AssetID", "LRUNode").value;
    if (!assetid) {
        Logger::Log(Logger::Level::Warning, "Failed to get LRUNode::AssetID");
        return;
    }

    auto str = std::string("rightleg");
    auto node = first_node;
    uintptr_t rightleg_node = 0;
    for (auto i = 0; i < 0x2000 && node != sentinel; i++) {
        auto id = Mem::ReadString(node + assetid);
        if (!id.empty() && id.find(str) != std::string::npos) {
            rightleg_node = node;
            break;
        }
        auto next = Mem::Read<uintptr_t>(node);
        if (!Mem::IsValid(next) || next == node) break;
        node = next;
    }

    if (!Mem::IsValid(rightleg_node)) {
        Logger::Log(Logger::Level::Warning, "Failed to get rightleg in LRUCache");
        return;
    }

    found = false;
    const Vector3 expected_min(-0.5f, -1.0f, -0.5f);
    const float expected[3] = { expected_min.x, expected_min.y, expected_min.z };
    uintptr_t fmd_ptr = 0; // pointer to filemeshdata of rightleg node
    for (auto i = 0; i < 0x100; i += 0x8) {
        if (!i || i == assetid) continue;
        auto cached_item = Mem::Read<uintptr_t>(rightleg_node + i);
        if (!Mem::IsValid(cached_item)) continue;
        auto vtable = Mem::Read<uintptr_t>(cached_item);
        if (!Mem::IsValid(vtable)) continue;
        for (auto j = 0x8; j < 0x100; j += 0x8) {
            auto filemeshdata = Mem::Read<uintptr_t>(cached_item + j);
            if (!Mem::IsValid(filemeshdata)) continue;
            for (auto k = 0; k < 0x300; k += 0x4) {
                auto all_match = true;
                for (auto l = 0; l < 3; l++) {
                    auto val = Mem::Read<float>(filemeshdata + k + (l*4));
                    if (std::isnan(val) || std::isinf(val) || std::abs(val - expected[l]) >= 0.01f) {
                        all_match = false;
                        break;
                    }
                }
                if (all_match) {
                    fmd_ptr = filemeshdata;
                    Globals::offsets.add("CachedItem", "LRUNode", i);
                    Globals::offsets.add("FileMeshData", "CachedItem", j);
                    Globals::offsets.add("AABBMin", "FileMeshData", k);
                    Globals::offsets.add("AABBMax", "FileMeshData", k+12);
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        if (found) break;
    }

    if (!fmd_ptr ||
        !Globals::offsets.get("CachedItem", "LRUNode").value ||
        !Globals::offsets.get("FileMeshData", "CachedItem").value ||
        !Globals::offsets.get("AABBMin", "FileMeshData").value ||
        !Globals::offsets.get("AABBMax", "FileMeshData").value) {
        Logger::Log(Logger::Level::Warning,
            "Failed to get LRUNode::CachedItem or CachedItem::FileMeshData or FileMeshData::AABBMin or FileMeshData::AABBMax");
        return;
    }

    const size_t vert_bytes = 0x690;
    const size_t face_bytes = 0x210;

    size_t verts = SIZE_MAX;
    for (auto i = 0; i < 0x100; i += 0x8) {
        auto start = Mem::Read<uintptr_t>(fmd_ptr + i);
        auto end = Mem::Read<uintptr_t>(fmd_ptr + i + 0x8);
        if (start && end &&
            Mem::IsValid(start) && Mem::IsValid(end) &&
            (end - start) == vert_bytes) {
            verts = i;
            break;
        }
    }

    if (verts == SIZE_MAX) {
        Logger::Log(Logger::Level::Warning, "Failed to get FileMeshData::Vertices");
        return;
    }

    size_t faces = SIZE_MAX;
    for (auto i = verts + 0x10; i < 0x100; i += 0x8) {
        auto start = Mem::Read<uintptr_t>(fmd_ptr + i);
        auto end = Mem::Read<uintptr_t>(fmd_ptr + i + 0x8);
        if (start && end &&
            Mem::IsValid(start) && Mem::IsValid(end) &&
            (end - start) == face_bytes) {
            faces = i;
            break;
        }
    }

    if (faces == SIZE_MAX) {
        Logger::Log(Logger::Level::Warning, "Failed to get FileMeshData::Faces");
        return;
    }

    Globals::offsets.add("Vertices", "FileMeshData", verts);
    Globals::offsets.add("VerticesEnd", "FileMeshData", verts + 0x8);
    Globals::offsets.add("Faces", "FileMeshData", faces);
    Globals::offsets.add("FacesEnd", "FileMeshData", faces + 0x8);
}
