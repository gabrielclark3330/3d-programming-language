#include <glew.h>
#include <glfw3.h>
#include <glfw3native.h>
#include <iostream> 
#include <math.h> 
static GLfloat camXAngle = 0.0;
static GLfloat camYAngle = 0.0;
static GLfloat camZAngle = 0.0;
static GLfloat camXPos = 0;
static GLfloat camYPos = 0;
static GLfloat camZPos = -7;
static int codeCubeIndex[3] = {0,0,0};
static int codeCubeStartSelectIndex[3] = { -1,-1,-1 };
static int codeCubeEndSelectIndex[3] = { -1,-1,-1 };
static int codeBoxBounds[3] = {3,3,3};
static char codeBox[80][80][80];
enum Mode {
    insert,
    normal,
    visual
};
static Mode mode = normal;

char getCharAtIndex(int *index) {return codeBox[index[0]][index[1]][index[2]];}
void setCharAtIndex(int *index, char ch) {codeBox[index[0]][index[1]][index[2]]=ch;}

struct pastelColors {
    // Colors are {red,green,blue} 0-255
    float blue[3] = {(float)38/255,(float)70/255,(float)83/255};
    float green[3] = {(float)42/255,(float)157/255,(float)143/255};
    float yellow[3] = {(float)233/255,(float)196/255,(float)106/255};
    float orange[3] = {(float)244/255,(float)162/255,(float)97/255};
    float red[3] = {(float)231/255,(float)111/255,(float)81/255};
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
    float ratio = width / height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glFrustum(-1.0*ratio, 1.0* ratio, -1.0*ratio, 1.0*ratio, .9, 40);
	glViewport(0, 0, width, height);
}

void connect2Points(GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2) {
    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glEnd();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (mode==normal) {
		// Camera movement keys
		switch (key) {
		case GLFW_KEY_W: camXAngle += 2; break;
		case GLFW_KEY_S: camXAngle -= 2; break;
		case GLFW_KEY_D: camYAngle += 2; break;
		case GLFW_KEY_A: camYAngle -= 2; break;
		case GLFW_KEY_O: camZPos += .5; break;
		case GLFW_KEY_P: camZPos -= .5; break;
		// Selection movement keys
		case GLFW_KEY_H: 
			if (codeCubeIndex[0] > 0 && action==GLFW_PRESS) { 
				codeCubeIndex[0] -= 1; 
			} break;
		case GLFW_KEY_L: 
			if (codeCubeIndex[0] < codeBoxBounds[0]-1 && action==GLFW_PRESS) { 
				codeCubeIndex[0] += 1; 
			} break;
		case GLFW_KEY_J: 
			if (codeCubeIndex[1] > 0 && action==GLFW_PRESS) { 
				codeCubeIndex[1] -= 1; 
			} break;
		case GLFW_KEY_K:
			if (codeCubeIndex[1] < codeBoxBounds[1]-1 && action==GLFW_PRESS) { 
				codeCubeIndex[1] += 1; 
			} break;
		case GLFW_KEY_E: 
			if (codeCubeIndex[2] > 0 && action==GLFW_PRESS) { 
				codeCubeIndex[2] -= 1; 
			} break;
		case GLFW_KEY_R: 
			if (codeCubeIndex[2] < codeBoxBounds[2]-1 && action==GLFW_PRESS) { 
				codeCubeIndex[2] += 1; 
			} break;
		// Editor mode keys
		case GLFW_KEY_I: mode = insert; break;

		}
	}
	else if (mode==insert) {
        switch (key) {
		// Editor mode keys
		case GLFW_KEY_ESCAPE: mode = normal; break;
            
        }
	}
}

void characterCallback(GLFWwindow* window, unsigned int codepoint) {
    if (mode==insert) {
        setCharAtIndex(codeCubeIndex, (char)codepoint);
    }
    
    printf("%c", getCharAtIndex(codeCubeIndex));
}

void setKeyBindings(GLFWwindow* window) {
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, characterCallback);
}

void drawCube(int x, int y, int z) {
    // (x,y,z) gives the smallest value on cube
    // it is the lower leftmost closest point
    // lines going x->(x+1)
    connect2Points(x,y,z, x+1,y,z);
    connect2Points(x,y+1,z, x+1,y+1,z);
    connect2Points(x,y,z+1, x+1,y,z+1);
    connect2Points(x,y+1,z+1, x+1,y+1,z+1);
    // lines going z->(z+1)
    connect2Points(x,y,z, x,y,z+1);
    connect2Points(x+1,y,z, x+1,y,z+1);
    connect2Points(x,y+1,z, x,y+1,z+1);
    connect2Points(x+1,y+1,z, x+1,y+1,z+1);
    // lines going y->(y+1)
    connect2Points(x, y, z, x, y+1, z);
    connect2Points(x+1, y, z, x+1, y+1, z);
    connect2Points(x,y,z+1, x,y+1,z+1);
    connect2Points(x+1,y,z+1, x+1,y+1,z+1);
}

void visCubes(int *start, int *end) {
    for (int x = start[0]; x < end[0]; x++) {
		for (int y = start[1]; y < end[1]; y++) {
			for (int z = start[2]; z < end[2]; z++) {
                drawCube(x,y,z);
			}
		}
    }
}

int main(void)
{
    pastelColors pastelColors;
    GLFWwindow* window;
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1200, 1200, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glewInit();
    setKeyBindings(window);
    
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		/* Set up perspective matrix */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(-1,1,-1,1, .9, 40);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(camXPos, camYPos, camZPos);
        glRotatef(camXAngle, 1,0,0);
        glRotatef(camYAngle, 0,1,0);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(pastelColors.blue[0], pastelColors.blue[1], pastelColors.blue[2], 1.0);        
        visCubes(new int[] {0,0,0}, codeBoxBounds);
        glColor3f(pastelColors.orange[0],pastelColors.orange[1],pastelColors.orange[2]);
        drawCube(codeCubeIndex[0],codeCubeIndex[1],codeCubeIndex[2]);
        glColor3f(1,1,1);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
