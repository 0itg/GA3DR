#include "Mesh.h"

#include "c3ga.h"
#include "c3ga_util.h"
#include "MatUtil.h"
#include "Window.h"
#include "Camera.h"

#include <SFML/Graphics.hpp>

using namespace c3ga;

Camera::Camera(Window3d* window, float FoVinit) :
	Rhoriz(c3ga::_rotor(1)), Rvert(c3ga::_rotor(1)),
	Rposition(c3ga::_normalizedTranslator(1)), parent(window), FoV(FoVinit),
	projMat(Mat4x4{ -(float)window->getSize().y /
	(window->getSize().x * tanf(FoVinit / 2)), 0.0f, 0.0f, 0.0f,
						 0.0f, -1 / tanf(FoVinit / 2), 0.0f, 0.0f,
						 0.0f, 0.0f, -1.0f, 0.0f,
						 0.0f, 0.0f, -1.0f, 0.0f }), speedFactor(1) {
	c3ga::TRversor RModel = _TRversor(Rvert * Rhoriz * Rposition);
	modelMat = versorToFlatPointMatrix(RModel);
	viewMat = matrixMultiply(projMat, modelMat);
	planeTransformMat = versorToPlaneMatrix(RModel);
}

void Camera::update_userInput(const sf::Vector2i& mouseVec, int tElapsed) {
	//Rotate model in plane of mouse vector and screen normal
	//Noncommutativity of rotations lead to undesirable rolling of the camera
	//bivectorE3GA MousePlane = _bivectorE3GA(vectorE3GA(vectorE3GA_e1_e2_e3,
	//										  Mouse.x, VMouse.y, 0.0f)^e3); 
	//if ((VMouse.x || VMouse.y )!= 0)
	//{
	//	rotor R = _rotor(exp(-tElapsed / 200000.0f * MousePlane));
	//	ROrientation = R * ROrientation;
	//}

	// Rotate using Euler Angles, no roll, for FPS-like controls
	bivectorE3GA hKeybd = _bivectorE3GA(tElapsed / 2000000.0f * e1 ^ e3);
	bivectorE3GA hMouse = hKeybd * mouseVec.x * 0.4;
	bivectorE3GA vKeybd = _bivectorE3GA(tElapsed / 2000000.0f * e3 ^ e2);
	bivectorE3GA vMouse = vKeybd * mouseVec.y * 0.4;

	if (mouseVec.x != 0) {
		rotor R = _rotor(exp(-hMouse));
		Rhoriz = R * Rhoriz;
	}
	if (mouseVec.y != 0) {
		rotor R = _rotor(exp(vMouse));
		Rvert = R * Rvert;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
		rotor R = _rotor(exp(hKeybd));
		Rhoriz = R * Rhoriz;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
		rotor R = _rotor(exp(-hKeybd));
		Rhoriz = R * Rhoriz;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
		rotor R = _rotor(exp(-vKeybd));
		Rvert = R * Rvert;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
		rotor R = _rotor(exp(vKeybd));
		Rvert = R * Rvert;
	}

	rotor ROrientation = Rvert * Rhoriz;
	rotor ROI = inverse(Rvert * Rhoriz);

	float Tspeed = speedFactor / 100000.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) Tspeed /= 8;

	freeVector forward = _freeVector(tElapsed * Tspeed * ROI * e3ni * ROrientation);
	freeVector up = _freeVector(tElapsed * Tspeed * ROI * e2ni * ROrientation);
	freeVector strafe = _freeVector(tElapsed * Tspeed * ROI * e1ni * ROrientation);


	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
		normalizedTranslator R = _normalizedTranslator(1 + strafe);
		Rposition = R * Rposition;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
		normalizedTranslator R = _normalizedTranslator(1 - strafe);
		Rposition = R * Rposition;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
		normalizedTranslator R = _normalizedTranslator(1 - forward);
		Rposition = R * Rposition;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
		normalizedTranslator R = _normalizedTranslator(1 + forward);
		Rposition = R * Rposition;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
		normalizedTranslator R = _normalizedTranslator(1 + up);
		Rposition = R * Rposition;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
		normalizedTranslator R = _normalizedTranslator(1 - up);
		Rposition = R * Rposition;
	}

	TRversor RModel = _TRversor(Rvert * Rhoriz * Rposition);
	modelMat = versorToFlatPointMatrix(RModel);
	viewMat = matrixMultiply(projMat, modelMat);
	planeTransformMat = versorToPlaneMatrix(RModel);
}

void Camera::setFoV(float newFoV){
	float FoVfactor = 1 / tanf(newFoV / 2);
	projMat.c[0] = -(float)parent->getSize().y / (float)parent->getSize().x * FoVfactor;
	projMat.c[5] = -FoVfactor;
	FoV = newFoV;
	viewMat = matrixMultiply(projMat, modelMat);
}