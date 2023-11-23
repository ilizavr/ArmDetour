#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace ilizavr {

class ArmHook {
public:
    static uintptr_t HookFunction(uintptr_t target, uintptr_t newFunction) {
        auto oldFunction = CreateTrampoline(target);
        if (!oldFunction) {
            throw std::runtime_error("Failed to create trampoline for the function.");
	}
        if (ChangeProtection(target) != 0) {
            throw std::runtime_error("Failed to change memory protection.");
	}
        WriteJump(target, newFunction);
        return reinterpret_cast<uintptr_t>(oldFunction.get());
    }

private:
    static std::unique_ptr<uint8_t[]> CreateTrampoline(uintptr_t target) {
        const size_t trampolineSize = 16;
        auto trampoline = std::make_unique<uint8_t[]>(trampolineSize);
        if (trampoline == nullptr) {
            throw std::bad_alloc();
        }

        std::memcpy(trampoline.get(), reinterpret_cast<void*>(target), 8);

        // write jump from trampoline to remaining part of original function
        trampoline[8] = 0x04;
        trampoline[9] = 0xF0;
        trampoline[10] = 0x1F;
        trampoline[11] = 0xE5;
        *reinterpret_cast<uintptr_t*>(&trampoline[12]) = target + 8;
        return trampoline;
    }

    static int ChangeProtection(uintptr_t target) {
        uintptr_t start = _PageStartOf(target);
        size_t length = _PageLength(target, 8);
        return mprotect(reinterpret_cast<void*>(start), length, PROT_READ | PROT_WRITE | PROT_EXEC);
    }

    static void WriteJump(uintptr_t target, uintptr_t destination) {
        // writing jump instruction to new function
        auto* targetPtr = reinterpret_cast<uint8_t*>(target);
        targetPtr[0] = 0x04;
        targetPtr[1] = 0xF0;
        targetPtr[2] = 0x1F;
        targetPtr[3] = 0xE5;
        *reinterpret_cast<uintptr_t*>(&targetPtr[4]) = destination;
    }

    static uintptr_t _PageStartOf(uintptr_t address) {
        return address & ~(static_cast<uintptr_t>(sysconf(_SC_PAGE_SIZE)) - 1);
    }

    static size_t _PageLength(uintptr_t address, size_t length) {
        uintptr_t start = _PageStartOf(address);
        uintptr_t end = _PageStartOf(address + length - 1);
        return end - start + sysconf(_SC_PAGE_SIZE);
    }
};

}
