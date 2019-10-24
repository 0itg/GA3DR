#pragma once
#include "Mesh.h"

// 3d model consisting of one or more meshes (groups)
// loaded from OBJ file. All rendering calculations are
// done at the mesh level, but this class keeps them
// organized as physical objects.
class Model {
public:
	std::string filePath;
	std::vector<Mesh*> groups;

	void load(std::string objPath);
	void motion(c3ga::normalizedPoint location =
		c3ga::_normalizedPoint(c3ga::no), c3ga::rotor R =
		c3ga::_rotor(1), float scale = 1) {
		for (Mesh* mesh : groups) mesh->motion(location, R, scale);
	}
	void recalcPlanes(bool smooth = false) {
		for (Mesh* mesh : groups) mesh->recalcPlanes(smooth);
	}
	// Splits large groups into eight smaller ones
	// to improve performance with multithreading
	void splitGroups(int faceCountThreshold = 10000);
	Model(std::string filePath) : filePath(filePath) {
		load(filePath);
	}
	~Model() {
		for (Mesh* mesh : groups) delete mesh;
	}
};