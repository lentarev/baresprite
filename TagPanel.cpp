#include "TagPanel.h"

namespace baresprite
{
TagPanel::TagPanel(HWND hWndBottomTolbar, HINSTANCE hInstance, AppState &appState)
    : _hWndBottomTolbar(hWndBottomTolbar), _hInstance(hInstance), _appState(appState)
{
}

TagPanel::~TagPanel() = default;

void TagPanel::OnSize(int clientW, int clientH)
{
}

} // namespace baresprite