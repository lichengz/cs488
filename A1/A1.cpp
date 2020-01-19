// Winter 2020

#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <sys/types.h>
#include <unistd.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

const float PI = 3.14159265f;
static const size_t DIM = 16;
Maze m(DIM);	

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 )
	
{
	initColour();
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::initColour()
{
	current_col = 0;
	colours[0] = 0.0f;
	colours[1] = 0.6f;
	colours[2] = 0.5f;
	cube_colour[0] = 0.0f;
	cube_colour[1] = 0.6f;
	cube_colour[2] = 0.5f;
	floor_colour[0] = 1.0f;
	floor_colour[1] = 1.0f;
	floor_colour[2] = 1.0f;
	avatar_colour[0] = 0.0f;
	avatar_colour[1] = 1.0f;
	avatar_colour[2] = 1.0f;
}

void A1::dig()
{
	m.digMaze();
	avatar_translation = mat4();
	avatar_translation = glm::translate( avatar_translation, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
	for(int i = 0; i < DIM; i++) {
		if(m.getValue(DIM - 1, i) == 0) {
			avatar_translation = glm::translate(avatar_translation, glm::vec3(i, 0, DIM - 1));
			avatar_pos = glm::vec2(i, DIM - 1);
			break;
		}
	}
}

void A1::init()
{
	// Initialize random number generator
	int rseed=getpid();
	srandom(rseed);
	// Print random number seed in case we want to rerun with
	// same random numbers
	cout << "Random number seed = " << rseed << endl;
	

	// DELETE FROM HERE...
	//m.digMaze();
	m.printMaze();
	// ...TO HERE
	
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the cube shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the cube uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );


	initGrid();	
	initCube();
	initAvatar();

	// init avatar
	// Build the avatar shader
	// TO DO
	// Set up the avatar uniforms
	// TO DO


	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, 2.*float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::initCube()
{
	float verts[] = {
		// x-y plane 1
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		//1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		//0.0f, 0.0f, 0.0f,

		// x-z plane 2
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		//1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		//0.0f, 0.0f, 1.0f,

		// y-z plane 1
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		//0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		//0.0f, 1.0f, 1.0f,

		// y-z plane 2
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		//1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,
		//1.0f, 1.0f, 1.0f,

		// x-z plane 1
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,
		//1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		//0.0f, 0.0f, 0.0f,

		// x-z plane 2
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,
		//1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		//0.0f, 1.0f, 0.0f
	};

	int indices[] = {
		0, 1, 2, // first triangle
		0, 2, 3, // second triangle
		4, 5, 6, 
		4, 6, 7,  
		8, 9, 10, 
		8, 10, 11, 
		12, 13, 14, 
		12, 14, 15,  
		16, 17, 18, 
		16, 18, 19,  
		20, 21, 22, 
		20, 22, 23  
	};
	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &cube_vao );
	glBindVertexArray( cube_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(verts),
		verts, GL_STATIC_DRAW );

	// Create the cube element buffer
	glGenBuffers( 1, &cube_ebo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cube_ebo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
		indices, GL_STATIC_DRAW );

	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if( ImGui::Button( "Reset" ) ) {
			reset();
		}
		if( ImGui::Button( "Dig" ) ) {
			dig();
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colours );
		//ImGui::SameLine();
		if( ImGui::RadioButton( "Cubes", &current_col, 0 ) ) {
			// Select this colour.
			colours[0] = cube_colour[0];
			colours[1] = cube_colour[1];
			colours[2] = cube_colour[2];
		}
		ImGui::PopID();
		
		ImGui::PushID( 1 );
		ImGui::SameLine();
		if( ImGui::RadioButton( "Floor", &current_col, 1 ) ) {
			// Select this colour.
			colours[0] = floor_colour[0];
			colours[1] = floor_colour[1];
			colours[2] = floor_colour[2];
		}
		ImGui::PopID();

		ImGui::PushID( 2 );
		ImGui::SameLine();
		if( ImGui::RadioButton( "Avatar", &current_col, 2 ) ) {
			// Select this colour.
			colours[0] = avatar_colour[0];
			colours[1] = avatar_colour[1];
			colours[2] = avatar_colour[2];
		}
		ImGui::PopID();


		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}


		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

void A1::reset()
{
	// reset rotation
	dragging_factor = 0;
	world_rotation = mat4();
	// reset maze height
	maze_height = 1;
	// reset avatar position
	avatar_translation = mat4();
	avatar_pos = glm::vec2(float(DIM)/2.0f, float(DIM)/2.0f);
	// reset colour
	initColour();
	// reset scaling
	scale = 1.0f;
	// clear maze
	m.reset();
}

void A1::initAvatar()
{
	avatar_pos = glm::vec2(float(DIM)/2.0f, float(DIM)/2.0f);
	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &avatar_vao );
	glBindVertexArray( avatar_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &avatar_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, avatar_vbo );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );

	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	CHECK_GL_ERRORS;

	float R = 0.5f;
	//int stack_amt = 20;
	float stack_angle = (float) (M_PI / stack_amt);
	//int sector_amt = 20;
	float slice_angle = (float) ( 2 *M_PI / sector_amt);

	float x0, x1, x2, x3, y1, y2, y3, y0, z1, z2, z3, z0; 
	float alpha_x = 0.0f;
	float alpha_z = 0.0f;
	
	GLfloat *coordsList = new GLfloat[stack_amt*sector_amt*18];
	//GLfloat coordsList[stack_amt*sector_amt*18];

	for( int i = 0;i < stack_amt;i++ ){

		alpha_z = (float) (i * stack_angle);

		float rsinz = R*sin(alpha_z);
		float rsinznext = R * sin(alpha_z + stack_angle);
		float z0 = R * cos(alpha_z);

		for( int j = 0;j < sector_amt; j++ ){
			alpha_x = j * slice_angle;

			x0 = rsinz * cos(alpha_x);
			x1 = rsinznext * cos(alpha_x);
			x2 = rsinznext * cos(alpha_x + slice_angle);
			x3 = rsinz * cos(alpha_x + slice_angle);


			y0 = rsinz * sin(alpha_x);
			y1 = rsinznext * sin(alpha_x);
			y2 = rsinznext * sin(alpha_x + slice_angle);
			y3 = rsinz * sin(alpha_x + slice_angle);

			z0 = z0;
			z1 = R * cos(alpha_z + stack_angle);
			z2 = z1;
			z3 = z0;

			coordsList[i*sector_amt*18 + j*18] = x0;
			coordsList[i*sector_amt*18 + j*18 + 1] = y0;
			coordsList[i*sector_amt*18 + j*18 + 2] = z0;
			coordsList[i*sector_amt*18 + j*18 + 3] = x1;
			coordsList[i*sector_amt*18 + j*18 + 4] = y1;
			coordsList[i*sector_amt*18 + j*18 + 5] = z1;
			coordsList[i*sector_amt*18 + j*18 + 6] = x2;
			coordsList[i*sector_amt*18 + j*18 + 7] = y2;
			coordsList[i*sector_amt*18 + j*18 + 8] = z2;
			coordsList[i*sector_amt*18 + j*18 + 9] = x2;
			coordsList[i*sector_amt*18 + j*18 + 10] = y2;
			coordsList[i*sector_amt*18 + j*18 + 11] = z2;
			coordsList[i*sector_amt*18 + j*18 + 12] = x3;
			coordsList[i*sector_amt*18 + j*18 + 13] = y3;
			coordsList[i*sector_amt*18 + j*18 + 14] = z3;
			coordsList[i*sector_amt*18 + j*18 + 15] = x0;
			coordsList[i*sector_amt*18 + j*18 + 16] = y0;
			coordsList[i*sector_amt*18 + j*18 + 17] = z0;
		}		
	}
	glBufferData( GL_ARRAY_BUFFER, stack_amt*sector_amt*18*sizeof(GLfloat), coordsList, GL_STATIC_DRAW );
	glBindVertexArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	delete [] coordsList;
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W = world_rotation;
	W = glm::scale(W, vec3(scale, scale, scale));
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
	world_rotation = glm::rotate(world_rotation, dragging_factor, glm::vec3(0.0f, 1.0f, 0.0f));

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes
		// Highlight the active square.
		glBindVertexArray( 0 );
		glBindVertexArray( cube_vbo );
		if(current_col == 0) {
			glUniform3f( col_uni, colours[0], colours[1], colours[2] );
			cube_colour[0] = colours[0];
			cube_colour[1] = colours[1];
			cube_colour[2] = colours[2];
		}else{
			glUniform3f( col_uni, cube_colour[0], cube_colour[1], cube_colour[2] );
		}
		for(int i = 0 ; i < DIM; i++) {
			for(int j = 0; j < DIM; j++) {
				if(m.getValue(i, j) == 1) {
					for(int h = 0; h < maze_height; h++){
						W = world_rotation;
						W = glm::scale(W, vec3(scale, scale, scale));
						W = glm::translate(W, glm::vec3(-float(DIM) / 2.0f, 0, -float(DIM) / 2.0f));
						W = glm::translate(W, glm::vec3(j, h, i));
						glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
						glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0 );
					}
				}
			}
		}

		// Draw the floor
		if(current_col == 1) {
			glUniform3f( col_uni, colours[0], colours[1], colours[2] );
			floor_colour[0] = colours[0];
			floor_colour[1] = colours[1];
			floor_colour[2] = colours[2];
		}else{
			glUniform3f( col_uni, floor_colour[0], floor_colour[1], floor_colour[2] );
		}
		for(int i = 0 ; i < DIM + 2; i++) {
			for(int j = 0; j < DIM + 2; j++) {
				W = world_rotation;
				W = glm::scale(W, vec3(scale, scale, scale));
				W = glm::translate(W, glm::vec3(-float(DIM) / 2.0f - 1.0f, 0, -float(DIM) / 2.0f - 1.0f));
				W = glm::translate(W, glm::vec3(j, -1, i));
				glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
				glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0 );
			}
		}

		// Draw the avatar
		if(current_col == 2) {
			glUniform3f( col_uni, colours[0], colours[1], colours[2] );
			avatar_colour[0] = colours[0];
			avatar_colour[1] = colours[1];
			avatar_colour[2] = colours[2];
		}else{
			glUniform3f( col_uni, avatar_colour[0], avatar_colour[1], avatar_colour[2] );
		}
		W = world_rotation;
		W = glm::scale(W, vec3(scale, scale, scale));
		W = glm::translate(W, vec3(0.5f, 0.5f, 0.5f));
		W =  W * avatar_translation;
		
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
		glBindVertexArray(avatar_vao);
		CHECK_GL_ERRORS;
		glDrawArrays(GL_TRIANGLES, 0, 6*stack_amt*sector_amt);
			

		glBindVertexArray( 0 );

	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		if (mouse_left_clicked) {
			float xDiff = xPos - mouse_prev_xPos;
			dragging_factor = abs(xDiff) > dragging_threshold ? xDiff / m_windowWidth * PI : 0;
		} 
		mouse_prev_xPos = xPos;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (actions == GLFW_PRESS) {
				mouse_left_clicked = true;
			} else if (actions == GLFW_RELEASE) {
				mouse_left_clicked = false;
			}
		}
	}
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
	scale = glm::clamp(scale + float(yOffSet) / 10.0f, 0.5f, 3.0f);

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		shiftPressed = true;
	}else if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
		shiftPressed = false;
	}
	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.

		// Avatar control
		if (key == GLFW_KEY_UP) {
			if(avatar_pos.y == 0) return false;
			if(shiftPressed) {
				m.setValue(avatar_pos.y - 1, avatar_pos.x, 0);
			}
			if(m.getValue(avatar_pos.y - 1, avatar_pos.x) == 0) {
				avatar_translation = glm::translate(avatar_translation, glm::vec3(0, 0, -1));
				avatar_pos = glm::vec2(avatar_pos.x, avatar_pos.y - 1);
			}
		}
		if (key == GLFW_KEY_DOWN) {
			if(avatar_pos.y == DIM - 1) return false;
			if(shiftPressed) {
				m.setValue(avatar_pos.y + 1, avatar_pos.x, 0);
			}
			if(m.getValue(avatar_pos.y + 1, avatar_pos.x) == 0) {
				avatar_translation = glm::translate(avatar_translation, glm::vec3(0, 0, 1));
				avatar_pos = glm::vec2(avatar_pos.x, avatar_pos.y + 1);
			}
		}
		if (key == GLFW_KEY_LEFT) {
			if(avatar_pos.x == 0) return false;
			if(shiftPressed) {
				m.setValue(avatar_pos.y, avatar_pos.x - 1, 0);
			}
			if(m.getValue(avatar_pos.y, avatar_pos.x - 1) == 0) {
				avatar_translation = glm::translate(avatar_translation, glm::vec3(-1, 0, 0));
				avatar_pos = glm::vec2(avatar_pos.x - 1, avatar_pos.y);
			}
		}
		if (key == GLFW_KEY_RIGHT) {
			if(avatar_pos.x == DIM - 1) return false;
			if(shiftPressed) {
				m.setValue(avatar_pos.y, avatar_pos.x + 1, 0);
			}
			if(m.getValue(avatar_pos.y, avatar_pos.x + 1) == 0) {
				avatar_translation = glm::translate(avatar_translation, glm::vec3(1, 0, 0));
				avatar_pos = glm::vec2(avatar_pos.x + 1, avatar_pos.y);
			}
		}

		// Wall height control
		if (key == GLFW_KEY_SPACE) {
			maze_height++;
		}
		if (key == GLFW_KEY_BACKSPACE) {
			if(maze_height > 1) maze_height--;
		}

		// Reset
		if (key == GLFW_KEY_R) {
			reset();
		}

		// Dig
		if (key == GLFW_KEY_D) {
			dig();
		}
	}

	return eventHandled;
}