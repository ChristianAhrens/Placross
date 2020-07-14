/*
  ==============================================================================

    AudioPlayerTitleTableModel.h
    Created: 13 Jul 2020 3:50:56pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <map>
#include <string>
#include <vector>

//==============================================================================
/*
*
*/
class AudioPlayerTitleTableModel : public TableListBoxModel
{
public:
    //==============================================================================
    AudioPlayerTitleTableModel();
    ~AudioPlayerTitleTableModel() override;

    void setRowBackgroundColours(const Colour &normalColour, const Colour &highlightColour, const Colour &lineColour);

    //==============================================================================
    int getNumRows() override;
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    //==============================================================================
    void addTitles(const std::vector<std::pair<std::string, int>> &titles);
    void addTitle(const std::pair<std::string, int> &titles);
    int getNextTitleId();

    //==============================================================================
    std::function<void()>   titleSelected;

private:
    //==============================================================================
    Colour m_normalColour{ Colours::black };
    Colour m_highlightColour{ Colours::black };
    Colour m_lineColour{ Colours::black };

    //==============================================================================
    std::map<int, std::pair<std::string, int>>  m_IdTitleKV;
    int m_TitleId{ 0 };
};
