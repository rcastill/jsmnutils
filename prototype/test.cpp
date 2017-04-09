#include <iostream>
#include <jsmn.h>
#include <cstring>
#include <cstdlib>
#include <cassert>

int isnumber(char c)
{
    return (c >= '0' && c <= '9') || c == '-';
}

int jsmn_geti(jsmntok_t *tokens, int size, const char *js, const char *key, int defvalue)
{
    char *endptr;
    char buffer[32];
    // Iterate over the tokens
    for (int i = 0; i < size; i++) {
        // Check if this token is the key we are looking for
        if (strncmp(key, &js[tokens[i].start], tokens[i].end - tokens[i].start) == 0) {
            // Check this token has children
            if (tokens[i].size == 0) {
                // not a key
                continue;
            }
            // Check that this is not the end (it should not)
            if (i + 1 == size) {
                return defvalue;
            }
            // Check that token is a primitive
            if (tokens[i + 1].type != JSMN_PRIMITIVE) {
                return defvalue;
            }
            // Check that primitive is a number
            if (!isnumber(js[tokens[i + 1].start])) {
                return defvalue;
            } 
            // Isolate number token
            strncpy(buffer, &js[tokens[i + 1].start], tokens[i + 1].end - tokens[i + 1].start);
            buffer[tokens[i + 1].end - tokens[i + 1].start] = 0;
            int value = strtol(buffer, &endptr, 10);
            // Check if parsing succeeded
            if (*endptr == 0) {
                return value;
            }
            // Failed to parse number
            return defvalue;
        }
    }
    // Could not find key
    return defvalue;
}

int jsmn_gets(jsmntok_t *tokens, int size, const char *js, const char *key, char *dst)
{
    for (int i = 0; i < size; i++) {
        if (strncmp(key, &js[tokens[i].start], tokens[i].end - tokens[i].start) == 0) {
            if (tokens[i].size == 0) {
                continue;
            }
            if (i + 1 == size) {
                return -1;
            }
            if (tokens[i + 1].type != JSMN_STRING) {
                return -1;
            }
            strncpy(dst, &js[tokens[i + 1].start], tokens[i + 1].end - tokens[i + 1].start);
            dst[tokens[i + 1].end - tokens[i + 1].start] = 0; // strncpy does not null-terminate
            return 0;
        }
    }
    return -1;
}

int jsmn_geta(jsmntok_t *tokens, int size, const char *js, const char *key)
{
    for (int i = 0; i < size; i++) {
        if (strncmp(key, &js[tokens[i].start], tokens[i].end - tokens[i].start) == 0) {
            if (tokens[i].size == 0) {
                continue;
            }
            if (i + 1 == size) {
                return -1;
            }
            if (tokens[i + 1].type != JSMN_ARRAY) {
                return -1;
            }
            return i + 1;
        }
    }
    return -1;
}

int jsmn_geto(jsmntok_t *tokens, int size, const char *js, const char *key)
{
    for (int i = 0; i < size; i++) {
        if (strncmp(key, &js[tokens[i].start], tokens[i].end - tokens[i].start) == 0) {
            if (tokens[i].size == 0) {
                continue;
            }
            if (i + 1 == size) {
                return -1;
            }
            if (tokens[i + 1].type != JSMN_OBJECT) {
                return -1;
            }
            return i + 1;
        }
    }
    return -1;
}

int jsmn_geti_at(jsmntok_t *tokens, const char *js, size_t i, int defvalue)
{
    if (tokens->type != JSMN_ARRAY) {
        return defvalue;
    }
    if (i >= tokens->size) {
        return defvalue;
    }
    if (tokens[i + 1].type != JSMN_PRIMITIVE) {
        return defvalue;
    }
    if (!isnumber(js[tokens[i + 1].start])) {
        return defvalue;
    }
    
    char *endptr;
    char buffer[32];

    strncpy(buffer, &js[tokens[i + 1].start], tokens[i + 1].end - tokens[i + 1].start);
    buffer[tokens[i + 1].end - tokens[i + 1].start] = 0;
    int value = strtol(buffer, &endptr, 10);

    if (*endptr == 0) {
        return value;
    }
    return defvalue;
}

int jsmn_geto_at(jsmntok_t *tokens, const char *js, size_t i)
{
    if (tokens->type != JSMN_ARRAY) {
        return -1;
    }
    if (i >= tokens->size) {
        return -1;
    }
    if (tokens[i + 1].type != JSMN_OBJECT) {
        return -1;
    }
    return i + 1;
}

void close_file(FILE **f)
{
    if (f && *f) {
        fclose(*f);
    }
}

int main(int argc, char **argv)
{
    jsmn_parser parser;
    jsmntok_t tokens[20];

    jsmn_init(&parser);

    const char *js = "{\
        \"hello\": \"this is sparta!\",\
        \"nested\": {\
            \"holo\": [1, 2, 3]\
        },\
        \"number\": 42\
    }";

    int parsed = jsmn_parse(&parser, js, strlen(js), tokens, 20);
    for (int i = 0; i < parsed; i++) {
        printf("tokens[%d].size = %d\n", i, tokens[i].size);
    }
    int i = jsmn_geti(tokens, parsed, js, "number", 0);
    printf("js[\"number\"] = %d\n", i);

    char buffer[256];
    jsmn_gets(tokens, parsed, js, "hello", buffer);
    printf("js[\"hello\"] = %s\n", buffer);

    int nested_offset = jsmn_geto(tokens, parsed, js, "nested");
    printf("nested_offset = %d\n", nested_offset);

    int array_offset = jsmn_geta(tokens + nested_offset,
            parsed - nested_offset, js, "holo") + nested_offset;
    printf("array_offset = %d\n", array_offset);

    int at = jsmn_geti_at(tokens + array_offset, js, 1, 0);
    printf("js[\"nested\"][\"holo\"][1] = %d\n", at);

    FILE *file __attribute__((cleanup (close_file)))
        = fopen("input_prototype.json", "r");
    
    // now read it and test it!
    size_t read = fread(buffer, 1, 256, file);
    buffer[read] = 0;

    char aux_buffer[256];
    jsmn_init(&parser);
    parsed = jsmn_parse(&parser, buffer, read, tokens, 20);

    jsmn_gets(tokens, parsed, buffer, "bind", aux_buffer);
    printf("%s / %lu\n", aux_buffer, strlen(aux_buffer));

    array_offset = jsmn_geta(tokens, parsed, buffer, "flows");
    nested_offset = jsmn_geto_at(tokens + array_offset, buffer, 0);

    assert(nested_offset != -1);

    int port = jsmn_geti(tokens + array_offset + nested_offset,
            parsed - array_offset - nested_offset,
            buffer, "port", 0);
    jsmn_gets(tokens + array_offset + nested_offset,
            parsed - array_offset - nested_offset,
            buffer, "file", aux_buffer);

    printf("port = %d\n", port);
    printf("file = %s\n", aux_buffer);
}
