#ifndef _WORLD_H
#define _WORLD_H

#include "rendering.h"

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 32
#define CHUNK_DEPTH 16

#define RENDER_DISTANCE 2

typedef struct {
    vec3s pos;
} BlockFace;

typedef struct {
    struct {
        Vertex *vertices;
        BlockFace *faces;
        u32 vertex_count;
    } mesh;

    u8 blocks[CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH];
} Chunk;

typedef struct {
    Chunk chunks[(RENDER_DISTANCE * 2 + 1) * (RENDER_DISTANCE * 2 + 1)];
} World;

Chunk init_chunk();
void destroy_chunk(Chunk *chunk);
void mesh_chunk(Chunk *chunk);
u8 chunk_get(Chunk *chunk, u8 x, u8 y, u8 z);

#endif