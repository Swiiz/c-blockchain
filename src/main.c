#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "sha256.h"

// The number of byte equal to NULL that must appear at the beginning of every block's hash
const unsigned int BLOCKCHAIN_DIFFICULTY = 2;

typedef unsigned char* Hash;
typedef struct {
    char* data;

    Hash previous_hash;
    time_t timestamp;
    unsigned int nonce;
} Block;
typedef struct {
    Block** blocks;
    int length;
    int capacity;
} Blockchain;

char* hash_to_string(Hash hash) {
    if(hash == NULL) {
        return NULL;
    }
    char* string = malloc(sizeof(char) * 64);
    for (int i = 0; i < 64; i++) {
        sprintf(string + i, "%02x", hash[i]);
    }
    return string;
}

Block* block_new_genesis(char* data) {
    Block* block = malloc(sizeof(Block));
    block->data = data;
    block->previous_hash = NULL;
    block->timestamp = time(NULL);
    return block;
}

Hash block_calculate_hash(Block* block) {
    Hash hash = malloc(SHA256_BLOCK_SIZE);
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (unsigned char*) block, sizeof(Block));
    sha256_final(&ctx, hash);
    return hash;
}

bool hash_is_valid(Hash hash) {
    for(int i = 0; i < BLOCKCHAIN_DIFFICULTY; i++) {
        if(hash[i] != '\0') {
            return false;
        }
    }
    return true;
}

Block* block_next(Block* previous_block, char* data) {
    Block* block = malloc(sizeof(Block));
    block->data = data;
    block->previous_hash = block_calculate_hash(previous_block);
    block->timestamp = time(NULL);
    block->nonce = 0;
    Hash hash;
    do {
        block->nonce++;
        if(block->nonce % 1000000 == 0) {
            printf("Calculating block hash, already tried %d different hashes...\n", block->nonce);
        }
        hash = block_calculate_hash(block);
    }while(!hash_is_valid(hash));
    free(hash);
    return block;
}

bool block_is_valid(Block* block, Block* previous_block) {
    Hash hash = block_calculate_hash(previous_block);
    return !memcmp(hash, block->previous_hash, SHA256_BLOCK_SIZE);
}

void block_print(Block* block) {
    printf("Block:\n");
    printf("\tData: %s\n", block->data);
    printf("\tPrevious hash: %s\n", hash_to_string(block->previous_hash));
    printf("\tTimestamp: %s\n", ctime(&block->timestamp));
    printf("\tHash: %s\n", hash_to_string(block_calculate_hash(block)));
}

void block_free(Block* block) {
    if(block->previous_hash != NULL) {
        free(block->previous_hash);
    }
    free(block);
}

Blockchain* blockchain_new(unsigned int capacity) {
    Blockchain* bc = malloc(sizeof(Blockchain));
    bc->blocks = malloc(sizeof(Block*) * capacity);
    bc->length = 0;
    bc->capacity = capacity;
    return bc;
}

void blockchain_append(Blockchain* bc, char* data) {
    if(bc->length >= bc->capacity) {
        bc->blocks = realloc(bc->blocks, sizeof(Block*) * bc->capacity * 2);
        bc->capacity *= 2;
    }
    if(bc->length == 0) {
        bc->blocks[0] = block_new_genesis(data);
    }else {
        bc->blocks[bc->length] = block_next(bc->blocks[bc->length - 1], data);
    }
    bc->length++;
}

bool blockchain_verify(Blockchain* bc) {
    for(int i = 1; i < bc->length; i++) {
        if(!block_is_valid(bc->blocks[i], bc->blocks[i - 1])) {
            return false;
        }
    }
    return true;
}

void blockchain_print(Blockchain* bc) {
    for(int i = 0; i < bc->length; i++) {
        block_print(bc->blocks[i]);
    }
}

void blockchain_free(Blockchain* bc) {
    for(int i = 0; i < bc->length; i++) {
        block_free(bc->blocks[i]);
    }
    free(bc->blocks);
    free(bc);
}

int main(void) {
    Blockchain* bc = blockchain_new(10);
    blockchain_append(bc, "Hello World!");
    blockchain_append(bc, "How are you?");
    blockchain_append(bc, "I'm fine!");

    blockchain_print(bc);
    printf("Blockchain valid: %s\n", blockchain_verify(bc) ? "true" : "false");

    bc->blocks[1]->data = "How do you feel?";
    printf("Blockchain valid: %s\n", blockchain_verify(bc) ? "true" : "false");

    blockchain_free(bc);
    return 0;
}