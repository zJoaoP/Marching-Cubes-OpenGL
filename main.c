#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <math.h>

#define INF (1 << 29)
#define RES_W 680
#define RES_H 680

typedef struct XYZ{
	GLdouble *vertexArray;
	int listSize;
} XYZ;

typedef struct MarchingCubesMesh{
	GLdouble *vertexArray;
	GLuint *faces;
	int vertexCount, facesCount;
} MCM;

XYZ *model = NULL;
MCM *mcm = NULL;

int pointCloudVisualization = 1;
float ratio = 0.0;

double max(double a, double b){
	return (a > b) ? a : b;
}

double min(double a, double b){
	return (a < b) ? a : b;
}

XYZ* initXYZ(int listSize){
	XYZ *model = (XYZ*) malloc(sizeof(XYZ));
	model->vertexArray = (GLdouble*) malloc(sizeof(GLdouble) * listSize * 3);
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

void freeXYZ(XYZ **model){
	free((*model)->vertexArray);
}

void freeMCM(MCM **mcm){
	free((*mcm)->vertexArray);
	free((*mcm)->faces);
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

double generateMarchingCubesCoord_X(int x, int vertex, double cubeSize){
	if(vertex == 3 || vertex == 8 || vertex == 7 || vertex == 11)
		return x * cubeSize;
	else if(vertex == 0 || vertex == 4 || vertex == 6 || vertex == 2)
		return x * cubeSize + cubeSize / 2;
	else
		return (x + 1) * cubeSize;
}

double generateMarchingCubesCoord_Y(int y, int vertex, double cubeSize){
	if(vertex == 0 || vertex == 1 || vertex == 2 || vertex == 3)
		return y * cubeSize;
	else if(vertex == 8 || vertex == 9 || vertex == 10 || vertex == 11)
		return y * cubeSize + cubeSize / 2;
	else
		return (y + 1) * cubeSize;
}

double generateMarchingCubesCoord_Z(int z, int vertex, double cubeSize){
	if(vertex == 0 || vertex == 9 || vertex == 4 || vertex == 8)
		return z * cubeSize;
	else if(vertex == 3 || vertex == 1 || vertex == 5 || vertex == 7)
		return z * cubeSize + cubeSize / 2;
	else
		return (z + 1) * cubeSize;
}

void insertVertexMCM(MCM **mcm, double x, double y, double z){
	(*mcm)->vertexArray = (GLdouble*) realloc((*mcm)->vertexArray, ((*mcm)->vertexCount + 3) * sizeof(GLdouble));

	int currentPosition = (*mcm)->vertexCount;
	
	(*mcm)->vertexArray[currentPosition] = x;
	(*mcm)->vertexArray[currentPosition + 1] = y;
	(*mcm)->vertexArray[currentPosition + 2] = z;
	(*mcm)->vertexCount += 3;
}

void insertVertexOfFaceMCM(MCM **mcm, int vertexID){
	if((*mcm)->faces == NULL)
		(*mcm)->faces = (GLuint*) malloc(3 * sizeof(GLuint));
	else
		(*mcm)->faces = (GLuint*) realloc((*mcm)->faces, ((*mcm)->facesCount + 1) * sizeof(GLuint));

	int currentPosition = (*mcm)->facesCount;

	(*mcm)->faces[currentPosition] = vertexID;
	(*mcm)->facesCount++;
}

//GET_VERTEX_ID == (if none) ? -1 : ID;

int getVertexIDFromMCM(MCM *mcm, double x, double y, double z){
	int i = 0;
	for(i = 0; i < mcm->vertexCount; i += 3){
		if(mcm->vertexArray[i] == x && mcm->vertexArray[i + 1] == y && mcm->vertexArray[i + 2] == z)
			return i / 3;
	}
	return -1;
}

void generateAndInsertTriangles(MCM **mcm, int x, int y, int z, int triangles[16], double cubeSize){
	int i;
	for(i = 0; triangles[i] != -1; i++){
		double tX = generateMarchingCubesCoord_X(x, triangles[i], cubeSize) - cubeSize / 2;
		double tY = generateMarchingCubesCoord_Y(y, triangles[i], cubeSize) - cubeSize / 2;
		double tZ = generateMarchingCubesCoord_Z(z, triangles[i], cubeSize) - cubeSize / 2;

		int id = getVertexIDFromMCM((*mcm), tX, tY, tZ);
		if(id == -1){
			insertVertexMCM(mcm, tX, tY, tZ);
			insertVertexOfFaceMCM(mcm, ((*mcm)->vertexCount - 1) / 3);
		}
		else{
			insertVertexOfFaceMCM(mcm, id);
		}
	}
}

MCM* generateMeshFromXYZ(XYZ *model, double cubeSize, char *lutFileName){
	int cubesPerDimension = floor(1.0 / cubeSize) + 2;
	printf("Cubes Per Dimension: %d\n", cubesPerDimension);
	int *data = (int *) calloc(((int) pow(cubesPerDimension, 3)), sizeof(int));
	if(data == NULL)
		return NULL;

	int lut[256][16];
	int i, j, k;
	for(i = 0; i < model->listSize; i++){
		double x = model->vertexArray[i * 3];
		double y = model->vertexArray[i * 3 + 1];
		double z = model->vertexArray[i * 3 + 2];

		int cubeX = min((int) floor(x / cubeSize) + 1, cubesPerDimension - 2);
		int cubeY = min((int) floor(y / cubeSize) + 1, cubesPerDimension - 2);
		int cubeZ = min((int) floor(z / cubeSize) + 1, cubesPerDimension - 2);

		set3DValue(data, cubeX, cubeY, cubeZ, cubesPerDimension, 1);
	}
	
	MCM *mcm = initMCM();
	loadLookUpTable(lut, lutFileName);

	printf("Inicio do processamento.\n");
	for(i = 0; i < cubesPerDimension - 1; i++){
		for(j = 0; j < cubesPerDimension - 1; j++){
			for(k = 0; k < cubesPerDimension - 1; k++){
				int cubeID = 0;
				if(get3DValue(data, i, j, k, cubesPerDimension))
					cubeID |= 1;

				if(get3DValue(data, i + 1, j, k, cubesPerDimension))
					cubeID |= 2;

				if(get3DValue(data, i + 1, j, k + 1, cubesPerDimension))
					cubeID |= 4;

				if(get3DValue(data, i, j, k + 1, cubesPerDimension))
					cubeID |= 8;

				if(get3DValue(data, i, j + 1, k, cubesPerDimension))
					cubeID |= 16;

				if(get3DValue(data, i + 1, j + 1, k, cubesPerDimension))
					cubeID |= 32;

				if(get3DValue(data, i + 1, j + 1, k + 1, cubesPerDimension))
					cubeID |= 64;

				if(get3DValue(data, i, j + 1, k + 1, cubesPerDimension))
					cubeID |= 128;

				generateAndInsertTriangles(&mcm, i, j, k, lut[cubeID], cubeSize);
			}
		}
	}
	free(data);
	printf("Fim do processamento.\n");
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
		double minX = INF, minY = INF, minZ = INF;
		double maxX = 0, maxY = 0, maxZ = 0;
		int listPosition = 0, i;
		double x, y, z;
		
		model = initXYZ(listSize);
		while(fscanf(file, "%lf %lf %lf", &x, &y, &z) == 3){
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

	drawUnitaryBox();

	glColor3f(1.0, 1.0, 1.0);
	if(pointCloudVisualization){
		glPushMatrix();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_DOUBLE, 0, model->vertexArray);
			glDrawArrays(GL_POINTS, 0, model->listSize);
			glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();
	}
	else{
		printf("MCM: %d faces e %d vértices.\n", mcm->facesCount / 3, mcm->vertexCount / 3);
		glPushMatrix();
			// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_DOUBLE, 0, mcm->vertexArray);
			glDrawElements(GL_TRIANGLES, mcm->facesCount, GL_UNSIGNED_INT, mcm->faces);
			glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
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
	glEnable(GL_POLYGON_SMOOTH);
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

	freeMCM(&mcm);
	freeXYZ(&model);
	return 0;
}