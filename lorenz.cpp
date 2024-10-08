#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include<SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <iostream>
#include <stdio.h>
#include "Text.h"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
static void draw_screen( void );
void handle_key_event(SDL_Event event);
void gl_error();
void resize(int width, int height);
/*  Lorenz Parameters  */
double s  = 10;
double b  = 2.6666;
double r  = 28;
double scale = 0.02;

int ph = 20;
int th = 55;
int w_width = 800;
int w_height = 600;
float aspect;
bool is_fullscreen = false;
SDL_Window * Window;
SDL_GLContext Context;
Text arial = Text("arial.ttf", 24);
bool quit = false;
bool normalize_lorenz = false;
float points[50000][3];

float translate[3];
int lorenz_changed = false;
bool play_animation = false;
int animation_speed = 10;
int last_index = 0;
bool show_commands = false;

float x_max = -10000000;
float x_min = 10000000;

float y_max = -10000000;
float y_min = 10000000;

float z_max = -10000000;
float z_min = 10000000;
//take directly from teh example
void lorenz()
{
    /*  Coordinates  */
    double x = 1;
    double y = 1;
    double z = 1;
    /*  Time step  */
    double dt = 0.001;

    /*
    *  Integrate 50,000 steps (50 time units with dt = 0.001)
    *  Explicit Euler integration
    */
   //reset the min and max
    x_max = -10000000;
    x_min = 10000000;

    y_max = -10000000;
    y_min = 10000000;

    z_max = -10000000;
    z_min = 10000000;

    for (int i=0;i<50000;i++)
    {
            
            double dx = s*(y-x);
            double dy = x*(r-z)-y;
            double dz = x*y - b*z;
            x += dt*dx;

            x > x_max ? x_max = x : x < x_min ? x_min = x : 0; //set min and max
            y += dt*dy;
            y > y_max ? y_max = y : y < y_min ? y_min = y : 0; //set min and max

            z += dt*dz;
            z > z_max ? z_max = z : z < z_min ? z_min = z : 0; //set min and max
            //normalize x and y to between -1 and 1

            

            points[i][0] = x;
            points[i][1] = y;
            points[i][2] = z;
    }
}
int main(int argc, char *argv[])
{
    // glfw: initialize and configure
    // ------------------------------
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
     
    
    // Turn on double buffering with a 24bit Z buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    

    Window = SDL_CreateWindow("Lorenz", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w_width, w_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | (is_fullscreen * SDL_WINDOW_FULLSCREEN));
    if (Window == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create OpenGL window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }


    // Create a OpenGL Context
    Context = SDL_GL_CreateContext(Window);
    if (Context == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(Window);
        SDL_Quit();
        return 1;
    }
    //ennable depth testing
    glEnable(GL_DEPTH_TEST);
    
    //initialize lorenz and fps counter
    lorenz();
    unsigned int fpsLast = SDL_GetTicks();
    unsigned int fpsCurrent = 0;
    unsigned int fpsCounter = 0;
    resize(w_width, w_height); // initialize the viewport
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_WINDOWEVENT:
                    if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        w_width = event.window.data1;
                        w_height = event.window.data2;
                        //resize the viewport
                        resize(event.window.data1, event.window.data2);
                    }
                    break;
                default:
                handle_key_event(event);
                    break;
            }
        }
        draw_screen();
        //get error
        gl_error();
        fpsCounter++;

        if(SDL_GetTicks() - fpsLast >= 1000)
        {
            fpsCurrent = fpsCounter;
            fpsCounter = 0;
            fpsLast = SDL_GetTicks();
            std::string title = "Lorenz FPS: " + std::to_string(fpsCurrent);
            SDL_SetWindowTitle(Window, title.c_str());
            SDL_Log("FPS: %d\n", fpsCurrent);

        }
        SDL_GL_SwapWindow(Window);
    }
    // sdl: terminate, clearing all previously allocated sdl resources.
    // ------------------------------------------------------------------
    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Window);
    SDL_Quit();
    return 0;
}

static void draw_screen(void)
{
    //avoid regenerating the lorenz points each time
    if(lorenz_changed)
    {
        lorenz();
        lorenz_changed = false;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //how I originally had it for rotation, but I could not translate
    glLoadIdentity();
    //basic rotation
    glRotated(ph,1,0,0);
    glRotated(th,0,1,0);
    //set the background to black
    glClearColor(0,0,0, 0.0f);
    //  Draw triangle
    
    glColor3f(1,1,0);
    glPointSize(1);
    

    
    if(!play_animation)
    {
        glBegin(GL_LINE_STRIP);
        for (int i=0;i<50000;i++)
        {
            float new_x = points[i][0];
            float new_y = points[i][1];
            float new_z = points[i][2];
            //for testing, i think its pretty cool
            if(normalize_lorenz)
            {
                new_x = 2*(new_x- x_min) / (x_max - x_min) - 1;
                new_y = 2*(new_y- y_min) / (y_max - y_min) - 1;
                new_z = 2*(new_z- z_min) / (z_max - z_min) - 1;
            }
            else //scale if not normalized, then it is all done by the same amount
            {
                new_x = new_x * scale;
                new_y = new_y * scale;
                new_z = new_z * scale;
            }
            glVertex3f(new_x, new_y, new_z);
        }  
        glEnd();
    }
    else
    {
        glBegin(GL_LINE_STRIP);
        //the easy way i found to speed up the animation was to skip points, which makes it more choppy, but faster
        //in the future I can implement a better way to handle animation, but this was a good start
        for (int i=0;i<last_index && last_index < 50000;i+=animation_speed)
        {
            //set up the coordinates to 
            float new_x = points[i][0];
            float new_y = points[i][1];
            float new_z = points[i][2]; 
            
            if(normalize_lorenz)
            {
                new_x = 2*(new_x- x_min) / (x_max - x_min) - 1;
                new_y = 2*(new_y- y_min) / (y_max - y_min) - 1;
                new_z = 2*(new_z- z_min) / (z_max - z_min) - 1;
            }
            else //scale if not normalized, then it is all done by the same amount
            {
                new_x = new_x * scale;
                new_y = new_y * scale;
                new_z = new_z * scale;
            }
            glVertex3f(new_x, new_y, new_z);
        }
        glEnd();
        last_index+=animation_speed;
        last_index %= 50000;
        //draw lines 10 points apar

    }        
    //  Make scene visible
    //print string to screen
    glColor3f(1,1,1);
    glBegin(GL_LINES);
    //draw x
    glColor3f(1,0,0);
    glVertex3f(0,0,0);
    glVertex3f(0.5,0,0);
    //draw y
    glColor3f(0,1,0);
    glVertex3f(0,0,0);
    glVertex3f(0,0.5,0);
    //draw z
    glColor3f(0,0,1);
    glVertex3f(0,0,0);
    glVertex3f(0,0,0.5);
    glEnd();


    //the coordiantes are not exact because i couldn't figure out how to normalize the texture coordinates properly
    arial.RenderText("Show Commands: c", Text::WHITE, 10, w_height-25);
    if(show_commands)
    {
        arial.RenderText("Quit: ESC", Text::WHITE, 10, w_height-50);
        arial.RenderText("Normalize: n", Text::WHITE, 10, w_height-75);
        arial.RenderText("Increase s: q", Text::WHITE, 10, w_height-100);
        arial.RenderText("Decrease s: a", Text::WHITE, 10, w_height-125);
        arial.RenderText("Increase b: w", Text::WHITE, 10, w_height-150);
        arial.RenderText("Decrease b: s", Text::WHITE, 10, w_height-175);
        arial.RenderText("Increase r: e", Text::WHITE, 10, w_height-200);
        arial.RenderText("Decrease r: d", Text::WHITE, 10, w_height-225);
        arial.RenderText("Reset: r", Text::WHITE, 10, w_height-250);
        arial.RenderText("Random: x", Text::WHITE, 10, w_height-275);
        arial.RenderText("Play/Pause: p", Text::WHITE, 10, w_height-300);
        arial.RenderText("Increase Speed: UP", Text::WHITE, 10, w_height-325);
        arial.RenderText("Decrease Speed: DOWN", Text::WHITE, 10, w_height-350);
        arial.RenderText("Fullscreen: f", Text::WHITE, 10, w_height-375);
        
    }
        //create a string to display the lorenz factors with doubles to 2 decimal places
    std::string lorenz_factors = "s: %.2f b: %.2f r: %.2f";
    //format the string
    char lorenz_factors_c[100];
    sprintf(lorenz_factors_c, lorenz_factors.c_str(), s, b, r);
    //change the font size, currently this is very intensive because it remakes the font every time, later I can implement the sdl font cache
    arial.UpdateFont(48, w_width, w_height);
    arial.RenderText(lorenz_factors_c, Text::RED, 10, 10);
    arial.UpdateFont(24, w_width, w_height);

    glFlush();
    
}
void handle_key_event(SDL_Event event)
{
    if(event.type == SDL_KEYDOWN)
    {
        switch(event.key.keysym.sym)
        {
            case SDLK_ESCAPE:
                SDL_Log("Escape key pressed\n");
                quit = true;
                break;
           //change the loren factors
           case SDLK_n:
                normalize_lorenz = !normalize_lorenz;
                lorenz_changed = true;
                break;
            case SDLK_q:
                s += 0.1;
                lorenz_changed = true;
                break;
            case SDLK_a:
                s -= 0.1;
                lorenz_changed = true;
                break;
            case SDLK_w:
                b += 0.1;
                lorenz_changed = true;
                break;
            case SDLK_s:
                b -= 0.1;
                lorenz_changed = true;
                break;
            case SDLK_e:
                r += 0.1;
                lorenz_changed = true;
                break;
            case SDLK_d:
                r -= 0.1;
                lorenz_changed = true;
                break;
            case SDLK_r:
                //reset the lorenz factors
                s = 10;
                b = 2.6666;
                r = 28;
                lorenz_changed = true;
                last_index = 0;
                break;
            case SDLK_x:
                //pick random values for the lorenz factors
                s = rand() % 100;
                b = rand() % 100;
                r = rand() % 100;
                lorenz_changed = true;
                break;
            case SDLK_p:
                //play animation
                play_animation = !play_animation;
                break;
            case SDLK_UP:
                animation_speed += 5;
                //max is 30
                animation_speed = std::min(30, animation_speed);
                break;
            case SDLK_DOWN:
                animation_speed -= 5;
                //min is 1
                animation_speed = std::max(1, animation_speed);
                break;
            case SDLK_f:
                is_fullscreen = !is_fullscreen;
                SDL_SetWindowFullscreen(Window, is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
                break;
            case SDLK_c:
                show_commands = !show_commands;
                break;
            default:
                break;
        }
    }
    //if scroll wheel
    else if(event.type == SDL_MOUSEWHEEL)
    {
        if(event.wheel.y > 0)
        {
            scale += 0.01;
        }
        else if(event.wheel.y < 0)
        {
            scale -= 0.01;
        }
        //bound by max gl depth size
        scale = std::max(0.01, std::min(1.0, scale));
        lorenz_changed = true;
    }
    //change ph and th by mouse movement and mouse button one being held down
    else if(event.type == SDL_MOUSEMOTION && (event.motion.state & SDL_BUTTON_LMASK))
    {
        th += event.motion.xrel;
        ph += event.motion.yrel;
        th %= 360;
        ph %= 360;
        
    }
}
void gl_error()
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        SDL_Log("OpenGL error: %s\n", gluErrorString(err));
    }
}

void resize(int width, int height)
{
    w_width = width;
    w_height = height;
    aspect = (float)width / (float)height;
    glViewport(0, 0, width, height);
    //set frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //perspective is hard
    glOrtho(-aspect, aspect, -1.5, 1.5, -1.5, 10);
    glMatrixMode(GL_MODELVIEW);
    //update the font so it can display properly
    arial.UpdateFont(24, width, height);

}