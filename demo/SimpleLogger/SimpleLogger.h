#pragma once

#include <ctime>
#include <string>
#include <fstream>

#include "../../include/Consumer.h"
using namespace csm;

namespace logger
{
	//��־��Ϣ�ṹ
	typedef struct LogMsg
	{
		LogMsg(const time_t t = 0, const std::string txt = "") : time(t), text(txt) {}

		time_t time;		//��Ϣ�ύ��ʱ���
		std::string text;	//��Ϣ�ı�����
	}*LogMsgPtr;

	//������������
	using T = LogMsg;

	//����־��
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

		// ͨ�� csm::IConsumerHander<T> �̳�
	public:
		virtual bool OnStart(Consumer<T>* pSender) override;
		virtual bool OnConsume(Consumer<T>* pSender, const T & data) override;
		virtual bool OnStop(Consumer<T>* pSender, bool bForce) override;

	protected:
		std::string m_strFilePath;	//��־�ļ�·��
		std::ofstream m_ofsLog;		//��־�ļ�������
		Consumer<T> m_LogConsumer;	//��־��Ϣ�������
	};
}
