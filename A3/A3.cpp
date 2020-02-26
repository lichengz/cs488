// Winter 2020

#include "A3.hpp"
#include "scene_lua.hpp"
#include "trackball.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.85, 0.85, 0.85, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.

	resetAll();
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
	
	// Init variables
	root_ori = m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f);
	totalNode = m_rootNode->totalSceneNodes();
	for(int i = 0; i < totalNode; i++){
		SceneNode * node = findNodeById(*m_rootNode, i);
		if(node->m_name == "head"){
			head_id = i;
		}
		if(node->m_nodeType == NodeType::JointNode){
			jointIndex.push(i);
		}
	}
	// Convert stack into a vector
	while(!jointIndex.empty()) {
        jointIndexVector.push_back(jointIndex.top());
        jointIndex.pop();
    }
	// Joint Orientation Vector
	for(auto const& id: jointIndexVector) {
		SceneNode * node = findNodeById(*m_rootNode, id);
		JointNode * jointNode = static_cast<JointNode*>(node);
		ori_joint_angle.push_back(jointNode->m_joint_x.init);
	}
	ori_joint_angle.push_back(head_rotation);

}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(10.0f, 10.0f, 10.0f);
	m_light.rgbIntensity = vec3(1.0f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));

		location = m_shader.getUniformLocation("picking");
		glUniform1i(location, need_reRender ? 1 : 0);

		CHECK_GL_ERRORS;

		if(!need_reRender)
		{
			//-- Set LightSource uniform for the scene:
			{
				location = m_shader.getUniformLocation("light.position");
				glUniform3fv(location, 1, value_ptr(m_light.position));
				location = m_shader.getUniformLocation("light.rgbIntensity");
				glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
				CHECK_GL_ERRORS;
			}

			//-- Set background light ambient intensity
			{
				location = m_shader.getUniformLocation("ambientIntensity");
				vec3 ambientIntensity(0.25f);
				glUniform3fv(location, 1, value_ptr(ambientIntensity));
				CHECK_GL_ERRORS;
			}
		}
		
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...
		if (ImGui::BeginMenuBar()){
			// Application Menu
			if(ImGui::BeginMenu("Application")){
				if(ImGui::MenuItem("Reset Position (I)")) {
					resetHandler(0);
				}
				if(ImGui::MenuItem("Reset Orientation (O)")) {
					resetHandler(1);
				}
				if(ImGui::MenuItem("Reset Joints (S)")) {
					resetHandler(2);
				}
				if(ImGui::MenuItem("Reset All (A)")) {
					resetHandler(3);
				}
				if( ImGui::Button( "Quit (Q)" ) ) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				ImGui::EndMenu();
			}

			// Edit Menu
			if(ImGui::BeginMenu("Edit")){
				if(ImGui::MenuItem("Undo (U)")) {
					undo();
				}
				if(ImGui::MenuItem("Redo (R)")) {
					redo();
				}
				ImGui::EndMenu();
			}

			// Interaction Mode
			if(ImGui::BeginMenu("Options")){
				if(ImGui::Checkbox("Circle (C)", &circle)) {

				}
				if(ImGui::Checkbox("Z-buffer (Z)", &z_buffer)) {
			
				}
				if(ImGui::Checkbox("Backface culling (B)", &backface_culling)) {
			
				}
				if(ImGui::Checkbox("Frontface culling (F)", &frontface_culling)) {
			
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		

		// Interaction Mode
		if (ImGui::RadioButton("Position/Orientation (P)", &i_mode, 0)) {
                
        }

		if (ImGui::RadioButton("Joints (J)", &i_mode, 1)) {
                
        }

		if(!undo_succeed) {
			ImGui::OpenPopup("oops, undo stack is empty");
		}

		if(ImGui::BeginPopupModal("oops, undo stack is empty")) {
			ImGui::Text("Undo operation is invalid now");
			if(ImGui::Button("OK", ImVec2(100, 0))) {
				ImGui::CloseCurrentPopup();
				undo_succeed = true;
			}

			ImGui::EndPopup();
		}

		if(!redo_succeed) {
			ImGui::OpenPopup("oops, redo stack is empty");
		}

		if(ImGui::BeginPopupModal("oops, redo stack is empty")) {
			ImGui::Text("Redo operation is invalid now");
			if(ImGui::Button("OK", ImVec2(100, 0))) {
				ImGui::CloseCurrentPopup();
				redo_succeed = true;
			}

			ImGui::EndPopup();
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A3::updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;

		if(need_reRender){
			int id = node.m_nodeId;
			float r = float(id&0xff) / 255.0f;
			float g = float((id>>8)&0xff) / 255.0f;
			float b = float((id>>16)&0xff) / 255.0f;

			location = m_shader.getUniformLocation("material.kd");
			glUniform3f( location, r, g, b );
			CHECK_GL_ERRORS;
		}else{
			//-- Set Material values:
			location = shader.getUniformLocation("material.kd");
			
			vec3 kd = node.material.kd;
			if(node.isSelected){
				kd = vec3(0.19f, 0.82f, 0.55f);
			}
			glUniform3fv(location, 1, value_ptr(kd));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.ks");
			vec3 ks = node.material.ks;
			if(node.isSelected){
				ks = vec3(0.5f);
			}
			glUniform3fv(location, 1, value_ptr(ks));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.shininess");
			glUniform1f(location, node.material.shininess);
			CHECK_GL_ERRORS;
		}
		
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {
	if(z_buffer) glEnable( GL_DEPTH_TEST );
	if(backface_culling || frontface_culling){
		glEnable(GL_CULL_FACE);
		if(backface_culling && frontface_culling){
			glCullFace(GL_FRONT_AND_BACK);
		}else if(backface_culling){
			glCullFace(GL_BACK);
		}else if(frontface_culling){
			glCullFace(GL_FRONT);
		}
	}
	
	renderSceneGraph(*m_rootNode);


	if(z_buffer) glDisable( GL_DEPTH_TEST );
	renderArcCircle();
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	for (const SceneNode * node : root.children) {

		if (node->m_nodeType != NodeType::GeometryNode)
			continue;

		const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);

		updateShaderUniforms(m_shader, *geometryNode, m_view);


		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		//-- Now render the mesh:
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}
	for(const SceneNode * node : root.children){
		renderSceneGraph(*node);
	}
	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	if(!circle)return;
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
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
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		
		if (button == GLFW_MOUSE_BUTTON_LEFT){
			if (actions == GLFW_PRESS) {
				
				if(i_mode == 1) // when joint mode enbaled
				{
					// Do Picking
					double xpos, ypos;
					glfwGetCursorPos( m_window, &xpos, &ypos );

					need_reRender = true;

					uploadCommonSceneUniforms();
					glClearColor(1.0, 1.0, 1.0, 1.0 );
					glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
					glClearColor(0.85, 0.85, 0.85, 1.0);

					draw();

					xpos *= double(m_framebufferWidth) / double(m_windowWidth);
					ypos = m_windowHeight - ypos;
					ypos *= double(m_framebufferHeight) / double(m_windowHeight);
					GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
		
					glReadBuffer( GL_BACK );
					glReadPixels( int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
					CHECK_GL_ERRORS;
					// Reassemble the object ID.
					unsigned int what = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);

					findNodeBelowJoint(*m_rootNode, what);

					need_reRender = false;

					CHECK_GL_ERRORS;
				}
				
			}
			if(!mouse_left_pressed && !mouse_mid_pressed && !mouse_right_pressed){
				resetMouseLocation();
			}
			mouse_left_pressed = true;
	
			if (actions == GLFW_RELEASE) {
				mouse_left_pressed = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_MIDDLE){
			if (actions == GLFW_PRESS) {
				if(!mouse_left_pressed && !mouse_mid_pressed && !mouse_right_pressed){
					resetMouseLocation();
				}
				// if(i_mode == 1){ // for joint mode
				// 	std::vector<GLfloat> cur_joint_angle;
				// 	for(auto const& id: jointIndexVector) {
				// 		SceneNode * node = findNodeById(*m_rootNode, id);
				// 		JointNode * jointNode = static_cast<JointNode*>(node);
				// 		cur_joint_angle.push_back(jointNode->m_joint_x.init);
				// 	}
				// 	cur_joint_angle.push_back(head_rotation);
				// 	joint_rotation_undo.push(cur_joint_angle);

				// }
				mouse_mid_pressed = true;
				
			}
	
			if (actions == GLFW_RELEASE) {
				if(i_mode == 1){ // for joint mode
					std::vector<GLfloat> cur_joint_angle;
					for(auto const& id: jointIndexVector) {
						SceneNode * node = findNodeById(*m_rootNode, id);
						JointNode * jointNode = static_cast<JointNode*>(node);
						cur_joint_angle.push_back(jointNode->m_joint_x.init);
					}
					cur_joint_angle.push_back(head_rotation);
					joint_rotation_undo.push(cur_joint_angle);

				}
				mouse_mid_pressed = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT){
			if (actions == GLFW_PRESS) {
				if(!mouse_left_pressed && !mouse_mid_pressed && !mouse_right_pressed){
					resetMouseLocation();
				}
				if(i_mode == 1){ // for joint mode
					std::vector<GLfloat> cur_joint_angle;
					for(auto const& id: jointIndexVector) {
						SceneNode * node = findNodeById(*m_rootNode, id);
						JointNode * jointNode = static_cast<JointNode*>(node);
						cur_joint_angle.push_back(jointNode->m_joint_x.init);
					}
					cur_joint_angle.push_back(head_rotation);
					joint_rotation_undo.push(cur_joint_angle);

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
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
		// --------------Options--------------
		// Circle (C):
		if( key == GLFW_KEY_C ) {
			circle = !circle;
			eventHandled = true;
		}
		//Z-buffer (Z)
		if( key == GLFW_KEY_Z ) {
			z_buffer = !z_buffer;
			eventHandled = true;
		}
		//Backface culling (B)
		if( key == GLFW_KEY_B ) {
			backface_culling = !backface_culling;
			eventHandled = true;
		}
		//Frontface culling (F)
		if( key == GLFW_KEY_F ) {
			frontface_culling = !frontface_culling;
			eventHandled = true;
		}
		// --------------Application--------------
		//Reset Position (I)
		if( key == GLFW_KEY_I ) {
			resetHandler(0);
			eventHandled = true;
		}
		//Reset Orientation (O)
		if( key == GLFW_KEY_O ) {
			resetHandler(1);
			eventHandled = true;
		}
		//Reset Joints (S)
		if( key == GLFW_KEY_S ) {
			resetHandler(2);
			eventHandled = true;
		}
		//Reset All (A)
		if( key == GLFW_KEY_A ) {
			resetHandler(3);
			eventHandled = true;
		}
		//Quit (Q)
		if( key == GLFW_KEY_Q ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}
		// --------------Edit--------------	
		//Undo (U)			
		if( key == GLFW_KEY_U ) {
			undo();
			eventHandled = true;
		}
		//Redo (R)
		if( key == GLFW_KEY_R ) {
			redo();
			eventHandled = true;
		}
		// -------- Position & Joints --------
		//Position/Orientation (P)			
		if( key == GLFW_KEY_P ) {
			i_mode = 0;
			eventHandled = true;
		}
		//Joints (J)
		if( key == GLFW_KEY_J ) {
			i_mode = 1;
			eventHandled = true;
		}
	}
	


	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles reset events.
 */
void A3::resetHandler(int type){
	switch(type){
		case 0: // reset position
			resetPosition();
			break;
		case 1:
			resetOrietation();
			break;
		case 2:
			resetJoints();
			break;
		case 3:
			resetAll();
			break;
	}
}

void A3::resetPosition(){
	glm::mat4 resetPMatrix = glm::translate(mat4(), vec3(root_ori - m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f)));
	recursiveRotate(glm::mat4(), *m_rootNode, resetPMatrix);
}

void A3::resetOrietation(){
	glm::mat4 moveToOrigin = glm::translate(mat4(), -vec3(m_rootNode->trans*vec4(0.0f, 0.0f, 0.0f, 1.0f)));
	recursiveRotate(glm::mat4(), *m_rootNode, moveToOrigin);

	glm::mat4 resetOrientation = glm::inverse(m_rootNode->trans);
	recursiveRotate(glm::mat4(), *m_rootNode, resetOrientation);

	glm::mat4 moveBack = glm::inverse(moveToOrigin);
	recursiveRotate(glm::mat4(), *m_rootNode, moveBack);
}

void A3::resetJoints(){
	redo_succeed = true;
	undo_succeed = true;
	while(joint_rotation_undo.size() > 1){
		undo();
	}
	resetUndoRedo();
}

void A3::resetAll(){
	resetOrietation();
	resetPosition();
	resetJoints();
	resetVariables();
	resetMouseLocation();
}

void A3::resetVariables(){
	show_gui = true;
	i_mode = 0; 
	z_buffer = true; 
	circle = true;
	backface_culling = false;
	frontface_culling = false;
	selection = false;
	need_reRender = false;
	undo_succeed = true;
	redo_succeed = true;
	mouseReseted = false;
	mouse_left_pressed = false;
	mouse_mid_pressed = false;
	mouse_right_pressed = false;
}

void A3::resetUndoRedo(){
	while(joint_rotation_redo.size() > 0){
		joint_rotation_redo.pop();
	}
	while(joint_rotation_undo.size() > 0){
		joint_rotation_undo.pop();
	}
	joint_rotation_undo.push(ori_joint_angle);
}

void A3::rotateJoints(std::vector<GLfloat> angleVector){
	for(int i = 0; i < jointIndexVector.size(); i++){
		JointNode * node =  static_cast<JointNode*>(findNodeById(*m_rootNode, jointIndexVector[i]));
		GLfloat ang = angleVector[i] - node->m_joint_x.init;
		node->m_joint_x.init = angleVector[i];
		glm::mat4 rot = glm::rotate(mat4(), ang, vec3(1.0f, 0.0f, 0.0f));
		recursiveRotate(node->trans, *node, rot);
	}
	SceneNode * headnode = findNodeById(*m_rootNode, head_id);
	GLfloat prev_head_rotation = angleVector[jointIndexVector.size()];
	GLfloat reverseAngle = prev_head_rotation - head_rotation;

	head_rotation = prev_head_rotation;
	glm::mat4 y_rotateMatrix = glm::rotate(mat4(), reverseAngle, vec3(0.0f, 1.0f, 0.0f));
	recursiveRotate(headnode->trans, *headnode, y_rotateMatrix);
}

void A3::undo(){
	if(!redo_succeed) return;
	if(joint_rotation_undo.size() == 1){
		undo_succeed = false;
		return;
	} 
		
	std::vector<GLfloat> lastAngleVector = joint_rotation_undo.top();
	
	joint_rotation_undo.pop();
	while(lastAngleVector == joint_rotation_undo.top() && joint_rotation_undo.size() > 1){
		joint_rotation_undo.pop();
	}
	joint_rotation_redo.push(lastAngleVector);

	rotateJoints(joint_rotation_undo.top());
}

void A3::redo(){
	if(!undo_succeed) return;
	// check if any change has been made
	if(joint_rotation_redo.size() == 0){
		redo_succeed = false;
		return;
	}

	std::vector<GLfloat> lastAngleVector = joint_rotation_redo.top();
	joint_rotation_redo.pop();
	joint_rotation_undo.push(lastAngleVector);

	rotateJoints(lastAngleVector);
}

void A3::resetMouseLocation(){
	mouse_prev_x = 0.0f;
	mouse_prev_y = 0.0f;
	mouseReseted = true;
}

//----------------------------------------------------------------------------------------
/*
 * Mouse movenment event handler.
 */
void A3::mouseMoveEventHandler(double xPos, double yPos){
	double offsetX = xPos - mouse_prev_x;
	double offsetY = yPos - mouse_prev_y;
	switch (i_mode)
	{
	case 0: // rotate position/poientation
		if(mouse_left_pressed){
			P_OHandler(offsetX, offsetY, 0);
		}
		if(mouse_mid_pressed){
			P_OHandler(offsetX, offsetY, 1);
		}
		if(mouse_right_pressed){
			P_OHandler(xPos, yPos, 2);
		}
		break;

	case 1: // rotate joint
		if(mouse_left_pressed){
			
		}
		if(mouse_mid_pressed){
			rotateJointHandler(offsetX, offsetY, 1);
		}
		if(mouse_right_pressed){
			rotateJointHandler(offsetX, offsetY, 2);
		}
		break;

	}
}

//----------------------------------------------------------------------------------------
/*
 * Position/Orientation handler.
 */
void A3::P_OHandler(double offsetX, double offsetY, int axis){
	switch (axis){
		case 0: //left button move x/y
			m_rootNode->translate(vec3(offsetX/movementBase, -offsetY/movementBase, 0.0f));
			break;
		case 1:
			m_rootNode->translate(vec3(0.0f, 0.0f, offsetY/movementBase));
			break;
		case 2:
			trackballHandler(offsetX, offsetY);
			break;
			
	}
}

//----------------------------------------------------------------------------------------
/*
 * Trackball handler.
 */
void A3::trackballHandler(double xPos, double yPos){
	float diameter = (m_framebufferWidth < m_framebufferHeight) ? m_framebufferWidth * 0.5 : m_framebufferHeight * 0.5;
	float xCenter = m_framebufferWidth / 2;
	float yCenter = m_framebufferHeight / 2;
	float fNewX = xPos - xCenter;
	float fNewY = yPos - yCenter;
	float fOldX = mouse_prev_x - xCenter;
	float fOldY = mouse_prev_y - yCenter;
	float fVecX, fVecY, fVecZ;
	vCalcRotVec(fNewX, fNewY,
        fOldX, fOldY,
        diameter,
        &fVecX, &fVecY, &fVecZ);

	glm::mat4 rotationMatrix = vAxisRotMatrix(fVecX, -fVecY, fVecZ, rotationMatrix);
	rotationMatrix = glm::scale(glm::transpose(rotationMatrix),glm::vec3(1.0f,1.0f,1.0f));
	recursiveRotate(m_rootNode->trans, *m_rootNode, rotationMatrix);
}

SceneNode * A3::findNodeById(SceneNode& rootNode, unsigned int id){
	if(rootNode.m_nodeId == id){
		return &rootNode;
	}
	for(SceneNode * nextNode : rootNode.children){
		SceneNode * res = findNodeById(*nextNode, id);
		if(res!= NULL){
			return res;
		}
	}
	return NULL;
}

void A3::findNodeBelowJoint(SceneNode &node, unsigned int id){
	if(node.m_nodeId == id){
		// it is assumed that when you select the face/mini_head/eyes and all other decorations on the head
		// you actually mean to select the head.
		SceneNode * head = findNodeById(*m_rootNode, head_id);
		if(node.parent == head){
			(*head).isSelected = !(*head).isSelected;
			(*head).parent->isSelected = !(*head).parent->isSelected;
			return;
		}
		if(node.parent->m_nodeType == NodeType::JointNode){
			node.isSelected = !node.isSelected;
			node.parent->isSelected = !node.parent->isSelected;
		}
		return;
	}else{
		for(SceneNode *child : node.children){
			findNodeBelowJoint(*child, id);
		}
	}
}

void A3::recursiveRotate(glm::mat4 trans, SceneNode& root, glm::mat4 rotation) {
	glm::mat4 transformBack = glm::inverse(trans);
	root.trans = trans*rotation*transformBack*root.trans;
	for(SceneNode* node: root.children) {
		recursiveRotate(trans, *node, rotation);
	}
}

//----------------------------------------------------------------------------------------
/*
 * Rotate Joint handler.
 */
void A3::rotateJointHandler(double offsetX, double offsetY, int type){
	GLfloat angleY = offsetY/angleBase;
	GLfloat angleX = offsetX/angleBase;
	switch (type){
		case 1:
			rotateJoint(angleY, *m_rootNode, type);
			break;
		case 2:
			rotateJoint(angleX, *m_rootNode, type);
			break;

	}
}

void A3::rotateJoint(GLfloat angle, SceneNode & root, int type){
	
	for (SceneNode * child : root.children) {
		// type 1: rotate joints other than head
		if(child->m_name != "head" && child->isSelected && type == 1 && child->m_nodeType == NodeType::JointNode){
			JointNode* joint = (JointNode*)child;
			GLfloat curAngle = joint->m_joint_x.init + angle;
			if(curAngle < joint->m_joint_x.max && curAngle > joint->m_joint_x.min){
				joint->m_joint_x.init = curAngle;
				glm::mat4 x_rotateMatrix = glm::rotate(mat4(), angle, vec3(1.0f, 0.0f, 0.0f));
				A3::recursiveRotate(child->trans, *child, x_rotateMatrix);
			}
		}
		else if(child->m_name == "head" && type == 2 && child->isSelected){
			GLfloat newAngle = head_rotation + angle;
			if((newAngle < head_rotation_max) && (newAngle > head_rotation_min)){
				head_rotation = newAngle;
				glm::mat4 y_rotateMatrix = glm::rotate(mat4(), angle, vec3(0.0f, 1.0f, 0.0f));
				recursiveRotate(child->trans, *child, y_rotateMatrix);
				return;
			}
		}
	}
	for( SceneNode * child : root.children){
		rotateJoint(angle, *child, type);
	}
	
}