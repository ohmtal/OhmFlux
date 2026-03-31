//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// AudioBuffer - using float- ThreadSafe
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <mutex>
#include <algorithm>
#include <cstring>

namespace FluxAudio {

class AudioBuffer {
private:
    std::vector<float> mBuffer;
    size_t mHead = 0; // write index
    size_t mTail = 0; // read index
    size_t mFullCount = 0;
    std::recursive_mutex mMutex;

public:
    AudioBuffer(size_t capacity) : mBuffer(capacity) {}
    // -------------------------------------------------------------------------
    size_t getCapacity() const { return mBuffer.size(); }
    // -------------------------------------------------------------------------
    void push(const float* data, size_t count) {
        if (count == 0) return;
        std::lock_guard<std::recursive_mutex> lock(mMutex);

        if (count > mBuffer.size()) {
            data += (count - mBuffer.size());
            count = mBuffer.size();
        }

        size_t spaceToEnd = mBuffer.size() - mHead;
        size_t firstPart = std::min(count, spaceToEnd);
        size_t secondPart = count - firstPart;

        std::memcpy(&mBuffer[mHead], data, firstPart * sizeof(float));
        if (secondPart > 0) {
            std::memcpy(mBuffer.data(), data + firstPart, secondPart * sizeof(float));
        }

        mHead = (mHead + count) % mBuffer.size();

        if (mFullCount + count > mBuffer.size()) {
            mFullCount = mBuffer.size();
            mTail = mHead;
        } else {
            mFullCount += count;
        }
    }
    // -------------------------------------------------------------------------
    size_t pop(float* out, size_t requestedSize) {
        std::lock_guard<std::recursive_mutex> lock(mMutex);

        size_t actualRead = std::min(requestedSize, mFullCount);
        if (actualRead == 0) return 0;

        size_t dataToEnd = mBuffer.size() - mTail;
        size_t firstPart = std::min(actualRead, dataToEnd);
        size_t secondPart = actualRead - firstPart;

        std::memcpy(out, &mBuffer[mTail], firstPart * sizeof(float));

        if (secondPart > 0) {
            std::memcpy(out + firstPart, mBuffer.data(), secondPart * sizeof(float));
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
