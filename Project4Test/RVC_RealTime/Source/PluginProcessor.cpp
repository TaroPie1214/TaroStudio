/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RVC_RealTimeAudioProcessor::RVC_RealTimeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    formatManager.registerBasicFormats();
}

RVC_RealTimeAudioProcessor::~RVC_RealTimeAudioProcessor()
{
}

//==============================================================================
const juce::String RVC_RealTimeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RVC_RealTimeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RVC_RealTimeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RVC_RealTimeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RVC_RealTimeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RVC_RealTimeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RVC_RealTimeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RVC_RealTimeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RVC_RealTimeAudioProcessor::getProgramName (int index)
{
    return {};
}

void RVC_RealTimeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RVC_RealTimeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void RVC_RealTimeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RVC_RealTimeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RVC_RealTimeAudioProcessor::setAudioBufferSize(int newNumChannels, int newNumSamples, bool keepExistingContent, bool clearExtraSpace, bool avoidRealLocationg)
{
    audioBuffer.setSize(newNumChannels, newNumSamples, keepExistingContent, clearExtraSpace, avoidRealLocationg);
}

juce::AudioBuffer<float>& RVC_RealTimeAudioProcessor::getAudioBuffer()
{
    return audioBuffer;
}

// 固定读取Assets文件夹下的wav文件，并存入变量audioBuffer
void RVC_RealTimeAudioProcessor::readWav()
{
    auto* reader = formatManager.createReaderFor(std::make_unique<juce::MemoryInputStream>(BinaryData::dry_short_cut_wav, BinaryData::dry_short_cut_wavSize, false));
    if (reader != nullptr)
    {
        audioBufferSampleRate = reader->sampleRate;
        setAudioBufferSize(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
        reader->read(&getAudioBuffer(), 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
    }
}

void RVC_RealTimeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    //if (firstProcess == true)
    //{
        //postWav();
        write2CircularBuffer(buffer);
    //}
    //firstProcess = false;
}

// 写入circular buffer
void RVC_RealTimeAudioProcessor::write2CircularBuffer(juce::AudioBuffer<float>& inputBuffer)
{
    // 如果此时circular buffer中还放得下当前inputBuffer的内容，直接写入
    if (writePointer + inputBuffer.getNumSamples() < circularBuffer.getNumSamples())
    {
        circularBuffer.copyFrom(0, writePointer, inputBuffer, 0, 0, inputBuffer.getNumSamples());
        writePointer += inputBuffer.getNumSamples();
    }
    else
    {
        // 如果此时circular buffer中放不下当前inputBuffer的内容，先写入circular buffer的末尾，再写入circular buffer的开头
		int firstPartSize = circularBuffer.getNumSamples() - writePointer;
		circularBuffer.copyFrom(0, writePointer, inputBuffer, 0, 0, firstPartSize);

        // circular buffer被写满，将其内容传给backend
        // 注：此步骤为优化重点，由于请求和模型推理的时间消耗，需要修改为异步并行处理
        postAudio2Backend(inputBuffer);

		circularBuffer.copyFrom(0, 0, inputBuffer, 0, firstPartSize, inputBuffer.getNumSamples() - firstPartSize);
		writePointer = inputBuffer.getNumSamples() - firstPartSize;
    }
}

// 向后端发送音频数据
void RVC_RealTimeAudioProcessor::postAudio2Backend(juce::AudioBuffer<float>& buffer)
{
    int numSamples = circularBuffer.getNumSamples();

    // 获取AudioBuffer数据指针数组
    const float* data = circularBuffer.getReadPointer(0);

    juce::MemoryBlock memoryBlock(data, numSamples * sizeof(float));

    int bufferSize = circularBuffer.getNumSamples(); // 你的bufferSize值
    int sampleRate = getSampleRate(); // 你的sampleRate值

    // 将bufferSize和sampleRate转换为字节序列
    juce::MemoryBlock bufferSizeBlock(sizeof(bufferSize));
    juce::MemoryBlock sampleRateBlock(sizeof(sampleRate));

    bufferSizeBlock.append(&bufferSize, sizeof(bufferSize));
    sampleRateBlock.append(&sampleRate, sizeof(sampleRate));

    // 将bufferSize和sampleRate的字节序列追加到wavBlock中
    // 后端收到数据后需要首先解析提取出buffer size和sample rate的内容，具体如何提取已在后端代码中标注
    memoryBlock.append(bufferSizeBlock.getData(), bufferSizeBlock.getSize());
    memoryBlock.append(sampleRateBlock.getData(), sampleRateBlock.getSize());

    auto url = juce::URL("http://127.0.0.1:8080/voiceChangeModel")
        .withPOSTData(memoryBlock);

    // 发送请求，并得到回复
    std::unique_ptr<juce::InputStream> response = url.createInputStream(juce::URL::InputStreamOptions((juce::URL::ParameterHandling::inPostData)));

    if (response != nullptr)
    {
        juce::MemoryBlock responseBlock;
        response->readIntoMemoryBlock(responseBlock);

        responseBlock.removeSection(0, 44);
        DBG(responseBlock.getSize());

        float* audioData = buffer.getWritePointer(0); // 获取写入指针
        const float* binaryFloatData = reinterpret_cast<const float*>(responseBlock.getData()); // 获取二进制数据的float指针
        int binaryDataSize = responseBlock.getSize() / sizeof(float); // 计算二进制数据的大小
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            if (i < binaryDataSize) {
                audioData[i] = binaryFloatData[i]; // 将二进制数据复制到AudioBuffer中
            }
            else {
                audioData[i] = 0.0f; // 如果二进制数据不足，将剩余位置填充为0
            }
        }
    }
}

// 用于测试通信使用，发送固定的wav文件，并得到返回结果进行通信验证
void RVC_RealTimeAudioProcessor::postWav()
{
    readWav();
    const int numSamples = audioBuffer.getNumSamples();

    // 获取AudioBuffer数据指针数组
    const float* data = audioBuffer.getReadPointer(0);

    juce::MemoryBlock memoryBlock(data, numSamples * sizeof(float));

    int bufferSize = 480000; // 你的bufferSize值
    int sampleRate = 48000; // 你的sampleRate值

    // 将bufferSize和sampleRate转换为字节序列
    juce::MemoryBlock bufferSizeBlock(sizeof(bufferSize));
    juce::MemoryBlock sampleRateBlock(sizeof(sampleRate));

    bufferSizeBlock.append(&bufferSize, sizeof(bufferSize));
    sampleRateBlock.append(&sampleRate, sizeof(sampleRate));

    //// 将bufferSize和sampleRate的字节序列追加到wavBlock中
    memoryBlock.append(bufferSizeBlock.getData(), bufferSizeBlock.getSize());
    memoryBlock.append(sampleRateBlock.getData(), sampleRateBlock.getSize());

    auto url = juce::URL("http://127.0.0.1:8080/voiceChangeModel")
        .withPOSTData(memoryBlock);

    std::unique_ptr<juce::InputStream> response = url.createInputStream(juce::URL::InputStreamOptions((juce::URL::ParameterHandling::inPostData)));

    if (response != nullptr)
    {
        juce::MemoryBlock responseBlock;
        response->readIntoMemoryBlock(responseBlock);

        // 把返回的结果写入指定目录
        juce::FileOutputStream outputStream(juce::String("E:\\test.wav"));
        outputStream.write(responseBlock.getData(), responseBlock.getSize());
    }
}

//==============================================================================
bool RVC_RealTimeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RVC_RealTimeAudioProcessor::createEditor()
{
    return new RVC_RealTimeAudioProcessorEditor (*this);
}

//==============================================================================
void RVC_RealTimeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RVC_RealTimeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RVC_RealTimeAudioProcessor();
}
