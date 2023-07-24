/*
  ==============================================================================

    postAudio.h
    Created: 20 Jul 2023 5:08:29pm
    Author:  sunwei06

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// ���ļ�Ŀ����Ϊ�˱�ʵ�������������̣߳��������˷������󲢵ȴ��ظ�
// Ŀǰ��Ϊ���̲߳��Դ��룬δ��Ӿ��幦��ʵ��
class postAudio : public juce::Thread
{
public:
    postAudio() : juce::Thread("MyThread") {}

    void run() override
    {
        // �����߳���ִ�е�����
        for (int i = 0; i < 10; ++i)
        {
            if (threadShouldExit())
                return; // ����߳��Ƿ�Ӧ���˳�

            // ִ������
            juce::String message = juce::String("Task ") + juce::String(i);
            juce::Logger::writeToLog(message);
            juce::Thread::sleep(1000); // ģ���ʱ����
        }
    }
};