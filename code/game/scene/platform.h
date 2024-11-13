#ifndef PLATFORM_H
#define PLATFORM_H

typedef struct {

  Box box;

} PlatformCollider;


typedef struct {

  uint32_t id;
  T3DMat4FP *mat;
  Vector3 position;
  PlatformCollider collider;
  color_t color;
  uint32_t platformTimer;

} Platform;

Platform hexagons[PLATFORM_COUNT];

// Forward Declarations

void platform_init(Platform* platform, T3DModel* model, Vector3 position, color_t color);
void platform_loop(Platform* platform, size_t index);
void platform_drawBatch(void);
void platform_hexagonGrid(Platform* platform, T3DModel* model, float z, color_t color);
void platform_destroy(Platform* platform);

// Definitions

void platform_init(Platform* platform, T3DModel* model, Vector3 position, color_t color)
{

  static uint32_t platformIdx = 0;

  platform->id = platformIdx;
  platform->mat = malloc_uncached(sizeof(T3DMat4FP)); // needed for t3d
  platform->position = position;

  // Initialize the three boxes for collision within each hexagon

  platform->collider.box = (Box) {
    .size = {275.0f, 275.0f, 30.0f},
    .center = platform->position,
    .rotation = {0,0,0}
  };

  platform->color = color; // Set color

  platform->platformTimer = 0;

  platformIdx++;

}

//// BEHAVIORS ~ Start ////

// Example behavior: Oscillate platform height up and down over time with staggered offsets
void platform_updateHeight(Platform* platform, float time, size_t index)
{
  float amplitude = 75.0f;    // Maximum oscillation distance from the base height
  float baseHeight = 0.0f;    // Center height around which the platform oscillates
  float phaseOffset = index * 0.75f;  // Staggered phase offset based on platform index

  // Oscillate `platform->position.z` with a staggered start for each platform
  platform->position.z = baseHeight + amplitude * fm_sinf(time + phaseOffset);
}

void platform_loop(Platform* platform, size_t index)
{

  static float timeElapsed = 0.0f;  // Accumulated time

  timeElapsed += (display_get_delta_time() * 0.05f);  // Increment by some fraction of deltaTime

  // Run behaviors
  platform_updateHeight(platform, timeElapsed, index);

  // Translate collision
  platform->collider.box.center = platform->position;

  // Update matrix
  t3d_mat4fp_from_srt_euler(
    platform->mat,
    (float[3]){1.0f,1.0f,1.0f},
    (float[3]){0.0f,0.0f,0.0f},
    (float[3]){platform->position.x, platform->position.y, platform->position.z}
  );

}

//// BEHAVIORS ~ End ////

//// RENDERING ~ Start ////

// T3D MODEL DRAW BATCHING
#define BATCH_LIMIT 8      // Number of objects per rspq block
#define BLOCK_COUNT 3     // Pre-calculated block count

T3DModel *batchModel = NULL;
rspq_block_t* rspqBlocks[BLOCK_COUNT] = {NULL};  // Static array of rspq block pointers

void platform_createBatch(Platform* platform, T3DModel* model)
{
  // Load model once for the entire batch if not already loaded
  if (batchModel == NULL) {
    batchModel = model;
  }

  // Initialize the rspq block index and start a new rspq block
  size_t blockIndex = 0;
  rspq_block_begin();

  for (size_t i = 0; i < PLATFORM_COUNT; i++)
  {

    // Set the model matrix and draw
    t3d_matrix_set(platform[i].mat, true);
    rdpq_set_prim_color(platform[i].color);
    t3d_model_draw(batchModel);

    // End the current rspq block and start a new one every n objects
    if ((i + 1) % BATCH_LIMIT == 0 || i == PLATFORM_COUNT - 1)
    {
      rspqBlocks[blockIndex] = rspq_block_end();  // Store the completed rspq block
      blockIndex++;
      if (i < PLATFORM_COUNT - 1) rspq_block_begin();  // Start a new rspq block if more objects remain
    }
  }
}


// Iterate through and run RSPQ blocks
inline void platform_drawBatch(void)
{
  for (size_t i = 0; i < BLOCK_COUNT; i++)
  {
    if (rspqBlocks[i] != NULL)  // Check for NULL before running
    {
      rspq_block_run(rspqBlocks[i]);
    }
  }
}

// Generate a hexagonal grid of 30 platforms at desired height, with desired model and color
void platform_hexagonGrid(Platform* platform, T3DModel* model, float z, color_t color)
{
  float x_offset = 300.0f;    // Horizontal distance between centers of adjacent columns
  float y_offset = 300.0f;    // Vertical distance between centers of adjacent rows
  float start_x = 0.0f;       // Starting X coordinate for the first row
  float start_y = 0.0f;       // Starting Y coordinate for the first row

  int rows[] = {3, 4, 5, 4, 3};  // Number of hexagons per row
  int hexagon_index = 0;

  for (int row_index = 0; row_index < 5; row_index++)
  {
    int hex_count = rows[row_index];
    float row_start_x = start_x - (hex_count - 1) * x_offset / 2.0f;

    for (int i = 0; i < hex_count; i++)
    {
      platform[hexagon_index].position.x = row_start_x + i * x_offset;
      platform[hexagon_index].position.y = start_y + row_index * y_offset;
      platform[hexagon_index].position.z = z;
      hexagon_index++;
    }
  }

  // Initialize the platforms
  for (size_t p = 0; p < PLATFORM_COUNT; p++)
  {
    platform_init(&platform[p], model, platform[p].position, color);
  }

  platform_createBatch(platform, model);

}

// Frees T3D model, matrices and RSPQ Blocks used for rendering
void platform_destroy(Platform* platform)
{
  for (size_t b = 0; b < BLOCK_COUNT; b++)
  {
    if(rspqBlocks[b] != NULL) rspq_block_free(rspqBlocks[b]);
  }

  for (size_t p = 0; p < PLATFORM_COUNT; p++)
  {
    if(platform[p].mat != NULL) free_uncached(platform[p].mat);
  }

  if(batchModel != NULL) t3d_model_free(batchModel);
}

#endif // PLATFORM_H