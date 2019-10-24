#pragma once
#include "Mesh.h"

class TexLib {
public:
	std::unordered_map<std::string, sf::Image*> tex;

	bool addTexture(std::string texPath) {
		// Don't add texture if it's already in the library
		if (tex.find(texPath) == tex.end()) {
			sf::Image* texPtr = new sf::Image;
			if (texPtr->loadFromFile(texPath)) {
				tex[texPath] = texPtr;
				return true;
			}
			else return false;
		}
		// but tell the caller it's in there and the filename is a valid key.
		else return true;
	}
	TexLib() {
		// Texture Library should always have a solid-color default
		// for untextured objects and missing textures
		sf::Image* defaultTex = new sf::Image;
		defaultTex->create(1, 1, sf::Color::White);
		tex["default"] = defaultTex;
	}
	~TexLib()
	{
		for (auto& T : tex)
			delete T.second;
	}
};

class Material
{
public:
	TexLib& texLib;
	std::string mtlName;
	std::string map_Kd;
	std::string map_Ka;
	std::string map_Ks;
	float Ns;
	/*unsigned char Ka[3];
	unsigned char Kd[3];
	unsigned char Ks[3];*/
	pixel Ka;
	pixel Kd;
	pixel Ks;
	float Ni;
	float d;
	int illum;

	Material(TexLib& tL, std::string mtl, const unsigned char a[3],
		const unsigned char d[3], const unsigned char s[3],
		std::string map_KaPath = "", std::string map_KdPath = "",
		std::string map_KsPath = "") : texLib(tL), mtlName(mtl) {
		Ka.c[0] = a[0]; Ka.c[1] = a[1]; Ka.c[2] = a[2];
		Kd.c[0] = d[0]; Kd.c[1] = d[1]; Kd.c[2] = d[2];
		if (texLib.addTexture(map_KaPath)) map_Ka = map_KaPath;
		else map_Ka = "default";
		if (texLib.addTexture(map_KdPath)) map_Kd = map_KdPath;
		else map_Kd = "default";
		if (texLib.addTexture(map_KsPath)) map_Ks = map_KsPath;
		else map_Ks = "default";
	}
	pixel* texture() {
		return (pixel*)texLib.tex[map_Kd]->getPixelsPtr();
	}
	sf::Vector2u texSize() {
		return texLib.tex[map_Kd]->getSize();
	}
};

class MtlLib {
public:
	std::unordered_map<std::string, Material*> mtl;
	TexLib texLib;

	void addMaterial(TexLib& tL, std::string matl,
		const unsigned char a[3], const unsigned char d[3],
		const unsigned char s[3], std::string map_KaPath = "",
		std::string map_KdPath = "", std::string map_KsPath = "") {
		Material* mtlPtr = new Material(tL, matl, a, d, s,
			map_KaPath, map_KdPath, map_KsPath);
		mtl[mtlPtr->mtlName] = mtlPtr;
	}
	bool load(std::string filePath);
	MtlLib() {
		const unsigned char white[] = { 191, 191, 191 };
		const unsigned char grey[] = { 64, 64, 64 };
		mtl["default"] = new Material(texLib, "default", grey, white, white);
	}
	~MtlLib() {
		for (auto& T : mtl) delete T.second;
	}
};

void splitFileNameAndDirectory(std::string& inputPath,
	std::string& outputDir, std::string& outputName);