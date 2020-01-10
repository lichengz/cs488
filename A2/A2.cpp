// Spring 2019

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
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		
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
		tmpLine.first = cube_vec4_VCS[firstIndex];
		tmpLine.second = cube_vec4_VCS[secondIndex];
		
		/* compute half width of frustum: left=-width, right=width */
		/* fovy is radian, and aspectRatio=w/h  */
		// width = near x tan(fovy/2) x aspectRatio;

		/* compute n / r */
		// matrix00 = near / width
        // = near / (near x tan(fovy/2) x aspectRatio)
        // = 1 / (tan(fovy/2) x aspectRatio);
		pair<glm::vec2, glm::vec2 > perspectiveLine;
		perspectiveLine.first.x = (tmpLine.first.x/tmpLine.first.z)/(tan(fov/2.0f/180.0f*M_PI));
		perspectiveLine.second.x = (tmpLine.second.x/tmpLine.second.z)/(tan(fov/2.0f/180.0f*M_PI));
		perspectiveLine.first.y = (tmpLine.first.y/tmpLine.first.z)/(tan(fov/2.0f/180.0f*M_PI));
		perspectiveLine.second.y = (tmpLine.second.y/tmpLine.second.z)/(tan(fov/2.0f/180.0f*M_PI));
		
		setLineColour(vec3(0.8f, 1.0f, 1.0f));
		drawLine(perspectiveLine.first, perspectiveLine.second);
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
	
	drawPerspectiveLine(center, forward, modelFrame_color[2]);
	drawPerspectiveLine(center, left, modelFrame_color[0]);
	drawPerspectiveLine(center, up, modelFrame_color[1]);

	// World Gnomon 
	drawPerspectiveLine(viewTransfer * base_0, viewTransfer * base_z, worldFrame_color[2]);
	drawPerspectiveLine(viewTransfer * base_0, viewTransfer * base_x, worldFrame_color[0]);
	drawPerspectiveLine(viewTransfer * base_0, viewTransfer * base_y, worldFrame_color[1]);
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
	setLineColour(color);
	drawLine(perspectiveLine.first, perspectiveLine.second);
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

	// reset base for model

	model_base_x_i = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	model_base_y_i = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	model_base_z_i = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);


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
	vp1 = glm::vec2(-0.9f, 0.9f);
	vp2 = glm::vec2( 0.9f, -0.9f);
}

void A2::resetMouseLocation(){

}

//----------------------------------------------------------------------------------------

/*
 *  trivial clipping test return 0 for remove, 1 for keep, -1 for reverse keep.
 */
int A2::easyClipping(glm::vec4 *cube_vec4_VCS, std::pair<int, int> *indexPair, int index){

}

//----------------------------------------------------------------------------------------
/*
 *  view volume clipping and transfer to view point helper function.
 *  input pair 
 */
bool A2::clipAndTtoViewPoint(pair<glm::vec2, glm::vec2 > &input2DPair){

}

/*
 *  helper function to sort two points base on x(base = 0) or y (base = 1); 
 */
void A2::sortTwoPoints(glm::vec2 &P1, glm::vec2 &P2, int base){

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
		
	}
}

//----------------------------------------------------------------------------------------
// mode handlers

void A2::rotateViewHandler(double offset, int axis){
	
}
void A2::translateViewHandler(double offset, int axis){


}
void A2::perspectiveHanlder(double offset, int type){

}
void A2::rotateModelHandler(double offset, int axis){
	GLfloat r = offset/angleBase; // rotation angle
	glm::vec3 a;
	switch(axis){
		case 0:  // x axis
			a = glm::vec3(1.0f, 0.0f, 0.0f);
			break;
		case 1: // y axis
			a = glm::vec3(0.0f, 1.0f, 0.0f);
			break;
		case 2: // z axis
			a = glm::vec3(0.0f, 0.0f, 1.0f);
			break;
	}

	modelTransfer = glm::mat4(
		glm::vec4(a.x * a.x * (1 - cos(r)) + cos(r),       a.x * a.y * (1 - cos(r)) + a.z * sin(r),  a.x * a.z * (1 - cos(r)) - a.y * sin(r), 0.0f),
		glm::vec4(a.x * a.y * (1 - cos(r)) - a.z * sin(r), a.y * a.y * (1 - cos(r)) + cos(r),        a.y * a.z * (1 - cos(r)) + a.x * sin(r), 0.0f),
		glm::vec4(a.x * a.z * (1 - cos(r)) + a.y * sin(r), a.y * a.z * (1 - cos(r)) - a.x * sin(r),  a.z * a.z * (1 - cos(r)) + cos(r),       0.0f),
		glm::vec4(0.0f,                                    0.0f,                                     0.0f,                                    1.0f)
	) * modelTransfer;
}
void A2::translateModelHandler(double offset, int axis){

}

void A2::scaleModelHandler(double offset, int axis){
	
}

void A2::viewPortHandler(double xPos, double yPos, int id){
	
}

//----------------------------------------------------------------------------------------

