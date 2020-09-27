#include <sstream>
#include <cstdio>
#include "client_listener.h"
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

using namespace std;
using namespace asr_stream_sdk;
class MyClientListener: public asr_stream_sdk::ClientListener
{
public:
    MyClientListener() {};
    virtual ~MyClientListener() {};
    void onTaskStarted()
    {
        onLog(LogLevel_notice, "onTaskStarted called. start task.");
    }
    void onTextReceived(uint32_t idx, std::list<std::string>& nbest, std::list<std::string>& uncertain, bool is_final)
    {
        std::ostringstream oss;
        std::string nbest0;
        std::string uncertain0;
        if(nbest.begin() != nbest.end())
        {
            nbest0 = *(nbest.begin());
        }
        if(uncertain.begin() != uncertain.end())
        {
            uncertain0 = *(uncertain.begin());
        }
        oss << "onTextReceived called. asr text info. "
                                                        << "idx["            << idx
                                                        << "], nbest0["      << nbest0
                                                        << "], uncertain0["  << uncertain0
                                                        << "], is_final["    << is_final
                                                        << "]";
        onLog(LogLevel_notice, oss.str());
    }
    void onTaskCompleted()
    {
        onLog(LogLevel_notice, "onTaskCompleted called. task completed.");
    }
    void onTaskFailed(int32_t error_code, std::string& info, std::string& trace_id)
    {
        std::ostringstream oss;
        oss << "onTaskFailed called. task failed. "
                                                    << "error_code:[" << error_code
                                                    << "], info["     << info
                                                    << "], trace_id[" << trace_id
                                                    << "]";
        onLog(LogLevel_notice, oss.str());
    }
    void onLog(LogLevel log_level, const std::string& log)
    {
        if((int)log_level >= (int)LogLevel_debug)
        {
            fprintf(stdout, "thread_id[%ld] -- log level[%d] log info[%s]\n", gettid(), log_level, log.c_str());
        }
    }
};


