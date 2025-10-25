/**
 * @file ParanodeMessageQueue.h
 * @brief Lightweight circular message queue for offline buffering and batching
 * @author Muhammad Daffa
 * @date 2025-10-25
 *
 * Features:
 * - Circular buffer for efficient memory usage
 * - Message batching to reduce overhead
 * - Offline message buffering
 * - Configurable queue size
 * - No dynamic allocation after initialization
 */

#ifndef PARANODE_MESSAGE_QUEUE_H
#define PARANODE_MESSAGE_QUEUE_H

#include <Arduino.h>

// Default configuration
#ifndef PARANODE_QUEUE_SIZE
#define PARANODE_QUEUE_SIZE 20
#endif

#ifndef PARANODE_MAX_MESSAGE_SIZE
#define PARANODE_MAX_MESSAGE_SIZE 384
#endif

/**
 * @struct QueuedMessage
 * @brief Structure to hold queued message data
 */
struct QueuedMessage {
    char data[PARANODE_MAX_MESSAGE_SIZE];
    uint16_t length;
    unsigned long timestamp;
    uint8_t priority; // 0=low, 1=normal, 2=high, 3=critical
    bool valid;
};

/**
 * @class ParanodeMessageQueue
 * @brief Circular message queue with batching support
 */
class ParanodeMessageQueue {
public:
    /**
     * @brief Constructor
     */
    ParanodeMessageQueue();

    /**
     * @brief Enqueue a message
     * @param message Message data
     * @param length Message length
     * @param priority Message priority (0-3)
     * @return True if enqueued successfully
     */
    bool enqueue(const char* message, uint16_t length, uint8_t priority = 1);

    /**
     * @brief Dequeue a message
     * @param buffer Output buffer
     * @param bufferSize Buffer size
     * @return Length of dequeued message, 0 if queue empty
     */
    uint16_t dequeue(char* buffer, size_t bufferSize);

    /**
     * @brief Peek at next message without removing
     * @param buffer Output buffer
     * @param bufferSize Buffer size
     * @return Length of peeked message, 0 if queue empty
     */
    uint16_t peek(char* buffer, size_t bufferSize);

    /**
     * @brief Get number of messages in queue
     */
    size_t count() const { return _count; }

    /**
     * @brief Check if queue is empty
     */
    bool isEmpty() const { return _count == 0; }

    /**
     * @brief Check if queue is full
     */
    bool isFull() const { return _count >= PARANODE_QUEUE_SIZE; }

    /**
     * @brief Clear all messages
     */
    void clear();

    /**
     * @brief Get oldest message timestamp
     * @return Timestamp in milliseconds, 0 if empty
     */
    unsigned long getOldestTimestamp();

    /**
     * @brief Batch multiple messages into one
     * @param buffer Output buffer
     * @param bufferSize Buffer size
     * @param maxMessages Maximum messages to batch
     * @return Number of messages batched
     */
    int batchMessages(char* buffer, size_t bufferSize, int maxMessages = 5);

    /**
     * @brief Remove expired messages older than timeout
     * @param timeout Age in milliseconds
     * @return Number of messages removed
     */
    int removeExpired(unsigned long timeout);

private:
    QueuedMessage _messages[PARANODE_QUEUE_SIZE];
    size_t _head;
    size_t _tail;
    size_t _count;

    size_t nextIndex(size_t index) const;
};

#endif
