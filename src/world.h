#ifndef _WORLD_H
#define _WORLD_H

#include "rendering.h"

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 32
#define CHUNK_DEPTH 16

#define RENDER_DISTANCE 2

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_GRASS = 1
} BlockType;

#define MAX_BLOCK_ID 0x000000FF

typedef struct {
    BlockType type;

    struct {
        vec2s neg_x, pos_x, neg_y, pos_y, neg_z, pos_z;
    } tex_coords;

    bool solid;
} Block;

extern Block blocks[MAX_BLOCK_ID + 1];

typedef struct {
    vec3s pos;
} BlockFace;

typedef struct {
    struct {
        Vertex *vertices;
        BlockFace *faces;
        u32 vertex_count;
        u32 vertex_count_alloc;
    } mesh;

    u8 blocks[CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH];
} Chunk;

typedef struct {
    Chunk chunks[(RENDER_DISTANCE * 2 + 1) * (RENDER_DISTANCE * 2 + 1)];
} World;

void init_blocks();

Chunk init_chunk();
void destroy_chunk(Chunk *chunk);
void mesh_chunk(Chunk *chunk);
Block *chunk_get(Chunk *chunk, u8 x, u8 y, u8 z);
void chunk_set(Chunk *chunk, const Block *block, u8 x, u8 y, u8 z);

#endif