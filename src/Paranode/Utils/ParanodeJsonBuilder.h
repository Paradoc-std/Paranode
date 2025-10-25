/**
 * @file ParanodeJsonBuilder.h
 * @brief Lightweight JSON builder for efficient message construction
 * @author Muhammad Daffa
 * @date 2025-10-25
 *
 * Optimized JSON builder that:
 * - Minimizes memory allocations
 * - Reuses buffers
 * - Avoids String class fragmentation
 * - Faster than ArduinoJson for simple cases
 */

#ifndef PARANODE_JSON_BUILDER_H
#define PARANODE_JSON_BUILDER_H

#include <Arduino.h>

/**
 * @class ParanodeJsonBuilder
 * @brief Lightweight JSON builder with buffer reuse
 */
class ParanodeJsonBuilder {
public:
    /**
     * @brief Constructor
     * @param buffer Pre-allocated buffer
     * @param size Buffer size
     */
    ParanodeJsonBuilder(char* buffer, size_t size);

    /**
     * @brief Reset builder for reuse
     */
    void reset();

    /**
     * @brief Start JSON object
     */
    void startObject();

    /**
     * @brief End JSON object
     */
    void endObject();

    /**
     * @brief Add string key-value pair
     */
    void addString(const char* key, const char* value);
    void addString(const char* key, const String& value);

    /**
     * @brief Add integer key-value pair
     */
    void addInt(const char* key, int value);
    void addLong(const char* key, long value);
    void addULong(const char* key, unsigned long value);

    /**
     * @brief Add float key-value pair
     */
    void addFloat(const char* key, float value, int decimals = 2);
    void addDouble(const char* key, double value, int decimals = 2);

    /**
     * @brief Add boolean key-value pair
     */
    void addBool(const char* key, bool value);

    /**
     * @brief Start nested object
     */
    void startNestedObject(const char* key);

    /**
     * @brief Get built JSON string
     */
    const char* getJson() const { return _buffer; }

    /**
     * @brief Get current JSON length
     */
    size_t length() const { return _position; }

    /**
     * @brief Check if buffer has space
     */
    bool hasSpace(size_t needed) const { return (_position + needed) < _size; }

private:
    char* _buffer;
    size_t _size;
    size_t _position;
    bool _firstElement;

    void addCommaIfNeeded();
    void appendEscaped(const char* str);
    void appendChar(char c);
    void appendString(const char* str);
    void appendNumber(long value);
    void appendFloat(double value, int decimals);
};

#endif
