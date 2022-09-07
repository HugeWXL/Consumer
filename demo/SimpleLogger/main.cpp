#include "SimpleLogger.h"

int main(int argc, char* argv[])
{
	logger::SimpleLogger mylogger;					//创建日志对象
	mylogger.Start("test.log");						//指定日志文件路径，并启动日志组件
	for (size_t i = 0; i < 10; i++)					//连续提交若干条日志消息
		mylogger.Commit("新的日志内容:%zu.", i + 1);
	mylogger.Stop();								//关闭日志组件
	return 0;
}