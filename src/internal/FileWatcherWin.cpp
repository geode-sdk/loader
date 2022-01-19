#include "FileWatcherWin.hpp"
#include <iostream>

#ifdef GEODE_IS_WINDOWS

static constexpr const auto notifyAttributes = FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE;

FileWatcherWin::FileWatcherWin(ghc::filesystem::path const& file, FileWatchCallback callback, ErrorCallback error) {
	this->m_filemode = ghc::filesystem::is_regular_file(file);
	this->m_handle =  FindFirstChangeNotificationW(
		(this->m_filemode ? file.parent_path() : file).wstring().c_str(),
		false,
		notifyAttributes
	);
	this->m_file = file;
	this->m_callback = callback;
	this->m_error = error;
	if (this->m_handle != INVALID_HANDLE_VALUE) {
		this->m_watch = std::thread(&FileWatcherWin::watch, this);
	} else {
		if (this->m_error) this->m_error("Invalid handle");
	}
}

FileWatcherWin::~FileWatcherWin() {
	FindCloseChangeNotification(this->m_handle);
	this->m_exiting = true;
	this->m_watch.detach();
}

void FileWatcherWin::watch() {
	while (WaitForSingleObject(this->m_handle, 10000) == WAIT_OBJECT_0) {
		if (this->m_exiting) return;
		if (this->m_callback) {
			if (this->m_filemode) {
				auto file = CreateFileW(
					this->m_file.parent_path().wstring().c_str(),
					FILE_LIST_DIRECTORY,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					nullptr,
					OPEN_EXISTING,
					FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
					nullptr
				);
				if (file == INVALID_HANDLE_VALUE) {
					this->m_handle = nullptr;
					if (this->m_error) this->m_error("Reading dir failed");
					return;
				}
				OVERLAPPED overlapped;
				overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
				std::vector<DWORD> buffer;
				buffer.resize(1024);
				if (!ReadDirectoryChangesW(
					file, buffer.data(), buffer.size(), false,
					notifyAttributes,
					nullptr, &overlapped, nullptr
				)) {
					this->m_handle = nullptr;
					if (this->m_error) this->m_error("Reading dir changes failed");
					return;
				}
				DWORD result = WaitForSingleObject(overlapped.hEvent, 500);
				if (result != WAIT_OBJECT_0 && result != WAIT_TIMEOUT) {
					this->m_handle = nullptr;
					if (this->m_error) this->m_error("Overlap hEvent was not WAIT_OBJECT_0");
					return;
				}
				DWORD bytes_transferred;
				GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);
				auto info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer.data());
				do {
					auto filename = std::wstring(info->FileName, info->FileName + info->FileNameLength / sizeof(wchar_t));
					if (
						ghc::filesystem::exists(this->m_file) &&
						ghc::filesystem::file_size(this->m_file) > 1000 &&
						info->Action == FILE_ACTION_MODIFIED && 
						this->m_file.filename().wstring() == filename
					) {
						this->m_callback(this->m_file);
					}
				} while (info->NextEntryOffset && (info = info + info->NextEntryOffset));
			} else {
				this->m_callback(this->m_file);
			}
		}
		if (!FindNextChangeNotification(this->m_handle)) {
			this->m_handle = nullptr;
			if (this->m_error) this->m_error("FindNextChangeNotification failed");
			return;
		}
	}
	this->m_handle = nullptr;
	if (this->m_error) this->m_error("WaitForSingleObject failed");
}

bool FileWatcherWin::watching() const {
	return this->m_handle != INVALID_HANDLE_VALUE &&
		   this->m_handle != nullptr;
}

#endif
