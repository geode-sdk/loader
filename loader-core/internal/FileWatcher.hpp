#pragma once

#include <functional>
#include <string>
#include <helpers/fs/filesystem.hpp>
#include <interface/Macros.hpp>
#include <interface/Types.hpp>

template<class Impl>
class FileWatcherBase {
public:
	using FileWatchCallback = std::function<void(ghc::filesystem::path)>;
	using ErrorCallback = std::function<void(std::string)>;

protected:
	ghc::filesystem::path m_file;
	FileWatchCallback m_callback;
	ErrorCallback m_error;
	bool m_filemode = false;

public:
	bool watching() const {
		return Impl::watching();
	}

	ghc::filesystem::path path() {
		return m_file;
	}
};

#ifdef LILAC_IS_WINDOWS
#include "FileWatcherWin.hpp"
#endif
