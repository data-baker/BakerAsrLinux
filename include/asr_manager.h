#ifndef __SYNTHESIZER_MANAGER_H__
#define __SYNTHESIZER_MANAGER_H__
#include "pub_headers.h"
#include "websocket_wrapper.h"

namespace asr_stream_sdk {

class SpeechManager
{
public:
    SpeechManager():_recv_last_segment(false),
                    _inited(false),
                    _open_conn(false),
                    _push_frame_thread(NULL),
                    _push_thread_running(true) {};
    virtual ~SpeechManager()
    {
        destroyPushFrameThread();
        _ws_frame_manager.reset();
    };

    /********************************************************************************************
    **	@func 初始化，每个SpeechManager实例必须init一次
    **
    **	@param client_id        从标贝申请的clientid
    **         secret           从标贝申请的secret
    **         server_url       流式合成url, eg wss://asr.data-baker.com/wss
    **         audio_format     音频格式, eg pcm, wav
    **         sample_rate      音频采样率, eg 16000, 8000
    **         recog_timeout    识别处理超时时间, 单位ms
    **         client_listener  回调指针
    **
    **	@return 成功返回0，失败返回-1
    ********************************************************************************************/
    int init(std::string& client_id,
                  std::string& secret,
                  std::string& server_url,
                  std::string& audio_format,
                  uint32_t sample_rate,
                  uint32_t recog_timeout,
                  boost::shared_ptr<ClientListener> client_listener);
   /********************************************************************************************
    **	@func 初始化，每个SpeechManager实例必须init一次
    **
    **	@param client_id        从标贝申请的clientid
    **         secret           从标贝申请的secret
    **         server_url       流式合成url, eg wss://asr.data-baker.com/wss
    **         audio_format     音频格式, eg pcm, wav
    **         sample_rate      音频采样率, eg 16000, 8000
    **         client_listener  回调指针
    **
    **	@return 成功返回0，失败返回-1
    ********************************************************************************************/
    int init(std::string& client_id,
                  std::string& secret,
                  std::string& server_url,
                  std::string& audio_format,
                  uint32_t sample_rate,
                  boost::shared_ptr<ClientListener> client_listener) {
        init(client_id, secret, server_url, audio_format, sample_rate, 120000, client_listener);
    }
    /********************************************************************************************
    **  @func  是否已经与服务建立连接jj
    **
    **
    **  @return true/false
    ********************************************************************************************/
    bool has_open_conn() {
        return _open_conn;
    }

    /********************************************************************************************
    **  @func 处理音频片段，每160ms的音频数据作为一个片段
    **
    **  @param speech_data 二进制音频数据
    **         last_frame  是否是最后一个音频片段
    **
    **  @return 成功返回0，任务已经结束返回1，失败返回-1
    ********************************************************************************************/
    int procSpeechData(std::string& speech_data, bool last_frame = false);

    /********************************************************************************************
    **	@func 结束当前的识别任务
    **
    **	@param 无
    **
    **	@return 成功返回0，失败返回-1
    ********************************************************************************************/
    int stopTask();

    /********************************************************************************************
    **	@func 主动获取识别结果，识别结果也会通过回调通知
    **
    **	@param nbest      nbest结果
    **         uncertain  预测结果
    **
    **	@return 无
    ********************************************************************************************/
    void getTaskResult(std::list<std::string>& nbest, std::list<std::string>& uncertain);

    /********************************************************************************************
    **	@func 主动获取任务状态，任务状态也会通过回调通知
    **
    **	@param 无
    **
    **	@return TaskStatus_inprocessing(任务进行中)
    **          TaskStatus_complete(任务完成)
    **          TaskStatus_error(任务出错)
    ********************************************************************************************/
    TaskStatus getTaskStatus();
private:
    void pushFrameHandler();
    void createPushFrameThread();
    void destroyPushFrameThread();

public:
    std::string      _audio_format;
    uint32_t         _sample_rate;
    int32_t          _req_idx;
    bool             _recv_last_segment;
    bool             _add_pct;
    bool             _enable_itn;
    std::string      _domain;
    boost::shared_ptr<WSFrameManager>  _ws_frame_manager;
    std::string      _client_id;
    std::string      _secret;
    std::string      _server_url;
    boost::shared_ptr<ClientListener>  _client_listener;
    boost::mutex     _api_mutex;
    volatile bool    _inited;
    volatile bool    _open_conn;

    boost::thread*   _push_frame_thread;
    volatile bool    _push_thread_running;
};
}
#endif
