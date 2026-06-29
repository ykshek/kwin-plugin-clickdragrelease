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

    class ContextMenuDragFilter : public Plugin, public InputEventFilter
    {
        Q_OBJECT
    public:
        explicit ContextMenuDragPlugin(QObject *parent, const QVariantList &args)
        : Plugin()
        // Initialize the filter order using the public InputFilterOrder enum
        , InputEventFilter(InputFilterOrder::Order::InputMethod)
        {
            Q_UNUSED(parent);
            Q_UNUSED(args);
        }

        bool pointerButton(PointerButtonEvent *event) override
        {
            if (event->button != Qt::RightButton) return false;

            if (event->state == PointerButtonState::Pressed) {
                m_delayedPressEvent = *event;
                m_pressPos = event->pos();
                m_isWithholding = true;
                return true;
            }

            if (event->state == PointerButtonState::Released) {
                if (m_isWithholding) {
                    m_isWithholding = false;
                }
                return false;
            }
            return false;
        }

        bool pointerMotion(PointerMotionEvent * /*event*/) override
        {
            if (m_isWithholding) {
                qreal distance = QLineF(m_pressPos, event->pos()).length();
                if (distance > QGuiApplication::styleHints()->startDragDistance()) {
                    m_isWithholding = false;
                }
            }
            return false;
        }

    private:
        bool m_isWithholding = false;
        PointerButtonEvent m_delayedPressEvent;
        QPointF m_pressPos;
    };

} // namespace KWin

K_PLUGIN_FACTORY_WITH_JSON(ContextMenuDragPluginFactory, "kwin_contextmenudrag.json",
                           registerPlugin<KWin::ContextMenuDragPlugin>();
)

#include "contextmenudragplugin.moc"
