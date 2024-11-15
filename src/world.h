#ifndef _WORLD_H
#define _WORLD_H

#include "rendering.h"

#include <pthread.h>

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 32
#define CHUNK_DEPTH 16

#define LOAD_DISTANCE 2
// Side width of the square of loaded chunks
#define LOAD_WIDTH (LOAD_DISTANCE * 2 + 1)

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_GRASS = 1,
    BLOCK_DIRT = 2,
    BLOCK_STONE = 3,
    BLOCK_GLASS = 4,
    BLOCK_PLANKS = 5,
    BLOCK_COBBLESTONE = 6,
    BLOCK_LOG = 7,
    BLOCK_SAND = 8,
    BLOCK_LEAVES = 9
} BlockType;

#define MAX_BLOCK_ID 0x000000FF

typedef struct {
    BlockType type;

    struct {
        vec2s neg_x, pos_x, neg_y, pos_y, neg_z, pos_z;
    } tex_coords;

    bool solid;
    bool transparent;
} Block;

extern Block blocks[MAX_BLOCK_ID + 1];

typedef struct {
    vec3s pos;
} BlockFace;

typedef struct {
    ivec3s pos;
    const Block *block;
} BlockSet;

typedef struct {
    ivec2s pos;

    struct {
        Vertex *vertices;
        BlockFace *faces;
        u32 vertex_count;
        u32 vertex_count_alloc;
        bool should_update;
        bool being_rendered;
    } mesh;

    u8 blocks[CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH];
} Chunk;

typedef struct {
    ivec2s pos;
    u8 blocks[CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH];
} StoredChunk;

typedef struct {
    Chunk **chunks;
    u32 chunk_count;

    struct {
        BlockSet *block_sets;
        u32 count;
        u32 allocated;
    } block_set_list;

    // Local chunk storage for unloaded chunks
    struct {
        StoredChunk *chunks;
        u32 count;
        u32 allocated;
    } chunk_storage;
} World;

void init_blocks();

void destroy_chunk(Chunk *chunk);
void mesh_chunk(Chunk *chunk, bool update_flag);
Block *chunk_get(Chunk *chunk, u8 x, u8 y, u8 z);
void chunk_set(Chunk *chunk, const Block *block, u8 x, u8 y, u8 z);

World *init_world();
Chunk *get_chunk(i32 x, i32 y);
void update_world();
void load_chunks();
void world_set(const Block *block, i32 x, i32 y, i32 z);
void world_set_and_mesh(const Block *block, i32 x, i32 y, i32 z);
Block *world_get(i32 x, i32 y, i32 z);
void destroy_world();

#endif