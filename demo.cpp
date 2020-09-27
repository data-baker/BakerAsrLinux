#include <cstdlib>
#include <sstream>
#include "asr_manager.h"
#include "demo_client_listener.h"
#include <boost/random.hpp>
#include <boost/thread.hpp>

using namespace std;
using namespace asr_stream_sdk;

string g_clientid           = "";               //从标贝获取的clientid和secret
string g_secret             = "";
string g_server_url_16k     = "ws://192.168.1.19:9002";  //16k识别服务的url
string g_server_url_8k      = "ws://192.168.1.19:9002"; //8k识别服务的url
char* param_file_path = NULL;

uint32_t getRandUInt(uint32_t start, uint32_t end)  //获取指定范围随机数
{
    struct timeval time;
    gettimeofday(&time, NULL);
    boost::mt19937 rng(time.tv_usec);
	boost::uniform_int<> ui(start, end);

	return ui(rng);
}

static void sendRequestFrame(boost::shared_ptr<SpeechManager> speech_manager)
{
    ostringstream oss;
    char * audio_buf = (char *)malloc(1024 * 1024);
    char *speech_audio_buf = (char*)malloc(1024 * 1024);
    string audio_file_name = param_file_path;  //从文件中读取音频流来模拟实时流
    FILE * fs = fopen(audio_file_name.c_str(), "rb");
    if (fs == NULL) {
        oss << "Error: fopen file: " <<  audio_file_name << " failed. error: " << strerror(errno);
        speech_manager->_client_listener->onLog(LogLevel_error, oss.str());
        return;
    }

    fseek(fs, 0, SEEK_END);
    int filesize = ftell(fs);
    fseek(fs, 0, SEEK_SET);
    int size = 0;
    int total_size = 0;
    bool last = false;  //last用于确认是不是最后一个音频包
    int package_len = 5120;
    if(speech_manager->_sample_rate == 8000)
    {
        package_len = 2560;
    }
    while ((size = fread(audio_buf, 1, package_len, fs)) != 0) {  //5120 = 16000 * 2 * 0.16ms，取160ms的音频
            total_size += size;
            if (total_size >= filesize) {
                last = true;  //必须有最后一包音频，最后一包audio_data可以为空
            }

            string audio_data(audio_buf, size);
            speech_manager->procSpeechData(audio_data, last);
            if(!last)
            {
                struct timeval wait_time;
                wait_time.tv_sec = 0;
                wait_time.tv_usec = 160000;
                select(0, NULL, NULL, NULL, &wait_time);  //等待160ms，用于模拟实时流
            }
    }
/***************************************************************************************************************/
//这部分代码仅用于演示通过getTaskStatus和getTaskResult方法实现阻塞的目的，其实识别结果和任务状态也会通过回调返回
    while(true)
    {
        ostringstream oss;
        TaskStatus task_status = speech_manager->getTaskStatus();
        if(task_status == TaskStatus_inprocess)
        {
            struct timeval wait_time;
            wait_time.tv_sec = 0;
            wait_time.tv_usec = 5000;
            select(0, NULL, NULL, NULL, &wait_time);
        }else if(task_status == TaskStatus_complete)
        {
            std::list<std::string> nbest;
            std::list<std::string> uncertain;
            speech_manager->getTaskResult(nbest, uncertain);
            string nbest0, uncertain0;
            if(!nbest.empty())
            {
                nbest0 = *(nbest.begin());
            }
            if(!uncertain.empty())
            {
                uncertain0 = *(uncertain.begin());
            }
            oss << "final log: asr result. " << "nbest0:["          << nbest0
                                             << "], uncertain0:["   << uncertain0
                                             << "]";
            speech_manager->_client_listener->onLog(LogLevel_notice, oss.str());
            break;
        }else
        {
            oss << "final log: task error";
            speech_manager->_client_listener->onLog(LogLevel_error, oss.str());
            break;
        }
    }
/***************************************************************************************************************/
    fclose(fs);
    free(audio_buf);
    free(speech_audio_buf);
}

int main(int argc, char* argv[])
{
    if(!(argc == 3 || argc == 4 || argc == 5))
    {
        printf("Usage: ./xxx sample_rate audio_file [ws_server_url] [domain]\n");
        return 0;
    }
    int param_sample = atoi(argv[1]);
    if(!(param_sample == 8000 || param_sample == 16000))
    {
        printf("sample rate is 8000 or 16000\n");
        return 0;
    }
    param_file_path = argv[2];   
    boost::shared_ptr<ClientListener> client_listener(new(std::nothrow) MyClientListener());  //实例化MyClientListener
    boost::shared_ptr<SpeechManager> speech_manager(new(std::nothrow) SpeechManager());       //实例化SpeechManager
    string audio_format = "wav";
    uint32_t sample_rate = 16000;
    string server_url = g_server_url_16k;
    if(param_sample == 8000)
    {
        sample_rate = 8000;
        server_url = g_server_url_8k;
    }
    string domain = "common";  //默认使用common模型，如需使用其它模型，请在启动参数中指定
    if(argc >= 4)
    {
        server_url = string(argv[3]);
        if(argc == 5)
        {
            domain = string(argv[4]);
        }
    }
    speech_manager->init(g_clientid, g_secret, server_url, audio_format, sample_rate, false, false, domain, client_listener);  //初始化
    sendRequestFrame(speech_manager);                                                                    //规律发送音频包

}
