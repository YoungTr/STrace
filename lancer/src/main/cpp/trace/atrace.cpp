//
// Created by YoungTr on 2022/9/26.
//

#include <trace.h>
#include <string>
#include <dlfcn.h>
#include "atrace.h"
#include "timers.h"
#include "hook_bridge.h"
#include "trace_provider.h"
#include "../log.h"

namespace swan {
namespace lancer {

ATrace &ATrace::Get() {
    static ATrace kInstance;
    return kInstance;
}

int32_t ATrace::StarTrace() {
    int64_t start = elapsedRealtimeMicros();

    if (atrace_started_) {
        return OK;
    }

    int32_t result = InstallProbe();
    if (result != OK) {
        LOGD("failed to install atrace, error: %d", result);
        return result;
    }

    auto prev = atrace_enable_tags_->exchange(DEFAULT_ATRACE_TAG);
    if (prev != UINT64_MAX) {
        original_tags_ = prev;
    }

    atrace_started_ = true;
//    ATRACE_BEGIN(("monotonic_time: " + std::to_string(systemTime(SYSTEM_TIME_MONOTONIC) / 1000000000.0)).c_str());
    int64_t cost_us = elapsedRealtimeMicros() - start;
    LOGD("start trace cost us: %ld", cost_us);
    return OK;
}

int32_t ATrace::StopTrace() {
    int64_t start = elapsedRealtimeMicros();
    if (!atrace_started_) {
        LOGE("please start trace firstly");
        return OK;
    }
    uint64_t tags = original_tags_;
    if (tags != UINT64_MAX) {
        atrace_enable_tags_->store(tags);
    }

    log_trace_cost_us_ = 0;
    int64_t cost_us = elapsedRealtimeMicros() - start;
    LOGD("stop trace cost us: %ld", cost_us);

    return OK;
}

bool ATrace::IsATrace(int fd, size_t count) {
   return (atrace_maker_fd_ != nullptr && fd == *atrace_maker_fd_ && count > 0);
}

void ATrace::LogTrace(const void *buf, size_t count) {
    const char *msg = (const char *) buf;
    switch (msg[0]) {
        case 'B':
            LOGD("%s", msg);
            break;
        case 'E':
            LOGD("E");
            break;
        default:
            break;
    }
}


// private functions
int32_t ATrace::InstallProbe() {
    if (atrace_probe_installed_) {
        return OK;
    }
    if (!HookBridge::Get().HookLoadedLibs()) {
        return HOOK_FAILED;
    }

    int32_t r = InstallAtraceProbe();
    if (r != OK) {
        return r;
    }
    
    atrace_probe_installed_ = true;
    return OK;

}

int32_t ATrace::InstallAtraceProbe() {

    void *handle;
    if (nullptr == (handle = dlopen(nullptr, RTLD_GLOBAL))) return INSTALL_ATRACE_FAILED;
    if (nullptr == (atrace_enable_tags_ = reinterpret_cast<std::atomic<uint64_t> *>(dlsym(handle,"atrace_enabled_tags")))) goto err;
    if (nullptr == (atrace_maker_fd_ =reinterpret_cast<int *>( dlsym(handle, "atrace_marker_fd")))) goto err;

    return OK;

   err:
    dlclose(handle);
    return INSTALL_ATRACE_FAILED;
}


ATrace::ATrace() = default;
ATrace::~ATrace() = default;


}
}


