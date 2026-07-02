#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include <KPluginMetaData>

#include <iostream>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        std::cout << "Usage: plugin_tester <path-to-plugin-so-or-json>" << std::endl;
        return 1;
    }

    const QString path = QString::fromLocal8Bit(argv[1]);
    std::cout << "Path: " << path.toStdString() << std::endl;

    // Read metadata using KPluginMetaData (use constructor for compatibility)
    KPluginMetaData meta(path);

    std::cout << "isValid: " << (meta.isValid() ? "true" : "false") << std::endl;
    std::cout << "Name: " << meta.name().toStdString() << std::endl;
    std::cout << "Description: " << meta.description().toStdString() << std::endl;
    std::cout << "Version: " << meta.version().toStdString() << std::endl;
    std::cout << "BugReportUrl: " << meta.bugReportUrl().toStdString() << std::endl;

    // Extract ServiceTypes from raw JSON to support KF6 variants
    const QJsonObject raw = meta.rawData();
    if (!raw.isEmpty()) {
        QJsonObject kplugin = raw.value("KPlugin").toObject();
        QJsonArray svc = kplugin.value("ServiceTypes").toArray();
        std::cout << "ServiceTypes:";
        for (const QJsonValue &v : svc) {
            if (v.isString()) std::cout << " " << v.toString().toStdString();
        }
        std::cout << std::endl;

        // Print full raw JSON for inspection (packaging metadata may be embedded)
        QJsonDocument doc(raw);
        std::cout << "Raw JSON: " << doc.toJson(QJsonDocument::Indented).toStdString() << std::endl;
    } else {
        std::cout << "No raw JSON available in metadata." << std::endl;
    }

    // Additionally, attempt to sniff embedded rpm-style metadata strings in the binary (best-effort)
    if (path.endsWith(".so") && QFile::exists(path)) {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly)) {
            const QByteArray bytes = f.read(65536); // read first 64KiB
            f.close();
            const QByteArray marker = "\"version\":";
            int idx = bytes.indexOf(marker);
            if (idx >= 0) {
                int start = bytes.indexOf('"', idx + (int)marker.size());
                if (start >= 0) {
                    int end = bytes.indexOf('"', start + 1);
                    if (end > start) {
                        QByteArray ver = bytes.mid(start + 1, end - start - 1);
                        std::cout << "Embedded version-like string (snippet): " << ver.toStdString() << std::endl;
                    }
                }
            }
        }
    }

    return 0;
}
