        tresult PLUGIN_API onKeyDown (char16 key, int16 keyCode, int16 modifiers) override
        {
            // Steinberg's VirtualKeyCodes::KEY_SPACE (the JUCE SDK subset omits keycodes.h).
            constexpr int16 vstSpaceKey = 7;
            if (modifiers != 0 || (keyCode != vstSpaceKey && key != u' ')
                || ! isAttached() || component == nullptr)
                return kResultFalse;

            auto* editor = static_cast<TalkCaptureAudioProcessorEditor*>(component->pluginEditor.get());
            return editor != nullptr && editor->handleHostSpaceKey() ? kResultTrue : kResultFalse;
        }
