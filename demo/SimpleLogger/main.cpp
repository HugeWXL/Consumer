#include "SimpleLogger.h"

int main(int argc, char* argv[])
{
	logger::SimpleLogger mylogger;					//������־����
	mylogger.Start("test.log");						//ָ����־�ļ�·������������־���
	for (size_t i = 0; i < 10; i++)					//�����ύ��������־��Ϣ
		mylogger.Commit("�µ���־����:%zu.", i + 1);
	mylogger.Stop();								//�ر���־���
	return 0;
}