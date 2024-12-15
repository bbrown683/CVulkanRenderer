#pragma once

#include "importer.hpp"

class CFbxImporter : IModelImporter {
public:
	CVulkanMesh Load();
};