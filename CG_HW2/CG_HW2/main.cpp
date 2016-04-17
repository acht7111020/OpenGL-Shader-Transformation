#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <freeglut/glut.h>
#include "textfile.h"
#include "glm.h"
#include <vector>

#include "Matrices.h"

#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "freeglut.lib")

#ifndef GLUT_WHEEL_UP
# define GLUT_WHEEL_UP   0x0003
# define GLUT_WHEEL_DOWN 0x0004
#endif

#ifndef GLUT_KEY_ESC
# define GLUT_KEY_ESC 0x001B
#endif

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

GLfloat PlaneMaxX = 1;
GLfloat PlaneMinX = -1;
GLfloat PlaneMaxY = 1;
GLfloat PlaneMinY = -1;
using namespace std;
class DisplayModel {
public:
	string filename;
	GLMmodel* model;
	vector<GLfloat> vertexBuffer;
	vector<GLfloat> colorBuffer;

	//Transformation matrix
	Matrix4 M;
	Matrix4 MVP; // only MVP is column major
};

struct CameraParameters {
	GLfloat eye[3];
	GLfloat center[3];
	GLfloat up[3];
} camera;
struct MouseButton{
	GLfloat start[2];
	GLfloat end[2];
	int flag = 0;
}mouseLeft, mouseRight;
vector<DisplayModel> Objects;
Matrix4 globalV;
Matrix4 globalP;

// Shader attributes
GLint iLocPosition;
GLint iLocColor;
GLint iLocMVP;

char filename[] = "ColorModels/bunny5KC.obj";
//GLMmodel* OBJ;
GLfloat* vertices;
GLfloat* colors;
int projFlag = 0;	// 1: ortho, 0: perspective
int transMode = 1;	// 1: Scale, 2: Trans, 3: Rotate, 4: Eyes
int selectModel = 0; // 0, 1, 2, 3

void calMinandMax(GLfloat *trans, GLfloat x, GLfloat y, GLfloat z){
	if (x > trans[0]) trans[0] = x;
	if (x < trans[1]) trans[1] = x;
	if (y > trans[2]) trans[2] = y;
	if (y < trans[3]) trans[3] = y;
	if (z > trans[4]) trans[4] = z;
	if (z < trans[5]) trans[5] = z;

}
GLfloat modelingScale(GLfloat *trans, DisplayModel* OBJ, GLfloat size){
	GLfloat diffX = (trans[0] - trans[1]);
	GLfloat diffY = (trans[2] - trans[3]);
	GLfloat diffZ = (trans[4] - trans[5]);
	GLfloat tmp = max(diffX, diffY);
	tmp = max(diffZ, tmp);
	if (tmp == diffX){
		// X max;
		if (trans[0] > trans[1] * -1) {
			glmScale(OBJ->model, size / trans[0]);
			return (size / trans[0]);
		}
		else {
			glmScale(OBJ->model, -1 * size / trans[1]);
			return (-size / trans[1]);
		}
	}
	else if (tmp == diffY){
		if (trans[2] > trans[3] * -1) { 
			glmScale(OBJ->model, size / trans[2]); 
			return (size / trans[2]);
		}
		else { 
			glmScale(OBJ->model, -1 * size / trans[3]); 
			return (-size / trans[3]);
		}
	}
	else {
		if (trans[4] > trans[5] * -1) { 
			glmScale(OBJ->model, size / trans[4]); 
			return (size / trans[4]);
		}
		else { 
			glmScale(OBJ->model, -1 * size / trans[5]); 
			return (-size / trans[5]);
		}
	}
}
void traverseColorModel(DisplayModel* OBJ)
{
	int i;

	GLfloat maxVal[3];
	GLfloat minVal[3];
	GLfloat trans[6] = { -10, 10, -10, 10, -10, 10 };

	// number of triangles
	OBJ->model->numtriangles;

	// number of vertices
	OBJ->model->numvertices;

	// find the center 
	GLfloat mid_x, mid_y, mid_z;
	mid_x = mid_y = mid_z = 0;
	for (int i = 0; i<(int)OBJ->model->numtriangles; i++)
	{
		int indv1 = OBJ->model->triangles[i].vindices[0];
		int indv2 = OBJ->model->triangles[i].vindices[1];
		int indv3 = OBJ->model->triangles[i].vindices[2];

		mid_x += OBJ->model->vertices[3 * indv1 + 0];
		mid_y += OBJ->model->vertices[3 * indv1 + 1];
		mid_z += OBJ->model->vertices[3 * indv1 + 2];
		calMinandMax(&trans[0], OBJ->model->vertices[indv1 * 3 + 0], OBJ->model->vertices[indv1 * 3 + 1], OBJ->model->vertices[indv1 * 3+ 2]);

		mid_x += OBJ->model->vertices[3 * indv2 + 0];
		mid_y += OBJ->model->vertices[3 * indv2 + 1];
		mid_z += OBJ->model->vertices[3 * indv2 + 2];
		calMinandMax(&trans[0], OBJ->model->vertices[indv2 * 3 + 0], OBJ->model->vertices[indv2 * 3 + 1], OBJ->model->vertices[indv2* 3 + 2]);

		mid_x += OBJ->model->vertices[3 * indv3 + 0];
		mid_y += OBJ->model->vertices[3 * indv3 + 1];
		mid_z += OBJ->model->vertices[3 * indv3 + 2];
		calMinandMax(&trans[0], OBJ->model->vertices[indv3 * 3 + 0], OBJ->model->vertices[indv3 * 3 + 1], OBJ->model->vertices[indv3 * 3 + 2]);
	}
	mid_x /= OBJ->model->numtriangles*3;
	mid_y /= OBJ->model->numtriangles*3;
	mid_z /= OBJ->model->numtriangles*3;
	printf("mide = %f %f %f\n", mid_x, mid_y, mid_z);
	// set the center position of the model
	OBJ->model->position[0] = (trans[0] + trans[1]) / 2;
	OBJ->model->position[1] = (trans[2] + trans[3]) / 2;
	OBJ->model->position[2] = (trans[4] + trans[5]) / 2;
	printf("position = %f %f %f\n\n", OBJ->model->position[0], OBJ->model->position[1], OBJ->model->position[2]);
	GLfloat scaleParas = modelingScale(trans, OBJ, 0.3f);
	OBJ->model->position[0] *= scaleParas;
	OBJ->model->position[1] *= scaleParas;
	OBJ->model->position[2] *= scaleParas;

	for(i=0; i<(int)OBJ->model->numtriangles; i++)
	{
		// the index of each vertex
		int indv1 = OBJ->model->triangles[i].vindices[0];
		int indv2 = OBJ->model->triangles[i].vindices[1];
		int indv3 = OBJ->model->triangles[i].vindices[2];

		// the index of each color
		int indc1 = indv1;
		int indc2 = indv2;
		int indc3 = indv3;

		// vertices
		GLfloat vx, vy, vz;
		OBJ->vertexBuffer.push_back(OBJ->model->vertices[indv1 * 3 + 0] - OBJ->model->position[0]);
		OBJ->vertexBuffer.push_back(OBJ->model->vertices[indv1 * 3 + 1] - OBJ->model->position[1]);
		OBJ->vertexBuffer.push_back(OBJ->model->vertices[indv1 * 3 + 2] - OBJ->model->position[2]);

		OBJ->vertexBuffer.push_back(OBJ->model->vertices[indv2 * 3 + 0] - OBJ->model->position[0]);
		OBJ->vertexBuffer.push_back(OBJ->model->vertices[indv2 * 3 + 1] - OBJ->model->position[1]);
		OBJ->vertexBuffer.push_back(OBJ->model->vertices[indv2 * 3 + 2] - OBJ->model->position[2]);

		OBJ->vertexBuffer.push_back(OBJ->model->vertices[indv3 * 3 + 0] - OBJ->model->position[0]);
		OBJ->vertexBuffer.push_back(OBJ->model->vertices[indv3 * 3 + 1] - OBJ->model->position[1]);
		OBJ->vertexBuffer.push_back(OBJ->model->vertices[indv3 * 3 + 2] - OBJ->model->position[2]);

		// colors
		GLfloat c1, c2, c3;
		OBJ->colorBuffer.push_back(OBJ->model->colors[indv1 * 3 + 0]);
		OBJ->colorBuffer.push_back(OBJ->model->colors[indv1 * 3 + 1]);
		OBJ->colorBuffer.push_back(OBJ->model->colors[indv1 * 3 + 2]);

		OBJ->colorBuffer.push_back(OBJ->model->colors[indv2 * 3 + 0]);
		OBJ->colorBuffer.push_back(OBJ->model->colors[indv2 * 3 + 1]);
		OBJ->colorBuffer.push_back(OBJ->model->colors[indv2 * 3 + 2]);

		OBJ->colorBuffer.push_back(OBJ->model->colors[indv3 * 3 + 0]);
		OBJ->colorBuffer.push_back(OBJ->model->colors[indv3 * 3 + 1]);
		OBJ->colorBuffer.push_back(OBJ->model->colors[indv3 * 3 + 2]);
	}
	
}
void OBJModelTranslate(DisplayModel *OBJ, GLfloat x, GLfloat y, GLfloat z){

	OBJ->M[3] += x;
	OBJ->M[7] += y;
	OBJ->M[11] += z;

	//update MVP first
	OBJ->MVP = globalP*globalV*OBJ->M;
}
void initOBJMVP(DisplayModel *OBJ){
	Matrix4 M = Matrix4(
		1, 0, 0, 0,			
		0, 1, 0, 0,			
		0, 0, 1, 0,		
		0, 0, 0, 1);

	Matrix4 MVP = globalP*globalV*M;
	
	OBJ->M = M;
	OBJ->MVP = MVP;
}
void initTrans(DisplayModel *OBJ){
	OBJ->M[3] = -OBJ->model->position[0];
	OBJ->M[7] = -OBJ->model->position[1];
	OBJ->M[11] = -OBJ->model->position[2];
}
void loadOBJModel()
{
	string file[4] = {"blitzcrank_incognito.obj", "brain18KC.obj", "bunny5KC.obj", "duck4KC.obj"};
	GLfloat xmove[4] = { 0.5f, 0.0f, -0.5f, 0.0f };
	GLfloat ymove[4] = { 0.0f, 0.5f, 0.0f, -0.5f };
	if (Objects.size() != 0){
		for (int i = 0; i < Objects.size(); i++)
			free(Objects[i].model);
	}
	Objects.resize(4);
	for (int i = 0; i < 4 ; i++){
		Objects[i].filename = "ColorModels/" + file[i];
		Objects[i].model = glmReadOBJ( &Objects[i].filename[0]);
		printf("load file : %s\n", &Objects[i].filename[0]);
		// traverse the color model
		traverseColorModel(&Objects[i]);
		initOBJMVP(&Objects[i]); 
		//initTrans(&Objects[i]);
		OBJModelTranslate(&Objects[i], xmove[i], ymove[i], 0.5f);
		
	}
	
}

void UpdateProjection(int flag){

	GLfloat nearZ = 0.5;
	GLfloat farZ = 5;
	Matrix4 P;

	switch (flag) {
		case 0:
			// flag = 0; Perspective
			P = Matrix4(
				2 * nearZ / (PlaneMaxX - PlaneMinX), 0, (PlaneMaxX + PlaneMinX) / (PlaneMaxX - PlaneMinX), 0,
				0, 2 * nearZ / (PlaneMaxY - PlaneMinY), (PlaneMaxY + PlaneMinY) / (PlaneMaxY - PlaneMinY), 0,
				0, 0, -(farZ + nearZ) / (farZ - nearZ), -2 * farZ*nearZ / (farZ - nearZ),
				0, 0, -1, 0);
			printf("Perspective mode\n");
			break;
		case 1:
			// flag = 1; Ortho
			P = Matrix4(
				2 / (PlaneMaxX - PlaneMinX), 0, 0, -(PlaneMaxX + PlaneMinX) / (PlaneMaxX - PlaneMinX),
				0, 2 / (PlaneMaxY - PlaneMinY), 0, -(PlaneMaxY + PlaneMinY) / (PlaneMaxY - PlaneMinY),
				0, 0, -2 / (farZ - nearZ), -(farZ + nearZ) / (farZ - nearZ),
				0, 0, 0, 1);
			printf("Ortho mode\n");
			break;
		default:
			P = Matrix4(
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, -1, 0,
				0, 0, 0, 1);
			break;
	}

	globalP = P;
}

void cameraLookAt(){

	Vector3 corP = Vector3(camera.eye[0], camera.eye[1], camera.eye[2]);
	Vector3 corE = Vector3(camera.center[0], camera.center[1], camera.center[2]);
	Vector3 vecU = Vector3(camera.up[0], camera.up[1], camera.up[2]);
	Vector3 vecF = Vector3(corE - corP).normalize();
	Vector3 vecS = vecF.cross(vecU).normalize();
	Vector3 vecU_ = vecS.cross(vecF).normalize();

	Matrix4 rotateV = Matrix4{
		vecS.x, vecU_.x, -vecF.x, 0,
		vecS.y, vecU_.y, -vecF.y, 0,
		vecS.z, vecU_.z, -vecF.z, 0,
		0, 0, 0, 1
	};

	Matrix4 transV = Matrix4{
		1, 0, 0, -camera.eye[0],
		0, 1, 0, -camera.eye[1],
		0, 0, 1, -camera.eye[2],
		0, 0, 0, 1,
	};
	globalV = rotateV*transV;
}
void UpdateCamera(GLfloat x, GLfloat y){

	camera.eye[0] += x;
	camera.eye[1] += y;
	cameraLookAt();

}
void UpdateModeMevement(int mode, GLfloat x, GLfloat y, GLfloat z){
	Matrix4 m;
	switch (mode){			// 1: Scale, 2: Trans, 3: Rotate, 4: Eyes
		case 1:	// Scale
			m = Matrix4{
					1+x, 0, 0, 0,
					0, 1+y, 0, 0,
					0, 0, 1+z, 0,
					0, 0, 0, 1,
			};
			m = m*Objects[selectModel].M;
			m[3] = Objects[selectModel].M[3];
			m[7] = Objects[selectModel].M[7];
			m[11] = Objects[selectModel].M[11];
			Objects[selectModel].M = m; 
			break;
		case 2:	// Trans
			Objects[selectModel].M[3] += x;
			Objects[selectModel].M[7] += y;
			Objects[selectModel].M[11] += z;
			break;
		case 3:	// Rotate
			if (x != 0){
				
				m = Matrix4{
					1, 0, 0, 0,
					0, cos(x), -sin(x), 0,
					0, sin(x), cos(x), 0,
					0, 0, 0, 1,
				};
				m = m*Objects[selectModel].M;
				m[3] = Objects[selectModel].M[3];
				m[7] = Objects[selectModel].M[7];
				m[11] = Objects[selectModel].M[11];
				Objects[selectModel].M = m;
			}
			if (y != 0){
				m = Matrix4{
				cos(y), 0, sin(y), 0,
				0, 1, 0, 0,
				-sin(y), 0, cos(y), 0,
				0, 0, 0, 1,
				};
				m = m*Objects[selectModel].M;
				m[3] = Objects[selectModel].M[3];
				m[7] = Objects[selectModel].M[7];
				m[11] = Objects[selectModel].M[11];
				Objects[selectModel].M = m ;
			}
			if (z != 0) {
				m = Matrix4{
					cos(z), -sin(z), 0, 0,
					sin(z), cos(z), 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1,
				};
				m = m*Objects[selectModel].M;
				m[3] = Objects[selectModel].M[3];
				m[7] = Objects[selectModel].M[7];
				m[11] = Objects[selectModel].M[11];
				Objects[selectModel].M = m;
			}
			break;
		case 4:	// Eyes
			camera.eye[0] += x;
			camera.eye[1] += y;
			camera.eye[2] += z;
			cameraLookAt();
			break;
		default:
			break;
	}
}

void initCameraAndPlane(){
	camera.eye[0] = 0;
	camera.eye[1] = 0;
	camera.eye[2] = 1.5;

	camera.center[0] = 0;
	camera.center[1] = 0;
	camera.center[2] = -5;

	camera.up[0] = 0;
	camera.up[1] = 1;
	camera.up[2] = 0;

	cameraLookAt();
	UpdateProjection(0);

}
void Reset(){

	initCameraAndPlane();
	loadOBJModel();
}
void onIdle()
{
	glutPostRedisplay();
}

void drawObjects(DisplayModel *OBJ){

	//MVP
	Matrix4 T;
	Matrix4 S;
	Matrix4 R;

	OBJ->MVP = globalP*globalV*OBJ->M;

	GLfloat mvp[16];
	// row-major ---> column-major
	mvp[0] = OBJ->MVP[0];  mvp[4] = OBJ->MVP[1];   mvp[8] = OBJ->MVP[2];    mvp[12] = OBJ->MVP[3];
	mvp[1] = OBJ->MVP[4];  mvp[5] = OBJ->MVP[5];   mvp[9] = OBJ->MVP[6];    mvp[13] = OBJ->MVP[7];
	mvp[2] = OBJ->MVP[8];  mvp[6] = OBJ->MVP[9];   mvp[10] = OBJ->MVP[10];   mvp[14] = OBJ->MVP[11];
	mvp[3] = OBJ->MVP[12]; mvp[7] = OBJ->MVP[13];  mvp[11] = OBJ->MVP[14];   mvp[15] = OBJ->MVP[15];

	// bind array pointers to shader
	glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, &OBJ->vertexBuffer[0]);
	glVertexAttribPointer(iLocColor, 3, GL_FLOAT, GL_FALSE, 0, &OBJ->colorBuffer[0]);

	// bind uniform matrix to shader
	glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, mvp);

	// draw the array we just bound
	glDrawArrays(GL_TRIANGLES, 0, OBJ->model->numtriangles *3);

}
void onDisplay(void)
{
	// clear canvas
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(iLocPosition);
	glEnableVertexAttribArray(iLocColor);

	// TODO:
	//// Please define the model transformation matrix, viewing transformation matrix, 
	//// projection transformation matrix

	for (int i = 0; i < 4; i++){
		drawObjects(&Objects[i]);
	}

	glutSwapBuffers();
}

void showShaderCompileStatus(GLuint shader, GLint *shaderCompiled)
{
	glGetShaderiv(shader, GL_COMPILE_STATUS, shaderCompiled);
	if(GL_FALSE == (*shaderCompiled))
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character.
		GLchar *errorLog = (GLchar*) malloc(sizeof(GLchar) * maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		fprintf(stderr, "%s", errorLog);

		glDeleteShader(shader);
		free(errorLog);
	}
}

void setShaders()
{
	GLuint v, f, p;
	char *vs = NULL;
	char *fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("shader.vert");
	fs = textFileRead("shader.frag");

	glShaderSource(v, 1, (const GLchar**)&vs, NULL);
	glShaderSource(f, 1, (const GLchar**)&fs, NULL);

	free(vs);
	free(fs);

	// compile vertex shader
	glCompileShader(v);
	GLint vShaderCompiled;
	showShaderCompileStatus(v, &vShaderCompiled);
	if(!vShaderCompiled) system("pause"), exit(123);

	// compile fragment shader
	glCompileShader(f);
	GLint fShaderCompiled;
	showShaderCompileStatus(f, &fShaderCompiled);
	if(!fShaderCompiled) system("pause"), exit(456);

	p = glCreateProgram();

	// bind shader
	glAttachShader(p, f);
	glAttachShader(p, v);

	// link program
	glLinkProgram(p);

	iLocPosition = glGetAttribLocation (p, "av4position");
	iLocColor    = glGetAttribLocation (p, "av3color");
	iLocMVP		 = glGetUniformLocation(p, "mvp");

	glUseProgram(p);
}


void onMouse(int who, int state, int x, int y)
{
	

	switch(who)
	{
		case GLUT_LEFT_BUTTON:   
			mouseLeft.flag = 1;
			mouseRight.flag = 0;
			
			break;
		case GLUT_MIDDLE_BUTTON:  break;
		case GLUT_RIGHT_BUTTON:  
			mouseLeft.flag = 0;
			mouseRight.flag = 1;
			break; 
		case GLUT_WHEEL_UP:      
			UpdateModeMevement(1, 0.02f, 0.02f, 0.02f);
			break;
		case GLUT_WHEEL_DOWN:    
			UpdateModeMevement(1, -0.02f, -0.02f, -0.02f);
			break;
		default:                 break;
	}

	switch(state)
	{
		case GLUT_DOWN: 
			
			break;
		case GLUT_UP:   
			
			if (mouseLeft.flag == 1){
				
				mouseLeft.start[0] =0;
				mouseLeft.start[1] = 0;
			}
			else{
				mouseRight.start[0] = 0;
				mouseRight.start[1] = 0;
			}
			break;
	}


}

void onMouseMotion(int x, int y)
{
	

	if (mouseLeft.flag == 1){
		mouseLeft.end[0] = (double)x / 300;
		mouseLeft.end[1] = (double)y / 300;
		if (mouseLeft.start[0] != 0 || mouseLeft.start[1] != 0){
			UpdateModeMevement(2, (mouseLeft.end[0] - mouseLeft.start[0]), -(mouseLeft.end[1] - mouseLeft.start[1]), 0);
		}
		mouseLeft.start[0] = mouseLeft.end[0];
		mouseLeft.start[1] = mouseLeft.end[1];
	}
	else{
		mouseRight.end[0] = (double)x / 300;
		mouseRight.end[1] = (double)y / 300;
		if (mouseRight.start[0] != 0 || mouseRight.start[1] != 0){
			UpdateModeMevement(3, (mouseRight.end[1] - mouseRight.start[1]), (mouseRight.end[0] - mouseRight.start[0]), 0);
		}
		mouseRight.start[0] = mouseRight.end[0];
		mouseRight.start[1] = mouseRight.end[1];
	}
}

void onKeyboard(unsigned char key, int x, int y) 
{
	//printf("%18s(): (%d, %d) key: %c(0x%02X) ", __FUNCTION__, x, y, key, key);
	switch(key) 
	{
		case GLUT_KEY_ESC: /* the Esc key */ 
			exit(0); 
			break;
		case 'h':
		case 'H':
			printf("\n\n====================================================================\n");
			printf("-----------There is help information. !!!-----------\n");
			printf("Press p/P to switch Ortho mode/ Perspective mode.\n");
			printf("Press t/T to set Translate mode.\n");
			printf("Press s/S to set Scale mode.\n");
			printf("Press r/R to set Rotate mode.\n");
			printf("Press e/E to set Camera Eyes mode.\n\n");
			printf("Press -> / <- to change the selecting object.\n\n");
			printf("-----------Then you can change x,y,z-axis.-------------\n");
			printf("Press l/L to increase x-axis value, j/J to decrease x-axis value\n");
			printf("Press i/I to increase y-axis value, k/K to decrease y-axis value\n");
			printf("Press m/M to increase z-axis value, o/O to decrease z-axis value\n\n");
			printf("-----------You can also press number to change camera eyes position.-----------\n");
			printf("Press 6 to increase x-axis camera eyes.\n");
			printf("Press 4 to decrease x-axis camera eyes.\n");
			printf("Press 8 to increase y-axis camera eyes.\n");
			printf("Press 2 to decrease y-axis camera eyes.\n\n");
			printf("-----------Mouse Event Function.-----------\n");
			printf("Press the left button and drag the objects to translate.\n");
			printf("Press the right button and drag the objects to rotate.\n");
			printf("Roll the middle button and drag the objects to scale.\n\n");
			printf("-----------Another help Functions.-----------\n");
			printf("Press 5 to reset objects.\n");
			printf("Press h to get help information.\n");
			printf("Press c/C to clear the terminal.\n");
			printf("\n====================================================================\n\n");
			break;
		/*
			Mode selected.
		*/
		case 'p':	// p: perspective projection
		case 'P':
			projFlag = (projFlag == 1)? 0 : 1;
			UpdateProjection(projFlag);
			break;
		/*
			Mode selected.
		*/
		case 's':	// s: scaling factors input (sx, sy, sz)
		case 'S':
			transMode = 1;
			break;
		case 't':	// t: Translation offsets input (tx, ty, tz)
		case 'T':
			transMode = 2;
			break;
		case 'r':	// r: Rotation angles input (qx, qy, qz), in degrees
		case 'R':
			transMode = 3;
			break;
		case 'e':	// e: eye coordinates input (ex, ey, ez)
		case 'E':
			transMode = 4;
			break;
		/*
			Movement
		*/
		case 'l':	// x++
		case 'L':
			UpdateModeMevement(transMode, 0.02f, 0, 0);
			break;
		case 'j':	// x--
		case 'J':
			UpdateModeMevement(transMode, -0.02f, 0, 0);
			break;
		case 'i':	// y++
		case 'I':
			UpdateModeMevement(transMode, 0, 0.02f, 0);
			break;
		case 'k':	// y--
		case 'K':
			UpdateModeMevement(transMode, 0, -0.02f, 0);
			break;
		case 'o':	// z--
		case 'O':
			UpdateModeMevement(transMode, 0, 0, 0.02f);
			break;
		case 'm':	// z++
		case 'M':
			UpdateModeMevement(transMode, 0, 0, -0.02f);
			break;
		/*
			Camera eye move.
		*/
		case'4':
			printf("x = %d\n", x);
			UpdateCamera(0.05f, 0);
			break;
		case'6':
			printf("x = %d\n", x);
			UpdateCamera(-0.05f, 0);
			break;
		case'8':
			printf("y = %d\n", y);
			UpdateCamera(0, -0.05f);
			break;
		case'2':
			printf("y = %d\n", y);
			UpdateCamera(0, 0.05f);
			break;
		case'5':
			Reset();
			break;
		case 'C':
		case 'c':
			system("CLS");
			break;
	}
	//printf("\n");
}

void onKeyboardSpecial(int key, int x, int y){
	switch(key)
	{
		case GLUT_KEY_LEFT:
			selectModel--;
			if (selectModel < 0) selectModel = 3;
			printf("SelectModel = %d\n", selectModel);
			break; 
		case GLUT_KEY_RIGHT:
			selectModel++;
			if (selectModel > 3) selectModel = 0;
			printf("SelectModel = %d\n", selectModel);
			break;
		default: 
			break;
	}
}


void onWindowReshape(int width, int height)
{
	printf("%18s(): %dx%d\n", __FUNCTION__, width, height);
	float viewportAspect = (float)width / (float)height;
}


int main(int argc, char **argv) 
{ 
	// glut init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	// create window
	glutInitWindowPosition(500, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("10420 CS550000 CG HW2 Blurry");

	glewInit();
	if(glewIsSupported("GL_VERSION_2_0")){
		printf("Ready for OpenGL 2.0\n");
	}else{
		printf("OpenGL 2.0 not supported\n");
		system("pause");
		exit(1);
	}

	// load obj models through glm
	loadOBJModel();

	// register glut callback functions
	glutDisplayFunc (onDisplay);
	glutIdleFunc    (onIdle);
	glutKeyboardFunc(onKeyboard);
	glutSpecialFunc (onKeyboardSpecial);
	glutMouseFunc   (onMouse);
	glutMotionFunc  (onMouseMotion);
	glutReshapeFunc (onWindowReshape);

	// set up shaders here
	setShaders();

	initCameraAndPlane();
	glEnable(GL_DEPTH_TEST);

	// main loop
	glutMainLoop();

	// free
	for (int i = 0; i < Objects.size(); i++){
		glmDelete(Objects[i].model);
	}

	return 0;
}

