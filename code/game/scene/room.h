#ifndef ROOM_H
#define ROOM_H


// Hook/callback to modify tile settings set by t3d_model_draw
void tile_scroll(void* userData, rdpq_texparms_t *tileParams, rdpq_tile_t tile) 
{
    float offset = *(float*)userData;

    if(tile == TILE0) {
        tileParams->s.translate = offset * 0.5f;
        tileParams->t.translate = offset * 0.8f;

        tileParams->s.translate = fm_fmodf(tileParams->s.translate, 32.0f);
        tileParams->t.translate = fm_fmodf(tileParams->t.translate, 32.0f);
    }
}

void move_lava(T3Model* model)
{
    transformOffset += 0.0042f;

    // returns the global vertex buffer for a model.
    // If you have multiple models and want to only update one, you have to manually iterate over the objects.
    // see the implementation of t3d_model_draw_custom in that case.
    T3DVertPacked* verts = t3d_model_get_vertices(model);
    float globalHeight = fm_sinf(transformOffset * 2.5f) * 30.0f;

    for(uint16_t i=0; i < model->totalVertCount; ++i)
    {
    // To better handle the interleaved vertex format,
    // t3d provides a few helper functions to access attributes
    int16_t *pos = t3d_vertbuffer_get_pos(verts, i);

    // water-like wobble effect
    float height = fm_sinf(
        transformOffset * 4.5f
        + pos[0] * 30.1f
        + pos[2] * 20.1f
    );
    pos[1] = 20.0f * height + globalHeight;

    // make lower parts darker, and higher parts brighter
    float color = height * 0.25f + 0.75f;
    uint8_t* rgba = t3d_vertbuffer_get_rgba(verts, i);
    rgba[0] = color * 255;
    rgba[1] = color * 200;
    rgba[2] = color * 200;
    rgba[3] = 0xFF;
    }

    // Don't forget to flush the cache again! (or use an uncached buffer in the first place)
    data_cache_hit_writeback(verts, sizeof(T3DVertPacked) * model->totalVertCount / 2);
}

bool scrollEnabled = true;
bool transformEnabled = true;


float tileOffset = 0.0f;
float transformOffset = 0.0f;



#endif