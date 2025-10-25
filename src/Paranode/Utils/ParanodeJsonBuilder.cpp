/**
 * @file ParanodeJsonBuilder.cpp
 * @brief Implementation of lightweight JSON builder
 * @author Muhammad Daffa
 * @date 2025-10-25
 */

#include "ParanodeJsonBuilder.h"

ParanodeJsonBuilder::ParanodeJsonBuilder(char* buffer, size_t size)
    : _buffer(buffer), _size(size), _position(0), _firstElement(true) {
    if (_buffer && _size > 0) {
        _buffer[0] = '\0';
    }
}

void ParanodeJsonBuilder::reset() {
    _position = 0;
    _firstElement = true;
    if (_buffer && _size > 0) {
        _buffer[0] = '\0';
    }
}

void ParanodeJsonBuilder::startObject() {
    appendChar('{');
    _firstElement = true;
}

void ParanodeJsonBuilder::endObject() {
    appendChar('}');
    _buffer[_position] = '\0';
}

void ParanodeJsonBuilder::addString(const char* key, const char* value) {
    if (!hasSpace(strlen(key) + strlen(value) + 10)) return;

    addCommaIfNeeded();
    appendChar('"');
    appendString(key);
    appendString("\":");
    appendChar('"');
    appendEscaped(value);
    appendChar('"');
}

void ParanodeJsonBuilder::addString(const char* key, const String& value) {
    addString(key, value.c_str());
}

void ParanodeJsonBuilder::addInt(const char* key, int value) {
    addLong(key, (long)value);
}

void ParanodeJsonBuilder::addLong(const char* key, long value) {
    if (!hasSpace(strlen(key) + 20)) return;

    addCommaIfNeeded();
    appendChar('"');
    appendString(key);
    appendString("\":");
    appendNumber(value);
}

void ParanodeJsonBuilder::addULong(const char* key, unsigned long value) {
    if (!hasSpace(strlen(key) + 20)) return;

    addCommaIfNeeded();
    appendChar('"');
    appendString(key);
    appendString("\":");

    // Convert unsigned long to string
    char numBuf[20];
    ultoa(value, numBuf, 10);
    appendString(numBuf);
}

void ParanodeJsonBuilder::addFloat(const char* key, float value, int decimals) {
    addDouble(key, (double)value, decimals);
}

void ParanodeJsonBuilder::addDouble(const char* key, double value, int decimals) {
    if (!hasSpace(strlen(key) + 30)) return;

    addCommaIfNeeded();
    appendChar('"');
    appendString(key);
    appendString("\":");
    appendFloat(value, decimals);
}

void ParanodeJsonBuilder::addBool(const char* key, bool value) {
    if (!hasSpace(strlen(key) + 10)) return;

    addCommaIfNeeded();
    appendChar('"');
    appendString(key);
    appendString("\":");
    appendString(value ? "true" : "false");
}

void ParanodeJsonBuilder::startNestedObject(const char* key) {
    if (!hasSpace(strlen(key) + 5)) return;

    addCommaIfNeeded();
    appendChar('"');
    appendString(key);
    appendString("\":{");
    _firstElement = true;
}

void ParanodeJsonBuilder::addCommaIfNeeded() {
    if (!_firstElement) {
        appendChar(',');
    }
    _firstElement = false;
}

void ParanodeJsonBuilder::appendEscaped(const char* str) {
    while (*str && _position < _size - 1) {
        if (*str == '"' || *str == '\\') {
            appendChar('\\');
        }
        appendChar(*str);
        str++;
    }
}

void ParanodeJsonBuilder::appendChar(char c) {
    if (_position < _size - 1) {
        _buffer[_position++] = c;
    }
}

void ParanodeJsonBuilder::appendString(const char* str) {
    while (*str && _position < _size - 1) {
        _buffer[_position++] = *str++;
    }
}

void ParanodeJsonBuilder::appendNumber(long value) {
    char numBuf[20];
    ltoa(value, numBuf, 10);
    appendString(numBuf);
}

void ParanodeJsonBuilder::appendFloat(double value, int decimals) {
    // Handle special cases
    if (isnan(value)) {
        appendString("null");
        return;
    }
    if (isinf(value)) {
        appendString(value > 0 ? "9999999" : "-9999999");
        return;
    }

    // Handle negative
    if (value < 0) {
        appendChar('-');
        value = -value;
    }

    // Integer part
    long intPart = (long)value;
    appendNumber(intPart);

    // Decimal part
    if (decimals > 0) {
        appendChar('.');
        double fracPart = value - intPart;
        for (int i = 0; i < decimals && _position < _size - 1; i++) {
            fracPart *= 10;
            int digit = (int)fracPart;
            appendChar('0' + digit);
            fracPart -= digit;
        }
    }
}
