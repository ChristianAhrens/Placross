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

    void setCellColours(const Colour &normalColour, const Colour &highlightColour, const Colour &lineColour, const Colour &textColour);

    //==============================================================================
    int getNumRows() override;
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void cellClicked (int rowNumber, int columnId, const MouseEvent&) override;
    void cellDoubleClicked (int rowNumber, int columnId, const MouseEvent&) override;

    //==============================================================================
    std::vector<int> addTitles(const std::vector<std::pair<std::string, int>> &titles);
    int addTitle(const std::pair<std::string, int> &title);
    int getNextTitleId();

    //==============================================================================
    std::function<void(String)>   titleSelected;

private:
    //==============================================================================
    Colour m_normalColour{ Colours::black };
    Colour m_highlightColour{ Colours::black };
    Colour m_lineColour{ Colours::black };
    Colour m_textColour{ Colours::black };

    //==============================================================================
    std::map<int, std::pair<std::string, int>>  m_IdTitleKV;
    int m_TitleId{ 0 };
};
