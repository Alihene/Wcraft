#include "world.h"

static vec4s get_block_uvs(u8 block_id) {
    switch(block_id) {
        case 1:
            return (vec4s) {0.0f, 0.0f, 1.0f / 8.0f, 1.0f / 8.0f};
            break;
        default:
            return (vec4s) {0.0f, 0.0f, 0.0f, 0.0f};
            break;
    }
}

Chunk init_chunk() {
    Chunk chunk = {0};
    chunk.blocks[0] = 1;
}

void destroy_chunk(Chunk *chunk) {
    if(chunk->mesh.vertices) {
        free(chunk->mesh.vertices);
    }

    if(chunk->mesh.faces) {
        free(chunk->mesh.faces);
    }
}

void mesh_chunk(Chunk *chunk) {
    for(u8 x = 0; x < CHUNK_WIDTH; x++) {
        for(u8 y = 0; y < CHUNK_HEIGHT; y++) {
            for(u8 z = 0; z < CHUNK_DEPTH; z++) {
                u8 block = chunk_get(chunk, x, y, z);
                if(block > 0) {
                    
                }
            }
        }
    }
}

u8 chunk_get(Chunk *chunk, u8 x, u8 y, u8 z) {
    return chunk->blocks[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_DEPTH)];
}