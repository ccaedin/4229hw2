#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>
#include <stdio.h>

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
bool is_fullscreen = false;
SDL_Window * Window;
SDL_GLContext Context;
bool quit = false;
bool normalize_lorenz = false;
float points[50000][3];

float translate[3];
int lorenz_changed = false;
bool play_animation = false;
int last_index = 0;
//http://www.sdltutorials.com/sdl-tip-sdl-surface-to-opengl-texture
void print()
{
TTF_Font* Sans = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-C.ttf", 24);

// this is the color in rgb format,
// maxing out all would give you the color white,
// and it will be your text's color
SDL_Color White = {255, 255, 255};

// as TTF_RenderText_Solid could only be used on
// SDL_Surface then you have to create the surface first
SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, "put your text here", White); 

//get a texture id
GLuint texture;
glGenTextures(1, &texture);
glBindTexture(GL_TEXTURE_2D, texture);

int Mode = GL_RGB;

if(surfaceMessage->format->BytesPerPixel == 4) {
    Mode = GL_RGBA;
}

glTexImage2D(GL_TEXTURE_2D, 0, Mode, surfaceMessage->w, surfaceMessage->h, 0, Mode, GL_UNSIGNED_BYTE, surfaceMessage->pixels);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

int x = 0;
int y = 0;
int height = 0;
int width = 0;
glBegin(GL_QUADS);
glTexCoord2f(0, 0); glVertex2f(x, y);
glTexCoord2f(1, 0); glVertex2f(x + width, y);
glTexCoord2f(1, 1); glVertex2f(x + width, y + height);
glTexCoord2f(0, 1); glVertex2f(x, y + height);
glEnd();

// Don't forget to free your surface and texture
SDL_FreeSurface(surfaceMessage);
}
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

    float x_max = -10000000;
    float x_min = 10000000;

    float y_max = -10000000;
    float y_min = 10000000;

    float z_max = -10000000;
    float z_min = 10000000;



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
            float new_x = x;
            float new_y = y;
            float new_z = z;
            
            if(normalize_lorenz)
            {
                new_x = 2*(x- x_min) / (x_max - x_min) - 1;
                new_y = 2*(y- y_min) / (y_max - y_min) - 1;
                new_z = 2*(z- z_min) / (z_max - z_min) - 1;
            }
            points[i][0] = new_x*scale;
            points[i][1] = new_y*scale;
            points[i][2] = new_z*scale;
    }
}
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return -1;
    }
    if(TTF_Init() != 0)
    {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
     
    
    // Turn on double buffering with a 24bit Z buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    glEnable(GL_TEXTURE_2D);
    


    Window = SDL_CreateWindow("AGT Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w_width, w_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | (is_fullscreen * SDL_WINDOW_FULLSCREEN));
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
    // render loop
    // -----------
    lorenz();
    unsigned int fpsLast = SDL_GetTicks();
    unsigned int fpsCurrent = 0;
    unsigned int fpsCounter = 0;
    resize(w_width, w_height);
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
    if(lorenz_changed)
    {
        lorenz();
        lorenz_changed = false;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //how I originally had it for rotation, but I could not translate
    glRotated(ph,1,0,0);
    glRotated(th,0,1,0);

    glTranslatef(translate[0], translate[1], 0);
    glClearColor(0,0,0, 1.0f);
    //  Draw triangle
    glColor3f(1,1,0);
    glPointSize(1);
    

    
    if(!play_animation)
    {
        glBegin(GL_LINE_STRIP);
        for (int i=0;i<50000;i++)
        {
            glVertex3f(points[i][0], points[i][1], points[i][2]);
        }  
        glEnd();
    }
    else
    {
        glPointSize(3);
        glBegin(GL_LINE_STRIP);
        for (int i=0;i<last_index && last_index < 50000;i++)
        { 
            glVertex3f(points[i][0], points[i][1], points[i][2]);
        }  
        glEnd();
        last_index++;
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
    print();
    //draw a cube around the borders of the clipping bounds

    glEnd();
    glFlush();
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    //swap the buffers
    
}
float translationSpeed = 0.1;
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
            case SDLK_p:
                //play animation
                play_animation = !play_animation;
                break;
            case SDLK_f:
                is_fullscreen = !is_fullscreen;
                SDL_SetWindowFullscreen(Window, is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
                break;
            case SDLK_UP:
                translate[1] += translationSpeed;
                break;
            case SDLK_DOWN:
                translate[1] -= translationSpeed;
                break;
            case SDLK_LEFT:
                translate[0] -= translationSpeed;
                break;
            case SDLK_RIGHT:
                translate[0] += translationSpeed;
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
    float aspect = (float)width / (float)height;
    glViewport(0, 0, width, height);
    //set frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-aspect, aspect, -1, 1, 1, 100);
    glMatrixMode(GL_MODELVIEW);

}