#pragma once
#include <utils/platform.hpp>

#ifdef GEODE_IS_IOS

class FileWatcheriOS : public FileWatcherBase<FileWatcheriOS> {
protected:
	void* dispatch_source; // objc moment
	bool m_exiting = false;

	void watch();
	
public:
	FileWatcheriOS(ghc::filesystem::path const& file, FileWatchCallback callback, ErrorCallback error = nullptr);
	~FileWatcheriOS();
	
	bool watching() const;
};

using FileWatcher = FileWatcheriOS;

#endif
