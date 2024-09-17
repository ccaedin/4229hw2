#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <algorithm>
#include <string>
class Text
{
public:
    Text(std::string fontPath, int fontSize);
    ~Text();
    void UpdateFont(int fontSize, int width, int height);
    void RenderText(const std::string& text, SDL_Color color, float x, float y);
    void RenderText(const std::string &text, SDL_Color color,float x, float y, float z);
    static bool Init();
    //get a bunch of constant colors, with alpha
    static const SDL_Color RED;
    static const SDL_Color GREEN;
    static const SDL_Color BLUE;
    static const SDL_Color WHITE;
    static const SDL_Color BLACK ;
private:
    GLuint LoadTexture(SDL_Surface* surface);
    TTF_Font* font;
    std::string fontPath;
    int width, height;
    static bool initialized;
};