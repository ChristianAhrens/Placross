/*
  ==============================================================================

    AnalyserComponent.h
    Created: 18 Jul 2020 2:06:17pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../submodules/JUCE-AppBasics/Source/OverlayToggleComponentBase.h"

//==============================================================================
/*
*/
class AudioBufferMessage : public Message
{
public:
    AudioBufferMessage(AudioBuffer<float>& buffer) { m_buffer = buffer; };
    ~AudioBufferMessage() {};

    const AudioBuffer<float>& getAudioBuffer() const { return m_buffer; };

private:
    AudioBuffer<float> m_buffer;
};

//==============================================================================
class AnalyserComponent :   public JUCEAppBasics::OverlayToggleComponentBase,
                            public AudioIODeviceCallback,
                            public MessageListener,
                            public Timer
{
public:
    //==============================================================================
    AnalyserComponent();
    ~AnalyserComponent() override;

    //==============================================================================
    void paint(Graphics& g) override;
    void resized() override;

    //==============================================================================
    void audioDeviceIOCallback(const float** inputChannelData,
        int numInputChannels,
        float** outputChannelData,
        int numOutputChannels,
        int numSamples) override;
    void audioDeviceAboutToStart(AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;

    //==============================================================================
    void timerCallback() override;

protected:
    void changeOverlayState() override;

private:
    void flushHold();

    //==============================================================================
    void toggleMinimizedMaximizedElementVisibility(bool maximized);

    //==============================================================================
    void handleMessage(const Message& message) override;

    //==============================================================================
    CriticalSection     m_audioDataLock{};
    double              m_sampleRate = 0;
    double              m_samplesPerCentiSecond = 0;
    int                 m_bufferSize = 0;

    float               *m_processorChannels[128];
    AudioBuffer<float>  m_buffer;
    int                 m_missingSamplesForCentiSecond;
    AudioBuffer<float>  m_centiSecondBuffer;

    const float* inputChans[128]; // this is only a member to enshure it is not recreated on every function call

    //==============================================================================
    enum
    {
        fftOrder = 12,
        fftSize = 1 << fftOrder
    };
    dsp::FFT                        m_fwdFFT;
    dsp::WindowingFunction<float>   m_windowF;
    float                           m_FFTdata[2 * fftSize];
    int                             m_FFTdataPos{ 0 };

    int                             m_holdTimeMs{ 500 };

    float m_minDB{ -90 };
    float m_maxDB{ 0 };
    float m_minFreq{ 20 };
    float m_maxFreq{ 20000 };
    int m_freqBands{ 1024 };
    int m_freqRes{ 20 };

    int                                 m_plotChannels;
    std::map<int, std::vector<float>>   m_plotPointsPeak;
    std::map<int, std::vector<float>>   m_plotPointsHold;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserComponent)
};
