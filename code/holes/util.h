#ifndef UTIL_H
#define UTIL_H

// Global defines
#define HITBOX_RADIUS       40.f

#define COUNTDOWN_DELAY     3.0f
#define GO_DELAY            1.0f
#define WIN_DELAY           5.0f
#define WIN_SHOW_DELAY      2.0f

#define NUM_OBJECTS 9

#define SCREEN_WIDTH display_get_width()
#define SCREEN_HEIGHT display_get_height()

// STRUCTS
typedef struct
{
  T3DMat4FP* mtxFP;
  rspq_block_t *block;
  T3DModel *model;
} map_data;

typedef struct
{
  T3DVec3 position;
  T3DVec3 target;

} camera_data;

enum OBJ_TYPES
{
  OBJ_CAR,
  OBJ_BUILDING,
  OBJ_HYDRANT,
  NUM_OBJ_TYPES
};

typedef struct
{
  uint8_t ID;
  uint8_t texID;
  T3DObject *model;
  T3DMat4FP* mtxFP;
  T3DVec3 position;
  T3DVec3 scale;
  float yaw;
  bool visible;
  bool hide;
  color_t color;
  rspq_block_t *modelBlock;
} object_data;

typedef struct
{
  uint8_t type;
  uint8_t scoreValue;
  T3DModel *model;
  float collisionRadius;
  object_data objects[NUM_OBJECTS];
} object_type;

typedef struct
{
  PlyNum plynum;
  T3DMat4FP* modelMatFP;
  rspq_block_t *dplHole;
  T3DVec3 moveDir;
  T3DVec3 playerPos;
  T3DVec3 scale;
  float rotY;
  float currSpeed;
  bool isAlive;
  PlyNum ai_target;
  int ai_reactionspeed;
  uint8_t score;
} player_data;

// Global variables
extern float countDownTimer;
extern bool isEnding;
extern float endTimer;
extern PlyNum winner;
extern rspq_syncpoint_t syncPoint;

// COLLISION
 
float vec2_dist_squared(T3DVec3* pos0, T3DVec3* pos1);
bool check_collision(T3DVec3* pos0, float radii0, T3DVec3* pos1, float radii1);

// Calculate squared distance between two positions' X and Z coordinates
float vec2_dist_squared(T3DVec3* pos0, T3DVec3* pos1)
{
  float dx = pos1->v[0] - pos0->v[0];
  float dy = pos1->v[2] - pos0->v[2];
  return  dx*dx + dy*dy;
}

// Return whether point is within a radius (radii0 == 0), or if 2 radii overlap (radii0 != 0) 
bool check_collision(T3DVec3* pos0, float radii0, T3DVec3* pos1, float radii1)
{

  float distSq = vec2_dist_squared(pos0,pos1);

  assert(radii0 >= 0.0f && radii1 >= 0.0f);

  if(radii0 != 0)
  {
    // Assume checking if radii overlap
    float radiiSum = radii0 + radii1;
    float radiiSumSq = radiiSum*radiiSum;

    return distSq <= radiiSumSq;
  } else {

    // Assume checking if pos0 is within pos1's radius
    return distSq <= radii1 * radii1;
  }


}

#endif // UTIL_H