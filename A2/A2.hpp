// Winter 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <string>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

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

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;


	// A2 variables:
	glm::mat4 modelTransfer;
	glm::mat4 viewTransfer;
	// plane info
	GLfloat nearPlane, farPlane, fov;
	// viewport info
	glm::vec2 topLeftVP, botRightVP;
	GLfloat window_width, window_height;
	// base v
	glm::vec4 world_base_x = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 world_base_y = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 world_base_z = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	glm::vec4 world_base_0 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// perspective vertix in 2D
	glm::vec2 perspective_cube_vertex_2D[8];
	glm::vec4 VCS_cube_vertex[8];

	// base for model
	glm::vec4 model_base_x = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 model_base_y = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 model_base_z = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	glm::vec4 model_base_0 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// base for view
	glm::vec4 view_base_x = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 view_base_y = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 view_base_z = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	glm::vec4 view_base_0 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// view info
	glm::vec3 eye_origin;
	glm::vec3 world_origin;
	glm::vec3 cube_origin;

	// color info
	glm::vec3 modelGnomonColor[3] = {glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};
	glm::vec3 worldGnomonColor[3] = {glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f)};

	// cube sacle
	GLfloat scale_X, scale_Y, scale_Z;

	// enum for mode
	std::string Mode[7] = {"Rotate View", "Translate View", "Perspective", "Rotate Model", "Translate Model", "Scale Model", "Viewport"};
	int interactionMode;

	// variables for mouse data
	double mouse_prev_x;
	double mouse_prev_y;
	bool mouseReseted;
	bool mouse_left_pressed;
	bool mouse_mid_pressed;
	bool mouse_right_pressed;

	GLfloat angleBase;
	GLfloat numBase = 50.0f;
	// A2 functions:

	// handlers:
	void CubeHandler(); // helper functin that process the cube data and draw the cube
	void GnomonHandler(); // helper function that process model frame info and draw the axis
	void mouseMoveEventHandler(double xPos, double yPos);
	void drawPerspectiveLine(glm::vec4 point1, glm::vec4 point2, glm::vec3 color);

	// mode handler
	void rotateViewHandler(double offset, int axis);
	void translateViewHandler(double offset, int axis);
	void perspectiveHanlder(double offset, int type);
	void rotateModelHandler(double offset, int axis);
	void translateModelHandler(double offset, int axis);
	void scaleModelHandler(double offset, int axis);
	void viewPortHandler(double xPos, double yPos, int id);


	// calculation helper functions
	bool ClipAgainstViewVolumn(std::pair<glm::vec2, glm::vec2 > &input2DPair);
	void sortTwoPoints(glm::vec2 &P1, glm::vec2 &P2, int base);
	std::pair<glm::vec4, glm::vec4 > clipBeforePerspective(glm::vec4 &P1, glm::vec4 &P2);
	glm::mat4 calculateView();

	// reset helper
	void reset();
	void resetFOV();
	void resetVP();
	void resetMouseLocation();

};
