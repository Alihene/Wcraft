#include "world.h"
#include "noise.h"
#include "player.h"

Block blocks[MAX_BLOCK_ID + 1];

World world;

void init_blocks() {
    blocks[BLOCK_AIR] = (Block) {
        .type = BLOCK_AIR,
        .tex_coords = {
            .neg_x = (vec2s) {0, 0},
            .pos_x = (vec2s) {0, 0},
            .neg_y = (vec2s) {0, 0},
            .pos_y = (vec2s) {0, 0},
            .neg_z = (vec2s) {0, 0},
            .pos_z = (vec2s) {0, 0}
        },
        .solid = false
    };
    blocks[BLOCK_GRASS] = (Block) {
        .type = BLOCK_GRASS,
        .tex_coords = {
            .neg_x = (vec2s) {3 / 8.0f, 0},
            .pos_x = (vec2s) {3 / 8.0f, 0},
            .neg_y = (vec2s) {2 / 8.0f, 0},
            .pos_y = (vec2s) {0, 0},
            .neg_z = (vec2s) {3 / 8.0f, 0},
            .pos_z = (vec2s) {3 / 8.0f, 0}
        },
        .solid = true
    };
    blocks[BLOCK_DIRT] = (Block) {
        .type = BLOCK_DIRT,
        .tex_coords = {
            .neg_x = (vec2s) {2 / 8.0f, 0},
            .pos_x = (vec2s) {2 / 8.0f, 0},
            .neg_y = (vec2s) {2 / 8.0f, 0},
            .pos_y = (vec2s) {2 / 8.0f, 0},
            .neg_z = (vec2s) {2 / 8.0f, 0},
            .pos_z = (vec2s) {2 / 8.0f, 0},
        },
        .solid = true
    };
}

void destroy_chunk(Chunk *chunk) {
    if(chunk->mesh.vertices) {
        free(chunk->mesh.vertices);
    }

    if(chunk->mesh.faces) {
        free(chunk->mesh.faces);
    }
}

static void push_vertex(Chunk *chunk, const Vertex *v) {
    if(!chunk->mesh.vertices) {
        chunk->mesh.vertices = malloc(128 * sizeof(Vertex));
        chunk->mesh.vertex_count_alloc = 128;
    }

    if(chunk->mesh.vertex_count >= chunk->mesh.vertex_count_alloc) {
        chunk->mesh.vertices = realloc(chunk->mesh.vertices, chunk->mesh.vertex_count_alloc * 2 * sizeof(Vertex));
        chunk->mesh.vertex_count_alloc *= 2;
    }

    chunk->mesh.vertices[chunk->mesh.vertex_count] = *v;
    chunk->mesh.vertex_count++;
}

static void try_mesh_left_face(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(x > 0 && (chunk_get(chunk, x - 1, y, z)->type != BLOCK_AIR || chunk_get(chunk, x - 1, y, z)->solid)) {
        return;
    }

    Chunk *neighbour = get_chunk(chunk->pos.x - 1, chunk->pos.y);
    if(neighbour) {
        Block *corresponding_block = chunk_get(neighbour, CHUNK_WIDTH - 1, y, z);
        if(x == 0 && corresponding_block->type != BLOCK_AIR) {
            return;
        }
    } else {
        if(x == 0) {
            return;
        }
    }

    Block *block = chunk_get(chunk, x, y, z);
    vec2s tex_coords = blocks[block->type].tex_coords.neg_x;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z + 1},
        1.0f,
        tex_coords,
        0.8f
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z},
        1.0f,
        tex_coords,
        0.8f
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z},
        1.0f,
        tex_coords,
        0.8f
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z},
        1.0f,
        tex_coords,
        0.8f
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z + 1},
        1.0f,
        tex_coords,
        0.8f
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z + 1},
        1.0f,
        tex_coords,
        0.8f
    });
}

static void try_mesh_right_face(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(x < CHUNK_WIDTH - 1 && (chunk_get(chunk, x + 1, y, z)->type != BLOCK_AIR || chunk_get(chunk, x + 1, y, z)->solid)) {
        return;
    }

    Chunk *neighbour = get_chunk(chunk->pos.x + 1, chunk->pos.y);
    if(neighbour) {
        Block *corresponding_block = chunk_get(neighbour, 0, y, z);
        if(x == CHUNK_WIDTH - 1 && corresponding_block->type != BLOCK_AIR) {
            return;
        }
    } else {
        if(x == CHUNK_WIDTH - 1) {
            return;
        }
    }

    Block *block = chunk_get(chunk, x, y, z);
    vec2s tex_coords = blocks[block->type].tex_coords.pos_x;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z},
        1.0f,
        tex_coords,
        0.8f
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z + 1},
        1.0f,
        tex_coords,
        0.8f
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z + 1},
        1.0f,
        tex_coords,
        0.8f
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z + 1},
        1.0f,
        tex_coords,
        0.8f
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords,
        0.8f
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z},
        1.0f,
        tex_coords,
        0.8f
    });
}

static void try_mesh_front_face(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(z > 0 && (chunk_get(chunk, x, y, z - 1)->type != BLOCK_AIR || chunk_get(chunk, x, y, z - 1)->solid)) {
        return;
    }

    Chunk *neighbour = get_chunk(chunk->pos.x, chunk->pos.y - 1);
    if(neighbour) {
        Block *corresponding_block = chunk_get(neighbour, x, y, CHUNK_DEPTH - 1);
        if(z == 0 && corresponding_block->type != BLOCK_AIR) {
            return;
        }
    } else {
        if(z == 0) {
            return;
        }
    }

    Block *block = chunk_get(chunk, x, y, z);
    vec2s tex_coords = blocks[block->type].tex_coords.neg_z;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z},
        1.0f,
        tex_coords,
        0.85f
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z},
        1.0f,
        tex_coords,
        0.85f
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords,
        0.85f
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords,
        0.85f
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z},
        1.0f,
        tex_coords,
        0.85f
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z},
        1.0f,
        tex_coords,
        0.85f
    });
}

static void try_mesh_back_face(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(z < CHUNK_DEPTH - 1 && (chunk_get(chunk, x, y, z + 1)->type != BLOCK_AIR || chunk_get(chunk, x, y, z + 1)->solid)) {
        return;
    }

    Chunk *neighbour = get_chunk(chunk->pos.x, chunk->pos.y + 1);
    if(neighbour) {
        Chunk *neighbour = get_chunk(chunk->pos.x, chunk->pos.y + 1);
        Block *corresponding_block = chunk_get(neighbour, x, y, 0);
        if(z == CHUNK_DEPTH - 1 && corresponding_block->type != BLOCK_AIR) {
            return;
        }
    } else {
        if(z == CHUNK_DEPTH - 1) {
            return;
        }
    }

    Block *block = chunk_get(chunk, x, y, z);
    vec2s tex_coords = blocks[block->type].tex_coords.pos_z;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z + 1},
        1.0f,
        tex_coords,
        0.85f
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z + 1},
        1.0f,
        tex_coords,
        0.85f
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z + 1},
        1.0f,
        tex_coords,
        0.85f
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z + 1},
        1.0f,
        tex_coords,
        0.85f
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z + 1},
        1.0f,
        tex_coords,
        0.85f
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z + 1},
        1.0f,
        tex_coords,
        0.85f
    });
}

static void try_mesh_bottom_face(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(y > 0 || chunk_get(chunk, x, y - 1, z)->type != BLOCK_AIR || chunk_get(chunk, x, y - 1, z)->solid) {
        return;
    }

    Block *block = chunk_get(chunk, x, y, z);
    vec2s tex_coords = blocks[block->type].tex_coords.neg_y;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z},
        1.0f,
        tex_coords,
        0.6f
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z},
        1.0f,
        tex_coords,
        0.6f
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z + 1},
        1.0f,
        tex_coords,
        0.6f
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z + 1},
        1.0f,
        tex_coords,
        0.6f
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z + 1},
        1.0f,
        tex_coords,
        0.6f
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z},
        1.0f,
        tex_coords,
        0.6f
    });
}

static void try_mesh_top_face(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(y == CHUNK_HEIGHT || chunk_get(chunk, x, y + 1, z)->type != BLOCK_AIR || chunk_get(chunk, x, y + 1, z)->solid) {
        return;
    }

    Block *block = chunk_get(chunk, x, y, z);
    vec2s tex_coords = blocks[block->type].tex_coords.pos_y;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z + 1},
        1.0f,
        tex_coords,
        1.0f
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z + 1},
        1.0f,
        tex_coords,
        1.0f
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords,
        1.0f
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords,
        1.0f
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z},
        1.0f,
        tex_coords,
        1.0f
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z + 1},
        1.0f,
        tex_coords,
        1.0f
    });
}

void mesh_chunk(Chunk *chunk, bool update_flag) {
    chunk->mesh.vertex_count = 0;

    for(u8 x = 0; x < CHUNK_WIDTH; x++) {
        for(u8 y = 0; y < CHUNK_HEIGHT; y++) {
            for(u8 z = 0; z < CHUNK_DEPTH; z++) {
                Block *block = chunk_get(chunk, x, y, z);
                if(block->type != BLOCK_AIR) {
                    try_mesh_left_face(chunk, x, y, z);
                    try_mesh_right_face(chunk, x, y, z);
                    try_mesh_front_face(chunk, x, y, z);
                    try_mesh_back_face(chunk, x, y, z);
                    try_mesh_bottom_face(chunk, x, y, z);
                    try_mesh_top_face(chunk, x, y, z);
                }
            }
        }
    }

    if(update_flag) {
        chunk->mesh.should_update = false;
    }
}

static void mesh_chunk_neighbours(Chunk *chunk) {
    Chunk *left = get_chunk(chunk->pos.x - 1, chunk->pos.y);
    Chunk *right = get_chunk(chunk->pos.x + 1, chunk->pos.y);
    Chunk *back = get_chunk(chunk->pos.x, chunk->pos.y + 1);
    Chunk *front = get_chunk(chunk->pos.x, chunk->pos.y - 1);
    if(left) {
        mesh_chunk(left, false);
    }
    if(right) {
        mesh_chunk(right, false);
    }
    if(back) {
        mesh_chunk(back, false);
    }
    if(front) {
        mesh_chunk(front, false);
    }
}

Block *chunk_get(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(x >= CHUNK_WIDTH || y >= CHUNK_HEIGHT || z >= CHUNK_DEPTH) {
        return &blocks[BLOCK_AIR];
    }
    return &blocks[chunk->blocks[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_DEPTH)]];
}

void chunk_set(Chunk *chunk, const Block *block, u8 x, u8 y, u8 z) {
    if(x >= CHUNK_WIDTH || y >= CHUNK_HEIGHT || z >= CHUNK_DEPTH) {
        return;
    }

    chunk->blocks[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_DEPTH)] = block->type;
}

static void gen_chunk(Chunk *chunk) {
    for(u8 x = 0; x < CHUNK_WIDTH; x++) {
        for(u8 z = 0; z < CHUNK_DEPTH; z++) {
            f32 a = perlin(
                (i32)(x + chunk->pos.x * 16) + INT16_MAX,
                (i32)(z + chunk->pos.y * 16) + INT16_MAX, 0.02f, 16);
            i32 h = a * 31.0f;
            for(i32 y = 0; y <= h; y++) {
                if(y == h) {
                    chunk_set(chunk, &blocks[BLOCK_GRASS], x, y, z);
                } else {
                    chunk_set(chunk, &blocks[BLOCK_DIRT], x, y, z);
                }
            }
        }
    }
}

World *init_world() {
    world.chunks = calloc(SQ(LOAD_WIDTH), sizeof(Chunk*));
    world.chunk_count = 0;
    return &world;
}

Chunk *get_chunk(i32 x, i32 y) {
    for(i32 i = 0; i < SQ(LOAD_WIDTH); i++) {
        Chunk *chunk = world.chunks[i];
        if(chunk) {
            if(chunk->pos.x == x && chunk->pos.y == y) {
                return chunk;
            }
        }
    }
    return NULL;
}

static Chunk *add_chunk(i32 x, i32 y) {
    Chunk *chunk = calloc(1, sizeof(Chunk));
    chunk->pos = (ivec2s) {x, y};
    chunk->mesh.should_update = true;
    world.chunks[world.chunk_count] = chunk;
    world.chunk_count++;
    return chunk;
}

void update_world() {
    load_chunks();
    for(i32 i = 0; i < world.chunk_count; i++) {
        Chunk *chunk = world.chunks[i];
        if(chunk->mesh.should_update) {
            mesh_chunk(chunk, true);
            mesh_chunk_neighbours(chunk);
            return;
        }
    }
}

void load_chunks() {
    i32 chunk_pos_x = player.pos.x / CHUNK_WIDTH;
    i32 chunk_pos_z = player.pos.z / CHUNK_DEPTH;

    // Indices of chunks to remove
    i32 chunks_to_remove[SQ(LOAD_WIDTH)];
    i32 a = 0;

    for(i32 i = 0; i < SQ(LOAD_WIDTH); i++) {
        Chunk *chunk = world.chunks[i];
        if(chunk) {
            if(fabsf(chunk->pos.x - chunk_pos_x) > LOAD_DISTANCE) {
                chunks_to_remove[a] = i;
                a++;
            } else if(fabsf(chunk->pos.y - chunk_pos_z) > LOAD_DISTANCE) {
                chunks_to_remove[a] = i;
                a++;
            }
        }
    }

    if(a > 0) {
        for(i32 i = a - 1; i >= 0; i--) {
            Chunk *chunk = world.chunks[chunks_to_remove[i]];
            i32 index = chunks_to_remove[i];
            if(chunk) {
                destroy_chunk(chunk);
                free(chunk);
                world.chunks[index] = NULL;
                if(index < SQ(LOAD_WIDTH) - 1) {
                    for(i32 j = index; j < SQ(LOAD_WIDTH) - 1; j++) {
                        world.chunks[j] = world.chunks[j + 1];
                    }
                    world.chunks[world.chunk_count - 1] = NULL;
                }
                world.chunk_count--;
            }
        }
    }

    for(i32 x = -LOAD_DISTANCE; x <= LOAD_DISTANCE; x++) {
        for(i32 z = -LOAD_DISTANCE; z <= LOAD_DISTANCE; z++) {
            Chunk *chunk = get_chunk(x + chunk_pos_x, z + chunk_pos_z);
            if(!chunk) {
                Chunk *new_chunk = add_chunk(x + chunk_pos_x, z + chunk_pos_z);
                gen_chunk(new_chunk);
            }
        }
    }
}

void world_set(const Block *block, u8 x, u8 y, u8 z) {
    i32 chunk_pos_x = x / 16.0f;
    i32 chunk_pos_z = z / 16.0f;

    Chunk *chunk = get_chunk(chunk_pos_x, chunk_pos_z);
    if(!chunk) {
        // TODO add support for unloaded chunks
        return;
    }

    i32 chunk_x = x % CHUNK_WIDTH;
    i32 chunk_z = z % CHUNK_DEPTH;

    chunk_set(chunk, block, chunk_x, y, chunk_z);
}

Block *world_get(u8 x, u8 y, u8 z) {
    i32 chunk_pos_x = x / 16.0f;
    i32 chunk_pos_z = z / 16.0f;

    Chunk *chunk = get_chunk(chunk_pos_x, chunk_pos_z);
    if(!chunk) {
        return NULL;
    }

    i32 chunk_x = x % CHUNK_WIDTH;
    i32 chunk_z = z % CHUNK_DEPTH;

    return chunk_get(chunk, chunk_x, y, chunk_z);
}

void destroy_world() {
    for(i32 i = 0; i < SQ(LOAD_WIDTH); i++) {
        Chunk *chunk = world.chunks[i];
        if(chunk) {
            destroy_chunk(chunk);
            free(chunk);
        }
    }
}