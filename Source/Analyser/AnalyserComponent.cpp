/*
  ==============================================================================

    AnalyserComponent.cpp
    Created: 18 Jul 2020 2:06:17pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "AnalyserComponent.h"

#include "../submodules/JUCE-AppBasics/Source/Image_utils.hpp"

AnalyserComponent::AnalyserComponent() :
    m_fwdFFT(fftOrder),
    m_windowF(fftSize, dsp::WindowingFunction<float>::hann)
{
    startTimer(m_holdTimeMs);
}

AnalyserComponent::~AnalyserComponent()
{
}

void AnalyserComponent::setChannelColours(const std::vector<Colour>& colours)
{
    m_channelColours = colours;
}

void AnalyserComponent::paint(Graphics& g)
{
    OverlayToggleComponentBase::paint(g);

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    // calculate what we need for our center circle
    auto width = getWidth();
    auto height = getHeight();
    auto outerMargin = 20;
    auto visuAreaWidth = width - 2 * outerMargin;
    auto visuAreaHeight = height - 2 * outerMargin;
    auto maxPlotFreq = 20000;
    auto minPlotFreq = 10;

    Rectangle<int> visuArea(outerMargin, outerMargin, visuAreaWidth, visuAreaHeight);

    // fill our visualization area background
    g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
    g.fillRect(visuArea);

    auto visuAreaOrigX = float(outerMargin);
    auto visuAreaOrigY = float(outerMargin + visuAreaHeight);

    for (int ch = 0; ch < m_plotChannels; ++ch)
    {
        // draw rta curve
        if (m_plotPointsPeak.count(ch) && !m_plotPointsPeak.at(ch).empty() && m_plotPointsHold.at(ch).size() == m_plotPointsPeak.at(ch).size())
        {
            auto minPlotIdx = jlimit(0, static_cast<int>(m_plotPointsPeak.at(ch).size() - 1), (minPlotFreq - static_cast<int>(m_minFreq)) / static_cast<int>(m_freqRes));
            auto maxPlotIdx = jlimit(0, static_cast<int>(m_plotPointsPeak.at(ch).size() - 1), (maxPlotFreq - static_cast<int>(m_minFreq)) / static_cast<int>(m_freqRes));

            // hold curve
            auto path = Path{};
            auto skewedProportionX = 1.0f / (log10(maxPlotFreq) - 1.0f) * (log10((minPlotIdx + 1) * m_freqRes) - 1.0f);
            auto newPointX = visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);
            auto newPointY = visuAreaOrigY - m_plotPointsHold.at(ch).at(minPlotIdx) * visuAreaHeight;
            path.startNewSubPath(juce::Point<float>(newPointX, newPointY));
            for (int i = minPlotIdx + 1; i <= maxPlotIdx; ++i)
            {
                skewedProportionX = 1.0f / (log10(maxPlotFreq) - 1.0f) * (log10((i + 1) * m_freqRes) - 1.0f);
                newPointX = visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);
                newPointY = visuAreaOrigY - m_plotPointsHold.at(ch).at(i) * visuAreaHeight;

                path.lineTo(juce::Point<float>(newPointX, newPointY));
            }
            g.setColour(Colours::grey);
            g.strokePath(path, PathStrokeType(1));

            // peak curve
            path = Path{};
            skewedProportionX = 1.0f / (log10(maxPlotFreq) - 1.0f) * (log10((minPlotIdx + 1) * m_freqRes) - 1.0f);
            newPointX = visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);
            newPointY = visuAreaOrigY - m_plotPointsPeak.at(ch).at(minPlotIdx) * visuAreaHeight;
            path.startNewSubPath(juce::Point<float>(newPointX, newPointY));
            for (int i = minPlotIdx + 1; i <= maxPlotIdx; ++i)
            {
                skewedProportionX = 1.0f / (log10(maxPlotFreq) - 1.0f) * (log10((i + 1) * m_freqRes) - 1.0f);
                newPointX = visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);
                newPointY = visuAreaOrigY - m_plotPointsPeak.at(ch).at(i) * visuAreaHeight;

                path.lineTo(juce::Point<float>(newPointX, newPointY));
            }
            jassert(m_channelColours.size() >= ch);
            g.setColour(m_channelColours.at(ch));
            g.strokePath(path, PathStrokeType(3));
        }
    }

    // draw dBFS
    g.setFont(12.0f);
    g.setColour(Colours::grey);
    g.drawText(String(m_minDB) + " ... " + String(m_maxDB) + " dBFS", Rectangle<float>(visuAreaOrigX + visuAreaWidth - 100.0f, float(outerMargin), 110.0f, float(outerMargin)), Justification::centred, true);

    g.setColour(getLookAndFeel().findColour(TableHeaderComponent::ColourIds::outlineColourId));
    // draw marker lines 10Hz, 100Hz, 1000Hz, 10000Hz
    auto markerLineValues = std::vector<float>{ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000 };
    auto markerLegendValues = std::map<int, std::string>{ {10, "10"}, {100, "100"}, {1000, "1k"}, {10000, "10k"}, {20000, "20k"} };
    auto legendValueWidth = 40.0f;
    for (auto i = 0; i < markerLineValues.size(); ++i)
    {
        auto skewedProportionX = 1.0f / (log10(markerLineValues.back()) - 1.0f) * (log10(markerLineValues.at(i)) - 1.0f);
        auto posX = visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);
        g.drawLine(Line<float>(posX, visuAreaOrigY, posX, visuAreaOrigY - visuAreaHeight));

        if (markerLegendValues.count(markerLineValues.at(i)))
            g.drawText(markerLegendValues.at(markerLineValues.at(i)), Rectangle<float>(posX - 0.5f * legendValueWidth, visuAreaOrigY, legendValueWidth, float(outerMargin)), Justification::centred, true);
    }

    // draw an outline around the visu area
    g.drawRect(visuArea, 1);
}

void AnalyserComponent::resized()
{
    OverlayToggleComponentBase::resized();
}

void AnalyserComponent::audioDeviceIOCallback(const float** inputChannelData, int numInputChannels, float** outputChannelData, int numOutputChannels, int numSamples)
{
    const ScopedLock sl(m_audioDataLock);

    AudioBuffer<float> buffer;
    buffer.setSize(numInputChannels, numSamples, false, true, true);
    
    for (int in = 0; in < numInputChannels; ++in)
    {
        buffer.copyFrom(in, 0, inputChannelData[in], numSamples);
    }

    postMessage(new AudioBufferMessage(buffer));
}

void AnalyserComponent::audioDeviceAboutToStart(AudioIODevice* device)
{
    ignoreUnused(device);
    m_sampleRate = device->getCurrentSampleRate();
    m_samplesPerCentiSecond = std::round(m_sampleRate * 0.01f);
    m_bufferSize = device->getCurrentBufferSizeSamples();
    m_missingSamplesForCentiSecond = static_cast<int>(m_samplesPerCentiSecond + 0.5f);
    m_centiSecondBuffer.setSize(2, m_missingSamplesForCentiSecond, false, true, false);
    
    m_plotChannels = device->getActiveInputChannels().toInteger();
    for (int ch = 0; ch < m_plotChannels; ++ch)
    {
        m_plotPointsHold.at(ch).resize(m_freqBands);
        m_plotPointsPeak.at(ch).resize(m_freqBands);
    }
}

void AnalyserComponent::audioDeviceStopped()
{
    m_sampleRate = 0;
    m_samplesPerCentiSecond = 0;
    m_bufferSize = 0;
    m_centiSecondBuffer.clear();
    m_missingSamplesForCentiSecond = 0;

    m_plotChannels = 0;
    m_plotPointsHold.clear();
    m_plotPointsPeak.clear();
}

void AnalyserComponent::audioDeviceError(const juce::String &errorMessage)
{
    ignoreUnused(errorMessage);
}

void AnalyserComponent::handleMessage(const Message& message)
{
    if (auto m = dynamic_cast<const AudioBufferMessage*> (&message))
    {
        auto buffer = m->getAudioBuffer();

        int numChannels = buffer.getNumChannels();
        
        // adjust member vectormaps if data requires it
        if (m_plotChannels < numChannels)
        {
            m_plotChannels = numChannels;
            for (int ch = 0; ch < m_plotChannels; ++ch)
            {
                m_plotPointsHold.insert(std::make_pair(ch, std::vector<float>(m_freqBands)));
                m_plotPointsPeak.insert(std::make_pair(ch, std::vector<float>(m_freqBands)));
            }
        }

        if (numChannels != m_centiSecondBuffer.getNumChannels())
            m_centiSecondBuffer.setSize(numChannels, m_samplesPerCentiSecond, false, true, true);

        int availableSamples = buffer.getNumSamples();

        int readPos = 0;
        int writePos = m_samplesPerCentiSecond - m_missingSamplesForCentiSecond;
        while (availableSamples >= m_missingSamplesForCentiSecond)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                // generate signal buffer data
                m_centiSecondBuffer.copyFrom(ch, writePos, buffer.getReadPointer(ch) + readPos, m_missingSamplesForCentiSecond);

                // generate spectrum data
                {
                    int unprocessedSamples = 0;
                    if (m_FFTdataPos < fftSize)
                    {
                        int missingSamples = fftSize - m_FFTdataPos;
                        if (missingSamples < m_samplesPerCentiSecond)
                        {
                            memcpy(m_FFTdata, m_centiSecondBuffer.getReadPointer(ch), missingSamples);
                            m_FFTdataPos += missingSamples;
                            unprocessedSamples = m_samplesPerCentiSecond - missingSamples;
                        }
                        else
                        {
                            memcpy(m_FFTdata, m_centiSecondBuffer.getReadPointer(ch), m_samplesPerCentiSecond);
                            m_FFTdataPos += m_samplesPerCentiSecond;
                        }
                    }

                    if (m_FFTdataPos >= fftSize)
                    {
                        m_windowF.multiplyWithWindowingTable(m_FFTdata, fftSize);
                        m_fwdFFT.performFrequencyOnlyForwardTransform(m_FFTdata);

                        m_minFreq = m_sampleRate / m_freqBands;
                        m_maxFreq = m_sampleRate / 2;
                        m_freqRes = (m_maxFreq - m_minFreq) / m_freqBands;

                        int spectrumStepWidth = 0.5f * (fftSize / m_freqBands);
                        int spectrumPos = 0;
                        for (int freq = 0; freq < m_freqBands && spectrumPos < fftSize; ++freq)
                        {
                            float spectrumVal = 0;

                            for (int k = 0; k < spectrumStepWidth; ++k, ++spectrumPos)
                                spectrumVal += m_FFTdata[spectrumPos];
                            spectrumVal = spectrumVal / spectrumStepWidth;

                            auto leveldB = jlimit(m_minDB, m_maxDB, Decibels::gainToDecibels(spectrumVal));
                            auto level = jmap(leveldB, m_minDB, m_maxDB, 0.0f, 1.0f);

                            if (m_plotPointsPeak.count(ch) > 0 && m_plotPointsPeak.at(ch).size() > freq)
                            {
                                m_plotPointsPeak.at(ch).at(freq) = level;
                                m_plotPointsHold.at(ch).at(freq) = std::max(level, m_plotPointsHold.at(ch).at(freq));
                            }
                        }

                        zeromem(m_FFTdata, sizeof(m_FFTdata));
                        m_FFTdataPos = 0;
                    }

                    if (unprocessedSamples != 0)
                    {
                        memcpy(m_FFTdata, m_centiSecondBuffer.getReadPointer(ch, m_samplesPerCentiSecond - unprocessedSamples), unprocessedSamples);
                        m_FFTdataPos += unprocessedSamples;
                    }
                }
            }

            readPos += m_missingSamplesForCentiSecond;
            availableSamples -= m_missingSamplesForCentiSecond;

            m_missingSamplesForCentiSecond = m_samplesPerCentiSecond;

            writePos = m_samplesPerCentiSecond - m_missingSamplesForCentiSecond;

            if (availableSamples <= 0)
                break;
        }

        if (availableSamples > 0)
        {
            for (int i = 0; i < numChannels; ++i)
            {
                m_centiSecondBuffer.copyFrom(i, writePos, buffer.getReadPointer(i) + readPos, availableSamples);
            }
        }

        repaint();
    }
}

void AnalyserComponent::changeOverlayState()
{
    OverlayToggleComponentBase::changeOverlayState();

    toggleMinimizedMaximizedElementVisibility(getCurrentOverlayState() == maximized);
}

void AnalyserComponent::toggleMinimizedMaximizedElementVisibility(bool maximized)
{
    //m_sumButton->setVisible(!maximized);
    //
    //for (auto const& nodeButtonRow : m_nodeButtons)
    //    for (auto const& button : nodeButtonRow)
    //        button->setVisible(maximized);
}

void AnalyserComponent::timerCallback()
{
    flushHold();
}

void AnalyserComponent::flushHold()
{
    // clear spectrum hold values
    for (int ch = 0; ch < m_plotChannels; ++ch)
    {
        for (int freq = 0; freq < m_plotPointsHold.at(ch).size(); ++freq)
            m_plotPointsHold.at(ch).at(freq) = 0.0f;
    }
}
