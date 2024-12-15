#pragma once

#include "importer.hpp"

class CGltfImporter : IModelImporter {
public:
	CVulkanMesh Load();
};