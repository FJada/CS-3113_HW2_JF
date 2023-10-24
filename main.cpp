#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <ctime>
#include "cmath"

enum Coordinate
{
    x_coordinate,
    y_coordinate
};

#define LOG(argument) std::cout << argument << '\n'

/**
* Author: [JADA FORRESTER
* Assignment: Pong Clone
* Date due: 2023-10-14, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

const int WINDOW_WIDTH = 840,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
            BG_BLUE = 0.549f,
            BG_GREEN = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const float MINIMUM_COLLISION_DISTANCE = 0.0f;
float player_previous_ticks = 0.0f;
float player_speed = 2.0f;
float elapsed_time = 0.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

//spirtes
const char BLUE_PADDLE_SPRITE[] = "/Users/jadaforrester/Desktop/SLDP Project 2/SLDP Project 2/assets/blue_paddle.png";
const char RED_PADDLE_SPRITE[] = "/Users/jadaforrester/Desktop/SLDP Project 2/SLDP Project 2/assets/red_paddle.png";
const char BALL_SPRITE[] = "/Users/jadaforrester/Desktop/SLDP Project 2/SLDP Project 2/assets/ball.png";


SDL_Window* g_display_window;
bool g_game_is_running = true;
bool is_player_one_mode = false;
ShaderProgram g_shader_program;

//texture ids
GLuint red_player_texture_id_1;
GLuint blue_player_texture_id_2;
GLuint ball_player_texture_id_3;

// player positions
glm::vec3 red_player_position = glm::vec3(-4.5f, 0.0f, 0.0f);
glm::vec3 blue_player_position = glm::vec3(4.5f, 0.0f, 0.0f);
glm::vec3 ball_player_position = glm::vec3(0.0f, 0.0f, 0.0f);

// movement trackers
glm::vec3 red_player_movement;
glm::vec3 blue_player_movement;
glm::vec3 ball_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

//matricies
glm::mat4 player_view_matrix,
          player_model_matrix,
          player_projection_matrix;

glm::mat4 red_model_matrix;
glm::mat4 blue_model_matrix;
glm::mat4 ball_model_matrix;


float get_screen_to_ortho(float coordinate, Coordinate axis)
{
    switch (axis) {
        case x_coordinate:
            return ((coordinate / WINDOW_WIDTH) * 10.0f ) - (10.0f / 2.0f);
        case y_coordinate:
            return (((WINDOW_HEIGHT - coordinate) / WINDOW_HEIGHT) * 7.5f) - (7.5f / 2.0);
        default:
            return 0.0f;
    }
}

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    g_display_window = SDL_CreateWindow("Pong Clone",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
   
    red_model_matrix = glm::mat4(1.0f);
    blue_model_matrix = glm::mat4(1.0f);
    ball_model_matrix = glm::mat4(1.0f);
    
    red_model_matrix = glm::translate(red_model_matrix, glm::vec3(-4.0f, 0.0f, 0.0f));
    red_player_position += red_player_movement;
    blue_model_matrix = glm::translate(blue_model_matrix, glm::vec3(4.0f, 0.0f, 0.0f));

    player_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    player_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    
    g_shader_program.set_projection_matrix(player_projection_matrix);
    g_shader_program.set_view_matrix(player_view_matrix);
    // Notice we haven't set our model matrix yet!
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    //load textures for sprites
    red_player_texture_id_1 = load_texture(RED_PADDLE_SPRITE);
    blue_player_texture_id_2 = load_texture(BLUE_PADDLE_SPRITE);
    ball_player_texture_id_3 = load_texture(BALL_SPRITE);
    
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 UP and DOWN keys controll the red paddle on the left side of the window when in two player mode and W and S controll the left side blue paddle when in two player mode
 when t is pressed the game swithces to one player mode and the blue paddle moves on its own
 */


void process_input()
{
    red_player_movement = glm::vec3(0.0f);
    blue_player_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_WINDOWEVENT_CLOSE:
            case SDL_QUIT:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_t:
                        is_player_one_mode = !is_player_one_mode;
                        //reset game positions when switching modes
                        red_player_position = glm::vec3(-4.5f, 0.0f, 0.0f);
                        blue_player_position = glm::vec3(4.5f, 0.0f, 0.0f);
                        ball_player_position = glm::vec3(0.0f, 0.0f, 0.0f);
                        red_model_matrix = glm::translate(glm::mat4(1.0f), red_player_position);
                        blue_model_matrix = glm::translate(glm::mat4(1.0f), blue_player_position);
                        ball_model_matrix = glm::translate(glm::mat4(1.0f), ball_player_position);
                        break;
                                             
                    case SDLK_UP:
                        red_player_movement.y = 2.0f;
                        break;
                    case SDLK_DOWN:
                        red_player_movement.y = -2.0f;
                        break;
                     //up
                    case SDLK_w:
                        blue_player_movement.y = 2.0f;
                        break;
                    // down
                    case SDLK_s:
                        blue_player_movement.y = -2.0f;
                        break;
                        
                    case SDLK_q:
                        g_game_is_running = false;
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }
    
    const Uint8 *key_states = SDL_GetKeyboardState(NULL); // array of key states [0, 0, 1, 0, 0, ...]
    if (!is_player_one_mode){
        
        if (key_states[SDL_SCANCODE_DOWN])
        {
            red_player_movement.y = -2.0f;
            
        } if (key_states[SDL_SCANCODE_UP])
        {
            red_player_movement.y = 2.0f;
        }
        // up
        if(key_states[SDL_SCANCODE_W]){
            blue_player_movement.y = 2.0f;
        }
        //down
        if(key_states[SDL_SCANCODE_S]){
            blue_player_movement.y = -2.0f;
        }
    }
    else{
        if (key_states[SDL_SCANCODE_DOWN])
        {
            red_player_movement.y = -2.0f;
            
        } if (key_states[SDL_SCANCODE_UP])
        {
            red_player_movement.y = 2.0f;
        }
       
    }
    
    // normalize
    if (glm::length(red_player_movement) > 1.0f)
    {
        red_player_movement = glm::normalize(red_player_movement);
    }
    
    if (glm::length(blue_player_movement) > 1.0f)
    {
        blue_player_movement = glm::normalize(blue_player_movement);
    }
   
}


/**This function ennsures that the paddles do exceed the windows either from the top or the bottoms of the screen by resetting the position **/
glm::vec3 player_bounds_enforcer(glm::vec3 position){
    if(position.y > 3.20f){
        position.y = 3.20f;
    }
    else if(position.y < -3.20f){
        position.y = -3.20f;
    }
    return position;
}

/*
manually checks collision between paddles and ball since the check_collision function from class caused odd thread errors, the ball bounces off the top ans bottome of the screen and sometimes bounces off the paddles and ends
 when the ball passes the paddle and quits the program
*/
glm::vec3 ball_velocity = glm::vec3(-0.5f, 0.5f, 0.0f);

void update() {
    //delta time
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - player_previous_ticks;
    player_previous_ticks = ticks;

 
    // check if new position is within bounds
    glm::vec3 new_position_red = red_player_position + red_player_movement * delta_time * player_speed;
    glm::vec3 new_position_blue = blue_player_position + blue_player_movement * delta_time * player_speed;
    
    // player one / red player will always move normally no matter the mode
    red_model_matrix = glm::mat4(1.0f);
    red_player_position = player_bounds_enforcer(new_position_red);
    red_player_position += red_player_movement * delta_time * player_speed;
    red_model_matrix = glm::translate(glm::mat4(2.0f), red_player_position);
    
    //ball movement
   
    if(!is_player_one_mode){
        blue_model_matrix = glm::mat4(1.0f);
        blue_player_position = player_bounds_enforcer(new_position_blue);
        blue_player_position += blue_player_movement * delta_time * player_speed;
        blue_model_matrix = glm::translate(glm::mat4(2.0f), blue_player_position);
    }
    // if t is pressed switch blue player to auto move up and down
    else{
        elapsed_time += delta_time;
        // periodic translate
        float period = 2.0f;
        float yOffset = 3.0f * sin(2.0f * M_PI * elapsed_time / period);
        blue_player_position.y = yOffset;
        blue_model_matrix =glm::mat4(1.0f);
        blue_player_position = player_bounds_enforcer(blue_player_position);
        blue_model_matrix = glm::translate(glm::mat4(2.0f), blue_player_position);
    }
    
    glm::vec3 new_position_ball = ball_player_position + ball_velocity * delta_time * player_speed;
    ball_model_matrix = glm::mat4(1.0f);
    ball_player_position += ball_velocity * delta_time * player_speed;
    ball_model_matrix = glm::translate(glm::mat4(2.0f), ball_player_position);
    
    //bounce off top of window
    if (new_position_ball.y >= 3.0f) {
        new_position_ball.y = 3.0f;
        ball_velocity = glm::vec3(1.2f * ball_velocity.x, -1.2f *ball_velocity.y, 0.0f);
    }
    
    if (new_position_ball.y <= -3.0f) {
        new_position_ball.y = -3.0f;
        ball_velocity = glm::vec3(1.2f * ball_velocity.x, -1.2f * ball_velocity.y, 0.0f);
    }
    //chesk collisions
    if (new_position_ball.x <= red_player_position.x + 1.f && new_position_ball.x >= red_player_position.x - 1.0f) {
        ball_velocity = glm::vec3(-ball_velocity.x, ball_velocity.y, 0.0f);
    }

    if (new_position_ball.x >= blue_player_position.x - 1.0f && new_position_ball.x <= blue_player_position.x + 1.0f) {
        ball_velocity = glm::vec3(-ball_velocity.x, ball_velocity.y, 0.0f);
    }
    //checks if passed the paddles 
    if (new_position_ball.x < -4.5f || new_position_ball.x > 4.5f) {
            
            ball_player_position = glm::vec3(0.0f, 0.0f, 0.0f);
            ball_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
            ball_model_matrix = glm::mat4(1.0f);
            g_game_is_running  = false;
        }
    
}


void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    
    //green window
       glClearColor(0.0f, 0.5f, 0.2f, 1.0f);
       glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    // Bind texture
    draw_object(red_model_matrix, red_player_texture_id_1);
    
    draw_object(blue_model_matrix, blue_player_texture_id_2);
    
    draw_object(ball_model_matrix, ball_player_texture_id_3);
    

    
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    //SDL_JoystickClose(g_player_one_controller);
    SDL_Quit();
}

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}


