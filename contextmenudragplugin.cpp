#include <QGuiApplication>
#include <QStyleHints>
#include <QScopedValueRollback>
#include <QLineF>

#include <kwin_export.h>
#include <plugin.h>
#include <input.h>
#include <input_event.h>
#include <KPluginFactory>

namespace KWin {

    class ContextMenuDragFilter : public InputEventFilter
    {
    public:
        // Use an existing order value – InputMethod places us after the input‑method filter
        ContextMenuDragFilter() : InputEventFilter(InputFilterOrder::Order::InputMethod) {}

        bool pointerButton(PointerButtonEvent *event) override
        {
            if (m_isInjecting) return false;
            if (event->button != Qt::RightButton) return false;

            if (event->state == PointerButtonState::Pressed) {
                m_delayedPressEvent = *event;
                m_pressPos = input()->globalPointer();
                m_isWithholding = true;
                return true;
            }

            if (event->state == PointerButtonState::Released) {
                if (m_isWithholding) {
                    m_isWithholding = false;
                    m_delayedPressEvent.timestamp = event->timestamp;
                    QScopedValueRollback<bool> injectionGuard(m_isInjecting, true);
                    input()->processFilters(&InputEventFilter::pointerButton, &m_delayedPressEvent);
                }
                return false;
            }
            return false;
        }

        bool pointerMotion(PointerMotionEvent * /*event*/) override
        {
            if (m_isWithholding) {
                qreal distance = QLineF(m_pressPos, input()->globalPointer()).length();
                if (distance > QGuiApplication::styleHints()->startDragDistance()) {
                    m_isWithholding = false;
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
        explicit ContextMenuDragPlugin(QObject *parent, const QVariantList &args)
        : Plugin()
        {
            Q_UNUSED(parent); Q_UNUSED(args);
            m_filter = std::make_unique<ContextMenuDragFilter>();
            input()->installInputEventFilter(m_filter.get());
        }

    private:
        std::unique_ptr<ContextMenuDragFilter> m_filter;
    };

} // namespace KWin

K_PLUGIN_FACTORY_WITH_JSON(ContextMenuDragPluginFactory, "metadata.json",
                           registerPlugin<KWin::ContextMenuDragPlugin>();
)

#include "contextmenudragplugin.moc"
