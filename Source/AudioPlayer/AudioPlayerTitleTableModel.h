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

    int getNumRows() override;
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
};