#include "jsmnutils.h"
#include <fstream>
#include <vector>
#include <cstring>

// Exceptions
JSMNUtilsEx jsmn_invalid_type_ex("Invalid lvalue type.");
JSMNUtilsEx jsmn_key_not_found_ex("Key does not exist.");
JSMNUtilsEx jsmn_array_out_of_bounds_ex("Array index out of bounds.");

JSONObject::JSONObject(jsmntok_t *tokens, size_t toksize, const char *buffer) :
    m_tokens(tokens),
    m_toksize(toksize),
    m_buffer(buffer)
{
}

JSONElement JSONObject::operator[](const std::string &key) const
{
    // Iterate over the tokens
    for (int i = 0; i < m_toksize; i++) {
        // Check if this token is the key we are looking for
        if (strncmp(key.c_str(), m_buffer + m_tokens[i].start, m_tokens[i].end - m_tokens[i].start) == 0) {
            // Check size - strn* strikes again
            if (key.size() != m_tokens[i].end - m_tokens[i].start) {
                continue;
            }
            // Check this token has children
            if (m_tokens[i].size == 0) {
                // not a key
                continue;
            }
            // Check that this is not the end (it should not)
            if (i + 1 == m_toksize) {
                return JSONElement(nullptr, 0, nullptr);
            }
            return JSONElement(m_tokens + i + 1,
                    m_toksize - i - 1, m_buffer);
        }
    }
    // Could not find key
    throw jsmn_key_not_found_ex;
}

JSONArray::JSONArray(jsmntok_t *tokens, size_t toksize, const char *buffer) :
    m_tokens(tokens),
    m_toksize(toksize),
    m_buffer(buffer)
{
}

JSONElement JSONArray::operator[](size_t i) const
{
    if (i >= m_tokens->size) {
        throw jsmn_array_out_of_bounds_ex; 
    }
    // First array's item is next token
    size_t offset = 1;
    // Consume requested index
    while (i--) {
        // Update the offset to the token who's starting mark
        // is greater that current's ending
        for (int j = offset + 1 ; j < m_toksize; j++) {
            if (m_tokens[j].start > m_tokens[offset].end) {
                offset = j;
                goto continue_while;
            }
        }
        // Index out of bounds!
        throw jsmn_array_out_of_bounds_ex;
        // Exit to outer loop
        continue_while:
            ; // no op
    }
    // Return JSONElement
    return JSONElement(m_tokens + offset,
            m_toksize - offset, m_buffer);
}

size_t JSONArray::size() const
{
    return m_tokens->size;
}

JSONElement::JSONElement(jsmntok_t *tokens, size_t toksize, const char *buffer) :
    m_tokens(tokens),
    m_toksize(toksize),
    m_buffer(buffer)
{
}

bool JSONElement::is_valid() const
{
    return m_tokens != nullptr &&
        m_toksize > 0 && m_buffer != nullptr;
}

JSONElement::operator JSONObject()
{
    if (!is_valid() || m_tokens->type != JSMN_OBJECT) {
        throw jsmn_invalid_type_ex;
    }
    return JSONObject(m_tokens,
            m_toksize, m_buffer);
}

JSONElement::operator std::string()
{
    if (!is_valid() || m_tokens->type != JSMN_STRING) {
        throw jsmn_invalid_type_ex;
    }
    return std::string(m_buffer + m_tokens->start,
            m_tokens->end - m_tokens->start);
}

JSONElement::operator JSONArray()
{
    if (!is_valid() || m_tokens->type != JSMN_ARRAY) {
        throw jsmn_invalid_type_ex;
    }
    return JSONArray(m_tokens,
            m_toksize, m_buffer);
}

// Is the primitive, a numner?
int isnumber(char c)
{
    return (c >= '0' && c <= '9') || c == '-';
}

JSONElement::operator int()
{
    if (!is_valid() || m_tokens->type != JSMN_PRIMITIVE) {
        throw jsmn_invalid_type_ex;
    }
    if (!isnumber(m_buffer[m_tokens->start])) {
        throw jsmn_invalid_type_ex;
    }
    char *endptr;
    char buffer[32];
    std::strncpy(buffer, m_buffer + m_tokens->start,
            m_tokens->end - m_tokens->start);
    buffer[m_tokens->end - m_tokens->start] = 0;
    int value = std::strtol(buffer, &endptr, 10);
    // Check if parsing succeeded
    if (*endptr == 0) {
        return value;
    }
    throw jsmn_invalid_type_ex;
}            

JSONParser::JSONParser() :
    m_tokens(nullptr),
    m_toksize(0),
    m_buffer_loaded(false)
{
    jsmn_init(&m_parser);
}

JSONParser::~JSONParser()
{
    delete[] m_tokens;
}

void JSONParser::load_file(std::string filename)
{
    std::ifstream is(filename);
    // If couldn't load
    if (!is) {
        return;
    }
    // Ignore previous data if any
    m_buffer.resize(256, 0);
    size_t len = 0;

    while (!is.eof()) {
        if (len > 0) {
            m_buffer.resize(m_buffer.size() +
                    m_buffer.size() / 2, 0);
        }
        len += is.read(&m_buffer[len], m_buffer.size())
            .gcount();
    }
    m_buffer.resize(len);
    m_buffer_loaded = true;
}

std::vector<char> &JSONParser::buffer()
{
    return m_buffer;
}

JSONElement JSONParser::parse()
{
    // First run to count tokens
    jsmn_init(&m_parser);
    int toksize = jsmn_parse(&m_parser, &m_buffer[0],
            m_buffer.size() - 1, NULL, 0);
    if (toksize < 0) {
        return JSONElement(nullptr, 0, nullptr);
    }
    delete[] m_tokens;
    m_tokens = new jsmntok_t[toksize];
    m_toksize = toksize;
    // Parse tokens
    jsmn_init(&m_parser); // must re-initialize
    jsmn_parse(&m_parser, &m_buffer[0],
            m_buffer.size() - 1, m_tokens, m_toksize);
    return JSONElement(m_tokens, m_toksize, &m_buffer[0]);
}
