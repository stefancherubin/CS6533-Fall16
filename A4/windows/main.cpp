/*Stefan Cherubin HW4
This is the same as HW3, except that there are some post processing shaders used this time (in the following order):

HDR Tone Mapping + Exposure
Luminance 
Horizontal Blur
Vertical Blur

Originally wanted to HDR Tone Mapping + Color Correction LUTs, but couldn't get LUT to work properly.

*/
#define TINYOBJLOADER_IMPLEMENTATION
#include "glsupport.h" //include this file first
#include "matrix4.h"
#include "quat.h"
#include "geometrymaker.h"
#include "VertexPrime.h"
#include "tiny_obj_loader.h"
#include "stb_image.h"
#include <GL\freeglut.h>


GLuint program, screenShaderProgram, luminanceShaderProgram, blurredShaderProgramh, blurredShaderProgramv, finaladditiveShaderProgram;
GLuint vertPositionVBO, vertTexCoordVBO, colorVBO, normalVBO;
GLuint positionAttribute, colorAttribute, texCoordAttribute, normalAttribute, binormalAttribute, tangentAttribute;
GLuint texture;
GLuint positionUniform, timeUniform, lightDirectionUniform;
GLuint modelviewMatrixUniformLocation;
GLuint projectionMatrixUniformLocation;
GLuint colorUniformLocation, normalUniformLocation, lightDirectionUniformLocation0, lightDirectionUniformLocation1, normalTexUniformLocation;
GLuint diffuseTexture, specularTexture, diffuseTexture2, normalTexture;
GLuint diffuseTextureUniformLocation, diffuseTextureUniformLocation1;
GLuint specularUniformLocation, blurredUniform;
GLuint cubeMap, lookupTableUniform;
GLuint frameBuffer, frameBufferTexture, depthBufferTexture, frameBufferUniformLocation, luminanceFBO, blurredhFBO, blurredvFBO, exposureFBO, finaladdFBO;
GLuint screenTrianglesPositionBuffer, screenTrianglesPositionAttribute, screenTrianglesUVBuffer, screenTrianglesTexCoordAttribute;
 
float textureOffset = 0.0;
Entity ball;
int ibLen2, vbLen2;
Entity cube;
int ibLen, vbLen;
Entity bigCube;
int ibLen3, vbLen3;
Entity object, object2;
Entity areaFloor;
int ibLen4, vbLen4;




 
void display(void) {
	//NORMAL RENDERING
	object.transform.setEyeTranslation(8.0, 6.05, 10.25);
	object.transform.createProjectionMatrix(projectionMatrixUniformLocation);
	object.transform.setModelviewMatrix();
	object.transform.rotateYwithTime(timeUniform);

	object2.transform.setEyeTranslation(-8.0, 6.05, 10.25);
	object2.transform.createProjectionMatrix(projectionMatrixUniformLocation);
	object2.transform.setModelviewMatrix();
	object2.transform.rotateYwithTime(timeUniform);

	glBindBuffer(GL_ARRAY_BUFFER, vertPositionVBO);
	glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(colorAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(normalAttribute);

	glEnableVertexAttribArray(binormalAttribute);
	glVertexAttribPointer(binormalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPrime), (void*)offsetof(VertexPrime, b));
	glDisableVertexAttribArray(binormalAttribute);

	glEnableVertexAttribArray(tangentAttribute);
	glVertexAttribPointer(tangentAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPrime), (void*)offsetof(VertexPrime, tg));
	glDisableVertexAttribArray(tangentAttribute);



	glDisableVertexAttribArray(positionAttribute);
	glDisableVertexAttribArray(colorAttribute);
	glDisableVertexAttribArray(normalAttribute);
	
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, 800, 800);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(program);
	glClearColor(0.2, 0.2, 0.2, 1.0);


	
	

	glUniform1f(timeUniform, textureOffset);
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	glUniform1f(timeUniform, (float)timeSinceStart / 1000.0f);
	
	
	
	

	
	object.Draw(object.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);
	object2.Draw(object2.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);

	

	
	
	glBindFramebuffer(GL_FRAMEBUFFER, exposureFBO);
	glViewport(0, 0, 800, 800);	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//HDR TONE MAPPING + EXPOSURE
	glUseProgram(screenShaderProgram);

	
	object.Draw(object.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);
	object2.Draw(object2.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);


	//LUMINANCE SHADER
	glBindFramebuffer(GL_FRAMEBUFFER, luminanceFBO);
	glViewport(0, 0, 800, 800);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(luminanceShaderProgram);
	

	object.Draw(object.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);
	object2.Draw(object2.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);


	
	//HORIZONTAL BLUR SHADER
	glBindFramebuffer(GL_FRAMEBUFFER, blurredhFBO);
	glViewport(0, 0, 800, 800);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(blurredShaderProgramh);
	

	object.Draw(object.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);
	object2.Draw(object2.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);



	//VERTICAL BLUR SHADER
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 800);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(blurredShaderProgramv);
	
	object.Draw(object.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);
	object2.Draw(object2.transform.modelviewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.1, 0.1);


	

	

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutSwapBuffers();
}
void calculateFaceTangent(const Cvec3f &v1, const Cvec3f &v2, const Cvec3f &v3, const Cvec2f &texCoord1, const Cvec2f &texCoord2,
	const Cvec2f &texCoord3, Cvec3f &tangent, Cvec3f &binormal) {
	Cvec3f side0 = v1 - v2;
	Cvec3f side1 = v3 - v1;
	Cvec3f normal = cross(side1, side0);
	normalize(normal);
	float deltaV0 = texCoord1[1] - texCoord2[1];
	float deltaV1 = texCoord3[1] - texCoord1[1];
	tangent = side0 * deltaV1 - side1 * deltaV0;
	normalize(tangent);
	float deltaU0 = texCoord1[0] - texCoord2[0];
	float deltaU1 = texCoord3[0] - texCoord1[0];
	binormal = side0 * deltaU1 - side1 * deltaU0;
	normalize(binormal);
	Cvec3f tangentCross = cross(tangent, binormal);
	if (dot(tangentCross, normal) < 0.0f) {
		tangent = tangent * -1;
	}
}

void loadObjFile(const std::string &fileName, std::vector<VertexPrime> &outVertices, std::vector<unsigned short> &outIndices) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), NULL, true);
	if (ret) {
		for (int i = 0; i < shapes.size(); i++) {
			for (int j = 0; j < shapes[i].mesh.indices.size(); j++) {
				unsigned int vertexOffset = shapes[i].mesh.indices[j].vertex_index * 3;
				unsigned int normalOffset = shapes[i].mesh.indices[j].normal_index * 3;
				unsigned int texOffset = shapes[i].mesh.indices[j].texcoord_index * 2;
				VertexPrime v;
				v.p[0] = attrib.vertices[vertexOffset];
				v.p[1] = attrib.vertices[vertexOffset + 1];
				v.p[2] = attrib.vertices[vertexOffset + 2];
				v.n[0] = attrib.normals[normalOffset];
				v.n[1] = attrib.normals[normalOffset + 1];
				v.n[2] = attrib.normals[normalOffset + 2];
				v.t[0] = attrib.texcoords[texOffset];
				v.t[1] = 1.0 - attrib.texcoords[texOffset + 1];
				outVertices.push_back(v);
				outIndices.push_back(outVertices.size() - 1);


			}

		}
		for (int i = 0; i < outVertices.size(); i += 3) {
			Cvec3f tangent;
			Cvec3f binormal;
			calculateFaceTangent(outVertices[i].p, outVertices[i + 1].p, outVertices[i + 2].p,
				outVertices[i].t, outVertices[i + 1].t, outVertices[i + 2].t, tangent, binormal);
			outVertices[i].tg = tangent;
			outVertices[i + 1].tg = tangent;
			outVertices[i + 2].tg = tangent;
			outVertices[i].b = binormal;
			outVertices[i + 1].b = binormal;
			outVertices[i + 2].b = binormal;
		}
		/*std::vector<std::string> cubemapFiles;
		cubemapFiles.push_back("alps_ft.tga");
		cubemapFiles.push_back("alps_bk.tga");
		cubemapFiles.push_back("alps_up.tga");
		cubemapFiles.push_back("alps_dn.tga");
		cubemapFiles.push_back("alps_rt.tga");
		cubemapFiles.push_back("alps_lf.tga");
		cubeMap = loadGLCubemap(cubemapFiles);*/
		
	}
	else {
		std::cout << err << std::endl;
		assert(false);
	}
}
//Will try out refraction at a later time
//GLuint loadGLCubemap(std::vector<std::string> faces) {
//	GLuint textureID;
//	glGenTextures(1, &textureID);
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
//	for (GLuint i = 0; i < faces.size(); i++) {
//		int w, h, comp;
//		unsigned char* image = stbi_load(faces[i].c_str(), &w, &h, &comp, STBI_rgb_alpha);
//		if (image) {
//			glTexImage2D(
//				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
//				GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image
//			);
//			stbi_image_free(image);
//		}
//	}
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
//	return textureID;
//}
void init() {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //GL_BACK will enable that nice lighting effect, but you may be able to see through your object
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glReadBuffer(GL_BACK);
	glEnable(GL_MULTISAMPLE);
	glClearDepth(0.0f);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);

	program = glCreateProgram();
	
	readAndCompileShader(program, "vertex_textured.glsl", "fragment_textured.glsl");
	
	screenShaderProgram = glCreateProgram();
	readAndCompileShader(screenShaderProgram, "vertex_textured.glsl", "fragment_textured_frame.glsl");

	luminanceShaderProgram = glCreateProgram();
	readAndCompileShader(luminanceShaderProgram, "vertex_textured.glsl", "fragment_textured_luminance.glsl");

	blurredShaderProgramh = glCreateProgram();
	readAndCompileShader(blurredShaderProgramh, "vertex_textured.glsl", "fragment_textured_blurredh.glsl");

	blurredShaderProgramv = glCreateProgram();
	readAndCompileShader(blurredShaderProgramv, "vertex_textured.glsl", "fragment_textured_blurredv.glsl");

	finaladditiveShaderProgram = glCreateProgram();
	readAndCompileShader(finaladditiveShaderProgram, "vertex_textured.glsl", "fragment_textured_final_additive.glsl");


	positionAttribute = glGetAttribLocation(program, "position");
	


	timeUniform = glGetUniformLocation(program, "time");
	normalAttribute = glGetAttribLocation(program, "normal");
	texCoordAttribute = glGetAttribLocation(program, "texCoord");

	

	binormalAttribute = glGetAttribLocation(program, "binormal");
	tangentAttribute = glGetAttribLocation(program, "tangent");


	normalUniformLocation = glGetUniformLocation(program, "normalMatrix");
	colorUniformLocation = glGetUniformLocation(program, "uColor");
	modelviewMatrixUniformLocation = glGetUniformLocation(program, "modelViewMatrix");
	projectionMatrixUniformLocation = glGetUniformLocation(program, "projectionMatrix");
	diffuseTextureUniformLocation = glGetUniformLocation(program, "diffuseTexture");
	diffuseTextureUniformLocation1 = glGetUniformLocation(program, "diffuseTexture[1]");
	specularUniformLocation = glGetUniformLocation(program, "specularTexture");
	normalTexUniformLocation = glGetUniformLocation(program, "normalTexture");
	frameBufferUniformLocation = glGetUniformLocation(program, "screenFrameBuffer");
	lookupTableUniform = glGetUniformLocation(program, "lookupTable");

	lightDirectionUniformLocation0 = glGetUniformLocation(program, "lights[0].lightDirection");
	lightDirectionUniformLocation1 = glGetUniformLocation(program, "lights[1].lightDirection");

	blurredUniform = glGetUniformLocation(finaladditiveShaderProgram, "blurredHighlights");
	


	glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, 1024, 1024, 0, GL_RGB,
	GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT,
		GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);



	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, frameBufferTexture, 0);

	glGenTextures(1, &depthBufferTexture);
	glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT,
		GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		depthBufferTexture, 0);
	
	


	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropyAmount);
	loadObjFile("Monk_Giveaway_Fixed.obj", object.meshVertices, object.meshIndices);
	object.geometry.numIndices = object.meshIndices.size();
	object.geometry.vertexBO = object.meshVertices.size();
	object.geometry.indexBO = object.meshIndices.size();

	glGenBuffers(1, &object.geometry.vertexBO);
	glBindBuffer(GL_ARRAY_BUFFER, object.geometry.vertexBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPrime) * object.meshVertices.size(), object.meshVertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &object.geometry.indexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.geometry.indexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * object.meshIndices.size(), object.meshIndices.data(), GL_STATIC_DRAW);


	object.diffuseTexture = loadGLTexture("Monk_D.tga");
	object.specularTexture = loadGLTexture("Monk_S.tga");
	object.normalTexture = loadGLTexture("Monk_N_Normal_Bump.tga");
	
	glUniform1i(diffuseTextureUniformLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object.diffuseTexture);

	glUniform1i(specularUniformLocation, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, object.specularTexture);

	glUniform1i(normalTexUniformLocation, 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, object.normalTexture);


	loadObjFile("Monk_Giveaway_Fixed.obj", object2.meshVertices, object2.meshIndices);
	object2.geometry.numIndices = object2.meshIndices.size();
	object2.geometry.vertexBO = object2.meshVertices.size();
	object2.geometry.indexBO = object2.meshIndices.size();

	glGenBuffers(1, &object2.geometry.vertexBO);
	glBindBuffer(GL_ARRAY_BUFFER, object2.geometry.vertexBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPrime) * object2.meshVertices.size(), object2.meshVertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &object2.geometry.indexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object2.geometry.indexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * object2.meshIndices.size(), object2.meshIndices.data(), GL_STATIC_DRAW);


	object2.diffuseTexture = loadGLTexture("Monk_D.tga");
	object2.specularTexture = loadGLTexture("Monk_S.tga");
	object2.normalTexture = loadGLTexture("Monk_N.tga");

	glUniform1i(diffuseTextureUniformLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object2.diffuseTexture);

	glUniform1i(specularUniformLocation, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, object2.specularTexture);

	glUniform1i(normalTexUniformLocation, 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, object2.normalTexture);

	glEnableVertexAttribArray(texCoordAttribute);
	glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPrime), (void*)offsetof(VertexPrime, t));
	
	
	

	
	
	
} 




void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CS-6533 Rogue Two");

    glewInit();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    
    init();
    glutMainLoop();
    return 0;
}