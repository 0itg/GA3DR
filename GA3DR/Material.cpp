#include "Material.h"

#include <fstream>

bool MtlLib::load(std::string filePath) {
	std::ifstream file;
	file.open(filePath);

	bool firstMtl = true;
	std::string name;
	std::string fileDir;
	std::string fileName;
	std::string map_KaPath = "";
	std::string map_KdPath = "";
	std::string map_KsPath = "";
	float Ns = 0;
	unsigned char Ka[3] = { 64, 64, 64 };
	unsigned char Kd[3] = { 191, 191, 191 };
	unsigned char Ks[3] = { 0, 0, 0 };
	float Kaf[3] = { 0, 0, 0 };
	float Kdf[3] = { 0, 0, 0 };
	float Ksf[3] = { 0, 0, 0 };
	float Ni = 0;
	float d = 1;
	int illum = 2;
	std::string command;

	splitFileNameAndDirectory(filePath, fileDir, fileName);

	while (file >> command) {
		if (command == "newmtl" && firstMtl) {
			file >> name;
			firstMtl = false;
		}
		else if (command == "newmtl" && !firstMtl) {
			if (map_KsPath == "default") map_KsPath = map_KdPath;
			if (map_KaPath == "default") map_KaPath = map_KdPath;
			if (map_KdPath == "default") map_KdPath = map_KaPath;
			addMaterial(texLib, name, Ka, Kd, Ks,
				map_KaPath, map_KdPath, map_KsPath);
			file >> name;
			map_KaPath = "default"; // next material may not have texture
			map_KdPath = "default";
			map_KsPath = "default";
		}
		else if (command == "Ka") {
			file >> Kaf[0] >> Kaf[1] >> Kaf[2];
			Ka[0] = Kaf[0] * 255;
			Ka[1] = Kaf[1] * 255;
			Ka[2] = Kaf[2] * 255;
		}
		else if (command == "Kd") {
			file >> Kdf[0] >> Kdf[1] >> Kdf[2];
			Kd[0] = Kdf[0] * 255;
			Kd[1] = Kdf[1] * 255;
			Kd[2] = Kdf[2] * 255;
		}
		else if (command == "Ks") {
			file >> Ksf[0] >> Ksf[1] >> Ksf[2];
			Ks[0] = Ksf[0] * 255;
			Ks[1] = Ksf[1] * 255;
			Ks[2] = Ksf[2] * 255;
		}
		else if (command == "Ni") {
			file >> Ni;
		}
		else if (command == "d") {
			file >> d;
		}
		else if (command == "TR") {
			file >> d;
			d = 1 - d;
		}
		else if (command == "illum") {
			file >> illum;
		}
		else if (command == "map_Ka") {
			char x[1];
			file.read(x, 1); // throw out space
			getline(file, map_KaPath);
			map_KaPath = fileDir + map_KaPath;
		}
		else if (command == "map_Kd") {
			char x[1];
			file.read(x, 1); // throw out space
			getline(file, map_KdPath);
			map_KdPath = fileDir + map_KdPath;
		}
		else if (command == "map_Ks") {
			char x[1];
			file.read(x, 1); // throw out space
			getline(file, map_KsPath);
			map_KsPath = fileDir + map_KsPath;
		}
		else {
			// ignore comments and unimplemented commands
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
	file.close();
	if (firstMtl == true)
		return false;
	else {
		addMaterial(texLib, name, Ka, Kd, Ks, map_KaPath, map_KdPath, map_KsPath);
		return true;
	}
}

void splitFileNameAndDirectory(std::string& inputPath,
	std::string& outputDir, std::string& outputName) {
	size_t fileDirEnd = inputPath.rfind('\\');
	if (fileDirEnd == std::string::npos) {
		fileDirEnd = inputPath.rfind('/');
	}
	if (fileDirEnd != std::string::npos) {
		outputName = inputPath.substr(fileDirEnd + 1);
		outputDir = inputPath.substr(0, fileDirEnd + 1);
	}
	else outputName = inputPath;
}