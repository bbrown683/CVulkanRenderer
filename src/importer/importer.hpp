#pragma once

struct CVulkanMesh;

struct IModelImporter {
	virtual CVulkanMesh Load() = 0;
};