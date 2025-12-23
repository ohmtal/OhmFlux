//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Example usage:
// =============
//
// class Player {
//     FluxScheduler::TaskID mRespawnId = 0;
//
//     void die() {
//         // Schedule respawn in 3 seconds
//         mRespawnId = FluxSchedule.add(3.0, [this]() {
//             this->respawn(100); // Calls method with arguments
//         });
//     }
//
//     void respawn(int health) {
//         mRespawnId = 0;
//     }
//
//     void onDisconnect() {
//         // Cancel respawn if they leave the game
//         if (FluxSchedule.isPending(mRespawnId)) {
//             FluxSchedule.cancel(mRespawnId);
//         }
//     }
// };
// //-----------------------------------------------------------------------------

#pragma once
#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>
//-----------------------------------------------------------------------------
// Global shorthand macro
#define FluxSchedule FluxScheduler::get()
//-----------------------------------------------------------------------------
class FluxScheduler {
public:
    using TaskID = size_t;

    //-------------------------------------------------------------------------
    // Meyers' Singleton: Thread-safe initialization
    static FluxScheduler& get() {
        static FluxScheduler instance;
        return instance;
    }
    //-------------------------------------------------------------------------
    // Adds a task and returns a unique ID
    TaskID add(double delaySeconds, std::function<void()> action) {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        if (mIsShutdown) return 0; // Don't add tasks during shutdown
        TaskID newId = ++mNextId;
        mTasks.push_back({newId, delaySeconds, std::move(action)});
        return newId;
    }

    //-------------------------------------------------------------------------
    // Check if a task is still waiting
    bool isPending(TaskID id) {
        if (id == 0) return false;
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        return std::any_of(mTasks.begin(), mTasks.end(),
                           [id](const ScheduledTask& t) { return t.id == id; });
    }

    //-------------------------------------------------------------------------
    // Cancel a specific task
    void cancel(TaskID id) {
        if (id == 0) return;
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        mTasks.erase(std::remove_if(mTasks.begin(), mTasks.end(),
                                    [id](const ScheduledTask& t) { return t.id == id; }),
                     mTasks.end());
    }

    //-------------------------------------------------------------------------
    // Wipe all tasks (e.g., during level loading)
    void clear() {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        mTasks.clear();
    }

    //-------------------------------------------------------------------------
    void update(double dt)
    {
        if (mIsShutdown)
            return;

        std::vector<std::function<void()>> actionsToRun;
        {
            std::lock_guard<std::recursive_mutex> lock(mMutex);
            for (auto it = mTasks.begin(); it != mTasks.end(); ) {
                // Convert millisecond dt to seconds for comparison with timeRemaining
                it->timeRemaining -= (dt / 1000.0);

                if (it->timeRemaining <= 0.0) {
                    if (it->action) {
                        actionsToRun.push_back(std::move(it->action));
                    }
                    it = mTasks.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // Execute actions after the lock is released
        for (auto& action : actionsToRun) {
            action();
        }
    } //update

    //-------------------------------------------------------------------------
    void shutdown() {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        mTasks.clear(); // Clear all std::function objects
        mIsShutdown = true;
        Log("FluxScheduler: Shutdown complete.");
    } //shutdown
    //-------------------------------------------------------------------------


private:
    FluxScheduler() : mNextId(0) {}
    ~FluxScheduler() = default;

    // Delete copy/move to enforce Singleton
    FluxScheduler(const FluxScheduler&) = delete;
    void operator=(const FluxScheduler&) = delete;

    struct ScheduledTask {
        TaskID id;
        double timeRemaining;
        std::function<void()> action;
    };

    std::vector<ScheduledTask> mTasks;
    TaskID mNextId;

    // IMPORTANT: Recursive mutex allows a task to call .add()
    // or .cancel() without deadlocking the thread.
    std::recursive_mutex mMutex;

    bool mIsShutdown = false;
};
