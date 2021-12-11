/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "js_process.h"

#include <ctime>
#include <vector>

#include <grp.h>
#include <pwd.h>
#include <uv.h>
#include <sched.h>
#include <pthread.h>

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cstdlib>

#include "securec.h"
#include "utils/log.h"
namespace OHOS::Js_sys_module::Process {
    namespace {
        constexpr int NUM_OF_DATA = 4;
        constexpr int PER_USER_RANGE = 100000;
    }
    std::map<std::string, napi_value> Process::m_map_process_event_;
    Process::Process(napi_env env_) : env(env_) {}
    napi_value Process::GetUid() const
    {
        napi_value result = nullptr;
        auto processGetuid = static_cast<uint32_t>(getuid());
        NAPI_CALL(env, napi_create_uint32(env, processGetuid, &result));
        return result;
    }

    napi_value Process::GetGid() const
    {
        napi_value result = nullptr;
        auto processGetgid = static_cast<uint32_t>(getgid());
        NAPI_CALL(env, napi_create_uint32(env, processGetgid, &result));
        return result;
    }

    napi_value Process::GetEUid() const
    {
        napi_value result = nullptr;
        auto processGeteuid = static_cast<uint32_t>(geteuid());
        NAPI_CALL(env, napi_create_uint32(env, processGeteuid, &result));
        return result;
    }

    napi_value Process::GetEGid() const
    {
        napi_value result = nullptr;
        auto processGetegid = static_cast<uint32_t>(getegid());
        NAPI_CALL(env, napi_create_uint32(env, processGetegid, &result));
        return result;
    }

    napi_value Process::GetGroups() const
    {
        napi_value result = nullptr;
        int progroups = getgroups(0, nullptr);
        if (progroups == -1) {
            napi_throw_error(env, "-1", "getgroups initialize failed");
        }
        std::vector<gid_t> pgrous(progroups);
        progroups = getgroups(progroups, pgrous.data());
        if (progroups == -1) {
            napi_throw_error(env, "-1", "getgroups");
        }
        pgrous.resize(progroups);
        gid_t proegid = getegid();
        if (std::find(pgrous.begin(), pgrous.end(), proegid) == pgrous.end()) {
            pgrous.push_back(proegid);
        }
        std::vector<uint32_t> arry;
        for (auto iter = pgrous.begin(); iter != pgrous.end(); iter++) {
            auto recive = static_cast<uint32_t>(*iter);
            arry.push_back(recive);
        }
        NAPI_CALL(env, napi_create_array(env, &result));
        size_t len = arry.size();
        for (size_t i = 0; i < len; i++) {
            napi_value numvalue = nullptr;
            NAPI_CALL(env, napi_create_uint32(env, arry[i], &numvalue));
            NAPI_CALL(env, napi_set_element(env, result, i, numvalue));
        }
        return result;
    }

    napi_value Process::GetPid() const
    {
        napi_value result = nullptr;
        auto proPid = static_cast<int32_t>(getpid());
        napi_create_int32(env, proPid, &result);
        return result;
    }

    napi_value Process::GetPpid() const
    {
        napi_value result = nullptr;
        auto proPpid = static_cast<int32_t>(getppid());
        napi_create_int32(env, proPpid, &result);
        return result;
    }

    void Process::Chdir(napi_value args) const
    {
        size_t prolen = 0;
        napi_get_value_string_utf8(env, args, nullptr, 0, &prolen);
        char *path = nullptr;
        if (prolen > 0) {
            path = new char[prolen + 1];
            if (memset_s(path, prolen + 1, '\0', prolen + 1) != 0) {
                napi_throw_error(env, "-1", "chdir path memset_s failed");
            }
        } else {
            napi_throw_error(env, "-2", "prolen is error !");
        }
        napi_get_value_string_utf8(env, args, path, prolen + 1, &prolen);
        int proerr = 0;
        if (path != nullptr) {
            proerr = uv_chdir(path);
            delete []path;
            path = nullptr;
        }
        if (proerr) {
            napi_throw_error(env, "-1", "chdir");
        }
    }

    napi_value Process::Kill(napi_value proid, napi_value signal)
    {
        int32_t pid = 0;
        int32_t sig = 0;
        napi_get_value_int32(env, proid, &pid);
        napi_get_value_int32(env, signal, &sig);
        uv_pid_t ownPid = uv_os_getpid();
        // 64:The maximum valid signal value is 64.
        if (sig > 64 && (pid == 0 || pid == -1 || pid == ownPid || pid == -ownPid)) {
            napi_throw_error(env, "0", "process exit");
        }
        bool flag = false;
        int err = uv_kill(pid, sig);
        if (!err) {
            flag = true;
        }
        napi_value result = nullptr;
        NAPI_CALL(env, napi_get_boolean(env, flag, &result));
        return result;
    }

    napi_value Process::Uptime() const
    {
        napi_value result = nullptr;
        struct sysinfo information = {0};
        time_t systimer = 0;
        double runsystime = 0.0;
        if (sysinfo(&information)) {
            napi_throw_error(env, "-1", "Failed to get sysinfo");
        }
        systimer = information.uptime;
        if (systimer > 0) {
            runsystime = static_cast<double>(systimer);
            NAPI_CALL(env, napi_create_double(env, runsystime, &result));
        } else {
            napi_throw_error(env, "-1", "Failed to get systimer");
        }
        return result;
    }

    void Process::Exit(napi_value number) const
    {
        int32_t result = 0;
        napi_get_value_int32(env, number, &result);
        exit(result);
    }

    napi_value Process::Cwd() const
    {
        napi_value result = nullptr;
        char buf[260 * NUM_OF_DATA] = { 0 }; // 260:Only numbers path String size is 260.
        size_t length = sizeof(buf);
        int err = uv_cwd(buf, &length);
        if (err) {
            napi_throw_error(env, "1", "uv_cwd");
        }
        napi_create_string_utf8(env, buf, length, &result);
        return result;
    }

    void Process::Abort() const
    {
        abort();
    }

    void Process::On(napi_value str, napi_value function)
    {
        char *buffer = nullptr;
        size_t bufferSize = 0;
        napi_get_value_string_utf8(env, str, buffer, 0, &bufferSize);
        if (bufferSize > 0) {
            buffer = new char[bufferSize + 1];
        }
        napi_get_value_string_utf8(env, str, buffer, bufferSize + 1, &bufferSize);
        if (buffer != nullptr) {
            m_map_process_event_[buffer] = function;
            delete []buffer;
            buffer = nullptr;
        }
    }

    napi_value Process::Off(napi_value str)
    {
        char *buffer = nullptr;
        size_t bufferSize = 0;
        bool flag = true;
        napi_value result = nullptr;
        napi_get_value_string_utf8(env, str, buffer, 0, &bufferSize);
        if (bufferSize > 0) {
            buffer = new char[bufferSize + 1];
        }
        napi_get_value_string_utf8(env, str, buffer, bufferSize + 1, &bufferSize);
        std::string temp = "";
        if (buffer != nullptr) {
            temp = buffer;
            delete []buffer;
            buffer = nullptr;
        }
        for (auto iter = m_map_process_event_.cbegin(); iter != m_map_process_event_.cend(); ++iter) {
            if (iter->first == temp) {
                m_map_process_event_.erase(temp);
                NAPI_CALL(env, napi_get_boolean(env, flag, &result));
                return result;
            }
        }
        flag = false;
        NAPI_CALL(env, napi_get_boolean(env, flag, &result));
        return result;
    }

    napi_value Process::GetTid() const
    {
        napi_value result = nullptr;
        auto proTid = static_cast<int32_t>(gettid());
        napi_create_int32(env, proTid, &result);
        return result;
    }

    napi_value Process::IsIsolatedProcess() const
    {
        napi_value result = nullptr;
        bool flag = true;
        auto prouid = static_cast<int32_t>(getuid());
        auto uid = prouid % PER_USER_RANGE;
        if ((uid >= 99000 && uid <= 99999) || // 99999:Only isolateuid numbers between 99000 and 99999.
            (uid >= 9000 && uid <= 98999)) { // 98999:Only appuid numbers betweeen 9000 and 98999.
            NAPI_CALL(env, napi_get_boolean(env, flag, &result));
            return result;
        }
        flag = false;
        NAPI_CALL(env, napi_get_boolean(env, flag, &result));
        return result;
    }

    napi_value Process::IsAppUid(napi_value uid) const
    {
        int32_t number = 0;
        napi_value result = nullptr;
        bool flag = true;
        napi_get_value_int32(env, uid, &number);
        if (number > 0) {
            const auto appId = number % PER_USER_RANGE;
            if (appId >= FIRST_APPLICATION_UID && appId <= LAST_APPLICATION_UID) {
                napi_get_boolean(env, flag, &result);
                return result;
            }
            flag = false;
            NAPI_CALL(env, napi_get_boolean(env, flag, &result));
            return result;
        } else {
            flag = false;
            NAPI_CALL(env, napi_get_boolean(env, flag, &result));
            return result;
        }
    }

    napi_value Process::Is64Bit() const
    {
        napi_value result = nullptr;
        bool flag = true;
        auto size = sizeof(char*);
        if (size == NUM_OF_DATA) {
            flag = false;
            NAPI_CALL(env, napi_get_boolean(env, flag, &result));
            return result;
        } else {
            NAPI_CALL(env, napi_get_boolean(env, flag, &result));
            return result;
        }
    }

    napi_value Process::GetEnvironmentVar(napi_value name) const
    {
        char *buffer = nullptr;
        char *env_var = nullptr;
        napi_value result = nullptr;
        size_t bufferSize = 0;
        napi_get_value_string_utf8(env, name, buffer, 0, &bufferSize);
        if (bufferSize > 0) {
            buffer = new char[bufferSize + 1];
        }
        napi_get_value_string_utf8(env, name, buffer, bufferSize + 1, &bufferSize);
        std::string temp = "";
        if (buffer != nullptr) {
            temp = buffer;
            delete []buffer;
            buffer = nullptr;
        }
        env_var = getenv(temp.c_str());
        if (env_var == nullptr) {
            NAPI_CALL(env, napi_get_undefined(env, &result));
            return result;
        }
        napi_create_string_utf8(env, env_var, strlen(env_var), &result);
        return result;
    }

    napi_value Process::GetUidForName(napi_value name) const
    {
        struct passwd *user = nullptr;
        int32_t uid = 0;
        napi_value result = nullptr;
        char *buffer = nullptr;
        size_t bufferSize = 0;
        napi_get_value_string_utf8(env, name, buffer, 0, &bufferSize);
        if (bufferSize > 0) {
            buffer = new char[bufferSize + 1];
        }
        napi_get_value_string_utf8(env, name, buffer, bufferSize + 1, &bufferSize);
        std::string temp = "";
        if (buffer != nullptr) {
            temp = buffer;
            delete []buffer;
            buffer = nullptr;
        }
        user = getpwnam(temp.c_str());
        if (user != nullptr) {
            uid = static_cast<int32_t>(user->pw_uid);
            napi_create_int32(env, uid, &result);
            return result;
        }
        napi_create_int32(env, (-1), &result);
        return result;
    }

    napi_value Process::GetThreadPriority(napi_value tid) const
    {
        errno = 0;
        int32_t proTid = 0;
        napi_value result = nullptr;
        napi_get_value_int32(env, tid, &proTid);
        int32_t pri = getpriority(PRIO_PROCESS, proTid);
        if (errno != 0) {
            napi_throw_error(env, "-1", "Invalid tid");
        }
        napi_create_int32(env, pri, &result);
        return result;
    }

    napi_value Process::GetStartRealtime() const
    {
        struct timespec timespro;
        struct timespec timessys;
        napi_value result = nullptr;
        auto res = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timespro);
        if (res != 0) {
            return 0;
        }
        auto res1 = clock_gettime(CLOCK_MONOTONIC, &timessys);
        if (res1 != 0) {
            return 0;
        }
        double whenpro = ConvertTime(timespro.tv_sec, timespro.tv_nsec);
        double whensys = ConvertTime(timessys.tv_sec, timessys.tv_nsec);
        auto timedif = (whensys - whenpro);
        napi_create_double(env, timedif, &result);
        return result;
    }

    napi_value Process::GetAvailableCores() const
    {
        napi_value result = nullptr;
        auto num_cpus = static_cast<int32_t>(sysconf(_SC_NPROCESSORS_ONLN));
        NAPI_CALL(env, napi_create_array(env, &result));
        for (int i = 0; i <= num_cpus; i++) {
            napi_value numvalue = nullptr;
            napi_create_uint32(env, i, &numvalue);
            napi_status status = napi_set_element(env, result, i, numvalue);
            if (status != napi_ok) {
                HILOG_INFO("set element error");
            }
        }
        return result;
    }

    double Process::ConvertTime(time_t tvsec, long tvnsec) const
    {
        return double(tvsec * 1000) + double(tvnsec / 1000000); // 98999:Only converttime numbers is 1000 and 1000000.
    }

    napi_value Process::GetPastCputime() const
    {
        struct timespec times;
        napi_value result = nullptr;
        auto res = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &times);
        if (res != 0) {
            return 0;
        }
        double when =  ConvertTime(times.tv_sec, times.tv_nsec);
        napi_create_double(env, when, &result);
        return result;
    }

    napi_value Process::GetSystemConfig(napi_value name)
    {
        int32_t number = 0;
        napi_value result = nullptr;
        napi_get_value_int32(env, name, &number);
        auto configinfo = static_cast<int32_t>(sysconf(number));
        napi_create_int32(env, configinfo, &result);
        return result;
    }
}