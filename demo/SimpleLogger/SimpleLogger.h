/*!
 * �ļ� SimpleLogger.h
 * ���� huge
 * ���� 2022/09/08
 * ���� ����Consumer<std::string>���ʵ�ֵļ���־�����ͷ�ļ�
 */

#pragma once

#include <ctime>
#include <string>
#include <fstream>

#include "../../include/Consumer.h"
using namespace csm;

/**
 * ���� logger
 * ���� huge
 * ���� 2022/09/08
 * ���� ����־�����������ռ�
 */
namespace logger
{
	//��־��Ϣ�ṹ
	typedef struct LogMsg
	{
		LogMsg(const time_t t = 0, const std::string txt = "") : time(t), text(txt) {}

		time_t time;		//��Ϣ�ύ��ʱ���
		std::string text;	//��Ϣ�ı�����
	}*LogMsgPtr;

	//��־��������Ѳ�����������
	using T = LogMsg;

	//����־��
	class SimpleLogger : public csm::IConsumerHander<T>
	{
	public:
		SimpleLogger();
		virtual ~SimpleLogger();

	public:
		/**
		* ����: ������־���
		* ����: public 
		* ����: bool �����ɹ�����true�����򷵻�false
		* @ const char * pszFilePath: ָ����־�ļ�����ȫ·��
		* ��ע: 
		*	��ָ�����ļ�·���Ƿ��������ʧ��
		*	���������������ʲô������
		*/
		bool Start(const char* pszFilePath);
		/**
		* ����: ֹͣ��־���
		* ����: public 
		* ����: bool ֹͣ�ɹ�����true�����򷵻�false
		* @ bool bForce: �Ƿ�ǿ��ֹͣ��������ȴ����в�����־������ϣ�true��ʾǿ������ֹͣ��false��ʾ�ȴ�������־������Ϻ�ֹͣ��Ĭ��false
		* ��ע: 
		*	����Ĭ�Ϸ�ʽ(bForceΪfalse)ֹͣ��������п��������ֹͣʱ���ڻ���δ�������־����Stop������һ��ʱ��󷵻أ�����ϣ�������������ָ��bForceΪtrue
		*	�������ֹͣ����ʲô��������
		*	����ʱ���Զ���ǿ�Ʒ�ʽֹͣ���
		*/
		bool Stop(bool bForce = false);
		/**
		* ����: �ύһ����־��Ϣ
		* ����: public 
		* ����: bool �ύ�ɹ�����true�����򷵻�false
		* @ const char * pstrFormat: ���ύ����־��Ϣ(���ܴ���ʽ��)
		* @ ...: ��ʽ��ƥ����
		* ��ע: �ύ��־�ɹ�Ҫ�����������������־��Ϣ�ǿ�
		*/
		bool Commit(const char* pstrFormat, ...);

	public:
		/**
		* ����: ��time_t��ʽ��Ϊ�ַ���
		* ����: public static 
		* ����: std::string ���ر���ʽ������ַ�������ʽ��ʧ�ܷ��ؿմ�
		* @ time_t t: ����ʽ����ʱ���(��1970-01-01 08:00:00��������������)
		* @ const char * szFormat: ��ʽ����Ĭ��"%Y-%m-%d %H:%M:%S"
		*/
		static std::string FormatTimeToStr(time_t t, const char * szFormat = "%Y-%m-%d %H:%M:%S");

		// ͨ�� csm::IConsumerHander<T> �̳�
	public:
		/**
		* ����: ��־�����������
		* ����: virtual public 
		* ����: bool ����ɹ�����true�����򷵻�false
		* @ Consumer<T> * pSender: ָ�������¼���Consumer���ʵ��
		* ��ע: �ڴ���ɴ���־�ļ��Ĵ���
		*/
		virtual bool OnStart(Consumer<T>* pSender) override;
		/**
		* ����: ���µ���־��Ϣ������
		* ����: virtual public 
		* ����: bool ����ɹ�����true�����򷵻�false
		* @ Consumer<T> * pSender: ָ�������¼���Consumer���ʵ��
		* @ const T & data: �µ���־��Ϣ(LogMsgʵ��)������
		* ��ע: �ڴ���ɱ�����־��Ϣ���ļ��Ĵ���
		*/
		virtual bool OnConsume(Consumer<T>* pSender, const T & data) override;
		/**
		* ����: ��־�����ֹͣ
		* ����: virtual public 
		* ����: bool ����ɹ�����true�����򷵻�false
		* @ Consumer<T> * pSender: ָ�������¼���Consumer���ʵ��
		* @ bool bForce: �Ƿ�ǿ��ֹͣ��true��ʾǿ��ֹͣ��false����ֹͣ
		* ��ע: �ڴ���ɹر���־�ļ��Ĵ���
		*/
		virtual bool OnStop(Consumer<T>* pSender, bool bForce) override;

	protected:
		std::string m_strFilePath;	//��־�ļ�·��
		std::ofstream m_ofsLog;		//��־�ļ�������
		Consumer<T> m_LogConsumer;	//��־��Ϣ�������
	};
}
