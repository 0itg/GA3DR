#pragma once

#include "Window.h"

#include <SFML/Graphics.hpp>
#include "c3ga.h"
#include "c3ga_util.h"
#include "MatUtil.h"
#include "Spinlock.h"

using namespace c3ga;

Window3d::Window3d(int width, int height, std::string title) :
		RenderWindow(sf::VideoMode(width, height), title),
		camera(this, M_PI * 7 / 18) {  // Vertical FoV 70 degrees
	frameBuf = new pixel[width * height];
	depthBuf = new float[width * height];
	writeToScreen = new SpinLock[width * height];
	screenPtr = (sf::Uint8*)frameBuf;

	// Full-screen textured quad to send rasterized image
	// to the screen via SFML/OpenGl;
	screenTex.create(width, height);
	screenQuad = sf::VertexArray(sf::Quads, 4);
	screenQuad[0].position = sf::Vector2f(0, 0);
	screenQuad[1].position = sf::Vector2f(0, height);
	screenQuad[2].position = sf::Vector2f(width, height);
	screenQuad[3].position = sf::Vector2f(width, 0);
	screenQuad[0].texCoords = sf::Vector2f(0, 0);
	screenQuad[1].texCoords = sf::Vector2f(0, height);
	screenQuad[2].texCoords = sf::Vector2f(width, height);
	screenQuad[3].texCoords = sf::Vector2f(width, 0);

	windowSettings = sf::RenderStates(sf::BlendNone);
	windowSettings.texture = &screenTex;
}

Window3d::~Window3d() {
	delete[] frameBuf;
	delete[] depthBuf;
	delete[] writeToScreen;
}

void Window3d::drawLine(float x1, float y1, float x2, float y2, pixel color) {
	// Convert normalized coordinates to window coordinates.
	// Set mins/maxes to window borders for safety.
	x1 = std::min((unsigned int)((x1 + 1) *
		getSize().x * 0.5f), getSize().x - 1);
	y1 = std::min((unsigned int)((y1 + 1) *
		getSize().y * 0.5f), getSize().y - 1);
	x2 = std::min((unsigned int)((x2 + 1) *
		getSize().x * 0.5f), getSize().x - 1);
	y2 = std::min((unsigned int)((y2 + 1) *
		getSize().y * 0.5f), getSize().y - 1);
	x1 = std::max(x1, 0.0f);
	y1 = std::max(y1, 0.0f);
	x2 = std::max(x2, 0.0f);
	y2 = std::max(y2, 0.0f);
	int x, y, dx, dy, yPrg, xPrg;
	dx = x2 - x1;
	dy = y2 - y1;
	yPrg = 0; xPrg = 0;
	// loop over whichever direction has more
	// pixels to make sure we get them all.
	if (abs(dx) >= abs(dy))
	{
		if ((dx >= 0) && (dy >= 0))
		{
			y = y1;
			for (x = x1; x < x2; ++x)
			{
				yPrg += dy;	// count y-steps
				// when y-steps >= 1 x-step, advance y-coord by 1.
				// Subtract x-step to reset count without losing extra distance.
				if (yPrg >= dx) { y += 1; yPrg -= dx; }
				frameBuf[getSize().x * y + x] = color;
			}
		}
		else if ((dx < 0) && (dy >= 0))
		{
			y = y2;
			for (x = x2; x < x1; ++x)
			{
				yPrg += dy;
				if (yPrg >= -dx) { y -= 1; yPrg += dx; }
				frameBuf[getSize().x * y + x] = color;
			}
		}
		else if ((dx < 0) && (dy < 0))
		{
			y = y2;
			for (x = x2; x < x1; ++x)
			{
				yPrg -= dy;
				if (yPrg >= -dx) { y += 1; yPrg += dx; }
				frameBuf[getSize().x * y + x] = color;
			}
		}
		else if ((dx >= 0) && (dy < 0))
		{
			y = y1;
			for (x = x1; x < x2; ++x)
			{
				yPrg -= dy;
				if (yPrg >= dx) { y -= 1; yPrg -= dx; }
				frameBuf[getSize().x * y + x] = color;
			}
		}
	}
	else
	{
		if ((dx >= 0) && (dy >= 0))
		{
			x = x1;
			for (y = y1; y < y2; ++y)
			{
				xPrg += dx;
				if (xPrg >= dy) { x += 1; xPrg -= dy; }
				frameBuf[getSize().x * y + x] = color;
			}
		}
		else if ((dx < 0) && (dy >= 0))
		{
			x = x1;
			for (y = y1; y < y2; ++y)
			{
				xPrg -= dx;
				if (xPrg >= dy) { x -= 1; xPrg -= dy; }
				frameBuf[getSize().x * y + x] = color;
			}
		}
		else if ((dx < 0) && (dy < 0))
		{
			x = x2;
			for (y = y2; y < y1; ++y)
			{
				xPrg -= dx;
				if (xPrg >= -dy) { x += 1; xPrg += dy; }
				frameBuf[getSize().x * y + x] = color;
			}
		}
		else if ((dx >= 0) && (dy < 0))
		{
			x = x2;
			for (y = y2; y < y1; ++y)
			{
				xPrg += dx;
				if (xPrg >= -dy) { x -= 1; xPrg += dy; }
				frameBuf[getSize().x * y + x] = color;
			}
		}
	}
}

void Window3d::drawTriangle(const Triangle& tri) {
	int x, y, pix;
	float u, v, w, uStart, uEnd, vStart, vEnd, wStart,
		  wEnd, r, r1, s, s1, t, t1, tStep, slope12, slope23;

	// Load parameters from triangle object.
	float u1 = tri.ta().x; float u2 = tri.tb().x; float u3 = tri.tc().x;
	float v1 = tri.ta().y; float v2 = tri.tb().y; float v3 = tri.tc().y;
	// Calculate texture w for perspective correction
	float w1 = 1 / tri.a().noni(); float w2 = 1 / tri.b().noni();
	float w3 = 1 / tri.c().noni();  
	u1 *= w1; v1 *= w1; u2 *= w2; v2 *= w2; u3 *= w3; v3 *= w3;

	pixel* texture = tri.mtlPtr->texture();
	const int width = tri.mtlPtr->texSize().x;
	const int height = tri.mtlPtr->texSize().y;

	const unsigned char* amb = tri.mtlPtr->Ka.c;
	const unsigned char* dif = tri.mtlPtr->Kd.c;
	pixel ambient(amb[0], amb[1], amb[2]);
	
	const float orientA = std::max(tri.na().e1e2noni(), 0.0f);
	pixel color1(orientA * dif[0], orientA * dif[1], orientA * dif[2]);
	const float orientB = std::max(tri.nb().e1e2noni(), 0.0f);
	pixel color2(orientB * dif[0], orientB * dif[1], orientB * dif[2]);
	const float orientC = std::max(tri.nc().e1e2noni(), 0.0f);
	pixel color3(orientC * dif[0], orientC * dif[1], orientC * dif[2]);
	pixel diffuse, colStart, colEnd;

	// Convert normalized coordinates to window coordinates.
	int x1 = ((tri.a().e1ni() / tri.a().noni() + 1) * getSize().x * 0.5f - 0.5f);
	int y1 = ((tri.a().e2ni() / tri.a().noni() + 1) * getSize().y * 0.5f - 0.5f);
	int x2 = ((tri.b().e1ni() / tri.b().noni() + 1) * getSize().x * 0.5f - 0.5f);
	int y2 = ((tri.b().e2ni() / tri.b().noni() + 1) * getSize().y * 0.5f - 0.5f);
	int x3 = ((tri.c().e1ni() / tri.c().noni() + 1) * getSize().x * 0.5f - 0.5f);
	int y3 = ((tri.c().e2ni() / tri.c().noni() + 1) * getSize().y * 0.5f - 0.5f);

	// Convert normalized coordinates to window
	// coordinates. Clamp to edges of screen.
	//float x1 = std::min(((tri.a().e1ni() / tri.a().noni() + 1)
	//	* getSize().x * 0.5f), (float)(getSize().x - 1));
	//float y1 = std::min(((tri.a().e2ni() / tri.a().noni() + 1)
	//	* getSize().y * 0.5f), (float)(getSize().y - 1));
	//float x2 = std::min(((tri.b().e1ni() / tri.b().noni() + 1) 
	//* getSize().x * 0.5f), (float)(getSize().x - 1));
	//float y2 = std::min(((tri.b().e2ni() / tri.b().noni() + 1)
	//	* getSize().y * 0.5f), (float)(getSize().y - 1));
	//float x3 = std::min(((tri.c().e1ni() / tri.c().noni() + 1)
	//	* getSize().x * 0.5f), (float)(getSize().x - 1));
	//float y3 = std::min(((tri.c().e2ni() / tri.c().noni() + 1)
	//	* getSize().y * 0.5f), (float)(getSize().y - 1));
	//x1 = std::max(x1, 0.0f);
	//y1 = std::max(y1, 0.0f);
	//x2 = std::max(x2, 0.0f);
	//y2 = std::max(y2, 0.0f);
	//x3 = std::max(x3, 0.0f);
	//y3 = std::max(y3, 0.0f);

	// sort the vertices by y-coordinate
	if (y3 > y1) { 
		std::swap(x3, x1); std::swap(y3, y1); std::swap(u3, u1);
		std::swap(v3, v1); std::swap(w3, w1); std::swap(color3, color1);
	} 
	if (y3 > y2) {
		std::swap(x3, x2); std::swap(y3, y2); std::swap(u3, u2);
		std::swap(v3, v2); std::swap(w3, w2); std::swap(color3, color2);
	}
	if (y2 > y1) {
		std::swap(x2, x1); std::swap(y2, y1); std::swap(u2, u1);
		std::swap(v2, v1); std::swap(w2, w1); std::swap(color2, color1);
	}

	const float dy12 = y2 - y1;
	const float dy13 = y3 - y1;
	const float dy23 = y3 - y2;
	float slope13 = 0;
	if (dy12) slope12 = (x2 - x1) / dy12;
	if (dy13) slope13 = (x3 - x1) / dy13;
	if (dy23) slope23 = (x3 - x2) / dy23;

	float q = x1;
	float p = x1;
	s = 0; r = 0;
	float tStart = 0;

	if (dy12) {
		float* lineStart = &p;
		float* lineEnd = &q;
		if (slope12 < slope13) {
			std::swap(lineStart, lineEnd);
			tStart = 1;
		}
		for (y = y1; y > y2; --y) {
			s1 = 1 - s; r1 = 1 - r;
			uStart = u1 * s1 + u2 * s;
			vStart = v1 * s1 + v2 * s;
			wStart = w1 * s1 + w2 * s;
			colStart = color1 * s1 + color2 * s;
			uEnd = u1 * r1 + u3 * r;
			vEnd = v1 * r1 + v3 * r;
			wEnd = w1 * r1 + w3 * r;
			colEnd = color1 * r1 + color3 * r;
			s -= 1 / dy12; r -= 1 / dy13;
			t = tStart;
			tStep = 1 / (float)((int)q - (int)p + 1);

			for (x = (int)*lineStart; x < (int)*lineEnd; ++x) {
				t1 = 1 - t;
				u = uStart * t1 + uEnd * t;
				v = vStart * t1 + vEnd * t;
				w = wStart * t1 + wEnd * t;
				diffuse = colStart * (1 - t) + colEnd * t;
				t += tStep;

				pix = getSize().x * y + x;
				writeToScreen[pix].lock();
         		if (depthBuf[pix] < w) {
					pixel s = sample(u / w, v / w, texture, width, height);
					if (s.a == 255) {
						depthBuf[pix] = w;
						frameBuf[pix] = pixLighting(s, diffuse, ambient);
					}
					else if (s.a > 0) {
						depthBuf[pix] = w;
						frameBuf[pix] = alphaBlend(frameBuf[pix],
							pixLighting(s, diffuse, ambient));
					}
				}
				//else if (frameBuf[pix].a < 255) {
				//	pixel s = sample(u / w, v / w, texture, width, height);
				//		frameBuf[pix] = alphaBlend(pixLighting(s, diffuse, ambient),
				//			frameBuf[pix]);
				//}
				writeToScreen[pix].unlock();
			}
			p -= slope12;
			q -= slope13;
		}
	}
	if (dy23) {
		tStart = 0;
		p = x2; s = 0;
		float* lineStart = &p;
		float* lineEnd = &q;
		if (slope23 > slope13) {
			std::swap(lineStart, lineEnd);
			tStart = 1;
		}
		for (y = y2; y > y3; --y) {
			s1 = 1 - s; r1 = 1 - r;
			uStart = u2 * s1 + u3 * s;
			vStart = v2 * s1 + v3 * s;
			wStart = w2 * s1 + w3 * s;
			colStart = color2 * s1 + color3 * s;
			uEnd = u1 * r1 + u3 * r;
			vEnd = v1 * r1 + v3 * r;
			wEnd = w1 * r1 + w3 * r;
			colEnd = color1 * r1 + color3 * r;
			s -= 1 / dy23; r -= 1 / dy13;
			t = tStart;
			tStep = 1 / (float)((int)q - (int)p + 1);

			for (x = (int)*lineStart; x < (int)*lineEnd; ++x) {
				t1 = 1 - t;
				u = uStart * t1 + uEnd * t;
				v = vStart * t1 + vEnd * t;
				w = wStart * t1 + wEnd * t;
				diffuse = colStart * t1 + colEnd * t;
				t += tStep;

				pix = getSize().x * y + x;
				writeToScreen[pix].lock();
				if (depthBuf[pix] < w) {
					pixel s = sample(u / w, v / w, texture, width, height);
					if (s.a == 255) {
						depthBuf[pix] = w;
						frameBuf[pix] = pixLighting(s, diffuse, ambient);
					}
					else if (s.a > 0) {
						depthBuf[pix] = w;
						frameBuf[pix] = alphaBlend(frameBuf[pix],
							pixLighting(s, diffuse, ambient));
					}
				}
				//else if (frameBuf[pix].a < 255) {
				//	pixel s = sample(u / w, v / w, texture, width, height);
				//	frameBuf[pix] = alphaBlend(pixLighting(s, diffuse, ambient),
				//		frameBuf[pix]);
				//}
				writeToScreen[pix].unlock();
			}
			p -= slope23;
			q -= slope13;
		}
	}
}