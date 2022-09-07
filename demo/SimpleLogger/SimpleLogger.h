#pragma once

#include <ctime>
#include <string>
#include <fstream>

#include "../../include/Consumer.h"
using namespace csm;

namespace logger
{
	//日志消息结构
	typedef struct LogMsg
	{
		LogMsg(const time_t t = 0, const std::string txt = "") : time(t), text(txt) {}

		time_t time;		//消息提交的时间戳
		std::string text;	//消息文本内容
	}*LogMsgPtr;

	//消费数据类型
	using T = LogMsg;

	//简单日志类
	class SimpleLogger : public csm::IConsumerHander<T>
	{
	public:
		SimpleLogger();
		virtual ~SimpleLogger();

	public:
		bool Start(const char* pszFilePath);
		bool Stop(bool bForce = false);
		bool Commit(const char* pszLogMsg, ...);

	public:
		static std::string FormatTimeToStr(time_t t, const char * szFormat = "%Y-%m-%d %H:%M:%S");

		// 通过 csm::IConsumerHander<T> 继承
	public:
		virtual bool OnStart(Consumer<T>* pSender) override;
		virtual bool OnConsume(Consumer<T>* pSender, const T & data) override;
		virtual bool OnStop(Consumer<T>* pSender, bool bForce) override;

	protected:
		std::string m_strFilePath;	//日志文件路径
		std::ofstream m_ofsLog;		//日志文件流对象
		Consumer<T> m_LogConsumer;	//日志消息消费组件
	};
}
