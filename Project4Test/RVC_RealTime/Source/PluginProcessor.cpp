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
    if (firstProcess == true)
    {
        /*const char* wavData = BinaryData::dry_short_cut_wav;
        int wavSize = BinaryData::dry_short_cut_wavSize;
        juce::MemoryBlock wavBlock(wavData, wavSize);

        juce::AudioBuffer<float> currentAudioBuffer;*/ // 假设已经有一个AudioBuffer对象
        readWav();
        const int numSamples = audioBuffer.getNumSamples();
        
        // 获取AudioBuffer数据指针数组
        const float* data = audioBuffer.getReadPointer(0);

        juce::MemoryBlock memoryBlock(data, numSamples * sizeof(float));

        int bufferSize = 512; // 你的bufferSize值
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

            // 在这里对responseBlock进行处理，例如保存为.wav文件
            // 你可以使用juce::FileOutputStream写入到磁盘上的文件
            juce::FileOutputStream outputStream(juce::String("E:\\miao.wav"));
            outputStream.write(responseBlock.getData(), responseBlock.getSize());
        }

        ////int sampleRate = getSampleRate();
        ////const float* leftChannelData = buffer.getReadPointer(0);
        ////// ½«buffer×ª»»Îª×Ö½ÚÊý×é
        ////const int numSamples = buffer.getNumSamples();
        ////const int numChannels = buffer.getNumChannels();
        ////const int monoBufferSize = numSamples;
        //int sampleRate = 48000;
        //int monoBufferSize = audioBuffer.getNumSamples();
        //std::vector<uint8_t> audioData(audioBuffer.getNumSamples() * sizeof(float));
        //memcpy(audioData.data(), audioBuffer.getReadPointer(0), audioBuffer.getNumSamples() * sizeof(float));

        //// 创建并连接到服务器
        //juce::StreamingSocket socket;
        //if (socket.connect("127.0.0.1", 8080))
        //{
        //    // 构建 form-data 数据
        //    juce::MemoryBlock formBoundaryData;
        //    formBoundaryData.append("--boundary", 10);
        //    formBoundaryData.append("\r\n", 2);
        //    formBoundaryData.append("Content-Disposition: form-data; name=\"sample\"; filename=\"audio.wav\"\r\n", 70);
        //    formBoundaryData.append("Content-Type: audio/wav\r\n", 25);
        //    formBoundaryData.append("Content-Length: ", 16);
        //    formBoundaryData.append("\r\n", 2);
        //    formBoundaryData.append(audioData.data(), audioData.size());
        //    formBoundaryData.append("\r\n", 2);
        //    formBoundaryData.append("--boundary", 10);
        //    formBoundaryData.append("\r\n", 2);
        //    formBoundaryData.append("Content-Disposition: form-data; name=\"sampleRate\"\r\n", 47);
        //    formBoundaryData.append("\r\n", 2);
        //    formBoundaryData.append(juce::String(sampleRate).toRawUTF8(), juce::String(sampleRate).length());
        //    formBoundaryData.append("\r\n", 2);
        //    formBoundaryData.append("--boundary", 10);
        //    formBoundaryData.append("Content-Disposition: form-data; name=\"bufferSize\"\r\n", 49);
        //    formBoundaryData.append("\r\n", 2);
        //    formBoundaryData.append(juce::String(monoBufferSize).toRawUTF8(), juce::String(monoBufferSize).length());
        //    formBoundaryData.append("\r\n", 2);
        //    formBoundaryData.append("--boundary", 10);
        //    formBoundaryData.append("--\r\n", 4);

        //    // 发送 form-data 到服务器
        //    const juce::String postRequest = "POST /voiceChangeModel HTTP/1.1\r\n"
        //        "Host: 127.0.0.1:8080\r\n"
        //        "Content-Type: multipart/form-data; boundary=boundary\r\n"
        //        "Content-Length: " + juce::String(formBoundaryData.getSize()) + "\r\n"
        //        "\r\n";
        //    socket.write(postRequest.toRawUTF8(), postRequest.length());
        //    socket.write(formBoundaryData.getData(), formBoundaryData.getSize());

        //    // 接收服务器返回的数据
        //    juce::String response;
        //    while (socket.isConnected() && socket.waitUntilReady(true, 500))
        //    {
        //        char receivedData[4096];
        //        const int bytesRead = socket.read(receivedData, sizeof(receivedData) - 1, false);
        //        if (bytesRead <= 0)
        //            break;

        //        receivedData[bytesRead] = '\0';
        //        response += receivedData;
        //    }

        //    // 解析服务器返回的数据并用其替换当前buffer
        //    // 这里根据你的具体需求进行解析和处理
        //    // 例如，如果返回的是替换后的音频数据，你可以将其复制回buffer
        //    juce::AudioBuffer<float> resultBuffer(1, 480000);
        //    if (resultBuffer.getNumSamples() == monoBufferSize) {
        //        for (int channel = 0; channel < 1; ++channel) {
        //            float* writePointer = resultBuffer.getWritePointer(channel);
        //            const float* responsePointer = reinterpret_cast<const float*>(response.toRawUTF8());
        //            memcpy(writePointer, responsePointer, monoBufferSize * sizeof(float));
        //            responsePointer += monoBufferSize;
        //        }
        //    }
        //    /*const int responseSize = response.length();
        //    if (responseSize == monoBufferSize) {
        //        for (int channel = 0; channel < 1; ++channel) {
        //            float* writePointer = buffer.getWritePointer(channel);
        //            const float* responsePointer = reinterpret_cast<const float*>(response.toRawUTF8());
        //            memcpy(writePointer, responsePointer, monoBufferSize * sizeof(float));
        //            responsePointer += monoBufferSize;
        //        }
        //    }*/
        //}
    }
    firstProcess = false;
    
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
