/*!
 * \文件 Consumer.h
 * \作者 huge
 * \日期 2022/09/06
 * \描述 实现了一个具备通用性的消费者组件类模板
 */

#pragma once

#ifdef _DEBUG
#include <sstream>
#include <Windows.h>
#endif

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>

 /**
  * \命名空间 csm(Consumer)
  * \作者 huge
  * \日期 2022/09/06
  * \描述 包含了消费者组件的定义和实现(C++11)
  */
namespace csm
{
	//互斥锁类型
	using unique_lock_t = std::unique_lock<std::mutex>;

	//消费者组件类先导声明，以被IConsumerHander<T>类识别
	template<typename T>
	class Consumer;

	/**
	 * \类名 IConsumerHander<T>
	 * \作者 huge
	 * \日期 2022/09/06
	 * \描述 消费者组件事件处理Handler类
	 *	它定义了消费者组件Consumer<T>所有可能发生的事件，你可以酌情重写事件处理接口以满足特定需求
	 */
	template <class T>
	class IConsumerHander
	{
	public:
		/**
		* 描述: 消费者组件刚刚启动，
		* 访问: virtual public
		* 返回: bool 处理成功请返回true，否则返回false
		* @ Consumer<T> * pSender: 指向发生此事件的消费者组件实例
		* 备注:
		*	你可以在此初始化消费需要的相关资源，如建立网络连接、加载文件等
		*	返回值将直接决定消费者线程是否进入消费循环，返回true将进入消费循环，返回false直接退出消费线程，停止组件
		*/
		virtual bool OnStart(Consumer<T>* pSender) { return true; }
		/**
		* 描述: 发生单次消费动作
		* 访问: virtual public
		* 返回: bool 消费成功请返回true，否则返回false
		* @ Consumer<T> * pSender: 指向发生该事件的消费者组件实例
		* @ const T & data: 待消费的数据
		* 备注:
		*	由于该事件在消费者线程中发生，故请务必保证事件处理中的代码是线程安全的;
		*/
		virtual bool OnConsume(Consumer<T>* pSender, const T& data) = 0;
		/**
		* 描述: 消费者组件正在停止
		* 访问: virtual public
		* 返回: bool 操作成功请返回true，否则返回false
		* @ Consumer<T> * pSender: 指向发生该事件的消费者组件实例
		* @ bool bForce: 标识该组件是否被强制停止
		* 备注:
		*	你可以在此释放该组件启动时初始化的相关资源(如果需要的话)，如端口网络连接、关闭文件等
		*	该接口的返回值不对组件产生任何影响
		*/
		virtual bool OnStop(Consumer<T>* pSender, bool bForce) { return true; }

	public:
		virtual ~IConsumerHander() {}
	};

	/**
	 * \类名 Consumer
	 * \作者 huge
	 * \日期 2022/06/23
	 * \描述 消费者组件类模板
	 * \备注 要求编译器支持C++11或更高语言标准
	 *	该组件实现了单消费者线程模型，模板参数标识消费队列中的数据类型
	 *	[注意事项]：
	 *	构造时必须传入一个IConsumerHander<T>回调实例的非空地址
	 */
	template<typename T>
	class Consumer
	{
	public:
		using ConsumeStartCallback_t = std::function<bool(Consumer<T>*)>;		//回调函数原型-组件启动
		using ConsumeStopCallback_t = std::function<bool(Consumer<T>*, bool)>;	//回调函数原型-组件通知
		using ConsumingCallback_t = std::function<bool(Consumer<T>*, const T&)>;//回调函数原型-组件单次消费

	public:
		/**
		* 描述: 构造函数，指定组件Handler实例
		* 访问: public
		* @ IConsumerHander<T> * pHandler: 指向组件Handler实例
		* @ const void * pUserData: 指向组件关联的用户数据(可选)
		*/
		Consumer(IConsumerHander<T>* pHandler, const void* pUserData = nullptr)
			: m_pHandler(pHandler)
			, m_pUserData(pUserData) { }
		/**
		* 描述: 构造函数，指定组件回调函数
		* 访问: public
		* @ ConsumingCallback_t ConsumingCallback: 单次消费的回调函数，在此回调中进行数据处理
		* @ ConsumeStartCallback_t ConsumeStartCallback: 组件启动的回调函数，在此回调中进行组件初始化相关处理(可选)
		* @ ConsumeStopCallback_t ConsumeStopCallback: 组件停止的回调函数，在此回调中进行组件资源释放相关处理(可选)
		* @ const void * pUserData: 指向组件关联的用户数据(可选)
		*/
		Consumer(ConsumingCallback_t ConsumingCallback,
			ConsumeStartCallback_t ConsumeStartCallback = nullptr,
			ConsumeStopCallback_t ConsumeStopCallback = nullptr,
			const void* pUserData = nullptr)
			: m_pHandler(nullptr)
			, m_pUserData(pUserData)
			, m_ConsumingCallback(ConsumingCallback)
			, m_ConsumeStartCallback(ConsumeStartCallback)
			, m_ConsumeStopCallback(ConsumeStopCallback) { }
		/**
		* 描述: 析构函数
		* 访问: virtual public
		* 备注: 析构时自动强制停止组件
		*/
		virtual ~Consumer()
		{
			Stop(true);
		}

	public:
		/**
		* 描述: 设置用户数据
		* 访问: public
		* 返回: void
		* @ const void * pData: 指向用户数据的地址
		* 备注:
		*	你可以通过此接口将消费者组件实例与你的附加数据绑定起来，当组件发生任何事件时，你可以在处理事件时通过组件实例获取这些附加数据；
		*	附加数据一般是为了确定不同组件实例对应的不同业务，如某个文件路径，网络连接id等
		*/
		void SetUserData(const void* pData) { m_pUserData = pData; }
		/**
		* 描述: 获取消费者组件实例关联的用户数据
		* 访问: public
		* 返回: const void* 组件实例关联的用户数据
		*/
		const void* GetUserData()const { return m_pUserData; }
		/**
		* 描述: 启动组件
		* 访问: public
		* 返回: bool 启动成功返回true，否则返回false
		* @ unsigned short consumer_count: 指定消费者线程个数，默认1个
		* 备注:
		*	1.可多次调用，但只会真正启动一次
		*	2.消费线程个数建议不要超过[2 * 机器核数 + 2]个
		*	3.成功启动组件要求至少满足以下条件之一：
		*	·指定了Handler实例，并在组件启动事件处理中返回true;
		*	·未指定Handler实例，但指定了消费操作回调函数;
		*/
		bool Start(unsigned short consumer_count = 1)
		{
			if (!consumer_count)
				return false;

			unique_lock_t lock(m_mtxFlag);
			if (!HasStarted())
			{
				//组件即将启动，在此初始化资源
				//此处要求组件必须指定了Handler或者消费处理回调函数，否则不予创建消费线程
				if (!m_pHandler && !m_ConsumingCallback)
				{
					OutputDebugStringA("消费者组件启动失败，可能原因是：1.未指定Handler;2.未指定消费处理回调函数.\n");
					return false;
				}

				//优先通过Handler处理，若未指定Handler，则通过回调函数处理
				auto flag = m_pHandler ?
					m_pHandler->OnStart(this)
					: !m_ConsumeStartCallback || m_ConsumeStartCallback(this);
				if (!flag)
				{
					OutputDebugStringA("消费者组件启动失败，原因是：组件启动事件处理返回了false.\n");
					return false;
				}

				//启动若干个消费线程
				{
					unique_lock_t lock(m_mtxData);
					m_bStoped = false;
					m_bForceStop = false;
				}
				for (unsigned short i = 0; i < consumer_count; i++)
					m_Consumers.push_back(std::move(std::thread(&Consumer<T>::fnConsumer, this)));
			}
			return true;
		}
		/**
		* 描述: 停止组件
		* 访问: public
		* 返回: bool 停止成功返回true，否则false
		* @ bool bForce: 标识是否立即强制停止组件，true表示立即强制停止组件，false表示等待所有数据消费完成后停止，默认false
		* 备注:
		*	若组件已经停止，则不会做任何事；
		*	若指定bForce为false，则停止组件有可能因为消费操作耗时而阻塞当前线程一段时间，若不希望如此，可以传true
		*/
		bool Stop(bool bForce = false)
		{
			unique_lock_t lock(m_mtxFlag);
			if (HasStarted())
			{
				//将m_bStoped置true后唤醒所有消费者线程，使其安全结束
				{
					unique_lock_t lock(m_mtxData);
					m_bStoped = true;
					m_bForceStop = bForce;
				}
				m_cvNewDataOrStoped.notify_all();	//唤醒所有等待在此条件上的线程，让它们优雅地退出
				for (auto& consumer : m_Consumers)	//等待所有消费者线程结束
				{
					if (consumer.joinable())
						consumer.join();
				}
				m_Consumers.clear();

				//组件即将停止，在此释放资源
				//优先通过Handler处理，若未指定Handler，则通过回调处理
				return m_pHandler ?
					m_pHandler->OnStop(this, m_bForceStop)
					: !m_ConsumeStopCallback || m_ConsumeStopCallback(this, m_bForceStop);
			}
			return true;
		}
		/**
		* 描述: 检查组件是否已启动
		* 访问: public
		* 返回: bool true表示组件已启动，false已停止
		* 备注:
		*/
		bool HasStarted() { return !m_bStoped; }
		/**
		* 描述: 向组件提交一条数据(单次生产操作)
		* 访问: public
		* 返回: bool true表示提交成功，false提交失败
		* @ const T & data: 提交的数据对象
		* 备注: 若组件未启动，则不会提交数据，因此提交数据前请先启动组件
		*/
		bool Commit(const T& data)
		{
			if (!HasStarted())
				return false;
			//数据入列
			{
				unique_lock_t lock(m_mtxData);
				m_DataQue.push(data);
			}
			m_cvNewDataOrStoped.notify_one();	//唤醒某个消费者线程，使其消费刚刚提交的新数据
			return true;
		}

	protected:
		/**
		* 描述: 消费者线程函数
		* 访问: virtual protected
		* 返回: void
		*/
		virtual void fnConsumer()
		{
#ifdef _DEBUG
			auto tid = std::this_thread::get_id();
			std::stringstream ss;
			ss << "消费者线程[" << tid << "]已启动" << std::endl;
			OutputDebugStringA(ss.str().data());
			ss.str("");
#endif // _DEBUG
			while (true)
			{
				T data;	//用于缓存最新取出的单条数据
				{
					//检查队列中是否仍有待消费的数据，或者外部停止了组件
					unique_lock_t lock(m_mtxData);
					m_cvNewDataOrStoped.wait(lock, [this] {return m_bStoped || !m_DataQue.empty(); });

					//若外部停止了组件，且为立即强制退出或队列中没有待消费的数据，可以退出线程
					if (m_bStoped && (m_bForceStop || m_DataQue.empty()))
						break;

					//否则一定是队列中还有待消费数据，取出一条消费之
					data = std::move(m_DataQue.front());
					m_DataQue.pop();
				}

				//数据消费处理
				auto flag = m_pHandler && m_pHandler->OnConsume(this, data);
				if (!flag && !m_pHandler)
					flag = m_ConsumingCallback && m_ConsumingCallback(this, data);
#ifdef _DEBUG
				ss << "消费线程[" << tid << "]发生消费，操作结果：" << flag << std::endl;
				OutputDebugStringA(ss.str().data());
				ss.str("");
#endif // _DEBUG
			}
#ifdef _DEBUG
			ss.clear();
			if (m_bForceStop && !m_DataQue.empty())
				ss << "消费者线程[" << tid << "]已强制结束" << std::endl;
			else
				ss << "消费者线程[" << tid << "]已结束" << std::endl;
			OutputDebugStringA(ss.str().data());
			ss.str("");
#endif // _DEBUG
		}

	protected:
		const void* m_pUserData = nullptr;				//组件关联的用户附加数据
		IConsumerHander<T>* m_pHandler = nullptr;		//组件事件回调实例

		//仅当未指定Handler时，以下三个回调才有可能生效
		ConsumeStartCallback_t m_ConsumeStartCallback;	//回调-消费组件启动
		ConsumeStopCallback_t m_ConsumeStopCallback;	//回调-消费组件停止
		ConsumingCallback_t m_ConsumingCallback;		//回调-单次消费

		bool m_bStoped = true;							//标识组件是否已经停止运行，true表示已停止，false表示已启动
		bool m_bForceStop = false;						//退出线程标志，标识组件是否要立即强制退出，而不等待所有数据消费完成，true表示立即强制退出，默认false
		std::mutex m_mtxFlag;							//标识互斥量
		std::mutex m_mtxData;							//数据互斥量
		std::condition_variable m_cvNewDataOrStoped;	//条件变量，检查是否有新数据提交或组件被停止
		std::queue<T> m_DataQue;						//数据队列
		std::vector<std::thread> m_Consumers;			//消费者线程对象集合
	};
}