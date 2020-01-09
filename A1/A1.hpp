// Winter 2020

#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "maze.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

private:
	void initColour();
	void initGrid();
	void initCube();
	void drawAvatar();
	void dig();
	void reset();

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object
	GLuint maze_height = 1;
	glm::mat4 world_rotation; // rotate the maze and everything around the center
	

	// Fields related to cube geometry.
	GLuint cube_vao; // Vertex Array Object
	GLuint cube_vbo; // Vertex Buffer Object
	GLuint cube_ebo; // Element Buffer Object

	// Fields related to the avatar
	GLuint avatar_vao; // Vertex Array Object
	GLuint avatar_vbo; // Vertex Buffer Object
	GLuint avatar_ebo; // Element Buffer Object
	int attribVertex = 0;
	int attribNormal = 4;
	int attribTexCoord = 5;
	glm::mat4 avatar_translation;
	glm::vec2 avatar_pos; // x is horizontal, y is vertical
	bool shiftPressed = false;

	// Filds related to mouse control
	double mouse_prev_xPos;
	bool mouse_left_clicked;
	float dragging_factor = 0.0f;
	float dragging_threshold = 2.0f;
	float scale = 1.0f;

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	float colours[3];
	float cube_colour[3];
	float floor_colour[3];
	float avatar_colour[3];
	int current_col;
};
