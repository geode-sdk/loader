#include "MacOS.hpp"
#include "Core.hpp"

#include <mach/mach.h>
#include <mach/task.h>
#include <mach/mach_port.h>
#include <mach/mach_vm.h>       /* mach_vm_*            */
#include <mach/mach_init.h>     /* mach_task_self()     */

#include <signal.h>             /* sigaction            */
#include <sys/ucontext.h>       /* ucontext_t           */

using namespace geode::core::hook;

namespace {
	thread_local void* original = nullptr;

    void sigtrap(int signal, siginfo_t* signal_info, void* vcontext) {
        ucontext_t* context = reinterpret_cast<ucontext_t*>(vcontext);

        const void* current = reinterpret_cast<const void*>(context->uc_mcontext->__ss.__rip);

        impl::handleContext(vcontext, original, current);
    }

    void sigill(int signal, siginfo_t* signal_info, void* vcontext) {
        ucontext_t* context = reinterpret_cast<ucontext_t*>(vcontext);

        original = reinterpret_cast<void*>(context->uc_mcontext->__ss.__rip);

        impl::handleContext(vcontext, original, original);
    }
}

std::vector<std::byte> jump(const void* from, const void* to) {
	constexpr size_t size = sizeof(int) + 1;
	std::vector<std::byte> ret(size);
	ret[0] = {0xe9};

	int offset = (int)(to - from - size_of_jump);
	// im too lazy
	((int*)((size_t)ret.data() + 1))[0] = offset;

	return ret;
}


bool MacOSX::writeMemory(const void* to, const void* from, const size_t size) {

	kern_return_t status; //return status

	vm_size_t vmsize;
    vm_address_t address = (vm_address_t)to;
    vm_region_basic_info_data_t info;
    mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT;
    memory_object_name_t object;

    // get memory protection
    status = mach_vm_region(mach_task_self(), &address, &vmsize, VM_REGION_BASIC_INFO, (vm_region_info_t)&info, &info_count, &object);
    if (status != KERN_SUCCESS) return false;

    // set to write protection
    status = mach_vm_protect(mach_task_self(), (mach_vm_address_t)to, size, FALSE, VM_PROT_WRITE | VM_PROT_READ);
    if (status != KERN_SUCCESS) return false;

    // write to memory
    status = mach_vm_write(mach_task_self(), (mach_vm_address_t)to, (vm_offset_t)from, (mach_msg_type_number_t)size);
    if (status != KERN_SUCCESS) return false;

    // revert to old protection
    status = mach_vm_protect(mach_task_self(), (mach_vm_address_t)to, size, FALSE, info.protection);
    if (status != KERN_SUCCESS) return false;

    return status == KERN_SUCCESS;
}

bool MacOSX::initialize() {
	task_set_exception_ports(
		mach_task_self(),
		EXC_MASK_BAD_INSTRUCTION,
		MACH_PORT_NULL,//m_exception_port,
		EXCEPTION_DEFAULT,
		0); 
	// first reached here
    sigaction illaction = {};
    illaction.sa_sigaction = &sigill;
    illaction.sa_flags = SA_SIGINFO;

    // afterwards reached here
    sigaction trapaction = {};
    trapaction.sa_sigaction = &sigtrap;
    trapaction.sa_flags = SA_SIGINFO;

    return sigaction(SIGILL, &illaction, NULL) == 0 && sigaction(SIGTRAP, &trapaction, NULL) == 0;
}
