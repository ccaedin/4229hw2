/*
Thank you professor Schreuder, the bitmap code helped me to adapt the code to load a texture from a SDL_Surface, which is one of the challenges I undertook for this project.

*/
#include "Text.h"
#include <stdexcept>
const SDL_Color Text::RED = {255, 0, 0, 255};
const SDL_Color Text::GREEN = {0, 255, 0, 255};
const SDL_Color Text::BLUE = {0, 0, 255, 255};
const SDL_Color Text::WHITE = {255, 255, 255, 255};
const SDL_Color Text::BLACK = {0, 0, 0, 255};
bool Text::initialized = false; 

bool Text::Init()
{
    if(!Text::initialized)
    {
        if (TTF_Init() == -1) {
            SDL_Log("Unable to initialize SDL_ttf: %s", TTF_GetError());
            Text::initialized = false;
            return false;
        }
    }
    else
    {
        //already initialized print to error
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf already initialized");
    }
    Text::initialized = true;
    return true;
}
Text::Text(std::string fontPath, int fontSize = 12)
{
    Text::Init();
    this->fontPath = fontPath;
    font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!font) {
        SDL_Log("Unable to open font: %s", TTF_GetError());
        return;
    }
}

Text::~Text()
{
    TTF_CloseFont(font);
}
void Text::RenderText(const std::string &text, SDL_Color color,float x, float y)
{
  RenderText(text, color, x, y, 0);
}

//Thanks to ChatGPT and https://stackoverflow.com/questions/5289447/using-sdl-ttf-with-opengl, //http://www.sdltutorials.com/sdl-tip-sdl-surface-to-opengl-texture
//this is a poorely optimized function, the texture should not bew created every time, especially if the text is not changing
void Text::RenderText(const std::string &text, SDL_Color color,float x, float y, float z)
{
    if(!font)
        throw std::runtime_error("Font not initialized");
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        SDL_Log("Unable to render text surface: %s", TTF_GetError());
        return;
    }
    GLuint texture = LoadTexture(surface);
    if (!texture) {
        SDL_Log("Unable to create texture from rendered text: %s", TTF_GetError());
        return;
    }
    //reset the matrices so the texts lays flat in screen space, however this prevents the text from being rotated of put on the axis, so a more complex system will need to be written further down the line.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    gluOrtho2D(0, this->width, 0, this->height); // had problems with the -1 1 cords so I decided to do it in screen space. Will update later on.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    //make srue the back is transparent
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, texture);


    //define bottom left with x and y, but then use the aspect ratio to ensure the entire text is displayed
    float left = x;
    float right = x + surface->w;
    float bottom = y;
    float top = y + surface->h;



    //reset the color, so the text is not colored by the previous object
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f(left, bottom, z);
        glTexCoord2f(1, 1); glVertex3f(right, bottom, z);
        glTexCoord2f(1, 0); glVertex3f(right, top, z);
        glTexCoord2f(0, 0); glVertex3f(left, top, z);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    //free the texture and the surface
    if (surface) {
        SDL_FreeSurface(surface);
    }
    if (texture) {
        glDeleteTextures(1, &texture);
    }
    texture = 0;
}

void Text::UpdateFont(int fontSize, int width, int height)
{
    // Update font size
    this->width = width;
    this->height = height;
    TTF_CloseFont(font);
    font = TTF_OpenFont(fontPath.c_str(), fontSize);
    TTF_GetFontSDF(font);
    if (!font) {
        SDL_Log("Unable to open font: %s", TTF_GetError());
        return;
    }
}


GLuint Text::LoadTexture(SDL_Surface* surface)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
    if (formattedSurface == nullptr) {
        SDL_Log("Unable to convert surface to SDL_PIXELFORMAT_ABGR8888: %s", SDL_GetError());
        return 0;
    }
    //magic
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, formattedSurface->w, formattedSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, formattedSurface->pixels);
    //tell opengl to use a linear filter for minification and magnification for the least distortions.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //disable wrapping, we dont want that for text.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    SDL_FreeSurface(formattedSurface);

    return textureID;
}