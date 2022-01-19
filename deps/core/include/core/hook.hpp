#ifndef LILAC_CORE_HOOK_HPP
#define LILAC_CORE_HOOK_HPP

#include <meta/preproc.hpp>

namespace lilac::core::hook {
	/* opaque struct representing a handle to a hook.
	*/
	using Handle = void*;

	/*
	* params:
	* address - a pointer to the virtual memory to be hooked.
	* detour - a pointer to the function to redirect the instruction pointer to.
	*
	* returns:
	* null if failed.
	* a valid, constant pointer to a hook handle if succeeded.
	*
	* notes:
	* the hooked function should have the same calling convention and parameters
	* as the detour. otherwise, crashing is almost certain to occur.
	*/
	Handle LILAC_CALL add(const void* address, const void* detour);

	/*
	* params:
	* handle - a pointer to the hook handle to be removed.
	*
	* returns:
	* true if the hook was successfully removed.
	* false if removal failed.
	*/
	bool LILAC_CALL remove(Handle handle);

	/**
	 * @param address Address to write to
	 * @param data Data to write
	 * @param size Size of data
	 * @returns True if succesfully written, 
	 * false if not
	 */
	bool LILAC_CALL write_memory(void* address, void* data, size_t size);

	/**
	 * @param address Address to read from
	 * @param data Where to write data
	 * @param size How much data to read
	 * @returns True if succesfully read, 
	 * false if not
	 */
	bool LILAC_CALL read_memory(void* address, void* receive, size_t size);
}

#endif /* LILAC_CORE_HOOK_HPP */
