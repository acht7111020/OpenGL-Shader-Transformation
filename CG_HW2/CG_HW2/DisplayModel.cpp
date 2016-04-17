#include <stdio.h>
class DisplayModel {
public:
	std::string filename;
	GLMmodel* model;
	std::vector<GLfloat> vertexBuffer;
	std::vector<GLfloat> colorBuffer;

	//Transformation matrix
	GLfloat M[16];
	static GLfloat V[16];
	static GLfloat P[16];
	GLfloat MVP[16]; // only MVP is column major
};