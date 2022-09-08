/*!
 * 文件 SimpleLogger.h
 * 作者 huge
 * 日期 2022/09/08
 * 描述 基于Consumer<std::string>组件实现的简单日志组件，头文件
 */

#pragma once

#include <ctime>
#include <string>
#include <fstream>

#include "../../include/Consumer.h"
using namespace csm;

/**
 * 名称 logger
 * 作者 huge
 * 日期 2022/09/08
 * 描述 简单日志组件类的命名空间
 */
namespace logger
{
	//日志消息结构
	typedef struct LogMsg
	{
		LogMsg(const time_t t = 0, const std::string txt = "") : time(t), text(txt) {}

		time_t time;		//消息提交的时间戳
		std::string text;	//消息文本内容
	}*LogMsgPtr;

	//日志组件的消费操作数据类型
	using T = LogMsg;

	//简单日志类
	class SimpleLogger : public csm::IConsumerHander<T>
	{
	public:
		SimpleLogger();
		virtual ~SimpleLogger();

	public:
		/**
		* 描述: 启动日志组件
		* 访问: public 
		* 返回: bool 启动成功返回true，否则返回false
		* @ const char * pszFilePath: 指定日志文件保存全路径
		* 备注: 
		*	若指定的文件路径非法则会启动失败
		*	若组件已启动，则什么都不做
		*/
		bool Start(const char* pszFilePath);
		/**
		* 描述: 停止日志组件
		* 访问: public 
		* 返回: bool 停止成功返回true，否则返回false
		* @ bool bForce: 是否强制停止组件而不等待所有残余日志保存完毕，true表示强制立即停止，false表示等待所有日志保存完毕后停止，默认false
		* 备注: 
		*	若按默认方式(bForce为false)停止组件，则有可能在组件停止时由于还有未保存的日志导致Stop会阻塞一段时间后返回，若不希望这样，你可以指定bForce为true
		*	若组件已停止，则什么都不会做
		*	析构时会自动以强制方式停止组件
		*/
		bool Stop(bool bForce = false);
		/**
		* 描述: 提交一条日志消息
		* 访问: public 
		* 返回: bool 提交成功返回true，否则返回false
		* @ const char * pstrFormat: 待提交的日志消息(可能带格式符)
		* @ ...: 格式符匹配项
		* 备注: 提交日志成功要求组件已启动，且日志消息非空
		*/
		bool Commit(const char* pstrFormat, ...);

	public:
		/**
		* 描述: 将time_t格式化为字符串
		* 访问: public static 
		* 返回: std::string 返回被格式化后的字符串，格式化失败返回空串
		* @ time_t t: 待格式化的时间戳(自1970-01-01 08:00:00以来经历的秒数)
		* @ const char * szFormat: 格式符，默认"%Y-%m-%d %H:%M:%S"
		*/
		static std::string FormatTimeToStr(time_t t, const char * szFormat = "%Y-%m-%d %H:%M:%S");

		// 通过 csm::IConsumerHander<T> 继承
	public:
		/**
		* 描述: 日志组件正在启动
		* 访问: virtual public 
		* 返回: bool 处理成功返回true，否则返回false
		* @ Consumer<T> * pSender: 指向发生该事件的Consumer组件实例
		* 备注: 在此完成打开日志文件的处理
		*/
		virtual bool OnStart(Consumer<T>* pSender) override;
		/**
		* 描述: 有新的日志消息待保存
		* 访问: virtual public 
		* 返回: bool 保存成功返回true，否则返回false
		* @ Consumer<T> * pSender: 指向发生该事件的Consumer组件实例
		* @ const T & data: 新的日志消息(LogMsg实例)的引用
		* 备注: 在此完成保存日志消息到文件的处理
		*/
		virtual bool OnConsume(Consumer<T>* pSender, const T & data) override;
		/**
		* 描述: 日志组件已停止
		* 访问: virtual public 
		* 返回: bool 处理成功返回true，否则返回false
		* @ Consumer<T> * pSender: 指向发生该事件的Consumer组件实例
		* @ bool bForce: 是否被强制停止，true表示强制停止，false正常停止
		* 备注: 在此完成关闭日志文件的处理
		*/
		virtual bool OnStop(Consumer<T>* pSender, bool bForce) override;

	protected:
		std::string m_strFilePath;	//日志文件路径
		std::ofstream m_ofsLog;		//日志文件流对象
		Consumer<T> m_LogConsumer;	//日志消息消费组件
	};
}
