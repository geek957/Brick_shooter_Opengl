#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assert.h>
#include <ao/ao.h>
#include <cstring>
#include <string>
#include <stdlib.h>

using namespace std;

static const int BUF_SIZE = 4096;

struct WavHeader {
    char id[4]; //should contain RIFF
    int32_t totalLength;
    char wavefmt[8];
    int32_t format; // 16 for PCM
    int16_t pcm; // 1 for PCM
    int16_t channels;
    int32_t frequency;
    int32_t bytesPerSecond;
    int16_t bytesByCapture;
    int16_t bitsPerSample;
    char data[4]; // "data"
    int32_t bytesInData;
};

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;

GLFWwindow* window;


int score=0;

int penalty=20;

int quitflag=0;

int shoot_sound=0;

double prvxpos;
float guny=0;
int moveflag=0;

float rotatation_angle=0;
int rotateflag=0;


float bottomred=-1.11;
int bottomredflag=0;

float bottomgreen=1.40;
int bottomgreenflag=0;

float bottomrectre=-1.25;

float bottomrectgr=1.25;

float speed_bricks=0.0005;

float mirror1_angle=(M_PI*2.0)/12.0;

float mirror2_angle=-1*(M_PI*2.0)/12.0;

float screenxn=-4.0f,screenxp=4.0f,screenyn=-4.0f,screenyp=4.0f;

int flag_screenzoom=0;
int flag_screenpan=0;
int flag_screenmousepan=0;
int flag_control=0;
int flag_shift=0;
int flag_laserrelease=0;
int close_screen=0;

int collision_sound=0;

float x11=3.5+(0.6*cos((4*M_PI)/6.0)),x12=3.5+(0.6*cos(-2*M_PI/6.0)),y11=2.5+(0.6*sin((4*M_PI)/6.0)),y12=2.5+(0.6*sin(-2*M_PI/6.0));
float x21=3.5+(0.6*cos((4*M_PI)/3.0)),x22=3.5+(0.6*cos(M_PI/3.0)),y21=-2.5+(0.6*sin((4*M_PI)/3.0)),y22=-2.5+(0.6*sin(M_PI/3.0));
double last_time = glfwGetTime(), curr_time;


static GLfloat red_block[18*100000];
static GLfloat green_block[18*100000];
static GLfloat blue_block[18*100000];

int blue_no_blocks=0;
int red_no_blocks=0;
int green_no_blocks=0;

static  GLfloat red_color_block[18*100000];
static  GLfloat green_color_block[18*100000];
static  GLfloat blue_color_block[18*100000];

float laser_radius=0.08;
float laser_translate=0.005;

static GLfloat laser[1000*8*9];

int laser_no=0;

float laser_angle[100];

int laser_latest[100]={0};

int flag_pressleft=0;
int flag_pressright=0;
int flag_movement=0;

int flag_brickspeed=0;


static GLfloat laser_color[1000*8*9];

void createlaser();
float disst(float x1,float y1,float x2,float y2)
{
    return sqrt(((x2-x1)*(x2-x1))+((y2-y1)*(y2-y1)));
}
char* itoa(int i)
{
    char const digit[] = "0123456789";
    static char b[100];
    char* p=b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ 
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ 
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}
void getprvxpos()
{
    double xpos,ypos;
    glfwGetCursorPos(window,&xpos,&ypos);
    xpos=((xpos*(8.0))/700.0)-4.0;
    prvxpos=xpos;
}
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    //    exit(EXIT_SUCCESS);
}
/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    return vao;
}
/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}
/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_S:
                moveflag=0;
                break;
            case GLFW_KEY_F:
                moveflag=0;
                break;
            case GLFW_KEY_A:
                rotateflag=0;
                break;
            case GLFW_KEY_D:
                rotateflag=0;
                break;
            case GLFW_KEY_N:
                flag_brickspeed=0;
                break;
            case GLFW_KEY_M:
                flag_brickspeed=0;
                break;
            case GLFW_KEY_LEFT_SHIFT:
                {
                    flag_shift=0;bottomgreenflag=0;bottomredflag=0;
                    break;
                }
            case GLFW_KEY_RIGHT_SHIFT:
                {
                    flag_shift=0;bottomgreenflag=0;bottomredflag=0;
                    break;
                }
            case GLFW_KEY_LEFT_CONTROL:
                {
                    flag_control=0;bottomgreenflag=0;bottomredflag=0;
                    break;
                }
            case GLFW_KEY_RIGHT_CONTROL:
                {
                    flag_control=0;bottomgreenflag=0;bottomredflag=0;
                    break;
                }
            case GLFW_KEY_UP:
                flag_screenzoom=0;
                break;
            case GLFW_KEY_DOWN:
                flag_screenzoom=0;
                break;
            case GLFW_KEY_LEFT:
                {
                    flag_screenpan=0;bottomgreenflag=0;bottomredflag=0;
                    break;
                }
            case  GLFW_KEY_RIGHT:
                {
                    flag_screenpan=0;bottomgreenflag=0;bottomredflag=0;
                    break;
                }
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) 
    {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_S:
                moveflag=1;
                break;
            case GLFW_KEY_F:
                moveflag=-1;
                break;
            case GLFW_KEY_A:
                rotateflag=1;
                break;
            case GLFW_KEY_D:
                rotateflag=-1;
                break;
            case GLFW_KEY_N:
                flag_brickspeed=1;
                break;
            case GLFW_KEY_M:
                flag_brickspeed=-1;
                break;
            case GLFW_KEY_LEFT_SHIFT:
                flag_shift=1;
                break;
            case GLFW_KEY_RIGHT_SHIFT:
                flag_shift=1;
                break;
            case GLFW_KEY_LEFT_CONTROL:
                flag_control=1;
                break;
            case GLFW_KEY_RIGHT_CONTROL:
                flag_control=1;
                break;
            case GLFW_KEY_UP:
                flag_screenzoom=1;
                break;
            case GLFW_KEY_DOWN:
                flag_screenzoom=-1;
                break;
            case GLFW_KEY_RIGHT:
                {
                    if(flag_control==1)
                        bottomredflag=1;
                    else if(flag_shift==1)
                        bottomgreenflag=1;
                    else
                        flag_screenpan=1;
                    break;
                }
            case GLFW_KEY_LEFT:
                {
                    if(flag_control==1)
                        bottomredflag=-1;
                    else if(flag_shift==1)
                        bottomgreenflag=-1;
                    else
                        flag_screenpan=-1;
                    break;
                }
            case GLFW_KEY_SPACE:
                {
                    if(flag_laserrelease==1)
                    {
                        createlaser();
                        flag_laserrelease=0;	
                    }
                }
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
        	close_screen=1000;
            quit(window);
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS)
                flag_pressleft=1;
            else if (action == GLFW_RELEASE)
            {
                flag_pressleft=0;
                flag_movement=0;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if(action == GLFW_PRESS)
            {
                getprvxpos ();
                flag_screenmousepan=1;
            }
            else if (action == GLFW_RELEASE)
            {
                flag_screenmousepan=0;
                flag_screenpan=0;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
       glLoadIdentity ();
       gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(screenxn,screenxp,screenyn, screenyp, 0.1f, 500.0f);
}

VAO *cross1,*cross2,*segment[17],*Laser, *mirror1, *mirror2, *Greenblocks, *Redblocks, *Blueblocks, *triangle, *rectangle, *circlel1 ,*hovercirclel1, *bottomrectred ,*hoverbottomrectred , *bottomrectgreen ,*hoverbottomrectgreen,*bottomredcircle1 , *  bottomgreencircle1;

// Creates the triangle object used in this sample code
void createcross1()
{
    static const GLfloat vertex_buffer_data [] = {
        -0.1,-0.1,0, // vertex 1
        0.1,0.1,0, // vertex 2
    };

    static  GLfloat color_buffer_data []={
        0,0,0,
        0,0,0
    };
    cross2 = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINE);
}
void createcross2()
{
    static const GLfloat vertex_buffer_data [] = {
        0.1,-0.1,0, // vertex 1
        -0.1,0.1,0, // vertex 2
    };

    static  GLfloat color_buffer_data []={
        0,0,0,
        0,0,0
    };
    cross1 = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINE);
}
void createlaser ()
{
    shoot_sound=1;
    float teta=(M_PI*45.0)/180.0;
    float xx=teta;
    for(int i=0;i<8;i++)
    {
        laser[(laser_no*72)+9*i+0]=-3.5+(1.3*cos(rotatation_angle));laser[(laser_no*72)+9*i+1]=guny+(1.3*sin(rotatation_angle));laser[(laser_no*72)+9*i+2]=0;
        laser[(laser_no*72)+9*i+3]=laser[(laser_no*72)+9*i+0]+(laser_radius*cos(teta-(xx)));laser[(laser_no*72)+9*i+4]=laser[(laser_no*72)+9*i+1]+(laser_radius*sin(teta-(xx)));laser[(laser_no*72)+9*i+5]=0;
        laser[(laser_no*72)+9*i+6]=laser[(laser_no*72)+9*i+0]+(laser_radius*cos(teta));laser[(laser_no*72)+9*i+7]=laser[(laser_no*72)+9*i+1]+(laser_radius*sin(teta));laser[(laser_no*72)+9*i+8]=0;
        teta+=xx;	  
        for(int j=0;j<9;j+=3)
        {
            laser_color[(laser_no*72)+9*i+3*j+0]=153.0/255.0;
            laser_color[(laser_no*72)+9*i+3*j+1]=51.0/255.0;
            laser_color[(laser_no*72)+9*i+j*j+2]=255.0/255.0;
        }
    }
    laser_angle[laser_no]=rotatation_angle;
    laser_no++;
    Laser = create3DObject(GL_TRIANGLES, laser_no*8*3 , laser, laser_color, GL_FILL);
}
void createblock(float x,int type)
{
    float y=3.4;
    if(type==0)
    {
        red_block[18*red_no_blocks+0]=x;red_block[18*red_no_blocks+1]=y+0.2;red_block[18*red_no_blocks+2]=0;
        red_block[18*red_no_blocks+3]=x+0.2;red_block[18*red_no_blocks+4]=y+0.2;red_block[18*red_no_blocks+5]=0;
        red_block[18*red_no_blocks+6]=x;red_block[18*red_no_blocks+7]=y;red_block[18*red_no_blocks+8]=0;
        red_block[18*red_no_blocks+9]=x+0.2;red_block[18*red_no_blocks+10]=y+0.2;red_block[18*red_no_blocks+11]=0;
        red_block[18*red_no_blocks+12]=x+0.2;red_block[18*red_no_blocks+13]=y;red_block[18*red_no_blocks+14]=0;
        red_block[18*red_no_blocks+15]=x;red_block[18*red_no_blocks+16]=y;red_block[18*red_no_blocks+17]=0;
        for(int i=0;i<18;i+=3)
        {
            red_color_block[18*red_no_blocks+i]=1;
        }
        red_no_blocks++;
    }
    else if(type==1)
    {
        green_block[18*green_no_blocks+0]=x;green_block[18*green_no_blocks+1]=y+0.2;green_block[18*green_no_blocks+2]=0;
        green_block[18*green_no_blocks+3]=x+0.2;green_block[18*green_no_blocks+4]=y+0.2;green_block[18*green_no_blocks+5]=0;
        green_block[18*green_no_blocks+6]=x;green_block[18*green_no_blocks+7]=y;green_block[18*green_no_blocks+8]=0;
        green_block[18*green_no_blocks+9]=x+0.2;green_block[18*green_no_blocks+10]=y+0.2;green_block[18*green_no_blocks+11]=0;
        green_block[18*green_no_blocks+12]=x+0.2;green_block[18*green_no_blocks+13]=y;green_block[18*green_no_blocks+14]=0;
        green_block[18*green_no_blocks+15]=x;green_block[18*green_no_blocks+16]=y;green_block[18*green_no_blocks+17]=0;
        for(int i=type;i<18;i+=3)
        {
            green_color_block[18*green_no_blocks+i]=1;
        }
        green_no_blocks++;
    }
    else if(type==2)
    {
        blue_block[18*blue_no_blocks+0]=x;blue_block[18*blue_no_blocks+1]=y+0.2;blue_block[18*blue_no_blocks+2]=0;
        blue_block[18*blue_no_blocks+3]=x+0.2;blue_block[18*blue_no_blocks+4]=y+0.2;blue_block[18*blue_no_blocks+5]=0;
        blue_block[18*blue_no_blocks+6]=x;blue_block[18*blue_no_blocks+7]=y;blue_block[18*blue_no_blocks+8]=0;
        blue_block[18*blue_no_blocks+9]=x+0.2;blue_block[18*blue_no_blocks+10]=y+0.2;blue_block[18*blue_no_blocks+11]=0;
        blue_block[18*blue_no_blocks+12]=x+0.2;blue_block[18*blue_no_blocks+13]=y;blue_block[18*blue_no_blocks+14]=0;
        blue_block[18*blue_no_blocks+15]=x;blue_block[18*blue_no_blocks+16]=y;blue_block[18*blue_no_blocks+17]=0;
        for(int i=0;i<18;i+=1)
        {
            blue_color_block[18*blue_no_blocks+i]=0;
        }
        blue_no_blocks++;
    }
    Redblocks = create3DObject(GL_TRIANGLES, 6*red_no_blocks, red_block, red_color_block, GL_FILL);
    Greenblocks = create3DObject(GL_TRIANGLES, 6*green_no_blocks, green_block, green_color_block, GL_FILL);
    Blueblocks = create3DObject(GL_TRIANGLES, 6*blue_no_blocks, blue_block, blue_color_block, GL_FILL);
}
void createBottomrectred ()
{
    static const GLfloat vertex_buffer_data [] = {
        -0.5,-0.5,0, // vertex 1
        0.8,-0.5,0, // vertex 2
        0.8, 0.5,0, // vertex 3

        0.8,0.5,0, // vertex 3
        -0.5, 0.5,0, // vertex 4
        -0.5,-0.5,0  // vertex 1
    };

    static  GLfloat color_buffer_data [18];
    for(int i=0;i<18;i+=3)
        color_buffer_data[i]=1;
    bottomrectred = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void HovercreateBottomrectred ()
{
    static const GLfloat vertex_buffer_data [] = {
        -0.5,-0.5,0, // vertex 1
        0.8,-0.5,0, // vertex 2
        0.8, 0.5,0, // vertex 3

        0.8,0.5,0, // vertex 3
        -0.5, 0.5,0, // vertex 4
        -0.5,-0.5,0  // vertex 1
    };

    static  GLfloat color_buffer_data [18];
    for(int i=0;i<18;i+=3)
        color_buffer_data[i]=1.0/3.0;
    hoverbottomrectred = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBottomrectgreen ()
{
    static const GLfloat vertex_buffer_data [] = {
        -0.5,-0.5,0, // vertex 1
        0.8,-0.5,0, // vertex 2
        0.8, 0.5,0, // vertex 3

        0.8,0.5,0, // vertex 3
        -0.5, 0.5,0, // vertex 4
        -0.5,-0.5,0  // vertex 1
    };

    static GLfloat color_buffer_data [18];
    for(int i=1;i<18;i+=3)
        color_buffer_data[i]=1;
    bottomrectgreen = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void HovercreateBottomrectgreen ()
{
    static const GLfloat vertex_buffer_data [] = {
        -0.5,-0.5,0, // vertex 1
        0.8,-0.5,0, // vertex 2
        0.8, 0.5,0, // vertex 3

        0.8,0.5,0, // vertex 3
        -0.5, 0.5,0, // vertex 4
        -0.5,-0.5,0  // vertex 1
    };

    static GLfloat color_buffer_data [18];
    for(int i=1;i<18;i+=3)
        color_buffer_data[i]=1.0/3.0;
    hoverbottomrectgreen = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createRectangle ()
{
    static const GLfloat vertex_buffer_data [] = {
        -0.3,-0.15,0, // vertex 1
        1.0,-0.15,0, // vertex 2
        1.0, 0.15,0, // vertex 3

        1.0,0.15,0, // vertex 3
        -0.3, 0.15,0, // vertex 4
        -0.3,-0.15,0  // vertex 1
    };

    static  GLfloat color_buffer_data[18] ;
    for(int i=0;i<18;i+=1)
    {
    	color_buffer_data[3*i+0]=55.0/255.0;
    	color_buffer_data[3*i+1]=51.0/255.0;
    	color_buffer_data[3*i+2]=253.0/255.0;
    }
    rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createmirror1 ()
{
    static const GLfloat vertex_buffer_data [] = {
        -0.05,-0.6,0, // vertex 1
        0.05,-0.6,0, // vertex 2
        0.05, 0.6,0, // vertex 3

        0.05,0.6,0, // vertex 3
        -0.05, 0.6,0, // vertex 4
        -0.05,-0.6,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0,0.7,1, // color 1
        0,0.7,1, // color 2
        0,0.7,1, // color 3

        0,0.7,1, // color 3
        0,0.7,1, // color 4
        0,0.7,1  // color 1
    };
    mirror1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createmirror2 ()
{
    static const GLfloat vertex_buffer_data [] = {
        -0.05,-0.6,0, // vertex 1
        0.05,-0.6,0, // vertex 2
        0.05, 0.6,0, // vertex 3

        0.05,0.6,0, // vertex 3
        -0.05, 0.6,0, // vertex 4
        -0.05,-0.6,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        0,0.7,1, // color 1
        0,0.7,1, // color 2
        0,0.7,1, // color 3

        0,0.7,1, // color 3
        0,0.7,1, // color 4
        0,0.7,1  // color 1
    };
    mirror2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createCirclel1 ()
{
    static GLfloat vertex_buffer_data[720*9+10]={0};
    float teta=M_PI/360.0;
    float teta1=0.5;
    for(int i=0;i<360.0/teta1;i++)
    {
        vertex_buffer_data[9*i+0]=0.3*cos(teta-(M_PI/360.0));
        vertex_buffer_data[9*i+1]=0.3*sin(teta-(M_PI/360.0));
        vertex_buffer_data[9*i+3]=0.3*cos(teta);
        vertex_buffer_data[9*i+4]=0.3*sin(teta);
        teta+=M_PI/360.0;
    }

    static GLfloat color_buffer_data[720*9+10]={0};
    for(int i=0;i<360.0/teta1;i++)
    {
        color_buffer_data[9*i+0]=1;
        for(int j=1;j<9;j++)
            color_buffer_data[9*i+j]=0;
    }
    circlel1 = create3DObject(GL_TRIANGLES, (360.0/teta1)*3, vertex_buffer_data, color_buffer_data, GL_LINE);
}
void HovercreateCirclel1 ()
{
    static GLfloat vertex_buffer_data[720*9+10]={0};
    float teta=M_PI/360.0;
    float teta1=0.5;
    for(int i=0;i<360.0/teta1;i++)
    {
        vertex_buffer_data[9*i+0]=0.3*cos(teta-(M_PI/360.0));
        vertex_buffer_data[9*i+1]=0.3*sin(teta-(M_PI/360.0));
        vertex_buffer_data[9*i+3]=0.3*cos(teta);
        vertex_buffer_data[9*i+4]=0.3*sin(teta);
        teta+=M_PI/360.0;
    }

    static GLfloat color_buffer_data[720*9+10]={0};
    for(int i=0;i<360.0/teta1;i++)
    {
        color_buffer_data[9*i+0]=0;
        color_buffer_data[9*i+1]=1.0;
        for(int j=2;j<9;j++)
            color_buffer_data[9*i+j]=0;
    }
    hovercirclel1 = create3DObject(GL_TRIANGLES, (360.0/teta1)*3, vertex_buffer_data, color_buffer_data, GL_LINE);
}
void createBottomgreencircle1 ()
{
    static GLfloat vertex_buffer_data[720*9+10]={0};
    float teta=M_PI/360.0;
    float teta1=0.5;
    float r=0.65;
    for(int i=0;i<360.0/teta1;i++)
    {
        vertex_buffer_data[9*i+0]=r*cos(teta-(M_PI/360.0));
        vertex_buffer_data[9*i+1]=r*sin(teta-(M_PI/360.0));
        vertex_buffer_data[9*i+2]=0;
        vertex_buffer_data[9*i+3]=r*cos(teta);
        vertex_buffer_data[9*i+4]=r*sin(teta);
        vertex_buffer_data[9*i+5]=0;
        teta+=M_PI/360.0;
    }

    static GLfloat color_buffer_data[720*9+10]={0};
    for(int i=0;i<360.0/teta1;i++)
    {
        for(int j=1;j<9;j+=3)
            color_buffer_data[9*i+j]=1;
    }
    bottomgreencircle1 = create3DObject(GL_TRIANGLES, (360.0/teta1)*3, vertex_buffer_data, color_buffer_data, GL_LINE);
}
void createBottomredcircle1 ()
{
    static GLfloat vertex_buffer_data[720*9+10]={0};
    float teta=M_PI/360.0;
    float teta1=0.5;
    float r=0.65;
    for(int i=0;i<360.0/teta1;i++)
    {
        vertex_buffer_data[9*i+0]=r*cos(teta-(M_PI/360.0));
        vertex_buffer_data[9*i+1]=r*sin(teta-(M_PI/360.0));
        vertex_buffer_data[9*i+2]=0;
        vertex_buffer_data[9*i+3]=r*cos(teta);
        vertex_buffer_data[9*i+4]=r*sin(teta);
        vertex_buffer_data[9*i+5]=0;
        teta+=M_PI/360.0;
    }

    static GLfloat color_buffer_data[720*9+10]={0};
    for(int i=0;i<360.0/teta1;i++)
    {
        for(int j=0;j<9;j+=3)
            color_buffer_data[9*i+j]=1;
    }
    bottomredcircle1 = create3DObject(GL_TRIANGLES, (360.0/teta1)*3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

float camera_rotation_angle = 90;

void updateblockpos()
{
    for(int i=0;i<red_no_blocks;i++)
    {
        for(int j=1;j<18;j+=3)
            red_block[18*i+j]-=speed_bricks;
    }
    for(int i=0;i<green_no_blocks;i++)
    {
        for(int j=1;j<18;j+=3)
            green_block[18*i+j]-=speed_bricks;
    }
    for(int i=0;i<blue_no_blocks;i++)
    {
        for(int j=1;j<18;j+=3)
            blue_block[18*i+j]-=speed_bricks;
    }
    Redblocks = create3DObject(GL_TRIANGLES, 6*red_no_blocks, red_block, red_color_block, GL_FILL);
    Greenblocks = create3DObject(GL_TRIANGLES, 6*green_no_blocks, green_block, green_color_block, GL_FILL);
    Blueblocks = create3DObject(GL_TRIANGLES, 6*blue_no_blocks, blue_block, blue_color_block, GL_FILL);
}

void updatelaserspos()
{

    for(int i=0;i<laser_no;i++)
    {
        float teta=(M_PI*45.0)/180.0;
        float xx=teta;
        for(int j=0;j<8;j++)
        {
            laser[(i*72)+9*j+0]=laser[(i*72)+9*j+0]+(laser_translate*cos(laser_angle[i]));laser[(i*72)+9*j+1]=laser[(i*72)+9*j+1]+(laser_translate*sin(laser_angle[i]));
            laser[(i*72)+9*j+3]=laser[(i*72)+9*j+0]+(laser_radius*cos(teta-(xx)));laser[(i*72)+9*j+4]=laser[(i*72)+9*j+1]+(laser_radius*sin(teta-(xx)));
            laser[(i*72)+9*j+6]=laser[(i*72)+9*j+0]+(laser_radius*cos(teta));laser[(i*72)+9*j+7]=laser[(i*72)+9*j+1]+(laser_radius*sin(teta));
            teta+=xx;
        }
    }
    Laser = create3DObject(GL_TRIANGLES, laser_no*8*3 , laser, laser_color, GL_FILL);
}
void function_segment()
{
    static  GLfloat vertex_buffer_data [] = {
        -0.18,0.05,0, 0.18,0.05,0,  -0.18, -0.05,0, 

        0.18,-0.05,0, 0.18, 0.05,0, -0.18,-0.05,0  
    };

    static  GLfloat color_buffer_data [] = {
        1,1,1, 1,1,1, 1,1,1, 

        1,1,1, 1,1,1, 1,1,1  
    };
    segment[0] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=1;i<18;i+=3)
        vertex_buffer_data[i]+=0.36;
    segment[1] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=1;i<18;i+=3)
        vertex_buffer_data[i]-=0.72;
    segment[2] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=0;i<18;i+=3)
        vertex_buffer_data[i]+=0.44;
    segment[7] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=1;i<18;i+=3)
        vertex_buffer_data[i]+=0.36;
    segment[8] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=1;i<18;i+=3)
        vertex_buffer_data[i]+=0.36;
    segment[9] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    static GLfloat ertex_buffer_data[]={
        0.28,0.36,0, 0.28,0.05,0, 0.18,0.05,0,
        0.18,0.36,0, 0.28,0.36,0, 0.18,0.05,0
    };
    segment[3] = create3DObject(GL_TRIANGLES, 6, ertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=0;i<18;i+=3)
        ertex_buffer_data[i]-=0.46;
    segment[4] = create3DObject(GL_TRIANGLES, 6, ertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=1;i<18;i+=3)
        ertex_buffer_data[i]-=0.36;
    segment[5] = create3DObject(GL_TRIANGLES, 6, ertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=0;i<18;i+=3)
        ertex_buffer_data[i]+=0.46;
    segment[6] = create3DObject(GL_TRIANGLES, 6, ertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=0;i<18;i+=3)
        ertex_buffer_data[i]+=0.44;
    segment[10] = create3DObject(GL_TRIANGLES, 6, ertex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=1;i<18;i+=3)
        ertex_buffer_data[i]+=0.36;
    segment[11] = create3DObject(GL_TRIANGLES, 6, ertex_buffer_data, color_buffer_data, GL_FILL);
    static GLfloat rtex_buffer_data[]={
        0.26,-0.31,0, 0.26,-0.41,0,                                              0.26+(0.51*cos(M_PI/4.0)),-0.31+(0.51*sin(M_PI/4.0)),0,
        0.26,-0.41,0, 0.26+(0.51*cos(M_PI/4.0)),-0.31+(0.51*sin(M_PI/4.0)),0,     0.26+(0.51*cos(M_PI/4.0)),-0.41+(0.51*sin(M_PI/4.0)),0
    };
    segment[12] = create3DObject(GL_TRIANGLES, 6, rtex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=1;i<18;i+=3)
        rtex_buffer_data[i]+=0.36;
    segment[13] = create3DObject(GL_TRIANGLES, 6, rtex_buffer_data, color_buffer_data, GL_FILL);
    static GLfloat tex_buffer_data[]={
        0.20,-0.31,0, 0.20,-0.41,0,                                              0.20+(0.51*cos(3*M_PI/4.0)),-0.31+(0.51*sin(3*M_PI/4.0)),0,
        0.20,-0.41,0, 0.20+(0.51*cos(3*M_PI/4.0)),-0.31+(0.51*sin(3*M_PI/4.0)),0,    0.20+(0.51*cos(3*M_PI/4.0)),-0.41+(0.51*sin(3*M_PI/4.0)),0
    };
    segment[14] = create3DObject(GL_TRIANGLES, 6, tex_buffer_data, color_buffer_data, GL_FILL);
    for(int i=1;i<18;i+=3)
        tex_buffer_data[i]+=0.36;
    segment[15] = create3DObject(GL_TRIANGLES, 6, tex_buffer_data, color_buffer_data, GL_FILL);
}
void create(float xx,float yy,float zz,float angle,float rx,float ry,float rz,VAO *shape)
{
    glm::mat4 VP = Matrices.projection * Matrices.view;
    glm::mat4 MVP;
    Matrices.model = glm::mat4(1.0f);
    // glm::mat4 scaler = glm::scale(glm::vec3(2,3,5));
    glm::mat4 translate = glm::translate (glm::vec3(xx, yy , zz));        // glTranslatef
    glm::mat4 rotate = glm::rotate((float)(angle), glm::vec3(rx,ry,rz)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate * rotate);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(shape);
}
void print(VAO *shape)
{
    glm::mat4 VP = Matrices.projection * Matrices.view;
    glm::mat4 MVP;
    Matrices.model = glm::mat4(1.0f);
    MVP = VP;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(shape);
}
void print_onscreen(char str[],int type)
{
    char list[16][22]={
        {' ',' ','2','3','4','5','6',' ','8','9','A',' ','E',' ',' ',' ',' ','P','R','S',' ',' '},//0
        {'0',' ','2','3',' ','5','6','7','8','9','A','C','E','G',' ',' ','O','P','R','S',' ',' '},//1
        {'0',' ','2','3',' ','5','6',' ','8',' ',' ','C','E','G','L',' ','O',' ',' ','S',' ',' '},//2
        {'0','1','2','3','4',' ',' ','7','8','9','A',' ',' ',' ',' ',' ','O','P','R',' ',' ',' '},//3
        {'0',' ',' ',' ','4','5','6',' ','8','9','A','C','E','G','L','M','O','P','R','S','V',' '},//4
        {'0',' ','2',' ',' ',' ','6',' ','8',' ','A','C','E','G','L','M','O','P','R',' ',' ',' '},//5
        {'0','1',' ','3','4','5','6','7','8','9','A',' ',' ','G',' ',' ','O',' ',' ','S',' ','Y'},//6
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},//7
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','G',' ',' ',' ',' ',' ',' ',' ',' '},//8
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','G',' ',' ',' ',' ',' ',' ',' ',' '},//9
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','G',' ','M',' ',' ',' ',' ',' ',' '},//10
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','M',' ',' ',' ',' ','V',' '},//11
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','V',' '},//12
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','M',' ',' ',' ',' ',' ','Y'},//13
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','R',' ','V',' '},//14
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','M',' ',' ',' ',' ',' ','Y'},//15
    };
    int len=strlen(str);
    float posy,posx;
    if(type==1)
        posy=3.6,posx=3.7;
    else if(type==2)
        posy=3.5,posx=-3.0;
    else if(type==3)
        posx=3.4,posy=2.4;
    else if(type==4)
        posx=0.0,posy=-1.0;
    else if(type==5)
        posx=3.0,posy=-1.0;
    for(int i=len-1;i>=0;i--)
    {
        for(int j=0;j<16;j++)
        {
            bool flag=false;
            for(int k=0;k<22;k++)
            {
                if(list[j][k]==str[i] && str[i]!=' '){flag=true;break;}
            }
            if(flag )
            {
                create(posx,posy,0,0,1,0,0,segment[j]);
            }
        }
        if(type==1 || type==2 || type==5)posx-=0.7;
        else if((type==3 || type==4) && i>0)
        {
            if(str[i-1]=='G' || str[i-1]=='M' || str[i-1]=='V' || str[i-1]=='Y')
                posx-=1.2;
            else
                posx-=0.7;
        }
    }
}
void scroll_callback(GLFWwindow *window, double xoffset,double yoffset)
{
    if(yoffset==1)
    {
        if(screenxp+0.04>screenxn-0.04 && screenyp+0.04>screenyn-0.04 && screenyn>=-7.0)
        {
            screenxp+=0.04;
            screenxn-=0.04;
            screenyp+=0.04;
            screenyn-=0.04;
            if(screenyn<-7.0){screenyn+=0.04,screenyp-=0.04;screenxp-=0.04;screenxn+=0.04;}
        }
    }
    else if(yoffset==-1)
    {
        if(screenxp-0.04>screenxn+0.04 && screenyp-0.04>screenyn+0.04 && screenyp<=7.0)
        {
            screenxp-=0.04;
            screenxn+=0.04;
            screenyp-=0.04;
            screenyn+=0.04;
            if(screenyp>7.0){screenyp-=0.04,screenyn+=0.04;screenxp+=0.04;screenxn-=0.04;}
        }
    }
}
void draw ()
{
    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram (programID);
    Matrices.projection = glm::ortho(screenxn,screenxp,screenyn, screenyp, 0.1f, 500.0f);
    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);


    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    glm::mat4 MVP;	// MVP = Projection * View * Model

    if(moveflag==1 && guny<3.65)
        guny+=0.0001;
    else if(moveflag==-1 && guny>-3.5)
        guny-=0.0001;
    if(rotateflag==1 && rotatation_angle<(5*M_PI)/12.0)
        rotatation_angle+=0.0005;
    else if(rotateflag==-1 && rotatation_angle>-(5*M_PI)/12.0)
        rotatation_angle-=0.0005;
    if(bottomredflag == 1 && bottomred<3.3)
        bottomred+=0.001;
    else if(bottomredflag == -1 && bottomred>-2.0)
        bottomred-=0.001;
    if(bottomgreenflag == 1 && bottomgreen<3.3)
        bottomgreen+=0.001;
    else if(bottomgreenflag == -1 && bottomgreen>-2.0)
        bottomgreen-=0.001; 

    if(bottomredflag == 1 && bottomrectre<3.16)
        bottomrectre+=0.001;
    else if(bottomredflag == -1 && bottomrectre>-2.14)
        bottomrectre-=0.001;
    if(bottomgreenflag == 1 && bottomrectgr<3.15)
        bottomrectgr+=0.001;
    else if(bottomgreenflag == -1 && bottomrectgr>-2.15)
        bottomrectgr-=0.001;

    if(flag_brickspeed==1)
        speed_bricks+=0.000003;
    else if(flag_brickspeed==-1 && speed_bricks>0.0005)
        speed_bricks-=0.000003; 
    if(flag_screenzoom==1 && screenxp-0.004>screenxn+0.004 && screenyp-0.004>screenyn+0.004 && screenyp<=7.0)
    {
        screenxp-=0.004;
        screenxn+=0.004;
        screenyp-=0.004;
        screenyn+=0.004;
        if(screenyp>7.0){screenyp-=0.004,screenyn+=0.004;screenxp+=0.004;screenxn-=0.004;}
    }
    else if(flag_screenzoom==-1 && screenxp+0.004>screenxn-0.004 && screenyp+0.004>screenyn-0.004 && screenyn>=-7.0)
    {
        screenxp+=0.004;
        screenxn-=0.004;
        screenyp+=0.004;
        screenyn-=0.004;
        if(screenyn<-7.0){screenyn+=0.004,screenyp-=0.004;screenxp-=0.004;screenxn+=0.004;}
    }
    if(flag_screenmousepan==1)
    {
        double xpos,ypos;
        glfwGetCursorPos(window,&xpos,&ypos);
        xpos=((xpos*(8.0))/700.0)-4.0;
        screenxn+=-xpos+prvxpos;screenxp+=-xpos+prvxpos;
        getprvxpos();
        //if(xpos>prvxpos){flag_screenpan=-1;getprvxpos ();}
        //else if(xpos<prvxpos){flag_screenpan=1;getprvxpos ();}
    }
    if(flag_screenpan==1)
    {
        screenxn+=0.004;screenxp+=0.004;
    }
    else if(flag_screenpan==-1)
    {
        screenxn-=0.004;screenxp-=0.004;
    }
    if(flag_pressleft==1)
    {
        double xpos,ypos;
        glfwGetCursorPos(window,&xpos,&ypos);
        xpos=((xpos*(screenxp-screenxn))/700.0)+screenxn;ypos=(-(ypos*(screenyp-screenyn))/700.0)+screenyp;
        if(ypos<-3.0 && xpos<bottomred+0.65 && xpos>bottomred-0.65 && xpos<3.3  && xpos>-2.0)
        {	
            bottomred=xpos;
            bottomrectre=bottomred-0.14;flag_movement=1;
        }
        else if(ypos<-3.0 && xpos<bottomgreen+0.65 && xpos>bottomgreen-0.65 && xpos<3.3  && xpos>-2.0)
        {	
            bottomgreen=xpos;
            bottomrectgr=bottomgreen-0.15;flag_movement=2;
        }
        else if(xpos>-3.8 && disst(xpos,ypos,-3.5,guny)<0.3 && ypos<3.5 && ypos>-3.0)
        {
            guny=ypos;flag_movement=3;
        }
    }
    if(flag_pressleft==1 && flag_movement==0)
    {
        if(flag_laserrelease==1)
        {
            createlaser();
            flag_laserrelease=0;
        }
        flag_movement=4;
    }
    if(penalty>0){
        create(-3.5,guny,0,rotatation_angle,0,0,1,rectangle);
        if(flag_movement==3)
            create(-3.8,guny,0,0,0,0,1,hovercirclel1);
        else
            create(-3.8,guny,0,0,0,0,1,circlel1);
        if(flag_movement==1)
            create(bottomrectre,-3.40,0,0,0,0,1,hoverbottomrectred);
        else
            create(bottomrectre,-3.40,0,0,0,0,1,bottomrectred);
        create(bottomred,-3.83,0,M_PI/3.0,1,0,0,bottomredcircle1);
        create(bottomgreen,-3.83,0,M_PI/3.0,1,0,0,bottomgreencircle1);
        if(flag_movement==2)
            create(bottomrectgr,-3.40,0,0,0,0,1,hoverbottomrectgreen);
        else
            create(bottomrectgr,-3.40,0,0,0,0,1,bottomrectgreen);
        create(3.5,2.5,0,mirror1_angle,0,0,1,mirror1);
        create(3.5,-2.5,0,mirror2_angle,0,0,1,mirror2);
        print(Redblocks);
        print(Blueblocks);
        print(Greenblocks);
        print(Laser);
        char *x=itoa(score);
        print_onscreen(x,1);
        char *xx=itoa(penalty);
        print_onscreen(xx,2);
        double xpos,ypos;
        glfwGetCursorPos(window,&xpos,&ypos);
        xpos=((xpos*(screenxp-screenxn))/700.0)+screenxn;ypos=(-(ypos*(screenyp-screenyn))/700.0)+screenyp;
        create(xpos,ypos,0,0,1,0,0,cross1);
        create(xpos,ypos,0,0,1,0,0,cross2);
    }
    // for(int i=0;i<16;i++)
    //	create(0,0,0,0,1,0,0,segment[i]);
}
void checkreflection()
{
    for(int i=0;i<laser_no;i++)
    {
        float h;
        float k;
        h=laser[i*72+0];k=laser[i*72+1];
        if(laser_latest[i]!=1 && fabs(disst(x11,y11,h,k)+disst(x12,y12,h,k)-disst(x11,y11,x12,y12))<0.10)
        {
            laser_angle[i]=4*(M_PI/3.0)-laser_angle[i];
            laser_latest[i]=1;
            laser_latest[i]=1;
        }
        h=laser[i*72+0];k=laser[i*72+1];
        if(laser_latest[i]!=2 && fabs(disst(x21,y21,h,k)+disst(x22,y22,h,k)-disst(x21,y21,x22,y22))<0.10)
        {
            laser_angle[i]=8*(M_PI/3.0)-laser_angle[i];
            laser_latest[i]=2;//collided=1;
        }
    }
}
int collision(float x21,float y21,float x22,float y22,float lsrx,float lsry)
{
    float h=((y21-y22)*((((y22-y21)*lsrx)+((x21-x22)*lsry)+((y21*x22)-(x21*y22))/(((x22-x21)*(x22-x21))+((y22-y21)*(y22-y21))))))+lsrx;
    float k=((x22-x21)*((((y22-y21)*lsrx)+((x21-x22)*lsry)+((y21*x22)-(x21*y22))/(((x22-x21)*(x22-x21))+((y22-y21)*(y22-y21))))))+lsry;
    h=lsrx;k=lsry;
    if(fabs(disst(x21,y21,h,k)+disst(x22,y22,h,k)-disst(x21,y21,x22,y22))<0.1 && fabs(disst(lsrx,lsry,h,k)-laser_radius)<0.1)
    {
        return 1;
    }
    return 0;
}
void deleteredblock(int x)
{
    for(int i=x;i<red_no_blocks;i++)
    {
        for(int j=0;j<18;j++)
            red_block[18*i+j]=red_block[(18*(i+1))+j];
    }
    red_no_blocks--;
}
void deletegreenblock(int x)
{
    for(int i=x;i<green_no_blocks;i++)
    {
        for(int j=0;j<18;j++)
            green_block[18*i+j]=green_block[(18*(i+1))+j];
    }
    green_no_blocks--;
}
void deleteblueblock(int x)
{
    for(int i=x;i<blue_no_blocks;i++)
    {
        for(int j=0;j<18;j++)
            blue_block[18*i+j]=blue_block[(18*(i+1))+j];
    }
    blue_no_blocks--;
}
void deletelaser(int x)
{
    for(int i=x;i<laser_no-1;i++)
    {
        for(int j=0;j<72;j++)
        {
            laser[i*72+j]=laser[((i+1)*72)+j];
        }
    }
    for(int i=x;i<laser_no-1;i++)
    {
        laser_angle[i]=laser_angle[i+1];
    }
    for(int i=x;i<laser_no-1;i++)
    {
        laser_latest[i]=laser_latest[i+1];
    }
    laser_latest[laser_no-1]=-1;
    laser_no--;
}
void checkcollisions()
{
    for(int i=0;i<red_no_blocks;i++)
    {
        for(int j=0;j<laser_no;j++)
        {
            if(collision(red_block[18*i+0],red_block[18*i+1],red_block[18*i+3],red_block[18*i+4],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(red_block[18*i+3],red_block[18*i+4],red_block[18*i+6],red_block[18*i+7],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(red_block[18*i+0],red_block[18*i+1],red_block[18*i+6],red_block[18*i+7],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(red_block[18*i+9],red_block[18*i+10],red_block[18*i+12],red_block[18*i+13],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(red_block[18*i+12],red_block[18*i+13],red_block[18*i+15],red_block[18*i+16],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(red_block[18*i+15],red_block[18*i+16],red_block[18*i+9],red_block[18*i+10],laser[j*72+0],laser[j*72+1])==1  )

            {
                penalty--;
                collision_sound=1;
                deleteredblock(i);
                deletelaser(j);
            }
        }	
    }
    for(int i=0;i<green_no_blocks;i++)
    {
        for(int j=0;j<laser_no;j++)
        {
            if(collision(green_block[18*i+0],green_block[18*i+1],green_block[18*i+3],green_block[18*i+4],laser[j*72+0],laser[j*72+1])==1 ||
                    collision(green_block[18*i+3],green_block[18*i+4],green_block[18*i+6],green_block[18*i+7],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(green_block[18*i+0],green_block[18*i+1],green_block[18*i+6],green_block[18*i+7],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(green_block[18*i+9],green_block[18*i+10],green_block[18*i+12],green_block[18*i+13],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(green_block[18*i+12],green_block[18*i+13],green_block[18*i+15],green_block[18*i+16],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(green_block[18*i+15],green_block[18*i+16],green_block[18*i+9],green_block[18*i+10],laser[j*72+0],laser[j*72+1])==1  )
            {
                penalty--;
                collision_sound=1;
                deletegreenblock(i);
                deletelaser(j);
            }
        }	
    }
    for(int i=0;i<blue_no_blocks;i++)
    {
        for(int j=0;j<laser_no;j++)
        {
            if(collision(blue_block[18*i+0],blue_block[18*i+1],blue_block[18*i+3],blue_block[18*i+4],laser[j*72+0],laser[j*72+1])==1 ||
                    collision(blue_block[18*i+3],blue_block[18*i+4],blue_block[18*i+6],blue_block[18*i+7],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(blue_block[18*i+0],blue_block[18*i+1],blue_block[18*i+6],blue_block[18*i+7],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(blue_block[18*i+9],blue_block[18*i+10],blue_block[18*i+12],blue_block[18*i+13],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(blue_block[18*i+12],blue_block[18*i+13],blue_block[18*i+15],blue_block[18*i+16],laser[j*72+0],laser[j*72+1])==1  ||
                    collision(blue_block[18*i+15],blue_block[18*i+16],blue_block[18*i+9],blue_block[18*i+10],laser[j*72+0],laser[j*72+1])==1  )
            {
            	collision_sound=1;
                deleteblueblock(i);
                deletelaser(j);
            }
        }	
    }
}
void checkblocks()
{
    for(int i=0;i<red_no_blocks;i++)
    {
        for(int j=1;j<18;j+=3)
        {
            if(red_block[(18*i)+j]<-3.3)
            {
                deleteredblock(i);
                break;
            }
        }
    }
    for(int i=0;i<green_no_blocks;i++)
    {
        for(int j=1;j<18;j+=3)
        {
            if(green_block[(18*i)+j]<-3.3)
            {
                deletegreenblock(i);
                break;
            }
        }
    }
    for(int i=0;i<blue_no_blocks;i++)
    {
        for(int j=1;j<18;j+=3)
        {
            if(blue_block[(18*i)+j]<-3.3)
            {
                deleteblueblock(i);
                break;
            }
        }
    }
    for(int i=0;i<laser_no;i++)
    {
        if(fabs(laser[i*72+0])>4.4 || fabs(laser[i*72+1])>4.4)
        {
            deletelaser(i);
            break;
        }
    }
}
/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        //        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "GAME", NULL, NULL);

    if (!window) {
        glfwTerminate();
        //        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
       is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, mouseButton);
    glfwSetScrollCallback(window, scroll_callback);  // mouse button clicks

    return window;
}

int basketintersect()
{
    if(fabs(bottomred-bottomgreen)<=1.30)
        return 0;
    return 1;
}

void redbasket()
{
    for(int i=0;i<red_no_blocks;i++)
    {
        if(red_block[18*i+0]<bottomred+0.65 && red_block[18*i+0]>bottomred-0.65 && red_block[18*i+1]<-3.0 && basketintersect()==1)
        {
            score+=1;
            deleteredblock(i);
        }
    }
    for(int i=0;i<blue_no_blocks;i++)
    {
        if(blue_block[18*i+0]<bottomred+0.65 && blue_block[18*i+0]>bottomred-0.65 && blue_block[18*i+1]<-3.0 && basketintersect()==1)
        {
            quitflag=1;
            penalty--;
            deleteblueblock(i);
            break;
        }
    }
}

void greenbasket()
{
    for(int i=0;i<green_no_blocks;i++)
    {
        if(green_block[18*i+0]<bottomgreen+0.65 && green_block[18*i+0]>bottomgreen-0.65 && green_block[18*i+1]<-3.0 && basketintersect()==1)
        {
            score+=1;
            deletegreenblock(i);
        }
    }
    for(int i=0;i<blue_no_blocks;i++)
    {
        if(blue_block[18*i+0]<bottomgreen+0.65 && blue_block[18*i+0]>bottomgreen-0.65 && blue_block[18*i+1]<-3.0 && basketintersect()==1)
        {
            quitflag=1;
            penalty--;
            deleteblueblock(i);
            break;
        }
    }
}

void canon_rotation()
{
    double xpos,ypos;
    glfwGetCursorPos(window,&xpos,&ypos);
    xpos=((xpos*(screenxp-screenxn))/700.0)+screenxn;ypos=(-(ypos*(screenyp-screenyn))/700.0)+screenyp;
    if(xpos>-3.5+0.5 && xpos<+4.0 && ypos>screenyn && ypos<screenyp)
    {
        rotatation_angle=atan((guny-ypos)/(-3.8-xpos));
    }
}
/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    //**createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
    createRectangle ();
    createCirclel1 ();
    createBottomrectred ();
    createBottomrectgreen ();
    createBottomredcircle1 ();
    createBottomgreencircle1 ();
    createmirror1 ();
    createmirror2 ();
    function_segment ();
    HovercreateCirclel1 ();
    HovercreateBottomrectred ();
    HovercreateBottomrectgreen ();
    createcross2 ();
    createcross1 ();
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

//int main (int argc, char** argv)
void *gamefunction(void *)
{
    int width = 700;
    int height = 700;
    memset(laser_latest,-1,sizeof(int)*100);
    GLFWwindow* window = initGLFW(width, height);

    initGL (window, width, height);	

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */

    while (!glfwWindowShouldClose(window)) {


        if(penalty<=0)
        {
            close_screen+=1;
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // use the loaded shader program
            // Don't change unless you know what you are doing
            glUseProgram (programID);
            Matrices.projection = glm::ortho(screenxn,screenxp,screenyn, screenyp, 0.1f, 500.0f);
            // Eye - Location of camera. Don't change unless you are sure!!
            glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
            // Target - Where is the camera looking at.  Don't change unless you are sure!!
            glm::vec3 target (0, 0, 0);
            // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
            glm::vec3 up (0, 1, 0);


            // Compute Camera matrix (view)
            // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
            //  Don't change unless you are sure!!
            Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

            // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
            //  Don't change unless you are sure!!
            glm::mat4 VP = Matrices.projection * Matrices.view;

            glm::mat4 MVP;	
            char gameover[]="GAMEOVER";
            print_onscreen(gameover,3);
            char sscore[]="SCORE";
            print_onscreen(sscore,4);
            char *x=itoa(score);
            print_onscreen(x,5);
            if(close_screen>1000)
                break;
        }


        canon_rotation();
        updateblockpos();
        updatelaserspos();
        checkreflection();
        checkcollisions();
        checkblocks();
        redbasket();
        greenbasket();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);
        if(penalty>0)
        {draw();
            // Poll for Keyboard and mouse events
            glfwPollEvents();

            // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
            current_time = glfwGetTime(); // Time in seconds
            if ((current_time - last_update_time) >= 1.0)
            { // atleast 0.5s elapsed since last frame
                // do something every 0.5 seconds ..
                int count=rand()%(min((score/5)+2,3));
                flag_laserrelease=1;
                int vis[100]={0};
                for(int i=0;i<count;i++)
                {
                    int k=rand()%55-25;
                    if(vis[k+30]==0 && vis[k+1+30]==0 && vis[k+2+30]==0)
                    {
                        createblock ((float)(k*1.0)/10.0,rand()%3);
                        vis[k+30]=1;vis[k+1+30]=1;vis[k+2+30]=1;
                    }
                }
                last_update_time = current_time;
            }
        }
    }

    glfwTerminate();
    //    exit(EXIT_SUCCESS);
}

void *sound_1(void *)
{
    while(1)
    {
    	if(close_screen>1000)break;
        if(shoot_sound==0 && collision_sound==0)
            continue;
        ao_device* device;
        ao_sample_format format;
        int defaultDriver;
        WavHeader header;
        std::ifstream file;
        if(shoot_sound==1)
        {
        	       file.open("song.wav", std::ios::binary | std::ios::in);
    }
    else if(collision_sound==1)
    {
		
        file.open("son.wav", std::ios::binary | std::ios::in);

    }
        file.read(header.id, sizeof(header.id));
        assert(!std::memcmp(header.id, "RIFF", 4)); //is it a WAV file?
        file.read((char*)&header.totalLength, sizeof(header.totalLength));
        file.read(header.wavefmt, sizeof(header.wavefmt)); //is it the right format?
        assert(!std::memcmp(header.wavefmt, "WAVEfmt ", 8));
        file.read((char*)&header.format, sizeof(header.format));
        file.read((char*)&header.pcm, sizeof(header.pcm));
        file.read((char*)&header.channels, sizeof(header.channels));
        file.read((char*)&header.frequency, sizeof(header.frequency));
        file.read((char*)&header.bytesPerSecond, sizeof(header.bytesPerSecond));
        file.read((char*)&header.bytesByCapture, sizeof(header.bytesByCapture));
        file.read((char*)&header.bitsPerSample, sizeof(header.bitsPerSample));
        file.read(header.data, sizeof(header.data));
        file.read((char*)&header.bytesInData, sizeof(header.bytesInData));

        ao_initialize();

        defaultDriver = ao_default_driver_id();

        memset(&format, 0, sizeof(format));
        format.bits = header.format;
        format.channels = header.channels;
        format.rate = header.frequency;
        format.byte_format = AO_FMT_LITTLE;

        device = ao_open_live(defaultDriver, &format, NULL);
        if (device == NULL) {
            std::cout << "Unable to open driver" << std::endl;
        }


        char* buffer = (char*)malloc(BUF_SIZE * sizeof(char));

        // determine how many BUF_SIZE chunks are in file
        int fSize = header.bytesInData;
        int bCount = fSize / BUF_SIZE;

        for (int i = 0; i < bCount; ++i) {
            file.read(buffer, BUF_SIZE);
            ao_play(device, buffer, BUF_SIZE);
        }

        int leftoverBytes = fSize % BUF_SIZE;
       // std::cout << leftoverBytes;
        file.read(buffer, leftoverBytes);
        memset(buffer + leftoverBytes, 0, BUF_SIZE - leftoverBytes);
        ao_play(device, buffer, BUF_SIZE);

        ao_close(device);
        ao_shutdown();
        shoot_sound=0;
        collision_sound=0;
    }
}
void *sound_2(void *)
{
    while(1)
    {
        if(collision_sound==0 )
            continue;
        ao_device* device;
        ao_sample_format format;
        int defaultDriver;
        WavHeader header;

        std::ifstream file;
        file.open("son.wav", std::ios::binary | std::ios::in);

        file.read(header.id, sizeof(header.id));
        assert(!std::memcmp(header.id, "RIFF", 4)); //is it a WAV file?
        file.read((char*)&header.totalLength, sizeof(header.totalLength));
        file.read(header.wavefmt, sizeof(header.wavefmt)); //is it the right format?
        assert(!std::memcmp(header.wavefmt, "WAVEfmt ", 8));
        file.read((char*)&header.format, sizeof(header.format));
        file.read((char*)&header.pcm, sizeof(header.pcm));
        file.read((char*)&header.channels, sizeof(header.channels));
        file.read((char*)&header.frequency, sizeof(header.frequency));
        file.read((char*)&header.bytesPerSecond, sizeof(header.bytesPerSecond));
        file.read((char*)&header.bytesByCapture, sizeof(header.bytesByCapture));
        file.read((char*)&header.bitsPerSample, sizeof(header.bitsPerSample));
        file.read(header.data, sizeof(header.data));
        file.read((char*)&header.bytesInData, sizeof(header.bytesInData));

        ao_initialize();

        defaultDriver = ao_default_driver_id();

        memset(&format, 0, sizeof(format));
        format.bits = header.format;
        format.channels = header.channels;
        format.rate = header.frequency;
        format.byte_format = AO_FMT_LITTLE;

        device = ao_open_live(defaultDriver, &format, NULL);
        if (device == NULL) {
            std::cout << "Unable to open driver" << std::endl;
        }


        char* buffer = (char*)malloc(BUF_SIZE * sizeof(char));

        // determine how many BUF_SIZE chunks are in file
        int fSize = header.bytesInData;
        int bCount = fSize / BUF_SIZE;

        for (int i = 0; i < bCount; ++i) {
            file.read(buffer, BUF_SIZE);
            ao_play(device, buffer, BUF_SIZE);
        }

        int leftoverBytes = fSize % BUF_SIZE;
        std::cout << leftoverBytes;
        file.read(buffer, leftoverBytes);
        memset(buffer + leftoverBytes, 0, BUF_SIZE - leftoverBytes);
        ao_play(device, buffer, BUF_SIZE);

        ao_close(device);
        ao_shutdown();
        collision_sound=0;
    }
}
int main(int argc,char **argv)
{
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, gamefunction, NULL);
    pthread_create(&threads[1], NULL,sound_1 , NULL);
    //pthread_create(&threads[2], NULL,sound_2 , NULL);
    while(close_screen<1000)
    {
    	//cout<<"hii\n";
    }
    exit(0);
    return 0;
}
