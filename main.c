#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <math.h>

#define INF (1 << 29)
#define RES_W 680
#define RES_H 680

typedef struct XYZ{
	GLfloat *vertexArray;
	int listSize;
} XYZ;

typedef struct MarchingCubesMesh{
	GLfloat *vertexArray;
	GLuint *faces;
	int vertexCount, facesCount;
} MCM;

XYZ *model = NULL;
MCM *mcm = NULL;

int pointCloudVisualization = 1;
float ratio = 0.0;

float max(float a, float b){
	return (a > b) ? a : b;
}

float min(float a, float b){
	return (a < b) ? a : b;
}

XYZ* initXYZ(int listSize){
	XYZ *model = (XYZ*) malloc(sizeof(XYZ));
	model->vertexArray = (GLfloat*) malloc(sizeof(GLfloat) * listSize * 3);
	model->listSize = listSize;
	return model;
}

MCM* initMCM(){
	MCM *mcm = (MCM*) malloc(sizeof(MCM));
	mcm->vertexArray = NULL;
	mcm->faces = NULL;
	mcm->vertexCount = 0;
	mcm->facesCount = 0;
	return mcm;	
}

void loadLookUpTable(int lookUpTable[256][16], char *lutFileName){
	FILE *file = fopen(lutFileName, "rw+");
	if(file == NULL)
		return;

	int i, j;
	for(i = 0; i < 256; i++){
		for(j = 0; j < 4; j++)
			fscanf(file, "%d %d %d %d", &lookUpTable[i][4 * j + 0], &lookUpTable[i][4 * j + 1], &lookUpTable[i][4 * j + 2], &lookUpTable[i][4 * j + 3]);
	}
	fclose(file);
}

void set3DValue(int *data, int i, int j, int k, int size, int value){
	if(i >= size || j >= size || k >= size)
		return;

	data[(k * size * size) + (j * size) + i] = value;
}

int get3DValue(int *data, int i, int j, int k, int size){
	return data[(k * size * size) + (j * size) + i];
}

MCM* generateMeshFromXYZ(XYZ *model, double cubeSize, char *lutFileName){
	int cubesPerDimension = floor(1.0 / cubeSize);
	int *data = (int *) calloc(((int) pow(cubesPerDimension, 3)), sizeof(int));
	if(data == NULL)
		return NULL;

	int lut[256][16];
	int i, j, k;
	for(i = 0; i < model->listSize; i++){
		double x = model->vertexArray[i * 3];
		double y = model->vertexArray[i * 3 + 1];
		double z = model->vertexArray[i * 3 + 2];

		int cubeX = (int) floor(x / cubeSize);
		int cubeY = (int) floor(y / cubeSize);
		int cubeZ = (int) floor(z / cubeSize);

		set3DValue(data, cubeX, cubeY, cubeZ, cubesPerDimension, 1);
	}
	MCM *mcm = initMCM();
	loadLookUpTable(lut, lutFileName);

	//PAREI AQUI

	free(data);
	return mcm;
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

XYZ* readXYZFile(char fileName[]){
	int listSize = lineCount(fileName);
	if(listSize == 0)
		return NULL;

	FILE *file = fopen(fileName, "rw+");
	XYZ *model = NULL;
	if(file == NULL)
		return NULL;
	else{
		float minX = INF, minY = INF, minZ = INF;
		float maxX = 0, maxY = 0, maxZ = 0;
		int listPosition = 0, i;
		float x, y, z;
		
		model = initXYZ(listSize);
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

void drawUnitaryBox(){
	glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(0.5, 0.5, 0.5);
		glColor3f(1.0, 0.0, 0.0);

		glutWireCube(1.0);
	glPopMatrix();
}

void setupCamera(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(10.0f, ratio, 0.1, 500.0);
	gluLookAt(5, 2, 6, 0.3, 0.45, 0.4, 0, 1, 0);
}

void draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);

	// drawUnitaryBox();

	glColor3f(1.0, 1.0, 1.0);
	if(pointCloudVisualization){
		glPushMatrix();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, model->vertexArray);
			glDrawArrays(GL_POINTS, 0, model->listSize);
			glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();
	}
	else{

	}
	glutSwapBuffers();
}

void reshape(int w, int h){
	ratio = (float) w / h;
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);

	setupCamera();

	glutPostRedisplay();
}

void initScene(){
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glOrtho(-20.0, 20.0, -20.0, 20.0, -20.0, 20.0);

	glEnable(GL_DEPTH_TEST);
}

void keyboard(unsigned char key, int x, int y){
	if(key == 'm' || key == 'M'){
		pointCloudVisualization = !pointCloudVisualization;
		glutPostRedisplay();
	}
}

int main(int argc, char *argv[]){
	if(argc != 4){
		printf("Uso correto: %s [nuvem_de_pontos.xyz] [look-up_table.txt] [lado do cubo]\n", argv[0]);
		return 1;
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - RES_W) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - RES_H) / 2);
	glutInitWindowSize(RES_W, RES_W);

	glutCreateWindow("CG - Marching Cubes!");
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(draw);
	glutReshapeFunc(reshape);
	
	initScene();

	model = readXYZFile(argv[1]);
	mcm = generateMeshFromXYZ(model, atof(argv[3]), argv[2]);

	glutMainLoop();
	return 0;
}