#pragma once
#include "c3ga.h"
#include "MatUtil.h"
#include "Window.h"

#include <SFML/Graphics.hpp>
#include <execution>

class Window3d;

class Camera {
	Window3d* parent;
	c3ga::rotor Rhoriz;
	c3ga::rotor Rvert;
	c3ga::normalizedTranslator Rposition;
	Mat4x4 projMat;
	Mat4x4 viewMat;
	Mat4x4 modelMat;
	Mat4x4 planeTransformMat;
	float FoV;

public:
	float speedFactor;

	void update_userInput(const sf::Vector2i& mouseVec, int tElapsed);
	Camera(Window3d* window, float FoV);
	c3ga::flatPoint applyView(c3ga::flatPoint p) {
		return matrixFPMultiply(viewMat, p);
	}
	c3ga::plane applyView(c3ga::plane P) {
		return matrixPlaneMultiply(planeTransformMat, P);
	}
	void reset() {
		Rhoriz = 1; Rvert = 1; Rposition = 0;
		setFoV(M_PI / 2);
	}
	float getFoV() { return FoV; };
	void setFoV(float FoV);
	void applyView(Mesh* mesh, Mesh& viewMesh) {
		viewMesh.vertices.resize(mesh->vertices.size());
		std::transform(std::execution::par_unseq, mesh->vertices.begin(),
			mesh->vertices.end(), viewMesh.vertices.begin(),
			[&](c3ga::flatPoint& p) { return applyView(p); });

		viewMesh.texCoords = mesh->texCoords;

		viewMesh.planes.resize(mesh->planes.size());
		std::transform(std::execution::par_unseq, mesh->planes.begin(),
			mesh->planes.end(), viewMesh.planes.begin(),
			[&](c3ga::plane& P) { return applyView(P); });
		for (Triangle& tri : mesh->faces) {
			// using triangles of original mesh with vertices of transformed mesh
			c3ga::flatPoint a = viewMesh.vertices[tri.v[0]];
			c3ga::flatPoint b = viewMesh.vertices[tri.v[1]];
			c3ga::flatPoint c = viewMesh.vertices[tri.v[2]];

			// Calculate normal vector to find orientation.
			//If triangle is facing away from camera, don't draw it.
			c3ga::plane normal =
				_plane(((c3ga::no << a) ^ (c3ga::no << b)) ^ c);
			if (normal.e1e2noni() > 0)  viewMesh.addTriangle(tri);
		}
	}
};