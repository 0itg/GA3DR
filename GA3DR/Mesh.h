#pragma once

#include "c3ga.h"
#include "c3ga_util.h"

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <boost/pool/pool_alloc.hpp>

class Mesh;
class Material;
class MtlLib;
class TexLib;

class pixel {
public:
	union {
		struct {
			unsigned char r, g, b, a;
		};
		unsigned char c[4] = { 0, 0, 0, 255 };
	};
	pixel() {}
	pixel(unsigned char w, unsigned char x, unsigned char y, unsigned char z) :
		r(w), g(x), b(y), a(z) {}
	pixel(unsigned char w, unsigned char x, unsigned char y) :
		r(w), g(x), b(y), a(255) {}
	pixel(unsigned char rgba[4]) { c[0] = rgba[0]; c[1] = rgba[1]; c[2] = rgba[2]; c[3] = rgba[3]; }
	pixel(sf::Color& col) : r(col.r), g(col.g), b(col.b), a(col.a) {}

	friend pixel operator + (const pixel& p, const pixel& q) {
		return pixel(p.r + q.r, p.g + q.g, p.b + q.b, p.a + q.a);
	}
	friend pixel operator * (const pixel& p, const float& s) {
		return pixel(p.r * s, p.g * s, p.b * s, p.a * s);
	}
	friend pixel operator * (const float& s, const pixel& p) {
		return pixel(p.r * s, p.g * s, p.b * s, p.a * s);
	}
	//component-by-component multiplication
	friend pixel operator * (const pixel& p, const pixel& q) {
		return pixel(p.r * q.r, p.g * q.g, p.b * q.b, p.a * q.a);
	}
};

class Triangle {
public:
	Mesh* parent;
	int v[3]; int t[3]; int n[3];
	Material* mtlPtr;

	Triangle(Mesh* mesh, int a, int b, int c,
		int d = 0, int e = 1, int f = 2, Material * mtl = nullptr,
		int g = -1, int h = -1, int i = -1);

	Triangle(Mesh* newPar, Triangle tri) {
		parent = newPar; mtlPtr = tri.mtlPtr;
		v[0] = tri.v[0]; v[1] = tri.v[1]; v[2] = tri.v[2]; // vertex indices
		t[0] = tri.t[0]; t[1] = tri.t[1]; t[2] = tri.t[2]; // texture indices
		n[0] = tri.n[0]; n[1] = tri.n[1]; n[2] = tri.n[2]; // plane indices
		// "plane" as in the plane usually described by a normal vector
		// stored directly thanks to geometric algebra
	}
	void cycle() {
		std::swap(v[0], v[1]); std::swap(v[0], v[2]);
		std::swap(t[0], t[1]); std::swap(t[0], t[2]);
		std::swap(n[0], n[1]); std::swap(n[0], n[2]);
	}

	inline c3ga::flatPoint a() const;
	inline c3ga::flatPoint b() const;
	inline c3ga::flatPoint c() const;
	inline sf::Vector2f   ta() const;
	inline sf::Vector2f   tb() const;
	inline sf::Vector2f   tc() const;
	inline c3ga::plane    na() const;
	inline c3ga::plane    nb() const;
	inline c3ga::plane    nc() const;
};

// 3d mesh stored using flatPoints,
// which are most convenient for transforming with OM matrices.
class Mesh {
public:
	// Currently, normalizedFlatPoints would be totally acceptable and
	// save memory. Leaving as is in case I need the w-coord later.
	std::vector<c3ga::flatPoint, boost::pool_allocator<c3ga::flatPoint,
		boost::default_user_allocator_malloc_free>> vertices;
	std::vector<Triangle, boost::pool_allocator<Triangle,
		boost::default_user_allocator_malloc_free>> faces;
	//std::vector<sf::Vector2f, boost::fast_pool_allocator<sf::Vector2f,
	//  boost::default_user_allocator_malloc_free>> texCoords;
	std::vector<c3ga::plane, boost::pool_allocator<c3ga::plane,
		boost::default_user_allocator_malloc_free>> planes;
	std::vector<sf::Vector2f> texCoords;
	std::string groupName;
	static MtlLib mtlLib;
	c3ga::dualSphere boundingSphere;
	float boundingSphereR;

	void addVertex(float x, float y, float z) {
		vertices.emplace_back(
			c3ga::flatPoint_e1ni_e2ni_e3ni_noni, x, y, z, 1);
	}
	void addVertex(c3ga::flatPoint P) {
		vertices.push_back(P);
	}
	void addTriangle(int a, int b, int c, 
			int d = 0, int e = 1, int f = 2, Material* mtlPtr = nullptr,
			int g = -1, int h = -1, int i = -1) {
		faces.emplace_back(this, a, b, c, d, e, f, mtlPtr, g, h, i);
	}
	void addTriangle(Triangle tri) {
		faces.emplace_back(this, tri);
	}
	c3ga::flatPoint getTriVertex(int fIndex, int vIndex) {
		return vertices[faces[fIndex].v[vIndex]];
	}
	void addTexCoord(float u, float v) {
		texCoords.emplace_back(u, v);
	}
	void addTexCoord(sf::Vector2f vec) {
		texCoords.push_back(vec);
	}
	void addPlane(c3ga::plane N) {
		planes.push_back(N);
	}
	void addPlane(float x, float y, float z, float d = 0) {
		planes.emplace_back(
			c3ga::plane_e1e2e3ni_e1e2noni_e1e3noni_e2e3noni, x, y, z, d);
	}
	sf::Vector2f getTexCoords(int fIndex, int tIndex) {
		return texCoords[faces[fIndex].t[tIndex]];
	}

	// Load mesh from OBJ file. lines starting with 'v' contain xyz vertex 
	//coordinates, while lines starting with f list vertices (by place in list)
	// belonging to a face.
	//void load(std::string objPath);
	void motion(c3ga::normalizedPoint location, float scale = 1, c3ga::rotor R = c3ga::_rotor(1));
	void updateBoundingSphere();
	void recalcPlanes(bool smooth = false);
	void clip();

	Mesh() {
		//Default texture coords used by all untextured triangles
		addTexCoord(0, 0);
		addTexCoord(1, 0);
		addTexCoord(0, 1);
	}
	//Mesh(std::string objPath) {
	//	addTexCoord(0, 0);
	//	addTexCoord(1, 0);
	//	addTexCoord(0, 1);
	//	load(objPath);
	//}
};

inline  c3ga::flatPoint Triangle::a()  const { return parent->vertices[v[0]]; }
inline  c3ga::flatPoint Triangle::b()  const { return parent->vertices[v[1]]; }
inline  c3ga::flatPoint Triangle::c()  const { return parent->vertices[v[2]]; }
inline  sf::Vector2f    Triangle::ta() const { return parent->texCoords[t[0]];}
inline  sf::Vector2f    Triangle::tb() const { return parent->texCoords[t[1]];}
inline  sf::Vector2f    Triangle::tc() const { return parent->texCoords[t[2]];}
inline  c3ga::plane     Triangle::na() const { return parent->planes[n[0]];   }
inline  c3ga::plane     Triangle::nb() const { return parent->planes[n[1]];   }
inline  c3ga::plane     Triangle::nc() const { return parent->planes[n[2]];   }