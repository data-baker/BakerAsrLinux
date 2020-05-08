#ifndef __WEBSOCKET_WRAPPER_H__
#define __WEBSOCKET_WRAPPER_H__
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include "pub_headers.h"
#include "client_listener.h"

#define gettid() syscall(SYS_gettid)

#define WSS_URL 1

//void onLog(std::string log);

namespace asr_stream_sdk {
#if WSS_URL
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::config::asio_tls_client::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
#else
typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
#endif
typedef client::connection_ptr connection_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

enum SdkCallbackErrorCode
{
    Success_default                     = 80000,         //初始错误码
    Success_audio_data                  = 90000,         //音频数据
    Failed_connection_error             = 90001,         //连接流式服务器失败
    Failed_parse_json_error             = 90002,         //解析服务端返回的信息失败
    Failed_json_incomplete              = 90003,         //服务端返回的信息不全
    Failed_task_timeout                 = 90004,         //任务超时
    Failed_other_error                  = 90005,         //其它错误
    Failed_get_token_error              = 90006,         //获取token失败
    Failed_not_asr_clientid             = 90007,         //clientid不支持asr功能
    Failed_invalid_clientid_or_secret   = 90008          //clientid或secret错误
};

enum TaskStatus
{
    TaskStatus_inprocess = 0,  //任务进行中
    TaskStatus_complete  = 1,  //任务完成
    TaskStatus_error     = 2   //任务失败
};



class WSFrame
{
public:
    WSFrame(std::string& audio_data,
            std::string& audio_format,
            uint32_t sample_rate,
            int32_t req_idx,
            bool add_pct,
            bool enable_itn,
            std::string& domain)
           : _audio_data(audio_data),
             _audio_format(audio_format),
             _sample_rate(sample_rate),
             _req_idx(req_idx),
             _speech_type(0),
             _add_pct(false),
             _enable_itn(false),
             _domain(domain) {};
    virtual ~WSFrame() {};

public:
    std::string      _audio_data;
    std::string      _audio_format;
    uint32_t         _sample_rate;
    int32_t          _req_idx;
    int32_t          _speech_type;
    bool             _add_pct;
    bool             _enable_itn;
    std::string      _domain;
};

class WSFrameManager
{
public:
    WSFrameManager(std::string& client_id, std::string& secret, std::string server_url, boost::shared_ptr<ClientListener> client_listener);
    virtual ~WSFrameManager();
    std::string genJsonRequest(boost::shared_ptr<WSFrame> speech_frame);
    void sendRequestFrame(websocketpp::connection_hdl hdl, boost::shared_ptr<WSFrame> speech_frame);
    int procWSRequestFrame(boost::shared_ptr<WSFrame> speech_frame); //return value: [-1: cached frame; 0: sent frame; 1: task over]
    bool sendFrameInList();
    void recvTextFrame(const std::string& payload);
    bool openConnection();
    void closeConnection();
    void stopIOService();
    void taskFailed(int32_t error_code);
    void taskFailed(int32_t error_code, std::string& message);
    void taskCompleted();
    void getTaskResult(std::list<std::string>& nbest, std::list<std::string>& uncertain);
    bool getToken();
#if WSS_URL
    context_ptr on_tls_init(websocketpp::connection_hdl);
#endif
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_fail(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg);

public:
    std::string                        _token;
    std::string                        _client_id;
    std::string                        _secret;
    std::string                        _server_url;
    boost::shared_ptr<ClientListener>  _client_listener;
    volatile TaskStatus                _task_status;
    std::string                        _text;
    int32_t                            _error_code;        //任务失败时的json response中的code字段
    std::string                        _message;           //任务失败时的json response中的message字段
    std::string                        _err_json_res;      //任务失败时的json response
    std::string                        _server_trace_id;   //ws_server返回的trace_id
    client                             _ws_client;
    websocketpp::connection_hdl        _hdl;
    boost::mutex                       _hdl_mutex;

    std::list<boost::shared_ptr<WSFrame> > _speech_frame_list;   //用于缓存音频包
    boost::mutex                       _speech_frame_list_mutex;

    boost::thread*                     _conn_thread;
    volatile bool                      _stop_getting_token;

    long                               _task_start_tp;   //任务开始时间点，用于任务超时，目前设置为60s

    std::list<std::string>             _nbest;
    std::list<std::string>             _uncertain;
    boost::mutex                       _nbest_mutex;
};
}
#endif
