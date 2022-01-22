#pragma once

#ifdef GEODE_IS_WINDOWS

#include <Windows.h>
#include <thread>

class FileWatcherWin : public FileWatcherBase<FileWatcherWin> {
protected:
	HANDLE m_handle = nullptr;
	std::thread m_watch;
	bool m_exiting = false;

	void watch();
	
public:
	FileWatcherWin(ghc::filesystem::path const& file, FileWatchCallback callback, ErrorCallback error = nullptr);
	~FileWatcherWin();
	
	bool watching() const;
};

using FileWatcher = FileWatcherWin;

#endif
