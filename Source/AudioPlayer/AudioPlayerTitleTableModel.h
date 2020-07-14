/*
  ==============================================================================

    AudioPlayerTitleTableModel.h
    Created: 13 Jul 2020 3:50:56pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*
*/
class AudioPlayerTitleTableModel : public TableListBoxModel
{
public:
    AudioPlayerTitleTableModel();
    ~AudioPlayerTitleTableModel() override;

    void setRowBackgroundColours(Colour &normalColour, Colour &highlightColour, Colour &lineColour);

    int getNumRows() override;
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

private:
    Colour m_normalColour{ Colours::black };
    Colour m_highlightColour{ Colours::black };
    Colour m_lineColour{ Colours::black };
};