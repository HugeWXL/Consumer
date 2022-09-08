/*!
 * 文件 main.cpp
 * 作者 huge
 * 日期 2022/09/08
 * 描述 测试代码，测试基于Consumer组件实现的简单日志组件类SimpleLogger
 */

#include "SimpleLogger.h"

int main(int argc, char* argv[])
{
	logger::SimpleLogger mylogger;					//创建一个日志对象
	mylogger.Start("test.log");						//并启动日志组件，日志文件路径为当前目录的"test.log"
	for (size_t i = 0; i < 10; i++)					//连续提交若干条日志消息
		mylogger.Commit("新的日志内容:%zu.", i + 1);
	mylogger.Stop();								//关闭日志组件
	return 0;
}