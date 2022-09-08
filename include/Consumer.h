/*!
 * \�ļ� Consumer.h
 * \���� huge
 * \���� 2022/09/06
 * \���� ʵ����һ���߱�ͨ���Ե������������ģ��
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
  * \�����ռ� csm(Consumer)
  * \���� huge
  * \���� 2022/09/06
  * \���� ����������������Ķ����ʵ��(C++11)
  */
namespace csm
{
	//����������
	using unique_lock_t = std::unique_lock<std::mutex>;

	//������������ȵ��������Ա�IConsumerHander<T>��ʶ��
	template<typename T>
	class Consumer;

	/**
	 * \���� IConsumerHander<T>
	 * \���� huge
	 * \���� 2022/09/06
	 * \���� ����������¼�����Handler��
	 *	�����������������Consumer<T>���п��ܷ������¼��������������д�¼�����ӿ��������ض�����
	 */
	template <class T>
	class IConsumerHander
	{
	public:
		/**
		* ����: ����������ո�������
		* ����: virtual public
		* ����: bool ����ɹ��뷵��true�����򷵻�false
		* @ Consumer<T> * pSender: ָ�������¼������������ʵ��
		* ��ע:
		*	������ڴ˳�ʼ��������Ҫ�������Դ���罨���������ӡ������ļ���
		*	����ֵ��ֱ�Ӿ����������߳��Ƿ��������ѭ��������true����������ѭ��������falseֱ���˳������̣߳�ֹͣ���
		*/
		virtual bool OnStart(Consumer<T>* pSender) { return true; }
		/**
		* ����: �����������Ѷ���
		* ����: virtual public
		* ����: bool ���ѳɹ��뷵��true�����򷵻�false
		* @ Consumer<T> * pSender: ָ�������¼������������ʵ��
		* @ const T & data: �����ѵ�����
		* ��ע:
		*	���ڸ��¼����������߳��з�����������ر�֤�¼������еĴ������̰߳�ȫ��;
		*/
		virtual bool OnConsume(Consumer<T>* pSender, const T& data) = 0;
		/**
		* ����: �������������ֹͣ
		* ����: virtual public
		* ����: bool �����ɹ��뷵��true�����򷵻�false
		* @ Consumer<T> * pSender: ָ�������¼������������ʵ��
		* @ bool bForce: ��ʶ������Ƿ�ǿ��ֹͣ
		* ��ע:
		*	������ڴ��ͷŸ��������ʱ��ʼ���������Դ(�����Ҫ�Ļ�)����˿��������ӡ��ر��ļ���
		*	�ýӿڵķ���ֵ������������κ�Ӱ��
		*/
		virtual bool OnStop(Consumer<T>* pSender, bool bForce) { return true; }

	public:
		virtual ~IConsumerHander() {}
	};

	/**
	 * \���� Consumer
	 * \���� huge
	 * \���� 2022/06/23
	 * \���� �����������ģ��
	 * \��ע Ҫ�������֧��C++11��������Ա�׼
	 *	�����ʵ���˵��������߳�ģ�ͣ�ģ�������ʶ���Ѷ����е���������
	 *	[ע������]��
	 *	����ʱ���봫��һ��IConsumerHander<T>�ص�ʵ���ķǿյ�ַ
	 */
	template<typename T>
	class Consumer
	{
	public:
		using ConsumeStartCallback_t = std::function<bool(Consumer<T>*)>;		//�ص�����ԭ��-�������
		using ConsumeStopCallback_t = std::function<bool(Consumer<T>*, bool)>;	//�ص�����ԭ��-���֪ͨ
		using ConsumingCallback_t = std::function<bool(Consumer<T>*, const T&)>;//�ص�����ԭ��-�����������

	public:
		/**
		* ����: ���캯����ָ�����Handlerʵ��
		* ����: public
		* @ IConsumerHander<T> * pHandler: ָ�����Handlerʵ��
		* @ const void * pUserData: ָ������������û�����(��ѡ)
		*/
		Consumer(IConsumerHander<T>* pHandler, const void* pUserData = nullptr)
			: m_pHandler(pHandler)
			, m_pUserData(pUserData) { }
		/**
		* ����: ���캯����ָ������ص�����
		* ����: public
		* @ ConsumingCallback_t ConsumingCallback: �������ѵĻص��������ڴ˻ص��н������ݴ���
		* @ ConsumeStartCallback_t ConsumeStartCallback: ��������Ļص��������ڴ˻ص��н��������ʼ����ش���(��ѡ)
		* @ ConsumeStopCallback_t ConsumeStopCallback: ���ֹͣ�Ļص��������ڴ˻ص��н��������Դ�ͷ���ش���(��ѡ)
		* @ const void * pUserData: ָ������������û�����(��ѡ)
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
		* ����: ��������
		* ����: virtual public
		* ��ע: ����ʱ�Զ�ǿ��ֹͣ���
		*/
		virtual ~Consumer()
		{
			Stop(true);
		}

	public:
		/**
		* ����: �����û�����
		* ����: public
		* ����: void
		* @ const void * pData: ָ���û����ݵĵ�ַ
		* ��ע:
		*	�����ͨ���˽ӿڽ����������ʵ������ĸ������ݰ�����������������κ��¼�ʱ��������ڴ����¼�ʱͨ�����ʵ����ȡ��Щ�������ݣ�
		*	��������һ����Ϊ��ȷ����ͬ���ʵ����Ӧ�Ĳ�ͬҵ����ĳ���ļ�·������������id��
		*/
		void SetUserData(const void* pData) { m_pUserData = pData; }
		/**
		* ����: ��ȡ���������ʵ���������û�����
		* ����: public
		* ����: const void* ���ʵ���������û�����
		*/
		const void* GetUserData()const { return m_pUserData; }
		/**
		* ����: �������
		* ����: public
		* ����: bool �����ɹ�����true�����򷵻�false
		* @ unsigned short consumer_count: ָ���������̸߳�����Ĭ��1��
		* ��ע:
		*	1.�ɶ�ε��ã���ֻ����������һ��
		*	2.�����̸߳������鲻Ҫ����[2 * �������� + 2]��
		*	3.�ɹ��������Ҫ������������������֮һ��
		*	��ָ����Handlerʵ����������������¼������з���true;
		*	��δָ��Handlerʵ������ָ�������Ѳ����ص�����;
		*/
		bool Start(unsigned short consumer_count = 1)
		{
			if (!consumer_count)
				return false;

			unique_lock_t lock(m_mtxFlag);
			if (!HasStarted())
			{
				//��������������ڴ˳�ʼ����Դ
				//�˴�Ҫ���������ָ����Handler�������Ѵ���ص������������贴�������߳�
				if (!m_pHandler && !m_ConsumingCallback)
				{
					OutputDebugStringA("�������������ʧ�ܣ�����ԭ���ǣ�1.δָ��Handler;2.δָ�����Ѵ���ص�����.\n");
					return false;
				}

				//����ͨ��Handler������δָ��Handler����ͨ���ص���������
				auto flag = m_pHandler ?
					m_pHandler->OnStart(this)
					: !m_ConsumeStartCallback || m_ConsumeStartCallback(this);
				if (!flag)
				{
					OutputDebugStringA("�������������ʧ�ܣ�ԭ���ǣ���������¼���������false.\n");
					return false;
				}

				//�������ɸ������߳�
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
		* ����: ֹͣ���
		* ����: public
		* ����: bool ֹͣ�ɹ�����true������false
		* @ bool bForce: ��ʶ�Ƿ�����ǿ��ֹͣ�����true��ʾ����ǿ��ֹͣ�����false��ʾ�ȴ���������������ɺ�ֹͣ��Ĭ��false
		* ��ע:
		*	������Ѿ�ֹͣ���򲻻����κ��£�
		*	��ָ��bForceΪfalse����ֹͣ����п�����Ϊ���Ѳ�����ʱ��������ǰ�߳�һ��ʱ�䣬����ϣ����ˣ����Դ�true
		*/
		bool Stop(bool bForce = false)
		{
			unique_lock_t lock(m_mtxFlag);
			if (HasStarted())
			{
				//��m_bStoped��true���������������̣߳�ʹ�䰲ȫ����
				{
					unique_lock_t lock(m_mtxData);
					m_bStoped = true;
					m_bForceStop = bForce;
				}
				m_cvNewDataOrStoped.notify_all();	//�������еȴ��ڴ������ϵ��̣߳����������ŵ��˳�
				for (auto& consumer : m_Consumers)	//�ȴ������������߳̽���
				{
					if (consumer.joinable())
						consumer.join();
				}
				m_Consumers.clear();

				//�������ֹͣ���ڴ��ͷ���Դ
				//����ͨ��Handler������δָ��Handler����ͨ���ص�����
				return m_pHandler ?
					m_pHandler->OnStop(this, m_bForceStop)
					: !m_ConsumeStopCallback || m_ConsumeStopCallback(this, m_bForceStop);
			}
			return true;
		}
		/**
		* ����: �������Ƿ�������
		* ����: public
		* ����: bool true��ʾ�����������false��ֹͣ
		* ��ע:
		*/
		bool HasStarted() { return !m_bStoped; }
		/**
		* ����: ������ύһ������(������������)
		* ����: public
		* ����: bool true��ʾ�ύ�ɹ���false�ύʧ��
		* @ const T & data: �ύ�����ݶ���
		* ��ע: �����δ�������򲻻��ύ���ݣ�����ύ����ǰ�����������
		*/
		bool Commit(const T& data)
		{
			if (!HasStarted())
				return false;
			//��������
			{
				unique_lock_t lock(m_mtxData);
				m_DataQue.push(data);
			}
			m_cvNewDataOrStoped.notify_one();	//����ĳ���������̣߳�ʹ�����Ѹո��ύ��������
			return true;
		}

	protected:
		/**
		* ����: �������̺߳���
		* ����: virtual protected
		* ����: void
		*/
		virtual void fnConsumer()
		{
#ifdef _DEBUG
			auto tid = std::this_thread::get_id();
			std::stringstream ss;
			ss << "�������߳�[" << tid << "]������" << std::endl;
			OutputDebugStringA(ss.str().data());
			ss.str("");
#endif // _DEBUG
			while (true)
			{
				T data;	//���ڻ�������ȡ���ĵ�������
				{
					//���������Ƿ����д����ѵ����ݣ������ⲿֹͣ�����
					unique_lock_t lock(m_mtxData);
					m_cvNewDataOrStoped.wait(lock, [this] {return m_bStoped || !m_DataQue.empty(); });

					//���ⲿֹͣ���������Ϊ����ǿ���˳��������û�д����ѵ����ݣ������˳��߳�
					if (m_bStoped && (m_bForceStop || m_DataQue.empty()))
						break;

					//����һ���Ƕ����л��д��������ݣ�ȡ��һ������֮
					data = std::move(m_DataQue.front());
					m_DataQue.pop();
				}

				//�������Ѵ���
				auto flag = m_pHandler && m_pHandler->OnConsume(this, data);
				if (!flag && !m_pHandler)
					flag = m_ConsumingCallback && m_ConsumingCallback(this, data);
#ifdef _DEBUG
				ss << "�����߳�[" << tid << "]�������ѣ����������" << flag << std::endl;
				OutputDebugStringA(ss.str().data());
				ss.str("");
#endif // _DEBUG
			}
#ifdef _DEBUG
			ss.clear();
			if (m_bForceStop && !m_DataQue.empty())
				ss << "�������߳�[" << tid << "]��ǿ�ƽ���" << std::endl;
			else
				ss << "�������߳�[" << tid << "]�ѽ���" << std::endl;
			OutputDebugStringA(ss.str().data());
			ss.str("");
#endif // _DEBUG
		}

	protected:
		const void* m_pUserData = nullptr;				//����������û���������
		IConsumerHander<T>* m_pHandler = nullptr;		//����¼��ص�ʵ��

		//����δָ��Handlerʱ�����������ص����п�����Ч
		ConsumeStartCallback_t m_ConsumeStartCallback;	//�ص�-�����������
		ConsumeStopCallback_t m_ConsumeStopCallback;	//�ص�-�������ֹͣ
		ConsumingCallback_t m_ConsumingCallback;		//�ص�-��������

		bool m_bStoped = true;							//��ʶ����Ƿ��Ѿ�ֹͣ���У�true��ʾ��ֹͣ��false��ʾ������
		bool m_bForceStop = false;						//�˳��̱߳�־����ʶ����Ƿ�Ҫ����ǿ���˳��������ȴ���������������ɣ�true��ʾ����ǿ���˳���Ĭ��false
		std::mutex m_mtxFlag;							//��ʶ������
		std::mutex m_mtxData;							//���ݻ�����
		std::condition_variable m_cvNewDataOrStoped;	//��������������Ƿ����������ύ�������ֹͣ
		std::queue<T> m_DataQue;						//���ݶ���
		std::vector<std::thread> m_Consumers;			//�������̶߳��󼯺�
	};
}