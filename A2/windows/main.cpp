/*Stefan Cherubin HW2
Primitives used: 2 cubes, 1 sphere
sphere is inside a cube, which is inside a larger cube
*/

#include "glsupport.h" //include this file first
#include "matrix4.h"
#include "quat.h"
#include "geometrymaker.h"
#include "VertexPN.h"
#include <GL\freeglut.h>


GLuint program; 
GLuint vertPositionVBO, vertTexCoordVBO, colorVBO, normalVBO;
GLuint positionAttribute, colorAttribute, texCoordAttribute, normalAttribute;
GLuint texture;
GLuint positionUniform, timeUniform;
GLuint modelviewMatrixUniformLocation;
GLuint projectionMatrixUniformLocation;
GLuint colorUniformLocation, normalUniformLocation;
 
float textureOffset = 0.0;
Entity ball;
int ibLen2, vbLen2;
Entity cube;
int ibLen, vbLen;
Entity bigCube;
int ibLen3, vbLen3;

 
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glUniform1f(timeUniform, textureOffset);
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	glUniform1f(timeUniform, (float)timeSinceStart / 1000.0f);
	
	
	
	

	
	

	Matrix4 objectMatrix;
	//objectMatrix = objectMatrix.makeXRotation(0.01 * timeSinceStart);
	//objectMatrix = objectMatrix.makeYRotation(-0.02 * timeSinceStart );
	Quat q1 = Quat::makeXRotation( 0.001* timeSinceStart *70.0f);
	Quat q2 = Quat::makeZRotation(0.001 * timeSinceStart * 20.0f);
	Quat q3 = q1;
	Quat q4 = Quat::makeZRotation(-0.001 * timeSinceStart * 20.0f);
	Quat combined = q1 * q2;
	Quat combined2 = q3 * q4;
	Matrix4 rotationMatrix = quatToMatrix(combined);
	Matrix4 eyeMatrix;
	eyeMatrix = eyeMatrix.makeTranslation(Cvec3(0.00, 0.0, 3.00)); //z-value will decide how "far" away the object is

	Matrix4 modelViewMatrix = inv(eyeMatrix) * rotationMatrix; //keep eye matrix inverted at all times
	
	Matrix4 modelViewMatrix2 = inv(eyeMatrix) * quatToMatrix(combined2);


	//Should move matrix stuff inside Entity class
	Matrix4 projectionMatrix;
	projectionMatrix = projectionMatrix.makeProjection(90.0, 1.0, -0.1, -100.0); //vertical field of view is important (1st parameter)! higher vfov will allow for better perephial vision
	//Vertical Field of View = 2 * arctan( tan(hfox/2) * Aspect Ratio)
	GLfloat glmatrixProjection[16];
	projectionMatrix.writeToColumnMajorMatrix(glmatrixProjection);
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, false, glmatrixProjection);

	//For now, all three entites will share everything, except for color attribute
	cube.Draw(modelViewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 0.0, 0.45, 1.0);
	ball.Draw(modelViewMatrix , positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 1.0, 0.45, 0.0);
	bigCube.Draw(modelViewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation, colorUniformLocation, 0.1, 0.2, 0.4);
	

	glBindBuffer(GL_ARRAY_BUFFER, vertPositionVBO);
	glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(colorAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(normalAttribute);

	
	//glDrawArrays replaced by glDrawElement in Geometry class
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	glDisableVertexAttribArray(positionAttribute);
	glDisableVertexAttribArray(colorAttribute);
	glDisableVertexAttribArray(normalAttribute);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	

    glutSwapBuffers();
}


void init() {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glReadBuffer(GL_BACK);

	program = glCreateProgram();
	readAndCompileShader(program, "vertex_textured.glsl", "fragment_textured.glsl");
	glUseProgram(program);

	positionAttribute = glGetAttribLocation(program, "position");
	colorAttribute = glGetAttribLocation(program, "color");
	timeUniform = glGetUniformLocation(program, "time");
	normalAttribute = glGetAttribLocation(program, "normal");

	normalUniformLocation = glGetUniformLocation(program, "normalMatrix");
	colorUniformLocation = glGetUniformLocation(program, "uColor");
	modelviewMatrixUniformLocation = glGetUniformLocation(program, "modelViewMatrix");
	projectionMatrixUniformLocation = glGetUniformLocation(program, "projectionMatrix");


	
	//cube.adoption(&bigCube);
	//bigCube.adoption(&ball);
	bigCube.adoption(&cube);
	cube.adoption(&ball);
	
	getCubeVbIbLen(vbLen, ibLen);
	cube.geometry.indexBO = ibLen;
	cube.geometry.vertexBO = vbLen;
	cube.geometry.numIndices = ibLen;

	std::vector<VertexPN> vtx(vbLen);
	std::vector<unsigned short> idx(ibLen);

	makeCube(1, vtx.begin(), idx.begin());


	getSphereVbIbLen(20, 20, vbLen2, ibLen2); //more slices & stacks make it more smooth and circle like
	ball.geometry.indexBO = ibLen2;
	ball.geometry.vertexBO = vbLen2;
	ball.geometry.numIndices = ibLen2;

	std::vector<VertexPN> vtx2(vbLen2);
	std::vector<unsigned short> idx2(ibLen2);

	makeSphere(0.5, 20, 20, vtx2.begin(), idx2.begin());

	getCubeVbIbLen(vbLen3, ibLen3);
	bigCube.geometry.indexBO = ibLen3;
	bigCube.geometry.vertexBO = vbLen3;
	bigCube.geometry.numIndices = ibLen3;

	std::vector<VertexPN> vtx3(vbLen3);
	std::vector<unsigned short> idx3(ibLen3);

	makeCube(2, vtx3.begin(), idx3.begin());



	glGenBuffers(1, &ball.geometry.vertexBO);
	glBindBuffer(GL_ARRAY_BUFFER, ball.geometry.vertexBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx2.size(), vtx2.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &ball.geometry.indexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ball.geometry.indexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx2.size(), idx2.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &cube.geometry.vertexBO);
	glBindBuffer(GL_ARRAY_BUFFER, cube.geometry.vertexBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx.size(), vtx.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &cube.geometry.indexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.geometry.indexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx.size(), idx.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &bigCube.geometry.vertexBO);
	glBindBuffer(GL_ARRAY_BUFFER, bigCube.geometry.vertexBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx3.size(), vtx3.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &bigCube.geometry.indexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bigCube.geometry.indexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx3.size(), idx3.data(), GL_STATIC_DRAW);




	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	GLfloat normalVerts[] = {
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f,-1.0f,
		0.0f, 0.0f,-1.0f,
		0.0f, 0.0f,-1.0f,
		0.0f,-1.0f,0.0f,
		0.0f,-1.0f,0.0f,
		0.0f,-1.0f,0.0f,
		0.0f, 0.0f,-1.0f,
		0.0f, 0.0f,-1.0f,
		0.0f, 0.0f,-1.0f,
		-1.0f, 0.0f,0.0f,
		-1.0f, 0.0f,0.0f,
		-1.0f, 0.0f,0.0f,
		0.0f,-1.0f, 0.0f,
		0.0f,-1.0f, 0.0f,
		0.0f,-1.0f,0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f,0.0f, 1.0f,
		0.0f,0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f,0.0f,0.0f,
		1.0f, 0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f,0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f,0.0f,
		0.0f, 1.0f,0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f,0.0f, 1.0f
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(normalVerts), normalVerts, GL_STATIC_DRAW);

	

} 


void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CS-6533");

    glewInit();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

	//glutKeyboardFunc(keyboard);
	//glutMouseFunc(mouse);     
	//glutMotionFunc(mouseMove);
    
    init();
    glutMainLoop();
    return 0;
}