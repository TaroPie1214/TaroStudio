/*
  ==============================================================================

    postAudio.h
    Created: 20 Jul 2023 5:08:29pm
    Author:  sunwei06

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// 该文件目的是为了被实例化后启动新线程，并行向后端发送请求并等待回复
// 目前仅为多线程测试代码，未添加具体功能实现
class postAudio : public juce::Thread
{
public:
    postAudio() : juce::Thread("MyThread") {}

    void run() override
    {
        // 在新线程中执行的任务
        for (int i = 0; i < 10; ++i)
        {
            if (threadShouldExit())
                return; // 检查线程是否应该退出

            // 执行任务
            juce::String message = juce::String("Task ") + juce::String(i);
            juce::Logger::writeToLog(message);
            juce::Thread::sleep(1000); // 模拟耗时操作
        }
    }
};