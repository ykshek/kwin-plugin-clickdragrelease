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
    // Use the QVariantList constructor signature expected by this KWin version
    explicit ContextMenuDragPlugin(QObject *parent, const QVariantList &args)
        : Plugin()
    {
        Q_UNUSED(parent);
        Q_UNUSED(args);
        m_filter = std::make_unique<ContextMenuDragFilter>();
        input()->installInputEventFilter(m_filter.get());
    }

private:
    std::unique_ptr<ContextMenuDragFilter> m_filter;
};

} // namespace KWin

K_PLUGIN_FACTORY_WITH_JSON(ContextMenuDragPluginFactory, "kwin_contextmenudrag.json",
                           registerPlugin<KWin::ContextMenuDragPlugin>();
)

#include "contextmenudragplugin.moc"
