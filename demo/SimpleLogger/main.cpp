/*!
 * �ļ� main.cpp
 * ���� huge
 * ���� 2022/09/08
 * ���� ���Դ��룬���Ի���Consumer���ʵ�ֵļ���־�����SimpleLogger
 */

#include "SimpleLogger.h"

int main(int argc, char* argv[])
{
	logger::SimpleLogger mylogger;					//����һ����־����
	mylogger.Start("test.log");						//��������־�������־�ļ�·��Ϊ��ǰĿ¼��"test.log"
	for (size_t i = 0; i < 10; i++)					//�����ύ��������־��Ϣ
		mylogger.Commit("�µ���־����:%zu.", i + 1);
	mylogger.Stop();								//�ر���־���
	return 0;
}