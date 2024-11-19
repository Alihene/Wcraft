#include "world.h"
#include "noise.h"
#include "player.h"
#include "xorshift.h"

// Tree generation chance per block (1/n)
#define TREE_GENERATION_CHANCE 200

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
        .solid = false,
        .transparent = true
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
        .solid = true,
        .transparent = false
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
        .solid = true,
        .transparent = false
    };
    blocks[BLOCK_STONE] = (Block) {
        .type = BLOCK_STONE,
        .tex_coords = {
            .neg_x = (vec2s) {1 / 8.0f, 0},
            .pos_x = (vec2s) {1 / 8.0f, 0},
            .neg_y = (vec2s) {1 / 8.0f, 0},
            .pos_y = (vec2s) {1 / 8.0f, 0},
            .neg_z = (vec2s) {1 / 8.0f, 0},
            .pos_z = (vec2s) {1 / 8.0f, 0},
        },
        .solid = true,
        .transparent = false
    };
    blocks[BLOCK_GLASS] = (Block) {
        .type = BLOCK_GLASS,
        .tex_coords = {
            .neg_x = (vec2s) {4 / 8.0f, 0},
            .pos_x = (vec2s) {4 / 8.0f, 0},
            .neg_y = (vec2s) {4 / 8.0f, 0},
            .pos_y = (vec2s) {4 / 8.0f, 0},
            .neg_z = (vec2s) {4 / 8.0f, 0},
            .pos_z = (vec2s) {4 / 8.0f, 0},
        },
        .solid = true,
        .transparent = true
    };
    blocks[BLOCK_PLANKS] = (Block) {
        .type = BLOCK_PLANKS,
        .tex_coords = {
            .neg_x = (vec2s) {5 / 8.0f, 0},
            .pos_x = (vec2s) {5 / 8.0f, 0},
            .neg_y = (vec2s) {5 / 8.0f, 0},
            .pos_y = (vec2s) {5 / 8.0f, 0},
            .neg_z = (vec2s) {5 / 8.0f, 0},
            .pos_z = (vec2s) {5 / 8.0f, 0},
        },
        .solid = true,
        .transparent = false
    };
    blocks[BLOCK_COBBLESTONE] = (Block) {
        .type = BLOCK_COBBLESTONE,
        .tex_coords = {
            .neg_x = (vec2s) {6 / 8.0f, 0},
            .pos_x = (vec2s) {6 / 8.0f, 0},
            .neg_y = (vec2s) {6 / 8.0f, 0},
            .pos_y = (vec2s) {6 / 8.0f, 0},
            .neg_z = (vec2s) {6 / 8.0f, 0},
            .pos_z = (vec2s) {6 / 8.0f, 0},
        },
        .solid = true,
        .transparent = false
    };
    blocks[BLOCK_LOG] = (Block) {
        .type = BLOCK_LOG,
        .tex_coords = {
            .neg_x = (vec2s) {7 / 8.0f, 0},
            .pos_x = (vec2s) {7 / 8.0f, 0},
            .neg_y = (vec2s) {0, 1.0f / 8.0f},
            .pos_y = (vec2s) {0, 1.0f / 8.0f},
            .neg_z = (vec2s) {7 / 8.0f, 0},
            .pos_z = (vec2s) {7 / 8.0f, 0},
        },
        .solid = true,
        .transparent = false
    };
    blocks[BLOCK_SAND] = (Block) {
        .type = BLOCK_SAND,
        .tex_coords = {
            .neg_x = (vec2s) {2 / 8.0f, 1.0f / 8.0f},
            .pos_x = (vec2s) {2 / 8.0f, 1.0f / 8.0f},
            .neg_y = (vec2s) {2 / 8.0f, 1.0f / 8.0f},
            .pos_y = (vec2s) {2 / 8.0f, 1.0f / 8.0f},
            .neg_z = (vec2s) {2 / 8.0f, 1.0f / 8.0f},
            .pos_z = (vec2s) {2 / 8.0f, 1.0f / 8.0f},
        },
        .solid = true,
        .transparent = false
    };
    blocks[BLOCK_LEAVES] = (Block) {
        .type = BLOCK_LEAVES,
        .tex_coords = {
            .neg_x = (vec2s) {3 / 8.0f, 1.0f / 8.0f},
            .pos_x = (vec2s) {3 / 8.0f, 1.0f / 8.0f},
            .neg_y = (vec2s) {3 / 8.0f, 1.0f / 8.0f},
            .pos_y = (vec2s) {3 / 8.0f, 1.0f / 8.0f},
            .neg_z = (vec2s) {3 / 8.0f, 1.0f / 8.0f},
            .pos_z = (vec2s) {3 / 8.0f, 1.0f / 8.0f},
        },
        .solid = true,
        .transparent = true
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
    Block *block = chunk_get(chunk, x, y, z);
    if(x > 0) {
        Block *other_block = chunk_get(chunk, x - 1, y, z);
        bool both_transparent = block->transparent && other_block->transparent;
        bool both_non_transparent = !block->transparent && !other_block->transparent;
        bool both_same_type = block->type == other_block->type;

        if((both_transparent && both_same_type)
            || (both_non_transparent)
            || (block->transparent && !both_transparent && !both_non_transparent)) {
            return;
        }
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
    Block *block = chunk_get(chunk, x, y, z);
    if(x < CHUNK_WIDTH - 1) {
        Block *other_block = chunk_get(chunk, x + 1, y, z);
        bool both_transparent = block->transparent && other_block->transparent;
        bool both_non_transparent = !block->transparent && !other_block->transparent;
        bool both_same_type = block->type == other_block->type;

        if((both_transparent && both_same_type)
            || (both_non_transparent)
            || (block->transparent && !both_transparent && !both_non_transparent)) {
            return;
        }
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
    Block *block = chunk_get(chunk, x, y, z);
    if(z > 0) {
        Block *other_block = chunk_get(chunk, x, y, z - 1);
        bool both_transparent = block->transparent && other_block->transparent;
        bool both_non_transparent = !block->transparent && !other_block->transparent;
        bool both_same_type = block->type == other_block->type;

        if((both_transparent && both_same_type)
            || (both_non_transparent)
            || (block->transparent && !both_transparent && !both_non_transparent)) {
            return;
        }
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
    Block *block = chunk_get(chunk, x, y, z);
    if(z < CHUNK_DEPTH - 1) {
        Block *other_block = chunk_get(chunk, x, y, z + 1);
        bool both_transparent = block->transparent && other_block->transparent;
        bool both_non_transparent = !block->transparent && !other_block->transparent;
        bool both_same_type = block->type == other_block->type;

        if((both_transparent && both_same_type)
            || (both_non_transparent)
            || (block->transparent && !both_transparent && !both_non_transparent)) {
            return;
        }
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
    Block *block = chunk_get(chunk, x, y, z);
    if(y > 0) {
        Block *other_block = chunk_get(chunk, x, y - 1, z);
        bool both_transparent = block->transparent && other_block->transparent;
        bool both_non_transparent = !block->transparent && !other_block->transparent;
        bool both_same_type = block->type == other_block->type;

        if((both_transparent && both_same_type)
            || (both_non_transparent)
            || (block->transparent && !both_transparent && !both_non_transparent)) {
            return;
        }
    }

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
    Block *block = chunk_get(chunk, x, y, z);
    if(y < CHUNK_HEIGHT - 1) {
        Block *other_block = chunk_get(chunk, x, y + 1, z);
        bool both_transparent = block->transparent && other_block->transparent;
        bool both_non_transparent = !block->transparent && !other_block->transparent;
        bool both_same_type = block->type == other_block->type;

        if((both_transparent && both_same_type)
            || (both_non_transparent)
            || (block->transparent && !both_transparent && !both_non_transparent)) {
            return;
        }
    }

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

static void world_set_unloaded(const Block *block, i32 x, i32 y, i32 z) {
    BlockSet block_set = (BlockSet) {
        .pos = (ivec3s) {x, y, z},
        .block = block
    };

    if(!world.block_set_list.block_sets) {
        world.block_set_list.block_sets = malloc(32 * sizeof(BlockSet));
        world.block_set_list.allocated = 32;
    }

    if(world.block_set_list.count >= world.block_set_list.allocated) {
        world.block_set_list.block_sets = realloc(
            world.block_set_list.block_sets,
            2 * world.block_set_list.allocated * sizeof(BlockSet));
        world.block_set_list.allocated *= 2;
    }

    world.block_set_list.block_sets[world.block_set_list.count] = block_set;
    world.block_set_list.count++;
}

static void world_remove_block_set(u32 index) {
    if(index >= world.block_set_list.count) {
        return;
    }

    if(index < world.block_set_list.allocated - 1) {
        for(u32 i = index; i < world.block_set_list.count - 1; i++) {
            world.block_set_list.block_sets[i] = world.block_set_list.block_sets[i + 1];
        }
    }

    world.block_set_list.count--;
}

static void store_chunk(const Chunk *chunk) {
    if(!world.chunk_storage.chunks) {
        world.chunk_storage.chunks = malloc(128 * sizeof(StoredChunk));
        world.chunk_storage.allocated = 128;
    }

    if(world.chunk_storage.count >= world.chunk_storage.allocated) {
        world.chunk_storage.chunks = realloc(
            world.chunk_storage.chunks,
            2 * world.chunk_storage.allocated * sizeof(StoredChunk));
        world.chunk_storage.allocated *= 2;
    }
    
    StoredChunk *ptr = &world.chunk_storage.chunks[world.chunk_storage.count];
    ptr->pos = chunk->pos;
    memcpy(ptr->blocks, chunk->blocks, sizeof(chunk->blocks));
    world.chunk_storage.count++;
}

static void remove_stored_chunk(u32 index) {
    if(index >= world.chunk_storage.count) {
        return;
    }

    if(index < world.chunk_storage.allocated - 1) {
        for(u32 i = index; i < world.chunk_storage.count - 1; i++) {
            world.chunk_storage.chunks[i] = world.chunk_storage.chunks[i + 1];
        }
    }

    world.chunk_storage.count--;
}

static void gen_chunk(Chunk *chunk) {
    ivec3s tree_positions[CHUNK_WIDTH * CHUNK_HEIGHT];
    u32 tree_count = 0;

    for(i32 i = world.chunk_storage.count - 1; i >= 0; i--) {
        StoredChunk *c = &world.chunk_storage.chunks[i];
        if(c->pos.x == chunk->pos.x && c->pos.y == chunk->pos.y) {
            memcpy(chunk->blocks, c->blocks, sizeof(chunk->blocks));
            remove_stored_chunk(i);
            return;
        }
    }

    for(u8 x = 0; x < CHUNK_WIDTH; x++) {
        for(u8 z = 0; z < CHUNK_DEPTH; z++) {
            f32 a = perlin(
                (i32)(x + chunk->pos.x * 16) + INT16_MAX,
                (i32)(z + chunk->pos.y * 16) + INT16_MAX, 0.005f, 8);
            i32 h = a * 100.0f;
            for(i32 y = 0; y <= h; y++) {
                if(y == h) {
                    chunk_set(chunk, &blocks[BLOCK_GRASS], x, y, z);
                } else if(h - y <= 4) {
                    chunk_set(chunk, &blocks[BLOCK_DIRT], x, y, z);
                } else {
                    chunk_set(chunk, &blocks[BLOCK_STONE], x, y, z);
                }
            }

            u32 num = xorshift32() % TREE_GENERATION_CHANCE;
            if(!num) {
                tree_positions[tree_count] = (ivec3s) {x, h + 1, z};
                tree_count++;
            }
        }
    }

    for(u32 i = 0; i < tree_count; i++) {
        ivec3s pos = tree_positions[i];
        for(i32 y = pos.y; y < pos.y + 6; y++) {
            chunk_set(chunk, &blocks[BLOCK_LOG], pos.x, y, pos.z);
        }

        for(i32 x = pos.x - 2; x <= pos.x + 2; x++) {
            for(i32 z = pos.z - 2; z <= pos.z + 2; z++) {
                if(x != pos.x || z != pos.z) {
                    world_set(&blocks[BLOCK_LEAVES], x + chunk->pos.x * 16, pos.y + 3, z + chunk->pos.y * 16);
                    world_set(&blocks[BLOCK_LEAVES], x + chunk->pos.x * 16, pos.y + 4, z + chunk->pos.y * 16);
                }
            }
        }

        for(i32 x = pos.x - 1; x <= pos.x + 1; x++) {
            for(i32 z = pos.z - 1; z <= pos.z + 1; z++) {
                if(x != pos.x || z != pos.z) {
                    world_set(&blocks[BLOCK_LEAVES], x + chunk->pos.x * 16, pos.y + 5, z + chunk->pos.y * 16);
                }
                world_set(&blocks[BLOCK_LEAVES], x + chunk->pos.x * 16, pos.y + 6, z + chunk->pos.y * 16);
            }
        }
    }

    // Backwards iteration
    if(world.block_set_list.count > 0) {
        for(i32 i = world.block_set_list.count - 1; i >= 0; i--) {
            BlockSet block_set = world.block_set_list.block_sets[i];

            i32 chunk_pos_x = floorf(block_set.pos.x / 16.0f);
            i32 chunk_pos_z = floorf(block_set.pos.z / 16.0f);

            if(chunk_pos_x == chunk->pos.x
                && chunk_pos_z == chunk->pos.y) {
                i32 chunk_x = MOD(block_set.pos.x, CHUNK_WIDTH);
                i32 chunk_z = MOD(block_set.pos.z, CHUNK_DEPTH);

                chunk_set(chunk, block_set.block, chunk_x, block_set.pos.y, chunk_z);
                world_remove_block_set(i);
            }
        }
    }
}

World *init_world() {
    world.chunks = calloc(SQ(LOAD_WIDTH), sizeof(Chunk*));
    world.chunk_count = 0;

    world.block_set_list.block_sets = NULL;
    world.block_set_list.count = 0;

    set_xorshift32_seed(1);

    return &world;
}

Chunk *get_chunk(i32 x, i32 y) {
    for(u32 i = 0; i < world.chunk_count; i++) {
        Chunk *chunk = world.chunks[i];
        if(chunk->pos.x == x && chunk->pos.y == y) {
            return chunk;
        }
    }
    return NULL;
}

static Chunk *add_chunk(i32 x, i32 y) {
    Chunk *chunk = calloc(1, sizeof(Chunk));
    chunk->pos = (ivec2s) {x, y};
    chunk->mesh.should_update = true;
    chunk->mesh.being_rendered = false;
    world.chunks[world.chunk_count] = chunk;
    world.chunk_count++;
    return chunk;
}

void update_world() {
    load_chunks();
    for(u32 i = 0; i < world.chunk_count; i++) {
        Chunk *chunk = world.chunks[i];
        if(chunk->mesh.should_update) {
            mesh_chunk(chunk, true);
            mesh_chunk_neighbours(chunk);
            return;
        }
    }
}

void load_chunks() {
    i32 chunk_pos_x = floorf(player.pos.x / CHUNK_WIDTH);
    i32 chunk_pos_z = floorf(player.pos.z / CHUNK_DEPTH);

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
                store_chunk(chunk);
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

void world_set(const Block *block, i32 x, i32 y, i32 z) {
    i32 chunk_pos_x = floorf(x / 16.0f);
    i32 chunk_pos_z = floorf(z / 16.0f);

    Chunk *chunk = get_chunk(chunk_pos_x, chunk_pos_z);
    if(!chunk) {
        // Saved to an arraylist, will be used when the chunk is loaded
        world_set_unloaded(block, x, y, z);
        return;
    }

    i32 chunk_x = MOD(x, CHUNK_WIDTH);
    i32 chunk_z = MOD(z, CHUNK_DEPTH);

    chunk_set(chunk, block, chunk_x, y, chunk_z);
}

void world_set_and_mesh(const Block *block, i32 x, i32 y, i32 z) {
    world_set(block, x, y, z);

    i32 chunk_pos_x = floorf(x / 16.0f);
    i32 chunk_pos_z = floorf(z / 16.0f);

    Chunk *chunk = get_chunk(chunk_pos_x, chunk_pos_z);
    if(!chunk) {
        return;
    }

    mesh_chunk(chunk, true);
    mesh_chunk_neighbours(chunk);
}

Block *world_get(i32 x, i32 y, i32 z) {
    i32 chunk_pos_x = floorf(x / 16.0f);
    i32 chunk_pos_z = floorf(z / 16.0f);

    Chunk *chunk = get_chunk(chunk_pos_x, chunk_pos_z);
    if(!chunk) {
        return NULL;
    }

    i32 chunk_x = MOD(x, CHUNK_WIDTH);
    i32 chunk_z = MOD(z, CHUNK_DEPTH);

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