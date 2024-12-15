#pragma once

#include "importer.hpp"

class CObjImporter : IModelImporter {
public:
	CVulkanMesh Load();
};