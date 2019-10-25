#include "MatUtil.h"
#include "Window.h"
#include "Mesh.h"
#include "Model.h"

#include "c3ga.h"
#include "c3ga_util.h"
#include <SFML/Graphics.hpp>

#include <thread>
#include <algorithm>
#include <execution>

using namespace c3ga;

bool DEBUG_COLOR = false;
bool DEBUG_WIREFRAME = false;
bool DEBUG_NOSURFACES = false;
bool DEBUG_NOTEXTURES = false;
bool DEBUG_TOGGLESMOOTHSHADING = false;

int main() {
	const unsigned int windowW = 1024;
	const unsigned int windowH = 768;
	Window3d window(windowW, windowH, "Geometric Algebra-Based 3D Renderer");
	bool cursorVisible = false;
	window.setMouseCursorVisible(cursorVisible);

	const unsigned char darkRed[] = { 64, 0, 0 };
	const unsigned char red[] = { 191, 0, 0 };
	const unsigned char darkGreen[] = { 0, 64, 0 };
	const unsigned char green[] = { 0, 191, 0 };
	const unsigned char darkBlue[] = { 0, 0, 64 };
	const unsigned char blue[] = { 0, 0, 191 };
	Mesh::mtlLib.addMaterial(Mesh::mtlLib.texLib, "red", darkRed, red, red);
	Mesh::mtlLib.addMaterial(Mesh::mtlLib.texLib, "green", darkGreen, green, green);
	Mesh::mtlLib.addMaterial(Mesh::mtlLib.texLib, "blue", darkBlue, blue, blue);

	std::vector <Model*> models;
	std::vector<Mesh*> objects;

	Model teapot1("Models/teapot/teapot.obj"), teapot2("Models/teapot/teapot.obj");
	teapot1.motion(c3gaPoint(-30, 4, -4), 5);
	teapot2.motion(c3gaPoint(30, 4, -4), 5,
		exp(_bivectorE3GA(0.5 * M_PI * e1 ^ e3)));
	teapot1.recalcPlanes(true);
	models.push_back(&teapot1);
	models.push_back(&teapot2);

	//Model batman("Models/Batman/batman.obj");
	//batman.motion(c3gaPoint(0, 0, 2), 20);
	//models.push_back(&batman);

	//Model level("Models/WF/WF.obj");
	//level.motion(c3gaPoint(-100, 25, -50));
	//models.push_back(&level);

	Model head("Models/head/obj_free_male_head.obj");
	head.motion(c3gaPoint(0, 2, -12), 5);
	models.push_back(&head);
	head.recalcPlanes(true);
	head.splitGroups();
	//head.splitGroups(10000);


	for (Model* model : models)
		for (Mesh* group : model->groups) {
			objects.push_back(group);
		}

	sf::Clock clock;
	int tElapsed = 0;
	
	sf::Mouse::setPosition(sf::Vector2i(window.getSize().x/2,
		window.getSize().y/2), window);
	sf::Vector2i VMouse(0, 0);

	while (window.isOpen()) {
		tElapsed = clock.restart().asMicroseconds();
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:  // Function keys turn on/off debug code
				switch (event.key.code) {
				case sf::Keyboard::F1:
					DEBUG_WIREFRAME = !DEBUG_WIREFRAME;
					DEBUG_NOSURFACES = !DEBUG_NOSURFACES;
					break;
				case sf::Keyboard::F2:
					DEBUG_COLOR = !DEBUG_COLOR;
					break;
				case sf::Keyboard::F3:
					DEBUG_WIREFRAME = !DEBUG_WIREFRAME;
					break;
				case sf::Keyboard::F4:
					DEBUG_NOTEXTURES = !DEBUG_NOTEXTURES;
					break;
				case sf::Keyboard::F5: // Currently does nothing
					DEBUG_TOGGLESMOOTHSHADING = !DEBUG_TOGGLESMOOTHSHADING;
					break;
				case sf::Keyboard::R:
					window.camera.reset();
					break;
				case sf::Keyboard::LAlt:
					sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2,
						window.getSize().y / 2), window);
					cursorVisible = !cursorVisible;
					window.setMouseCursorVisible(cursorVisible);
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				case sf::Keyboard::Equal:
					window.camera.speedFactor *= 1.5;
					break;
				case sf::Keyboard::Subtract:
					window.camera.speedFactor /= 1.5;
					break;
				case sf::Keyboard::Q: {
					float FoV = window.camera.getFoV() + M_PI / 36;
					window.camera.setFoV(FoV); }
					break;
				case sf::Keyboard::E: {
					float FoV = window.camera.getFoV() - M_PI / 36;
					window.camera.setFoV(FoV); }
					break;
				}
				break;
			case sf::Event::MouseWheelScrolled:
				window.camera.speedFactor *= pow(1.5, event.mouseWheelScroll.delta);
				break;
			}
		}
		// L-alt releases mouse cursor, stops input from relative mouse movement
		if (!cursorVisible) { 
			sf::Vector2i newMouse = sf::Mouse::getPosition(window);
			sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2,
				window.getSize().y / 2), window);
			VMouse.x = newMouse.x - window.getSize().x / 2;
			VMouse.y = newMouse.y - window.getSize().y / 2;
		}
		else {
			VMouse = sf::Vector2i(0, 0);
		}

		window.camera.update_userInput(VMouse, tElapsed);
		//window.clearFrameBuf();
		window.hGradientFill(pixel(240, 240, 255, 0), pixel(96, 128, 255, 0));
		window.clearDepthBuf();

		std::for_each(std::execution::par_unseq, objects.begin(),
			objects.end(), [&window](auto && mesh) {
			Mesh viewMesh;
			window.camera.applyView(mesh, viewMesh);
			viewMesh.clip();
			if (DEBUG_NOSURFACES == false) window.renderObject(viewMesh);
			if (DEBUG_WIREFRAME == true) window.renderWireframe(viewMesh);
		});
			window.display();
	}
	return 0;
};