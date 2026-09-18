#include "PreviewPanel.h"

namespace frazil::water::preview {
class PreviewApplication final : public juce::JUCEApplication {
  public:
    const juce::String getApplicationName() override {
        return "FRAZIL Water Research Preview";
    }
    const juce::String getApplicationVersion() override {
        return "0.1.0";
    }
    void initialise(const juce::String& commandLine) override {
        window = std::make_unique<Window>(getApplicationName(), commandLine.trim());
    }
    void shutdown() override {
        window.reset();
    }

  private:
    // Keep all engineering controls and meter readouts legible on smaller displays.
    class ScrollablePanel final : public juce::Viewport {
      public:
        explicit ScrollablePanel(const juce::String& source) {
            setScrollBarsShown(true, false);
            setViewedComponent(new PreviewPanel(source), true);
            setSize(1180, 840);
        }
        void resized() override {
            juce::Viewport::resized();
            if (auto* panel = getViewedComponent())
                panel->setSize(getWidth() - getScrollBarThickness(), std::max(1595, getHeight()));
        }
    };
    class Window final : public juce::DocumentWindow {
      public:
        Window(const juce::String& name, const juce::String& source)
            : DocumentWindow(name, juce::Colour(0xff0d181f), allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(new ScrollablePanel(source), true);
            setResizable(true, false);
            setResizeLimits(1080, 700, 1600, 1100);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }
        void closeButtonPressed() override {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };
    std::unique_ptr<Window> window;
};
} // namespace frazil::water::preview

START_JUCE_APPLICATION(frazil::water::preview::PreviewApplication)
