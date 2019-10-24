#include "Model.h"
#include "Mesh.h"
#include "Material.h"

#include "c3ga.h"

#include <fstream>

using namespace c3ga;

//Load mesh from OBJ file and place/orient in space
void Model::load(std::string inputPath) {
	std::ifstream file;
	filePath = inputPath;
	std::string fileDir = "";
	std::string fileName;
	file.open(inputPath);
	std::string currentMtl = "default";
	Mesh* currentMesh = new Mesh;
	Material* currentMtlPtr = currentMesh->mtlLib.mtl["default"];
	std::string command;
	float x, y, z;
	std::string input;
	int v1, v2, v3, t1, t2, t3, n1 = -1, n2 = -1, n3 = -1;
	size_t s, t;
	bool texturedModel = true;
	bool firstGroup = true;

	splitFileNameAndDirectory(inputPath, fileDir, fileName);

	while (file >> command) {
		if (command == "g") {
			if (!firstGroup) {
				currentMesh->updateBoundingSphere();
				if (currentMesh->planes.size() == 0) currentMesh->recalcPlanes();
				groups.push_back(currentMesh);
				currentMesh = new Mesh;
			}
			else firstGroup = false;
			file >> currentMesh->groupName;
		}
		// lines starting with v have vertices, stored as 3 floats
		if (command == "v") {
			file >> x >> y >> z;
			currentMesh->addVertex(x, y, z);
		}
		// texture vertices, stored as 2 floats
		else if (command == "vt") {
			file >> x >> y;
			currentMesh->addTexCoord(x, y);
		}
		else if (command == "vn") {
			file >> x >> y >> z;
			currentMesh->addPlane(_plane(dual(dualPlane(dualPlane_e1_e2_e3_ni, x, y, z, 0))));
		}
		// faces may have texture coordinates and normal vectors.
		// If not, 3 ints giving indices for vertex array.
		// If all 3, formatted as x/t/n. if just vertices and textures, x/t.
		else if (command == "f") {
			file >> input;
			s = input.find("/");

			if (s != std::string::npos) {
				v1 = std::stoi(input.substr(0, s));
				t = input.rfind("/");
				if (s != t) {
					t1 = std::stoi(input.substr(s + 1, t));
					n1 = std::stoi(input.substr(t + 1));
				}
				else t1 = std::stoi(input.substr(s + 1));

				file >> input;
				s = input.find("/");
				v2 = std::stoi(input.substr(0, s));
				t = input.rfind("/");

				if (s != t) {
					t2 = std::stoi(input.substr(s + 1, t));
					n2 = std::stoi(input.substr(t + 1));
				}
				else t2 = std::stoi(input.substr(s + 1));

				file >> input;
				s = input.find("/");
				v3 = std::stoi(input.substr(0, s));
				t = input.rfind("/");

				if (s != t) {
					t3 = std::stoi(input.substr(s + 1, t));
					n3 = std::stoi(input.substr(t + 1));
				}
				else t3 = std::stoi(input.substr(s + 1));

				if (texturedModel) {
					// Negative vertices index from the current back of the vectors.
					// The compensations we have to make for positive indices are
					// unnecessary, so we undo them in advance (hence the +1's -2's)
					if (v1 < 0) v1 = currentMesh->vertices.size() + v1 + 1;
					if (v2 < 0) v2 = currentMesh->vertices.size() + v2 + 1;
					if (v3 < 0) v3 = currentMesh->vertices.size() + v3 + 1;
					if (t1 < 0) t1 = currentMesh->texCoords.size() + t1 - 2;
					if (t2 < 0) t2 = currentMesh->texCoords.size() + t2 - 2;
					if (t3 < 0) t3 = currentMesh->texCoords.size() + t3 - 2;
					if (n1 < 0) n1 = currentMesh->planes.size() + n1 + 1;
					if (n2 < 0) n2 = currentMesh->planes.size() + n2 + 1;
					if (n3 < 0) n3 = currentMesh->planes.size() + n3 + 1;

					// subtract 1 from indices because OBJ files index from one, 
					// not zero. But add 3 (net 2) to tex indices because we
					// already have 3 default coords
					currentMesh->addTriangle(v1 - 1, v2 - 1, v3 - 1,
						t1 + 2, t2 + 2, t3 + 2, currentMtlPtr, n1 - 1, n2 - 1, n3 - 1);
				}
				else {
					currentMesh->addTriangle(v1 - 1, v2 - 1, v3 - 1,
						0, 1, 2, currentMtlPtr, n1 - 1, n2 - 1, n3 - 1);
				}
			}
			else {
				v1 = std::stoi(input);
				file >> v2 >> v3;
				// subtract 1 from indices because OBJ files index from 1
				currentMesh->addTriangle(v1 - 1, v2 - 1, v3 - 1);
			}
		}
		else if (command == "usemtl") {
			std::string mtlName;
			char x[1];
			file.read(x, 1); // throw out space
			getline(file, mtlName);
			currentMtl = mtlName;
			currentMtlPtr = currentMesh->mtlLib.mtl[mtlName];
		}
		else if (command == "mtllib") {
			std::string materialLibrary;
			char x[1];
			file.read(x, 1); // throw out space
			getline(file, materialLibrary);
			materialLibrary = fileDir + materialLibrary;
			currentMesh->mtlLib.load(materialLibrary);
		}
		else {
			// other commands exist but will be ignored for now
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
	currentMesh->updateBoundingSphere();
	if (currentMesh->planes.size() == 0) currentMesh->recalcPlanes();
	groups.push_back(currentMesh);
	file.close();
}

void Model::splitGroups(int faceCountThreshold) {
	std::vector<Mesh*> preservedMeshes;
	std::vector<Mesh*> splitMeshes;
	for (Mesh* mesh : groups) {
		if (mesh->faces.size() < faceCountThreshold) {
			preservedMeshes.push_back(mesh);
			continue;
		}
		std::vector<Mesh*> newMesh;
		for (int i = 0; i < 8; ++i) newMesh.emplace_back(new Mesh);
		for (Triangle& tri : mesh->faces) {
			// Split the mesh into eight pieces depending on distance of
			// triangle centroid to the center of the bounding sphere.
			// Typically should create meshes of roughly equal size,
			// roughly partitioned according to axis planes of
			// the original bounding sphere.
			flatPoint center = _flatPoint(mesh->boundingSphere ^ ni);
			float xDist = (tri.a().e1ni() + tri.b().e1ni()
				+ tri.c().e1ni()) / 3 - center.e1ni();
			float yDist = (tri.a().e2ni() + tri.b().e2ni()
				+ tri.c().e2ni()) / 3 - center.e2ni();
			float zDist = (tri.a().e3ni() + tri.b().e3ni()
				+ tri.c().e3ni()) / 3 - center.e3ni();
			int octant = 0;
			if (xDist > 0) octant += 1;
			if (yDist > 0) octant += 2;
			if (zDist > 0) octant += 4;
			newMesh[octant]->addTriangle(tri);
		}
		auto emptyMeshes = std::remove_if(newMesh.begin(), newMesh.end(),
			[](Mesh* M) { return M->faces.size() == 0; });
		newMesh.erase(emptyMeshes, newMesh.end());

		splitMeshes.insert(splitMeshes.end(), newMesh.begin(), newMesh.end());
		for (Mesh* splitMesh : newMesh) {
			int i = 0, j = 0, k = 0;
			// Maps tell the new index of each vertex/texCoord/plane
			// in the new mesh. If the vertex hasn't been pulled in yet,
			// add it to the mesh and to the map. The next time a triangle
			// looks for that vertex, it will find the new index, so its
			// own stored index can be updated.
			std::map<int, int> vertMap;
			std::map<int, int> texMap;
			std::map<int, int> planeMap;
			for (Triangle& tri : splitMesh->faces) {
				for (int x = 0; x < 3; ++x) {
					if (vertMap.find(tri.v[x]) == vertMap.end()) {
						vertMap[tri.v[x]] = i;
						splitMesh->addVertex(mesh->vertices[tri.v[x]]);
						tri.v[x] = i;
						++i;
					}
					else tri.v[x] = vertMap[tri.v[x]];
					if (texMap.find(tri.t[x]) == texMap.end()) {
						texMap[tri.t[x]] = j;
						splitMesh->addTexCoord(mesh->texCoords[tri.t[x]]);
						tri.t[x] = j;
						++j;
					}
					else tri.t[x] = texMap[tri.t[x]];
					if (planeMap.find(tri.n[x]) == planeMap.end()) {
						planeMap[tri.n[x]] = k;
						splitMesh->addPlane(mesh->planes[tri.n[x]]);
						tri.n[x] = k;
						++k;
					}
					else tri.n[x] = planeMap[tri.n[x]];
				}
			}
			vertMap.clear();
			texMap.clear();
			planeMap.clear();		
		}
	}
	groups = std::move(splitMeshes);

	for (Mesh* g : groups) g->updateBoundingSphere();
	groups.insert(groups.end(), preservedMeshes.begin(), preservedMeshes.end());
}