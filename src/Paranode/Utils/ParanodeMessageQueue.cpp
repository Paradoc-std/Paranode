/**
 * @file ParanodeMessageQueue.cpp
 * @brief Implementation of lightweight message queue
 * @author Muhammad Daffa
 * @date 2025-10-25
 */

#include "ParanodeMessageQueue.h"
#include <string.h>

ParanodeMessageQueue::ParanodeMessageQueue()
    : _head(0), _tail(0), _count(0) {
    // Initialize all messages as invalid
    for (size_t i = 0; i < PARANODE_QUEUE_SIZE; i++) {
        _messages[i].valid = false;
    }
}

bool ParanodeMessageQueue::enqueue(const char* message, uint16_t length, uint8_t priority) {
    if (!message || length == 0 || length >= PARANODE_MAX_MESSAGE_SIZE) {
        return false;
    }

    // If queue is full, make room based on priority
    if (isFull()) {
        // If new message is high priority (>=2), remove oldest low priority message
        if (priority >= 2) {
            // Find and remove oldest low priority message
            size_t checkIdx = _tail;
            for (size_t i = 0; i < _count; i++) {
                if (_messages[checkIdx].priority < 2) {
                    // Remove this message by marking invalid
                    _messages[checkIdx].valid = false;
                    // Shift tail if needed
                    if (checkIdx == _tail) {
                        _tail = nextIndex(_tail);
                        _count--;
                    }
                    break;
                }
                checkIdx = nextIndex(checkIdx);
            }
        }

        // Still full? Drop oldest message
        if (isFull()) {
            _tail = nextIndex(_tail);
            _count--;
        }
    }

    // Add new message
    memcpy(_messages[_head].data, message, length);
    _messages[_head].data[length] = '\0';
    _messages[_head].length = length;
    _messages[_head].timestamp = millis();
    _messages[_head].priority = priority;
    _messages[_head].valid = true;

    _head = nextIndex(_head);
    _count++;

    return true;
}

uint16_t ParanodeMessageQueue::dequeue(char* buffer, size_t bufferSize) {
    if (isEmpty() || !buffer) {
        return 0;
    }

    // Find next valid message
    while (!_messages[_tail].valid && !isEmpty()) {
        _tail = nextIndex(_tail);
        _count--;
    }

    if (isEmpty()) {
        return 0;
    }

    QueuedMessage& msg = _messages[_tail];
    uint16_t copyLen = msg.length < bufferSize ? msg.length : bufferSize - 1;

    memcpy(buffer, msg.data, copyLen);
    buffer[copyLen] = '\0';

    msg.valid = false;
    _tail = nextIndex(_tail);
    _count--;

    return copyLen;
}

uint16_t ParanodeMessageQueue::peek(char* buffer, size_t bufferSize) {
    if (isEmpty() || !buffer) {
        return 0;
    }

    // Find next valid message
    size_t peekIdx = _tail;
    while (!_messages[peekIdx].valid && peekIdx != _head) {
        peekIdx = nextIndex(peekIdx);
    }

    if (!_messages[peekIdx].valid) {
        return 0;
    }

    QueuedMessage& msg = _messages[peekIdx];
    uint16_t copyLen = msg.length < bufferSize ? msg.length : bufferSize - 1;

    memcpy(buffer, msg.data, copyLen);
    buffer[copyLen] = '\0';

    return copyLen;
}

void ParanodeMessageQueue::clear() {
    _head = 0;
    _tail = 0;
    _count = 0;

    for (size_t i = 0; i < PARANODE_QUEUE_SIZE; i++) {
        _messages[i].valid = false;
    }
}

unsigned long ParanodeMessageQueue::getOldestTimestamp() {
    if (isEmpty()) {
        return 0;
    }

    // Find oldest valid message
    size_t checkIdx = _tail;
    for (size_t i = 0; i < _count; i++) {
        if (_messages[checkIdx].valid) {
            return _messages[checkIdx].timestamp;
        }
        checkIdx = nextIndex(checkIdx);
    }

    return 0;
}

int ParanodeMessageQueue::batchMessages(char* buffer, size_t bufferSize, int maxMessages) {
    if (isEmpty() || !buffer || bufferSize < 50) {
        return 0;
    }

    size_t pos = 0;
    int batched = 0;

    // Start batch array
    buffer[pos++] = '[';

    size_t checkIdx = _tail;
    for (int i = 0; i < maxMessages && i < (int)_count && pos < bufferSize - 2; i++) {
        if (!_messages[checkIdx].valid) {
            checkIdx = nextIndex(checkIdx);
            continue;
        }

        QueuedMessage& msg = _messages[checkIdx];

        // Add comma separator if not first message
        if (batched > 0) {
            if (pos >= bufferSize - 2) break;
            buffer[pos++] = ',';
        }

        // Check if we have space for this message
        if (pos + msg.length + 2 >= bufferSize) {
            break;
        }

        // Copy message
        memcpy(buffer + pos, msg.data, msg.length);
        pos += msg.length;
        batched++;

        checkIdx = nextIndex(checkIdx);
    }

    // End batch array
    buffer[pos++] = ']';
    buffer[pos] = '\0';

    return batched;
}

int ParanodeMessageQueue::removeExpired(unsigned long timeout) {
    if (isEmpty()) {
        return 0;
    }

    unsigned long currentTime = millis();
    int removed = 0;

    size_t checkIdx = _tail;
    for (size_t i = 0; i < _count; i++) {
        if (_messages[checkIdx].valid) {
            // Handle millis() overflow
            unsigned long age;
            if (currentTime >= _messages[checkIdx].timestamp) {
                age = currentTime - _messages[checkIdx].timestamp;
            } else {
                age = (ULONG_MAX - _messages[checkIdx].timestamp) + currentTime;
            }

            if (age > timeout) {
                _messages[checkIdx].valid = false;
                removed++;
            }
        }
        checkIdx = nextIndex(checkIdx);
    }

    // Update count
    _count -= removed;

    // Update tail to skip invalid messages
    while (!isEmpty() && !_messages[_tail].valid) {
        _tail = nextIndex(_tail);
    }

    return removed;
}

size_t ParanodeMessageQueue::nextIndex(size_t index) const {
    return (index + 1) % PARANODE_QUEUE_SIZE;
}
