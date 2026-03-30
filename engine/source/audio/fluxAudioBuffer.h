//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// AudioBuffer - ThreadSafe
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <mutex>
#include <algorithm>
#include <cstring>

namespace FluxAudio {

class AudioBuffer {
private:
    std::vector<uint8_t> mBuffer;
    size_t mHead = 0; // write index
    size_t mTail = 0; // read index
    size_t mFullCount = 0;
    std::recursive_mutex mMutex;

public:
    AudioBuffer(size_t capacity) : mBuffer(capacity) {}
    // -------------------------------------------------------------------------
    void push(const uint8_t* data, size_t size) {
        std::lock_guard<std::recursive_mutex> lock(mMutex);

        if (size > mBuffer.size()) {
            data += (size - mBuffer.size());
            size = mBuffer.size();
        }

        size_t spaceToEnd = mBuffer.size() - mHead;
        size_t firstPart = std::min(size, spaceToEnd);
        size_t secondPart = size - firstPart;

        std::memcpy(&mBuffer[mHead], data, firstPart);
        if (secondPart > 0) {
            std::memcpy(mBuffer.data(), data + firstPart, secondPart);
        }

        mHead = (mHead + size) % mBuffer.size();

        if (mFullCount + size > mBuffer.size()) {
            mFullCount = mBuffer.size();
            mTail = mHead;
        } else {
            mFullCount += size;
        }
    }

    // void push(const uint8_t* data, size_t size) {
    //     std::lock_guard<std::recursive_mutex> lock(mMutex);
    //
    //     size_t spaceToEnd = mBuffer.size() - mHead;
    //     size_t firstPart = std::min(size, spaceToEnd);
    //     size_t secondPart = size - firstPart;
    //
    //     std::memcpy(&mBuffer[mHead], data, firstPart);
    //     if (secondPart > 0) {
    //         std::memcpy(mBuffer.data(), data + firstPart, secondPart);
    //     }
    //
    //     mHead = (mHead + size) % mBuffer.size();
    //     mFullCount = std::min(mBuffer.size(), mFullCount + size);
    // }
    // -------------------------------------------------------------------------
    size_t pop(uint8_t* out, size_t requestedSize) {
        std::lock_guard<std::recursive_mutex> lock(mMutex);

        size_t actualRead = std::min(requestedSize, mFullCount);
        if (actualRead == 0) return 0;

        size_t dataToEnd = mBuffer.size() - mTail;
        size_t firstPart = std::min(actualRead, dataToEnd);
        size_t secondPart = actualRead - firstPart;

        std::memcpy(out, &mBuffer[mTail], firstPart);
        if (secondPart > 0) {
            std::memcpy(out + firstPart, mBuffer.data(), secondPart);
        }

        mTail = (mTail + actualRead) % mBuffer.size();
        mFullCount -= actualRead;

        return actualRead;
    }
    // -------------------------------------------------------------------------
    size_t getAvailable() {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        return mFullCount;
    }
    // -------------------------------------------------------------------------
    void clear() {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        mHead = 0; // write index
        mTail = 0; // read index
        mFullCount = 0;
    }
};
};
