#include "world.h"
#include "noise.h"

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

    bool has_neighbour = chunk->relative_pos.x > 0;
    if(has_neighbour) {
        Chunk *neighbour = &world.chunks[chunk->relative_pos.y * LOAD_WIDTH + chunk->relative_pos.x - 1];
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
        tex_coords
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z},
        1.0f,
        tex_coords
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z},
        1.0f,
        tex_coords
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z},
        1.0f,
        tex_coords
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z + 1},
        1.0f,
        tex_coords
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z + 1},
        1.0f,
        tex_coords
    });
}

static void try_mesh_right_face(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(x < CHUNK_WIDTH - 1 && (chunk_get(chunk, x + 1, y, z)->type != BLOCK_AIR || chunk_get(chunk, x + 1, y, z)->solid)) {
        return;
    }

    bool has_neighbour = chunk->relative_pos.x < LOAD_WIDTH - 1;
    if(has_neighbour) {
        Chunk *neighbour = &world.chunks[chunk->relative_pos.y * LOAD_WIDTH + chunk->relative_pos.x + 1];
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
        tex_coords
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z + 1},
        1.0f,
        tex_coords
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z + 1},
        1.0f,
        tex_coords
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z + 1},
        1.0f,
        tex_coords
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z},
        1.0f,
        tex_coords
    });
}

static void try_mesh_front_face(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(z > 0 && (chunk_get(chunk, x, y, z - 1)->type != BLOCK_AIR || chunk_get(chunk, x, y, z - 1)->solid)) {
        return;
    }

    bool has_neighbour = chunk->relative_pos.y > 0;
    if(has_neighbour) {
        Chunk *neighbour = &world.chunks[(chunk->relative_pos.y - 1) * LOAD_WIDTH + chunk->relative_pos.x];
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
        tex_coords
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z},
        1.0f,
        tex_coords
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z},
        1.0f,
        tex_coords
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z},
        1.0f,
        tex_coords
    });
}

static void try_mesh_back_face(Chunk *chunk, u8 x, u8 y, u8 z) {
    if(z < CHUNK_DEPTH - 1 && (chunk_get(chunk, x, y, z + 1)->type != BLOCK_AIR || chunk_get(chunk, x, y, z + 1)->solid)) {
        return;
    }

    bool has_neighbour = chunk->relative_pos.y < LOAD_WIDTH - 1;
    if(has_neighbour) {
        Chunk *neighbour = &world.chunks[(chunk->relative_pos.y + 1) * LOAD_WIDTH + chunk->relative_pos.x];
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
        tex_coords
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z + 1},
        1.0f,
        tex_coords
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z + 1},
        1.0f,
        tex_coords
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z + 1},
        1.0f,
        tex_coords
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z + 1},
        1.0f,
        tex_coords
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z + 1},
        1.0f,
        tex_coords
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
        tex_coords
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z},
        1.0f,
        tex_coords
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z + 1},
        1.0f,
        tex_coords
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y, z + 1},
        1.0f,
        tex_coords
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z + 1},
        1.0f,
        tex_coords
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y, z},
        1.0f,
        tex_coords
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
        tex_coords
    });
    tex_coords.x += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z + 1},
        1.0f,
        tex_coords
    });
    tex_coords.y += 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords
    });
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x + 1, y + 1, z},
        1.0f,
        tex_coords
    });
    tex_coords.x -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z},
        1.0f,
        tex_coords
    });
    tex_coords.y -= 1.0f / 8.0f;
    push_vertex(chunk, &(Vertex) {
        (vec3s) {x, y + 1, z + 1},
        1.0f,
        tex_coords
    });
}

void mesh_chunk(Chunk *chunk) {
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
            f32 a = perlin(x + chunk->pos.x * 16, z + chunk->pos.y * 16, 0.05f, 8);
            i32 h = a * 16.0f;
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
    for(u8 i = 0; i < LOAD_WIDTH; i++) {
        for(u8 j = 0; j < LOAD_WIDTH; j++) {
            Chunk chunk = {0};
            chunk.pos = (ivec2s) {i, j};
            chunk.relative_pos = (ivec2s) {i, j};
            gen_chunk(&chunk);
            world.chunks[i + j * LOAD_WIDTH] = chunk;
        }
    }

    for(i32 i = 0; i < SQ(LOAD_WIDTH); i++) {
        mesh_chunk(&world.chunks[i]);
    }
    return &world;
}