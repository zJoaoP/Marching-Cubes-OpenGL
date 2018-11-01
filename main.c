#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>

typedef struct Model{
	// Definir vetor de pontos.
	// Chamar função de normalização.
	// 
	int x;
} Model;

Model *model = NULL;
float ratio = 0.0;

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
		int listSize = lineCount(fileName);


		fclose(file);
	}
	return model;
}

void draw(){
	glClear(GL_COLOR_BUFFER_BIT);
	gluLookAt(1.0, 0.0, 0.0, 0, 0, 0, 0, 1, 0);

	glFlush();
}

void reshape(int w, int h){
	ratio = (float) w / h;
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

	gluPerspective(10.0f, ratio, 0.1, 50);
	glMatrixMode(GL_MODELVIEW);
}

void initScene(){
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0, 0.0, 0.5, 0.0);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
}

int main(int argc, char *argv[]){
	if(argc != 4){
		printf("Uso correto: %s [nuvem_de_pontos.xyz] [look-up_table.txt] [lado do cubo]\n", argv[0]);
		return 1;
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

	glutCreateWindow("CG - Marching Cubes!");
	glutDisplayFunc(draw);
	
	initScene();

	model = readXYZFile(argv[1]);

	glutMainLoop();
	return 0;
}