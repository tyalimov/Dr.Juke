#pragma once

#include "common.h"
#include "util.h"
#include "ps_monitor.h"
#include "EASTL/hash_map.h"
#include "EASTL/hash_set.h"
#include "preferences.h"
#include "EASTL/set.h"
#include "EASTL/list.h"

template<ULONG tr_info=TRACE_INFO, ULONG tr_err=TRACE_ERROR>
class AccessMonitor
{
protected:

	wstring mBaseKey;
	const wstring mcExcludedProcesses = L"\\ExcludedProcesses";
	const wstring mcProtectedObjects = L"\\ProtectedObjects";

	// Protected objects
	hash_map<wstring, ACCESS_MASK> mObjects;

	// Processes which bypass protection
	hash_set<wstring> mProcImages;
	set<PID> mProcPids;

	GuardedMutex mLockProc;
	GuardedMutex mLockObj;
	GuardedMutex mLockStatus;
	
	bool mEnabled = false;

public:

	//-------------------------------------------------------------------------------->
	// Constructors and destructors

	// Must be called before filter callback is registered
	AccessMonitor(const wstring& key_path)
	{
		mBaseKey = key_path;
		mEnabled = true;

		// read names of protected objects
		// and allowed access from registry
		regReadProtectedObjects();

		// read paths of excluded processes from registry
		// and save all pids matching those paths
		regReadExcludedProcesses();
		updatePidsForExcludedProcesses();

		if (mObjects.size() == 0)
			kprintf(tr_err, "No protected objects found!");

		if (mProcImages.size() == 0)
			kprintf(tr_err, "No excluded processes found!");
	}

	virtual ~AccessMonitor() = default;

	// Forbid copy/move
	AccessMonitor(const AccessMonitor&) = delete;
	AccessMonitor(AccessMonitor&&) = delete;

	//-------------------------------------------------------------------------------->
	// Driver <enable/disable/get state> routines

	void setEnabled(bool status)
	{
		mLockStatus.acquire();
		mEnabled = status;
		mLockStatus.release();

		kprintf(tr_info, "Filter mode changed. mEnabled=%d", mEnabled);
	}

	bool isEnabled() 
	{
		LockGuard<GuardedMutex> guard(&mLockStatus);
		return mEnabled;
	}

	//-------------------------------------------------------------------------------->
	// Protected objects manipulation routines

	void addObject(const wstring& name, ACCESS_MASK access)
	{
		mLockObj.acquire();

		auto result = mObjects.try_emplace(name, access);
		bool inserted = result.second;
		auto it = result.first;

		if (!inserted)
			it->second = access;

		mLockObj.release();

		if (inserted)
		{
			kprintf(tr_info, "Added <Object=%ws>, <Access=0x%08X>",
				name.c_str(), access);
		}
		else
		{
			kprintf(tr_info, "Modified <Object=%ws>, <Access=0x%08X>",
				name.c_str(), access);
		}
	}

	void removeObject(const wstring& name)
	{
		mLockObj.acquire();
		auto n = mObjects.erase(name);
		mLockObj.release();

		if (n > 0)
			kprintf(tr_info, "Removed <Object=%ws>", name.c_str());
		else
			kprintf(tr_err, "Attempt to remove"
				"non existent <Object=%ws>", name.c_str());
	}

	//-------------------------------------------------------------------------------->
	// Excluded processes manipulation routines
	
	void addProcessIfExcluded(PID pid, const wstring& image_path)
	{
		bool match = false;

		mLockProc.acquire();

		auto it = mProcImages.find(image_path);
		if (it != mProcImages.end())
		{
			match = true;
			mProcPids.emplace(pid);
		}
			
		mLockProc.release();

		if (match)
		{
			kprintf(tr_info, "Added new process "
				"<pid=%d, ImagePath=%ws>", pid, image_path.c_str());
		}
	}

	void addProcess(const wstring& image_path, bool update_pids=false)
	{
		mLockProc.acquire();
		auto result = mProcImages.emplace(image_path);
		mLockProc.release();

		bool inserted = result.second;
		if (inserted)
		{
			kprintf(tr_info, "Added new process "
				"<UpdatePids=%d, ImagePath=%ws>", update_pids, image_path.c_str());

			if (update_pids)
			{
				iterateProcessInstances(image_path, [this](PID pid) {
						mLockProc.acquire();
						mProcPids.emplace(pid);
						mLockProc.release();
					});
			}
		}
		else
		{
			kprintf(tr_info, "Failed to add new process: already exists "
				"<UpdatePids=%d, ImagePath=%ws>", update_pids, image_path.c_str());
		}
	}

	void removeProcessIfExcluded(PID pid)
	{
		mLockProc.acquire();
		mProcPids.erase(pid);
		mLockProc.release();
	}

	void removeProcess(const wstring& image_path)
	{
		iterateProcessInstances(image_path, [this](PID pid) {
				mLockProc.acquire();
				mProcPids.erase(pid);
				mLockProc.release();
			});

		mLockProc.acquire();
		auto n = mProcImages.erase(image_path);
		mLockProc.release();

		if (n > 0)
		{
			kprintf(tr_info, "Process removed " 
				"<ImagePath=%ws>", image_path.c_str());
		}
		else
		{
			kprintf(tr_err, "Unable to remove non-existent " 
				"process <ImagePath=%ws>", image_path.c_str());
		}
	}

	bool isProcessExcluded(PID pid)
	{
		LockGuard<GuardedMutex> guard(&mLockProc);
		auto res = mProcPids.find(pid);
		return res != mProcPids.end();
	}

	//-------------------------------------------------------------------------------->
	// Driver access check routines

	virtual bool isAccessAllowed(PID pid, const wstring& name, ACCESS_MASK desired_access) 
	{
		bool res = true;

		if (isEnabled())
		{
			if (!isProcessExcluded(pid))
			{
				LockGuard<GuardedMutex> guard(&mLockObj);

				auto it = mObjects.find(name);
				if (it != mObjects.end())
				{
					ACCESS_MASK actual_access = it->second;
					if (!(desired_access & actual_access))
						res = false;
				}
			}
		}

		if (res)
		{
			kprintf(tr_info, "Allow <pid=%d> to access %ws",
				pid, name.c_str());
		}
		else
		{
			kprintf(tr_info, "Deny <pid=%d> to access %ws", 
				pid, name.c_str());
		}

		return res;
	}

	//-------------------------------------------------------------------------------->
	// Driver regitry config key change event handler

	void onRegKeyChange(const wstring& KeyPath, 
		PREG_SET_VALUE_KEY_INFORMATION PreInfo, BOOLEAN bDeleted)
	{
		PUNICODE_STRING ValueName = PreInfo->ValueName;
		if (KeyPath == mBaseKey + mcProtectedObjects)
		{
			if (bDeleted)
			{
				modifyProtectedObject(ValueName->Buffer, ValueName->Length, 
					PreInfo->Data, PreInfo->DataSize, PreInfo->Type,
					[this](const wstring& obj_path, ACCESS_MASK access) {

						UNREFERENCED_PARAMETER(access);
						this->removeObject(obj_path);

					});
			}
			else
			{
				modifyProtectedObject(ValueName->Buffer, ValueName->Length, 
					PreInfo->Data, PreInfo->DataSize, PreInfo->Type,
					[this](const wstring& obj_path, ACCESS_MASK access) {

						this->addObject(obj_path, access);

					});
			}
		}
		else if (KeyPath == mBaseKey + mcExcludedProcesses)
		{
			if (bDeleted)
			{
				modifyExcludedProcess(ValueName->Buffer, ValueName->Length, 
					[this](const wstring& image_path) {

						this->removeProcess(image_path);

					});
			}
			else
			{
				modifyExcludedProcess(ValueName->Buffer, ValueName->Length, 
					[this](const wstring& image_path) {

						this->addProcess(image_path);

					});
			}
		}
		else
		{
			NOTHING;
		}
	}

protected:

	//-------------------------------------------------------------------------------->
	// Routines for reading protected objects and excluded processes from registry

	void modifyExcludedProcess(PWCH name, ULONG name_len, function<void(wstring)> cb)
	{
		wstring image_path(name, name_len / sizeof(WCHAR));
		if (str_util::startsWith(image_path, L"\\Device\\")) 
		{
			cb(image_path);
		}
		else
		{
			kprintf(tr_err, "Bad process format "
				"<ImagePath=%ws>", image_path.c_str());
		}
	}

	void regReadExcludedProcesses()
	{
		NTSTATUS status;
		wstring proc_key = mBaseKey + mcExcludedProcesses;

		const auto cbAddExcludedProcess = [this](PWCH name, ULONG name_len)
		{
			modifyExcludedProcess(name, name_len,
				[this](const wstring& image_path) {

					this->addProcess(image_path);

				});
		};

		status = PreferencesReadBasic(proc_key.c_str(), cbAddExcludedProcess);
		if (!NT_SUCCESS(status))
		{
			kprintf(tr_err, "Failed to read excluded processes registry key"
				"<Status=0x%08X, KeyPath=%ws>", status, proc_key.c_str());
		}
	}

	void modifyProtectedObject(PWCH Name, ULONG NameLen,
		PVOID Data, ULONG DataLen, ULONG Type, function<void(wstring, ACCESS_MASK)> cb)
	{
		UNREFERENCED_PARAMETER(DataLen);

		ACCESS_MASK access;
		if (Type == REG_DWORD)
			access = *(ACCESS_MASK*)Data;
		else
		{
			kprintf(tr_err, "Type != REG_DWORD. Default set to KEY_ALL_ACCESS");
			access = KEY_ALL_ACCESS;
		}
		
		wstring obj_path(Name, NameLen / sizeof(WCHAR));
		cb(obj_path, access);
	}

	void regReadProtectedObjects()
	{
		NTSTATUS status;
		wstring proc_key = mBaseKey + mcProtectedObjects;

		const auto cbAddProtectedObject = [this](PWCH Name, 
			ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type)
		{
			modifyProtectedObject(Name, NameLen, Data, DataLen, Type,
				[this](const wstring& obj_path, ACCESS_MASK access) {

					this->addObject(obj_path, access);

				});
		};

		status = PreferencesReadFull(proc_key.c_str(), cbAddProtectedObject);
		if (!NT_SUCCESS(status))
		{
			kprintf(tr_err, "Failed to read protected objects registry key"
				"<KeyPath=%ws>", proc_key.c_str());
		}
	}

	void iterateProcessInstances(const wstring& image_path, function<void(PID)> cb)
	{
		PID pid;
		ProcessList processes;

		for (const auto& proc : processes)
		{
			pid = proc->ProcessId;
			wstring proc_path = GetProcessImagePathByPid(pid);
			if (proc_path.length() > 0 && proc_path == image_path) {
				cb(pid);
			}
		}
	}

	void updatePidsForExcludedProcesses()
	{
		PID pid;
		ProcessList processes;

		for (const auto& proc : processes)
		{
			pid = proc->ProcessId;
			if (pid == 0)
				continue;

			wstring proc_path = GetProcessImagePathByPid(pid);
			if (proc_path.length() > 0)
			{
				mLockProc.acquire();

				auto it = mProcImages.find(proc_path);
				if (it != mProcImages.end())
					mProcPids.emplace(pid);

				mLockProc.release();
			}
		}
	}

};

template <ULONG tr_info, ULONG tr_err>
class __RegistryAccessMonitor : public AccessMonitor<tr_info, tr_err>
{
private:

	auto findRule(wstring key_path)
	{
		LockGuard<GuardedMutex> guard(&mLockObj);

		auto pos = key_path.rfind(L'\\');
		auto it = mObjects.find(key_path);

		while (pos != wstring::npos)
		{
			if (it != mObjects.end())
				return it;

			key_path = key_path.substr(0, pos);
			pos = key_path.rfind(L'\\');
			it = mObjects.find(key_path);
		}

		return mObjects.end();
	}

public:

	__RegistryAccessMonitor(const wstring& key_path) : AccessMonitor(key_path) {}

	virtual bool isAccessAllowed(PID pid, const wstring& name, ACCESS_MASK desired_access)
	{
		bool res = true;

		if (isEnabled())
		{
			if (!isProcessExcluded(pid))
			{
				auto it = findRule(name);
				if (it != mObjects.end())
				{
					ACCESS_MASK actual_access = it->second;
					if (!(desired_access & actual_access))
						res = false;

					if (res)
					{
						kprintf(tr_info, "Allow <pid=%d> to access %ws",
							pid, name.c_str());
					}
					else
					{
						kprintf(tr_info, "Deny <pid=%d> to access %ws", 
							pid, name.c_str());
					}
				}
			}
		}

		return res;
	}
};

using RegistryAccessMonitor = __RegistryAccessMonitor<TRACE_REGFILTER_INFO, TRACE_REGFILTER_ERROR>;
using PRegistryAccessMonitor = RegistryAccessMonitor*;

