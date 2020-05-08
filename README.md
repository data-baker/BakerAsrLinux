 

# 标贝科技实时语音识别服务Linux SDK使用说明文档

 

## 简介

​    本文档描述了标贝科技实时语音识别C++ SDK的使用指南。



## 库使用及demo

### 链接lib

​    参考BakerAsrLinux项目CMakeLists.txt中的语法

### 关键类及方法

​    所有涉及的字符均使用utf-8编码

#### SpeechManager：语音识别任务处理类。

​    类成员方法：

| 方法                   | 参数说明                                             | 说明                    |
| ---------------------- | ---------------------------------------------------- | ----------------------- |
| int init(…)            | string client_id                                     | 从标贝获取的client id   |
|                        | string secret                                        | 从标贝获取的secret      |
|                        | string server_url                                    | websockt服务地址        |
|                        | string audio_format                                  | 音频格式，pcm、wav      |
|                        | uint32_t sample_rate                                 | 采样率，8000、16000     |
|                        | boost::shared_ptr\<ClientListener\>  client_listener | 回调类指针              |
| int procSpeechData (…) | string speech_data                                   | **160ms的原始音频数据** |
|                        | bool last_frame                                      | 是否是最后一包音频数据  |
| void getTaskResult(…)  | list\<string\> nbest                                 | nbest结果集             |
|                        | list\<string\> uncertain                             | 预测结果集              |
| getTaskStatus()        | 无                                                   | 获取任务状态            |
| int stopTask()         | 无参数                                               | 停止当前识别任务        |

 

#### ClientListener：任务结果回调类。

​    在获得识别结果数据，发生错误等事件发生时会触发回调。您应当实现该类，在回调方法中加入自己的处理逻辑。

​    类成员方法：

| 方法                   | 参数                     | 说明                                     |
| ---------------------- | ------------------------ | ---------------------------------------- |
| void onTaskStarted()   | 无参数                   |                                          |
| void onTextReceived(…) | uint32_t idx             | 文本序号，暂时忽略                       |
|                        | list\<string>  nbest     | 识别结果，取第一个                       |
|                        | list\<string>  uncertain | 预测结果，暂无                           |
| void onTaskCompleted() | 无参数                   | 任务完成                                 |
| void onTaskFailed()    | uint32_t code            | 错误码                                   |
|                        | string info              | 提示信息                                 |
|                        | string trace_id          | 跟踪id，偶现忽略，重复出现反馈给开发人员 |
| void onLog             | string log               | sdk内部生成的日志，默认打印到终端        |

 

### 调用顺序

#### 顺序描述

​    1 实例化SpeechManager

​    2 调用init

​    3 传入一包音频数据

​    4 回调返回音频数据或错误信息

​     …步骤3 4循环

​    5 任务结束

​    如果需要开始新的识别任务，重复1-5步骤即可；

#### 方法调用顺序

​    boost::shared_ptr\<ClientListener> client_listener(new(std::nothrow) MyClientListener());

​    boost::shared_ptr\<SpeechManager> speech_manager(new(std::nothrow) SpeechManager());

​    speech_manager->init(g_clientid, g_secret, server_url, audio_format, sample_rate, client_listener);

​    speech_manager->procSpeechData(audio_data, last_frame); (每个audio_data是160ms的音频数据)

​     … callback

​    speech_manager.reset()

​    client_listener.reset()

### 错误码

​    识别相关code码：

| **错误码** | **含义**                       | **解决方案**                           |
| ---------- | ------------------------------ | -------------------------------------- |
| 90000      | 识别成功                       |                                        |
| 90001      | 连接服务器失败                 | 检查server_url是否正确                 |
| 90002      | 任务失败，返回json解析失败     | 偶现忽略，重复出现可反馈trace_id给开发 |
| 90003      | 任务失败，返回结果缺少必要字段 |                                        |
| 90004      | 任务失败，任务超时             |                                        |
| 90005      | 任务失败，其他错误             |                                        |
| 90006      | 任务失败，获取token失败        | 同上                                   |
| 90007      | clientid不支持asr功能          | 更换正确的clientid                     |
| 90008      | clientid或secret错误           | 更换正确的clientid或secret             |
|            |                                |                                        |
| 30001      | HTTP请求参数错误               | 偶现忽略，重复出现可反馈trace_id给开发 |
| 30002      | 服务内部错误                   |                                        |
| 30003      | 识别结果解析出错               |                                        |
| 30004      | 应用包名未知                   |                                        |
| 30005      | 语音质量问题                   |                                        |
| 30006      | 输入语音过长                   |                                        |
| 30008      | 会话id不存在                   |                                        |
| 30007      | 连接识别引擎失败               |                                        |
| 30009      | Rpc调用非法                    |                                        |
| 30010      | redis rpop操作返回空           |                                        |
| 30011      | redis rpop值不合法             |                                        |
| 30012      | rpc调用识别引擎失败            |                                        |
| 30013      | Redis rpop操作失败             |                                        |
| 30014      | redis lpush操作失败            |                                        |
| 30015      | 单个语音分片过长               |                                        |
| 30016      | 回调url失败                    | 同上                                   |
|            |                                |                                        |
| 40008      | token校验失败                  |                                        |
| 40009      | token处于未激活状态            | 检查相应的client_id                    |
| 40010      | token已过期                    | 重新获取token                          |
| 40011      | 使用量已超过购买量             | 检查相应的client_id                    |
| 40012      | qps错误                        | 增大qps                                |
|            |                                |                                        |
| 50001      | 服务端处理超时                 | 偶现忽略，重复出现可反馈trace_id给开发 |
| 50002      | 服务端内部rpc调用失败          |                                        |
| 50003      | 服务端繁忙                     |                                        |
| 50004      | 其他服务端内部错误             | 同上                                   |
|            |                                |                                        |

 

### demo的使用

​    demo.cpp文件描述了识别接口的使用细节，请仔细阅读后进行识别接口的使用。