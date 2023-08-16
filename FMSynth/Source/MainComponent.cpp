#include "MainComponent.h"
MainContentComponent::MainContentComponent()
: keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
synthAudioSource(keyboardState, fmodSlider, modIndexSlider)
{
    
    fmodSlider.setRange(100, 2000);
    modIndexSlider.setRange(1, 8);

    addAndMakeVisible(modIndexSlider);
    addAndMakeVisible(fmodSlider);
    
    addAndMakeVisible(keyboardComponent);
    
    setAudioChannels(0, 2);

    setSize(600, 400);
    startTimer(400);
}

//====================================================================================================================
void FMWaveVoice::startNote(int midiNoteNumber, float velocity,
    juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    currentAngle = 0.0;
    currentAngleMod = 0.0;
    
    level = velocity * 0.15;
    tailOff = 0.0;

    updateAngleDelta(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    updateAngleDeltaMod();
}

void FMWaveVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        if (tailOff == 0.0)
            tailOff = 1.0;
    }
    else
    {
        clearCurrentNote();
        angleDelta = 0.0;
        angleDeltaMod = 0.0;
    }
}

//====================================================================================================================
void FMWaveVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (angleDelta != 0.0)
    {
        if (tailOff > 0.0)
        {
            while (--numSamples >= 0)
            {
                auto modIndex = modIndexSlider.getValue();
                auto modValue = std::sin(currentAngleMod);
                auto currentSample = (float)(std::sin(currentAngle + modIndex * modValue) * level);

                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);

                currentAngle += angleDelta;
                currentAngleMod += angleDeltaMod;
                
                ++startSample;

                tailOff *= 0.99;

                if (tailOff <= 0.005)
                {
                    clearCurrentNote();
                    angleDelta = 0.0;
                    break;
                }
            }
        }
        else
            
        {
            while (--numSamples >= 0)
            {
                auto modIndex = modIndexSlider.getValue();
                auto modValue = std::sin(currentAngleMod);
                auto currentSample = (float)(std::sin(currentAngle + modIndex * modValue) * level);

                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);

                currentAngle += angleDelta;
                currentAngleMod += angleDeltaMod;
                ++startSample;
            }
        }
             
    }
}

void SynthAudioSource::setUsingSineWaveSound()
{
    synth.clearSounds();
}

void SynthAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer incomingMidi;
    keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample,
        bufferToFill.numSamples, true);

    synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
        bufferToFill.startSample, bufferToFill.numSamples);
}

//====================================================================================================================

void MainContentComponent::resized()
{
    
    keyboardComponent.setBounds(0, 200, getWidth(), getHeight() - 200);
    fmodSlider.setBounds (10, 50, getWidth() - 20, 20);
    modIndexSlider.setBounds (10, 100, getWidth() - 20, 20);
    //modIndexSlider.setBounds(10, 150, getWidth() - 20, 20);
    
}

void MainContentComponent::timerCallback()
{
    keyboardComponent.grabKeyboardFocus();
    stopTimer();
}

//====================================================================================================================
void FMWaveVoice::updateAngleDelta(double cyclesPerSecond)
{
    //auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    auto cyclesPerSample = cyclesPerSecond / getSampleRate();
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

void FMWaveVoice::updateAngleDeltaMod()
{
    auto fmodCyclesPerSecond = fmodSlider.getValue();
    auto fmodCyclesPerSample = fmodCyclesPerSecond / getSampleRate();
    angleDeltaMod = fmodCyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}
