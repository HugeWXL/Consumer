/*!
 * 文件 SimpleLogger.cpp
 * 作者 huge
 * 日期 2022/09/08
 * 描述 基于Consumer<std::string>组件实现的简单日志组件，源文件
 */

#include <cstdarg>
#include "SimpleLogger.h"

namespace logger
{
	SimpleLogger::SimpleLogger()
		: m_LogConsumer(this) //将SimpleLogger作为Consumer的Handler
	{
	}

	SimpleLogger::~SimpleLogger()
	{
		Stop(true);	//析构时强制停止组件
	}

	bool SimpleLogger::Start(const char * pszFilePath)
	{
		if (!pszFilePath || !strlen(pszFilePath))
			return false;
		
		//若组件未启动，则启动之
		if (!m_LogConsumer.HasStarted())
		{
			m_strFilePath = std::move(std::string(pszFilePath));
			return m_LogConsumer.Start();
		}
		return true;
	}
	
	bool SimpleLogger::Stop(bool bForce/* = false*/)
	{
		return m_LogConsumer.Stop(bForce);
	}

	bool SimpleLogger::Commit(const char * pstrFormat, ...)
	{
		if (!pstrFormat || !strlen(pstrFormat))
			return false;
		
		//在此完成日志消息的格式化，并将格式化结果提交到Consumer组件
		va_list Args;
		va_start(Args, pstrFormat);
		int nLen = vsnprintf(nullptr, 0, pstrFormat, Args);						//先得到日志消息文本的长度
		if (nLen <= 0)
			return false;

		char *pszBuffer = new char[nLen + 1]{ };
		nLen = vsnprintf(pszBuffer, nLen + 1, pstrFormat, Args);				//格式化日志消息
		auto bCommited = m_LogConsumer.Commit(T(::time(nullptr), pszBuffer));	//向队列中插入一条新的日志
		delete[] pszBuffer;
		va_end(Args);
		return bCommited;
	}

	std::string SimpleLogger::FormatTimeToStr(time_t t, const char * szFormat)
	{
		if (!szFormat) return "";
		if (t < 0) t = 0;
		char szTime[40] = { 0 };
		struct tm local_time;
		localtime_s(&local_time, &t);
		strftime(szTime, sizeof(szTime), szFormat, &local_time);
		return std::string(szTime);
	}

	bool SimpleLogger::OnStart(Consumer<T>* pSender)
	{
		//在此完成 打开指定日志文件 的处理
		if (m_strFilePath.empty())
			return false;
		m_ofsLog.open(m_strFilePath, std::ios::out | std::ios::app);	//以追加方式打开
		return m_ofsLog.is_open();
	}

	bool SimpleLogger::OnConsume(Consumer<T>* pSender, const T & data)
	{
		//在此完成 保存单条日志消息到日志文件 的处理
		if (!m_ofsLog.is_open())
			return false;
		m_ofsLog << FormatTimeToStr(data.time) << "\t" << data.text << std::endl;
		return true;
	}
	
	bool SimpleLogger::OnStop(Consumer<T>* pSender, bool bForce)
	{
		//在此关闭日志文件
		if (m_ofsLog.is_open())
			m_ofsLog.close();
		return true;
	}
}
