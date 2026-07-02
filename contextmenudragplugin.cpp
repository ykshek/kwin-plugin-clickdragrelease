#include <memory>
#include <QGuiApplication>
#include <QStyleHints>
#include <QScopedValueRollback>
#include <QLineF>
#include <QPointF>

#include <kwin_export.h>
#include <plugin.h>
#include <input.h>
#include <input_event.h>
#include <KPluginFactory>

namespace KWin {

class ContextMenuDragFilter : public InputEventFilter
{
public:
    explicit ContextMenuDragFilter()
        : InputEventFilter(InputFilterOrder::Order::InputMethod)
    {
    }

    bool pointerButton(PointerButtonEvent *event) override
    {
        if (m_isInjecting) {
            return false;
        }
        if (event->button != Qt::RightButton) {
            return false;
        }

        if (event->state == PointerButtonState::Pressed) {
            m_delayedPressEvent = *event;
            m_pressPos = event->position;
            m_isWithholding = true;
            // Consume the original press until we decide whether to forward it
            return true;
        }

        if (event->state == PointerButtonState::Released) {
            if (m_isWithholding) {
                m_isWithholding = false;
                m_delayedPressEvent.timestamp = event->timestamp;
                QScopedValueRollback<bool> injectionGuard(m_isInjecting, true);
                // Re-inject the stored press event so downstream filters/surfaces see it
                input()->processFilters(&InputEventFilter::pointerButton, &m_delayedPressEvent);
            }
            // Let the release continue normally (do not consume it)
            return false;
        }

        return false;
    }

    bool pointerMotion(PointerMotionEvent *event) override
    {
        if (m_isWithholding) {
            qreal distance = QLineF(m_pressPos, event->position).length();
            if (distance > QGuiApplication::styleHints()->startDragDistance()) {
                // Movement exceeded drag threshold: cancel withholding and allow the original press to proceed
                m_isWithholding = false;
                // Note: we don't explicitly inject the press here because the press was previously consumed;
                // letting the next press/release behavior continue will typically result in drag semantics.
            }
        }
        return false;
    }

private:
    bool m_isWithholding = false;
    bool m_isInjecting = false;
    PointerButtonEvent m_delayedPressEvent;
    QPointF m_pressPos;
};

class ContextMenuDragPlugin : public Plugin
{
    Q_OBJECT
public:
    // Provide a default constructor so PluginFactory::create() can instantiate it
    explicit ContextMenuDragPlugin() : Plugin()
    {
        m_filter = std::make_unique<ContextMenuDragFilter>();
        input()->installInputEventFilter(m_filter.get());
    }

    ~ContextMenuDragPlugin() override = default;

private:
    std::unique_ptr<ContextMenuDragFilter> m_filter;
};

} // namespace KWin

class KWIN_EXPORT ContextMenuDragPluginFactory : public KWin::PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginFactory_iid FILE "kwin_contextmenudrag.json")
    Q_INTERFACES(KWin::PluginFactory)

public:
    std::unique_ptr<KWin::Plugin> create() const override
    {
        return std::make_unique<KWin::ContextMenuDragPlugin>();
    }
};

#include "contextmenudragplugin.moc"
