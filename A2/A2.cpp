// Winter 2020

#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <utility>
#include <math.h>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

pair<int, int> cubeIndexPair[12] = {
	{0, 1},
	{0, 3},
	{0, 4},
	{1, 2},
	{1, 5},
	{2, 3},
	{2, 6},
	{3, 7},
	{4, 5},
	{4, 7},
	{5, 6},
	{6, 7},
};

pair<int, int> axisIndexPair[4] = {
	{0, 6}, //center
	{1, 6}, //forward
	{3, 6}, //left
	{4, 6} //up
};

static const glm::vec3 cube_vertex[8]=
{
	//bot x y z
	glm::vec3(-1.0f,  -1.0f, -1.0f), //0
	glm::vec3(-1.0f,  -1.0f,  1.0f), //1
	glm::vec3( 1.0f,  -1.0f,  1.0f), //2
	glm::vec3( 1.0f,  -1.0f, -1.0f), //3
	//top
	glm::vec3(-1.0f,  1.0f,  -1.0f), //4
	glm::vec3(-1.0f,  1.0f,   1.0f), //5
	glm::vec3( 1.0f,  1.0f,   1.0f), //6
	glm::vec3( 1.0f,  1.0f,  -1.0f), //7
};


//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{


}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	angleBase = m_windowWidth / 3.14159265f;
	reset();
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();



}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//---------------------------------------------------------------------------------------- Winter 2019
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & V0,   // Line Start (NDC coordinate)
		const glm::vec2 & V1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	// Draw the viewport
	setLineColour(vec3(0.6f, 0.9f, 0.4f));
	drawLine(topLeftVP, vec2(topLeftVP.x, botRightVP.y));//left
	drawLine(topLeftVP, vec2(botRightVP.x, topLeftVP.y));//top
	drawLine(botRightVP, vec2(topLeftVP.x, botRightVP.y));//bot
	drawLine(botRightVP, vec2(botRightVP.x, topLeftVP.y));//right

	// execute pipeline handler draw the cube
	CubeHandler();
	GnomonHandler();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...

		for (int i = 0; i < 7; i++) {
            ImGui::PushID(i);
            if (ImGui::RadioButton(Mode[i].c_str(), &interactionMode, i)) {

            }
		    ImGui::PopID();
        }


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button( "Reset Application" ) ) {
			reset();
		}

		ImGui::Text("Near Plane: %.1f", nearPlane);
		ImGui::Text("Far Plane: %.1f", farPlane);
		ImGui::Text("FOV: %.1f", fov);
		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {

		if(mouseReseted){
			mouseReseted = false;
			mouse_prev_x = xPos;
			mouse_prev_y = yPos;
		}
		if(mouse_left_pressed||mouse_mid_pressed||mouse_right_pressed){
			mouseMoveEventHandler(xPos, yPos);
			eventHandled = true;
		}
		mouse_prev_x = xPos;
		mouse_prev_y = yPos;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {

		if (button == GLFW_MOUSE_BUTTON_LEFT){
			if (actions == GLFW_PRESS) {
				if( interactionMode == 6 ){ // set y2
					viewPortHandler(mouse_prev_x, mouse_prev_y,1);
					viewPortHandler(mouse_prev_x, mouse_prev_y,2);
				}
				if(!mouse_left_pressed && !mouse_mid_pressed && !mouse_right_pressed){
					resetMouseLocation();
				}
				mouse_left_pressed = true;

			}

			if (actions == GLFW_RELEASE) {
				mouse_left_pressed = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_MIDDLE){
			if (actions == GLFW_PRESS) {
				if(!mouse_left_pressed && !mouse_mid_pressed && !mouse_right_pressed){
					resetMouseLocation();
				}
				mouse_mid_pressed = true;

			}

			if (actions == GLFW_RELEASE) {
				mouse_mid_pressed = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT){
			if (actions == GLFW_PRESS) {
				if(!mouse_left_pressed && !mouse_mid_pressed && !mouse_right_pressed){
					resetMouseLocation();
				}
				mouse_right_pressed = true;

			}

			if (actions == GLFW_RELEASE) {
				mouse_right_pressed = false;
			}
		}


	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(true);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Reset
		if (key == GLFW_KEY_A) {
			reset();
		}
		// Quit
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		// Rotate View (O)
		if (key == GLFW_KEY_O) {
			interactionMode = 0;
		}
		// Translate View (E)
		if (key == GLFW_KEY_E) {
			interactionMode = 1;
		}
		// Perspective (P)
		if (key == GLFW_KEY_P) {
			interactionMode = 2;
		}
		// Rotate Model (R)
		if (key == GLFW_KEY_R) {
			interactionMode = 3;
		}
		// Translate Model (T)
		if (key == GLFW_KEY_T) {
			interactionMode = 4;
		}
		// Scale Model (S)
		if (key == GLFW_KEY_S) {
			interactionMode = 5;
		}
		// Viewport (V)
		if (key == GLFW_KEY_V) {
			interactionMode = 6;
		}
	}


	return eventHandled;
}


//----------------------------------------------------------------------------------------
/*
 * Pipeline handler, process the cube info and update vertexs ready for draw.
 */

void A2::CubeHandler(){
	// Scaling
	glm::vec3 cube_vertex_scaled[8];
	for(int i = 0; i < 8; i++){
		cube_vertex_scaled[i].x = cube_vertex[i].x * scale_X;
		cube_vertex_scaled[i].y = cube_vertex[i].y * scale_Y;
		cube_vertex_scaled[i].z = cube_vertex[i].z * scale_Z;
	}
	// Convert to vec4 for mat4 multiplication
	glm::vec4 cube_vec4_temp[8];
	for( int i = 0 ; i < 8 ; i++){
		cube_vec4_temp[i] = vec4(cube_vertex_scaled[i], 1.0f);
	}
	// WCS
	glm::vec4 cube_vec4_WCS[8];
	for( int i = 0 ; i < 8 ; i++){
		cube_vec4_WCS[i] = modelTransfer * cube_vec4_temp[i];
	};
	// VCS
	glm::vec4 cube_vec4_VCS[8];
	for( int i = 0 ; i < 8 ; i++){
		cube_vec4_VCS[i] = viewTransfer * cube_vec4_WCS[i];
		VCS_cube_vertex[i] = cube_vec4_VCS[i];
	};

	int easyClipFlag[12] = {0};
	for(int i = 0 ; i < 12; i++){
		easyClipFlag[i] = 1;
	}

	for (int i = 0;  i < 12 ; i++){

		if(easyClipFlag[i] == 0){
			continue;
		}

		pair<glm::vec4, glm::vec4 > tmpLine;
		int firstIndex = cubeIndexPair[i].first;
		int secondIndex = cubeIndexPair[i].second;
		tmpLine.first = clipBeforePerspective(cube_vec4_VCS[firstIndex], cube_vec4_VCS[secondIndex]).first;
		tmpLine.second = clipBeforePerspective(cube_vec4_VCS[firstIndex], cube_vec4_VCS[secondIndex]).second;
		// tmpLine.first = cube_vec4_VCS[firstIndex];
		// tmpLine.second = cube_vec4_VCS[secondIndex];

		/* compute half width of frustum: left=-width, right=width */
		/* fovy is radian, and aspectRatio=w/h  */
		// width = near x tan(fovy/2) x aspectRatio;

		/* compute n / r */
		// matrix00 = near / width
        // = near / (near x tan(fovy/2) x aspectRatio)
        // = 1 / (tan(fovy/2) x aspectRatio);
		drawPerspectiveLine(tmpLine.first, tmpLine.second, vec3(0.8f, 1.0f, 1.0f));
	}


}

//----------------------------------------------------------------------------------------
/*
 * Frame handler, process the frame info and update vertexs ready for draw.
 */
void A2::GnomonHandler(){
	// method 1: just take the center point of vertices in perspective view

	// glm::vec2 center = (perspective_cube_vertex_2D[axisIndexPair[0].first] + perspective_cube_vertex_2D[axisIndexPair[0].second]) * 0.5f;
	// glm::vec2 forward = (perspective_cube_vertex_2D[axisIndexPair[1].first] + perspective_cube_vertex_2D[axisIndexPair[1].second]) * 0.5f;
	// glm::vec2 left = (perspective_cube_vertex_2D[axisIndexPair[2].first] + perspective_cube_vertex_2D[axisIndexPair[2].second]) * 0.5f;
	// glm::vec2 up = (perspective_cube_vertex_2D[axisIndexPair[3].first] + perspective_cube_vertex_2D[axisIndexPair[3].second]) * 0.5f;
	// drawLine(center, forward);
	// drawLine(center, left);
	// drawLine(center, up);

	// method 2:

	// Cube Gnomon
	glm::vec4 center = (VCS_cube_vertex[axisIndexPair[0].first] + VCS_cube_vertex[axisIndexPair[0].second]) * 0.5f;
	glm::vec4 forward = (VCS_cube_vertex[axisIndexPair[1].first] + VCS_cube_vertex[axisIndexPair[1].second]) * 0.5f;
	glm::vec4 left = (VCS_cube_vertex[axisIndexPair[2].first] + VCS_cube_vertex[axisIndexPair[2].second]) * 0.5f;
	glm::vec4 up = (VCS_cube_vertex[axisIndexPair[3].first] + VCS_cube_vertex[axisIndexPair[3].second]) * 0.5f;

	drawPerspectiveLine(clipBeforePerspective(center, forward).first, clipBeforePerspective(center, forward).second, modelFrame_color[2]);
	drawPerspectiveLine(clipBeforePerspective(center, left).first, clipBeforePerspective(center, left).second, modelFrame_color[0]);
	drawPerspectiveLine(clipBeforePerspective(center, up).first, clipBeforePerspective(center, up).second, modelFrame_color[1]);

	// World Gnomon
	drawPerspectiveLine(viewTransfer * world_base_0, viewTransfer * world_base_z, worldFrame_color[2]);
	drawPerspectiveLine(viewTransfer * world_base_0, viewTransfer * world_base_x, worldFrame_color[0]);
	drawPerspectiveLine(viewTransfer * world_base_0, viewTransfer * world_base_y, worldFrame_color[1]);
}

//----------------------------------------------------------------------------------------
/*
 * draw a line in perspective view, given two vec4 points
 */
void A2::drawPerspectiveLine(glm::vec4 point1, glm::vec4 point2, glm::vec3 color){
	pair<glm::vec2, glm::vec2 > perspectiveLine;
	perspectiveLine.first.x = (point1.x/point1.z)/(tan(fov/2.0f/180.0f*M_PI));
	perspectiveLine.second.x = (point2.x/point2.z)/(tan(fov/2.0f/180.0f*M_PI));
	perspectiveLine.first.y = (point1.y/point1.z)/(tan(fov/2.0f/180.0f*M_PI));
	perspectiveLine.second.y = (point2.y/point2.z)/(tan(fov/2.0f/180.0f*M_PI));
	if(ClipAgainstViewVolumn(perspectiveLine)){
		setLineColour(color);
		drawLine(perspectiveLine.first, perspectiveLine.second);
	}
}

//----------------------------------------------------------------------------------------

/*
 *  reset function and its helper functions.
 */

void A2::reset(){
	//reset selection
	interactionMode = 3;
	mouse_left_pressed = false;
	mouse_mid_pressed = false;
	mouse_right_pressed = false;

	// reset scale
	scale_X = 1.0f;
	scale_Y = 1.0f;
	scale_Z = 1.0f;

	// reset bases
	world_base_x = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	world_base_y = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	world_base_z = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	world_base_0 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	view_base_x = glm::vec4(-1.0f, 0.0f, 0.0f, 1.0f);
	view_base_y = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	view_base_z = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	view_base_0 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	model_base_x = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	model_base_y = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	model_base_z = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);


	eye_origin = glm::vec3(0.0f, 0.0f, 10.0f);
	world_origin = glm::vec3(0.0f, 0.0f, 0.0f);
	cube_origin = glm::vec3(0.0f, 0.0f, 0.0f);

	modelTransfer = glm::mat4(
					glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), // x
					glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), // y
					glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), // z
					glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // w

	viewTransfer = calculateView();

	resetFOV();
	resetVP();
}

void A2::resetFOV(){
	nearPlane = 5.0f;
	farPlane = 15.0f;
	fov = 30.0f;
}

void A2::resetVP(){
	topLeftVP = glm::vec2(-0.9f, 0.9f);
	botRightVP = glm::vec2( 0.9f, -0.9f);
}

void A2::resetMouseLocation(){

}

//----------------------------------------------------------------------------------------

/*
 *
 */
std::pair<glm::vec4, glm::vec4 > A2::clipBeforePerspective(glm::vec4 &P1, glm::vec4 &P2){
	// Trivial accept and reject
	if((P1.z < nearPlane && P2.z < nearPlane) || (P1.z > farPlane && P2.z > farPlane))
	{
		return {};
	}
	// clip against near and far planes
	glm::vec4 nearPoint = P1.z < P2.z ? P1 : P2;
	glm::vec4 farPoint = P1.z < P2.z ? P2 : P1;

	if(nearPoint.z < nearPlane){
		nearPoint = nearPoint + (farPoint - nearPoint) / (farPoint.z - nearPoint.z) * (nearPlane - nearPoint.z);
	}
	if(farPoint.z > farPlane){
		farPoint = farPoint - (farPoint - nearPoint) / (farPoint.z - nearPoint.z) * (farPoint.z - farPlane);
	}
	return {nearPoint, farPoint};
}

//----------------------------------------------------------------------------------------
/*
 *  view volume clipping and transfer to view point helper function.
 *  input pair
 */
bool A2::ClipAgainstViewVolumn(pair<glm::vec2, glm::vec2 > &vertex){
		vec2 P1 = vertex.first;
	vec2 P2 = vertex.second;

	GLfloat viewport_topleft_x, viewport_topleft_y, viewport_botright_x, viewport_botright_y;

	viewport_topleft_x = -1;
	viewport_topleft_y = 1;
	viewport_botright_x = 1;
	viewport_botright_y = -1;


	// first clip to -1 , 1
	// trivial reject
	if((P1.x < viewport_topleft_x && P2.x < viewport_topleft_x) || // all left
		(P1.x > viewport_botright_x && P2.x > viewport_botright_x) || // all right
		(P1.y > viewport_topleft_y && P2.y > viewport_topleft_y) || // all top
		(P1.y < viewport_botright_y && P2.y < viewport_botright_y)) // all bot
		{
			return false;
		}

	// Step 1. clip on x = viewport_topleft_x and viewport_botright_x
	sortTwoPoints(P1, P2, 0); // sort points base on x first

	// clip to viewport_topleft_x, check second only since P1.x > P2.x
	if(P2.x < viewport_topleft_x){ // clip required
			P2 = P2 + (P1 - P2) * (viewport_topleft_x - P2.x) / (P1.x - P2.x);
	}

	// clip to viewport_botright_x, check first only since P1.x > P2.x
	if(P1.x > viewport_botright_x){ // clip required
		P1 = P2 + (P1 - P2) * (viewport_botright_x - P2.x) / (P1.x - P2.x);
	}

	if((P1.x < viewport_topleft_x && P2.x < viewport_topleft_x) || // all left
		(P1.x > viewport_botright_x && P2.x > viewport_botright_x) || // all right
		(P1.y > viewport_topleft_y && P2.y > viewport_topleft_y) || // all top
		(P1.y < viewport_botright_y && P2.y < viewport_botright_y)) // all bot
		{
			return false;
		}


	// Step 2. clip on y =  viewport_topleft_y and viewport_botright_y
	sortTwoPoints(P1, P2, 1); // sort points base on y

	// clip to viewport_botright_y, check second only since P1.y > P2.y
	if(P2.y < viewport_botright_y){ // clip required
			P2 = P2 + (P1 - P2) * (viewport_botright_y - P2.y) / (P1.y - P2.y);
	}

	// clip to viewport_topleft_y, check first only since P1.y > P2.y
	if(P1.y > viewport_topleft_y){ // clip required
		P1 = P2 + (P1 - P2) * (viewport_topleft_y - P2.y) / (P1.y - P2.y);
	}



	if((P1.x < viewport_topleft_x && P2.x < viewport_topleft_x) || // all left
		(P1.x > viewport_botright_x && P2.x > viewport_botright_x) || // all right
		(P1.y > viewport_topleft_y && P2.y > viewport_topleft_y) || // all top
		(P1.y < viewport_botright_y && P2.y < viewport_botright_y)) // all bot
		{
			return false;
		}


	// map to view port
	viewport_topleft_x = std::min(topLeftVP.x, botRightVP.x);
	viewport_topleft_y = std::max(topLeftVP.y, botRightVP.y);
	viewport_botright_x = std::max(topLeftVP.x, botRightVP.x);
	viewport_botright_y = std::min(topLeftVP.y, botRightVP.y);



	// clip done ready to draw

	vertex.first.x = (P1.x + 1)/2.0f * (viewport_botright_x - viewport_topleft_x) + viewport_topleft_x;
	vertex.first.y = (P1.y + 1)/2.0f * (viewport_topleft_y - viewport_botright_y) + viewport_botright_y;
	vertex.second.x = (P2.x + 1)/2.0f * (viewport_botright_x - viewport_topleft_x) + viewport_topleft_x;
	vertex.second.y = (P2.y + 1)/2.0f * (viewport_topleft_y - viewport_botright_y) + viewport_botright_y;

	return true;
}

/*
 *  helper function to sort two points base on x(base = 0) or y (base = 1);
 */
void A2::sortTwoPoints(glm::vec2 &P1, glm::vec2 &P2, int base){
	if(base == 0){
		// sort on x let P1.x > P2.x
		if(P1.x > P2.x){
			return;
		}else{
			vec2 temp;
			temp.x = P1.x;
			temp.y = P1.y;
			P1.x = P2.x;
			P1.y = P2.y;
			P2.x = temp.x;
			P2.y = temp.y;
			return;
		}
	}else{
		// sort on y let P1.y > P2.y
		if(P1.y > P2.y){
			return;
		}else{
			vec2 temp;
			temp.x = P1.x;
			temp.y = P1.y;
			P1.x = P2.x;
			P1.y = P2.y;
			P2.x = temp.x;
			P2.y = temp.y;
			return;
		}
	}
}


//----------------------------------------------------------------------------------------

/*
 *  calculate worldtoview matrix follow uvn base
 */

glm::mat4 A2::calculateView(){
	glm::vec3 forward = glm::normalize(world_origin - eye_origin);
	glm::vec3 tmp_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 left = glm::normalize(glm::cross(tmp_up,forward));
	glm::vec3 up = glm::normalize(glm::cross(forward, left));

	glm::mat4 view = glm::mat4(
		glm::vec4(left.x, up.x, forward.x, 0.0f),
		glm::vec4(left.y, up.y, forward.y, 0.0f),
		glm::vec4(left.z, up.z, forward.z, 0.0f),
		glm::vec4(-glm::dot(eye_origin,left), -glm::dot(eye_origin,up), -glm::dot(eye_origin,forward), 1)
	);
	return view;
}

//----------------------------------------------------------------------------------------

// mouse movement handler
void A2::mouseMoveEventHandler(double xPos, double yPos){
	double offset = xPos - mouse_prev_x;
	switch(interactionMode){
		case 0: //Rotate view
			if(mouse_left_pressed){
				rotateViewHandler(offset, 0);
			}
			if(mouse_mid_pressed){
				rotateViewHandler(offset, 1);
			}
			if(mouse_right_pressed){
				rotateViewHandler(offset, 2);
			}
		break;
		case 1: //Translate view
			if(mouse_left_pressed){
				translateViewHandler(offset, 0);
			}
			if(mouse_mid_pressed){
				translateViewHandler(offset, 1);
			}
			if(mouse_right_pressed){
				translateViewHandler(offset, 2);
			}
		break;
		case 2: //Perspective
			if(mouse_left_pressed){
				perspectiveHanlder(offset, 0);
			}
			if(mouse_mid_pressed){
				perspectiveHanlder(offset, 1);
			}
			if(mouse_right_pressed){
				perspectiveHanlder(offset, 2);
			}
		break;
		case 3: //Rotate model
			if(mouse_left_pressed){
				rotateModelHandler(offset, 0);
			}
			if(mouse_mid_pressed){
				rotateModelHandler(offset, 1);
			}
			if(mouse_right_pressed){
				rotateModelHandler(offset, 2);
			}
		break;
		case 4: //Tranlation
		if(mouse_left_pressed){
				translateModelHandler(offset, 0);
			}
			if(mouse_mid_pressed){
				translateModelHandler(offset, 1);
			}
			if(mouse_right_pressed){
				translateModelHandler(offset, 2);
			}
		break;
		case 5: //Scaling
		if(mouse_left_pressed){
				scaleModelHandler(offset, 0);
			}
			if(mouse_mid_pressed){
				scaleModelHandler(offset, 1);
			}
			if(mouse_right_pressed){
				scaleModelHandler(offset, 2);
			}
		break;
		case 6: //Viewport
			if(mouse_left_pressed){
				viewPortHandler(xPos, yPos, 1);
			}
		break;
	}
}

//----------------------------------------------------------------------------------------
// mode handlers

void A2::rotateViewHandler(double offset, int axis){
	GLfloat r = offset/angleBase; // rotation angle
	glm::vec3 a;
	switch(axis){
		case 0:  // x axis
			a = glm::vec3(view_base_x);
			break;
		case 1: // y axis
			a = glm::vec3(view_base_y);
			break;
		case 2: // z axis
			a = glm::vec3(view_base_z);
			break;
	}

	// roate along model_base_axis
	glm::mat4 rotationMatrix = glm::mat4(
		glm::vec4(cos(r) + a.x*a.x*(1-cos(r)), a.y*a.x*(1-cos(r)) + a.z*sin(r), a.z*a.x*(1-cos(r)) - a.y*sin(r), 0.0f),
		glm::vec4(a.x*a.y*(1-cos(r)) - a.z*sin(r), cos(r) + a.y*a.y*(1-cos(r)), a.z*a.y*(1-cos(r)) + a.x*sin(r), 0.0f),
		glm::vec4(a.x*a.z*(1-cos(r)) + a.y*sin(r), a.y*a.z*(1-cos(r)) - a.x*sin(r), cos(r) + a.z*a.z*(1-cos(r)), 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	// Update cube model base
	view_base_x = rotationMatrix * view_base_x;
	view_base_y = rotationMatrix * view_base_y;
	view_base_z = rotationMatrix * view_base_z;

	// check if cube's centered at the origin. if not translate it to the origin, rotate, and then translate back.
	if(view_base_0.x == 0 && view_base_0.y == 0 && view_base_0.z == 0){
		viewTransfer = rotationMatrix * viewTransfer;
	}else{
		glm::mat4 translateMatrix1 = glm::mat4(
			glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(-view_base_0.x, -view_base_0.y, -view_base_0.z, 1.0f)
		);
		viewTransfer = translateMatrix1 * viewTransfer;
		viewTransfer = rotationMatrix * viewTransfer;
		glm::mat4 translateMatrix2 = glm::mat4(
			glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(view_base_0.x, view_base_0.y, view_base_0.z, 1.0f)
		);
		viewTransfer = translateMatrix2 * viewTransfer;
	}
}
void A2::translateViewHandler(double offset, int axis){
	GLfloat r = offset/angleBase; // rotation angle
	glm::vec3 a;
	switch(axis){
		case 0:  // x axis
			a = glm::vec3(view_base_x) * r;
			break;
		case 1: // y axis
			a = glm::vec3(view_base_y) * r;
			break;
		case 2: // z axis
			a = glm::vec3(view_base_z) * r;
			break;
	}

	// roate along model_base_axis
	glm::mat4 translateMatrix = glm::mat4(
		glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(a.x, a.y, a.z, 1.0f)
	);
	view_base_0 = translateMatrix * view_base_0;
	viewTransfer = translateMatrix * viewTransfer;
}
void A2::perspectiveHanlder(double offset, int type){
	switch(type){
		case 0: // fov change
			fov = std::max(std::min((fov + (float)offset), 160.0f), 5.0f);
			break;
		case 1: // change near plane
			nearPlane = std::max(std::min((nearPlane + (float)offset/numBase), farPlane), 0.0f);
			break;
		case 2: // change far plane
			farPlane = std::max(std::min((farPlane + (float)offset/numBase), 150.0f), nearPlane);
			break;
	}
}
void A2::rotateModelHandler(double offset, int axis){
	GLfloat r = offset/angleBase; // rotation angle
	glm::vec3 a;
	switch(axis){
		case 0:  // x axis
			a = glm::vec3(model_base_x);
			break;
		case 1: // y axis
			a = glm::vec3(model_base_y);
			break;
		case 2: // z axis
			a = glm::vec3(model_base_z);
			break;
	}

	// roate along model_base_axis
	glm::mat4 rotationMatrix = glm::mat4(
		glm::vec4(cos(r) + a.x*a.x*(1-cos(r)), a.y*a.x*(1-cos(r)) + a.z*sin(r), a.z*a.x*(1-cos(r)) - a.y*sin(r), 0.0f),
		glm::vec4(a.x*a.y*(1-cos(r)) - a.z*sin(r), cos(r) + a.y*a.y*(1-cos(r)), a.z*a.y*(1-cos(r)) + a.x*sin(r), 0.0f),
		glm::vec4(a.x*a.z*(1-cos(r)) + a.y*sin(r), a.y*a.z*(1-cos(r)) - a.x*sin(r), cos(r) + a.z*a.z*(1-cos(r)), 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	// Update cube model base
	model_base_x = rotationMatrix * model_base_x;
	model_base_y = rotationMatrix * model_base_y;
	model_base_z = rotationMatrix * model_base_z;

	// check if cube's centered at the origin. if not translate it to the origin, rotate, and then translate back.
	if(model_base_0.x == 0 && model_base_0.y == 0 && model_base_0.z == 0){
		modelTransfer = rotationMatrix * modelTransfer;
	}else{
		glm::mat4 translateMatrix1 = glm::mat4(
			glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(-model_base_0.x, -model_base_0.y, -model_base_0.z, 1.0f)
		);
		modelTransfer = translateMatrix1 * modelTransfer;
		modelTransfer = rotationMatrix * modelTransfer;
		glm::mat4 translateMatrix2 = glm::mat4(
			glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(model_base_0.x, model_base_0.y, model_base_0.z, 1.0f)
		);
		modelTransfer = translateMatrix2 * modelTransfer;
	}
}

void A2::translateModelHandler(double offset, int axis){
	GLfloat r = offset/angleBase; // rotation angle
	glm::vec3 a;
	switch(axis){
		case 0:  // x axis
			a = glm::vec3(model_base_x) * r;
			break;
		case 1: // y axis
			a = glm::vec3(model_base_y) * r;
			break;
		case 2: // z axis
			a = glm::vec3(model_base_z) * r;
			break;
	}
	glm::mat4 translateMatrix = glm::mat4(
		glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(a.x, a.y, a.z, 1.0f)
	);
	model_base_0 = translateMatrix * model_base_0;
	modelTransfer = translateMatrix * modelTransfer;
}

void A2::scaleModelHandler(double offset, int axis){
	offset = offset/m_windowWidth;
	switch(axis){
		case 0: // scale on X
			scale_X = std::max(std::min((scale_X + (float)offset), 2.0f), 0.1f);
			break;
		case 1: // scale on Y
			scale_Y = std::max(std::min((scale_Y + (float)offset), 2.0f), 0.1f);
			break;
		case 2: // scale on Z
			scale_Z = std::max(std::min((scale_Z + (float)offset), 2.0f), 0.1f);
			break;
	}
}

void A2::viewPortHandler(double xPos, double yPos, int id){
	GLfloat new_X = xPos*2.0f/m_windowWidth - 1.0f;
	GLfloat new_Y = (-yPos*2.0f/m_windowHeight) + 1.0f;
	new_X = std::max(std::min((new_X), 1.0f), -1.0f);
	new_Y = std::max(std::min((new_Y), 1.0f), -1.0f);
	switch(id){
		case 1: // update vp1
			topLeftVP.x = new_X;
			topLeftVP.y = new_Y;
			break;
		case 2: // update vp2
			botRightVP.x = new_X;
			botRightVP.y = new_Y;
			break;
	}
}

//----------------------------------------------------------------------------------------
