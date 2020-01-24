#pragma once

#include "common.h"
#include "util.h"
#include "EASTL/hash_map.h"
#include "EASTL/hash_set.h"
#include "EASTL/set.h"
#include "EASTL/list.h"

using PID = HANDLE;

class AccessMonitor
{
	// Protected objects
	hash_map<wstring, ACCESS_MASK> mObjects;

	// Processes which bypass protection
	hash_map<wstring, list<PID>> mProcImages;
	set<PID> mProcPids;

	GuardedMutex mLockProc;
	GuardedMutex mLockObj;

	void addObject(const wstring& name, ACCESS_MASK access)
	{
		LockGuard<GuardedMutex> guard(&mLockObj);

		auto _pair = mObjects.try_emplace(name, access);
		auto it = _pair.first;
		bool bInserted = _pair.second;

		if (bInserted)
		{
			kprintf(TRACE_INFO, "Added <Object=%ws>, <Access=0x%08X>",
				name.c_str(), access);
		}
		else
		{
			it->second = access;
			kprintf(TRACE_INFO, "Modified <Object=%ws>, <Access=0x%08X>",
				name.c_str(), access);
		}
	}

	void removeObject(const wstring& name)
	{
		LockGuard<GuardedMutex> guard(&mLockObj);

		auto n = mObjects.erase(name);
		if (n > 0)
			kprintf(TRACE_INFO, "Removed <Object=%ws>", name.c_str());
		else
			kprintf(TRACE_INFO, "Attempt to remove"
				"non existent <Object=%ws>", name.c_str());
	}

	void addProcess(const wstring& image_path, PID pid)
	{
		LockGuard<GuardedMutex> guard(&mLockProc);

		mProcPids.emplace(pid);

		auto it = mProcImages.find(image_path);
		if (it != mProcImages.end())
		{
			list<PID> pids = it->second;
			pids.push_back(pid);
		}
		else
		{
			list<PID> pids = { pid };
			mProcImages.emplace(image_path, move(pids));
		}

		kprintf(TRACE_INFO, "Added process <PID=%d, ImagePath=%ws>",
			pid, image_path.c_str());
	}

	void addProcess(const wstring& image_path, list<PID> pids)
	{
		LockGuard<GuardedMutex> guard(&mLockProc);

		for (const auto& pid : pids)
			mProcPids.emplace(pids);

		mProcImages.emplace(image_path, pids);

		kprintf(TRACE_INFO, "Added process "
			"<ImagePath=%ws> with its PIDs", image_path.c_str());
	}

	void removeProcess(const wstring& image_path, PID pid)
	{
		LockGuard<GuardedMutex> guard(&mLockProc);

		mProcPids.erase(pid);

		auto it = mProcImages.find(image_path);
		if (it != mProcImages.end())
		{
			list<PID> pids = it->second;
			pids.remove(pid);
		}
		else
		{
			kprintf(TRACE_INFO, "Unable to remove non-existent " 
				"process <ImagePath=%ws>", image_path.c_str());
		}
	}

	void removeProcess(const wstring& image_path)
	{
		LockGuard<GuardedMutex> guard(&mLockProc);

		auto it = mProcImages.find(image_path);
		if (it != mProcImages.end())
		{
			list<PID> pids = it->second;
			for (const auto& pid : pids)
				mProcPids.erase(pid);

			mProcImages.erase(it);
		}
		else
		{
			kprintf(TRACE_INFO, "Unable to remove non-existent " 
				"process <ImagePath=%ws>", image_path.c_str());
		}
	}

	virtual bool accessCheck(PID pid, const wstring& name, ACCESS_MASK access) 
	{
		kprintf(TRACE_INFO, "Allow <pid=%d> to access %ws", 
			pid, name.c_str());
	}
};
