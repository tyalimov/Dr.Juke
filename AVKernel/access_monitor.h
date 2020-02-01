#pragma once

#include "common.h"
#include "util.h"
#include "preferences.h"
#include "ps_monitor.h"
#include "EASTL/hash_map.h"
#include "EASTL/hash_set.h"
#include "EASTL/map.h"
#include "EASTL/set.h"

template <typename TFilterLog, LogMode INFO, LogMode WARN, LogMode ERR>
class AccessMonitor
{
private:
	
	struct unused_parameter 
	{
		template<typename ...Args> 
		unused_parameter(Args const & ... ) {} 
	};

protected:

	wstring mKeyBase;
	const wstring mKeyExcludedProcesses = L"\\ExcludedProcesses";
	const wstring mKeyProtectedObjects = L"\\ProtectedObjects";

	// Protected objects
	hash_map<wstring, ACCESS_MASK> mObjects;

	// Processes which bypass protection
	hash_set<wstring> mProcImages;
	set<PID> mProcPids;

	GuardedMutex mLockProc;
	GuardedMutex mLockObj;
	
public:

	//-------------------------------------------------------------------------------->
	// Constructors and destructors

	// Must be called before filter callback is registered
	AccessMonitor(const wstring& key_path)
		: mKeyBase(key_path) {}

	virtual ~AccessMonitor() = default;

	// Forbid copy/move
	AccessMonitor(const AccessMonitor&) = delete;
	AccessMonitor(AccessMonitor&&) = delete;

	//-------------------------------------------------------------------------------->
	// Protected objects manipulation routines

	bool addObject(const wstring& name, ACCESS_MASK access)
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
			logInfo("Added <Object=%ws>, <Access=0x%08X>",
				name.c_str(), access);
		}
		else
		{
			logInfo("Modified <Object=%ws>, <Access=0x%08X>",
				name.c_str(), access);
		}

		return inserted;
	}

	bool removeObject(const wstring& name)
	{
		mLockObj.acquire();
		auto n = mObjects.erase(name);
		mLockObj.release();

		if (n > 0)
			logInfo("Removed <Object=%ws>", name.c_str());
		else
			logError("Attempt to remove"
				"non existent <Object=%ws>", name.c_str());

		return n > 0;
	}

	//-------------------------------------------------------------------------------->
	// Excluded processes manipulation routines
	
	bool addProcessIfExcluded(PID pid, const wstring& image_path)
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
			logInfo("Added new process <pid=%d, ImagePath=%ws>", 
				pid, image_path.c_str());
		}

		return match;
	}

	bool addProcess(const wstring& image_path, bool update_pids=false)
	{
		mLockProc.acquire();
		auto result = mProcImages.emplace(image_path);
		mLockProc.release();

		bool inserted = result.second;
		if (inserted)
		{
			logInfo("Added new process <UpdatePids=%d, ImagePath=%ws>", 
				update_pids, image_path.c_str());

			if (update_pids)
			{
				// We need to atomically iterate process list
				// See explanation inside updatePidsForExcludedProcesses
				mLockProc.acquire();

				iterateProcessesEqual(image_path, [this](PID pid) {
						mProcPids.emplace(pid);
					});

				mLockProc.release();
			}
		}
		else
		{
			logError("Failed to add new process: already exists "
				"<UpdatePids=%d, ImagePath=%ws>", update_pids, image_path.c_str());
		}

		return inserted;
	}

	bool removeProcessIfExcluded(PID pid)
	{
		mLockProc.acquire();
		auto n = mProcPids.erase(pid);
		mLockProc.release();

		if (n > 0)
			logInfo("Process removed <pid=%d>", pid);

		return n > 0;
	}

	bool removeProcess(const wstring& image_path)
	{
		// We need to atomically iterate process list
		// See explanation inside updatePidsForExcludedProcesses
		
		mLockProc.acquire();
		
		iterateProcessesEqual(image_path, [this](PID pid) {
				mProcPids.erase(pid);
			});

		auto n = mProcImages.erase(image_path);

		mLockProc.release();

		if (n > 0)
		{
			logInfo("Process removed " 
				"<ImagePath=%ws>", image_path.c_str());
		}
		else
		{
			logError("Unable to remove non-existent " 
				"process <ImagePath=%ws>", image_path.c_str());
		}

		return n > 0;
	}

	bool isProcessExcluded(PID pid)
	{
		LockGuard<GuardedMutex> guard(&mLockProc);
		auto res = mProcPids.find(pid);
		return res != mProcPids.end();
	}

	//-------------------------------------------------------------------------------->
	// Access check routines

	virtual bool isAccessAllowed(PID pid, const wstring& name, ACCESS_MASK desired_access) 
	{
		bool res = true;

		if (!isProcessExcluded(pid))
		{
			mLockObj.acquire();
			auto it = mObjects.find(name);
			mLockObj.release();

			if (it != mObjects.end())
			{
				ACCESS_MASK actual_access = it->second;
				if (!(desired_access & actual_access))
					res = false;

				if (res)
				{
					logInfo("Allow <pid=%d> to access %ws",
						pid, name.c_str());
				}
				else
				{
					logInfo("Deny <pid=%d> to access %ws", 
						pid, name.c_str());
				}
			}
		}


		return res;
	}

	//-------------------------------------------------------------------------------->
	// Driver regitry config key change event handler

	virtual void onRegKeyChange(const wstring& KeyPath, 
		PREG_SET_VALUE_KEY_INFORMATION PreInfo, BOOLEAN bDeleted)
	{
		PUNICODE_STRING ValueName = PreInfo->ValueName;
		if (KeyPath == mKeyBase + mKeyProtectedObjects)
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
		else if (KeyPath == mKeyBase + mKeyExcludedProcesses)
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

						this->addProcess(image_path, true);

					});
			}
		}
		else
		{
			NOTHING;
		}
	}

	//-------------------------------------------------------------------------------->
	// Logging routines

	template<typename ...T>
	inline void logInfo(const char* fmt, T... args)
	{
		if constexpr (INFO == LogMode::ON)
			TFilterLog::logInfo(fmt, args...);
		else
			unused_parameter{ fmt, args... };
	}

	template<typename ...T>
	inline void logWarning(const char* fmt, T... args)
	{
		if constexpr(WARN == LogMode::ON)
			TFilterLog::logWarning(fmt, args...);
		else
			unused_parameter{ fmt, args... };
	}

	template<typename ...T>
	inline void logError(const char* fmt, T... args)
	{
		if constexpr(ERR == LogMode::ON)
			TFilterLog::logError(fmt, args...);
		else
			unused_parameter{ fmt, args... };
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
			logError("Do nothing. Bad process format "
				"<ImagePath=%ws>", image_path.c_str());
		}
	}

	void regReadExcludedProcesses()
	{
		NTSTATUS status;
		wstring proc_key = mKeyBase + mKeyExcludedProcesses;

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
			logError("Failed to read excluded processes registry key"
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
			access = 0;
			logWarning("Type != REG_DWORD. "
				"Default <Access=0x%08X>", access);
		}
		
		wstring obj_path(Name, NameLen / sizeof(WCHAR));
		bool ok = false;
		
		ok |= str_util::startsWith(obj_path, L"\\Device");
		ok |= str_util::startsWith(obj_path, L"\\REGISTRY");

		if (ok)
			cb(obj_path, access);
		else
			logError("Do nothing. Bad format <ObjectPath=%ws>", obj_path.c_str());
	}

	void regReadProtectedObjects(function<void(PWCH Name, 
		ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type)> cbAddProtectedObject)
	{
		NTSTATUS status;
		wstring proc_key = mKeyBase + mKeyProtectedObjects;

		status = PreferencesReadFull(proc_key.c_str(), cbAddProtectedObject);
		if (!NT_SUCCESS(status))
		{
			logError("Failed to read protected objects registry key "
				"<KeyPath=%ws>", proc_key.c_str());
		}
	}

	void iterateProcessesEqual(const wstring& image_path, function<void(PID)> cb)
	{
		PID pid;
		ProcessList processes;
		for (const auto& proc : processes)
		{
			pid = proc->ProcessId;
			if (pid == (PID)0)
				continue;

			wstring proc_path = GetProcessImagePathByPid(pid);
			if (proc_path.length() > 0 && proc_path == image_path) {
				cb(pid);
			}
		}
	}

	void iterateProcesses(function<void(PID, const wstring&)> cb)
	{
		ProcessList processes;
		PID pid;

		for (const auto& proc : processes)
		{
			pid = proc->ProcessId;
			if (pid == 0)
				continue;

			wstring path = GetProcessImagePathByPid(pid);
			cb(pid, path);
		}

	}

	void updatePidsForExcludedProcesses()
	{
		//
		// Need to prevent adding processes to mProcPids
		// while iterating process list snaphot
		//
		// This is done to avoid situations when snapshot 
		// of process list is done but new process
		// is terminated during this time
		//
		// mProcPids will be empty at this time
		// but, snapshot still contains terminated process,
		// so its pid will be added to excluded.
		//
		// That pid might be used by another (random) process
		// and cause it to have access to protected objects
		//

		mLockProc.acquire();

		iterateProcesses(
			[this](PID pid, const wstring& path) 
			{
				if (path.length() > 0)
				{
					auto it = mProcImages.find(path);
					if (it != mProcImages.end())
						mProcPids.emplace(pid);
				}
			});

		mLockProc.release();
	}

public:

	virtual void regReadConfiguration()
	{
		// Read names of protected objects
		// and allowed access from registry
		regReadProtectedObjects([this](PWCH Name,
			ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type)
			{
				modifyProtectedObject(Name, NameLen, Data, DataLen, Type,
					[this](const wstring& obj_path, ACCESS_MASK access) {

						this->addObject(obj_path, access);

					});
			});

		// Read paths of excluded processes from registry
		// and save all pids matching those paths
		regReadExcludedProcesses();
		updatePidsForExcludedProcesses();

		if (mObjects.size() == 0)
			logWarning("Configuration: No protected objects found!");

		if (mProcImages.size() == 0)
			logWarning("Configuration: No excluded processes found!");
	}

};

template <typename TFilterLog, LogMode INFO, LogMode WARN, LogMode ERR>
class HierarchyAccessMonitor : public AccessMonitor<TFilterLog, INFO, WARN, ERR>
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

	HierarchyAccessMonitor(const wstring& key_path) : AccessMonitor(key_path) {}

	virtual bool isAccessAllowed(PID pid, const wstring& name, ACCESS_MASK desired_access)
	{
		bool res = true;

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
					logInfo("Allow <pid=%d> to access %ws",
						pid, name.c_str());
				}
				else
				{
					logInfo("Deny <pid=%d> to access %ws", 
						pid, name.c_str());
				}
				
			}
		}

		return res;
	}
};

template <typename TFilterLog, LogMode INFO, LogMode WARN, LogMode ERR>
class ProcessAccessMonitorImpl : public AccessMonitor<TFilterLog, INFO, WARN, ERR>
{
	const wstring mCsrss = L"Windows\\System32\\csrss.exe";

	set<PID> mProcSystem;
	set<PID> mProcInit;
	map<PID, ACCESS_MASK> mObjectPids;

public:

	ProcessAccessMonitorImpl(const wstring& key_path)
		: AccessMonitor(key_path) {}

	//-------------------------------------------------------------------------------->
	// Get running processes pids depending on condition

	void updatePidsForRequiredProcesses()
	{
		mLockObj.acquire();

		iterateProcesses(
			[this](PID pid, const wstring& path) 
			{
				if (path.length() > 0)
				{
					// Protected processes
					auto it = mObjects.find(path);
					if (it != mObjects.end())
						mObjectPids.emplace(pid, it->second);

					// System processes
					if (isSystemProcess(path))
						mProcSystem.emplace(pid);

					// Excluded processes
					auto it_ = mProcImages.find(path);
					if (it_ != mProcImages.end())
						mProcPids.emplace(pid);
				}
			});

		mLockObj.release();
	}

	//-------------------------------------------------------------------------------->
	// Protected objects manipulation routines

	bool addProcessIfProtected(PID pid, const wstring& image_path)
	{
		bool match = false;

		mLockObj.acquire();

		auto it = mObjects.find(image_path);
		if (it != mObjects.end())
		{
			match = true;
			mObjectPids.emplace(pid, it->second);
		}
			
		mLockObj.release();

		if (match)
		{
			// Mark it not initialized 
			// and allow csrss.exe to access protected process
			addProcessNotInitialized(pid, image_path);

			logInfo("Added new process object <pid=%d, ImagePath=%ws>", 
				pid, image_path.c_str());
		}

		return match;
	}

	bool removeProcessIfProtected(PID pid)
	{
		mLockObj.acquire();
		auto n = mObjectPids.erase(pid);
		mLockObj.release();

		if (n > 0)
			logInfo("Process object removed <pid=%d>", pid);

		return n > 0;
	}

 	bool addProcessObject(const wstring& image_path, 
		ACCESS_MASK access, bool update_pids=false)
	{
		mLockObj.acquire();

		auto result = mObjects.try_emplace(image_path, access);
		bool inserted = result.second;
		auto it = result.first;

		if (!inserted)
			it->second = access;
		
		mLockObj.release();

		if (update_pids)
		{
			// We need to atomically iterate process list
			// See explanation inside updatePidsForExcludedProcesses
			mLockObj.acquire();

			iterateProcessesEqual(image_path, [&access, this](PID pid) {
					mObjectPids.emplace(pid, access);
				});

			mLockObj.release();
		}

		if (inserted)
		{
			logInfo("Added process object <UpdatePids=%d, ImagePath=%ws, "
				"access=0x%08X>", update_pids, image_path.c_str(), access);
		}
		else
		{
			logInfo("Modified process object <UpdatePids=%d, ImagePath=%ws, "
				"access=0x%08X>", update_pids, image_path.c_str(), access);
		}

		return inserted;
	}
	
	bool removeProcessObject(const wstring& image_path)
	{
		// We need to atomically iterate process list
		// See explanation inside updatePidsForExcludedProcesses

		mLockObj.acquire();

		iterateProcessesEqual(image_path, [this](PID pid) {
				mObjectPids.erase(pid);
			});

		auto n = mObjects.erase(image_path);
		
		mLockObj.release();

		if (n > 0)
		{
			logInfo("Process object removed " 
				"<ImagePath=%ws>", image_path.c_str());
		}
		else
		{
			logError("Unable to remove non-existent " 
				"process object <ImagePath=%ws>", image_path.c_str());
		}

		return n > 0;
	}

	//-------------------------------------------------------------------------------->
	// System processes manipulation routines

	bool isSystemProcess(const wstring& image_path) {
		return str_util::endsWith(image_path, mCsrss.c_str());
	}

	bool isSystemProcess(PID pid)
	{
		LockGuard<GuardedMutex> guard(&mLockObj);

		auto it = mProcSystem.find(pid);
		return it != mProcSystem.end();
	}

	bool addProcessIfSystem(PID pid, const wstring& image_path)
	{
		mLockObj.acquire();

		bool match = isSystemProcess(image_path);

		if (match)
			mProcSystem.emplace(pid);
			
		mLockObj.release();

		if (match)
		{
			logInfo("Added new system process <pid=%d, ImagePath=%ws>", 
				pid, image_path.c_str());
		}

		return match;
	}

	bool removeProcessIfSystem(PID pid)
	{
		mLockObj.acquire();
		auto n = mProcSystem.erase(pid);
		mLockObj.release();

		if (n > 0)
			logInfo("System process removed <pid=%d>", pid);

		return n > 0;
	}

	//-------------------------------------------------------------------------------->
	// Initialized/non-initialized processes manipulation routines

	bool addProcessNotInitialized(PID pid, const wstring& image_path)
	{
		// all processes here are non-initialized
		mLockObj.acquire();
		auto res = mProcInit.emplace(pid);	
		bool inserted = res.second;
		mLockObj.release();

		if (inserted)
		{
			logInfo("Added new not initialized process "
				"<pid=%d, ImagePath=%ws>", pid, image_path.c_str());
		}
		else
		{
			logError("Failed to add not initialized process. "
				"Already exists <pid=%d, ImagePath=%ws>", pid, image_path.c_str());
		}

		return inserted;
	}

	bool removeProcessInitialized(PID pid)
	{
		// remove processes when they become initialized
		mLockObj.acquire();
		auto n = mProcInit.erase(pid);	
		mLockObj.release();

		if (n > 0)
			logInfo("Process is initialized <pid=%d>", pid);
		else
		{
			logError("Failed to remove non-existent "
				"not initialized process <pid=%d>", pid);
		}

		return n > 0;
	}

	bool isProcessInitialized(PID pid)
	{
		LockGuard<GuardedMutex> guard(&mLockObj);

		auto it = mProcInit.find(pid);
		return it == mProcInit.end();
	}

	//-------------------------------------------------------------------------------->
	// Routines for reading protected objects and excluded processes from registry

	virtual void regReadConfiguration()
	{
		// Read names of protected objects
		// and allowed access from registry
		regReadProtectedObjects([this](PWCH Name,
			ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type)
			{
				modifyProtectedObject(Name, NameLen, Data, DataLen, Type,
					[this](const wstring& obj_path, ACCESS_MASK access) {

						this->addProcessObject(obj_path, access);

					});
			});

		// Read paths of excluded processes from registry
		// Save all pids matching protected/system/excluded processes
		regReadExcludedProcesses();
		updatePidsForRequiredProcesses();

		if (mObjects.size() == 0)
			logWarning("Configuration: No protected objects found!");

		if (mProcImages.size() == 0)
			logWarning("Configuration: No excluded processes found!");

		if (mProcSystem.size() == 0)
			logWarning("Configuration: No system processes found!");
	}

	//-------------------------------------------------------------------------------->
	// Driver regitry config key change event handler

	virtual void onRegKeyChange(const wstring& KeyPath,
		PREG_SET_VALUE_KEY_INFORMATION PreInfo, BOOLEAN bDeleted)
	{
		PUNICODE_STRING ValueName = PreInfo->ValueName;
		if (KeyPath == mKeyBase + mKeyProtectedObjects)
		{
			if (bDeleted)
			{
				modifyProtectedObject(ValueName->Buffer, ValueName->Length,
					PreInfo->Data, PreInfo->DataSize, PreInfo->Type,
					[this](const wstring& obj_path, ACCESS_MASK access) {

						UNREFERENCED_PARAMETER(access);
						this->removeProcessObject(obj_path);

					});
			}
			else
			{
				modifyProtectedObject(ValueName->Buffer, ValueName->Length,
					PreInfo->Data, PreInfo->DataSize, PreInfo->Type,
					[this](const wstring& obj_path, ACCESS_MASK access) {

						this->addProcessObject(obj_path, access, true);

					});
			}
		}
		else if (KeyPath == mKeyBase + mKeyExcludedProcesses)
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

						this->addProcess(image_path, true);

					});
			}
		}
		else
		{
			NOTHING;
		}
	}

	//-------------------------------------------------------------------------------->
	// Access check routines

	bool accessCheck(PID pid_from, PID pid_to) 
	{
		bool res = true;

		if (isProcessInitialized(pid_to))
		{
			if (!isSystemProcess(pid_from))
			{
				if (!isProcessExcluded(pid_from))
				{
					LockGuard<GuardedMutex> guard(&mLockObj);

					auto it = mObjectPids.find(pid_to);
					if (it != mObjectPids.end())
					{
						ACCESS_MASK actual_access = it->second;
						if (actual_access != 0)
						{
							logInfo("Allow <pid=%d> to access <pid=%d>",
								pid_from, pid_to);
						}
						else
						{
							res = false;
							logInfo("Deny <pid=%d> to access <pid=%d>",
								pid_from, pid_to);
						}
					}
				}
			}
		}
		else
		{
			// allow access to not initialized process
			// once csrss.exe process accessed 
			// not initialized process, it will become initialized
			if (isSystemProcess(pid_from))
				removeProcessInitialized(pid_to);
		}

		return res;
	}
};

//template <typename TFilterLog, LogMode INFO, LogMode WARN, LogMode ERR>
//class NoUnloadHierarchyAccessMonitor : public HierarchyAccessMonitor<TFilterLog, INFO, WARN, ERR>
//{
//private:
//
//	bool mNoUnload = false;
//	const wchar_t* mValueNoUnload = L"NoUnload";
//
//	void regReadNonUnload()
//	{
//		NTSTATUS Status;
//		DWORD32 NoUnload = FALSE;
//
//		Status = PreferencesQueryKeyValue(mKeyBase.c_str(), mValueNoUnload,
//			[&NoUnload](PKEY_VALUE_FULL_INFORMATION Info) {
//
//				if (Info->Type == REG_DWORD)
//				{
//					NoUnload = *(DWORD32*)((PCH)Info + Info->DataOffset);
//					return STATUS_SUCCESS;
//				}
//				else
//					return STATUS_INVALID_PARAMETER;
//
//			});
//
//		if (!NT_SUCCESS(Status))
//			logError("Failed to read NoUnload option");
//
//		mNoUnload = (bool)NoUnload;
//		logInfo("Mode <mNoUnload=%d>", NoUnload);
//	}
//
//public:
//
//	NoUnloadHierarchyAccessMonitor(const wstring& key_path) 
//		: HierarchyAccessMonitor(key_path) {}
//
//	bool noUnload() {
//		return mNoUnload;
//	}
//	
//	virtual void regReadConfiguration() override
//	{
//		HierarchyAccessMonitor::regReadConfiguration();
//		regReadNonUnload();
//	}
//
//	virtual void onRegKeyChange(const wstring& KeyPath,
//		PREG_SET_VALUE_KEY_INFORMATION PreInfo, BOOLEAN bDeleted) override
//	{
//		HierarchyAccessMonitor::onRegKeyChange(KeyPath, PreInfo, bDeleted);
//
//		PUNICODE_STRING ValueName = PreInfo->ValueName;
//		if (KeyPath == mKeyBase)
//		{
//			if (bDeleted)
//				mNoUnload = false;
//			else
//			{
//				UNICODE_STRING NoUnload = RTL_CONSTANT_STRING(mValueNoUnload);
//				if (RtlEqualUnicodeString(&NoUnload, ValueName, FALSE))
//					regReadNonUnload();
//			}
//		}
//	}
//
//};


using ProcessAccessMonitor = ProcessAccessMonitorImpl<FilterLog<PsMonitorPrefix>, LOG_MODE_PSPROTECT>;
using RegistryAccessMonitor = HierarchyAccessMonitor<FilterLog<RegFilterPrefix>, LOG_MODE_REGFILTER>;
using FileSystemAccessMonitor = HierarchyAccessMonitor<FilterLog<FsFilterPrefix>, LOG_MODE_FSFILTER>;

using PFileSystemAccessMonitor = FileSystemAccessMonitor*;
using PRegistryAccessMonitor = RegistryAccessMonitor*;
using PProcessAccessMonitor = ProcessAccessMonitor*;