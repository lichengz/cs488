// Winter 2020

#include <glm/ext.hpp>

#include "A4.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"

double intersection_threshold = 0.001;

void initTree(SceneNode *root) {
	for(SceneNode *node: root->children) {
		node->trans = root->trans*node->trans;
		node->invtrans = glm::inverse(node->trans);
		initTree(node);
	}
}

void A4_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters  
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...  
	initTree(root);

  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t imageh = image.height();
	size_t imagew = image.width();
	
	double d = distance(eye, view);
	// wh is the windows height
	double windowh = 2 * d * glm::tan(glm::radians(fovy/2));
	// ww is the windows width
	double windoww = imagew/imageh*windowh; 
	glm::vec4 eye_v4 = glm::vec4(eye, 1.0f);

	int perc = ceil(((double)imageh/10));
	for (int y = 0; y < imageh; ++y) {
		if(y% perc == 0){
			std::cout<< "Current render percentage : " << y/perc * 10<<std::endl;
		}
		for (int x = 0; x < imagew; ++x) {
			Ray curRay;
			curRay.start = eye_v4;
			glm::vec3 col = glm::vec3(0.0f, 0.0f, 0.0f);
			//-----------------------------------
			float imageAspectRatio = imagew / (float)imageh; // assuming width > height 
			float Px = (2 * ((x + 0.5f) / imagew) - 1) * d * glm::tan(glm::radians(fovy / 2)) * imageAspectRatio; 
			float Py = (1 - 2 * ((y + 0.5f) / imageh)) * d * glm::tan(glm::radians(fovy / 2)); 
			glm::vec4 rayDirection = glm::vec4(Px, Py, 0, 1) - eye_v4; // note that this just equal to Vec3f(Px, Py, -1); 
			rayDirection = normalize(rayDirection); // it's a direction so don't forget to normalize 
			//------------------------------------
			curRay.direction = rayDirection;
								
			
			col = color(curRay, 0, root, ambient, lights);
			
			// Red: 
			image(x, y, 0) = col.r;
			// Green: 
			image(x, y, 1) = col.g;
			// Blue: 
			image(x, y, 2) = col.b;
		}
	}

}

SceneNode * findNodeById(SceneNode& rootNode, unsigned int id){
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

intersection hit(SceneNode *&resNode, SceneNode *rootNode, Ray &ray, double limit, int depth){
	intersection int_res;
	int_res.t = -1;
	glm::vec4 start;
	glm::vec4 direction;
	for(int i = 0 ; i < rootNode->totalSceneNodes(); i++){
		const GeometryNode *geometryNode;
		SceneNode *node = findNodeById(*rootNode, i);
		
		if(node !=NULL){
			if(node->m_nodeType == NodeType::GeometryNode){ // only check intersection with geometrynode
				if(node->next != NULL){
					geometryNode = static_cast<const GeometryNode*>(node->next);
				}else{
					geometryNode = static_cast<const GeometryNode*>(node);
				}
				
				
				Ray transferd_ray;

				transferd_ray.start = node->invtrans*ray.start;
				transferd_ray.direction = glm::normalize(node->invtrans*(ray.start + ray.direction) - node->invtrans*ray.start);
				
				

				intersection temp_int_res;
				if(geometryNode->m_name == "plane"){
					geometryNode->m_primitive->isPlane = true;
				}
				if(node->trans != glm::mat4()){

					temp_int_res = geometryNode->m_primitive->checkIntersection(transferd_ray);
				}else{
					temp_int_res = geometryNode->m_primitive->checkIntersection(ray);
				}
				
				if (temp_int_res.t > intersection_threshold){
					if(node->trans != glm::mat4()){
						temp_int_res.t = glm::distance(glm::vec3(node->trans*(transferd_ray.start + temp_int_res.t * transferd_ray.direction)), glm::vec3(ray.start));
						temp_int_res.norm_v = glm::normalize(glm::transpose(glm::mat3(node->invtrans))*temp_int_res.norm_v);
					}

					if(temp_int_res.t < limit){
						resNode = node;
						limit = temp_int_res.t;
						int_res.t = temp_int_res.t;
						int_res.norm_v = temp_int_res.norm_v;
					} 
				}
			}
		}	
	}

	return int_res;
}

Ray reflectedRay(Ray r, glm::vec3 intersect, glm::vec3 n) {
	Ray ray;
	glm::vec3 v = glm::vec3(r.direction); 
	ray.start = glm::vec4(intersect, 1.0f);
    ray.direction = glm::vec4(v - 2*glm::dot(v,n)*n, 0.0f);
	return ray;
}

glm::vec3 color(Ray &ray, int depth, SceneNode *rootNode, const glm::vec3 & ambient,
				const std::list<Light *> & lights){
		double exp_test = 1e10; 
		SceneNode *resNode = NULL;
		double curLim = exp_test;
		intersection int_near = hit(resNode, rootNode, ray, curLim, depth);

		// performation invese transformation
		
		
		if(resNode !=NULL && int_near.t != -1){

			glm::vec3 col = glm::vec3(0.0f, 0.0f, 0.0f); 
			GeometryNode * geometryNode;
			// add ambient
			if(resNode->next !=NULL){
				geometryNode = static_cast<GeometryNode*>(resNode->next);
			}else{
				geometryNode = static_cast<GeometryNode*>(resNode);
			}
			
			PhongMaterial* pMaterial = static_cast<PhongMaterial*>(geometryNode->m_material);
			col += pMaterial->get_m_kd()*ambient;

			// clacultae new ori
			glm::vec3 newOri = glm::vec3(ray.start + int_near.t*(ray.direction));

			for(Light * light : lights){
				Ray curRay;

				// add transformation functions
				curRay.start = glm::vec4(newOri, 1.0f);
				curRay.direction = glm::normalize(glm::vec4(light->position - newOri, 0.0f));

				SceneNode *tempResNode = NULL;
				double tempCurLim = exp_test;
				intersection int_w_light = hit(tempResNode, rootNode, curRay, tempCurLim, depth);
				double diss = glm::abs( glm::distance(newOri, light->position + glm::vec3(int_w_light.t*curRay.direction)));
				if(int_w_light.t == -1 || (int_w_light.t > 0 && diss < 0.01)){// has interection
	
					glm::vec3 N = int_near.norm_v;
					glm::vec3 L = glm::vec3(curRay.direction);
					glm::vec3 V = glm::vec3(ray.direction);
					glm::vec3 h = glm::normalize(L - glm::vec3(ray.direction));
					float n_dot_l = std::max((float)glm::dot(N, L), 0.0f);
					float n_dot_h = std::max((float)glm::dot(N, h), 0.0f);
					glm::vec3 R = glm::normalize(2*glm::dot(L, N)*N - L);
					// Steps based on course notes page 171
					//Step 1. calculate diffuse
					glm::vec3 Diffuse = pMaterial->get_m_kd() * n_dot_l;

					//Step 2. calculate Specular
					float pf = std::pow(n_dot_h, pMaterial->get_m_shine() );

					glm::vec3 Specular =  pMaterial->get_m_ks() * pf;


					//Step 3. calculate attenuation
					double dis = glm::distance(glm::vec3(newOri), light->position);
					double devider = (light->falloff[0] + light->falloff[1] * dis + light->falloff[2] * dis * dis);

					// step 4 update color

					col += light->colour*(Diffuse + Specular);
					// if(depth < 1 && pMaterial->get_m_reflect() > 0){
					// 	Ray reflected = reflectedRay(ray, newOri, int_near.norm_v);
					// 	col += pMaterial->get_m_reflect() * color(reflected, depth + 1, rootNode, ambient, lights);
					// }
				}else{

				}

			}
			if(depth < 10 && pMaterial->get_m_reflect() > 0){
				Ray reflected = reflectedRay(ray, newOri, int_near.norm_v);
				col += pMaterial->get_m_reflect() * color(reflected, depth + 1, rootNode, ambient, lights);
			}
			return col; 
		}else{

			// for background color
			double y = ray.direction.y;
			glm::vec3 color = glm::vec3(y/10*4.0f, y/10*10.0f*1.0f, y/10*14.0f*1.0f);
			return color;
	
		}
};