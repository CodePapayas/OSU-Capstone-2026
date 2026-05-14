/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462 
*/
#pragma once

#include <vector>
#include <memory>
#include <stdexcept>

using namespace std;

/**
 * CircularBuffer - FIFO ring buffer for simulation state history (1000-2000 ticks)
 * Enables O(1) rewind functionality by overwriting oldest data when full
 */
template<typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t capacity);
    
    void push(const T& item);           // Add element (overwrites oldest if full)
    void push(T&& item);
    
    const T& get(size_t index) const;   // Get by index (0=oldest, size-1=newest)
    T& get(size_t index);
    const T& latest() const;            // Most recent element
    T& latest();
    const T& rewind(size_t ticksAgo) const;  // Get N ticks in past
    
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    bool empty() const { return m_size == 0; }
    bool full() const { return m_size == m_capacity; }
    void clear();

private:
    vector<T> m_buffer;
    size_t m_capacity, m_head, m_size;
    size_t toPhysicalIndex(size_t logicalIndex) const;
};

// Template implementation
template<typename T>
CircularBuffer<T>::CircularBuffer(size_t capacity)
    : m_buffer(capacity)  // Pre-allocate full capacity
    , m_capacity(capacity)
    , m_head(0)           // Next write position
    , m_size(0)           // Current elements stored
{
    if (capacity == 0) {
        throw invalid_argument("CircularBuffer capacity must be > 0");
    }
}

template<typename T>
void CircularBuffer<T>::push(const T& item) {
    m_buffer[m_head] = item;
    m_head = (m_head + 1) % m_capacity;  // Wrap around using modulo
    if (m_size < m_capacity) {
        m_size++;  // Still filling, increment size
    }
    // If full, oldest element just got overwritten (head points to it now)
}

template<typename T>
void CircularBuffer<T>::push(T&& item) {
    m_buffer[m_head] = std::move(item);  // Move semantics for efficiency
    m_head = (m_head + 1) % m_capacity;
    if (m_size < m_capacity) {
        m_size++;
    }
}

template<typename T>
const T& CircularBuffer<T>::get(size_t index) const {
    if (index >= m_size) {
        throw out_of_range("CircularBuffer index out of range");
    }
    return m_buffer[toPhysicalIndex(index)];
}

template<typename T>
T& CircularBuffer<T>::get(size_t index) {
    if (index >= m_size) {
        throw out_of_range("CircularBuffer index out of range");
    }
    return m_buffer[toPhysicalIndex(index)];
}

template<typename T>
const T& CircularBuffer<T>::latest() const {
    if (empty()) {
        throw runtime_error("CircularBuffer is empty");
    }
    return get(m_size - 1);
}

template<typename T>
T& CircularBuffer<T>::latest() {
    if (empty()) {
        throw runtime_error("CircularBuffer is empty");
    }
    return get(m_size - 1);
}

template<typename T>
const T& CircularBuffer<T>::rewind(size_t ticksAgo) const {
    if (ticksAgo >= m_size) {
        throw out_of_range("Cannot rewind beyond buffer history");
    }
    return get(m_size - 1 - ticksAgo);
}

template<typename T>
void CircularBuffer<T>::clear() {
    m_head = 0;
    m_size = 0;
}

template<typename T>
size_t CircularBuffer<T>::toPhysicalIndex(size_t logicalIndex) const {
    // If not full yet, logical == physical
    // If full, need to offset by head position and wrap
    return m_size < m_capacity ? logicalIndex : (m_head + logicalIndex) % m_capacity;
}
