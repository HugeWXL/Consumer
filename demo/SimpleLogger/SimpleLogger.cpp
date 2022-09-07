
#include <cstdarg>
#include "SimpleLogger.h"

namespace logger
{
	SimpleLogger::SimpleLogger()
		: m_LogConsumer(this)
	{
	}

	SimpleLogger::~SimpleLogger()
	{
		Stop(true);
	}

	bool SimpleLogger::Start(const char * pszFilePath)
	{
		if (!pszFilePath || !strlen(pszFilePath))
			return false;
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
		if (m_strFilePath.empty())
			return false;
		m_ofsLog.open(m_strFilePath, std::ios::out | std::ios::app);
		return m_ofsLog.is_open();
	}

	bool SimpleLogger::OnConsume(Consumer<T>* pSender, const T & data)
	{
		if (!m_ofsLog.is_open())
			return false;
		m_ofsLog << FormatTimeToStr(data.time) << "\t" << data.text << std::endl;
		return true;
	}
	
	bool SimpleLogger::OnStop(Consumer<T>* pSender, bool bForce)
	{
		if (m_ofsLog.is_open())
			m_ofsLog.close();
		return true;
	}
}
