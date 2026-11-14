#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"
#include "gfc_primitives.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"

#include "gf3d_mesh.h"

#include "game_physics.h"

#include "model.h"

#include "entity.h"
#include "player.h"
#include "basketball.h"
#include "backboard.h"
#include "collision.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 1000 / GAME_FPS;
static float fps = 0;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();

void exitGame()
{
    _done = 1;
}


int main(int argc,char *argv[])
{
    //local variables
    //Sprite *bg;

    Model* court;
    GFC_Matrix4 courtMat;
    GFC_Box courtBounds;

    GFC_Matrix4 *view;

    // entities
    Entity* player;
    Entity* basketball;
    Entity* backboard;

    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");
    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);

    model_init_system(1024);
    entity_system_init(1024);
    
    //game init
    srand(SDL_GetTicks());
    slog_sync();
    //bg = gf2d_sprite_load_image("images/bg_flat.png");
    gf2d_mouse_load("actors/mouse.actor");
    //gfc_matrix4_identity(mat);
    view = gf3d_vgraphics_get_view_matrix(); // <-- used for rendering the camera view
    //model = model_load("models/dino.model");

    court = model_load("models/court.model");
    gfc_matrix4_identity(courtMat);

    if (court && court->mesh)
    {
        courtBounds = court->mesh->bounds;
        slog("court bounds: pos(%f, %f, %f) size (%f, %f, %f)",
             courtBounds.x, courtBounds.y, courtBounds.z,
             courtBounds.w, courtBounds.h, courtBounds.d);
    }
    else
    {
        slog("court failed to load so using hard-coded bounds");
        courtBounds = gfc_box(-100, -60, 0, 200, 120, 2);
    }

    player = player_new();
    basketball = basketball_new();
    backboard = backboard_new();

    //ball_attach(basketball, player);



    //basketball->position = gfc_vector3d(5, 0, 0);
    //basketball->scale = gfc_vector3d(3, 3, 3);

    //slog("aftert model_load: model %p", (void*) model);

    // main game loop    
    while(!_done)
    {
        //SDL_Event event;

        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();

        entity_system_think();
        entity_system_update(GAME_DELTA_TIME);
        collision_boundary(player, courtBounds);
        collision_boundary(basketball, courtBounds);
        collision_boundary(backboard, courtBounds);

        entity_check_collisions(player);

        //camera updates
        gfc_matrix4_view(
            *view,                                                                               // out
            gfc_vector3d(player->localPosition.x, player->localPosition.y +100, player->localPosition.z + 15),       // position
            player->localPosition,                                                                          // target
            gfc_vector3d(0,0,1)                                                                  // up
        );
        gf3d_vgraphics_render_start();
        // 3D DRAWS
        //model_queue_render(model, mat, GFC_COLOR_WHITE);
        model_draw(court, courtMat, GFC_COLOR_WHITE);
        entity_system_draw();
        //entity_draw(player);
        //entity_draw(basketball);

        //2D draws
                //gf2d_sprite_draw_image(bg,gfc_vector2d(0,0));
                gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
                /*gf2d_font_draw_line_tag(
                    "NORMAL FONT TEST",
                    FT_Normal,
                    GFC_COLOR_WHITE,
                    gfc_vector2d(10,10)
                );*/

                /*gf2d_font_draw_line_tag(
                    "ALT+F4 to exit",
                    FT_H1,
                    GFC_COLOR_WHITE,
                    gfc_vector2d(10,10)
                );*/

                /*gf2d_font_draw_line_tag(
                    "H1 FONT TEST",
                    FT_H1,
                    GFC_COLOR_WHITE,
                    gfc_vector2d(10,90)
                );

                gf2d_font_draw_line_tag(
                    "H2 FONT TEST",
                    FT_H2,
                    GFC_COLOR_WHITE,
                    gfc_vector2d(10,140)
                );

                gf2d_font_draw_line_tag(
                    "H6 FONT TEST",
                    FT_H6,
                    GFC_COLOR_WHITE,
                    gfc_vector2d(10,190)
                );*/
                gf2d_mouse_draw();
        gf3d_vgraphics_render_end();
        /*static int frames = 0;
        if (frames < 3) slog ("frame %i done", frames++);*/

        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay();
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    entity_clear_all();
    slog("gf3d program end");
    exit(0);
    slog_sync();
    return 0;
}

void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
    }    
}

void game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
//     slog("fps: %f",fps);
}
/*eol@eof*/
