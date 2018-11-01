#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <math.h>

#define INF (1 << 29)
#define RES_W 680
#define RES_H 440

typedef struct Model{
	GLfloat *vertexArray;
	int listSize;
} Model;

Model *model = NULL;
float ratio = 0.0;
char operation = 'v';

float max(float a, float b){
	return (a > b) ? a : b;
}

float min(float a, float b){
	return (a < b) ? a : b;
}

Model* initModelFile(int listSize){
	Model *model = (Model*) malloc(sizeof(Model));
	model->vertexArray = (GLfloat*) malloc(sizeof(GLfloat) * listSize * 3);
	model->listSize = listSize;
	return model;
}

int lineCount(char fileName[]){
	FILE *file = fopen(fileName, "rw+");
	int count = 0;
	if(file == NULL)
		return 0;

	while(!feof(file)){
		char c = fgetc(file);
		if(c == '\n')
			count++;
	}
	fclose(file);
	return count;
}

Model* readXYZFile(char fileName[]){
	int listSize = lineCount(fileName);
	if(listSize == 0)
		return NULL;

	FILE *file = fopen(fileName, "rw+");
	Model *model = NULL;
	if(file == NULL)
		return NULL;
	else{
		float minX = INF, minY = INF, minZ = INF;
		float maxX = 0, maxY = 0, maxZ = 0;
		int listPosition = 0, i;
		float x, y, z;
		
		model = initModelFile(listSize);
		while(fscanf(file, "%f %f %f", &x, &y, &z) == 3){
			model->vertexArray[listPosition] = x;
			model->vertexArray[listPosition + 1] = y;
			model->vertexArray[listPosition + 2] = z;
			listPosition += 3;

			maxX = max(maxX, x);
			maxY = max(maxY, y);
			maxZ = max(maxZ, z);

			minX = min(minX, x);
			minY = min(minY, y);
			minZ = min(minZ, z);
		}
		fclose(file);

		for(i = 0; i < listSize * 3; i += 3){
			model->vertexArray[i] = (model->vertexArray[i] - minX) / (maxX - minX);
			model->vertexArray[i + 1] = (model->vertexArray[i + 1] - minY) / (maxY - minY);
			model->vertexArray[i + 2] = (model->vertexArray[i + 2] - minZ) / (maxZ - minZ);
		}
	}
	return model;
}

void draw(){
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(10.0f, ratio, 0.1, 5.0);
	gluLookAt(1.0, 0.5, 1.0, 0, 0, 0, 0, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, model->vertexArray);
		glDrawArrays(GL_POINTS, 0, model->listSize);
		glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();

	glFlush();
}

void reshape(int w, int h){
	ratio = (float) w / h;
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	glutPostRedisplay();
}

void initScene(){
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
}

int main(int argc, char *argv[]){
	if(argc != 4){
		printf("Uso correto: %s [nuvem_de_pontos.xyz] [look-up_table.txt] [lado do cubo]\n", argv[0]);
		return 1;
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - RES_W) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - RES_H) / 2);
	glutInitWindowSize(RES_W, RES_W);

	glutCreateWindow("CG - Marching Cubes!");
	glutDisplayFunc(draw);
	
	initScene();

	model = readXYZFile(argv[1]);

	glutMainLoop();
	return 0;
}