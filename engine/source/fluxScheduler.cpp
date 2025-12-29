//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>
#include "errorlog.h"

#include "fluxScheduler.h"
#include "fluxBaseObject.h"


//-------------------------------------------------------------------------
FluxScheduler& FluxScheduler::get()
{
    static FluxScheduler instance;
    return instance;
}
//-------------------------------------------------------------------------
FluxScheduler::TaskID FluxScheduler::add(double lDelaySeconds, FluxBaseObject* lOwner, std::function<void()> lAction)
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if (mIsShutdown) return 0; // Don't add tasks during shutdown
    TaskID lNewId = ++mNextId;
    mTasks.push_back({lNewId, lDelaySeconds, std::move(lAction), lOwner});

    if (lOwner != nullptr )
    {
        lOwner->setScheduleUsed(true);
    }

    return lNewId;
}
//-------------------------------------------------------------------------

bool FluxScheduler::isPending(TaskID id){
    if (id == 0) return false;
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    return std::any_of(mTasks.begin(), mTasks.end(),
                        [id](const ScheduledTask& t) { return t.id == id; });
}
//-------------------------------------------------------------------------
void FluxScheduler::cancel(TaskID id){
    if (id == 0) return;
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mTasks.erase(std::remove_if(mTasks.begin(), mTasks.end(),
                                [id](const ScheduledTask& t) { return t.id == id; }),
                mTasks.end());
}
//-------------------------------------------------------------------------
void FluxScheduler::clear(){
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mTasks.clear();
}
//-------------------------------------------------------------------------

void FluxScheduler::update(double dt)
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
}
//-------------------------------------------------------------------------
void FluxScheduler::shutdown(){
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mTasks.clear(); // Clear all std::function objects
    mIsShutdown = true;
    Log("FluxScheduler: Shutdown complete.");
}
//-------------------------------------------------------------------------
void FluxScheduler::cleanByOwner(void* lOwner)
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if (mIsShutdown)
        return;

    if (lOwner == nullptr) return; // Safety: Never clear tasks with nullptr lOwner accidentally

    auto it = std::remove_if(mTasks.begin(), mTasks.end(),
                            [lOwner](const ScheduledTask& task) {
                                return task.owner == lOwner;
                            });
    mTasks.erase(it, mTasks.end());
}
//-------------------------------------------------------------------------
