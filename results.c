/***************************************************************
                           results.c
                               
The file contains the results screen
***************************************************************/

#include "results.h"
#include "core.h"
#include "menu.h"
#include <libdragon.h>

#define FONT_TEXT       1

#define FADE_IN_DURATION        0.5f
#define BOX_ANIMATION_DELAY     0.2f
#define BOX_ANIMATION_DURATION  0.4f
#define TEXT_DELAY              0.7f
#define FADE_OUT_DURATION       0.6f
#define FADE_OUT_POST_DELAY     0.2f

static int points_to_win;
static int global_points[4];

static bool ending;
static rdpq_font_t *font;

static sprite_t *bg_pattern;
static sprite_t *bg_gradient;
static sprite_t *btn_game;

static float time;

static bool fading_out;
static float fade_out_start;

int results_get_points(PlyNum player)
{
    return global_points[player];
}

void results_set_points(PlyNum player, int points)
{
    global_points[player] = points;
}

int results_get_points_to_win()
{
    return points_to_win;
}

void results_set_points_to_win(int points)
{
    points_to_win = points;
}

inline bool player_has_won(PlyNum player)
{
    return results_get_points(player) >= results_get_points_to_win();
}

void results_init()
{
    // Award points to winning players
    for (PlyNum i = 0; i < MAXPLAYERS; i++) {
        if (core_get_winner(i)) {
            results_set_points(i, results_get_points(i) + 1);
        }

        if (player_has_won(i)) {
            ending = true;
        }
    }

    font = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){.color = RGBA32(0xFF,0xDD,0xDD,0xFF), .outline_color = RGBA32(0x31,0x39,0x3C,0xFF) });

    color_t player_colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4
    };

    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        rdpq_font_style(font, i+1, &(rdpq_fontstyle_t){.color = player_colors[i] });
    }

    bg_pattern = sprite_load("rom:/pattern.i8.sprite");
    bg_gradient = sprite_load("rom:/gradient.i8.sprite");
    btn_game = sprite_load("rom:/btnGame.i4.sprite");

    time = 0;
    fading_out = false;

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
}

/**
 * @brief Draws scrolling background
 * @param pattern texture for the checkerboard pattern
 * @param gradient gradient on the Y axis
 * @param offset scroll offset
 */
void menu_draw_bg(sprite_t *pattern, sprite_t *gradient, float offset, float fade)
{
    rdpq_set_mode_standard();
    rdpq_mode_begin();
    rdpq_mode_blender(0);
    rdpq_mode_alphacompare(0);
    rdpq_mode_combiner(RDPQ_COMBINER2(
        (TEX0, 0, TEX1, 0), (0, 0, 0, 1),
        (COMBINED, 0, PRIM, 0), (0, 0, 0, 1)
    ));
    rdpq_mode_dithering(DITHER_BAYER_BAYER);
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_mode_end();

    float brightness = 0.75f * fade;
    rdpq_set_prim_color((color_t){0xFF * brightness, 0xCC * brightness, 0xAA * brightness, 0xFF});

    offset = fmodf(offset, 64.0f);
    rdpq_texparms_t param_pattern = {
        .s = {.repeats = REPEAT_INFINITE, .mirror = true, .translate = offset, .scale_log = 0},
        .t = {.repeats = REPEAT_INFINITE, .mirror = true, .translate = offset, .scale_log = 0},
    };
    rdpq_texparms_t param_grad = {
        .s = {.repeats = REPEAT_INFINITE},
        .t = {.repeats = 1, .scale_log = 2},
    };
    rdpq_tex_multi_begin();
    rdpq_sprite_upload(TILE0, pattern, &param_pattern);
    rdpq_sprite_upload(TILE1, gradient, &param_grad);
    rdpq_tex_multi_end();

    rdpq_texture_rectangle(TILE0, 0, 0, display_get_width(), display_get_height(), 0, 0);
}

float ease_cubic_out(float t)
{
    float f;
    f = t - 1.0f;
    return f * f * f + 1.0f;
}

void results_loop(float deltatime)
{
    time += deltatime;

    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

    surface_t *disp = display_get();

    rdpq_attach(disp, NULL);
    
    float bg_fade = time < FADE_IN_DURATION ? (time/FADE_IN_DURATION) : 1.0f;
    menu_draw_bg(bg_pattern, bg_gradient, time * 12.0f, bg_fade);

    // Box background
    int rect_width = 260;
    int rect_height = 180;

    float box_time = time - BOX_ANIMATION_DELAY;
    if (box_time < BOX_ANIMATION_DURATION) {
        float box_factor = ease_cubic_out(box_time/BOX_ANIMATION_DURATION);
        rect_width *= box_factor;
        rect_height *= box_factor;
    }

    int pos_x = display_get_width() / 2 - (rect_width / 2) - 8;
    int ycur = display_get_height() / 2 - (rect_height / 2) - 8;

    if (time > BOX_ANIMATION_DELAY) {
        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
        rdpq_mode_filter(FILTER_BILINEAR);

        rdpq_tex_multi_begin();
        rdpq_sprite_upload(TILE0, btn_game, &(rdpq_texparms_t){
            .s.repeats = 1, .t.repeats = 1,
        });
        rdpq_tex_reuse_sub(TILE1, &(rdpq_texparms_t){
            .s.repeats = 1, .t.repeats = 1,
        }, 0, 0, 16, 16);
        rdpq_tex_multi_end();

        rdpq_set_prim_color(RGBA32(0xAD,0xBA,0xBD,0xFF));
        rdpq_texture_rectangle(TILE1, // left, top
                            pos_x, ycur,
                            pos_x + rect_width, ycur + rect_height, 0, 0);
        rdpq_texture_rectangle(TILE1, // right, top
                            pos_x + rect_width + 14, ycur,
                            pos_x + rect_width - 1, ycur + rect_height, 0, 0);
        rdpq_texture_rectangle(TILE1, // left, bottom
                            pos_x, ycur + rect_height + 14,
                            pos_x + rect_width, ycur + rect_height - 1, 0, 0);
        rdpq_texture_rectangle(TILE1, // right, bottom
                            pos_x + rect_width + 14, ycur + rect_height + 14,
                            pos_x + rect_width - 1, ycur + rect_height - 1, 0, 0);
    }

    if (time > TEXT_DELAY) {
        // Text
        rdpq_textparms_t parms = {
            .align = ALIGN_CENTER,
            .width = 260
        };

        rdpq_set_mode_standard();

        ycur += 20;

        if (ending) {
            ycur += rdpq_text_printf(&parms, FONT_TEXT, pos_x, ycur, "WINNER\n").advance_y;
            ycur += 40;
            for (PlyNum i = 0; i < MAXPLAYERS; i++)
            {
                if (player_has_won(i)) {
                    ycur += rdpq_text_printf(&parms, FONT_TEXT, pos_x, ycur, "^0%dPlayer %d\n", i+1, i+1).advance_y;
                }
            }
        } else {
            ycur += rdpq_text_printf(&parms, FONT_TEXT, pos_x, ycur, "RESULTS\n").advance_y;
            ycur += 40;
            for (PlyNum i = 0; i < MAXPLAYERS; i++)
            {
                ycur += rdpq_text_printf(NULL, FONT_TEXT, pos_x + 20, ycur, "^0%dP%d: %d points\n", i+1, i+1, results_get_points(i)).advance_y;
            }
        }

        ycur += 40;
        ycur += rdpq_text_printf(&parms, FONT_TEXT, pos_x, ycur, "Press A to continue\n").advance_y;
    }

    // Fade out
    if (fading_out) {
        float fade_time = time - fade_out_start;
        float fade_out_factor = (fade_time/FADE_OUT_DURATION);
        if (fade_out_factor > 1.0f) fade_out_factor = 1.0f;
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_set_prim_color(RGBA32(0, 0, 0, fade_out_factor*0xFF));
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
    }

    rdpq_detach_show();

    if (btn.a) {
        fading_out = true;
        fade_out_start = time;
    }

    if (fading_out && time > fade_out_start + FADE_OUT_DURATION + FADE_OUT_POST_DELAY) {
        if (ending) menu_reset();
        core_level_changeto(LEVEL_MINIGAMESELECT);
    }
}

void results_cleanup()
{
    rspq_wait();
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
    sprite_free(bg_pattern);
    sprite_free(bg_gradient);
    sprite_free(btn_game);
    display_close();
}
