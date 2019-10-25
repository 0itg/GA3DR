#pragma once
#include "c3ga.h"
#include "c3ga_util.h"
#include "Mesh.h"
#include "Material.h"
#include "MatUtil.h"
#include "Camera.h"
#include "Spinlock.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <execution>

class Window3d : public sf::RenderWindow {
public:
	Camera camera;
	sf::RenderStates windowSettings;

	Window3d(int width, int height, std::string title);
	~Window3d();
	void clearFrameBuf(pixel color = pixel(0, 0, 0, 0)) {
		memset(frameBuf, 0, getSize().x * getSize().y * sizeof(pixel));
		/*std::fill(std::execution::par_unseq, frameBuf, frameBuf
			+ getSize().x * getSize().y, color);*/
	}
	void hGradientFill(pixel colorStart, pixel colorEnd) {
		pixel* lastPixel = frameBuf + getSize().x * (getSize().y);
		int rowLen = getSize().x;
		float t = 0, tStep = 1.0f / getSize().y;
		for (pixel* pix = frameBuf; pix < lastPixel; pix += rowLen) {
			pixel color = colorStart * (1 - t) + colorEnd * t;
			std::fill(std::execution::par_unseq, pix, pix + rowLen - 1, color);
			t += tStep;
		}
	}
	void clearDepthBuf() {
		memset(depthBuf, 0, getSize().x * getSize().y * sizeof(float));
	}
	void display() {
		screenTex.update(screenPtr);
		draw(screenQuad, windowSettings);
		sf::RenderWindow::display();
	}

	void drawLine(float x1, float y1, float x2, float y2, 
				  pixel color = pixel(255, 255, 255, 255));
	inline void renderObject(Mesh& viewMesh);
	inline void renderWireframe(Mesh& viewMesh);
	// Takes 3d coordinates and divides by z to project to screen.
	void drawTriangle(const Triangle& tri);

private:
	pixel* frameBuf;
	float* depthBuf;
	SpinLock* writeToScreen; //One SpinLock per pixel
	sf::Texture screenTex;
	sf::Uint8* screenPtr;
	sf::VertexArray screenQuad;
};

inline pixel alphaBlend(pixel dst, pixel src) {
	int src_a_cmpl = ((255 - src.a) * dst.a) / 255;
	int fout_a = src_a_cmpl + src.a;
	return pixel((src_a_cmpl * dst.r + src.a * src.r) / fout_a,
		(src_a_cmpl * dst.g + src.a * src.g) / fout_a,
		(src_a_cmpl * dst.b + src.a * src.b) / fout_a,
		fout_a);
}

inline pixel sample(float u, float v, pixel* texture, int w, int h) {
	u -= (int)u; v -= (int)v;
	if (u < 0) u += 1;
	if (v < 0) v += 1;
	return texture[(int)((1 - v) * (h - 1)) * w + (int)(u * (w - 1))];
}

inline pixel pixLighting(pixel pix, pixel diffuse, pixel ambient) {
	return pixel(std::min(pix.r * (diffuse.r + ambient.r) / 255, 255),
				 std::min(pix.g * (diffuse.g + ambient.g) / 255, 255),
				 std::min(pix.b * (diffuse.b + ambient.b) / 255, 255), pix.a);
}

extern bool DEBUG_COLOR;
extern bool DEBUG_WIREFRAME;
extern bool DEBUG_NOSURFACES;
extern bool DEBUG_NOTEXTURES;
extern bool DEBUG_TOGGLESMOOTHSHADING;

inline void Window3d::renderObject(Mesh& viewMesh) {
	std::sort(std::execution::par_unseq,viewMesh.faces.begin(),
		viewMesh.faces.end(), [](const Triangle& T1, const Triangle& T2)->bool {
			char T1alpha = sample(T1.t[0], T1.t[1], T1.mtlPtr->texture(),
				T1.mtlPtr->texSize().x, T1.mtlPtr->texSize().y).a;
			char T2alpha = sample(T2.t[0], T2.t[1], T2.mtlPtr->texture(),
				T2.mtlPtr->texSize().x, T2.mtlPtr->texSize().y).a;
			return T1alpha < T2alpha;
		});
	std::for_each(std::execution::par_unseq, viewMesh.faces.begin(),
		viewMesh.faces.end(), [this](auto&& tri) {
			if (DEBUG_NOTEXTURES) {
				std::string name = tri.mtlPtr->mtlName;
				if (name != "red" && name != "green" && name != "blue")
				// default = solid white material
				tri.mtlPtr = tri.parent->mtlLib.mtl["default"];
			}
			drawTriangle(tri);
	});
}

inline void Window3d::renderWireframe(Mesh& viewMesh) {
	std::for_each(std::execution::seq, viewMesh.faces.begin(),
		viewMesh.faces.end(), [this](auto&& tri) {
			c3ga::flatPoint a = tri.a();
			c3ga::flatPoint b = tri.b();
			c3ga::flatPoint c = tri.c();
			drawLine(a.e1ni() / a.e3ni(), a.e2ni() / a.e3ni(),
				b.e1ni() / b.e3ni(), b.e2ni() / b.e3ni(), pixel(0, 0, 0, 255));
			drawLine(b.e1ni() / b.e3ni(), b.e2ni() / b.e3ni(),
				c.e1ni() / c.e3ni(), c.e2ni() / c.e3ni(), pixel(0, 0, 0, 255));
			drawLine(c.e1ni() / c.e3ni(), c.e2ni() / c.e3ni(),
				a.e1ni() / a.e3ni(), a.e2ni() / a.e3ni(), pixel(0, 0, 0, 255));
		});
}