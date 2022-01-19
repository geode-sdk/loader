#pragma once

#include "FileWatcher.hpp"

#ifdef LILAC_IS_MACOS

class FileWatcherMac : public FileWatcherBase<FileWatcherMac> {
protected:
	void* dispatch_source; // objc moment
	bool m_exiting = false;

	void watch();
	
public:
	FileWatcherMac(ghc::filesystem::path const& file, FileWatchCallback callback, ErrorCallback error = nullptr);
	~FileWatcherMac();
	
	bool watching() const;
};

using FileWatcher = FileWatcherMac;

#endif
