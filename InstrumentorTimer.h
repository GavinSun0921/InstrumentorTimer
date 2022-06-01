//
// Basic instrumentation profiler by Cherno
// ------------------------
// Modified by Gavin
// 1. Make 'WriteProfile' thread-safe by add 'std::mutex' and 'std::lock_guard'
// 2. Remove the 'InstrumentationSession' which is no significance
// 3. Add destructor for 'Instrumentor'
// 4. Change the Constructor of 'Instrumentor' to private
// ------------------------

// Usage: include this header file somewhere in your code (e.g. precompiled header), and then use like:
//
// Instrumentor::Get().BeginSession("Session Name");        // Begin session
// {
//     InstrumentationTimer timer("Profiled Scope Name");   // Place code like this in scopes you'd like to include in profiling
//     // Code
// }
// Instrumentor::Get().EndSession();                        // End Session
//
// You will probably want to macro-fy this, to switch on/off easily and use things like __FUNCSIG__ for the profile name.
//
#pragma once

#include <string>
#include <chrono>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>

struct ProfileResult {
    std::string Name;
    long long Start, End;
    uint64_t ThreadID;
};

class Instrumentor {
private:
    std::string m_SessionName;
    std::ofstream m_OutputStream;
    int m_ProfileCount;
    std::mutex m_Lock;
    bool m_ActiveSession;

public:

    ~Instrumentor() {
        EndSession();
    }

    Instrumentor(const Instrumentor &) = delete;

    static void BeginSession(const std::string &name, const std::string &filepath = "Results.json") {
        GetInstance().BeginSessionImpl(name, filepath);
    }

    static void EndSession() {
        GetInstance().EndSessionImpl();
    }

    static void WriteProfile(const ProfileResult &result) {
        GetInstance().WriteProfileImlp(result);
    }

private:

    Instrumentor()
            : m_SessionName("None"), m_ProfileCount(0), m_ActiveSession(false) {
        ;
    }

    static Instrumentor &GetInstance() {
        static Instrumentor instance;
        return instance;
    }

    void BeginSessionImpl(const std::string &name, const std::string &filepath = "Results.json") {
        if (m_ActiveSession) {
            EndSession();
        }
        m_ActiveSession = true;
        m_SessionName = name;

        m_OutputStream.open(m_SessionName + std::string("_") + filepath);
        WriteHeader();
    }

    void EndSessionImpl() {
        if (!m_ActiveSession) {
            return;
        }
        m_ActiveSession = false;
        m_ProfileCount = 0;

        WriteFooter();
        m_OutputStream.close();
    }

    void WriteHeader() {
        m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
        m_OutputStream.flush();
    }

    void WriteFooter() {
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }

    void WriteProfileImlp(const ProfileResult &result) {
        std::lock_guard<std::mutex> lockGuard(m_Lock);

        if (m_ProfileCount++ > 0)
            m_OutputStream << ",";

        std::string name = result.Name;
        std::replace(name.begin(), name.end(), '"', '\'');

        m_OutputStream << "{";
        m_OutputStream << "\"cat\":\"function\",";
        m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
        m_OutputStream << "\"name\":\"" << name << "\",";
        m_OutputStream << "\"ph\":\"X\",";
        m_OutputStream << "\"pid\":\"" << m_SessionName << "\",";
        m_OutputStream << "\"tid\":" << result.ThreadID << ",";
        m_OutputStream << "\"ts\":" << result.Start;
        m_OutputStream << "}";

        m_OutputStream.flush();
    }
};

class InstrumentationTimer {
private:
    const char *m_Name;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
    bool m_Stopped;

public:
    explicit InstrumentationTimer(const char *name)
            : m_Name(name), m_Stopped(false) {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~InstrumentationTimer() {
        if (!m_Stopped) {
            Stop();
        }
    }

    void Stop() {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(
                m_StartTimepoint).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(
                endTimepoint).time_since_epoch().count();

        uint64_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
        Instrumentor::WriteProfile({m_Name, start, end, threadID});

        m_Stopped = true;
    }
};