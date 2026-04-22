//-----------------------------------------------------------------------------
// Copyright (c) 2025 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>
#include "utils/errorlog.h"

#include "utils/fluxScheduler.h"
#include "core/fluxBaseObject.h"


//-------------------------------------------------------------------------
FluxScheduler& FluxScheduler::get()
{
    static FluxScheduler instance;
    return instance;
}
//-------------------------------------------------------------------------
FluxScheduler::TaskID FluxScheduler::add(double lDelaySeconds, FluxBaseObject* lOwner, std::function<void()> lAction, bool ticker )
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if (mIsShutdown) return 0; // Don't add tasks during shutdown
    TaskID lNewId = ++mNextId;
    mTasks.push_back({lNewId, lDelaySeconds, std::move(lAction), lOwner, ticker, lDelaySeconds});


    if (lOwner != nullptr )
    {
        lOwner->setScheduleUsed(true);
    }

    return lNewId;
}

FluxScheduler::TaskID FluxScheduler::addTicker(double lDelaySeconds, FluxBaseObject* lOwner, std::function<void()> lAction){
    return add(lDelaySeconds, lOwner, lAction, true);
}
//-------------------------------------------------------------------------
bool FluxScheduler::extend(TaskID id, double lDelaySeconds) {
    if ( id == 0 ) return false;
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    for (auto it = mTasks.begin(); it != mTasks.end(); it++ ) {
        if (it->id == id) {
                it->timeRemaining = lDelaySeconds;
                return true;
        }
    }
    return false;
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
void FluxScheduler::listPending() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    Log("%-5s %-10s %-14s %-8s %-10s", "ID", "TimeRem", "Owner", "Ticker", "Interval");

    for (const auto& task : mTasks) {
        Log("%-5d %-10.2f %-14p %-8s %-10.2f",
            (int)task.id,
            task.timeRemaining,
            task.owner,
            task.isTicker ? "Yes" : "No",
            task.timeInterval);
    }

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
                    actionsToRun.push_back(it->isTicker ? it->action : std::move(it->action));
                }
                if (it->isTicker) {
                    it->timeRemaining += it->timeInterval;
                    if (it->timeRemaining <= 0.0) {
                        it->timeRemaining = it->timeInterval;
                    }

                    ++it;
                } else {
                    it = mTasks.erase(it);
                }
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
